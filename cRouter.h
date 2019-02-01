//jindou
#include "network.h"
using namespace std;

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

void primaryRouter(int sockID,cRouter & Router, const sockaddr_in rou1Addr);
void secondRouter(cRouter & Router, const sockaddr_in rou1Addr);
