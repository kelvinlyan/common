#include "bgActivity_system.h"
#include "playerManager.h"
#include "game_helper.hpp"
#include "response_def.h"
#include "commom.h"
#include "gm_tools.h"
#include "timmer.hpp"

namespace gg
{
	bgActivity_system* const bgActivity_system::bgActivitySys = new bgActivity_system;

	bgActivity_system::bgActivity_system()
	{
		_current_activity_value = Json::nullValue;
	}

	void bgActivity_system::initData()
	{
		loadDB();
	}

	void bgActivity_system::infoReq(msg_json& m, Json::Value& r)
	{
		r[msgStr][1u] = _current_activity_value;
		Return(r, 0);
	}

	void bgActivity_system::updateReq(msg_json& m, Json::Value& r)
	{
		AsyncGetPlayerData(m);
		ReadJsonArray;
		int id = js_msg[0u].asInt();
		for(unsigned i = 0; i < bgActivity::_max_type; ++i)
		{
			ForEach(bgActivityList, iter, _activities[i])
			{
				if((*iter)->getId() == id)
				{
					r[msgStr][1u] = (*iter)->getInfo(d);
					Return(r, 0);
				}
			}
		}
		r[msgStr][1u] = id;
		Return(r, 1);
	}

	void bgActivity_system::gmInfoReq(msg_json& m, Json::Value& r)
	{
		ReadJsonArray;
		int key_id = js_msg[1u].asInt();

		key_value::iterator iter = _activity_values.find(key_id);
		if(iter == _activity_values.end())
		{
			r[msgStr][1u] = key_id;
			Return(r, 1);
		}

		r[msgStr][1u] = iter->second;
		Return(r, 0);
	}

	void bgActivity_system::gmModifyReq(msg_json& m, Json::Value& r)
	{
		ReadJsonArray;
		const Json::Value& info = js_msg[1u];
		int key_id = info["kid"].asInt();
		unsigned begin_time = info["bt"].asUInt() + na::time_helper::timeZone() * na::time_helper::HOUR;
		unsigned end_time = info["et"].asUInt() + na::time_helper::timeZone() * na::time_helper::HOUR;

		Timer::AddEventTickTime(boostBind(bgActivity_system::startActivities, bgActivity_system::bgActivitySys, key_id), begin_time);
		Timer::AddEventTickTime(boostBind(bgActivity_system::stopActivities, bgActivity_system::bgActivitySys), end_time);

		_activity_values.insert(make_pair(key_id, info));

		saveDB();

		Return(r, 0);
	}

	void bgActivity_system::startActivities(int key_id)
	{
		key_value::iterator iter = _activity_values.find(key_id);
		if(iter == _activity_values.end())
			return;

		_current_activity_value = iter->second;

		_current_activity_value["bt"] = _current_activity_value["bt"].asUInt() + na::time_helper::timeZone() * na::time_helper::HOUR;
		_current_activity_value["et"] = _current_activity_value["et"].asUInt() + na::time_helper::timeZone() * na::time_helper::HOUR;

		const Json::Value& page = _current_activity_value["pg"];
		for(unsigned i = 0; i < page.size(); ++i)
		{
			bgActivity::basePtr ptr = bgActivity::create(page[i]);
			if(ptr == bgActivity::basePtr())
				continue;
			ptr->setState(bgActivity::_loading);
			_activities[ptr->getType()].push_back(ptr);
		}

		_thread_ptr = creator<boost::thread>::run(boostBind(bgActivity_system::doStartActivities, this));
	}

	void bgActivity_system::stopActivities()
	{
		_current_activity_value = Json::nullValue;

		for(unsigned i = 0; i < bgActivity::_max_type; ++i)
			_activities[i].clear();
	}

	void bgActivity_system::doStartActivities()
	{
		sleep(10);
		for(unsigned i = 0; i < bgActivity::_max_type; ++i)
		{
			ForEach(bgActivityList, iter, _activities[i])
				(*iter)->start();
		}

		string str;
		na::msg::msg_json m(gate_client::bgActivity_thread_callback_inner_req, str);
		player_mgr.postMessage(m);
	}

	void bgActivity_system::threadCallBack(msg_json& m, Json::Value& r)
	{
		_thread_ptr->join();
		_thread_ptr.reset();
	}

	void bgActivity_system::functionCall(msg_json& m, Json::Value& r)
	{
		_func();
	}

	void bgActivity_system::loadDB()
	{
		objCollection objs = db_mgr.Query(bgActivity_info_list_db_str);
		if(!objs.empty())
		{
			const mongo::BSONObj& obj = objs.front();
			checkNotEoo(obj["v"])
			{
				vector<mongo::BSONElement> ele = obj["v"].Array();
				for(unsigned i = 0; i < ele.size(); ++i)
				{
					string str = ele[i].String();
					Json::Value info = commom_sys.string2json(str);
					int key_id = info["kid"].asInt();
					unsigned begin_time = info["bt"].asUInt() + na::time_helper::timeZone() * na::time_helper::HOUR;
					unsigned end_time = info["et"].asUInt() + na::time_helper::timeZone() * na::time_helper::HOUR;

					Timer::AddEventTickTime(boostBind(bgActivity_system::startActivities, bgActivity_system::bgActivitySys, key_id), begin_time);
					Timer::AddEventTickTime(boostBind(bgActivity_system::stopActivities, bgActivity_system::bgActivitySys), end_time);

					_activity_values.insert(make_pair(key_id, info));
				}
			}
		}
	}

	void bgActivity_system::saveDB()
	{
		mongo::BSONObj key = BSON("i" << 0);
		mongo::BSONObjBuilder obj;
		obj << "i" << 0;
		if(!_activity_values.empty())
		{
			mongo::BSONArrayBuilder array_b;
			ForEach(key_value, iter, _activity_values)
				array_b.append(iter->second.toStyledString().c_str());
			obj << "v" << array_b.arr();
		}
		db_mgr.save_mongo(bgActivity_info_list_db_str, key, obj.obj());
	}

	void bgActivity_system::setFunc(func f)
	{
		_func = f;
	}
}
