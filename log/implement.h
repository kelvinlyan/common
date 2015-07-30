#ifndef _LOG_IMPLEMENT_H
#define _LOG_IMPLEMENT_H

#include <string>
#include "iLink.h"
using namespace std;

namespace nLog
{
	enum
	{
		INFO = 0,
		WARM,
		ERROR,
		DEBUG,
		SMAX
	};

	const char* severityStr[SMAX] = { "INFO", "WARM", "ERROR", "DEBUG" };

	class implement : public iLink
	{
		public:
			inline void preLog(int severity)
			{
				_buff.clear();
				_severityPtr = severityStr[severity];
			}
			inline void doLog()
			{
				run(_buff.c_str());
			}
			
			virtual const char* handle(const char* pstr)
			{
				return pstr;
			}
			
			implement& operator<<(const char* pstr)
			{
				_buff += pstr;
				return *this;
			}
			const char* const * getSeverityPtr() const { return &_severityPtr; }

		private:
			string _buff;
			const char* _severityPtr;
	};
}

#endif

