#ifndef _ASYNC_FN_H
#define _ASYNC_FN_H

#include <boost/function.hpp>
#include <boost/bind.hpp>

typedef boost::function<void(int)> async_fn;

#define BIND(fn, ...) \
	boost::bind(fn, __VA_ARGS__) 

#endif
