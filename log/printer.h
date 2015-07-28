#ifndef _LOG_PRINTER_H
#define _LOG_PRINTER_H

#include <stdio.h>
#include "iLink.h"

namespace nLog
{
	class printer : public iLinker
	{
		public:
			virtual const char* handle(const char* pstr)
			{
				printf("%s", pstr);
				return pstr;
			}
	};
}

#endif
