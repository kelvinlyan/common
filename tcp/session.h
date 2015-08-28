#ifndef _SESSION_H
#define _SESSION_H

#include "i_poll_events.h"
#include "poller.h"
#include <sys/types.h>
#include <string>
#include <vector>

class session
	: public i_poll_events
{
	public:
		session();
		virtual ~session(){}

		void set_fd(int fd);

		int syn_connect(const char* addr);
		ssize_t syn_send(const void* buff, size_t size);
		ssize_t syn_recv(void* buff, size_t size);
		int close();
		
		void async_send(const void* buff, size_t size);
		void async_recv(void* buff, size_t size);
		
		virtual void in_event();
		virtual void out_event();
		virtual void timer_event(int id){}

	private:
		int _fd;

		void* _buff;
		size_t _size;
		
		int _free_buff_index;
		std::string _send_buffs[2];

	//	uint32_t _pollout_count;
	//	mutex _pollout_count_mutex;

		poller* _poller;
};

#endif
