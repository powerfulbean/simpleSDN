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
	struct sockaddr_in rou2ExternalAddr;
	int iSockID = Router.iSockID;
	int iRawSockID;
	fd_set fdSetAll, rd;
	timeout.tv_sec = 15;
	timeout.tv_usec = 0;
	FD_ZERO(&fdSetAll);
	FD_SET(iSockID, &fdSetAll);
	if (Router.iStage == 3)
	{
		iRawSockID = getIcmpRawSocket();
		Router.iRawSockID = iRawSockID;
		rou2ExternalAddr.sin_addr.s_addr = inet_addr("192.168.201.2");
		rou2ExternalAddr.sin_family = AF_INET;
		rou2ExternalAddr.sin_port = htons(0);
		socklen_t len = sizeof(rou2ExternalAddr);
		if (-1 == bind(iRawSockID,(struct sockaddr*) &rou2ExternalAddr, len))
		{
			perror("secondRouter_s2 error, bind error");
		}

	}
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
				struct sockaddr_in sockDstAddr;
				u_int8_t icmp_type;
				icmpUnpack(buffer, srcAddr, dstAddr, icmp_type);
				int iCheck = packetDstCheck(dstAddr, "10.5.51.0","255.255.255.0");
				if (iCheck == 1)
				{
					icmpReply_secondRouter(Router.iSockID, buffer, sizeof(buffer), rou1Addr);
				}
				else
				{
					sockDstAddr.sin_addr = dstAddr;
					icmpForward_secondRouter(Router, buffer, sizeof(buffer),rou1Addr, sockDstAddr, rou2ExternalAddr.sin_addr);
				}
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

void icmpForward_secondRouter(cRouter & Router, char* buffer, unsigned int iSize, const struct sockaddr_in rou1Addr,
	       	struct sockaddr_in dstAddr, const struct in_addr addrForReplace)
{
	const struct in_addr oriSrcAddr = icmpReply_Edit(addrForReplace, buffer,FromUdp);
	struct msghdr msg;
	struct iovec iov;
	int iSockID = Router.iRawSockID;
	iov.iov_base = buffer;
	iov.iov_len = iSize;
	msg.msg_name = &dstAddr;
	msg.msg_namelen = sizeof(dstAddr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = NULL;
	msg.msg_controllen = 20;
	msg.msg_flags = 0;
	int err = sendmsg(iSockID, &msg, 0);
	char buffer2[2048];
	iov.iov_base = buffer2;
	iov.iov_len = sizeof(buffer2);
	err = recvmsg(iSockID, &msg, 0);
	icmpForward_log(Router, buffer2, sizeof(buffer2), FromRawSock, ntohs(dstAddr.sin_port));
	icmpReply_Edit(oriSrcAddr, buffer2, FromRawSock);
	sendMsg(iSockID, buffer2, sizeof(buffer2), rou1Addr);
}


int packetDstCheck(struct in_addr &packetDstAddr, string targetDst, string mask)
{

	struct in_addr netmask;
	int err = inet_aton(mask.data(),&netmask);
	if (err == 0)
	{
		printf("packetDstCheck error: aton");
	}
	struct in_addr subnet;
	subnet.s_addr = netmask.s_addr & packetDstAddr.s_addr;

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
