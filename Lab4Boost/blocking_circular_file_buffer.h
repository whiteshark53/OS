#pragma once
#include <fstream>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/named_semaphore.hpp>

using boost::interprocess::named_mutex;
using boost::interprocess::named_semaphore;

using boost::interprocess::scoped_lock;
using boost::interprocess::open_or_create;
using boost::interprocess::open_only;

typedef char message[20];

//двоичный файл, первые две записи счётчики чтения и записи
//дальше место для записи сообщений
//[n_records][read_count][write_count][message...]
class blocking_circular_file_buffer
{
protected:
	static constexpr const char* data_event_name = "DataEvent";
	static constexpr const char* space_event_name = "SpaceEvent";
	static constexpr const char* file_mutex_name = "FileMutex";
	static constexpr int header_offset = 3 * sizeof(int);
	static constexpr int n_records_offset = 0;
	static constexpr int read_count_offset = sizeof(int);
	static constexpr int write_count_offset = 2 * sizeof(int);
	std::fstream& file_;

	void set_read_count(int read_count) const;
	void set_write_count(int write_count) const;
	void set_n_records(int n_records) const;
	int get_n_records() const;


private:

	named_semaphore* data_available_;
	named_semaphore* space_available_;
	named_mutex* file_mutex_;

	int get_read_count() const;
	int get_write_count() const;
	int size() const;

public:

	static void clear();

	blocking_circular_file_buffer(const blocking_circular_file_buffer& b) = delete;
	blocking_circular_file_buffer(blocking_circular_file_buffer&& b) = delete;
	blocking_circular_file_buffer& operator=(blocking_circular_file_buffer b) = delete;
	blocking_circular_file_buffer& operator=(blocking_circular_file_buffer&& b) = delete;

	blocking_circular_file_buffer(
		std::fstream& file,
		named_semaphore* data_available,
		named_semaphore* space_available,
		named_mutex* file_mutex);

	std::string pop() const;

	void push(message m) const;

	virtual ~blocking_circular_file_buffer();
};
