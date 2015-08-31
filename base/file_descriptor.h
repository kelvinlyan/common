#ifndef _FILE_DESCRIPTER_H
#define _FILE_DESCRIPTER_H

#include "macro.h"
#include <fcntl.h>

class file_descripter
{
	public:
		static int set_nonblock(int fd)
		{
			int flags = fcntl(fd, F_GETFL, 0);
			int rc = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
			errno_assert(rc != -1);
			return rc;
		}

		static int set_block(int fd)
		{
			int flags = fcntl(fd, F_GETFL, 0);
			int rc = fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
			errno_assert(rc != -1);
			return rc;
		}
};

typedef file_descripter FD;

#endif
