#include "attack_tools.hpp"

int main(int argc, char ** argv) {
try {
	
	
	// Load settings
	
	if (argc < 2) throw runtime_error("usage:\n\t$ ./attack settings.yaml");
	const YAML::Node config = YAML::LoadFile(argv[1]);
	
	const string dirname = config["dirname"].as<string>();
	const int n_traces = config["n_traces"].as<int>();
	const byte attack_bytes = (byte)config["attack_bytes"].as<int>();
	
	//~ const YAML::Node targetbits = config["targetbits"];
	//~ const byte tb_size = 8;
	
	const byte targetbits[] = {0,1,2,3,4,5,6,7};
	const byte tb_size = sizeof(targetbits)/sizeof(*targetbits);
	const byte exp_key[] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
	
	
	// Load traces
	
	printf("Loading traces ...\n");
	vector<trace_t> traces;
	load_traces(dirname, traces, n_traces);
	
	printf("Expecting ");
	printnbytes(exp_key);
	
	
	// Attack bytewise
	
	byte bestguess[16];
	byte hyp;
	unsigned int trace_size = traces.front().trace->size();
	
	for (byte bnum = 0; bnum < attack_bytes; bnum++) {
		printf("Attacking %2d. byte ...\n", bnum + 1);
		fprintf(stderr, "________________________________________________________________\n");
		
		vector<kg_tb> diffs;
		diffs.resize(256);
		for (kg_tb & kt: diffs) kt.tbits.resize(tb_size);
		
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
				hyp = sboxTable[trace_s.pt[bnum] ^ kguess];
				
				for (auto tb: targetbits) {
					//~ byte tb = tbi.as<byte>();
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
		
		
		// Print table for single attacked byte
		
		double tmpmax = 0;
		
		for (auto tb: targetbits) {
			//~ byte tb = tbi.as<byte>();
			printf("\n  Target bit %d:\n=================\n   No. |     Diffs | Keyguess | Argument\n=========================================\n", tb);
			
			// sort at target bit
			sort(diffs.begin(), diffs.end(), sort_by_tb(tb));
			// find global max for bestguess
			if (diffs.rbegin()->tbits[tb].max > tmpmax) {
				bestguess[bnum] = diffs.rbegin()->kguess;
				tmpmax = diffs.rbegin()->tbits[tb].max;
			}
			int exp_index = (int)(find_if(diffs.rbegin(), diffs.rend(), [bnum, exp_key](const kg_tb & kt) {return kt.kguess == exp_key[bnum];}) - diffs.rbegin());
			
			int i = 0;
			for (auto kg = diffs.rbegin(); kg != diffs.rend(); ++kg) {
				if (i <= 2 || abs(i-exp_index) <= 2) {
					printf("%6d | %02.7f |       %02x |    %5d %s\n", i, kg->tbits[tb].max, kg->kguess, kg->tbits[tb].argmax, i == exp_index ? "<=" : "");
				} else if (i == 3 || i-exp_index == 3) {
					printf("     : |         : |        : |        :\n");
				}
				i++;
			}
			printf("\n");
		}
	}   /// for (byte bnum = 0; bnum < 16; bnum++)
	
	
	// Print final result
	
	for (byte bg: bestguess)
		printf("%02x ", bg);
	printf("\n");
	
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