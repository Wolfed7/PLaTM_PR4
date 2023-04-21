#pragma once
#include <vector>
#include <iostream>

using namespace std;

class StaticTable
{
private:

   vector<string> Table;

public:

   // � �������� ��������� ��������� ���� � ���������� ��������.
   StaticTable(string FilePath);

   // ���������� ����� ���������� � ������� ��������,
   // � ������ ������� ���������� -1. 
   int BinarySearch(string Name);

   string GetName(int addr);

   // ���������� ������ �������.
   size_t Lenth();
};

