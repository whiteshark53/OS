#include <iostream>
#include <string>
#include <fstream>
#include <Windows.h>
#include <tchar.h>
#include <vector>

using namespace std;

struct employee
{
	int num;
	char name[10];
	double hours;
};
vector<employee> read_bin_file(const string& filename, const int count_of_lines)
{
	vector<employee> buf;
	ifstream input(filename, ios_base::binary);
	for (int i = 0; i < count_of_lines; i++) {
		employee emp{};
		input.read(reinterpret_cast<char*>(&emp.num), sizeof emp.num);
		input.read(reinterpret_cast<char*>(&emp.name), sizeof emp.name);
		input.read(reinterpret_cast<char*>(&emp.hours), sizeof emp.hours);

		buf.push_back(emp);
	}
	return buf;
}
LPWSTR convert_string(const std::string& instr)
{
	// Assumes std::string is encoded in the current Windows ANSI codepage
	const int buffer_len = MultiByteToWideChar(CP_ACP, 0, instr.c_str(), static_cast<int>(instr.size()), nullptr, 0);

	if (buffer_len == 0)
	{
		// Something went wrong. Perhaps, check GetLastError() and log.
		cerr << GetLastError();
		return nullptr;
	}

	// Allocate new LPWSTR - must deallocate it later
	const auto wide_str = new WCHAR[buffer_len + 1];

	MultiByteToWideChar(CP_ACP, 0, instr.c_str(), static_cast<int>(instr.size()), wide_str, buffer_len);

	// Ensure wide string is null terminated
	wide_str[buffer_len] = 0;

	// Do something with widestr
	return wide_str;
	//delete[] widestr;
}
int main(int argc,char** argv)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof si);
	si.cb = sizeof si;
	ZeroMemory(&pi, sizeof pi);
	setlocale(LC_ALL, "rus");
	cout << "Введите имя бинарного файла" << endl;
	string bin_file_name;
	cin >> bin_file_name;
	cout << "Введите количество записей в бинарном файле" << endl;
	string count_of_blocks;
	cin >> count_of_blocks;

	string command_line = bin_file_name + " " + count_of_blocks;
	LPCTSTR cmd_prog = _tcsdup(TEXT("Util_1_4.exe"));
	wstring wstr(command_line.begin(), command_line.end());
	LPWSTR cmdArgs = convert_string(command_line);

	if (CreateProcessW(cmd_prog, cmdArgs, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi))
		WaitForSingleObject(pi.hProcess, INFINITE);
	else cout << "Process Util_1 crushed";
	
	WaitForSingleObject(pi.hProcess, INFINITE);
	vector<employee> buf = read_bin_file(bin_file_name, atoi(count_of_blocks.c_str()));
	for (auto& i : buf)
		cout << to_string(i.num) + " " + static_cast<string>(i.name) + " " + to_string(i.hours) << endl;
	
	string report_name;
	cout << "Введите название файла отчета" << endl;
	cin >> report_name;
	double price_per_hour;
	cout << "Введите зп за час" << endl;
	cin >> price_per_hour;
	cmd_prog = _tcsdup(TEXT("Util_2.exe"));
	command_line = bin_file_name + " " + report_name + " " + to_string(price_per_hour);
	wstring wstr1(command_line.begin(), command_line.end());
	cmdArgs = convert_string(command_line);
	STARTUPINFO si1;
	PROCESS_INFORMATION pi1;
	ZeroMemory(&si1, sizeof si1);
	si1.cb = sizeof si1;
	ZeroMemory(&pi1, sizeof pi1);
	if (CreateProcessW(cmd_prog, cmdArgs, nullptr, nullptr, false, 0, nullptr, nullptr, &si1, &pi1))
		WaitForSingleObject(pi1.hProcess, INFINITE);
	else cout << "Process Util_2 crushed";
	
	WaitForSingleObject(pi1.hProcess, INFINITE);
	cout << endl << "Success!" << endl;
	system("pause");
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	CloseHandle(pi1.hThread);
	CloseHandle(pi1.hProcess);

	return 0;
}