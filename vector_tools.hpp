#pragma once

template<typename T, typename U>
vector<T>& operator+= (vector<T> & a, const vector<U> & b)
{
	assert(a.size() == b.size());
	
	transform(a.begin(), a.end(), b.begin(), a.begin(), plus<T>());
	return a;
}

template<typename T, typename U>
vector<T> operator- (const vector<T> & a, const vector<U> & b)
{
	assert(a.size() == b.size());
	
	vector<T> result;
	result.reserve(a.size());
	
	transform(a.begin(), a.end(), b.begin(), back_inserter(result), minus<T>());
	return result;
}

template<typename T, typename U>
vector<T>& operator/= (vector<T> & a, const U b)
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
