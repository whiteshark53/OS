#include <iostream>
#include <Windows.h>
#include "..\common.h"

using namespace std;
using std::cin;

void get_request_from_console(request& req)
{
	char c;
	cout << "0 чтение, 1 модификаця, 2 выход" << endl;
	cin >> c;
	if (c == '2')
		ExitThread(0);
	cout << "Введите id студента" << endl;
	cin >> req.id;
	req.act = static_cast<action>(c - '0');
}

int main()
{
	setlocale(LC_ALL, "Rus");
	const char* pipe_name = R"(\\.\pipe\ClientPipe)";

	request req{};
	employee student{};
	HANDLE pipe{};
	while (true)
	{
		get_request_from_console(req);
		cout << "id " << req.id << " act " << req.act << endl;
		pipe = CreateFileA(pipe_name, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
		ASSERT_CONSOLE(pipe != INVALID_HANDLE_VALUE && pipe != nullptr, "Open named pipe");
		DWORD dwMode = PIPE_READMODE_MESSAGE;
		ASSERT_CONSOLE(SetNamedPipeHandleState(pipe, &dwMode, nullptr, nullptr),"Set pipe type message");
		ASSERT_CONSOLE(WriteFile(pipe, &req, sizeof req, nullptr, nullptr),"Write request to pipe");
		ASSERT_CONSOLE(ReadFile(pipe, &student, sizeof student, nullptr, nullptr),"Read employee from pipe");
		print_employee_to_console(student);
		if (req.act == action::modify)
		{
			read_employee_from_console(student);
			ASSERT_CONSOLE(WriteFile(pipe, &student, sizeof student, nullptr, nullptr),"Write new employee to pype");
		}
		CloseHandle(pipe);
	}
	cin.get();
	return 0;
}