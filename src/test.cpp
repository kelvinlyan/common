#include "../include/database.h"
#include <iostream>
using namespace std;

int main()
{
	if(database_mongo::shared()->connect("127.0.0.1"))
		cout << "success" << endl;

	mongo::BSONObj obj = BSON("a" << 1);
	database_mongo::shared()->save("testA.table1", obj, obj);
	BSONObjVec objs;
	database_mongo::shared()->load("testA.table1", objs);
}
