#include "bgActivity_system.h"
#include "playerManager.h"
#include "game_helper.hpp"
#include "response_def.h"
#include "commom.h"
#include "gm_tools.h"
#include "timmer.hpp"
#include "email_system.h"

namespace gg
{
	bgActivity_system* const bgActivity_system::bgActivitySys = new bgActivity_system;
	int bgActivity_system::_timer_id = 0;

	bgActivity_system::bgActivity_system()
	{
		_current_activity_value._key_id = -1;
		_current_activity_value._value = Json::nullValue;
		_async = false;
	}

	void bgActivity_system::initData()
	{
		loadDB();
	}

	void bgActivity_system::infoReq(msg_json& m, Json::Value& r)
	{
		if(_async)
		{
			_func_list.push_back(boostBind(playerManager::postMessage, player_mgr, m));
			return;
		}

		r[msgStr][1u] = _current_activity_value._value;
		Return(r, 0);
	}

	void bgActivity_system::info(playerDataPtr d)
	{
		Json::Value msg;
		msg[msgStr][0u] = 0;
		msg[msgStr][1u] = _current_activity_value._value;
		if(d == playerDataPtr())
			player_mgr.sendToAll(gate_client::bgActivity_info_resp, msg);
		else
			player_mgr.sendToPlayer(d->playerID, d->netID, gate_client::bgActivity_info_resp, msg);
	}

	void bgActivity_system::updateReq(msg_json& m, Json::Value& r)
	{
		if(_async)
		{
			_func_list.push_back(boostBind(playerManager::postMessage, player_mgr, m));
			return;
		}

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
		if(_async)
		{
			_func_list.push_back(boostBind(playerManager::postMessage, player_mgr, m));
			return;
		}

		ReadJsonArray;
		int key_id = js_msg[1u].asInt();

		key_value::iterator iter = _activity_values.find(key_id);
		if(iter == _activity_values.end())
		{
			r[msgStr][1u] = key_id;
			Return(r, 1);
		}

		r[msgStr][1u] = iter->second._value;
		Return(r, 0);
	}

	void bgActivity_system::gmModifyReq(msg_json& m, Json::Value& r)
	{
		if(_async)
		{
			_func_list.push_back(boostBind(playerManager::postMessage, player_mgr, m));
			return;
		}

		ReadJsonArray;
		const Json::Value& info = js_msg[1u];
		int key_id = info["kid"].asInt();
		unsigned begin_time = info["bt"].asUInt() + na::time_helper::timeZone() * na::time_helper::HOUR;
		unsigned end_time = info["et"].asUInt() + na::time_helper::timeZone() * na::time_helper::HOUR;

		//DEBUGLOG(__FUNCTION__, " [ keyId, begin_time, end_time, current_time ]: ", key_id, begin_time, end_time, na::time_helper::get_current_time());

		if(info["et"].asUInt() != 0
				&& !checkTime(begin_time, end_time, key_id))
			Return(r, 1);
		
		if(_current_activity_value._key_id != -1 && _current_activity_value._key_id == key_id)
		{
			_current_activity_value._key_id = -1;
			_current_activity_value._value = Json::nullValue;
			for(unsigned i = 0; i < bgActivity::_max_type; ++i)
				_activities[i].clear();
		}

		if(info["et"].asUInt() != 0)
		{
			++_timer_id;
			Timer::AddEventTickTimeID(boostBind(bgActivity_system::startActivities, bgActivity_system::bgActivitySys, key_id, _timer_id), begin_time);
			Timer::AddEventTickTimeID(boostBind(bgActivity_system::stopActivities, bgActivity_system::bgActivitySys, _timer_id), end_time);

			activity_value value = { key_id, begin_time, end_time, _timer_id, info };
			_activity_values[key_id] = value;
		}

		saveDB();

		Return(r, 0);
	}

	bool bgActivity_system::checkTime(unsigned begin_time, unsigned end_time, int key_id)
	{
		if(begin_time >= end_time)
			return false;

		ForEach(key_value, iter, _activity_values)
		{
			if(iter->first == key_id)
				continue;

			unsigned target_begin_time =  iter->second._value["bt"].asUInt() + na::time_helper::timeZone() * na::time_helper::HOUR;
			unsigned target_end_time = iter->second._value["et"].asUInt() + na::time_helper::timeZone() * na::time_helper::HOUR;

			if(begin_time >= target_begin_time && begin_time <= target_end_time)
				return false;
			if(end_time >= target_begin_time && end_time <= target_end_time)
				return false;
			if(begin_time <= target_begin_time && end_time >= target_end_time)
				return false;
		}
		return true;
	}

	void bgActivity_system::startActivities(int key_id, int timer_id)
	{
		if(_async)
		{
			_func_list.push_back(boostBind(bgActivity_system::startActivities, this, key_id, timer_id));
			return;
		}

		//DEBUGLOG(__FUNCTION__, " [ keyId ]: ", key_id);

		key_value::iterator iter = _activity_values.find(key_id);
		if(iter == _activity_values.end())
			return;

		if(timer_id != iter->second._timer_id)
			return;

		_current_activity_value = iter->second;

		_current_activity_value._value["bt"] = _current_activity_value._value["bt"].asUInt() + na::time_helper::timeZone() * na::time_helper::HOUR;
		_current_activity_value._value["et"] = _current_activity_value._value["et"].asUInt() + na::time_helper::timeZone() * na::time_helper::HOUR;

		const Json::Value& page = _current_activity_value._value["pg"];
		for(unsigned i = 0; i < page.size(); ++i)
		{
			bgActivity::basePtr ptr = bgActivity::create(page[i]);
			if(ptr == bgActivity::basePtr())
				continue;
			ptr->setState(bgActivity::_loading);
			_activities[ptr->getType()].push_back(ptr);
		}

		_thread_ptr = creator<boost::thread>::run(boostBind(bgActivity_system::doStartActivities, this));
		_thread_ptr->detach();
		_async = true;
	}

	void bgActivity_system::stopActivities(int timer_id)
	{
		if(_async)
		{
			_func_list.push_back(boostBind(bgActivity_system::stopActivities, this, timer_id));
			return;
		}


		if(timer_id != _current_activity_value._timer_id)
			return;
		
		int key_id = _current_activity_value._key_id;
		_activity_values.erase(key_id);

		//DEBUGLOG(__FUNCTION__, " [ keyId ]: ", key_id);

		_current_activity_value._value = Json::nullValue;

		for(unsigned i = 0; i < bgActivity::_max_type; ++i)
			_activities[i].clear();

		saveDB();
		info();
	}

	void bgActivity_system::doStartActivities()
	{
		try
		{
			sleep(3);
			for(unsigned i = 0; i < bgActivity::_max_type; ++i)
			{
				ForEach(bgActivityList, iter, _activities[i])
					(*iter)->load();
			}

			_Logic_Post(boostBind(bgActivity_system::threadCallBack, this));
		}
		catch(string& error)
		{
			LogE << "Error: " << __FUNCTION__ << " (" << error << ")" << LogEnd;
		}
	}

	void bgActivity_system::threadCallBack()
	{
		try
		{
			_thread_ptr.reset();
			info();
			_async = false;
			while(!_func_list.empty())
			{
				if(_async)
					return;

				func f = _func_list.front();
				_func_list.pop_front();
				f();
			}
		}
		catch(string& error)
		{
			LogE << "Error: " << __FUNCTION__ << " (" << error << ")" << LogEnd;
		}
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
				vector<activity_value> sort_values;
				for(unsigned i = 0; i < ele.size(); ++i)
				{
					string str = ele[i].String();
					Json::Value info = commom_sys.string2json(str);
					int key_id = info["kid"].asInt();
					unsigned begin_time = info["bt"].asUInt() + na::time_helper::timeZone() * na::time_helper::HOUR;
					unsigned end_time = info["et"].asUInt() + na::time_helper::timeZone() * na::time_helper::HOUR;
					
					++_timer_id;
					activity_value value = { key_id, begin_time, end_time, _timer_id, info };
					sort_values.push_back(value);
				}

				std::sort(sort_values.begin(), sort_values.end());

				for(unsigned i = 0; i < sort_values.size(); ++i)
				{
					Timer::AddEventTickTime(boostBind(bgActivity_system::startActivities, bgActivity_system::bgActivitySys, sort_values[i]._key_id, sort_values[i]._timer_id), sort_values[i]._begin_time);
					Timer::AddEventTickTime(boostBind(bgActivity_system::stopActivities, bgActivity_system::bgActivitySys, sort_values[i]._timer_id), sort_values[i]._end_time);

					_activity_values.insert(make_pair(sort_values[i]._key_id, sort_values[i]));

	//				DEBUGLOG(__FUNCTION__, " [ key_id, begin_time, end_time ]: ", sort_values[i]._key_id, sort_values[i]._begin_time, sort_values[i]._end_time);
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
				array_b.append(iter->second._value.toStyledString().c_str());
			obj << "v" << array_b.arr();
		}
		db_mgr.save_mongo(bgActivity_info_list_db_str, key, obj.obj());
	}

	void bgActivity_system::update(playerDataPtr d, int type)
	{
		if(_current_activity_value._key_id == -1)
			return;

		if(type < bgActivity::_defeat_npc || type >= bgActivity::_max_type)
			return;

	//	DEBUGLOG(__FUNCION__, " [ player_id, type ]: ", d->playerID, type);

		ForEach(bgActivityList, iter, _activities[type])
			(*iter)->update(d);
	}

	void bgActivity_system::rewardInnerReq(msg_json& m, Json::Value& r)
	{
		ReadJsonArray;

		const Json::Value& player_list = js_msg[0u];

		vector<playerDataPtr> vec;
		for(unsigned i = 0; i < player_list.size(); ++i)
		{
			playerDataPtr d = player_mgr.getPlayerMain(player_list[i]["i"].asInt());
			if(d != playerDataPtr())
				vec.push_back(d);
		}

		if(vec.size() != player_list.size())
		{
			if(m._post_times < 2)
				player_mgr.postMessage(m);
			else
				LogE << "Error: " << __FUNCTION__ << LogEnd;
			return;
		}

		Json::Value reward = gm_tools_mgr.simple2complex(js_msg[1u]);
		
		for(unsigned i = 0; i < vec.size(); ++i)
			email_sys.sendSystemEmailCommon(vec[i], email_type_system_attachment, email_team_system_bgActivity_reward, player_list[i]["c"], reward);
	}

}
