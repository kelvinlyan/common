#include "session.h"
#include "../base/macro.h"
#include <sys/socket.h>
#include <string.h>
#include <string>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>

session::session()
{
	_fd = -1;
	_poller = new poller();
	_poller->start();
}

void session::set_fd(int fd)
{
	_fd = fd;
}

int session::syn_connect(const char* addr)
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

	rc = ::connect(_fd, (struct sockaddr*)&sock_addr, addr_len);
	errno_assert(rc == 0);

	return rc;
}

ssize_t session::syn_send(const void* buff, size_t size)
{
	return ::send(_fd, buff, size, 0);
}

ssize_t session::syn_recv(void* buff, size_t size)
{
	return ::recv(_fd, buff, size, 0);
}

int session::close()
{
	if(_fd != -1)
	{
		int rc = ::close(_fd);
		errno_assert(rc == 0);
		_fd = -1;
	}
	return 0;
}

void session::async_send(const void* buff, size_t size)
{
	_send_buff.assign((const char*)buff, size);
	_poller->add_fd(_fd, this);
	_poller->set_pollout(_fd);
}

void session::async_recv(void* buff, size_t size)
{
	_buff = buff;
	_size = size;
	_poller->add_fd(_fd, this);
	_poller->set_pollin(_fd);
}

void session::in_event()
{
	::recv(_fd, _buff, _size, 0);
	printf("%s\n", _buff);
	_poller->reset_pollin(_fd);
}

void session::out_event()
{
	::send(_fd, _send_buff.c_str(), _send_buff.size(), 0);
	printf("%s\n", _send_buff.c_str());
	_poller->reset_pollout(_fd);
}


