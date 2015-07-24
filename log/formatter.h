#ifndef _LOG_FORMATTER_H
#define _LOG_FORMATTER_H

#include "attr.h"
#include "ticker.h"

namespace nLog
{
	class formatter
	{
		public:
			formatter(tickerMgr& ticker_mgr)
				: _tickerMgr(tickerMgr){}

			formatter& operator<<(const char* attr_name, const char* format = NULL, int color = nColor::DEFAULT)
			{
				_attrList.push_back(createAttr(attr_name, format, color));
			}

			const char* run(const char* str)
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
