#ifndef _MESSAGE_H
#define _MESSAGE_H

#include <list>

struct message
{
	void* _data;
	size_t _size;
};

typedef std::list<message*> msgQueue;

#endif
