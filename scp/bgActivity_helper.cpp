#include "bgActivity_helper.h"
#include "game_server.h"
#include "playerManager.h"
#include "player_war_story.h"
#include "bgActivity_system.h"
#include "timmer.hpp"
#include "commom.h"
#include "player_office.h"

namespace gg
{
	namespace bgActivity
	{
		template<typename T, int N>
		rankList_1<T, N>::rankList_1()
		{
			_size = 0;
		}

		template<typename T, int N>
		void rankList_1<T, N>::setParam(unsigned max_size, unsigned client_size)
		{
			_max_size = max_size;
			_client_size = client_size;
		}

		template<typename T, int N>
		bool rankList_1<T, N>::erase(int id, int interval)
		{
			rankItemsWithCount& r = _rankInfo[interval];
			ForEach(typename rankItems, iter, r._items)
			{
				if(iter->getId() == id)
				{
					r._items.erase(iter);
					return true;
				}
			}
			return false;
		}

		template<typename T, int N>
		void rankList_1<T, N>::insert(const T& item)
		{
			rankItemsWithCount& r = _rankInfo[item.getInterval()];
			ForEach(typename rankItems, iter, r._items)
			{
				if(*iter < item)
				{
					r._items.insert(iter, item);
					return;
				}
			}
			r._items.push_back(item);
		}

		template<typename T, int N>
		const T& rankList_1<T, N>::max() const
		{
			return _rankInfo[_max_interval].front();
		}

		template<typename T, int N>
		const T& rankList_1<T, N>::min() const
		{
			return _rankInfo[_min_interval].back();
		}

		template<typename T, int N>
		void rankList_1<T, N>::update(const T& item, int oInterval)
		{
			if(item.getInterval() < 0 || item.getInterval() >= N)
				return;

			//if(_size >= _max_size && item < min())
			//	return;

			bool erased = false;

			if(oInterval >= 0 && oInterval < N)
				erased = erase(item.getId(), oInterval);
			else
			{
				for(unsigned i = 0; i < N; ++i)
				{
					if(erased = erase(item.getId(), i))
					{
						oInterval = i;
						break;
					}
				}
			}

			insert(item);

			if(!erased)
				++_size;

			int begin = erased? oInterval : 0;
			int end = item.getInterval();
			while(begin < end)
				++_rankInfo[begin++]._count;
		}
		
		template<typename T, int N>
		int rankList_1<T, N>::getRank(int id, int interval)
		{
			if(interval < 0 || interval >= N)
				return -1;

			rankItemsWithCount& r = _rankInfo[interval];
			int rank = r._count + 1;
			ForEach(typename rankItems, iter, r._items)
			{
				if(iter->getId() != id)
					++rank;
				else
					break;
			}
			return rank > _max_size? -1 : _max_size;
		}

		template<typename T, int N>
		Json::Value rankList_1<T, N>::getInfo()
		{
			Json::Value rank_list = Json::arrayValue;
			unsigned count = 0;
			for(int i = N - 1; i >= 0; --i)
			{
				if(count >= _client_size)
					break;

				rankItemsWithCount& r = _rankInfo[i];	
				if(r._items.empty())
					continue;

				ForEach(typename rankItems, iter, r._items)
				{
					if(count >= _client_size)
						break;
					rank_list.append(iter->getInfo());
				}
			}
			return rank_list;
		}
		
		int defeat_npc::_current_npc_id = -1;

		defeat_npc::defeat_npc(const Json::Value& info)
			: _npc_id(999999), _max_size(10), _client_size(10), _end_time(0), _rewarded(false)
		{
			setType(_defeat_npc);
			setState(_stopped);
			setId(info["k"].asUInt());

			if(info["i"] != Json::nullValue)
				_npc_id = info["i"].asInt();
			
			if(info["m"] != Json::nullValue) 
				_max_size = info["m"].asUInt();

			if(info["s"] != Json::nullValue)
				_client_size = info["s"].asUInt();

			if(info["e"] != Json::nullValue)
				_end_time = info["e"].asUInt();

			if(info["b"] != Json::nullValue)
				_rewarded = info["b"].asBool();
		}
		
		void defeat_npc::loadDB_1()
		{
			db_manager db;
			if(!db.connect_db(game_svr->getGGData()._mongoDB.c_str()))
				return;
			
			mongo::BSONObj key = BSON("i" << getId());
			mongo::BSONObj obj = db_mgr.FindOne(bgActivity_history_data_db_str, key);
			if(obj.isEmpty())
				return;
			
			checkNotEoo(obj["l"])
			{
				string str = obj["l"].String();
				_records = commom_sys.string2json(str);
			}
			
			checkNotEoo(obj["p"])
			{
				std::vector<mongo::BSONElement> ele = obj["p"].Array();
				for(unsigned i = 0; i < ele.size(); ++i)
				{
					int player_id = ele[i]["i"].Int();
					int rank = ele[i]["r"].Int();
					_playerId_ranks.insert(make_pair(player_id, rank));
				}
			}
		}

		void defeat_npc::loadDB_2()
		{
			db_manager db;
			if(!db.connect_db(game_svr->getGGData()._mongoDB.c_str()))
				return;

			typedef std::set<defeat_npc::helper> pass_time_map;
			pass_time_map pass_times;
			typedef std::map<int, string> name_map;
			name_map names;

			objCollection objs;
			string db_name = playerWarStoryDBStr + boost::lexical_cast<string, int>(_npc_id / 100);
			objs = db.Query(db_name);
			for(unsigned i = 0; i < objs.size(); ++i)
			{
				int player_id = objs[i][playerIDStr].Int();
				unsigned pass_time;
				bool found = false;
				checkNotEoo(objs[i][playerWarStoryDataStr])	
				{
					vector<mongo::BSONElement> eles = objs[i][playerWarStoryDataStr].Array();
					for(unsigned j = 0; j < eles.size(); ++j)
					{
						int npc_id = eles[j][playerWarStoryIDStr].Int();
						if(npc_id == _npc_id)
						{
							found = true;
							if(!eles[j][playerWarStoryPastTimeStr].eoo())
								pass_time = eles[j][playerWarStoryPastTimeStr].Int();
							else
								pass_time = 0;
							break;
						}
					}
				}
				if(found)
				{
					helper h = { player_id, pass_time };
					pass_times.insert(h);
				}
			}

			objs = db.Query(playerBaseDBStr);
			for(unsigned i = 0; i < objs.size(); ++i)
			{
				int player_id = objs[i][playerIDStr].Int();
				const string& name = objs[i][playerNameStr].String();
				names.insert(make_pair(player_id, name));
			}

			int count = 0;
			for(pass_time_map::reverse_iterator iter = pass_times.rbegin(); iter != pass_times.rend(); ++iter)
			{
				if(count >= _max_size)
					break;

				name_map::iterator it = names.find(iter->_player_id);
				if(it == names.end())
					continue;

				Json::Value rcd;
				rcd.append(it->first);
				rcd.append(it->second);
				_records.append(rcd);

				_playerId_ranks.insert(make_pair(it->first, _records.size()));
			}
			
		}

		void defeat_npc::start()
		{
			if(_rewarded)
			{
				loadDB_1();
				setState(_stopped);
			}
			else
			{
				loadDB_2();
				Timer::AddEventTickTime(boostBind(defeat_npc::stop, this), _end_time);
				bgActivity_sys.setFunc(boostBind(defeat_npc::run, this));
				string str;
				na::msg::msg_json m(gate_client::bgActivity_function_call_inner_req, str);
				player_mgr.postMessage(m);
			}
		}

		void defeat_npc::run()
		{
			if(getState() == _stopped)
			{
				_cache_list.clear();
				return;
			}

			ForEach(cache_list, iter, _cache_list)
			{
				playerId_ranks::iterator it = _playerId_ranks.find(iter->_player_id);
				if(it == _playerId_ranks.end())
				{
					Json::Value rcd;
					rcd.append(iter->_player_id);
					rcd.append(iter->_name);
					_records.append(rcd);
					_playerId_ranks.insert(make_pair(iter->_player_id, _records.size()));
				}
			}

			_cache_list.clear();
			setState(_running);
		}

		void defeat_npc::stop()
		{
			setState(_stopped);
			// todo: reward
			_rewarded = true;
		}

		void defeat_npc::update(playerDataPtr d)
		{
			if(getState() == _stopped)
				return;

			if(_current_npc_id != _npc_id)
				return;

			if(getState() == _loading)
			{
				cacheItem item = { d->playerID, d->Base.getName() };
				_cache_list.push_back(item);
				return;
			}

			playerId_ranks::iterator iter = _playerId_ranks.find(d->playerID);
			if(iter == _playerId_ranks.end())
				return;

			Json::Value rcd;
			rcd.append(d->playerID);
			rcd.append(d->Base.getName());
			_records.append(rcd);
			_playerId_ranks.insert(make_pair(d->playerID, _records.size()));
		}

		Json::Value defeat_npc::getInfo(playerDataPtr d)
		{
			Json::Value info;
			info["k"] = getId();
			info["l"] = _records;
			info["p"] = Json::arrayValue;
			info["p"].append(getRank(d->playerID));
			return info;
		}

		int defeat_npc::getRank(int player_id)
		{
			playerId_ranks::iterator iter = _playerId_ranks.find(player_id);
			if(iter == _playerId_ranks.end())
				return -1;
			return iter->second;
		}	

		ruler_title::ruler_title(const Json::Value& info)
		{
			setType(_ruler_title);
			setId(info["k"].asUInt());
		}

		void ruler_title::start()
		{
			
		}

		void ruler_title::stop()
		{
			
		}

		void ruler_title::update(playerDataPtr d)
		{
		
		}

		Json::Value ruler_title::getInfo(playerDataPtr d)
		{
		
		}
		
		arena_rank::arena_rank(const Json::Value& info)
		{
			setType(_arena_rank);
			setId(info["k"].asUInt());
		}

		void arena_rank::start()
		{
			
		}

		void arena_rank::stop()
		{
			
		}

		void arena_rank::update(playerDataPtr d)
		{
		
		}

		Json::Value arena_rank::getInfo(playerDataPtr d)
		{
		
		}
		
		int level_rank::_oLv = -1;

		level_rank::level_rank(const Json::Value& info)
		{
			setType(_level_rank);
			setId(info["k"].asUInt());

			int max_size = 10;
			int client_size = 10;
			if(info["m"] != Json::nullValue)
				max_size = info["m"].asUInt();
			if(info["s"] != Json::nullValue)
				client_size = info["s"].asUInt();
			_rankList.setParam(max_size, client_size);
		}

		void level_rank::start()
		{	
			db_manager db;
			if(!db.connect_db(game_svr->getGGData()._mongoDB.c_str()))
				return;

			objCollection objs;
			objs = db.Query(playerBaseDBStr);
			for(unsigned i = 0; i < objs.size(); ++i)
			{
				int player_id = objs[i][playerIDStr].Int();
				const string& name = objs[i][playerNameStr].String();
				int lv = objs[i][playerLevelStr].Int();
				unsigned exp = objs[i][playerExpStr].Int();
				levelItem item(player_id, name, lv, exp);
				_rankList.update(item, lv);
			}

			bgActivity_sys.setFunc(boostBind(level_rank::run, this));
			string str;
			na::msg::msg_json m(gate_client::bgActivity_function_call_inner_req, str);
			player_mgr.postMessage(m);
		}
			
		void level_rank::run()
		{
			ForEach(cache_list, iter, _cache_list)
				_rankList.update(*iter);

			setState(_running);
		}

		void level_rank::stop()
		{
			setState(_stopped);		
		}

		void level_rank::update(playerDataPtr d)
		{
			if(getState() == _stopped)
				return;

			if(getState() == _loading)
			{
				levelItem item(d->playerID, d->Base.getName(), d->Base.getLevel(), d->Base.getExp());
				_cache_list.push_back(item);
				return;
			}

			levelItem item(d->playerID, d->Base.getName(), d->Base.getLevel(), d->Base.getExp());
			_rankList.update(item, _oLv);
		}

		Json::Value level_rank::getInfo(playerDataPtr d)
		{
		
		}

		int office_rank::_oLv = -1;

		office_rank::office_rank(const Json::Value& info)
		{
			setType(_office_rank);
			setId(info["k"].asUInt());

			int max_size = 10;
			int client_size = 10;
			if(info["m"] != Json::nullValue)
				max_size = info["m"].asUInt();
			if(info["s"] != Json::nullValue)
				client_size = info["s"].asUInt();
			_rankList.setParam(max_size, client_size);
		}

		void office_rank::start()
		{
			setState(_loading);
			sleep(5);

			db_manager db;
			if(!db.connect_db(game_svr->getGGData()._mongoDB.c_str()))
				return;

			objCollection objs;
			objs = db.Query(playerBaseDBStr);
			
			typedef std::map<int, officeItem> item_map;
			item_map items;
			
			for(unsigned i = 0; i < objs.size(); ++i)
			{
				int player_id = objs[i][playerIDStr].Int();
				const string& name = objs[i][playerNameStr].String();
				int weiwang = objs[i][playerWeiWangStr].Int();
				officeItem item(player_id, name, -1, weiwang);
				items.insert(make_pair(player_id, item));
			}

			objs = db.Query(office::officeDBStr);
			for(unsigned i = 0; i < objs.size(); ++i)
			{
				int player_id = objs[i][playerIDStr].Int();
				int level = objs[i][office::playerOfficialLevelStr].Int();
				item_map::iterator iter = items.find(player_id);
				if(iter != items.end())
					iter->second.setLv(level);
			}
			
			ForEach(item_map, iter, items)
				_rankList.update(iter->second, iter->second.getInterval());
			
			bgActivity_sys.setFunc(boostBind(office_rank::run, this));
			string str;
			na::msg::msg_json m(gate_client::bgActivity_function_call_inner_req, str);
			player_mgr.postMessage(m);
		}
			
		void office_rank::run()
		{
			ForEach(cache_list, iter, _cache_list)
				_rankList.update(*iter);

			setState(_running);
		}

		void office_rank::stop()
		{
			setState(_stopped);		
		}

		void office_rank::update(playerDataPtr d)
		{
			if(getState() == _stopped)
				return;

			if(getState() == _loading)
			{
				officeItem item(d->playerID, d->Base.getName(), d->Office.getOffcialLevel(), d->Base.getWeiWang());
				_cache_list.push_back(item);
				return;
			}

			officeItem item(d->playerID, d->Base.getName(), d->Office.getOffcialLevel(), d->Base.getWeiWang());
			_rankList.update(item, _oLv);
		}

		Json::Value office_rank::getInfo(playerDataPtr d)
		{
		
		}

		guild_rank::guild_rank(const Json::Value& info)
		{
			setType(_guild_rank);
			setId(info["k"].asUInt());
		}

		void guild_rank::start()
		{
			
		}

		void guild_rank::stop()
		{
			
		}

		void guild_rank::update(playerDataPtr d)
		{
		
		}

		Json::Value guild_rank::getInfo(playerDataPtr d)
		{
		
		}
	}
}
