#ifndef _LOG_FORMATTER_H
#define _LOG_FORMATTER_H

#include "iLink.h"
#include "attr.h"

namespace nLog
{
	class formatter : public iLink
	{
		public:
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
				if(_messageAttr)
					_messageAttr->setMsgPtr(pstr);
				FOREACH(attrList, iter, _attrList)
					_buff += (*iter)->get();
				return _buff.c_str();
			}

		private:
			attrList _attrList;
			string _buff;
			messageAttr* _messageAttr;
	};
}

#endif
