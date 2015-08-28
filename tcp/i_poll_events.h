#ifndef _I_POLL_EVENTS_H
#define _I_POLL_EVENTS_H

class i_poll_events
{
	public:
		virtual ~i_poll_events(){}
		virtual void in_event() = 0;
		virtual void out_event() = 0;
		virtual void timer_event(int id) = 0;
};

#endif
