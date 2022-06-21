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

int blocking_circular_file_buffer::size() const
{
	const int max_records = get_n_records();
	const int read_count = get_read_count();
	const int write_count = get_write_count();
	return read_count < write_count ? write_count - read_count : write_count + read_count - max_records;
}

void blocking_circular_file_buffer::clear()
{
	named_semaphore::remove(data_event_name);
	named_semaphore::remove(space_event_name);
	named_mutex::remove(file_mutex_name);
}

int blocking_circular_file_buffer::get_n_records() const
{
	int n_records = 0;
	file_.sync();
	file_.seekg(n_records_offset, std::ios::beg);
	file_.read(reinterpret_cast<char*>(&n_records), sizeof n_records);
	return n_records;
}

blocking_circular_file_buffer::blocking_circular_file_buffer(std::fstream& file,
	named_semaphore* data_available,
	named_semaphore* space_available,
	named_mutex* file_mutex) :
	file_(file),
	data_available_(data_available),
	space_available_(space_available),
	file_mutex_(file_mutex)
{}

std::string blocking_circular_file_buffer::pop() const
{
	static message m;
	data_available_->wait();
	{
		scoped_lock<named_mutex> lock(*file_mutex_);
		const int read_count = get_read_count();
		file_.seekg(header_offset + read_count * sizeof(message), std::ios::beg);
		file_.read(m, sizeof(message));
		set_read_count((read_count + 1) % get_n_records());
	}
	space_available_->post();
	return m;
}

void blocking_circular_file_buffer::push(message m) const
{
	space_available_->wait();
	{
		scoped_lock<named_mutex> lock(*file_mutex_);
		const int write_count = get_write_count();
		file_.seekg(header_offset + write_count * sizeof(message), std::ios::beg);
		file_.write(m, sizeof(message));
		file_.flush();
		set_write_count((write_count + 1) % get_n_records());
	}
	data_available_->post();
}

blocking_circular_file_buffer::~blocking_circular_file_buffer()
{
	delete file_mutex_;
	delete data_available_;
	delete space_available_;
}
