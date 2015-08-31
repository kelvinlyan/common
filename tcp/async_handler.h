#ifndef _ASYNC_HANDLER_H
#define _ASYNC_HANDLER_H

#include "../base/macro.h"

class async_handler
{
	public:
		virtual ~async_handler(){}
		virtual void handle_async_accept(int error)
		{
			base_assert(false);	
		}
		virtual void handle_async_connect(int error)
		{
			base_assert(false);	
		}
		virtual void handle_async_send(int error)
		{
			base_assert(false);	
		}
		virtual void handle_async_recv(int error)
		{
			base_assert(false);	
		}
};

#endif
