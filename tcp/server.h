#ifndef _SERVER_H
#define _SERVER_H

class session;

class server
{
	public:
		server();
		int bind(const char* addr);
		int syn_accept(session* s);
		void async_accept(session* s)
		{}

	protected:
		virtual void handle_async_accept(int error)
		{}

	private:
		int _fd;
};

#endif
