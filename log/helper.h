#ifndef _LOG_HELPER_H
#ifndef _LOG_HELPER_H

namespace nLog
{
	class implement : public iLinker
	{
		public:
			inline void log()
			{
				_buff.clear();
				_tickerMgr.tick();
				run(_buff.c_str());
			}
			
			virtual const char* handle(const char* pstr)
			{
				return pstr;
			}

			implement& operator<<(const char* pstr)
			{
				_buff += pstr;
				return *this;
			}

		private:
			string _buff;
			tickerMgr _tickerMgr;
	};

	class helper
	{
		public:
			helper(implement* ptr, int severity)
				: _impl_ptr(ptr)
			{
			}
			~helper()
			{
				_impl_ptr->log();
			}
			inline helper& operator<<(const char* pstr)
			{
				(*_impl_ptr) << pstr;
			}

		private:
			implement* _impl_ptr;
	};
}

#endif

