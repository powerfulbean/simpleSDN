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
		int iPortNum;
//		int sockID;
		cRouter(){iStage=0;iRouteNum=0;iRouterID=0;}
		void readConfigFile(char* filePath);
		void writeLogFile(vector<string> vLog);
		int parser(const string &temp, vector<string> &output);
		int syntax();
};
int getUdpSocket();
void setTempAddr(const char* pIp,struct sockaddr_in & locAddr);
void getDynmcPortSrv(const struct sockaddr_in & locAddr,
			struct sockaddr_in & outputAddr);
