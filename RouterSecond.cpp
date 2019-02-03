#include "Router.h"


void secondRouter(cRouter & Router, const sockaddr_in rou1Addr)
{
	vector<string> &vLog = Router.vLog;
	int sockID2 = getUdpSocket();
	struct sockaddr_in rou2Addr;
	socklen_t len = sizeof(rou2Addr);
	string sPid = to_string(getpid());
	string msg = "router: 1, pid: xxxx";
	sendMsg(sockID2, sPid.data(), sPid.size() * sizeof(char), rou1Addr);
	// sendto(sockID2,msg.data(),msg.size()*sizeof(char),0,(struct sockaddr*)&rou1Addr,len2);
	getsockname(sockID2, (struct sockaddr*) &rou2Addr, &len);
	unsigned int iRou2Port;
	char pRou2Addr[16];
	inet_ntop(AF_INET, &rou2Addr.sin_addr, pRou2Addr, sizeof(pRou2Addr));
	iRou2Port = ntohs(rou2Addr.sin_port);
	Router.iRouterID = 1;
	Router.iPortNum = iRou2Port;
	cout << "router2: local address: " << pRou2Addr << endl;
	cout << "router2: local  port: " << iRou2Port << endl;
	cout << endl;
	string temp2 = "router 1, pid: " + sPid + ", port: " + to_string(iRou2Port);
	vLog.push_back(temp2);
}