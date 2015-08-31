#ifndef _ATOMIC_H
#define _ATOMIC_H

class atomic
{
	public:
		static void add(uint32_t* pw)
		{
			__asm__
			(
				"lock\n\t"
				"incl %0":
				"=m"(*pw):
				"m"(pw):
				"cc"
			);
		}
		static void sub(uint32_t* pw)
		{
			__asm__
			(
				"lock\n\t"
				"decl %0":
				"=m"(*pw):
				"m"(pw):
				"cc"
			);
		}
};

#endif
