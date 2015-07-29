#ifndef _LOG_IMPLEMENT_H
#define _LOG_IMPLEMENT_H

#include <string>
#include "iLink.h"
using namespace std;

namespace nLog
{
	class implement : public iLink
	{
		public:
			inline void preLog()
			{
			
			}
			inline void doLog()
			{
				_buff.clear();
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

		private:
			string _buff;
	};
}

#endif

