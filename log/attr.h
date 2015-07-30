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
	namespace nAttr
	{

#ifndef NLOGATTRBUF
#define NLOGATTRBUF 256
#endif

#ifndef NLOGMSGBUF
#define NLOGMSGBUF 10240
#endif
		enum
		{
			LINEID = 0,
			CONSTANT,
			TIME,
			THREADID,
			MESSAGE
		};


		class iAttr
		{
			public:
				virtual const char* get() = 0;
				virtual ~iAttr(){}
		};

		typedef vector<iAttr*> attrList;
		typedef const char*(*getAttr)(void);

		class constant : public iAttr
		{
			public:
				static constant* create(const char* pstr, int color = nColor::DEFAULT)
				{
					return new constant(pstr, color);
				}
				virtual const char* get()
				{
					return _value.c_str();
				}
			protected:
				constant(const char* pstr, int color = nColor::DEFAULT)
				{
					_value = nColor::makeColor(pstr, color);
				}
			private:
				string _value;
		};

		class counter : public iAttr
		{
			public:
				static counter* create(int begin = 0, int step = 1, const char* format = NULL, int color = nColor::DEFAULT)
				{
					return new counter(begin, step, format, color);
				}
				virtual const char* get()
				{
					_count += _step;
					snprintf(_buff, NLOGATTRBUF - 1, _format.c_str(), _count);
					return _buff;
				}
			protected:
				counter(int begin = 0, int step = 1, const char* format = NULL, int color = nColor::DEFAULT)
					: _count(begin), _step(step)
				{
					_format = format? format : "%d";
					_format = nColor::makeFormat(_format.c_str(), color);
				}
			private:
				int _count;
				int _step;
				string _format;
				char _buff[NLOGATTRBUF];
		};

		class lineId : public counter
		{
			public:
				static lineId* create(const char* format = NULL, int color = nColor::DEFAULT)
				{
					return new lineId(format, color);
				}
			protected:
				lineId(const char* format = NULL, int color = nColor::DEFAULT)
					: counter(0, 1, format, color){}
		};

#define CREATE_BINDATTR(BINDTYPE, ATTRNAME, TYPEFORMAT) \
	class ATTRNAME : public iAttr \
	{ \
		public: \
			static ATTRNAME* create(const BINDTYPE* ptr, const char* format = NULL, int color = nColor::DEFAULT) \
			{ \
				return new ATTRNAME(ptr, format, color); \
			} \
			virtual const char* get() \
			{ \
				snprintf(_buff, NLOGATTRBUF - 1, _format.c_str(), *_value); \
				return _buff; \
			} \
		protected: \
			ATTRNAME(const BINDTYPE* ptr, const char* format = NULL, int color = nColor::DEFAULT) \
				: _value(ptr) \
			{ \
				_format = format? format : TYPEFORMAT; \
				_format = nColor::makeFormat(_format.c_str(), color); \
			} \
		private: \
			const BINDTYPE* _value; \
			string _format; \
			char _buff[NLOGATTRBUF]; \
	}; \

		CREATE_BINDATTR(int, bindInt, "%d")
		CREATE_BINDATTR(unsigned, bindUInt, "%u")
		CREATE_BINDATTR(char* const, bindCStr, "%s")

		class severity : public bindCStr
		{
			public:
				static severity* create(logger& lg, const char* format = NULL, int color = nColor::DEFAULT)
				{
					return new severity(lg, format, color);
				}
			protected:
				severity(logger& lg, const char* format = NULL, int color = nColor::DEFAULT)
					: bindCStr(lg.getSeverity(), format, color){}
		};

		class nTime : public iAttr
		{
			public:
				static nTime* create(const char* format, int color = nColor::DEFAULT)
				{
					return new nTime(format, color);
				}
				virtual ~nTime()
				{
					FOREACH(vector<iAttr*>, iter, _attrs)
						delete *iter;
				}
				virtual const char* get()
				{
					update();
					_buff.clear();
					FOREACH(attrList, iter, _attrs)
						_buff += (*iter)->get();
					if(_color != nColor::DEFAULT)
						_buff = nColor::makeColor(_buff.c_str(), _color);
					return _buff.c_str();
				}
				void setTime(time_t t)
				{
					_now = t;
					_flag = true;
				}
			private:
				nTime(const char* format, int color = nColor::DEFAULT)
					: _color(color), _flag(false)
				{
					analyse(format);
				}
				void update()
				{
					if(_flag)
						_flag = false;
					else
						_now = time(NULL);

					localtime_r(&_now, &_tm);
					_year = _tm.tm_year + 1900;
					_month = _tm.tm_mon + 1;
				}

				bool analyseField(const string& format, int begin, int end, const int* bind_ptr);
				void analyse(const string& format);

			private:
				bool _flag;
				time_t _now;
				struct tm _tm;
				int _year;
				int _month;

				attrList _attrs;		
				string _buff;
				int _color;
		};
		class threadId : public iAttr
		{
			public:
				static threadId* create(const char* format = NULL, int color = nColor::DEFAULT)
				{
					return new threadId(format, color);
				}
				virtual const char* get()
				{
					pid_t tid = syscall(SYS_gettid);
					snprintf(_buff, NLOGATTRBUF - 1, _format.c_str(), tid);
					return _buff;
				}
			private:
				threadId(const char* format, int color)
				{
					_format = format? format : "%u";
					_format = nColor::makeFormat(_format.c_str(), color);
				}
			private:
				string _format;
				char _buff[NLOGATTRBUF];
		};
		class message : public iAttr
		{
			public:
				static message* create(int color = nColor::DEFAULT)
				{
					return new message(color);
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
				message(int color = nColor::DEFAULT)
					: _color(color)
				{
					if(_color != nColor::DEFAULT)
						_format = nColor::makeFormat("%s", color);
				}
			private:
				const char* _msg_ptr;
				char _buff[NLOGMSGBUF];
				string _format;
				int _color;
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
		bool nTime::analyseField(const string& format, int begin, int end, const int* bind_ptr)
		{
			string temp = format.substr(begin, end - begin + 1);
			temp[temp.size() - 1] = 'd';
			_attrs.push_back(bindInt::create(bind_ptr, temp.c_str()));
			return true;
		}
		void nTime::analyse(const string& format)
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
									_attrs.push_back(bindUInt::create((unsigned*)&_now, temp.c_str()));
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
					_attrs.push_back((iAttr*)(constant::create(temp.c_str())));
			}
		}
	}
}


#endif
