#ifndef _LOG_ILINK_H
#define _LOG_ILINK_H

#include <vector>
using namespace std;

namespace nLog
{
	class iLinker;

	typedef vector<iLinker*> linkerList;

	class iLinker
	{
		public:
			void addLinker(iLinker* ptr)
			{
				_linkerList.push_back(ptr);
			}

			void run(const char* pstr)
			{
				const char* re_pstr = handle(pstr);			
				FOREACH(linkerList, iter, _linkerList)
					run(re_pstr);
			}

			virtual const char* handle(const char* pstr) = 0;

		private:
			linkerList _linkerList;
	};

}

#endif
