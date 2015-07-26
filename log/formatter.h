#ifndef _LOG_FORMATTER_H
#define _LOG_FORMATTER_H

#include "attr.h"
#include "ticker.h"
#include "iLink.h"

namespace nLog
{
	class formatter : public iLinker
	{
		public:
			formatter(tickerMgr& ticker_mgr)
				: _tickerMgr(ticker_mgr){}

			formatter& operator<<(iAttr* attr_ptr)
			{
				_attrList.push_back(attr_ptr);
				return *this;
			}

			virtual const char* handle(const char* str)
			{
				_buff.clear();
				FOREACH(attrList, iter, _attrList)
					_buff += (*iter)->get();
				return _buff.c_str();
			}
		private:
			iAttr* createAttr(const char* attr_name, const char* format, int color);

		private:
			tickerMgr& _tickerMgr;
			attrList _attrList;
			string _buff;
	};
}

#endif
