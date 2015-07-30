#ifndef _LOG_LOGGER_H
#define _LOG_LOGGER_H

#include "implement.h"

namespace nLog
{
	class helper
	{
		public:
			helper(implement* ptr, int severity)
				: _impl_ptr(ptr)
			{
				_impl_ptr->preLog(severity);
			}
			~helper()
			{
				_impl_ptr->doLog();
			}
			inline helper& operator<<(const char* pstr)
			{
				_impl_ptr->operator<<(pstr);
				return *this;
			}

		private:
			implement* _impl_ptr;
	};

	class logger
	{
		public:
			inline helper rcd(int severity = INFO)
			{
				return helper(&_impl, severity);
			}
			void push(iLink* ptr)
			{
				_impl.push(ptr);
			}
			const char* const* getSeverity() const { return _impl.getSeverityPtr(); }
		private:
			implement _impl;
	};
}

#endif
