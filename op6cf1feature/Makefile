CCOMPILE=mpic++
CPPFLAGS= -I$(HADOOP_HOME)/include -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/linux -I .  -Wno-deprecated -O2 -fopenmp
LIB = -L$(HADOOP_HOME)/lib/native
LDFLAGS = -lhdfs

all: run

run: run.cpp
	$(CCOMPILE) run.cpp $(CPPFLAGS) $(LIB) $(LDFLAGS) -lpthread -std=c++11  -o run

clean:
	-rm run
