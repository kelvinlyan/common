#ifndef _LOG_LOGGER_H
#define _LOG_LOGGER_H

#include "helper.h"

namespace nLog
{
	class logger
	{
		public:
			inline helper info()
			{
				return helper(&_log_impl);
			}
			void addLinker(iLinker* ptr)
			{
				_log_impl.addLinker(ptr);
			}
		private:
			log_impl _log_impl;
	};
}

#endif
