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
