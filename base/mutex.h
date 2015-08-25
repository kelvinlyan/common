#ifndef _MUTEX_H
#define _MUTEX_H

#include <pthread.h>
#include "noncopyable.h"
#include "macro.h"

class mutex
	: public noncopyable
{
	public:
		mutex()
		{
			int rc = pthread_mutex_init(&_mutex, NULL);
			posix_assert(rc);
		}
		virtual ~mutex()
		{
			int rc = pthread_mutex_destroy(&_mutex);
			posix_assert(rc);
		}
		inline void lock()
		{
			int rc = pthread_mutex_lock(&_mutex);
			posix_assert(rc);
		}
		inline bool trylock()
		{
			int rc = pthread_mutex_trylock(&_mutex);
			if(rc == EBUSY)
				return false;
			posix_assert(rc);
			return true;
		}
		inline void unlock()
		{
			int rc = pthread_mutex_unlock(&_mutex);
			posix_assert(rc);
		}

	private:
		pthread_mutex_t _mutex;
};

class scoped_lock
	: public noncopyable
{
	public:
		scoped_lock(mutex& m)
			: _mutex(m)
		{
			_mutex.lock();
		}
		virtual ~scoped_lock()
		{
			_mutex.unlock();
		}
	private:
		mutex& _mutex;
};

#endif
