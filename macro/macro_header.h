#ifndef _MACRO_MACRO_HEADER_H
#define _MACRO_MACRO_HEADER_H

#define FOREACH(TYPE, ITER, VALUE) \
	for(TYPE::iterator ITER = VALUE.begin(); ITER != VALUE.end(); ++ITER) \

#define FOREACHC(TYPE, ITER, VALUE) \
	for(TYPE::const_iterator ITER = VALUE.begin(); ITER != VALUE.end(); ++ITER) \

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
