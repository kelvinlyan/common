#ifndef _COMMON_ITIME_H
#define _COMMON_ITIME_H

#include <sys/time.h>
#include <stdio.h>
#include <string>

using namespace std;

namespace iTime
{
	static inline string str()
	{
		char buff[32];
		time_t now = time(NULL);
		struct tm t = {0};
		localtime_r(&now, &t);
		sprintf(buff, "%04d-%02d-%02d %02d:%02d:%02d", 
				t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
		return string(buff);
	}

	class timer
	{
		public:
			timer();
			long usec();
			double sec();
		private:	
			struct timeval _t;
	};
}

#endif
