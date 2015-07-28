#include "attr.h"

namespace nLog
{
	bool timeAttr::analyseField(const string& format, int begin, int end, const int* bind_ptr)
	{
		string temp = format.substr(begin, end - begin + 1);
		if(!temp.empty())
		{ 
			temp[temp.size() - 1] = 'd';
			_attrs.push_back((iAttr*)(new bindIntAttr(bind_ptr, temp.c_str())));
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
							flag = analyseField(format, begin, i, _year);
							break;
						case 'm':
							flag = analyseField(format, begin, i, _month);
							break;
						case 'd':
							flag = analyseField(format, begin, i, _day);
							break;
						case 'H':
							flag = analyseField(format, begin, i, _hour);
							break;
						case 'M':
							flag = analyseField(format, begin, i, _min);
							break;
						case 'S':
							flag = analyseField(format, begin, i, _sec);
							break;
						case 's':
						{
							string temp = format.substr(begin, i - begin + 1);
							if(!temp.empty())
							{
								temp[temp.size() - 1] = 'u';
								_attrs.push_back((iAttr*)(new bindUIntAttr((const unsigned*)(_timestamp), temp.c_str())));
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

