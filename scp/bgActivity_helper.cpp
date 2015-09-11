#include "bgActivity_helper.h"
#include "arena_system.h"
#include "game_server.h"
#include "playerManager.h"
#include "player_war_story.h"
#include "bgActivity_system.h"
#include "timmer.hpp"
#include "commom.h"
#include "player_office.h"
#include "guild_system.h"

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
			bool found = false;
			int rank = r._count + 1;
			ForEach(typename rankItems, iter, r._items)
			{
				if(iter->getId() != id)
					++rank;
				else
				{
					found = true;
					break;
				}
			}
			if(!found)
				rank = -1;
			else
				rank = rank > _max_size? -1 : rank;

			return rank;
		}

		template<typename T, int N>
		Json::Value rankList_1<T, N>::getInfo()
		{
			print();
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

		template<typename T, int N>
		void rankList_1<T, N>::run(handler h)
		{
			for(int i = N - 1; i >= 0; --i)
			{
				rankItemsWithCount& rk = _rankInfo[i];
				ForEach(typename rankItems, iter, rk._items)
					h(*iter);
			}
		}

		template<typename T, int N>
		void rankList_1<T, N>::print()
		{
			for(int i = N - 1; i >= 0; --i)
			{
				rankItemsWithCount& rk = _rankInfo[i];
				std::cout << "Interval(" << i << ") Count(" << rk._count << "): ";
				ForEach(typename rankItems, iter, rk._items)
				{
					std::cout << iter->getId() << ", ";
				}
				std::cout << "End." << std::endl;
			}
		}


		Json::Value data_package::_null = Json::nullValue;

		bool data_package::loadDB(int key_id)
		{
			db_manager db;
			if(!db.connect_db(game_svr->getGGData()._mongoDB.c_str()))
				return false;
			
			mongo::BSONObj key = BSON("i" << key_id);
			mongo::BSONObj obj = db_mgr.FindOne(bgActivity_history_data_db_str, key);
			if(obj.isEmpty())
				return false;
			
			checkNotEoo(obj["a"])
			{
				string str = obj["a"].String();
				_value = commom_sys.string2json(str);
			}
			
			checkNotEoo(obj["p"])
			{
				std::vector<mongo::BSONElement> ele = obj["p"].Array();
				for(unsigned i = 0; i < ele.size(); ++i)
				{
					int player_id = ele[i]["k"].Int();
					string str = ele[i]["v"].String();
					Json::Value temp = commom_sys.string2json(str);
					_pid_infos.insert(make_pair(player_id, temp));
				}
			}

			return true;
		}

		bool data_package::saveDB(int key_id)
		{
			mongo::BSONObj key = BSON("i" << key_id);
			mongo::BSONObjBuilder obj;
			obj << "i" << key_id << "a" << _value.toStyledString();
			mongo::BSONArrayBuilder b;
			ForEach(pid_info, iter, _pid_infos)
				b.append(iter->second.toStyledString());
			obj << "p" << b.arr();
			
			return db_mgr.save_mongo(bgActivity_history_data_db_str, key, obj.obj());
		}

		void data_package::insert(int player_id, Json::Value info)
		{
			if(player_id == -1)
				_value = info;
			else
				_pid_infos.insert(make_pair(player_id, info));
		}
		
		int defeat_npc::_current_npc_id = -1;

		defeat_npc::defeat_npc(const Json::Value& info)
			: _npc_id(999999), _max_size(10), _client_size(10), _end_time(0), _reward_type(0)
		{
			setType(_defeat_npc);
			setId(info["k"].asUInt());

			if(info["i"] != Json::nullValue)
				_npc_id = info["i"].asInt();
			
			if(info["m"] != Json::nullValue) 
				_max_size = info["m"].asUInt();

			if(info["s"] != Json::nullValue)
				_client_size = info["s"].asUInt();

			if(info["e"] != Json::nullValue)
				_end_time = info["e"].asUInt();

			if(info["r"] != Json::nullValue)
				_reward_info = info["r"];

			if(info["c"] != Json::nullValue)
				_reward_type = info["c"].asInt();

			Timer::AddEventTickTime(boostBind(defeat_npc::stop, this), _end_time);
			setState(_loading);
		}

		void defeat_npc::loadDB()
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

		void defeat_npc::load()
		{
			_rewarded = _package.loadDB(getId());

			if(!_rewarded)
				loadDB();

			_Logic_Post(boostBind(defeat_npc::end_load, this));
		}

		void defeat_npc::end_load()
		{
			try
			{
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

				if(na::time_helper::get_current_time() >= _end_time)
					stop();
			}
			catch(string& error)
			{
				LogE << "Error: " << __FUNCTION__ << " (" << error << ")" << LogEnd;
			}
		}

		void defeat_npc::stop()
		{
			if(getState() != _running)
				return;

			if(!_rewarded)
			{
				_package.insert(-1, _records);
				ForEach(playerId_ranks, iter, _playerId_ranks)
				{
					Json::Value temp = Json::arrayValue;
					temp.append(iter->second);
					_package.insert(iter->first, temp);
				}
				_package.saveDB(getId());


				//todo reward
			}
			setState(_stopped);
		}

		void defeat_npc::update(playerDataPtr d)
		{
			if(getState() == _stopped)
				return;

			if(_current_npc_id != _npc_id)
				return;

			if(getState() == _loading)
			{
				if(na::time_helper::get_current_time() < _end_time)
				{
					cacheItem item = { d->playerID, d->Base.getName() };
					_cache_list.push_back(item);
				}
				return;
			}

			if(_playerId_ranks.size() >= _max_size)
				return;

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
			if(getState() == _loading)
				return Json::nullValue;

			Json::Value info;
			info["k"] = getId();
			if(_rewarded)
			{
				info["l"] = _package.getInfo();
				info["p"] = _package.getPInfo(d->playerID);
				if(info["p"] == Json::nullValue)
					info["p"] = Json::arrayValue;
			}
			else
			{
				info["l"] = _records;
				info["p"] = Json::arrayValue;
				info["p"].append(getRank(d->playerID));
			}
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
			setState(_loading);
		}

		void ruler_title::load()
		{
			
		}

		void ruler_title::update(playerDataPtr d)
		{
		
		}

		Json::Value ruler_title::getInfo(playerDataPtr d)
		{
			return Json::nullValue;
		}
		
		arena_rank::arena_rank(const Json::Value& info)
			: _end_time(0), _max_size(20), _client_size(20)
		{
			setType(_arena_rank);
			setId(info["k"].asUInt());

			if(info["m"] != Json::nullValue)
				_max_size = info["m"].asUInt();
			if(info["s"] != Json::nullValue)
				_client_size = info["s"].asUInt();
			if(info["e"] != Json::nullValue)
				_end_time = info["e"].asUInt();

			if(info["r"] != Json::nullValue)
				_reward_info = info["r"];

			if(info["c"] != Json::nullValue)
				_reward_type = info["c"].asInt();

			Timer::AddEventTickTime(boostBind(arena_rank::stop, this), _end_time);
			setState(_loading);
		}

		void arena_rank::load()
		{
			_rewarded = _package.loadDB(getId());

			_Logic_Post(boostBind(arena_rank::end_load, this));
		}

		void arena_rank::end_load()
		{
			setState(_running);
			if(na::time_helper::get_current_time() >= _end_time)
				stop();
		}

		void arena_rank::stop()
		{
			if(getState() != _running)
				return;

			if(!_rewarded)
			{
				
			}	
			setState(_stopped);
		}

		void arena_rank::update(playerDataPtr d)
		{
			return;
		}

		Json::Value arena_rank::getInfo(playerDataPtr d)
		{
			if(getState() == _loading)
				return Json::nullValue;
			
			Json::Value info;
			info["k"] = getId();
			if(_rewarded)
			{
				info["l"] = _package.getInfo();
				info["p"] = _package.getPInfo(d->playerID);
				if(info["p"] == Json::nullValue)
					info["p"] = Json::arrayValue;
			}
			else
			{
				int count = 0;
				info["l"] = Json::arrayValue;
				Json::Value& ref = info["l"];
				const arena_system::HeroInfos& hero_infos = arena_sys.getAllRankList();
				for(unsigned i = 1; i < hero_infos.size(); ++i)
				{
					if(count >= _client_size)
						break;
					Json::Value temp;
					temp.append(hero_infos[i].get_player_id());
					temp.append(hero_infos[i].get_nickname());
					ref.append(temp);
				}
				info["p"] = Json::arrayValue;
				info["p"].append(d->Arena.getRank());
			}
			return info;
		}
		
		int level_rank::_oLv = -1;

		level_rank::level_rank(const Json::Value& info)
			: _end_time(0)
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

			if(info["e"] != Json::nullValue)
				_end_time = info["e"].asUInt();

			if(info["r"] != Json::nullValue)
				_reward_info = info["r"];

			if(info["c"] != Json::nullValue)
				_reward_type = info["c"].asInt();

			Timer::AddEventTickTime(boostBind(level_rank::stop, this), _end_time);
			setState(_loading);
		}

		void level_rank::load()
		{
			_rewarded = _package.loadDB(getId());
			if(!_rewarded)
				loadDB();

			_Logic_Post(boostBind(level_rank::end_load, this));
		}

		void level_rank::loadDB()
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
		}

		void level_rank::end_load()
		{
			ForEach(cache_list, iter, _cache_list)
				_rankList.update(*iter);

			setState(_running);
			if(na::time_helper::get_current_time() >= _end_time)
				stop();
		}

		void level_rank::stop()
		{
			if(getState() != _running)
				return;

			if(!_rewarded)
			{
				_rank_helper = 0;
				_reward_package.clear();
				_reward_package.assign(_reward_info.size(), Json::nullValue);

				_rankList.run(boostBind(level_rank::run, this, _1));

				for(unsigned i = 0; i < _reward_package.size(); ++i)
				{
					Json::Value msg;
					msg[msgStr] = Json::arrayValue;
					msg[msgStr].append(_reward_package[i]);
					msg[msgStr].append(_reward_info[i][1u]);
					string str = msg.toStyledString();
					na::msg::msg_json m(gate_client::bgActivity_reward_inner_req, str);
					player_mgr.postMessage(m);
				}

				_rewarded = true;
				_package.saveDB(getId());
			}
			setState(_stopped);
		}

		void level_rank::update(playerDataPtr d)
		{
			if(getState() == _stopped)
				return;

			if(getState() == _loading)
			{
				if(na::time_helper::get_current_time() < _end_time)
				{
					levelItem item(d->playerID, d->Base.getName(), d->Base.getLevel(), d->Base.getExp());
					_cache_list.push_back(item);
				}
				return;
			}

			levelItem item(d->playerID, d->Base.getName(), d->Base.getLevel(), d->Base.getExp());
			_rankList.update(item, _oLv);
		}

		Json::Value level_rank::getInfo(playerDataPtr d)
		{
			if(getState() == _loading)
				return Json::nullValue;

			Json::Value info;
			info["k"] = getId();
			if(_rewarded)
			{
				info["l"] = _package.getInfo();
				info["p"] = _package.getPInfo(d->playerID);
				if(info["p"] == Json::nullValue)
					info["p"] = Json::arrayValue;
			}
			else
			{
				info["l"] = _rankList.getInfo();
				info["p"] = Json::arrayValue;
				info["p"].append(_rankList.getRank(d->playerID, d->Base.getLevel()));
			}
			return info;
		}

		void level_rank::run(const levelItem& item)
		{
			++_rank_helper;
			int rank = _rank_helper;
			
			for(unsigned i = 0; i < _reward_info.size(); ++i)
			{
				Json::Value& range = _reward_info[i][0u];
				if(rank >= range[0u].asInt() && rank <= range[1u].asInt())
				{
					Json::Value content;
					content["x"] = _name;
					content["y"] = rank;

					Json::Value info;
					info["i"] = item.getId();
					info["c"] = content;
					
					_reward_package[i].append(info);
				}
			}
		}

		int office_rank::_oLv = -1;

		office_rank::office_rank(const Json::Value& info)
			: _end_time(0)
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

			if(info["e"] != Json::nullValue)
				_end_time = info["e"].asUInt();

			if(info["r"] != Json::nullValue)
				_reward_info = info["r"];

			if(info["c"] != Json::nullValue)
				_reward_type = info["c"].asInt();

			Timer::AddEventTickTime(boostBind(office_rank::stop, this), _end_time);
			setState(_loading);
		}
		
		void office_rank::loadDB()
		{
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
		}
		
		void office_rank::load()
		{
			_rewarded = _package.loadDB(getId());
			if(!_rewarded)
				loadDB();

			_Logic_Post(boostBind(office_rank::end_load, this));
		}

		void office_rank::end_load()
		{
			ForEach(cache_list, iter, _cache_list)
				_rankList.update(*iter);

			setState(_running);
			if(na::time_helper::get_current_time() >= _end_time)
				stop();
		}

		void office_rank::stop()
		{
			if(getState() != _running)
				return;

			if(!_rewarded)
			{
				_rank_helper = 0;
				_reward_package.clear();
				_reward_package.assign(_reward_info.size(), Json::nullValue);

				_rankList.run(boostBind(office_rank::run, this, _1));

				for(unsigned i = 0; i < _reward_package.size(); ++i)
				{
					Json::Value msg;
					msg[msgStr] = Json::arrayValue;
					msg[msgStr].append(_reward_package[i]);
					msg[msgStr].append(_reward_info[i][1u]);
					string str = msg.toStyledString();
					na::msg::msg_json m(gate_client::bgActivity_reward_inner_req, str);
					player_mgr.postMessage(m);
				}

				_rewarded = true;
				_package.saveDB(getId());
				
			}
			setState(_stopped);
		}

		void office_rank::run(const officeItem& item)
		{
			++_rank_helper;
			int rank = _rank_helper;
			
			for(unsigned i = 0; i < _reward_info.size(); ++i)
			{
				Json::Value& range = _reward_info[i][0u];
				if(rank >= range[0u].asInt() && rank <= range[1u].asInt())
				{
					Json::Value content;
					content["x"] = _name;
					content["y"] = rank;

					Json::Value info;
					info["i"] = item.getId();
					info["c"] = content;
					
					_reward_package[i].append(info);
				}
			}
		}

		void office_rank::update(playerDataPtr d)
		{
			if(getState() == _stopped)
				return;

			if(getState() == _loading)
			{
				if(na::time_helper::get_current_time() < _end_time)
				{
					officeItem item(d->playerID, d->Base.getName(), d->Office.getOffcialLevel(), d->Base.getWeiWang());
					_cache_list.push_back(item);
				}
				return;
			}

			officeItem item(d->playerID, d->Base.getName(), d->Office.getOffcialLevel(), d->Base.getWeiWang());
			_rankList.update(item, _oLv);
		}

		Json::Value office_rank::getInfo(playerDataPtr d)
		{
			if(getState() == _loading)
				return Json::nullValue;

			Json::Value info;
			info["k"] = getId();
			if(_rewarded)
			{
				info["l"] = _package.getInfo();
				info["p"] = _package.getPInfo(d->playerID);
				if(info["p"] == Json::nullValue)
					info["p"] = Json::arrayValue;
			}
			else
			{
				info["l"] = _rankList.getInfo();
				info["p"] = Json::arrayValue;
				info["p"].append(_rankList.getRank(d->playerID, d->Office.getOffcialLevel()));
			}
			return info;
		}

		guild_rank::guild_rank(const Json::Value& info)
		{
			setType(_guild_rank);
			setId(info["k"].asUInt());
		}

		void guild_rank::load()
		{
			_rewarded == _package.loadDB(getId());		
		}

		void guild_rank::update(playerDataPtr d)
		{
			return;
		}

		Json::Value guild_rank::getInfo(playerDataPtr d)
		{
			Json::Value info;
			info["g"] = guild_sys.packageGuild(_client_size, d->Base.getSphereID());
			info["p"] = Json::arrayValue;
			int gid = d->Guild.getGuildID();
			Guild::guildPtr gptr = guild_sys.getGuild(gid);
			if(gptr != Guild::guildPtr())
			{
				int rk = 0;
				for(; rk < info["g"].size(); ++rk)
				{
					if(info["g"][rk][0u].asInt() == gid)
						break;
				}
				if(rk != info["g"].size())
				{
					info["p"].append(gptr->guildName);
					info["p"].append(rk + 1);
				}
			}

			return info;
		}
	}
}
