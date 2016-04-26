#pragma once

const string get_target(string & target, bool & bitmode);
void load_target_table(const string filename, byte * targetTable);
void load_traces(const string dirname, vector<trace_t> & traces, unsigned int n_traces = -1);
