default: attack.cpp
	g++ -Wall -o attack attack.cpp -std=c++0x -lboost_system -lboost_filesystem

test:
	g++ -Wall -o test test.cpp -std=c++0x -lboost_system -lboost_filesystem

all: attack.cpp test.cpp
	g++ -Wall -o attack attack.cpp -std=c++0x -lboost_system -lboost_filesystem
	g++ -Wall -o test test.cpp -std=c++0x -lboost_system -lboost_filesystem