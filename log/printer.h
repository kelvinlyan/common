#ifndef _LOG_PRINTER_H
#define _LOG_PRINTER_H

#include <stdio.h>
#include "iLink.h"

namespace nLog
{
	class printer : public iLink
	{
		public:
			virtual const char* handle(const char* pstr)
			{
				printf("%s\n", pstr);
				return pstr;
			}
	};
}

#endif
