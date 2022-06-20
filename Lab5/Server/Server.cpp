#include <iostream>
#include <Windows.h>
#include "..\common.h"

using namespace std;

HANDLE data_file;
HANDLE* locks;
int students_count;

DWORD WINAPI request_handler(LPVOID arg)
{
	HANDLE* pipe = reinterpret_cast<HANDLE*>(arg);
	request req{};
	ASSERT_CONSOLE(ReadFile(pipe, &req, sizeof req, nullptr, nullptr), "Pipe read request");
	employee student;
	switch (req.act)
	{
	case action::read:
		WaitForSingleObject(locks[req.id], INFINITE);
		ReleaseMutex(locks[req.id]);
		SetFilePointer(data_file, sizeof(student) * req.id, 0, FILE_BEGIN);
		ASSERT_CONSOLE(ReadFile(data_file, &student, sizeof student, nullptr, nullptr), "Action::read, read data_file");
		ASSERT_CONSOLE(WriteFile(pipe, &student, sizeof student, nullptr, nullptr), "Action::read, write to pipe");
		break;
	case action::modify:
		WaitForSingleObject(locks[req.id],INFINITE);
		// читаем и отправляем запись клиенту
		SetFilePointer(data_file, sizeof(student) * req.id, 0, FILE_BEGIN);
		ASSERT_CONSOLE(ReadFile(data_file, &student, sizeof student, nullptr, nullptr), "Action::modify,read data_file");
		ASSERT_CONSOLE(WriteFile(pipe, &student, sizeof student, nullptr, nullptr), "Action::modify,write to pipe");
		// получаем новую запись и записываем на диск
		ASSERT_CONSOLE(ReadFile(pipe, &student, sizeof student, nullptr, nullptr), "Action::modify,read new from pipe");
		SetFilePointer(data_file, sizeof(student) * req.id, 0, FILE_BEGIN);
		ASSERT_CONSOLE(WriteFile(data_file, &student, sizeof student, nullptr, nullptr), "Action::modify,write new to data_file");
		ReleaseMutex(locks[req.id]);
		break;
	}
	DisconnectNamedPipe(pipe);
	CloseHandle(pipe);
	return 0;
}

DWORD WINAPI connection_handler(LPVOID arg)
{
	UNREFERENCED_PARAMETER(arg);
	const char* pipe_name = R"(\\.\pipe\ClientPipe)";
	while (true)
	{
		HANDLE pipe = CreateNamedPipeA(pipe_name, PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, sizeof(employee), sizeof(employee), NMPWAIT_USE_DEFAULT_WAIT, nullptr);
		ASSERT_CONSOLE(pipe != INVALID_HANDLE_VALUE && pipe != nullptr, "Open named pipe");
		if (!ConnectNamedPipe(pipe, nullptr))
		{
			cout << GetLastError() << endl;

			return -1;
		}
		CreateThread(nullptr, 0, request_handler, pipe, 0, nullptr);
	}
	return 0;
}


void print_data_file_to_console()
{
	employee student{};
	cout << "****\t\tФайл данных\t\t****" << endl;
	SetFilePointer(data_file, 0, 0, FILE_BEGIN);
	for (int i = 0; i < students_count; i++)
	{
		if (!ReadFile(data_file, &student, sizeof student, nullptr, nullptr))
		{
			cout << "Failed to show all students with error " << GetLastError();
			ExitThread(-1);
		}
		print_employee_to_console(student);
	}
	cout << "*************************" << endl;
}


int main()
{
	setlocale(LC_ALL, "Russian");
	cout << "Введите имя файла" << endl;
	char* filename = new char[256];
	cin >> filename;
	data_file = CreateFileA(filename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	delete[] filename;

	cout << "Введите количество студентов" << endl;
	cin >> students_count;

	locks = new HANDLE[students_count];
	employee student{};
	for (int i = 0; i < students_count; i++)
	{
		locks[i] = CreateMutexA(nullptr, false, nullptr);
		ASSERT_CONSOLE(locks[i] != INVALID_HANDLE_VALUE && locks[i] != nullptr, "Create mutex");
		student.id = i;
		read_employee_from_console(student);
		WriteFile(data_file, &student, sizeof student, nullptr, nullptr);
	}
	print_data_file_to_console();
	cout << "Введите количество процессов client" << endl;
	int clients_count;
	cin >> clients_count;

	auto clients_si = new STARTUPINFOA[clients_count];
	auto clients_pi = new PROCESS_INFORMATION[clients_count];
	for (int i = 0; i < clients_count; i++)
	{
		ZeroMemory(&clients_si[i], sizeof(STARTUPINFO));
		clients_si[i].cb = sizeof(STARTUPINFO);
		ZeroMemory(&clients_pi[i], sizeof(PROCESS_INFORMATION));
		CreateProcessA("Client.exe", nullptr, nullptr, nullptr, true, CREATE_NEW_CONSOLE, nullptr, nullptr, &clients_si[i], &clients_pi[i]);
	}
	HANDLE connection_hanlder_thread = CreateThread(nullptr, 0, connection_handler, nullptr, 0, nullptr);
	ASSERT_CONSOLE(connection_hanlder_thread != INVALID_HANDLE_VALUE && connection_hanlder_thread != nullptr, "Connection thread");

	char с; // заглушка
	cout << "Для завершения ввелите любой символ" << endl;
	cin >> с;

	print_data_file_to_console();
	for (int i = 0; i < clients_count; i++)
	{
		CloseHandle(clients_pi[i].hProcess);
		CloseHandle(clients_pi[i].hThread);
	}
	for (int i = 0; i < students_count; i++)
		CloseHandle(locks[i]);
	CloseHandle(data_file);
	delete[] locks;
	return 0;
}