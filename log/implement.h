#ifndef _LOG_IMPLEMENT_H
#define _LOG_IMPLEMENT_H

#include "shared_data.h"
#include <string>
#include "iLink.h"
using namespace std;

namespace nLog
{
	class implement : public iLinker
	{
		public:
			inline void preLog()
			{
				_buff.clear();
				_sharedData.update();
			}

			inline void doLog()
			{
				_buff += "\n";
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

			sharedData& getSharedData() { return _sharedData; } 

		private:
			string _buff;
			sharedData _sharedData;
	};
}

#endif

