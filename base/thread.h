#ifndef _THREAD_H
#define _THREAD_H

#include <pthread.h>
#include "noncopyable.h"

typedef void(thread_fn)(void*);

void* thread_routine(void* arg);

class thread 
	: public noncopyable
{
	public:
		virtual ~thread(){}

		void start(thread_fn* tfn, void* arg)
		{
			_tfn = tfn;
			_arg = arg;
			int rc = pthread_create(&_tid, NULL, thread_routine, this);
			posix_assert(rc);
		}
		
		void stop()
		{
			int rc = pthread_join(_tid, NULL);
			posix_assert(rc);
		}

		friend void* thread_routine(void* arg);

	private:
		thread_fn* _tfn;
		void* _arg;

		pthread_t _tid;
};

void* thread_routine(void* arg)
{
	thread* self = (thread*)arg;
	self->_tfn(self->_arg);
	return NULL;
}

#endif
