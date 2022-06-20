#define  _CRT_NON_CONFORMING_WCSTOK
#define STRLEN(x) (sizeof(x)/sizeof(TCHAR) - 1)
#include <windows.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <fcntl.h>
#include <algorithm>
using namespace std;
struct employee
{
	int num; // идентификационный номер сотрудника
	char name[10]; // имя сотрудника
	double hours; // количество отработанных часов
};

vector<employee> read_bin_file(const wstring& filename)
{
	vector<employee> buf;
	employee emp{};
	ifstream input(filename, ios_base::binary);
	while (input.read(reinterpret_cast<char*>(&emp.num), sizeof emp.num))
	{
		input.read(reinterpret_cast<char*>(&emp.name), sizeof emp.name);
		input.read(reinterpret_cast<char*>(&emp.hours), sizeof emp.hours);
		buf.push_back(emp);
	}
	return buf;
}
bool comp(const employee& a, const employee& b)
{
	return a.num < b.num;
}
void write_report(const wstring& path, const vector<employee>& vec, const double price_per_hour, wstring bin_name)
{
	ofstream out;
	out.open(path);
	sort(vec.begin(), vec.end(), comp);
	const string buf(bin_name.begin(), bin_name.end());
	out << " Отчет по файлу " + buf + "\n";
	for (const auto& i : vec)
		out << to_string(i.num) + " " + i.name + " " + to_string(i.hours) + " " + to_string(i.hours * price_per_hour) + "\n";

	out.close();
}
int WINAPI WinMain(HINSTANCE hThisInst, HINSTANCE hPrevInst, LPSTR lpszArgs, int nWinMode)
{
	setlocale(LC_ALL, "rus");

	const wstring str = GetCommandLineW();
	const auto s = new TCHAR[str.length() + 1];
	wcscpy(s, str.c_str());
	const wchar_t* ptr = wcstok(s, L" ");
	const wstring bin_name = ptr;
	ptr = wcstok(nullptr, L" ");
	const wstring report_name = ptr;
	ptr = wcstok(nullptr, L" ");
	const int price_per_hour = stoi(ptr);
	const vector<employee> emps = read_bin_file(bin_name);
	write_report(report_name, emps, price_per_hour, bin_name);
	return 0;
}