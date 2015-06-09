#ifndef _TIMER_H
#define _TIMER_H

#include <boost/function.hpp>

class timer
{
	public:
		typedef boost::function<void()> Handler;
		static int push(unsigned time, Handler h);
		static void pop(int id);
		static void run();
	
	private:
		

};


#endif
