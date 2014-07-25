#ifndef _COMMON_TIMER_H
#define _COMMON_TIMER_H

#include <sys/time.h>
#include <stdio.h>

class timer
{
    public:
        timer()
        {
            gettimeofday(&_begin_time, NULL);
        }
        long get_usec()
        {
            struct timeval _end_time;
            gettimeofday(&_end_time, NULL);
            long diff = _end_time.tv_sec * 1000000 + _end_time.tv_usec - _begin_time.tv_sec * 1000000 - _begin_time.tv_usec;
            return diff;
        }
        double get_sec()
        {
            struct timeval _end_time;
            gettimeofday(&_end_time, NULL);
            double diff = _end_time.tv_sec - _begin_time.tv_sec + (_end_time.tv_usec - _begin_time.tv_usec) / 1000000.0;
            return diff;
        }
        
    private:
        struct timeval _begin_time;
};

#endif
