#include "formatter.h"
#include <string.h>

namespace nLog
{
	iAttr* formatter::createAttr(const char* attr_name, const char* format, int color)
	{
		if(strcmp(attr_name, "TIME") == 0)
		{
			dTime* time_data = (dTime*)_sharedData.getDataByType(DTIME);		
			return new timeAttr(time_data, format, color);
		}
		if(strcmp(attr_name, "LINEID") == 0)
		{
			dLineId* line_id_data = (dLineId*)_sharedData.getDataByType(DLINEID);
			return new lineIdAttr(line_id_data, format, color);
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
			return new messageAttr();
		}
		return NULL;
	}
}
