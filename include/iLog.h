#ifndef _COMMON_ILOG_H
#define _COMMON_ILOG_H

#include <fstream>
#include <stdio.h>
#include <pthread.h>
#include <sys/stat.h>
#include <iomanip>

using namespace std;

namespace iLog
{
	class log_stream
	{
		public:
			log_stream()
			{
				pthread_mutex_init(&_mutex, NULL);
			}
			~log_stream()
			{
				if(_write.is_open())
					_write.close();

				pthread_mutex_destroy(&_mutex);	
			}

			inline bool is_open()
			{
				return _write.is_open();
			}

			template<typename T>
				inline log_stream& operator<<(const T& data)
				{
					_write << data;
				}

			inline void flush()
			{
				_write.flush();
			}

			inline void open(const char* file)
			{
				_write.open(file, ios::out | ios::app);
			}

			inline void close()
			{
				_write.close();
			}

			inline void lock()
			{
				pthread_mutex_lock(&_mutex);
			}

			inline void unlock()
			{
				pthread_mutex_unlock(&_mutex);
			}

		private:
			pthread_mutex_t _mutex;
			ofstream _write;
	};

	enum
	{
		DEBUG = 0,
		INFO,
		WARM,
		ERROR
	};

	const char* severity_strs[] = { "DEBUG", "INFO", "WARM", "ERROR" };

	class log_impl
	{
		public:
			log_impl()
			{
				_file_name = "../log/sample.log";
				time_t now = time(NULL);
				_current_begin_time = now - now % iTime::DAY;
				_current_end_time = _current_begin_time + iTime::DAY;

				if(isFileExist(_file_name.c_str()))
				{
					time_t create_time = getFileCreateTime(_file_name.c_str());
					if(create_time < _current_begin_time || create_time >= _current_end_time)
					{
						string new_file_name = getFileName(create_time);
						if(rename(_file_name.c_str(), new_file_name.c_str()) != 0)
							perror("rename");
					}
				}

				_stream.open(_file_name.c_str());
			}


			void update(time_t now)
			{
				if(_stream.is_open())
				{
					_stream.close();
				}

				string new_file_name = getFileName(_current_begin_time);

				if(rename(_file_name.c_str(), new_file_name.c_str()) != 0)
					perror("rename");

				_current_begin_time = now - now % 86400;
				_current_end_time = _current_begin_time + 86400;
				_stream.open(_file_name.c_str());
			}


			inline void checkAndUpdate(time_t now)
			{
				if(now >= _current_begin_time || now < _current_end_time)
					return;

				update(now);
			}

			string getFileName(time_t time_stamp)
			{
				struct tm t;
				localtime_r(&time_stamp, &t);

				char buff[1024];
				int n = sprintf(buff, "%s-%4d-%2d-%2d", _file_name.c_str(), t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);

				int count = 1;
				while(true)
				{
					if(count != 1)
						sprintf(buff + n, "-%d", count);
					if(!isFileExist(buff))
						break;

					++count;
				}

				return string(buff);

			}

			bool isFileExist(const char* pstr)
			{
				return (bool)ifstream(pstr);
			}

			time_t getFileCreateTime(const char* pstr)
			{
				struct stat st;
				stat(pstr, &st);
				return st.st_ctime;
			}

			inline void preLog(int severity, time_t now)
			{
				struct tm t;
				localtime_r(&now, &t);
				char buff[128];
				sprintf(buff, "%02d:%02d:%02d [%s] ", t.tm_hour, t.tm_min, t.tm_sec, severity_strs[severity]);
				_stream << buff;
			}

			inline void lastLog()
			{
				_stream << "\n";
				_stream.flush();
			}

			inline log_stream& stream()
			{
				return _stream;
			}

		private:
			time_t _current_begin_time;
			time_t _current_end_time;

			bool _bFile;
			string _file_name;

			log_stream _stream;
	};

	class log_helper_mt
	{
		public:
			log_helper_mt(log_impl* ptr, int severity)
				: _pImpl(ptr)
			{
				_pImpl->stream().lock();
				time_t now = time(NULL);
				_pImpl->checkAndUpdate(now);
				_pImpl->preLog(severity, now);
			}
			~log_helper_mt()
			{
				_pImpl->lastLog();
				_pImpl->stream().unlock();
			}

			template<typename T> 
			inline log_helper_mt& operator<<(const T& data)
			{
				_pImpl->stream() << data;
				return *this;
			}


		private:
			log_impl* _pImpl;
	};

	class logger_mt
	{
		public:
			inline log_helper_mt info()
			{
				return log_helper_mt(&_impl, INFO);
			}

		private:
			log_impl _impl;
	};
	
	class log_helper
	{
		public:
			log_helper(log_impl* ptr, int severity)
				: _pImpl(ptr)
			{
				time_t now = time(NULL);
				_pImpl->checkAndUpdate(now);
				_pImpl->preLog(severity, now);
			}
			~log_helper()
			{
				_pImpl->lastLog();
			}

			template<typename T> 
			inline log_helper& operator<<(const T& data)
			{
				_pImpl->stream() << data;
				return *this;
			}
			

		private:
			log_impl* _pImpl;
	};

	class logger
	{
		public:
			inline log_helper info()
			{
				return log_helper(&_impl, INFO);
			}

		private:
			log_impl _impl;
	};

}




#endif

