#ifndef _LOG_WRITER_H
#define _LOG_WRITER_H

#include <string.h>
#include "../macro/macro_header.h"
#include "../file/file_header.h"
#include "attr.h"

namespace nLog
{
	class fileName
	{
		public:
			void push(iAttr* attr_ptr)
			{
				_attrList.push_back(attr_ptr);
			}
			void setByteSize(unsigned size)
			{
				_byte_size = size;
			}
			void setTimeSize(unsigned size)
			{
				_time_size = size;
			}
			const char* getName()
			{
				checkAndUpdate();
				return _buff.c_str();
			}
		private:
			void checkAndUpdate()
			{
				if(_time_size != 0)
				{
					unsigned now = time(NULL);
					if(now >= _next_update_time)
					{
						
						_next_update_time = getNextUpdateTime(now);
						_getName();
						_current_size = getCurrentSize();
					}
				}
				if(_byte_size != 0 && _current_size >= _byte_size)
				{
					
				}
			}
			void _getName()
			{
				_buff.clear();
				FOREACH(attrList, iter, _attrList)
					_buff += (*iter)->get();
			}
			unsigned getNextUpdateTime(unsigned now)
			{
				return (now / _time_size + 1) * _time_size;
			}
			unsigned getCurrentSize()
			{
				long size = nFile::getFileSize(_buff.c_str());
				return size == -1? 0 : (unsigned)size;
			}

		private:
			attrList _attrList;
			timeAttr* timeAttrPtr;
			string _buff;

			unsigned _time_size;
			unsigned _next_update_time;
			unsigned _byte_size;
			unsigned _current_size;
	};

	class writer : public iLink
	{
		public:
			writer(logger& lg)
				: _sharedData(lg.getSharedData()), _byte_size(0), _time_size(0)
			{
			}
			void setByteSize(unsigned size)
			{
				_file_name.setByteSize(size);
			}
			void setTimeSize(unsigned size)
			{
				_file_name.setTimeSize(size);
			}
			writer& operator<<(iAttr* attr_ptr)
			{
				_file_name.push(attr_ptr);
				return *this;
			}
			virtual const char* handle(const char* pstr)
			{
				_handle(pstr);
				return pstr;
			}

		private:
			string getFileName();
			void _handle(const char* pstr)
			{
				string name = getFileName();
				fwrite(pstr, strlen(pstr) + 1, 1, _fptr);
			}

		private:
			FILE* _fptr;
			string _fname;

			unsigned _byte_size;
			unsigned _time_size;
			fileName _file_name;

			int _vbuf_type;
			char* _vbuf_ptr;
			unsigned _vbuf_size;

			sharedData& _sharedData;
			attrList _attrList;
	};
}

#endif
