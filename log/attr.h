#ifndef _LOG_ATTR_H
#define _LOG_ATTR_H

#include <sys/syscall.h>
#include <sys/types.h>
#include <string>
#include <vector>
#include "../macro/header.h"
#include "../color/nColor.hpp"
#include "ticker.h"
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
			lineIdAttr(lineIdTicker* line_id_ticker, const char* format = NULL, int color = nColor::DEFAULT)
			{
				_line_id = line_id_ticker->getLineId();
				_format = format? format : "%u";
				_format = nColor::makeFormat(_format.c_str(), color);
			}
			virtual const char* get()
			{
				snprintf(_buff, NLOGATTRBUF - 1, _format.c_str(), *_line_id);
				return _buff;
			}
		private:
			string _format;
			char _buff[NLOGATTRBUF];
			const unsigned* _line_id;
	};	

#define CREATE_BINDATTR(BINDTYPE, ATTRNAME, TYPEFORMAT) \
	class ATTRNAME : public iAttr \
	{ \
		public: \
			ATTRNAME(const BINDTYPE* ptr, const char* format = NULL, int color = nColor::DEFAULT) \
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
			timeAttr(timeTicker* time_ticker, const string& format, int color = nColor::DEFAULT)
				: _color(color)
			{
				_year = time_ticker->getYear();
				_month = time_ticker->getMonth();
				_day = time_ticker->getDay();
				_hour = time_ticker->getHour();
				_min = time_ticker->getMin();
				_sec = time_ticker->getSec();
				_timestamp = time_ticker->getTimeStamp();

				analyse(format);
			}
			~timeAttr()
			{
				FOREACH(vector<iAttr*>, iter, _attrs)
					delete *iter;
			}
			virtual const char* get()
			{
				int count = 0;
				FOREACH(attrList, iter, _attrs)
					count += snprintf(_buff + count, NLOGATTRBUF - 1 - count, "%s", (*iter)->get());
				if(_color != nColor::DEFAULT)
				{
					string temp = nColor::makeColor(_buff, _color);
					snprintf(_buff, NLOGATTRBUF - 1, "%s", temp.c_str());
				}
				return _buff;
			}
			
		private:
			bool analyseField(const string& format, int begin, int end, const int* bind_ptr);
			void analyse(const string& format);

		private:
			const int* _year;
			const int* _month;
			const int* _day;
			const int* _hour;
			const int* _min;
			const int* _sec;
			const time_t* _timestamp;

			attrList _attrs;		
			char _buff[NLOGATTRBUF];
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
