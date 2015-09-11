#ifndef _SERVER_H
#define _SERVER_H

#include "i_poll_events.h"
#include "poller.h"
#include "async_fn.h"

class session;

class server
	: public i_poll_events
{
	public:
		server();
		int bind(const char* addr);
		int syn_accept(session* s);
		void async_accept(session* s, async_fn fn);

		virtual void in_event();
		virtual void out_event();
		virtual void timer_event(int id);

	private:
		int _fd;

		session* _async_session;
		async_fn _async_fn;

		poller* _poller;
};

#endif
