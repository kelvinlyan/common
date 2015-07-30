#ifndef _LOG_FORMATTER_H
#define _LOG_FORMATTER_H

#include "iLink.h"
#include "attr.h"

namespace nLog
{
	class formatter : public iLink
	{
		public:
			formatter()
				: _msgPtr(NULL)
			{
			}
			formatter& operator<<(nAttr::iAttr* attr_ptr)
			{
				_attrList.push_back(attr_ptr);
				nAttr::message* ptr = dynamic_cast<nAttr::message*>(attr_ptr);
				if(ptr != NULL)
					_msgPtr = (nAttr::message*)attr_ptr;
				return *this;
			}

			virtual const char* handle(const char* pstr)
			{
				_buff.clear();
				if(_msgPtr)
					_msgPtr->setMsgPtr(pstr);
				FOREACH(nAttr::attrList, iter, _attrList)
					_buff += (*iter)->get();
				return _buff.c_str();
			}

		private:
			nAttr::attrList _attrList;
			string _buff;
			nAttr::message* _msgPtr;
	};
}

#endif
