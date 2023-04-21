#include "parser.h"
#include "tables.h"
#include "token.h"
#include <cstdlib>
#include <vector>

using namespace std;

const Token UPLtok = { DynamicLogic, 0 };
const Token BPtok = { DynamicLogic, 1 };

// ���� ���������� ��� ���������� �������,
// ���������� ��� ���� ������. ׸ - �� �� ��������.

// -------------------------------//
//           ������� �������.     //
// -------------------------------//

ParseTable::ParseTable(string filename) 
{
   ifstream file(filename);
   string line;

   getline(file, line);

   // ��������� �������� �� ����� ���������.
   while (getline(file, line))
   {
      stringstream lineStream(line);

      vector<string> words;
      string word;

      while (getline(lineStream, word, '\t'))
         words.push_back(word);

      ParseTableRow row;

      stringstream terminalStream(words[1]);
      string terminal;

      while (getline(terminalStream, terminal, ' '))
         row.terminals_.push_back(terminal);

      row.jump_ = stoi(words[2]);
      row.accept_ = stoi(words[3]);
      row.stack_ = stoi(words[4]);
      row.return_ = stoi(words[5]);
      row.error_ = stoi(words[6]);

      data.push_back(row);
   }
}

ParseTableRow &ParseTable::at(size_t idx)
{
   return data[idx - 1];
}

void ParseTable::push(ParseTableRow row) 
{
   data.push_back(row);
}

void ParseTable::print() 
{
   for (auto &row : data) 
   {

      for (auto &word : row.terminals_)
         cout << word << " ";

      cout << row.jump_ << " ";
      cout << row.accept_ << " ";
      cout << row.stack_ << " ";
      cout << row.return_ << " ";
      cout << row.error_ << endl;
   }
}

int GetTokenPriority(Token tok) 
{
   switch (tok.tableID)
   {
      case StaticOperators:
      {
         switch (tok.rowID)
         {
            case 5:
               return 1;

            case 0:
            case 4:
            case 6:
               return 6;

            case 2:
            case 3:
               return 7;

            case 1:
               return 8;

               break;
         }
      }

      case StaticSeparators:
      {
         switch (tok.rowID)
         {
            case 0:
            case 2:
               return 1;
         }
      }

      case StaticBrackets:
      {
         switch (tok.rowID)
         {
            case 0:
               return 0;

            case 1:
               return 1;
         }
      }

   }

   return 199;
}

// ------------------------------------------------//
//       �������������� ���������� (������)        //
// ------------------------------------------------//

Parser::Parser(ParseTable *table, Tables tables, vector<size_t> tokenLineIndeces)
{
   this->table = table;
   this->tables = tables;
   this->tokenLineIndeces = tokenLineIndeces;
}

// ������ ��� �� ������. ����������� var � const ��� ��������������� ������, ��� ��������� - ��������.
string Parser::GetTokenStr(Token token)
{
   string tokenStr;

   switch (token.tableID)
   {
   case StaticBrackets:
      tokenStr = tables.GetStaticLex(pair<int, int>({ StaticBrackets, token.rowID }));
      break;

   case StaticSpecials:
      tokenStr = tables.GetStaticLex(pair<int, int>({ StaticSpecials, token.rowID }));
      break;

   case StaticOperators:
      tokenStr = tables.GetStaticLex(pair<int, int>({ StaticOperators, token.rowID }));
      break;

   case StaticKeywords:
      tokenStr = tables.GetStaticLex(pair<int, int>({ StaticKeywords, token.rowID }));
      break;

   case StaticSeparators:
      tokenStr = tables.GetStaticLex(pair<int, int>({ StaticSeparators, token.rowID }));
      break;

   case DynamicVariables:
      tokenStr = "var";
      break;

   case DynamicConstants:
      tokenStr = "const";
      break;
   }

   return tokenStr;
}

// ��� ����� �� ������� ��������.
string Parser::GetRealTokenStr(Token token)
{
   string tokenStr;

   switch (token.tableID)
   {
      case StaticBrackets:
      case StaticSpecials:
      case StaticOperators:
      case StaticKeywords:
      case StaticSeparators:
         tokenStr = GetTokenStr(token);
         break;

      case DynamicVariables:
         tokenStr = tables.SearchOnDynamic(pair<int, int>({ DynamicVariables, token.rowID }))->Name;
         break;

      case DynamicConstants:
         tokenStr = tables.SearchOnDynamic(pair<int, int>({ DynamicConstants, token.rowID }))->Name;
         break;

      case DynamicMark:
         tokenStr = markTable[token.rowID];
         break;

      case DynamicMarkGo:
         tokenStr = markTable[token.rowID] + ":";
         break;

      case DynamicLogic:
      {
         if (token.rowID == 0)
         {
            tokenStr = "UPL";
         }
         if (token.rowID == 1)
         {
            tokenStr = "BP";
         }
         break;
      }
   }

   return tokenStr;
}

bool Parser::ContainsTerminal(vector<string> terminals, string terminal) 
{
   for (auto &t : terminals)
      if (t.compare(terminal) == 0)
         return true;

   return false;
}

vector<Token> Parser::GetPolish() 
{
   return polish;
}

void Parser::Parse(vector<Token> tokens)
{
   stack<size_t> st;
   stack<Token> workStack;
   size_t currentRow = 1;
   size_t currentTokenNumber = 0;
   size_t currentLine = 1;
   int tempTypeID = None;
   int declLine = 0;
   Variable *initVar = NULL;
   Token *maybeOperand = NULL;
   int markNum = 0;

   stack<Token> ifmarks;
   stack<Token> elsemarks;

   workStack.push({ 300, 300 });

   st.push(0);

   for (auto &token : tokens)
   {
      string tokenStr = GetTokenStr(token);

      while (tokenLineIndeces[currentLine] == currentTokenNumber)
         currentLine++;

      while (true)
      {
         ParseTableRow row = table->at(currentRow);
         size_t tempCurrentRow = currentRow;

         //cout << row.jump_ << " |\t" << currentRow << " &\t" << tokenStr;
         //getchar();
         //cout << '\n';

         if (row.stack_)
         {
            //cout << "- PUSH TO STACK " << currentRow << endl;
            st.push(currentRow);
         }

         if (row.return_)
         {
            if (st.top() == 0)
               return;

            //cout << " . RETURN TO " << st.top() << endl;
            currentRow = st.top() + 1;
            st.pop();
         }
         else if (row.jump_ != 0 && ContainsTerminal(row.terminals_, tokenStr))
         {
            //cout << " . JUMP TO " << row.jump_ << endl;
            currentRow = row.jump_;
         }
         else if (!row.error_)
         {
            currentRow += 1;
         }
         else
         {
            string msg = "����������� ������: ";
            msg += to_string(currentRow);
            ParserError err = { msg, (int)currentLine };

            switch (currentRow)
            {
            case 22:
               err.msg = "��������� �������������";
               break;

            case 26:
               err.msg = "��������� '=' ��� ';'";
               break;

            case 93:
               err.msg = "��������� ���������";
               break;

            case 9:
            case 95:
            case 102:
               err.msg = "��������� '{'";
               break;

            case 11:
            case 97:
            case 104:
               err.msg = "��������� '}'";
               break;

            case 24:
            case 74:
            case 41:
               err.msg = "��������� ';'";
               break;

            default:
               break;
            }

            PushError(err);
            return;
         }

         Variable *constant;
         Variable *var;

         if (row.accept_) 
         {  
            //--------------------------------//
            // �������� ������������� ������. //
            //--------------------------------//

            if (tempCurrentRow == 22)
            {
               declLine = currentLine;
            }

            // ���� �� �� 30 �� �������, �� ���������� ����� �������������������.
            // � ���� ������ �� ������ ������� �������������� ������.
            if (tempCurrentRow == 30 && initVar != NULL && initVar->Type != None)
            {
               initVar->IsInitialised = true;
            }

            switch (token.tableID) 
            {
            case DynamicConstants:
               constant = tables.SearchOnDynamic(pair<int, int>({ DynamicConstants, token.rowID }));
               break;

            case DynamicVariables:
               var = tables.SearchOnDynamic(pair<int, int>({ DynamicVariables, token.rowID }));

               if (tempCurrentRow == 25)
               {
                  initVar = var;
               }

               if (var->Type == None)
               {
                  if (tempCurrentRow == 25 || (tempCurrentRow == 75 && declLine == currentLine))
                  {
                     var->Type = Int;
                  }
                  else
                  {
                     ParserError err = { GetRealTokenStr(token) + ": ������������� �� ��������", (int)currentLine };
                     PushError(err);
                     return;
                  }
               }
               else if (var->Type == Int)
               {
                  if (tempCurrentRow == 25)
                  {
                     ParserError err = { GetRealTokenStr(token) + ": ���������������; ������������� �������������", (int)currentLine };
                     PushError(err);
                     return;
                  }
                  else if (tempCurrentRow == 75)
                  {
                     var->IsInitialised = true;
                  }
               }

               if ( var->IsInitialised == false && tempCurrentRow == 44 )
               {
                  ParserError err = { GetRealTokenStr(token) + ": ������������� �������������������� ����������", (int)currentLine };
                  PushError(err);
                  return;
               }

               if (tempCurrentRow == 75 && var->Type == None)
               {
                  ParserError err = { GetRealTokenStr(token) + ": ����������� ������������� �������������", (int)currentLine };
                  PushError(err);
                  return;
               }

               break;

            case StaticSeparators:
               break;

            // ������� �����.
            case StaticOperators:
               if (tempCurrentRow == 35)
               {
                  // �� ��������
                  // ����� ���� �� ���������� � �������� ����... 
               }
               break;
            }

            // --------------------------------
            //  �������� � �������� ������.
            // --------------------------------
            if (tempCurrentRow == 25)
            {
               maybeOperand = &token;
            }
            else if (tempCurrentRow == 30)
            {
               polish.push_back(*maybeOperand);
               maybeOperand = NULL;
            }

            // ������� � �������� ��������� ��� ��������� �����.
            // (����� ������� (���, ����� �������� �� ��), �������� �� ������ ���� ��� � �� �����)
            if (tempCurrentRow == 95)
            {
               markTable.push_back("m" + to_string(markNum));

               Token mark; 
               mark.tableID = DynamicMark;
               mark.rowID = markNum;
               polish.push_back(mark);
               polish.push_back(UPLtok);
               markNum++;

               ifmarks.push(mark);
            }
            if (tempCurrentRow == 97)
            {
               ifmarks.top().tableID = DynamicMarkGo;
               polish.push_back(ifmarks.top());
               ifmarks.pop();
            }
            if (tempCurrentRow == 101)
            {
               markTable.push_back("m" + to_string(markNum));
               Token mark;
               mark.tableID = DynamicMark;
               mark.rowID = markNum;
               Token lastMark = polish.back();
               polish[polish.size() - 1] = mark;
               polish.push_back(BPtok);
               polish.push_back(lastMark);
               markNum++;

               elsemarks.push(mark);
            }
            if (tempCurrentRow == 104)
            {
               elsemarks.top().tableID = DynamicMarkGo;
               polish.push_back(elsemarks.top());
               elsemarks.pop();
            }

            if 
               (  
                  token.tableID == DynamicConstants
               || token.tableID == DynamicVariables
               || token.tableID == StaticSeparators
               || token.tableID == StaticOperators
               || (token.tableID == StaticBrackets && currentRow > 9 && (token.rowID == 0 || token.rowID == 1))
               ) 
            {
   

               switch (token.tableID)
               {
               case DynamicConstants:
                  polish.push_back(token);
                  break;

               case DynamicVariables:
               {
                  if (tempCurrentRow == 25)
                  {
                     break;
                  }
                  else
                  {
                     polish.push_back(token);
                  }
                  break;
               }


               case StaticSeparators:
                  while (workStack.top().tableID != 300)
                  {
                     polish.push_back(workStack.top());
                     workStack.pop();
                  }

                  //polish.push_back(token); // ��� ���������.
                  break;

               case StaticBrackets:
                  /* 
                     ����������� ������� ������, ������� �������
                     ��������� ����, ������ ������������ � �������
                     ����� � �� ����������� �� ������ �����.
                  */
                  if (GetTokenPriority(token) == 0)
                  {
                     workStack.push(token);
                  }
                  /*
                     ��������� ����������� ������ �������� ������������ ���� ������ �� ��������� ����������� ������
                     ������������. � ���� ����������� ������ �� ������������. ����������� � ����������� ������ ��� ��
                     ������� ������������ � � �������� ������ �� �����������.
                  */
                  else if (GetTokenPriority(token) == 1)
                  {
                     while (!(workStack.top().tableID != 300 && workStack.top().tableID == StaticBrackets && workStack.top().rowID == 0))
                     {
                        polish.push_back(workStack.top());
                        workStack.pop();
                     }

                     workStack.pop(); // ���� ������ '('
                  }
                  break;


               case StaticOperators:
                  Token last = workStack.top();

                  //if (token.rowID == 4)
                  //{ // ��������
                  //   int a = 1;
                  //}

                  if (last.tableID == 300)
                  {
                     workStack.push(token);
                  }
                  else //if (last.tableID == StaticOperators)
                  {
                     /*
                       ���� ��������� �������� ����� ����� ���� 
                       ��� ������ ���������� �����, ������������ �
                       ������� �����, �� ����� ���� �����������
                       � ������� �����.
                     */
                     if (GetTokenPriority(token) == 0 || GetTokenPriority(token) > GetTokenPriority(workStack.top()))
                     {
                        workStack.push(token);
                     }
                     /*
                        � ��������� ������ �� ����� ��������������� � ��������������
                        � �������� ������ ����, ����������� � �������,
                        � ����� ��������� �� ��� ����� � ������������,
                        �������� ��� ������� ���������� �������� �����.
                        ����� ����� ������� ���� ����������� � ������� �����.
                     */
                     else
                     {
                        while 
                           (
                              workStack.top().tableID != 300
                              && GetTokenPriority(token) <= GetTokenPriority(workStack.top())
                           )
                        {
                           polish.push_back(workStack.top());
                           workStack.pop();
                        }
                        workStack.push(token);
                     }
                  }
                  // ?
                  //else 
                  //{
                  //   workStack.push(token);
                  //}

                  break;
               }
            }
            break;
         }
      }
      currentTokenNumber++;
      //cout << ">|\t" << tokenStr << endl;
   }

   if (!st.empty())
   {
      ParserError err = { "��������� '}'", (int)currentLine };
      PushError(err);
      return;
   }

   //cout << endl;
}

void Parser::PushError(ParserError error)
{
   errors.push_back(error);
}

vector<ParserError> Parser::GetErrors()
{
   return errors;
}