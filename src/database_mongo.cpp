#include "../include/database_mongo.h"
#include <string>
#include <iostream>

using namespace std;

bool database_mongo::connect(const char* ip_str)
{
	string error_str;
	if(!_db_conn.connect(ip_str, error_str))
	{
		cout << "connect db (" << ip_str << ") fail: " << error_str << endl;
		return false;
	}
	return true;
}

bool database_mongo::ensureIndex(const string& connection, const mongo::BSONObj& key)
{
	//return _db_conn.ensureIndex(connection, key, true);
	return true;
}

bool database_mongo::save(const string& connection, const mongo::BSONObj& key, const mongo::BSONObj& value)
{
	try
	{
		_db_conn.update(connection, mongo::Query(key), value, true);
	}
	catch(mongo::DBException& e)
	{
		cout << "catch exception: " << e.what() << endl;
		cout << "connection: " << connection << endl; 
		cout << "key: " << key.toString() << endl; 
		cout << "value: " << value.toString() << endl; 
		return false;
	}
	return true;
}

bool database_mongo::drop(const string& connection)
{
	return _db_conn.dropCollection(connection);
}

void database_mongo::remove(const string& connection, const mongo::BSONObj& key)
{
	_db_conn.remove(connection, mongo::Query(key));
}

int database_mongo::load(const string& connection, BSONObjVec& objs)
{
	std::auto_ptr<mongo::DBClientCursor> cursor = _db_conn.query(connection, mongo::Query());
	int num = 0;
	while(cursor->more())
	{
		mongo::BSONObj res = cursor->next();
		objs.push_back(res);
		++num;
	}

	cout << "load " << connection << " finish(" << num << ")" << endl;
	return num;
}

void database_mongo::find(const string& connection, const mongo::BSONObj& key, mongo::BSONObj& obj)
{
	obj = _db_conn.findOne(connection, mongo::Query(key));
}











