#ifndef _COMMON_LOGGER_H
#define _COMMON_LOGGER_H

class iAttr
{
	public:
		virtual string get() = 0;
};

class logger
{
	public:
		logger& operator<<(const char* pstr);
		

	private:
		bool _bFile;
		string _filename;
		vector<iAttr*> attrs;
};



#endif

