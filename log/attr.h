#ifndef _Log_ATTR_H
#define _Log_ATTR_H

#include <sys/syscall.h>
#include <sys/types.h>
#include <string>
#include <vector>
#include "../macro/header.h"
#include "../color/nColor.hpp"
using namespace std;

namespace nLog
{
	enum
	{
		LINEID = 0,
		CONSTANT,
		TIME,
		THREADID,
		MESSAGE
	};

#ifndef NLOGATTRBUF
#define NLOGATTRBUF 256
#endif

#ifndef NLOGMSGBUF
#define NLOGMSGBUF 10240
#endif

	class iAttr
	{
		public:
			virtual const char* get() = 0;
	};

	typedef const char*(*getAttr)(void);

	class constantAttr : public iAttr
	{
		public:
			constantAttr(const char* str, int color = nColor::DEFAULT)
			{
				_value = nColor::makeColor(str, color);
			}
			virtual const char* get()
			{
				return _value.c_str();
			}
		private:
			string _value;
	};

	class lineIdAttr : public iAttr
	{
		public:
			lineIdAttr(const char* format = NULL, int color = nColor::DEFAULT)
				: _line_id(0)
			{
				_format = format? format : "%u";
				_format = nColor::makeFormat(_format.c_str(), color);
			}
			virtual const char* get()
			{
				++_line_id;
				snprintf(_buff, NLOGATTRBUF - 1, _format.c_str(), _line_id);
				return _buff;
			}
		private:
			string _format;
			char _buff[NLOGATTRBUF];
			unsigned _line_id;
	};	

#define CREATE_BINDATTR(BINDTYPE, ATTRNAME, TYPEFORMAT) \
	class ATTRNAME : public iAttr \
	{ \
		public: \
			ATTRNAME(BINDTYPE* ptr, const char* format = NULL, int color = nColor::DEFAULT) \
				: _value(ptr) \
			{ \
				_format = format? format : TYPEFORMAT; \
				_format = nColor::makeFormat(_format.c_str(), color); \
			} \
			virtual const char* get() \
			{ \
				snprintf(_buff, NLOGATTRBUF - 1, _format.c_str(), *_value); \
				return _buff; \
			} \
		private: \
			BINDTYPE* _value; \
			string _format; \
			char _buff[NLOGATTRBUF]; \
	}; \

	CREATE_BINDATTR(int, bindIntAttr, "%d")
	CREATE_BINDATTR(unsigned, bindUIntAttr, "%u")
	CREATE_BINDATTR(const char, bindCStrAttr, "%s")

	class timeAttr : public iAttr
	{
		public:
			timeAttr(const string& format, int color = nColor::DEFAULT)
				: _color(color)
			{
				analyse(format);
			}
			virtual const char* get()
			{
				_now = time(NULL);
				localtime_r(&_now, &_tm);
				_year = _tm.tm_year + 1900;
				_month = _tm.tm_mon + 1;
				int count = 0;
				FOREACH(vector<iAttr*>, iter, _attrs)
					count += snprintf(_buff + count, NLOGATTRBUF - 1 - count, "%s", (*iter)->get());
				if(_color != nColor::DEFAULT)
				{
					string temp = nColor::makeColor(_buff, _color);
					snprintf(_buff, NLOGATTRBUF - 1, "%s", temp.c_str());
				}
				return _buff;
			}
			
		private:
			void analyse(const string& format)
			{
				int begin = 0;
				int end = 0;
				do
				{
					end = format.find('%', begin);
					if(end == string::npos)
					{
						break;
					}
					else
					{
						for(int i = end + 1; i < format.size(); ++i)
						{
							char c = format[i];
							string temp;
							bool flag = false;
							switch(c)
							{
								case 'Y':
								{
									temp = format.substr(begin, i - begin + 1);
									if(!temp.empty())
									{
										temp[temp.size() - 1] = 'd';
										_attrs.push_back((iAttr*)(new bindIntAttr(&_year, temp.c_str())));
									}
									flag = true;
									break;
								}
								case 'm':
								{
									temp = format.substr(begin, i - begin + 1);
									if(!temp.empty())
									{
										temp[temp.size() - 1] = 'd';
										_attrs.push_back((iAttr*)(new bindIntAttr(&_month, temp.c_str())));
									}
									flag = true;
									break;
								}
								case 'd':
								{
									temp = format.substr(begin, i - begin + 1);
									if(!temp.empty())
									{
										temp[temp.size() - 1] = 'd';
										_attrs.push_back((iAttr*)(new bindIntAttr(&_tm.tm_mday, temp.c_str())));
									}
									flag = true;
									break;
								}
								case 'H':
								{
									temp = format.substr(begin, i - begin + 1);
									if(!temp.empty())
									{
										temp[temp.size() - 1] = 'd';
										_attrs.push_back((iAttr*)(new bindIntAttr(&_tm.tm_hour, temp.c_str())));
									}
									flag = true;
									break;
								}
								case 'M':
								{
									temp = format.substr(begin, i - begin + 1);
									if(!temp.empty())
									{
										temp[temp.size() - 1] = 'd';
										_attrs.push_back((iAttr*)(new bindIntAttr(&_tm.tm_min, temp.c_str())));
									}
									flag = true;
									break;
								}
								case 'S':
								{
									temp = format.substr(begin, i - begin + 1);
									if(!temp.empty())
									{
										temp[temp.size() - 1] = 'd';
										_attrs.push_back((iAttr*)(new bindIntAttr(&_tm.tm_sec, temp.c_str())));
									}
									flag = true;
									break;
								}
								case 's':
								{
									temp = format.substr(begin, i - begin + 1);
									if(!temp.empty())
									{
										temp[temp.size() - 1] = 'u';
										_attrs.push_back((iAttr*)(new bindUIntAttr((unsigned*)(&_now), temp.c_str())));
									}
									flag = true;
									break;
								}
								default:
									break;
							}
							if(flag)
							{
								begin = i + 1;
								break;
							}
						}
					}
				}
				while(true);
				if(begin != format.size())
				{
					string temp = format.substr(begin);
					if(!temp.empty())
						_attrs.push_back((iAttr*)(new constantAttr(temp.c_str())));
				}
			}

		private:
			time_t _now;
			struct tm _tm;
			int _year;
			int _month;
			vector<iAttr*> _attrs;		
			char _buff[128];
			int _color;
	};

	class threadIdAttr : public iAttr
	{
		public:
			threadIdAttr(const char* format, int color)
			{
				_format = format? format : "%u";
				_format = nColor::makeFormat(_format.c_str(), color);
			}
			virtual const char* get()
			{
				pid_t tid = syscall(SYS_gettid);
				snprintf(_buff, NLOGATTRBUF - 1, _format.c_str(), tid);
				return _buff;
			}
		private:
			string _format;
			char _buff[NLOGATTRBUF];
	};

	class userDefinedAttr : public iAttr
	{
		public:
			userDefinedAttr(getAttr h)
				: _handler(h){}

			virtual const char* get()
			{
				return _handler();
			}
		private:
			getAttr _handler;
	};

	class messageAttr : public iAttr
	{
		public:
			messageAttr(const char* addr_ptr)
				: _addr_ptr(addr_ptr){}

			

			virtual const char* get()
			{
				return _addr_ptr;
			}

		private:
			const char* _addr_ptr;
	};
}

#endif
