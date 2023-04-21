#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stack>

#include "CodeGenerator.h"
#include "Tables.h"
#include "variable.h"

using namespace std;

// Принимает два операнда операции сложения (с сохранением результата).
// Возвращает ассемблерный эквивалент.
string AddTemplate(string operand1, string operand2, string savedRes)
{
   string code_template =
      "MOV EAX, " + operand1 + "\n" +
      "MOV EBX, " + operand2 + "\n" +
      "ADD EAX, EBX" + "\n" +
      "MOV " + savedRes + ", EAX" + "\n\n";

   return code_template;
}

// Принимает два операнда операции вычитания (с сохранением результата).
// Возвращает ассемблерный эквивалент.
string SubTemplate(string operand1, string operand2, string savedRes)
{
   string code_template =
      "MOV EAX, " + operand1 + "\n" +
      "MOV EBX, " + operand2 + "\n" +
      "SUB EAX, EBX" + "\n" +
      "MOV " + savedRes + ", EAX" + "\n\n";

   return code_template;
}

// Принимает два операнда операции умножения (с сохранением результата).
// Возвращает ассемблерный эквивалент.
string MulTemplate(string operand1, string operand2, string savedRes)
{
   string code_template =
      "MOV EAX, " + operand1 + "\n" +
      "MOV EBX, " + operand2 + "\n" +
      "MUL EBX" + "\n" +
      "MOV " + savedRes + ", EAX" + "\n\n";

   return code_template;
}

// Принимает два операнда - переменную и присваеваемое значение.
// Возвращает ассемблерный эквивалент.
string AssignTemplate(string operand1, string operand2)
{
   string code_template =
      "MOV EAX, " + operand2 + "\n" +
      "MOV " + operand1 + ", EAX \n\n";
   return code_template;
}

// Принимает два операнда любой логической операции (< == !=).
// Возвращает ассемблерный эквивалент.
string CompareTemplate(string operand1, string operand2)
{
   string code_template =
      "MOV EAX, " + operand1 + "\n" +
      "MOV EBX, " + operand2 + "\n" +
      "CMP EAX, EBX" + "\n";

   return code_template;
}

// Принимает имя последнего логического токена и имя метки перехода.
// Возвращает ассемблерный эквивалент условного перехода по лжи.
string CondTransByLie(string logicalOperator, string markName)
{
   if (logicalOperator == "<")
      return "JGE " + markName + "\n\n";

   if (logicalOperator == "!=")
      return "JE " + markName + "\n\n";

   if (logicalOperator == "==")
      return "JNE " + markName + "\n\n";

   return "err";
}

// Принимает имя метки. 
// Возвращает ассемблерный эквивалент безусловного перехода на метку.
string NonCondTrans(string markName)
{
   return "JMP " + markName + "\n\n";
}

string CodeGenerator::GetRealTokenName(Token token)
{
   pair <int, int> addr;
   addr.first = token.tableID;
   addr.second = token.rowID;

   switch (token.tableID)
   {
      case DynamicConstants:
         return tables.SearchOnDynamic(addr)->Name;

      case DynamicVariables:
         return "_" + tables.SearchOnDynamic(addr)->Name;

      case DynamicMark:
         return "m" + to_string(token.rowID);

      case DynamicMarkGo:
         return "m" + to_string(token.rowID) + ":";

      case DynamicLogic:
      {
         if (token.rowID == 0)
            return "UPL";
         if (token.rowID == 1)
            return "BP";
      }

      case StaticOperators:
         return tables.GetStaticLex(pair<int, int>({ StaticOperators, token.rowID }));
         break;

      case DynamicWorkingVar:
         return "r" + to_string(token.rowID);
         break;
   }

   return "err";
}

// -----------------------------------------------------------
// CODE GENERATOR
// -----------------------------------------------------------

CodeGenerator::CodeGenerator(Tables tables, vector<Token> polish)
{
   this->tables = tables;
   this->polish = polish;
}


// Пример стека 
// a 6 = b 2 = a b < m0 UPL c b a - = a 1 == m1 UPL
// c a b + = m1: m2 BP m0: a 1 == m3 UPL c a b - = m3: a 5 = m2:
void CodeGenerator::Generate(string filename)
{
   string code_str = "";
   string lastLogicalOp;
   vector<string> dynamicTableStrs;

   stack<Token> operandStack;
   size_t greaterWorkVar = 0;
   size_t mindex = 0;

   stringstream sstr;
   ofstream fout(filename); // Выходной файл.

   sstr << ".CODE  ; Сегмент кода. \n"
        << "MAIN PROC \n\n";

   for (auto token : polish)
   {
      code_str = "";
      switch (token.tableID)
      {
         case DynamicMark:
         case DynamicVariables:
         case DynamicConstants:
            operandStack.push(token);
            break;

         case DynamicLogic:
         case StaticOperators:
         {
            string tokenstring = GetRealTokenName(token);
            if (tokenstring == "UPL")
            {
               Token mark = operandStack.top();
               operandStack.pop();
               code_str = CondTransByLie(lastLogicalOp, GetRealTokenName(mark));
               break;
            }

            if (tokenstring == "BP")
            {
               Token mark = operandStack.top();
               operandStack.pop();
               code_str = NonCondTrans(GetRealTokenName(mark));
               break;
            }

            Token tokOp2 = operandStack.top();
            operandStack.pop();

            Token tokOp1 = operandStack.top();
            operandStack.pop();

            string operand1 = GetRealTokenName(tokOp1);
            string operand2 = GetRealTokenName(tokOp2);

            // В зависимости от операции будем генерировать шаблонный ассемблерный код.
            // Для присваиваний и сравнений не требуется рабочая переменная.
            if (tokenstring == "=")
            {
               if (operand2.data()[0] == 'r')
               {
                  mindex--;
               }
               code_str = AssignTemplate(operand1, operand2);
               break;
            }

            if (tokenstring == "==" || tokenstring == "!=" || tokenstring == "<")
            {
               lastLogicalOp = tokenstring;
               code_str = CompareTemplate(operand1, operand2);
               break;
            }

            // Для математическийх операций требуется рабочая переменная.
            Token workTok;
            string workVar;
            if (operand1.data()[0] == 'r' || operand2.data()[0] == 'r')
            {
               workTok.tableID = DynamicWorkingVar;

               if (operand2.data()[0] == 'r')
               {
                  workVar = operand2;
                  workTok.rowID = tokOp2.rowID;
               }
               else
               {
                  workVar = operand1;
                  workTok.rowID = tokOp1.rowID;
               }
            }
            else
            {
               workTok.tableID = DynamicWorkingVar;
               workTok.rowID = mindex;

               if (mindex > greaterWorkVar)
               {
                  greaterWorkVar = mindex;
                  workVariables.push("r" + to_string(greaterWorkVar));
               }
               workVar = "r" + to_string(mindex);
               mindex++;
            }

            if (tokenstring == "+")
            {
               code_str = AddTemplate(operand1, operand2, workVar);
               operandStack.push(workTok);
            }

            if (tokenstring == "-")
            {
               code_str = SubTemplate(operand1, operand2, workVar);
               operandStack.push(workTok);
            }

            if (tokenstring == "*")
            {
               code_str = MulTemplate(operand1, operand2, workVar);
               operandStack.push(workTok);
            }
            break;
         }

         case DynamicMarkGo:
         {
            code_str = GetRealTokenName(token) + "\n\n";
         }
      }

      sstr << code_str;
   }

   //-------------------------------------------//
   //            Область данных + начало        //
   //-------------------------------------------//

   // Сикер в начало файла.
   fout.seekp(0);
   fout << ".386 \n"
        << ".MODEL FLAT, STDCALL \n\n"
        << "EXTERN  ExitProcess@4 : PROC  ; функция выхода из программы.\n"
        << ".DATA  ; Сегмент данных. \n\n";

   for (size_t i = 0; i <  tables.GetDVsize(); i++)
   {
      Token tok = { DynamicVariables, i };
      fout << GetRealTokenName(tok) << " DD ? \n";
   }

   for (size_t i = 0; i <= greaterWorkVar; i++)
   {
      Token tok = { DynamicVariables, i };
      fout << "r" << i << " DD ? \n";
   }
   fout << '\n';

   sstr.seekp(0);
   string buf;
   while (!sstr.eof())
   {
      getline(sstr, buf, '\0');
      fout << buf;
   }

   //-------------------------------------------//
   //            Вывод конца кода               //
   //-------------------------------------------//

   fout << "PUSH 0                 ; Параметр: код выхода. \n"
        << "CALL ExitProcess@4 \n"
        << "MAIN ENDP              ; Завершение описания процедуры с именем MAIN. \n"
        << "END MAIN               ; Завершение описания модуля с указанием первой выполняемой процедуры. \n";
}