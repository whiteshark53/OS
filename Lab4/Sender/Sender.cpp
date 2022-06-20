#include <iostream>
#include <string>
#include "../blocking_circular_file_buffer.h"

using namespace std;

class sender final : public blocking_circular_file_buffer
{
public:
	explicit sender(fstream& file) :
		blocking_circular_file_buffer(
			file,
			OpenSemaphoreA(SEMAPHORE_MODIFY_STATE | SYNCHRONIZE, false, data_semaphore_name),
			OpenSemaphoreA(SEMAPHORE_MODIFY_STATE | SYNCHRONIZE, false, space_semaphore_name),
			OpenMutexA(MUTEX_MODIFY_STATE | SYNCHRONIZE, false, file_mutex_name))
	{}
};

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "Russian");
	if (argc != 2)
		return -1;

	HANDLE sender_ready_event = OpenEventA(EVENT_MODIFY_STATE | SYNCHRONIZE, false, ("SenderReady" + string(argv[1])).c_str());
	HANDLE start_event = OpenEventA(SYNCHRONIZE, false, "SenderStartEvent");

	if (!sender_ready_event || !start_event)
		return -1;

	fstream file(argv[0], ios::in | ios::out | ios::binary | ios::ate);
	const sender sen(file);

	SignalObjectAndWait(sender_ready_event, start_event, INFINITE, FALSE);

	constexpr auto buff_len = 20;
	message m;
	ZeroMemory(m, buff_len);
	while (true)
	{
		cin.getline(m, buff_len);
		if (string(m) == "exit")
			break;
		sen.push(m);
		cout << "Отправлено " << m << endl;
	}
	CloseHandle(sender_ready_event);
	CloseHandle(start_event);

	return 0;
}