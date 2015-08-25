#ifndef _MACRO_H
#define _MACRO_H

#include <string.h>
#include <errno.h>
#include <stdio.h>

#define FOREACH(TYPE, ITER, VALUE) \
	for(TYPE::iterator ITER = VALUE.begin(); ITER != VALUE.end(); ++ITER) 

#define FOREACHC(TYPE, ITER, VALUE) \
	for(TYPE::const_iterator ITER = VALUE.begin(); ITER != VALUE.end(); ++ITER) 

#define errno_assert(x)\
    do{\
        if(!(x)){\
            const char* errstr = strerror(errno);\
            fprintf(stderr, "%s (%s:%d)\n", errstr, __FILE__, __LINE__);\
        }\
    }while(false)

#define posix_assert(x)\
    do{\
        if(x){\
            const char* errstr = strerror(x);\
            fprintf(stderr, "%s (%s:%d)\n", errstr, __FILE__, __LINE__);\
        }\
    }while(false)

#define base_assert(x)\
    do{\
        if(!(x)){\
            fprintf(stderr, "(%s) is false (%s:%d)\n", #x, __FILE__, __LINE__);\
        }\
    }while(false)

enum
{
	KB = 1024,
	MB = KB * 1024,
	GB = MB * 1024,
	//TB = GB * 1024
	
	MINUTE = 60,
	HOUR = 60 * MINUTE,
	DAY = 24 * HOUR,
};

#endif
