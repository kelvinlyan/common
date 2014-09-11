#ifndef _COMMON_DATABASE_MONGO_H
#define _COMMON_DATABASE_MONGO_H

#include <mongo/client/dbclient.h>
#include <mongo/bson/bson.h>
#include "singleton.h"
#include <string>

using namespace std;

typedef vector<mongo::BSONObj> BSONObjVec;

class database_mongo : public singleton<database_mongo>
{
	public:
		bool connect(const char* ip_str);
		bool ensureIndex(const string& connection, const mongo::BSONObj& key);
		bool save(const string& connection, const mongo::BSONObj& key, const mongo::BSONObj& value);
		bool drop(const string& connection);
		void remove(const string& connection, const mongo::BSONObj& key);
		int load(const string& connection, BSONObjVec& objs);
		void find(const string& connection, const mongo::BSONObj& key, mongo::BSONObj& obj);

	private:
		mongo::DBClientConnection _db_conn;
};


#endif
