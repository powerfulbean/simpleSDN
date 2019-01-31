//jindou
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>

using namespace std;

#define SERV_PORT 80
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
