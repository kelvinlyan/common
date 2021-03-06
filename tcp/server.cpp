#include "server.h"
#include "session.h"
#include "ctx.h"
#include "../base/macro.h"
#include <sys/socket.h>
#include <string.h>
#include <string>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>

server::server()
{
	_poller = ctx::shared().get_poller();
}

int server::bind(const char* addr)
{
	_fd = socket(AF_INET, SOCK_STREAM, 0);
	errno_assert(_fd != -1);

    const char* delimiter = strrchr(addr, ':');
    if(!delimiter)
		return -1;
	
	std::string addr_str(addr, delimiter - addr);
	std::string port_str(delimiter + 1);

	uint16_t port;
	if(port_str == "*")
		port = 0;
	else
	{
		port = (uint16_t)atoi(port_str.c_str());
		if(port == 0)
			return -1;
	}

	int rc = -1;

	struct sockaddr_in sock_addr;
	memset(&sock_addr, 0, sizeof(sock_addr));
	sock_addr.sin_family = AF_INET;
	rc = inet_pton(AF_INET, addr_str.c_str(), &sock_addr.sin_addr);
	if(rc != 1)
		return -1;

	sock_addr.sin_port = htons(port);

	socklen_t addr_len = sizeof(sock_addr);
	rc = ::bind(_fd, (struct sockaddr*)&sock_addr, addr_len);
	errno_assert(rc == 0);

	rc = ::listen(_fd, 5);
	errno_assert(rc == 0);

	_poller->add_fd(_fd, this);

	return rc;
}	

int server::syn_accept(session* s)
{
	struct sockaddr_in sock_addr;
	socklen_t addr_len = sizeof(sock_addr);
	int cli_fd = ::accept(_fd, (struct sockaddr*)&sock_addr, &addr_len);
	s->set_fd(cli_fd);
	errno_assert(cli_fd != -1);	
	return 0;
}

void server::async_accept(session* s, async_fn* fn, void* arg)
{
	_async_session = s;
	_async_fn = fn;
	_arg = arg;

	_poller->set_pollin(_fd);
}

void server::in_event()
{
	_poller->set_pollout(_fd);
	int rc = syn_accept(_async_session);
	if(!_async_fn.empty())
	{
		_async_fn(rc);
		_async_fn.reset();
	}
}

void server::out_event()
{
	base_assert(false);
}

void server::timer_event(int id)
{
	base_assert(false);
}

