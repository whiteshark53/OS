#include <iostream>
#include <string>

#include "../blocking_circular_file_buffer.h"
#include "../sync.h"


using namespace std;

class sender final : public blocking_circular_file_buffer
{
public:
	explicit sender(fstream& file) :
		blocking_circular_file_buffer(
			file,
			new named_semaphore(open_only, data_event_name),
			new named_semaphore(open_only, space_event_name),
			new named_mutex(open_only, file_mutex_name))
	{}
};

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "Russian");

	if (argc != 3)
		return -1;

	fstream file(argv[1], ios::in | ios::out | ios::binary | ios::ate);
	const sender sen(file);

	named_semaphore ready(open_only, ready_semaphore_name);
	named_semaphore start(open_only, start_semaphore_name);
	ready.post();
	start.wait();
	message m;
	fill_n(m, sizeof m, 0);
	cout << "Введите exit для выхода" << endl;
	while (true)
	{
		cin.getline(m, sizeof m);
		if (string(m) == "exit")
			break;
		sen.push(m);
		cout << "Отправлено " << m << endl;
	}

	return 0;
}