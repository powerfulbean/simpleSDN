//jindou
#pragma once
#include "network.h"
using namespace std;

//vector<string>:{<stage1>,<num_routers1>,<stage2>,<num_routers2>...}

class cRouter{
	public:
		int iStage;
		int iRouteNum;
		int iRouterID;
		int iPortNum;
		int iFPID;// store the return value of fork()
		vector<string> vConfig;
		vector<string> vLog;
		int iConfigReg;
//		int sockID;


		cRouter(){iStage=0;iRouteNum=0;iRouterID=0;iConfigReg=0;}
		void readConfigFile(char* filePath);
		void writeLogFile();
		int parser(const string &temp, vector<string> &output);
		int syntax();
		int stageEngine();
		void close();
};


