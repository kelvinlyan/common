#include "iTime.h"

namespace iTime
{
	timer::timer()
	{
		gettimeofday(&_t, NULL);
	}
	long timer::usec()
	{
		struct timeval _end_t;
		gettimeofday(&_end_t, NULL);
		long diff = _end_t.tv_sec * 1000000 + _end_t.tv_usec - _t.tv_sec * 1000000 - _t.tv_usec;
		_t = _end_t;
		return diff;
	}
	double timer::sec()
	{
		struct timeval _end_t;
		gettimeofday(&_end_t, NULL);
		double diff = _end_t.tv_sec - _t.tv_sec + (_end_t.tv_usec - _t.tv_usec) / 1000000.0;
		_t = _end_t;
		return diff;
	}
}
