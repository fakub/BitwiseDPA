default: attack.cpp
	g++ -Wall -o attack attack.cpp -std=c++0x -lboost_system -lboost_filesystem -lyaml-cpp

test: test.cpp
	g++ -Wall -o test test.cpp -std=c++0x -lboost_system -lboost_filesystem -lyaml-cpp

all: attack.cpp test.cpp
	g++ -Wall -o attack attack.cpp -std=c++0x -lboost_system -lboost_filesystem -lyaml-cpp
	g++ -Wall -o test test.cpp -std=c++0x -lboost_system -lboost_filesystem -lyaml-cpp