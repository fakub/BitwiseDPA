#include <fstream>
#include <string>
#include <stdexcept>
#include <vector>
#include <yaml-cpp/yaml.h>

#include "types.hpp"
#include "hexTools.hpp"
#include "maxTools.hpp"
#include "manipTools.hpp"
#include "vectorTools.hpp"

int main(int argc, char ** argv) {
try {
	
	if (argc < 2) throw runtime_error("Usage:\n\t$ ./main settings.yaml");
	fprintf(stderr, "\n");
	
	
	// ===   P R E P A R E   A T T A C K   =======================================
	
	// load settings
	const YAML::Node config(YAML::LoadFile(argv[1]));
	
	// get dirname
	const string dirname(config["dirname"].as<string>());
	
	// get number of traces
	int n_traces(-1);
	if (const YAML::Node n_traces_node = config["n_traces"])
		n_traces = n_traces_node.as<int>();
	
	// get byte to be attacked
	byte attack_byte(0);
	if (const YAML::Node attack_byte_node = config["attack_byte"]) {
		attack_byte = static_cast<byte>(attack_byte_node.as<int>());
		if (attack_byte > 15) attack_byte = 15;
	}
	
	// get expected key
	byte exp_key[] {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
	if (const YAML::Node exp_key_node = config["exp_key"]) {
		for (byte i=0; i<16; i++) exp_key[i] = 0x00;
		const string exp_key_str(config["exp_key"].as<string>());
		hex2nbytes(exp_key_str.c_str(), exp_key, exp_key_str.length() > 32 ? 16 : exp_key_str.length() / 2);
	}
	
	// get target
	string target("0x1f");
	if (const YAML::Node target_node = config["target"])
		target = config["target"].as<string>();
	bool bitmode(false);
	const string filename(get_target(target, bitmode));
	const byte targetbits(bitmode ? 1 : 8);
	
	// load target table
	byte targetTable[256];
	load_target_table(filename, targetTable);
	
	// load traces
	vector<trace_t> traces;
	load_traces(dirname, traces, n_traces);
	
	// init attack variables
	byte bestguess(0);
	byte besttb(0);
	double maxdiff(0.0);
	byte kguess(0);
	byte hyp(0);
	unsigned int trace_size(traces.front().trace->size());
	vector<kg_tb> diffs;
	diffs.resize(256);
	for (kg_tb & kt: diffs) kt.tbits.resize(targetbits);
	
	
	// ===   S T A R T !   =======================================================
	
	// init YAML output to stdout
	printf("---\n");
	
	// print progress bar
	fprintf(stderr, "Attacking %2d. byte\n\t# of traces: %d\n\ttarget: %s\n", attack_byte, n_traces, target.c_str());
	fprintf(stderr, "________________________________________________________________\n");
	
	// measure elapsed time begin
	clock_t begin(clock());
	
	// ---   begin loop key guesses   --------------------------------------------
	do {
		// progress
		if (!(kguess & 0x03)) fprintf(stderr, ".");
		
		// init mean, num
		vector<double> mean1[targetbits];
		for (auto & v: mean1) v.resize(trace_size, 0.0);
		vector<double> mean0[targetbits];
		for (auto & v: mean0) v.resize(trace_size, 0.0);
		unsigned int num0[targetbits];
		for (byte i=0; i<targetbits; i++) num0[i] = 0;
		unsigned int num1[targetbits];
		for (byte i=0; i<targetbits; i++) num1[i] = 0;
		
		// most demanding part
		for (auto trace_s: traces) {
			// what we expect
			hyp = targetTable[trace_s.pt[attack_byte] ^ kguess];
			
			for (byte tb=0; tb<targetbits; tb++) {
				// switch traces according to what we expect
				if (hyp & (1 << tb)) {
					mean1[tb] += *(trace_s.trace);
					num1[tb] += 1;
				} else {
					mean0[tb] += *(trace_s.trace);
					num0[tb] += 1;
				}
			}
		}
		
		// compute mean value
		for (byte i = 0; i < targetbits; i++) {
			if (num1[i]) mean1[i] /= num1[i];
			if (num0[i]) mean0[i] /= num0[i];
		}
		
		// init max diffs
		vector<vector<double>> absdiff;
		absdiff.resize(targetbits);
		
		// abs value of difference of means
		for (byte i=0; i<targetbits; i++)
			absdiff[i] = absv(mean1[i] - mean0[i]);
		
		// fill diffs[kguess] with respective maximum values and its indexes
		transform(absdiff.begin(), absdiff.end(), diffs[kguess].tbits.begin(), maxargmax);
		diffs[kguess].kguess = kguess;
		
	} while (++kguess);
	// ---   end loop key guesses   ----------------------------------------------
	fprintf(stderr, "\n");
	
	// measure elapsed time end
	clock_t end(clock());
	double elapsed(static_cast<double>(end - begin) / CLOCKS_PER_SEC);
	
	// print table for each target bit
	double tmpmax(0.0);
	for (byte tb=0; tb<targetbits; tb++) {
		fprintf(stderr, "\n  Target bit %d:\n=================\n   No. |     Diffs | Keyguess | Argument\n=========================================\n", tb);
		// YAML output
		printf("%d:   # target bit\n", tb);
		
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
		// get rank of expected key
		int exp_index(static_cast<int>(find_if(diffs.rbegin(), diffs.rend(), [attack_byte, exp_key](const kg_tb & kt) {return kt.kguess == exp_key[attack_byte];}) - diffs.rbegin()));
		
		// print full table of results
		int i(0);
		for (auto kg = diffs.rbegin(); kg != diffs.rend(); ++kg, i++) {
			if (i <= 2 || abs(i-exp_index) <= 2) {
				fprintf(stderr, "%6d | %02.7f |       %02x |    %5d %s\n", i, kg->tbits[tb].max, kg->kguess, kg->tbits[tb].argmax, i == exp_index ? "<=" : "");
			} else if (i == 3 || i-exp_index == 3) {
				fprintf(stderr, "     : |         : |        : |        :\n");
			}
			// YAML output
			printf("- - %.7f\n  - %d%s\n  - %d\n", kg->tbits[tb].max, kg->kguess, i == exp_index ? "   # expected" : "", kg->tbits[tb].argmax);
		}
		fprintf(stderr, "\n");
	}
	// print elapsed time
	fprintf(stderr, "Time per %d. byte: %01.0f:%02.0f:%02.0f\n\n", attack_byte, floor(elapsed/3600.0), floor(fmod(elapsed,3600.0)/60.0), fmod(elapsed,60.0));
	
	// print best guess
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