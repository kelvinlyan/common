#ifndef _LOG_TICKER_H
#define _LOG_TICKER_H

namespace nLog
{
	enum
	{
		TTIME = 0,
		TLINEID,
		TMAX
	};


	class iTicker
	{
		public:
			virtual ~iTicker(){}
			virtual void tick() = 0;
	};

	typedef vector<iTicker*> tickerList;

	class timeTicker : public iTicker
	{
		public:
			virtual void tick()
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

	class lineIdTicker : public iTicker
	{
		public:
			lineIdTicker() : _line_id(0){}
			virtual void tick()
			{
				++_line_id;
			}
			const unsigned* getLineId() const { return &_line_id; }

		private:
			unsigned _line_id;
	};

	class tickerMgr
	{
		public:
			tickerMgr()
			{
				for(int i = 0; i < TMAX; ++i)
					_tickerType[i] = NULL;
			}
			~tickerMgr()
			{
				FOREACH(tickerList, iter, _tickerList)
					delete *iter;
			}
			void tick()
			{
				FOREACH(tickerList, iter, _tickerList)
					(*iter)->tick();
			}
			iTicker* getTickerByType(int type)
			{
				if(type < 0 || type >= TMAX)
					return NULL;
				if(_tickerType[type] == NULL)
				{
					_tickerType[type] = createTicker(type);
					_tickerList.push_back(_tickerType[type]);
				}
				return _tickerType[type];
			}
		private:
			iTicker* createTicker(int type)
			{
				switch(type)
				{
					case TTIME:
						return new timeTicker();
					case TLINEID:
						return new lineIdTicker();
					default:
						return NULL;
				}
			}

		private:
			tickerList _tickerList;
			iTicker* _tickerType[TMAX];
	};
}

#endif
