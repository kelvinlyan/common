#include <mongo/bson/bson.h>
#include <mongo/client/dbclient.h>
#include "json/json.h"
#include <string>
#include <iostream>

using namespace std;

int main()
{
	Json::Value var;
	var["a"] = 1;
	Json::Value var2;
	var2["b1"] = "b1";
	var2["b2"] = 2;

	var["b"] = var2;
	cout << var.toStyledString() << endl;

	mongo::BSONObj obj = mongo::fromjson(var.toStyledString());

	cout << obj.jsonString() << endl;

	Json::Value result;
	Json::Reader reader;
	reader.parse(obj.jsonString(), result);
	cout << result.toStyledString() << endl;

}
