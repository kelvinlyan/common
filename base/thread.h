#ifndef _THREAD_H
#define _THREAD_H

#include <pthread.h>
#include "noncopyable.h"
#include "macro.h"

typedef void*(thread_fn)(void*);

class thread 
	: public noncopyable
{
	public:
		virtual ~thread(){}

		void start(thread_fn* tfn, void* arg)
		{
			int rc = pthread_create(&_tid, NULL, tfn, arg);
			posix_assert(rc);
		}
		
		void stop()
		{
			int rc = pthread_join(_tid, NULL);
			posix_assert(rc);
		}

		friend void* thread_routine(void* arg);

	private:
		pthread_t _tid;
};

#endif
