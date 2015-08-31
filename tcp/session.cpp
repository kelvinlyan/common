#include "session.h"
#include "ctx.h"
#include "../base/macro.h"
#include "../base/file_descriptor.h"
#include <sys/socket.h>
#include <string.h>
#include <string>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>

session::session()
{
	_fd = -1;
	_free_buff_index = 0;
	_connected = false;
	_poller = ctx::shared().get_poller();
}

void session::set_async_handler(async_handler* h)
{
	_async_handler = h;
}

void session::set_fd(int fd)
{
	_fd = fd;
	_connected = true;
}

int session::connect(const char* addr, bool async)
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

	if(async)
	{
		FD::set_nonblock(_fd);
		::connect(_fd, (struct sockaddr*)&sock_addr, addr_len);
		_poller->add_fd(_fd, this);
		_poller->set_pollout(_fd);
	}
	else
	{
		rc = ::connect(_fd, (struct sockaddr*)&sock_addr, addr_len);
		errno_assert(rc == 0);
		_connected = true;
		_poller->add_fd(_fd, this);
	}

	return rc;
}


int session::syn_connect(const char* addr)
{
	return connect(addr, false);
}

int session::async_connect(const char* addr)
{
	return connect(addr, true);
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
	_send_buffs[_free_buff_index].insert(_send_buffs[_free_buff_index].end()
		, (const char*)buff, (const char*)buff + size);
	_poller->set_pollout(_fd);
}

void session::async_recv(void* buff, size_t size)
{
	_buff = buff;
	_size = size;
	_poller->set_pollin(_fd);
}

void session::in_event()
{
	ssize_t size = syn_recv(_buff, _size);
	printf("in: (%d)", size);
	for(int i = 0; i < size; ++i)
		printf("%c", ((char*)_buff)[i]);
	printf("\n");
	_poller->reset_pollin(_fd);
}

void session::out_event()
{
	if(!_connected)
	{
		_connected = true;
		FD::set_block(_fd);
		_poller->reset_pollout(_fd);
		printf("connected\n");
		return;
	}

	int working_buff_index = _free_buff_index;
	_free_buff_index = _free_buff_index == 0? 1 : 0;
	syn_send(_send_buffs[working_buff_index].c_str(), _send_buffs[working_buff_index].size());
	printf("out: (%d)", _send_buffs[working_buff_index].size());
	for(int i = 0; i < _send_buffs[working_buff_index].size(); ++i)
		printf("%c", _send_buffs[working_buff_index][i]);
	printf("\n");
	_send_buffs[working_buff_index].clear();
	_poller->reset_pollout(_fd);
}


