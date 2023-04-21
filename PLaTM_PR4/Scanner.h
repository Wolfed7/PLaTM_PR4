#pragma once
#include <vector>
#include <string>
#include <iostream>

#include "tables.h"
#include "token.h"

struct ScannerError 
{
   string msg;
   int line;
};

class Scanner;

class ScannerState 
{
public:
   virtual ~ScannerState() { }
   virtual void update(Scanner &scanner, char lexeme) { }
};

class Scanner 
{
public:
   Scanner(Tables tables);

   // ��������� �������� �������.
   // �������� - ����� �������� �����.
   void StartScan(string code);

   // ����� ��������� ��������� �������� �������.
   void ChangeStateTo(ScannerState *newState);

   // ��������� ������ � ������������������,
   // ������������ �������.
   void PushToBuffer(char lexeme);

   // ��������� � ������ ����� ������, 
   // ������������ ��� ������������.
   void PushError(string errorMsg);

   // ���������� ������, � �������
   // �������� ������ � ������ ������.
   int GetLine();

   // ������� ����� �� �������.
   void ClearBuffer();

   // ������ ����� ��������� ����.
   // TODO: !!! ��� ������������ ��, � �� ��������,
   // ����� ���������������� ���� ������ 
   // TokenType � TableType ???
   void CreateToken(TokenType type);

   // ���������� vector<char> buf � ���� string.
   // TODO: ����� ������ ��� ������������� � ��������� �� ������� ����.
   string GetBufferAsString();
   vector<ScannerError> GetErrors();
   vector<Token> GetTokens();
   vector<size_t> GetTokenLineIndexes();

   bool multilineComment = false;

private:
   int currentLine = 1;
   int currentTokenID = 0;
   ScannerState *state_;
   Tables tables_;
   vector<char> buf;
   vector<Token> tokens;
   vector<ScannerError> errors;
   vector<size_t> tokenLineIndexes;
};

class ScannerStateStart : public ScannerState 
{
public:
   ScannerStateStart() { }
   virtual void update(Scanner &scanner, char lexeme);
};

class ScannerStateWord : public ScannerState 
{
public:
   ScannerStateWord() { }
   virtual void update(Scanner &scanner, char lexeme);
};

class ScannerStateNumber : public ScannerState 
{
public:
   ScannerStateNumber() { }
   virtual void update(Scanner &scanner, char lexeme);
};

class ScannerStateOperation : public ScannerState 
{
public:
   ScannerStateOperation() { }
   virtual void update(Scanner &scanner, char lexeme);
};

class ScannerStateCommentOrOperation : public ScannerState 
{
public:
   ScannerStateCommentOrOperation() { }
   virtual void update(Scanner &scanner, char lexeme);
};

class ScannerStateSinglelineComment : public ScannerState 
{
public:
   ScannerStateSinglelineComment() { }
   virtual void update(Scanner &scanner, char lexeme);
};

class ScannerStateMultilineComment : public ScannerState 
{
public:
   ScannerStateMultilineComment() { }
   virtual void update(Scanner &scanner, char lexeme);
};

class ScannerStateFinishMultilineComment : public ScannerState 
{
public:
   ScannerStateFinishMultilineComment() { }
   virtual void update(Scanner &scanner, char lexeme);
};