#include <fstream>
#include <string>
#include <boost/filesystem.hpp>

#include "types.hpp"
#include "hexTools.hpp"

// get target
const string get_target(string & target, bool & bitmode) {
	if (target.compare("sbox") == 0)
		target = "0x1f";
	if (target.compare("rijinv") == 0)
		target = "0x01";
	if (target.compare(0,2,"0b") == 0) {
		/// bit mode
		bitmode = true;
	} else if (target.compare(0,2,"0x") == 0) {
		/// byte mode
		bitmode = false;
	} else {
		throw runtime_error("Invalid target: " + target + ".");
	}
	
	const string pathname("sboxes");
	path p(pathname);
	if (!exists(status(p)))
		throw runtime_error("\"" + pathname + "\" directory not found!");
	
	return pathname + "/" + target;
}

// load target table
void load_target_table(const string filename, byte * targetTable) {
	if (!exists(filename))
		throw runtime_error("Target file \"" + filename + "\" not found.");
	ifstream infile(filename);
	string line;
	int i(0);
	while (getline(infile, line))
		targetTable[i++] = stoi(line, nullptr, 0);
}

// load traces
void load_traces(const string dirname, vector<trace_t> & traces, unsigned int n_traces = -1) {
	path p(dirname);
	
	trace_t trace_read;
	string filename, pt, ct;
	unsigned int i(0);
	
	for (auto f = directory_iterator(p); f != directory_iterator(); f++) {
		if (!is_directory(f->path()))
		{
			if (++i > n_traces && n_traces >= 0) break;
			filename = f->path().filename().string();
			
			// plaintext
			pt = filename.substr(0, filename.find("_"));
			trace_read.pt = new byte[16];
			hex2nbytes(pt.c_str(), trace_read.pt);
			
			// ciphertext
			ct = filename.substr(filename.find("_")+1, filename.length()-1);
			trace_read.ct = new byte[16];
			hex2nbytes(ct.c_str(), trace_read.ct);
			
			// trace
			trace_read.trace = new vector<byte>();
			
			ifstream file(f->path().string(), ios::binary);
			char data;
			while (file.read(&data, 1))
				for (int j = 7; j >= 0; j--)
					trace_read.trace->push_back((data >> j) & 1);
			
			traces.push_back(trace_read);
		}
	}
}
