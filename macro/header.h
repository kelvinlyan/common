#ifndef _MACRO_HEADER_H
#define _MACRO_HEADER_H

#define FOREACH(TYPE, ITER, VALUE) \
	for(TYPE::iterator ITER = VALUE.begin(); ITER != VALUE.end(); ++ITER) \

#define FOREACHC(TYPE, ITER, VALUE) \
	for(TYPE::const_iterator ITER = VALUE.begin(); ITER != VALUE.end(); ++ITER) \

#endif
