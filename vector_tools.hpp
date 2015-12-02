#pragma once

ma_t maxargmax(const vector<double> & v) {
	ma_t result;
	auto max_iter = max_element(v.begin(), v.end());
	result.argmax = max_iter - v.begin();
	result.max = *max_iter;
	
	return result;
}

bool cmp_ma_t(ma_t ma1, ma_t ma2) {
	return ma1.max < ma2.max;
}

bool cmp_tb(vector<ma_t> v1, vector<ma_t> v2) {
	return max_element(v1.begin(), v1.end(), cmp_ma_t)->max > max_element(v1.begin(), v1.end(), cmp_ma_t)->max;
}

template<typename T, typename U>
vector<T>& operator+=(vector<T> & a, const vector<U> & b)
{
	assert(a.size() == b.size());
	
	transform(a.begin(), a.end(), b.begin(), a.begin(), plus<T>());
	return a;
}

template<typename T, typename U>
vector<T> operator-(const vector<T> & a, const vector<U> & b)
{
	assert(a.size() == b.size());
	
	vector<T> result;
	result.reserve(a.size());
	
	transform(a.begin(), a.end(), b.begin(), back_inserter(result), minus<T>());
	return result;
}

template<typename T, typename U>
vector<T>& operator/=(vector<T> & a, const U b)
{
	for (auto & e: a)
		e /= b;
	return a;
}

template<typename T>
T templ_abs(const T i) {return abs(i);}

template<typename T>
vector<T> absv(const vector<T> & a)
{
	vector<T> result;
	result.reserve(a.size());
	
	transform(a.begin(), a.end(), back_inserter(result), templ_abs<T>);
	return result;
}
