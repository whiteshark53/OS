#include "blocking_circular_file_buffer.h"

void blocking_circular_file_buffer::set_read_count(const int read_count) const
{
	file_.sync();
	file_.seekg(read_count_offset, std::ios::beg);
	file_.write(reinterpret_cast<const char*>(&read_count), sizeof read_count);
	file_.flush();
}

void blocking_circular_file_buffer::set_write_count(const int write_count) const
{
	file_.sync();
	file_.seekg(write_count_offset, std::ios::beg);
	file_.write(reinterpret_cast<const char*>(&write_count), sizeof write_count);
	file_.flush();
}

void blocking_circular_file_buffer::set_n_records(const int n_records) const
{
	file_.sync();
	file_.seekg(n_records_offset, std::ios::beg);
	file_.write(reinterpret_cast<const char*>(&n_records), sizeof n_records);
	file_.flush();
}

int blocking_circular_file_buffer::get_read_count() const
{
	int read_count = 0;
	file_.sync();
	file_.seekg(read_count_offset, std::ios::beg);
	file_.read(reinterpret_cast<char*>(&read_count), sizeof read_count);
	return read_count;
}

int blocking_circular_file_buffer::get_write_count() const
{
	int write_count = 0;
	file_.sync();
	file_.seekg(write_count_offset, std::ios::beg);
	file_.read(reinterpret_cast<char*>(&write_count), sizeof write_count);
	return write_count;
}

int blocking_circular_file_buffer::get_n_records() const
{
	int n_records = 0;
	file_.sync();
	file_.seekg(n_records_offset, std::ios::beg);
	file_.read(reinterpret_cast<char*>(&n_records), sizeof n_records);
	return n_records;
}

blocking_circular_file_buffer::blocking_circular_file_buffer(std::fstream& file, HANDLE data_available,
                                                             HANDLE space_available, HANDLE file_mutex) :
	file_(file),
	data_available_(data_available),
	space_available_(space_available),
	file_mutex_(file_mutex)
{}

std::string blocking_circular_file_buffer::pop() const
{
	WaitForSingleObject(data_available_, INFINITE);
	WaitForSingleObject(file_mutex_, INFINITE);

	static message m;
	const int read_count = get_read_count();
	file_.seekg(header_offset + read_count * sizeof(message), std::ios::beg);
	file_.read(m, sizeof(message));
	set_read_count((read_count + 1) % get_n_records());

	ReleaseMutex(file_mutex_);
	ReleaseSemaphore(space_available_, 1, nullptr);
	return m;
}

void blocking_circular_file_buffer::push(message m) const
{
	WaitForSingleObject(space_available_, INFINITE);
	WaitForSingleObject(file_mutex_, INFINITE);

	const int write_count = get_write_count();
	file_.seekg(header_offset + write_count * sizeof(message), std::ios::beg);
	file_.write(m, sizeof(message));
	file_.flush();
	set_write_count((write_count + 1) % get_n_records());

	ReleaseMutex(file_mutex_);
	ReleaseSemaphore(data_available_, 1, nullptr);
}

blocking_circular_file_buffer::~blocking_circular_file_buffer()
{
	CloseHandle(data_available_);
	CloseHandle(space_available_);
	CloseHandle(file_mutex_);
}
