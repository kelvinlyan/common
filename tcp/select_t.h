#ifndef _SELECT_T_H
#define _SELECT_T_H

#include "../base/thread.h"
#include "i_poll_events.h"
#include <vector>
#include <sys/select.h>

const int retired_fd = -1;

class select_t
{
	public:
		select_t();
		void add_fd(int fd, i_poll_events* events);
		void rm_fd(int fd);

		void set_pollin(int fd);
		void reset_pollin(int fd);
		void set_pollout(int fd);
		void reset_pollout(int fd);

		void start();
		void stop();

	private:
		static void* worker_routine(void* arg);
		void loop();

	private:
		struct fd_entry
		{
			int _fd;
			i_poll_events* _events;
		};

		static bool is_retired_fd(const fd_entry& entry);
		
		typedef std::vector<fd_entry> fd_entry_set;
		fd_entry_set _fds;

		fd_set _read_fds;
		fd_set _write_fds;
		fd_set _except_fds;

		fd_set _source_in;
		fd_set _source_out;
		fd_set _source_err;

		int _max_fd;
		
		bool _running;
		bool _retired;

		thread _worker;
};

#endif
