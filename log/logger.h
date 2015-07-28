#ifndef _LOG_LOGGER_H
#define _LOG_LOGGER_H

#include "implement.h"
#include "writer.h"

namespace nLog
{
	class helper
	{
		public:
			helper(implement* ptr)
				: _impl_ptr(ptr)
			{
				_impl_ptr->preLog();
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
			inline helper info()
			{
				return helper(&_impl);
			}
			void addLinker(iLinker* ptr)
			{
				_impl.addLinker(ptr);
			}

			sharedData& getSharedData()
			{
				return _impl.getSharedData();
			}
		private:
			implement _impl;
	};
}

#endif
