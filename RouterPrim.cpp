#include "Router.h"

void primaryRouter(const int sockID, cRouter & Router, sockaddr_in & rou2Addr)
{
	Router.iSockID = sockID;
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

void primaryRouter_s2(cRouter & Router, sockaddr_in &rou2Addr)
{
	int tun_fd = set_tunnel_reader();
	char buffer[2048];
	while (1)
	{
		int nread = read_tunnel(tun_fd, buffer, sizeof(buffer));
		if (nread < 0)
		{
			exit(1);
		}
		else
		{
			printf("Read a packet from tunnel, packet length:%d\n", nread);
			int a = icmpForward_log(Router, buffer, sizeof(buffer),  FromTunnel, ntohs(rou2Addr.sin_port));
			if (a != 1)
			{
				return;
			}
			sendMsg(Router.iSockID, buffer, sizeof(buffer), rou2Addr);
			//icmpReply_primRouter(tun_fd, buffer, nread);
		}
	}
}

int icmpForward_log(cRouter & Router, char * buffer, unsigned int iSize, int flag, int iPort)
{
	vector<string> &vLog = Router.vLog;
	struct in_addr srcAddr;
	struct in_addr dstAddr;
	u_int8_t icmp_type;
	int a = icmpUnpack(buffer, srcAddr, dstAddr, icmp_type);
	if(a!=1)
	{
		return 0;
	}
	string sSrcAddr = inet_ntoa(srcAddr);
	string sDstAddr = inet_ntoa(dstAddr);
	string sIcmp_type = to_string(icmp_type);

	if (flag == FromTunnel)
	{
		string sLog = "ICMP from tunnel, src: " + sSrcAddr + ", dst : " + sDstAddr + ", type : " + sIcmp_type;
		vLog.push_back(sLog);
	}
	else if (flag == FromUdp)
	{
		string sSrcPort = to_string(iPort);
		string sLog = "ICMP from port : " +  sSrcPort +  ", src: " + sSrcAddr + ", dst : " + sDstAddr + ", type : " + sIcmp_type;
		vLog.push_back(sLog);
	}
	return 1;
}

void icmpReply_primRouter(int tun_fd, char* buffer, int nread)
{
	icmpReply_Edit(buffer);
	cwrite(tun_fd, buffer, nread);// send packet back to tunnel
}

void stage1(cRouter &Router,
			struct sockaddr_in & rou1Addr,
			struct sockaddr_in & rou2Addr)
{
	int sockID;
	socklen_t len;
	struct sockaddr_in  locAddr;
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
		secondRouter(Router, rou1Addr, rou2Addr);
		Router.iFPID = fPid;
		secondRouter_s2(Router);
	}
	else
	{
		cout << "child process pid: " << fPid << endl << endl;
		Router.iFPID = fPid;
		primaryRouter(sockID, Router, rou2Addr);
	}
}

void stage2(cRouter &Router,
			struct sockaddr_in & rou1Addr,
			struct sockaddr_in & rou2Addr)
{
	stage1(Router, rou1Addr, rou2Addr);
	if(Router.iFPID == 0) // if it is secondary router
	{
		;
	}
	else// if it is primary router
	{
		//tunnel_reader();
		primaryRouter_s2(Router, rou2Addr);
	}
}

