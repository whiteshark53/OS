#pragma once
#include <iostream>

#define ASSERT_CONSOLE(cond,msg) if(!(cond)) \
{\
	cout << (msg) << " " << GetLastError() << endl;\
	return -1;\
}

enum action
{
	read = 0,
	modify = 1
};

struct request
{
	action act;
	int id;
};

class employee
{
public:
	static constexpr int invalid_id = -1;
	int id = invalid_id;
	char name[10]{};
	double hours = 0;
};

void read_employee_from_console(employee& student)
{
	std::cout << "������� ��� ��������" << std::endl;
	std::cin >> student.name;
	std::cout << "������� ���� ��������" << std::endl;
	std::cin >> student.hours;
}

void print_employee_to_console(employee& student)
{
	std::cout << "������� " << "id " << student.id << " ��� " << student.name << " ���� " << student.hours << std::endl;
}