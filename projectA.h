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
		int iRouterID;
		cRouter(){iStage=0;iRouteNum=0;iRouterID=0;}
		void readConfigFile(char* filePath);
		void writeLogFile(vector<string> vLog);
		int parser(const string &temp, vector<string> &output);
		int syntax();
};
