#pragma once
#include <vector>
#include <iostream>

#include "Variable.h"

using namespace std;

class DynamicTable
{
public:
   vector<Variable> Table;
   DynamicTable() { Table = {}; };

   // ���������� ����� ���������� � ������� ��������,
   // � ������ ������� ���������� -1. 
   int LinearSearch(string Name);
};

