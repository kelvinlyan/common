#ifndef _COMMON_SINGLETON_H
#define _COMMON_SINGLETON_H

#include "noncopyable.h"
#include "mutex.h"

template <typename T>
class singleton 
	: public noncopyable
{
	public:
    	static T& shared()
    	{
        	if(_pInstance == NULL)
        	{
				_mutex.lock();
				if(_pInstance == NULL)
				{
            		_pInstance = new T();
				}
				_mutex.unlock();
        	}
        	return *_pInstance;
    	}
    
    	void destroy()
    	{
        	if(_pInstance)
        	{
				_mutex.lock();
				if(_pInstance)
				{
					delete _pInstance;
            		_pInstance = NULL;
				}
				_mutex.unlock();
        	}
    	}

	protected:
		singleton(){}
		virtual ~singleton(){}

	private:
    	static T* _pInstance;
		static mutex _mutex;
};

template<typename T>
T* singleton<T>::_pInstance = NULL;

template<typename T>
mutex singleton<T>::_mutex;

#define SINGLETON_HELPER(T)\
	friend class singleton<T>

#endif 
