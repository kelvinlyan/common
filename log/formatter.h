#ifndef _LOG_FORMATTER_H
#define _LOG_FORMATTER_H

#include "attr.h"
#include "iLink.h"
#include "logger.h"

namespace nLog
{
	class formatter : public iLinker
	{
		public:
			formatter(logger& l)
				: _sharedData(l.getSharedData()){}

			formatter& operator<<(iAttr* attr_ptr)
			{
				_attrList.push_back(attr_ptr);
				messageAttr* ptr = dynamic_cast<messageAttr*>(attr_ptr);
				if(ptr != NULL)
					_messageAttr = (messageAttr*)attr_ptr;
				return *this;
			}

			virtual const char* handle(const char* pstr)
			{
				_buff.clear();
				_messageAttr->setMsgPtr(pstr);
				FOREACH(attrList, iter, _attrList)
					_buff += (*iter)->get();
				return _buff.c_str();
			}

			iAttr* createAttr(const char* attr_name, const char* format = NULL, int color = nColor::DEFAULT);

		private:
			sharedData& _sharedData;
			attrList _attrList;
			string _buff;
			messageAttr* _messageAttr;
	};
}

#endif
