#ifndef _FILE_HELPER_H
#define _FILE_HELPER_H

#include <sys/types.h>
#include <sys/stat.h>

namespace nFile
{
	static inline long getFileSize(const char* file_name)
	{
		struct stat st;
		if(stat(file_name, &st) == -1)
			return -1;
		return st.st_size;
	}
}

#endif
