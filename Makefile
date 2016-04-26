CXXFLAGS = -O3 -Wall -std=c++0x
LIBS = -lboost_system -lboost_filesystem -lyaml-cpp
OBJS = hexTools.o manipTools.o maxTools.o

attack: $(OBJS) main.o
	$(CXX) -o main $(OBJS) main.o $(CXXFLAGS) $(LIBS)

test: $(OBJS) test.o
	$(CXX) -o test $(OBJS) test.o $(CXXFLAGS) $(LIBS)

default: attack

all: attack test
