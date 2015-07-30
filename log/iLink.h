#ifndef _LOG_ILINK_H
#define _LOG_ILINK_H

#include "../macro/macro_header.h"
#include <vector>
using namespace std;

namespace nLog
{
	class iLink;

	typedef vector<iLink*> linkerList;

	class iLink
	{
		public:
			virtual void push(iLink* ptr)
			{
				_linkerList.push_back(ptr);
			}

			void run(const char* pstr)
			{
				const char* re_pstr = handle(pstr);			
				FOREACH(linkerList, iter, _linkerList)
					(*iter)->run(re_pstr);
			}
		
			virtual const char* handle(const char* pstr) = 0;

		private:
			linkerList _linkerList;
	};

}

#endif
