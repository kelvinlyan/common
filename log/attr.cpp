#include "attr.h"

namespace nLog
{
	bool timeAttr::analyseField(const string& format, int begin, int end, int& bind_value)
	{
		string temp = format.substr(begin, end - begin + 1);
		if(!temp.empty())
		{
			temp[temp.size() - 1] = 'd';
			_attrs.push_back((iAttr*)(new bindIntAttr(&bind_value, temp.c_str())));
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
							flag = analyseField(temp, begin, i, _year);
							break;
						case 'm':
							flag = analyseField(temp, begin, i, _month);
							break;
						case 'd':
							flag = analyseField(temp, begin, i, _tm.tm_mday);
							break;
						case 'H':
							flag = analyseField(temp, begin, i, _tm.tm_hour);
							break;
						case 'M':
							flag = analyseField(temp, begin, i, _tm.tm_min);
							break;
						case 'S':
							flag = analyseField(temp, begin, i, _tm.tm_sec);
							break;
						case 's':
						{
							string temp = format.substr(begin, i - begin + 1);
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
		} while(true);

		if(begin != format.size())
		{
			string temp = format.substr(begin);
			if(!temp.empty())
				_attrs.push_back((iAttr*)(new constantAttr(temp.c_str())));
		}
	}
}
