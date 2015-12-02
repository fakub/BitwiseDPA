#include "attack_tools.hpp"

int main(int argc, char ** argv) {
	
	//~ const string dirname = "traces/dusan_WB_read_addr_lsB_2048";
	const string dirname = "traces/naiveAES_256";
	const int n_traces = 16;
	const byte targetbits[] = {0,1,2,3,4,5,6,7};
	const byte tb_size = sizeof(targetbits)/sizeof(*targetbits);
	
	vector<trace_t> traces;
	
	printf("\nRunning Philippe Teuwen DPA attack ...\n\n");
	
	printf("Loading traces ...\n");
	load_traces(dirname, traces, n_traces);
	
	printf("Expecting 2b7e151628aed2a6abf7158809cf4f3c ...\n");
	byte bestguess[16];
	byte hyp;
	unsigned int trace_size = traces.front().trace->size();
	
	for (byte bnum = 0; bnum < 16; bnum++) {
		printf("Attacking %2d. byte ...\n", bnum + 1);
		fprintf(stderr, "________________________________________________________________\n");
		
		vector<vector<ma_t>> diffs;
		diffs.resize(256);
		for (auto & v: diffs) v.resize(tb_size);
		
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
			
			for (auto trace_s: traces) {
				hyp = sboxTable[trace_s.pt[bnum] ^ kguess];
				
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
			
			for (byte i = 0; i < tb_size; i++) {
				if (num1[i]) mean1[i] /= num1[i];
				if (num0[i]) mean0[i] /= num0[i];
			}
			
			vector<vector<double>> absdiff;
			absdiff.resize(tb_size);
			
			for (byte i=0; i<tb_size; i++)
				absdiff[i] = absv(mean1[i] - mean0[i]);
			
			transform(absdiff.begin(), absdiff.end(), diffs[kguess].begin(), maxargmax);
			
			for (auto & tb: diffs[kguess])
				printf("%2.3f ", tb.max);
			printf("\n");
			return 0;
			
		} while (++kguess);
		fprintf(stderr, "\n");
		
		bestguess[bnum] = (byte)(max_element(diffs.begin(), diffs.end(), cmp_tb) - diffs.begin());
		
		//~ for (int ii=0; ii<4; ii++) {
			//~ printf("Key Guess %02x:\n\t", ii);
			//~ for (int jj=0; jj<8; jj++) {
				//~ printf("%4d. ", jj);
			//~ }
			//~ printf("\n\t");
			//~ for (int jj=0; jj<8; jj++) {
				//~ printf("%2.3f ", diffs[ii][jj].max);
			//~ }
			//~ printf("\n");
		//~ }
		
		printf("Currently the best is: %02x\n", bestguess[bnum]);
		
		//~ bestguess[bnum] = diffs.each_with_index.max[1]
		
		//~ # Print table
		//~ targetbits.each do |tb|
			//~ puts "\n\n  Target bit #{tb}:"
			//~ puts "================="
			//~ puts "   No. |     Diffs | Keyguess | Argument"
			//~ puts "========================================="
			//~ 
			//~ diffs_sort = diffs.each_with_index.sort_by{|a,i|a[tb][0]}.reverse
			//~ exp_index = diffs_sort.index{|dak|dak[1] == EXPKEY_ARR[bnum]}
			//~ 
			//~ diffs_sort.each_with_index do |dak, i|
				//~ if i <= 2 or (i-exp_index).abs <=2
					//~ puts "   #{"%3d" % i} | #{"%02.7f" % dak[0][tb][0]} |       #{"%02x" % dak[1]} |    #{"%5d" % dak[0][tb][1]}" + (EXPKEY_ARR[bnum] == dak[1] ? " <=" : "")
				//~ elsif i == 3 or i-exp_index == 3
					//~ puts "     : |         : |        : |        :"
				//~ end
			//~ end
		//~ end
	}
	
	for (byte bg: bestguess)
		printf("%02x ", bg);
	printf("\n");
	
	for (auto i: traces) {
		delete[] i.pt;
		delete[] i.ct;
		delete i.trace;
	}
	
	return 0;
}