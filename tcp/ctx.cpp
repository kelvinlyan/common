#include "ctx.h"

ctx::ctx()
{
	_poller = new poller();
	_poller->start();
}

ctx::~ctx()
{
	delete _poller;
}

poller* ctx::get_poller()
{
	return _poller;
}
