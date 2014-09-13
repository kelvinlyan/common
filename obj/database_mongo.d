obj/database_mongo.o obj/database_mongo.d : src/database_mongo.cpp src/../include/database_mongo.h \
 src/../include/singleton.h src/../include/noncopyable.h
	g++ -c src/database_mongo.cpp -g -o obj/database_mongo.o -Iinclude -lboost_system -lboost_thread -ljsoncpp -ltinyxml -lboost_regex -lmongoclient
