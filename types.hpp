#pragma once

using namespace std;
using namespace boost::filesystem;

typedef unsigned char byte;

struct trace_t {
	vector<bool> * trace;
	byte * pt;
	byte * ct;
};

struct ma_t {
	double max;
	int argmax;
};

struct kg_tb {
	byte kguess;
	vector<ma_t> tbits;
};

struct sort_by_tb {
	sort_by_tb(byte tb){this->tb = tb;}
	bool operator() (kg_tb kt1, kg_tb kt2) {
		return kt1.tbits[tb].max < kt2.tbits[tb].max;
	}
	byte tb;
};