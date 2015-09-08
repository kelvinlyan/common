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
			typedef boost::function<void()> func;
			static bgActivity_system* const bgActivitySys;

			bgActivity_system();

			void initData();

			void infoReq(msg_json& m, Json::Value& r);
			void updateReq(msg_json& m, Json::Value& r);

			void gmInfoReq(msg_json& m, Json::Value& r);
			void gmModifyReq(msg_json& m, Json::Value& r);

			void threadCallBack(msg_json& m, Json::Value& r);
			void functionCall(msg_json& m, Json::Value& r);

			void startActivities(int key_id);
			void stopActivities();

			void update(playerDataPtr d, int type);
			void setFunc(func f);

		private:
			void doStartActivities();
			void loadDB();
			void saveDB();

		private:
			func _func;

			thread_ptr _thread_ptr;
			
			typedef std::pair<int, int> key_action;
			typedef std::list<key_action> key_actions;
			key_actions _key_actions_cache;

			typedef std::map<int, Json::Value> key_value;
			key_value _activity_values;

			Json::Value _current_activity_value;

			bgActivityList _activities[bgActivity::_max_type];
	};
}
