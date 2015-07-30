#ifndef _LOG_WRITER_H
#define _LOG_WRITER_H

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "../macro/macro_header.h"
#include "../file/file_header.h"
#include "attr.h"

namespace nLog
{
	class writer : public iLink
	{
		public:
			writer()
				: _timePtr(NULL), _counter(NULL), _time_size(0), _byte_size(0), _init(false){}
			writer& operator<<(nAttr::iAttr* attr_ptr)
			{
				if(!_init)
				{
					_attrList.push_back(attr_ptr);
					nAttr::nTime* ptr = dynamic_cast<nAttr::nTime*>(attr_ptr);
					if(ptr != NULL)
						_timePtr = ptr;
					nAttr::counter* ptr_2 = dynamic_cast<nAttr::counter*>(attr_ptr);
					if(ptr != NULL)
						_counter = ptr_2;
				}
				return *this;
			}
			virtual ~writer()
			{
				FOREACH(nAttr::attrList, iter, _attrList)
					delete *iter;
				if(_fptr)
					fclose(_fptr);
			}
			void open()
			{
				_init = true;
				_buff = getNewFileName();
				if(_time_size != 0)
					_next_update_time = getNextUpdateTime(time(NULL));
				if(_byte_size != 0)
					_current_size = getCurrentSize();
			}
			void setByteSize(unsigned size)
			{
				if(!_init)
					_byte_size = size;
			}
			void setTimeSize(unsigned size)
			{
				if(!_init)
					_time_size = size;
			}
			virtual const char* handle(const char* pstr)
			{
				if(_init)
				{
					checkAndUpdate();
					fwrite(pstr, strlen(pstr), 1, _fptr);
					fwrite("\n", 1, 1, _fptr);
				}
				return pstr;
			}
		private:
			string getFilePath(const string& temp)
			{
				string str = "";
				int index = temp.rfind('/');
				if(index != string::npos)
					str = temp.substr(0, index);
				return str;
			}
			void checkAndUpdate()
			{
				if(_time_size != 0)
				{
					unsigned now = time(NULL);
					if(now >= _next_update_time)
					{
						if(_timePtr)
							_timePtr->setTime(now / _time_size * _time_size);
						_buff = getNewFileName();
						_next_update_time = getNextUpdateTime(now);
						_current_size = getCurrentSize();
					}
				}
				if(_byte_size != 0 && _current_size >= _byte_size)
				{
					if(_timePtr && _time_size != 0)
						_timePtr->setTime(time(NULL) / _time_size * _time_size);
					_buff = getNewFileName();
					_current_size = getCurrentSize();
				}
			}
			string getNewFileName()
			{
				string str;
				do
				{
					str = _getName();			
					if(nFile::isFileExist(str.c_str()) 
						&& _byte_size != 0 
						&& nFile::getFileSize(str.c_str()) < _byte_size)
					{
						if(!_fptr)
							fclose(_fptr);
						string path = getFilePath(str);
						if(path != "")
							nFile::mkDir(path.c_str());
						_fptr = fopen(str.c_str(), "a");
						return str;
					}
				}
				while(nFile::isFileExist(str.c_str()) && _counter != NULL);
				
				if(!nFile::isFileExist(str.c_str()))
				{
					if(!_fptr)
						fclose(_fptr);
					string path = getFilePath(str);
					if(path != "")
						nFile::mkDir(path.c_str());
					_fptr = fopen(str.c_str(), "a");
					return str;
				}

				string temp = str;
				unsigned count = 1;
				while(nFile::isFileExist(temp.c_str()))
				{
					ostringstream os;
					os << "-" << count++;
					temp += os.str();
				}
				rename(str.c_str(), temp.c_str());
				if(!_fptr)
					fclose(_fptr);
				string path = getFilePath(str);
				if(path != "")
					nFile::mkDir(path.c_str());
				_fptr = fopen(str.c_str(), "a");
				return str;
			}
			string _getName()
			{
				string str;
				FOREACH(nAttr::attrList, iter, _attrList)
					str += (*iter)->get();
				return str;
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
			bool _init;
			nAttr::attrList _attrList;
			nAttr::nTime* _timePtr;
			nAttr::counter* _counter;
			string _buff;
			FILE* _fptr;

			unsigned _time_size;
			unsigned _next_update_time;
			unsigned _byte_size;
			unsigned _current_size;

			int _vbuf_type;
			char* _vbuf_ptr;
			unsigned _vbuf_size;
	};
}

#endif
