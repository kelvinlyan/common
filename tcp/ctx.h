#ifndef _CTX_H
#define _CTX_H

#include "../base/singleton.h"
#include "poller.h"


class ctx
	: public singleton<ctx>
{
	public:
		poller* get_poller();


	protected:
		ctx();
		~ctx();


	private:
		SINGLETON_HELPER(ctx);

		poller* _poller;
};

#endif
