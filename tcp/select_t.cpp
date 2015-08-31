#include "select_t.h"
#include <algorithm>

select_t::select_t()
{
	_retired = false;
	_running = false;
	_max_fd = retired_fd;
}

void select_t::start()
{
	_running = true;
	_worker.start(select_t::worker_routine, this);
}

void select_t::stop()
{
	_running = false;
	_worker.stop();
}

void select_t::add_fd(int fd, i_poll_events* events)
{
	fd_entry entry = { fd, events };
	_fds.push_back(entry);
	FD_SET(fd, &_source_err);
	if(fd > _max_fd)
		_max_fd = fd;
}

void select_t::rm_fd(int fd)
{
	fd_entry_set::iterator iter;
	for(iter = _fds.begin(); iter != _fds.end(); ++iter)
	{
		if(iter->_fd == fd)
			break;
	}
	if(iter == _fds.end())
		return;

	FD_CLR(fd, &_source_in);
	FD_CLR(fd, &_source_out);
	FD_CLR(fd, &_source_err);

	FD_CLR(fd, &_read_fds);
	FD_CLR(fd, &_write_fds);
	FD_CLR(fd, &_except_fds);

	_retired = true;

	if(fd == _max_fd)
	{
		_max_fd = retired_fd;
		for(fd_entry_set::iterator iter = _fds.begin(); iter != _fds.end(); ++iter)
		{
			if(iter->_fd > _max_fd)
				_max_fd = iter->_fd;
		}
	}
}

void select_t::set_pollin(int fd)
{
    FD_SET(fd, &_source_in);
}

void select_t::reset_pollin(int fd)
{
    FD_CLR(fd, &_source_in);
}

void select_t::set_pollout(int fd)
{
    FD_SET(fd, &_source_out);
}

void select_t::reset_pollout(int fd)
{
    FD_CLR(fd, &_source_out);
}

void* select_t::worker_routine(void* arg)
{
	((select_t*)arg)->loop();
	return NULL;
}

bool select_t::is_retired_fd(const fd_entry& entry)
{
	return entry._fd == retired_fd;
}

void select_t::loop()
{
	while(_running)
	{
		memcpy(&_read_fds, &_source_in, sizeof(_source_in));
		memcpy(&_write_fds, &_source_out, sizeof(_source_out));
		memcpy(&_except_fds, &_source_err, sizeof(_source_err));

		int timeout = 1000;

        struct timeval tv = {(long)(timeout / 1000),
            (long)(timeout % 1000 * 1000)};

		int rc = ::select(_max_fd + 1, &_read_fds, &_write_fds, &_except_fds, &tv);

		if(rc == -1)
		{
			if(errno == EINTR)
				continue;
		}
		
		if(rc == 0)
			continue;
		
		for(fd_entry_set::size_type i = 0; i < _fds.size(); ++i)
		{
			if(FD_ISSET(_fds[i]._fd, &_except_fds))	
				_fds[i]._events->in_event();
			if(_fds[i]._fd == retired_fd)
				continue;
			if(FD_ISSET(_fds[i]._fd, &_write_fds))	
				_fds[i]._events->out_event();
			if(_fds[i]._fd == retired_fd)
				continue;
			if(FD_ISSET(_fds[i]._fd, &_read_fds))
				_fds[i]._events->in_event();
		}

		if(_retired)
		{
			_fds.erase(std::remove_if(_fds.begin(), _fds.end(), select_t::is_retired_fd)
				, _fds.end());
			_retired = false;
		}
	}
}
