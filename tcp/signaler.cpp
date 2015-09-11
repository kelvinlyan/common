#include "signal_t.h"

signaler::signaler()
{
	int fd[2];
	int rc = socketpair(AF_UNIX, SOCK_STREAM, 0, fd);
	errno_assert(rc != -1);
	_w = fd[0];
	_r = fd[1];
	FD::set_nonblock(_w);
	FD::set_nonblock(_r);
}

signaler::~signaler()
{
	int rc = close(_w);
	errno_assert(rc == 0);
	rc = close(_r);
	errno_assert(rc == 0);
}

void signaler::send()
{
	unsigned char dummy = 0;
	while(true)
	{
		ssize_t nbytes = ::send(_w, &dummy, sizeof(dummy), 0);
		if(nbytes == -1 && errno == EINTR)
			continue;

		errno_assert(nbytes == sizeof(dummy));
		break;
	}
}

void signaler::recv()
{
	unsigned char dummy;
	ssize_t nbytes = ::recv(_r, &dummy, sizeof(dummy), 0);
	errno_assert(nbytes == sizeof(dummy));
}

int signaler::get_fd()
{
	return _r;
}






