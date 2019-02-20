#include "Router.h"


void secondRouter(cRouter & Router, const sockaddr_in rou1Addr, struct sockaddr_in &rou2Addr)
{
	vector<string> &vLog = Router.vLog;
	int sockID2 = getUdpSocket();
	//struct sockaddr_in rou2Addr;
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
	Router.iSockID = sockID2;
	cout << "router2: local address: " << pRou2Addr << endl;
	cout << "router2: local  port: " << iRou2Port << endl;
	cout << endl;
	string temp2 = "router 1, pid: " + sPid + ", port: " + to_string(iRou2Port);
	vLog.push_back(temp2);
}

void secondRouter_s2(cRouter & Router)
{
	char buffer[2048];
	struct sockaddr_in rou1Addr;
	struct timeval timeout;
	int iSockID = Router.iSockID;
	fd_set fdSetAll, rd;
	timeout.tv_sec = 15;
	timeout.tv_usec = 0;
	FD_ZERO(&fdSetAll);
	FD_SET(iSockID, &fdSetAll);
	while (1)
	{
		rd = fdSetAll;
		int iSelect = select(Router.iSockID + 1, &rd, NULL, NULL, &timeout);
		if (iSelect == 0)
		{
			cout << "timeout!" << endl;
			return;
		}
		else
		{
			char buffer[2048];
			int nread = recvMsg(Router.iSockID, buffer, sizeof(buffer), rou1Addr);
			if (nread < 0)
			{
				exit(1);
			}
			else
			{
				printf("Read a packet from primary router, packet length:%d\n", nread);
				icmpForward_log(Router, buffer, sizeof(buffer), FromUdp, ntohs(rou1Addr.sin_port));
				struct in_addr srcAddr;
				struct in_addr dstAddr;
				u_int8_t icmp_type;
				icmpUnpack(buffer, srcAddr, dstAddr, icmp_type);
				packetDstCheck(dstAddr, "10.5.51.0")
				icmpReply_secondRouter(Router.iSockID, buffer, sizeof(buffer), rou1Addr);
				//icmpReply_primRouter(tun_fd, buffer, nread);
			}
			timeout.tv_sec = 15;
			timeout.tv_usec = 0;
		}
	}
	

}

void icmpReply_secondRouter(int iSockID, char* buffer, unsigned int iSize, const struct sockaddr_in rou1Addr)
{
	icmpReply_Edit(buffer);
	sendMsg(iSockID, buffer, iSize, rou1Addr);
}


int packetDstCheck(struct in_addr &packetDstAddr, string targetDst, string mask)
{

	struct in_addr netmask;
	int err = inet_aton(mask,netmask);
	if (err == 0)
	{
		printf("packetDstCheck error: aton");
	}
	struct in_addr subnet;
	subnet.s_addr = netmask.s_addr & subnet.s_addr;



	string packetDst(inet_ntoa(subnet));
	if (targetDst == packetDst)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
