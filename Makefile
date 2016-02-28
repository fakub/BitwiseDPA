CXXFLAGS = -O3 -Wall -std=c++0x
LIBS = -lboost_system -lboost_filesystem -lyaml-cpp

attack: attack.cpp
	$(CXX) -o attack attack.cpp $(CXXFLAGS) $(LIBS)

test: test.cpp
	$(CXX) -o test   test.cpp   $(CXXFLAGS) $(LIBS)

default: attack

all: attack test