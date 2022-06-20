#include <iostream>
#include <fstream>
#include <string>
#include <Windows.h>
#include "../blocking_circular_file_buffer.h"

using namespace std;

class receiver final : public blocking_circular_file_buffer
{
	SECURITY_ATTRIBUTES sa_ = { sizeof(SECURITY_ATTRIBUTES), nullptr, true };

public:

	receiver(fstream& file, const int n_records) :
		blocking_circular_file_buffer(
			file,
			CreateSemaphoreA(&sa_, 0, n_records, data_semaphore_name),
			CreateSemaphoreA(&sa_, n_records, n_records, space_semaphore_name),
			CreateMutexA(&sa_, false, file_mutex_name))
	{

		for (unsigned int i = 0; i < header_offset + n_records * sizeof(message); i++)
			file_.write("", 1);
		set_n_records(n_records);
		set_write_count(0);
		set_read_count(0);
	}
};


int main()
{
	setlocale(LC_ALL, "Russian");

	cout << "Название бинарного файла" << endl;
	string file_name;
	cin >> file_name;

	cout << "Введите максимаьное число записей в файле" << endl;
	int n_records;
	cin >> n_records;

	cout << "Введите количество процессов Sender" << endl;
	int count_of_sender;
	cin >> count_of_sender;

	fstream file(file_name, ios::in | ios::out | ios::binary | ios::trunc);
	const receiver rec(file, n_records);

	const auto senders_si = new STARTUPINFOA[count_of_sender];
	const auto senders_pi = new PROCESS_INFORMATION[count_of_sender];
	SECURITY_ATTRIBUTES security_attributes = { sizeof(SECURITY_ATTRIBUTES), nullptr, true };

	HANDLE event_to_start = CreateEventA(&security_attributes, true, true, "SenderStartEvent");
	if (!event_to_start)
		return -1;

	const auto sender_ready_event = new HANDLE[count_of_sender];
	for (int i = 0; i < count_of_sender; i++)
	{
		constexpr auto app_name = "sender.exe";
		ZeroMemory(&senders_si[i], sizeof(STARTUPINFO));
		senders_si[i].cb = sizeof(STARTUPINFO);
		ZeroMemory(&senders_pi[i], sizeof(PROCESS_INFORMATION));

		sender_ready_event[i] = CreateEventA(&security_attributes, false, false, ("SenderReady" + to_string(i)).c_str());
		if (!sender_ready_event[i])
			return -1;
		string arguments = file_name + " " + to_string(i);
		if (!CreateProcessA(app_name, const_cast<char*>(arguments.c_str()), nullptr, nullptr, true, CREATE_NEW_CONSOLE, nullptr, nullptr, &senders_si[i], &senders_pi[i]))
			return -1;
	}
	WaitForMultipleObjects(count_of_sender, sender_ready_event, true, INFINITE);

	char input;
	cout << "Введите любой символ для чтения сообщений из файла (0-завершение работы потока)" << endl;
	while (true)
	{
		cin >> input;
		if (input == '0')
			break;

		cout << "Принято " << rec.pop() << endl;
	}

	CloseHandle(event_to_start);
	for (int i = 0; i < count_of_sender; i++)
	{
		CloseHandle(senders_pi[i].hThread);
		CloseHandle(senders_pi[i].hProcess);
		CloseHandle(sender_ready_event[i]);
	}
	return 0;
}
