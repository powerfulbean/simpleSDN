#include "Router.h"

void primaryRouter(const int sockID, cRouter & Router, sockaddr_in & rou2Addr)
{
	vector<string> &vLog = Router.vLog;
	string temp = "primary port: " + to_string(Router.iPortNum);
	vLog.push_back(temp);
	char buf[1024], pRou2Addr[16];
	memset(buf, 0, 1024);
	recvMsg(sockID, buf, 1024, rou2Addr);
	string sMsgRecv(buf);
	inet_ntop(AF_INET, &rou2Addr.sin_addr, pRou2Addr, sizeof(pRou2Addr));
	int iRou2Port = ntohs(rou2Addr.sin_port);
	string temp2 = "router 1, pid: " + sMsgRecv + ", port: " + to_string(iRou2Port);
	vLog.push_back(temp2);
}

void stage1(cRouter &Router)
{
	int sockID;
	socklen_t len;
	struct sockaddr_in rou1Addr, locAddr;
	setTempAddr("127.0.0.1", locAddr);
	getDynmcPortSrv(locAddr, rou1Addr);
	sockID = getUdpSocket();
	char pRou1Addr[16];
	unsigned int iRou1Port;
	inet_ntop(AF_INET, &rou1Addr.sin_addr, pRou1Addr, sizeof(pRou1Addr));// translate the router 1 ip address to ascii
	iRou1Port = ntohs(rou1Addr.sin_port);//translate the router 1 port  to ascii
	Router.iPortNum = iRou1Port;
	int iBind = bind(sockID, (struct sockaddr*)&rou1Addr, sizeof(rou1Addr));
	if (iBind<0)
	{
		perror("bind error");
	}
	cout << "primary address:" << pRou1Addr << endl;
	cout << "primary port: " << iRou1Port << endl;


	pid_t fPid;
	fPid = fork();
	if (fPid<0)
	{
		cout << " fork : error" << endl;
	}
	else if (fPid == 0)
	{
		secondRouter(Router, rou1Addr);
		Router.iFPID = fPid;

	}
	else
	{
		cout << "child process pid: " << fPid << endl << endl;
		Router.iFPID = fPid;
		sockaddr_in rou2Addr;
		primaryRouter(sockID, Router, rou2Addr);
	}
}

void stage2(cRouter &Router)
{
	stage1(Router);
	if(Router.iFPID == 0) // if it is secondary router
	{
		;
	}
	else// if it is primary router
	{
		//tunnel_reader();
		int tun_fd = set_tunnel_reader();
		char buffer[2048];
		while(1)
		{
			int nread = read_tunnel(tun_fd,buffer,sizeof(buffer));
			if(nread < 0)
			{
				exit(1);
			}
			else
			{
				printf("Read a packet from tunnel, packet length:%d\n", nread);
				IPhandler(buffer);
				cwrite(tun_fd, buffer, nread);
			}
		}
	}
}

