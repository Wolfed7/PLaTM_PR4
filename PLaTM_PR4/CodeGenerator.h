#pragma once

#include <string>

#include "Token.h"
#include "Tables.h"

using namespace std;

class CodeGenerator
{
   Tables tables;
   vector<Token> polish;
   stack<string> workVariables;

public:
   CodeGenerator(Tables tables, vector<Token> polish);
   string GetRealTokenName(Token token);
   void Generate(string filename);
};