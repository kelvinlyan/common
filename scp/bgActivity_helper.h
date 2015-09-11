#pragma once

#include <list>
#include "block.h"
#include "game_helper.hpp"

namespace gg
{
	const static string bgActivity_history_data_db_str = "gl.bgActivity_history_data";

	namespace bgActivity
	{
		enum
		{
			_loading = 0,
			_running,
			_stopped
		};

		enum
		{
			_defeat_npc = 1,
			_npc_progress,
			_ruler_title,
			_arena_rank,
			_level_rank,
			_office_rank,
			_guild_rank,
			
			_max_type
		};

		class i_base
		{
			public:
				
				//virtual void initData(const Json::Value& info) = 0;
				virtual ~i_base(){}
				virtual void load() = 0;
				virtual void update(playerDataPtr d) = 0;
				virtual Json::Value getInfo(playerDataPtr d = playerDataPtr()) = 0;
			
				void setState(int state){ _state = state; }
				int getState() const { return _state; }

				int getType() const { return _type; };
				void setType(int type){ _type = type; }

				int getId() const { return _id; }
				void setId(int id){ _id = id; }

			private:
				int _id;
				int _type;
				int _state;
		};

		typedef boost::shared_ptr<i_base> basePtr;

#define BGACTIVITY_ERROR() \
	do \
	{ \
		LogE << "error: " << __FUNCTION__ << "(" << __FILE__ << ":" << __LINE__ << ")" << LogEnd; \
	} \
	while(false)

		class rankItem_1
		{
			public:
				int getId() const 
				{
					BGACTIVITY_ERROR();
					return -1; 
				}
				int getInterval() const
				{
					BGACTIVITY_ERROR();
					return -1; 
				}
				bool operator<(const rankItem_1& rhs) const 
				{
					BGACTIVITY_ERROR();
					return false; 
				} 
				Json::Value getInfo() const
				{
					BGACTIVITY_ERROR();
					return Json::nullValue;
				}
		};

		template<typename T, int N>
		class rankList_1
		{	
			public:
				typedef boost::function<void(const T&)> handler; 

				rankList_1();
				void setParam(unsigned max_size, unsigned client_size);
				void update(const T& item, int oInterval = -1);
				int getRank(int id, int interval);
				Json::Value getInfo();
				unsigned size() const { return _size; }
				const T& max() const;
				const T& min() const;

				void run(handler h);
				void print();

			private:
				bool erase(int id, int interval);
				void insert(const T& item);

				typedef std::list<T> rankItems;
				struct rankItemsWithCount
				{
					rankItems _items;
					unsigned _count;
				};
				
				rankItemsWithCount _rankInfo[N];
				unsigned _size;
				unsigned _max_size;
				unsigned _client_size;
				int _max_interval;
				int _min_interval;
		};

		class data_package
		{
			public:
				typedef std::map<int, Json::Value> pid_info;

				bool loadDB(int key_id);
				bool saveDB(int key_id);

				void insert(int player_id, Json::Value info);

				const Json::Value& getInfo() const { return _value; }
				const Json::Value& getPInfo(int player_id) const
				{
					pid_info::const_iterator iter = _pid_infos.find(player_id);
					if(iter == _pid_infos.end())
						return _null;
					return iter->second;
				}

			private:
				static Json::Value _null;
				pid_info _pid_infos;

				Json::Value _value;
		};

		class levelItem : public rankItem_1
		{
			public:
				levelItem(int player_id, const string& name, int lv, int exp)
					: _player_id(player_id), _name(name), _level(lv), _exp(exp){}
				int getId() const { return _player_id; }
				int getInterval() const { return _level; }
				unsigned getExp() const { return _exp; }
				void setLv(int lv) { _level = lv; }
				bool operator<(const levelItem& rhs) const { return _exp < rhs.getExp(); }
				Json::Value getInfo() const
				{
					Json::Value info;
					info.append(_player_id);
					info.append(_name);
					info.append(_level);
					return info;
				}

			private:
				int _player_id;
				string _name;
				int _level;
				unsigned _exp;
		};

		typedef levelItem officeItem;

		class defeat_npc : public i_base
		{
			public:
				defeat_npc(const Json::Value& info);
				virtual void load();
				virtual void update(playerDataPtr d);
				virtual Json::Value getInfo(playerDataPtr d);

				static void setNpcId(int id){ _current_npc_id = id; }

				void end_load();

			private:
				int getRank(int player_id);
				void loadDB();
				void stop();

				struct helper
				{
					bool operator<(const helper& rhs) const
					{
						if(_pass_time == rhs._pass_time)
							return _player_id < rhs._player_id;
						return _pass_time < rhs._pass_time;
					}
					int _player_id;
					unsigned _pass_time;
				};
				
				struct cacheItem
				{
					int _player_id;
					string _name;
				};

				typedef std::vector<cacheItem> cache_list;
				cache_list _cache_list;

				static int _current_npc_id;
				int _npc_id;
				unsigned _max_size;
				unsigned _client_size;
				unsigned _end_time;

				bool _rewarded;
				data_package _package;

				Json::Value _records;
				typedef std::map<int, int> playerId_ranks;
				playerId_ranks _playerId_ranks;

				Json::Value _reward_info;
				int _reward_type;
		};

		class npc_progress : public i_base
		{
			public:
				npc_progress(const Json::Value& info){}
				virtual void load(){}
				virtual void update(playerDataPtr d){}
				virtual Json::Value getInfo(playerDataPtr d){ return Json::nullValue; }
		};

		class ruler_title : public i_base
		{
			public:
				ruler_title(const Json::Value& info);
				virtual void load();
				virtual void update(playerDataPtr d);
				virtual Json::Value getInfo(playerDataPtr d);

			private:
				Json::Value _title_infos;
		};

		class arena_rank : public i_base
		{
			public:
				arena_rank(const Json::Value& info);
				virtual void load();
				virtual void update(playerDataPtr d);
				virtual Json::Value getInfo(playerDataPtr d);

				void stop();
				void end_load();

			private:
				unsigned _max_size;
				unsigned _client_size;
				unsigned _end_time;
				bool _rewarded;

				data_package _package;

				Json::Value _reward_info;
				int _reward_type;
		};

		class level_rank : public i_base
		{
			public:
				level_rank(const Json::Value& info);
				virtual void load();
				virtual void update(playerDataPtr d);
				virtual Json::Value getInfo(playerDataPtr d);
				void end_load();
				void run(const levelItem& item);
				
				static void setLv(int olv){ _oLv = olv; }

			private:
				void loadDB();
				void stop();

				typedef std::vector<levelItem> cache_list;
				cache_list _cache_list;
				rankList_1<levelItem, 101> _rankList;

				data_package _package;

				unsigned _end_time;

				bool _rewarded;
				Json::Value _reward_info;
				int _reward_type;

				string _name;

				int _rank_helper;
				std::vector<Json::Value> _reward_package;

				static int _oLv;
		};

		class office_rank : public i_base
		{
			public:
				office_rank(const Json::Value& info);
				virtual void load();
				virtual void update(playerDataPtr d);
				virtual Json::Value getInfo(playerDataPtr d);
				void end_load();

				void run(const officeItem& item);

				static void setLv(int lv){ _oLv = lv; }
				void loadDB();

			private:
				void stop();

				typedef std::vector<officeItem> cache_list;
				cache_list _cache_list;

				rankList_1<officeItem, 51> _rankList;

				data_package _package;

				unsigned _end_time;

				bool _rewarded;
				Json::Value _reward_info;
				int _reward_type;

				string _name;

				int _rank_helper;
				std::vector<Json::Value> _reward_package;

				static int _oLv;
		};

		class guild_rank : public i_base
		{
			public:
				guild_rank(const Json::Value& info);
				virtual void load();
				virtual void update(playerDataPtr d);
				virtual Json::Value getInfo(playerDataPtr d);

			private:
				bool _rewarded;
				data_package _package;
				unsigned _client_size;
				unsigned _max_size;
		};

		static inline basePtr create(const Json::Value& info)
		{
			int type = info["t"].asInt();
			switch(type)
			{
				case _defeat_npc:
					return creator<defeat_npc>::run(info);
				case _npc_progress:
					return creator<npc_progress>::run(info);
				case _ruler_title:
					return creator<ruler_title>::run(info);
				case _arena_rank:
					return creator<arena_rank>::run(info);
				case _level_rank:
					return creator<level_rank>::run(info);
				case _office_rank:
					return creator<office_rank>::run(info);
				case _guild_rank:
					return creator<guild_rank>::run(info);
				default:
					return basePtr();
			}
		}
	}
}
