#ifndef _SERVER_H
#define _SERVER_H

class session;
class async_handler;

class server
{
	public:
		server();
		int bind(const char* addr);
		int syn_accept(session* s);
		void async_accept(session* s)
		{}
		void set_async_handler(async_handler* h);

	protected:
		virtual void handle_async_accept(int error)
		{}

	private:
		int _fd;

		async_handler* _async_handler;
};

#endif
