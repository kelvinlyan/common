#include "formatter.h"
#include <string.h>

namespace nLog
{
	iAttr* formatter::createAttr(const char* attr_name, const char* format, int color)
	{
		if(strcmp(attr_name, "TIME") == 0)
		{
			timeTicker* time_ticker = (timeTicker*)_tickerMgr.getTickerByType(TTIME);		
			return new timeAttr(time_ticker, format, color);
		}
		if(strcmp(attr_name, "LINEID") == 0)
		{
			lineIdTicker* line_id_ticker = (lineIdTicker*)_tickerMgr.getTickerByType(TLINEID);
			return new lineIdAttr(line_id_ticker, format, color);
		}
		if(strcmp(attr_name, "CONSTANT") == 0)
		{
			return new constantAttr(format, color);
		}
		if(strcmp(attr_name, "THREADID") == 0)
		{
			return new threadIdAttr(format, color);
		}
		if(strcmp(attr_name, "MESSAGE") == 0)
		{
			return NULL;//new messageAttr();
		}
	}
}
