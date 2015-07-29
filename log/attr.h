#ifndef _LOG_ATTR_H
#define _LOG_ATTR_H

#include <sys/syscall.h>
#include <sys/types.h>
#include <string.h>
#include <string>
#include <vector>
#include "../macro/macro_header.h"
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

	typedef vector<iAttr*> attrList;
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
			unsigned _line_id;
			string _format;
			char _buff[NLOGATTRBUF];
	};	

	class counterAttr : public iAttr
	{
		public:
			counterAttr(const char* format = NULL, int color = nColor::DEFAULT)
				: _count(0), _step(1)
			{
				_format = format? format : "%d";
				_format = nColor::makeFormat(_format.c_str(), color);
			}
			virtual const char* get()
			{
				_count += _step;
				snprintf(_buff, NLOGATTRBUF - 1, _format.c_str(), _count);
				return _buff;
			}
			void setArgs(int count, int step)
			{
				_count = count;
				_step = step;
			}

		private:
			int _count;
			int _step;
			string _format;
			char _buff[NLOGATTRBUF];
	};

#define CREATE_BINDATTR(BINDTYPE, ATTRNAME, TYPEFORMAT) \
	class ATTRNAME : public iAttr \
	{ \
		public: \
			ATTRNAME(const char* format = NULL, int color = nColor::DEFAULT) \
			{ \
				_format = format? format : TYPEFORMAT; \
				_format = nColor::makeFormat(_format.c_str(), color); \
			} \
			virtual const char* get() \
			{ \
				snprintf(_buff, NLOGATTRBUF - 1, _format.c_str(), *_value); \
				return _buff; \
			} \
			void setArgs(const BINDTYPE* ptr) \
			{ \
				_value = ptr; \
			} \
		private: \
			const BINDTYPE* _value; \
			string _format; \
			char _buff[NLOGATTRBUF]; \
	}; \

	CREATE_BINDATTR(int, bindIntAttr, "%d")
	CREATE_BINDATTR(unsigned, bindUIntAttr, "%u")
	CREATE_BINDATTR(char*, bindCStrAttr, "%s")

	class timeAttr : public iAttr
	{
		public:
			timeAttr(const string& format, int color = nColor::DEFAULT)
				: _color(color), _auto(true)
			{
				analyse(format);
			}
			~timeAttr()
			{
				FOREACH(vector<iAttr*>, iter, _attrs)
					delete *iter;
			}
			virtual const char* get()
			{
				update();
				FOREACH(attrList, iter, _attrs)
					_buff += (*iter)->get();
				if(_color != nColor::DEFAULT)
					_buff = nColor::makeColor(_buff.c_str(), _color);
				return _buff.c_str();
			}
			void setArg(bool flag)
			{
				_auto = flag;
			}
			void setTime(time_t t)
			{
				_now = t;
			}

		private:
			void update()
			{
				if(_auto)
					_now = time(NULL);
				localtime_r(&_now, &_tm);
				_year = _tm.tm_year + 1900;
				_month = _tm.tm_mon + 1;
			}

			bool analyseField(const string& format, int begin, int end, const int* bind_ptr);
			void analyse(const string& format);

		private:
			bool _auto;
			time_t _now;
			struct tm _tm;
			int _year;
			int _month;

			attrList _attrs;		
			string _buff;
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
			messageAttr(const char* format, int color)
				: _color(color)
			{
				if(_color != nColor::DEFAULT)
					_format = nColor::makeFormat("%s", color);
			}
			void setMsgPtr(const char* pstr)
			{
				_msg_ptr = pstr;
			}
			virtual const char* get()
			{
				if(_color == nColor::DEFAULT)
					return _msg_ptr;
				else
				{
					snprintf(_buff, NLOGMSGBUF - 1, _format.c_str(), _msg_ptr);
					return _buff;
				}
			}

		private:
			const char* _msg_ptr;
			char _buff[NLOGMSGBUF];
			string _format;
			int _color;
	};

	class attrFactory
	{
		public:
			static iAttr* create(const char* attr_name, const char* format = NULL, int color = nColor::DEFAULT)
			{
				if(strcmp(attr_name, "TIME") == 0)
					return new timeAttr(format, color);
				if(strcmp(attr_name, "LINEID") == 0)
					return new lineIdAttr(format, color);
				if(strcmp(attr_name, "CONSTANT") == 0)
					return new constantAttr(format, color);
				if(strcmp(attr_name, "THREADID") == 0)
					return new threadIdAttr(format, color);
				if(strcmp(attr_name, "MESSAGE") == 0)
					return new messageAttr(format, color);
				return NULL;
			}
	};

	bool timeAttr::analyseField(const string& format, int begin, int end, const int* bind_ptr)
	{
		string temp = format.substr(begin, end - begin + 1);
		if(!temp.empty())
		{ 
			temp[temp.size() - 1] = 'd';
			bindIntAttr* ptr = new bindIntAttr(temp.c_str());
			ptr->setArgs(bind_ptr);
			_attrs.push_back(ptr);
		}
		return true;
	}

	void timeAttr::analyse(const string& format)
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
					bool flag = false;
					switch(format[i])
					{
						case 'Y':
							flag = analyseField(format, begin, i, &_year);
							break;
						case 'm':
							flag = analyseField(format, begin, i, &_month);
							break;
						case 'd':
							flag = analyseField(format, begin, i, &_tm.tm_mday);
							break;
						case 'H':
							flag = analyseField(format, begin, i, &_tm.tm_hour);
							break;
						case 'M':
							flag = analyseField(format, begin, i, &_tm.tm_min);
							break;
						case 'S':
							flag = analyseField(format, begin, i, &_tm.tm_sec);
							break;
						case 's':
							{
								string temp = format.substr(begin, i - begin + 1);
								if(!temp.empty())
								{
									temp[temp.size() - 1] = 'u';
									bindUIntAttr* ptr = new bindUIntAttr(temp.c_str());
									ptr->setArgs((unsigned*)(&_now));
									_attrs.push_back(ptr);
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
		} while(true);

		if(begin != format.size())
		{
			string temp = format.substr(begin);
			if(!temp.empty())
				_attrs.push_back((iAttr*)(new constantAttr(temp.c_str())));
		}
	}
}

#endif
