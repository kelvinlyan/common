#pragma once

#include <mqueue.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <string>
#include <stdlib.h>
#include "unistd_header.h"
using namespace std;

/*
struct mq_attr
{
	long mq_flags;           // 0 or O_NONBLOCK
	long mq_maxmsg;         // max number of messages allowed on queue
	long mq_msgsize;         // max size of a message (in bytes)
	long mq_curmsgs;         // number of messages currently on queue
};
*/

class mqueue
{
	public:
		typedef void(*handler)(void*);

		mqueue(const char* name)
			: _name(name), _buff(NULL), _size(0){}

		bool open(int maxmsg = 0, int msgsize = 0)
		{
			struct mq_attr attr;
			attr.mq_flags = 0;
			attr.mq_maxmsg = maxmsg;
			attr.mq_msgsize = msgsize;
			attr.mq_curmsgs = 0;
			if((_mqd = mq_open(_name.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR, (maxmsg == 0 && msgsize == 0)? NULL : &attr)) == -1)
			{
				fprintf(stderr, "open error: %s\n", strerror(errno));
				return false;
			}
			if(getattr(&attr))
			{
				_size = attr.mq_msgsize;
				_buff = (char*)malloc(_size);
			}
			return true;
		}

		bool close()
		{
			if(mq_close(_mqd) == -1)
			{
				fprintf(stderr, "close error: %s\n", strerror(errno));
				return false;
			}
			return true;
		}

		bool unlink()
		{
			if(mq_unlink(_name.c_str()) == -1)
			{
				fprintf(stderr, "unlink error: %s\n", strerror(errno));
				return false;
			}
			return true;
		}

		bool getattr(struct mq_attr* attr)
		{
			if(mq_getattr(_mqd, attr) == -1)
			{
				fprintf(stderr, "getattr error: %s\n", strerror(errno));
				return false;
			}
			printf("********************************\n");
			printf("MQ_FLAGS: %d\n", attr->mq_flags);
			printf("MQ_MAXMSG: %d\n", attr->mq_maxmsg);
			printf("MQ_MSGSIZE: %d\n", attr->mq_msgsize);
			printf("MQ_CURMSGS: %d\n", attr->mq_curmsgs);
			printf("********************************\n");
			return true;
		}

		bool printattr()
		{
			struct mq_attr attr;
			return getattr(&attr);
		}
		
		bool setblock(bool flag = true)
		{
			struct mq_attr attr;
			attr.mq_flags = flag? 0 : O_NONBLOCK;
			if(mq_setattr(_mqd, &attr, NULL) == -1)
			{
				fprintf(stderr, "setblock error: %s\n", strerror(errno));
				return false;
			}
			return true;
		}

		bool send(const char* pstr, size_t len, unsigned int prio = 0)
		{
			if(mq_send(_mqd, pstr, len, prio) == -1)
			{
				fprintf(stderr, "send error: %s\n", strerror(errno));
				return false;
			}
			printf("send(%d): %s\n", len, pstr);
			return true;
		}

		const char* receive(unsigned int* prio = NULL)
		{
			int len;
			if((len = mq_receive(_mqd, _buff, _size, prio)) == -1)
			{
				fprintf(stderr, "receive error: %s\n", strerror(errno));
				return NULL;
			}
			printf("receive(%d): %s\n", len, _buff);
			return _buff;
		}

		void notify(nUnistd::sig_handler h)
		{
			nUnistd::signal(SIGUSR1, h, NULL);
			_sigev.sigev_notify = SIGEV_SIGNAL;
			_sigev.sigev_signo = SIGUSR1;
			mq_notify(_mqd, &_sigev);
		}

	private:
		string _name;
		mqd_t _mqd;
		struct sigevent _sigev;
		char* _buff;
		int _size;
};
