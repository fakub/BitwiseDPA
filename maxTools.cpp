#include "types.hpp"

ma_t maxargmax(const vector<double> & v) {
	ma_t result;
	auto max_iter = max_element(v.begin(), v.end());
	result.argmax = max_iter - v.begin();
	result.max = *max_iter;
	
	return result;
}
