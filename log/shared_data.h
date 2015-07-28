#ifndef _LOG_SHARED_DATA_H
#define _LOG_SHARED_DATA_H

#include <vector>
#include <stdio.h>
#include <time.h>
#include "../macro/header.h"
using namespace std;

namespace nLog
{
	enum
	{
		DTIME = 0,
		DLINEID,
		DMAX
	};
	
	class iTick
	{
		public:
			virtual ~iTick(){}
			virtual void run() = 0;
	};

	typedef vector<iTick*> tickerList;

	class iData
	{
		public:
			virtual ~iData(){}
	};

	class dTime : public iTick, public iData
	{
		public:
			virtual void run()
			{
				_now = time(NULL);
				localtime_r(&_now, &_tm);
				_year = _tm.tm_year + 1900;
				_month = _tm.tm_mon + 1;
			}
			const time_t* getTimeStamp() const { return &_now; }
			const int* getYear() const { return &_year; }
			const int* getMonth() const { return &_month; }
			const int* getDay() const { return &_tm.tm_mday; }
			const int* getHour() const { return &_tm.tm_hour; }
			const int* getMin() const { return &_tm.tm_min; }
			const int* getSec() const { return &_tm.tm_sec; }

		private:
			time_t _now;
			struct tm _tm;
			int _year;
			int _month;
	};

	class dLineId : public iTick, public iData
	{
		public:
			dLineId() : _line_id(0){}
			virtual void run()
			{
				++_line_id;
			}
			const unsigned* getLineId() const { return &_line_id; }

		private:
			unsigned _line_id;
	};

	class sharedData
	{
		public:
			sharedData()
			{
				for(int i = 0; i < DMAX; ++i)
					_dataList[i] = NULL;
			}
			~sharedData()
			{
				for(int i = 0; i < DMAX; ++i)
				{
					if(!(_dataList[i]))
						delete _dataList[i];
				}
			}
			void update()
			{
				FOREACH(tickerList, iter, _tickerList)
					(*iter)->run();
			}
			iData* getDataByType(int type)
			{
				if(type < 0 || type >= DMAX)
					return NULL;
				if(!_dataList[type])
				{
					_dataList[type] = getDataPtr(type);
					iTick* ptr = dynamic_cast<iTick*>(_dataList[type]);	
					if(ptr != NULL)
						_tickerList.push_back(ptr);
				}
				return _dataList[type];
			}
		private:
			iData* getDataPtr(int type)
			{
				switch(type)
				{
					case DTIME:
						return new dTime();
					case DLINEID:
						return new dLineId();
					default:
						return NULL;
				}
			}

		private:
			tickerList _tickerList;
			iData* _dataList[DMAX];
			//dataList _dataList; 
	};
}

#endif
