#include <iostream>
#include <string>
#include<fstream>
#include <Windows.h>
#include <tchar.h>
#include<vector>

using namespace std;

struct to_thread
{
	vector<int> mas;
	int numbers[5]{};
};

LPWSTR convert_string(const std::string& instr)
{
	const int buffer_len = MultiByteToWideChar(CP_ACP, 0, instr.c_str(), static_cast<int>(instr.size()), nullptr, 0);
	if (buffer_len == 0)
		return nullptr;
	const auto wide_str = new WCHAR[buffer_len + 1];
	MultiByteToWideChar(CP_ACP, 0, instr.c_str(), static_cast<int>(instr.size()), wide_str, buffer_len);
	wide_str[buffer_len] = 0;
	return wide_str;
}
DWORD WINAPI average(LPVOID lpParam)
{
	const auto buf = static_cast<to_thread*>(lpParam);
	const vector<int> mas = buf->mas;
	int sum = 0;
	for (const int ma : mas)
	{
		sum += ma;
		Sleep(12);
	}
	buf->numbers[0] = sum / mas.size();
	return 0;
}
DWORD WINAPI min_max(LPVOID lpParam)
{
	const auto buf = static_cast<to_thread*>(lpParam);
	const vector<int> mas = buf->mas;
	int min = mas[0];
	int max = mas[0];
	for (int i = 1; i < mas.size(); i++)
	{
		if (mas[i] > max)
		{
			Sleep(7);
			max = mas[i];
		}
		if (mas[i] < min)
		{
			Sleep(7);
			min = mas[i];
		}
	}
	buf->numbers[0] = min;
	buf->numbers[1] = max;
	return 0;
}

int main()
{
	setlocale(LC_ALL, "rus");
	cout << "Введите размер массива ";
	int size;
	cin >> size;
	vector<int> mas;
	for (int i = 0; i < size; i++) {
		int buf;
		cout << "Введите число " + to_string(i + 1) << endl;
		cin >> buf;
		mas.push_back(buf);
	}
	const auto to_min_max_thr = new to_thread();
	to_min_max_thr->mas = mas;
	const auto to_average_thr = new to_thread();
	to_average_thr->mas = mas;
	DWORD dw_thread_id, dw_thread_id2;
	HANDLE h_thread1 = CreateThread(
		nullptr,         // атрибуты безопасности по умолчанию
		0,            // размер стека используется по умолчанию
		min_max,   // функция потока
		to_min_max_thr, // аргумент функции потока
		0,            // флажки создания используются по умолчанию
		&dw_thread_id);
	HANDLE h_thread2 = CreateThread(
		nullptr,         // атрибуты безопасности по умолчанию
		0,            // размер стека используется по умолчанию
		average,   // функция потока
		to_average_thr, // аргумент функции потока
		0,            // флажки создания используются по умолчанию
		&dw_thread_id2);

	if (h_thread1 == nullptr || h_thread2 == nullptr)
	{
		cerr << GetLastError();
		return -1;
	}

	WaitForSingleObject(h_thread1, INFINITE);
	WaitForSingleObject(h_thread2, INFINITE);
	cout << endl << "min= " + to_string(to_min_max_thr->numbers[0]) + " max= " + to_string(to_min_max_thr->numbers[1]) << endl;
	cout << "average= " + to_string(to_average_thr->numbers[0]) << endl << endl << "mas:" << endl;
	for (int& ma : mas)
	{
		if (ma == to_min_max_thr->numbers[0] || ma == to_min_max_thr->numbers[1]) {
			ma = to_average_thr->numbers[0];
		}
		cout << to_string(ma) + " ";
	}
	system("pause");
	return 0;
}