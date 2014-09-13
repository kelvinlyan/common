SRC := src
INC := include
OBJ := obj

srcs = $(wildcard $(SRC)/*.cpp)
objs = $(patsubst %.cpp, $(OBJ)/%.o, $(notdir $(srcs)))
deps = $(objs:%.o=%.d)

CC := g++
CCFLAGS := -I$(INC)
LDFLAGS := -lboost_system -lboost_thread -ljsoncpp -ltinyxml -lboost_regex -lmongoclient
CCFLAGS += $(LDFLAGS)

vpath %.h $(INC)
vpath %.cpp $(SRC)

.PHONY : target
target : $(deps) $(objs)
	
$(OBJ)/%.d : $(SRC)/%.cpp
	@set -e;
	@rm -f $@;
	@$(CC) -MM $< $(CCFLAGS) > $@.temp;
#	@sed 's,\($*\)\.o[ :]*,$(OBJ)/\1.d : ,g' < $@.temp >> $@;
	@sed 's,\($*\)\.o[ :]*,$(OBJ)/\1.o $@ : ,g' < $@.temp >> $@;
	@echo -e "\t"$(CC) -c $< -g -o $(subst .d,.o,$@) $(CCFLAGS) >> $@;
	@rm -f $@.temp;
	@echo here

include $(deps)

.PHONY : info
info :
	@echo $(srcs);
	@echo $(objs);
	@echo $(deps);
	@echo $(CCFLAGS);

.PHONY : clean
clean :
	-rm -rf $(OBJ)/*;









