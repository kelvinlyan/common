#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include "bgActivity_helper.h"

#define bgActivity_sys (*gg::bgActivity_system::bgActivitySys)

namespace gg
{
	typedef boost::shared_ptr<boost::thread> thread_ptr;
	typedef std::vector<bgActivity::basePtr> bgActivityList;
	
	const static string bgActivity_info_list_db_str = "gl.bgActivity_info_list";

	class bgActivity_system
	{
		public:
			static bgActivity_system* const bgActivitySys;

			struct activity_value
			{
				bool operator<(const activity_value& rhs) const
				{
					return _begin_time < rhs._begin_time;
				}

				int _key_id;
				unsigned _begin_time;
				unsigned _end_time;
				int _timer_id;
				Json::Value _value;
			};

			bgActivity_system();

			void initData();

			void infoReq(msg_json& m, Json::Value& r);
			void updateReq(msg_json& m, Json::Value& r);

			void gmInfoReq(msg_json& m, Json::Value& r);
			void gmModifyReq(msg_json& m, Json::Value& r);

			void rewardInnerReq(msg_json& m, Json::Value& r);

			void update(playerDataPtr d, int type);

		private:
			void info(playerDataPtr d = playerDataPtr());
			void startActivities(int key_id, int timer_id);
			void stopActivities(int timer_id);
			void threadCallBack();
			void doStartActivities();
			void loadDB();
			void saveDB();
			bool checkTime(unsigned begin_time, unsigned end_time, int key_id);

		private:
			thread_ptr _thread_ptr;

			typedef boost::function<void()> func;
			typedef std::list<func> func_list;
			func_list _func_list;
			bool _async;
			
			typedef std::map<int, activity_value> key_value;
			key_value _activity_values;

			activity_value _current_activity_value;

			bgActivityList _activities[bgActivity::_max_type];

			static int _timer_id;
	};
}
