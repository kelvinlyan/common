#ifndef _ASYNC_HANDLER_H
#define _ASYNC_HANDLER_H

class async_handler
{
	public:
		virtual ~async_handler(){}
		virtual void in_event();
		virtual void out_event();
};

#endif
