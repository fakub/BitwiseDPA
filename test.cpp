#include "attack_tools.hpp"

int main(int argc, char ** argv) {
	
	vector<int> u {-1,0,1,2,3};
	vector<double> v {0.5, 0.2, 0.1, 0.3, 0.4};
	vector<bool> w {0,1,1,0,1};
	
	int a = 5;
	double b = 2.3;
	
	//~ cout << "u+v:" << endl;
	//~ for (auto e: (u+=v))
		//~ cout << e << " ";
	//~ cout << endl;
	//~ 
	//~ cout << "v+w:" << endl;
	//~ for (auto e: (v+=w))
		//~ cout << e << " ";
	//~ cout << endl;
	//~ 
	//~ cout << "v/a:" << endl;
	//~ for (auto e: (v/=a))
		//~ cout << e << " ";
	//~ cout << endl;
	//~ 
	//~ cout << "abs(u):" << endl;
	//~ for (auto e: absv(u))
		//~ cout << e << " ";
	//~ cout << endl;
	
	ma_t ma = maxargmax(v);
	cout << ma.max << ma.argmax << endl;
	
	return 0;
}