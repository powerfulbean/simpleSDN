//jindou
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>

using namespace std;

class cRouter{
	public:
		int iStage;
		int iRouteNum;
		cRouter(){;}
		void readConfigFile(char* filePath);
		int parser(const string &temp, vector<string> &output);
};
