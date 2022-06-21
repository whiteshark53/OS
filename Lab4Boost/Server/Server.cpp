#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>

#include <boost/process/child.hpp>
#include <boost/process/search_path.hpp>
#include <boost/process/args.hpp>
#include <boost/process/group.hpp>
#include <boost/process/extend.hpp>

#include "../blocking_circular_file_buffer.h"
#include "../sync.h"

using std::cin;
using std::cout;
using std::endl;
using std::fstream;
using std::string;
using std::ios;
using std::to_string;
using std::error_code;
using boost::process::child;
using boost::process::search_path;
using boost::process::args;
using boost::process::group;
using boost::process::extend::handler;
using boost::process::extend::windows_executor;

struct new_console : handler
{
	template <typename Sequence>
	void on_setup(windows_executor<wchar_t, Sequence>& ex)
	{
		ex.creation_flags |= CREATE_NEW_CONSOLE;
	}
};

class receiver final : public blocking_circular_file_buffer
{
public:

	receiver(fstream& file, const int n_records) :
		blocking_circular_file_buffer(
			file,
			new named_semaphore(open_or_create, data_event_name, 0),
			new named_semaphore(open_or_create, space_event_name, n_records),
			new named_mutex(open_or_create, file_mutex_name))
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
	receiver::clear();
	const receiver rec(file, n_records);


	group clients_group;
	child* clients = new child[count_of_sender];

	named_semaphore::remove(ready_semaphore_name);
	named_semaphore::remove(start_semaphore_name);
	named_semaphore ready(open_or_create, ready_semaphore_name, 0);
	named_semaphore start(open_or_create, start_semaphore_name, 0);
	for (int i = 0; i < count_of_sender; i++)
	{
		constexpr auto app_name = "Client.exe";
		error_code ec;
		clients[i] = child(search_path(app_name), new_console(), args({ file_name ,to_string(i) }), ec);
		clients_group.add(clients[i]);
	}
	for (int i = 0; i < count_of_sender; i++)
		ready.wait();
	for (int i = 0; i < count_of_sender; i++)
		start.post();

	char input;
	cout << "Введите любой символ для чтения сообщений из файла (0-завершение работы потока)" << endl;
	while (true)
	{
		cin >> input;
		if (input == '0')
			break;

		cout << "Принято " << rec.pop() << endl;
	}

	clients_group.wait();
	return 0;
}
