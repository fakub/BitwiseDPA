#include "attack_tools.hpp"

int main(int argc, char ** argv) {
try {
	
	
	// Load settings
	
	fprintf(stderr, "\n");
	
	if (argc < 2) throw runtime_error("Usage:\n\t$ ./attack settings.yaml");
	const YAML::Node config = YAML::LoadFile(argv[1]);
	
	const string dirname = config["dirname"].as<string>();
	
	int n_traces = -1;
	if (const YAML::Node n_traces_node = config["n_traces"])
		n_traces = n_traces_node.as<int>();
	
	byte attack_byte = 0;
	if (const YAML::Node attack_byte_node = config["attack_byte"]) {
		attack_byte = (byte)attack_byte_node.as<int>();
		if (attack_byte > 15) attack_byte = 15;
	}
	
	const byte targetbits[] = {0,1,2,3,4,5,6,7};
	const byte tb_size = sizeof(targetbits)/sizeof(*targetbits);
	
	byte exp_key[] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
	if (const YAML::Node exp_key_node = config["exp_key"]) {
		for (byte i=0; i<16; i++) exp_key[i] = 0x00;
		const string exp_key_str = config["exp_key"].as<string>();
		hex2nbytes(exp_key_str.c_str(), exp_key, exp_key_str.length() > 32 ? 16 : exp_key_str.length() / 2);
	}
	
	byte target_b = 0x01;
	string target = "01";
	byte linTable[256];
	if (const YAML::Node target_node = config["target"])
		target = config["target"].as<string>();
	hex2nbytes(target.c_str(), &target_b, 1);
	const string pathname = "sboxes";
	const string filename = pathname + "/0x" + target;
	const string noninv_filename = pathname + "/sg_0x" + target;
	path p(pathname);
	if (!exists(status(p)))
		throw runtime_error("\"sboxes\" directory not found!\nYou can still use \"sbox\" or \"rijinv\" as attack target.");
	if (!exists(filename) && !exists(noninv_filename))
		throw runtime_error("Target 0x" + target + " was not found!\n(File " + filename + " not found.)");
	string actual_fn = exists(filename) ? filename : noninv_filename;
	ifstream infile(actual_fn);
	string line;
	byte i = 0;
	while (getline(infile, line))
		linTable[i++] = stoi(line, nullptr, 0);
	
	
	// Load traces
	
	vector<trace_t> traces;
	load_traces(dirname, traces, n_traces);
	
	
	// o=============o
	// | Attack byte |
	// o=============o
	
	byte bestguess = 0;
	byte besttb = 0;
	double maxdiff = 0.0;
	byte hyp;
	unsigned int trace_size = traces.front().trace->size();
	printf("---\n");   // init YAML output to stdout
	
	fprintf(stderr, "Attacking %2d. byte\n\t# of traces: %d\n\ttarget: %s\n", attack_byte, n_traces, target.c_str());
	printf("%d:   # byte\n", attack_byte);   // YAML
	fprintf(stderr, "________________________________________________________________\n");
	
	// measure elapsed time begin
	clock_t begin = clock();
	
	// prepare memory for results
	vector<kg_tb> diffs;
	diffs.resize(256);
	for (kg_tb & kt: diffs) kt.tbits.resize(tb_size);
	
	// loop key guesses
	byte kguess = 0;
	do {
		if (!(kguess & 0x03)) fprintf(stderr, ".");
		
		// initiate mean, num
		vector<double> mean1[tb_size];
		for (auto & v: mean1) v.resize(trace_size, 0.0);
		vector<double> mean0[tb_size];
		for (auto & v: mean0) v.resize(trace_size, 0.0);
		unsigned int num0[tb_size] = {0};
		unsigned int num1[tb_size] = {0};
		
		// most demanding part
		for (auto trace_s: traces) {
			hyp = linTable[trace_s.pt[attack_byte] ^ kguess];
			
			for (auto tb: targetbits) {
				if (hyp & (1 << tb)) {
					mean1[tb] += *(trace_s.trace);
					num1[tb] += 1;
				} else {
					mean0[tb] += *(trace_s.trace);
					num0[tb] += 1;
				}
			}
		}
		
		// mean value
		for (byte i = 0; i < tb_size; i++) {
			if (num1[i]) mean1[i] /= num1[i];
			if (num0[i]) mean0[i] /= num0[i];
		}
		
		vector<vector<double>> absdiff;
		absdiff.resize(tb_size);
		
		// abs value of means difference
		for (byte i=0; i<tb_size; i++)
			absdiff[i] = absv(mean1[i] - mean0[i]);
		
		// fill diffs[kguess] with respective maximum values and its indexes
		transform(absdiff.begin(), absdiff.end(), diffs[kguess].tbits.begin(), maxargmax);
		diffs[kguess].kguess = kguess;
		
	} while (++kguess);
	fprintf(stderr, "\n");
	
	// measure elapsed time end
	clock_t end = clock();
	double elapsed = double(end - begin) / CLOCKS_PER_SEC;
	
	
	// Print table for each target bit
	
	double tmpmax = 0.0;
	for (auto tb: targetbits) {
		fprintf(stderr, "\n  Target bit %d:\n=================\n   No. |     Diffs | Keyguess | Argument\n=========================================\n", tb);
		printf("  %d:   # target bit\n", tb);   // YAML
		
		// sort at target bit by max
		sort(diffs.begin(), diffs.end(), sort_by_tb(tb));
		// find global max for bestguess
		if (diffs.rbegin()->tbits[tb].max - (++diffs.rbegin())->tbits[tb].max > tmpmax) {
			bestguess = diffs.rbegin()->kguess;
			besttb = tb;
			tmpmax = diffs.rbegin()->tbits[tb].max - (++diffs.rbegin())->tbits[tb].max;
			maxdiff = tmpmax;
			fprintf(stderr, "New local max: %.7f\n-------------------------\n", tmpmax);
		}
		int exp_index = (int)(find_if(diffs.rbegin(), diffs.rend(), [attack_byte, exp_key](const kg_tb & kt) {return kt.kguess == exp_key[attack_byte];}) - diffs.rbegin());
		
		int i = 0;
		for (auto kg = diffs.rbegin(); kg != diffs.rend(); ++kg, i++) {
			if (i <= 2 || abs(i-exp_index) <= 2) {
				fprintf(stderr, "%6d | %02.7f |       %02x |    %5d %s\n", i, kg->tbits[tb].max, kg->kguess, kg->tbits[tb].argmax, i == exp_index ? "<=" : "");
			} else if (i == 3 || i-exp_index == 3) {
				fprintf(stderr, "     : |         : |        : |        :\n");
			}
			printf("  - - %.7f\n    - %d%s\n    - %d\n", kg->tbits[tb].max, kg->kguess, i == exp_index ? "   # expected" : "", kg->tbits[tb].argmax);   // YAML
		}
		fprintf(stderr, "\n");
	}
	// print elapsed time
	fprintf(stderr, "Time per %d. byte: %01.0f:%02.0f:%02.0f\n\n", attack_byte, floor(elapsed/3600.0), floor(fmod(elapsed,3600.0)/60.0), fmod(elapsed,60.0));
	
	
	// Print final result
	
	fprintf(stderr, "o==============================================================o\n");
	fprintf(stderr, "| Best: %02x   (diff: %0.3f, at %d. target bit)                   |\n", bestguess, maxdiff, besttb);
	fprintf(stderr, "| Exp:  %02x                                                     |\n", exp_key[attack_byte]);
	fprintf(stderr, "o==============================================================o\n");
	
	// delete allocated memory
	for (auto i: traces) {
		delete[] i.pt;
		delete[] i.ct;
		delete i.trace;
	}
	
} catch(const runtime_error & re) {
	cerr << "Error: " << re.what() << endl;
} catch(exception & ex) {
	cerr << "Error: " << ex.what() << endl;
} catch(...) {
	cerr << "Error: unspecified." << endl;
}
return 0;
}