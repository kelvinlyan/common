#ifndef _COMMON_NONCOPYABLE_H
#define _COMMON_NONCOPYABLE_H

class noncopyable
{
	protected:
		noncopyable(){}
		~noncopyable(){}
	private:
		noncopyable(const noncopyable&);
		const noncopyable& operator=(const noncopyable&);
};

#endif
