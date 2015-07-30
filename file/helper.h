#ifndef _FILE_HELPER_H
#define _FILE_HELPER_H

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace nFile
{
	static inline long getFileSize(const char* file_name)
	{
		struct stat st;
		if(stat(file_name, &st) == -1)
			return -1;
		return st.st_size;
	}

	static inline bool isFileExist(const char* file_name)
	{
		return access(file_name, 0) != -1;
	}

	static inline bool mkDir(const char* dir_name)
	{
		return mkdir(dir_name, 0755) == 0;
	}
}

#endif
