#ifndef _LOG_WRITER_H
#define _LOG_WRITER_H

#include <stdio.h>

class nLog
{
	class printer : public iLinker
	{
		public:
			virtual const char* handle(const char* pstr)
			{
				printf("%s", pstr);
			}
	};
}

#endif
