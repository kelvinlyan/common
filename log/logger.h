#ifndef _LOG_LOGGER_H
#define _LOG_LOGGER_H

namespace nLog
{
	class logger
	{
		public:
			


		private:
			void tick()
			{
				_tickerMgr.tick();
			}

		private:
			tickerMgr _tickerMgr;
			
	};
}

#endif
