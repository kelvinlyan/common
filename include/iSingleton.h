#ifndef _COMMON_ISINGLETON_
#define _COMMON_ISINGLETON_

class iSingleton
{
public:
    virtual void destoryInstance() = 0;
	const char* get();
};

#endif 
