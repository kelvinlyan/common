#ifndef _COMMON_SINGLETON_
#define _COMMON_SINGLETON_

#include <iostream>
#include <pthread.h>
#include "iSingleton.h"

template <typename T>
class Singleton : public iSingleton
{
protected:
    Singleton()
    {
    }
    virtual ~Singleton()
    {
    }
    
public:
    static T* shared()
    {
        if(pInstance == NULL)
        {
			pthread_mutex_lock(&mutex);
			if(pInstance == NULL)
			{
            	pInstance = new T();
     //       	pInstance->initInstance();
			}
			pthread_mutex_unlock(&mutex);
        }
        return pInstance;
    }
    
    virtual void destoryInstance()
    {
        if(pInstance)
        {
       //     pInstance->finiInstance();
            pInstance = NULL;
        }
    }
    
    virtual void initInstance(){}
    virtual void finiInstance()
    {
        delete pInstance;
    }
    
private:
    static T* pInstance;
	static pthread_mutex_t mutex;
};

template<typename T>
T* Singleton<T>::pInstance = NULL;

template<typename T>
pthread_mutex_t Singleton<T>::mutex = PTHREAD_MUTEX_INITIALIZER;

#endif 
