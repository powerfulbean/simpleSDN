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
	string temp2 = "router: 1, pid: " + sPid + ", port: " + to_string(iRou2Port);
	vLog.push_back(temp2);
}

void secondRouter_s2(cRouter & Router)
{
	char buffer[2048];
	struct sockaddr_in rou1Addr;
	struct timeval timeout;
	struct sockaddr_in rou2ExternalAddr;
	struct in_addr oriSrcAddr;
	int iSockID = Router.iSockID;
	int iRawSockID;
	int iMaxfdpl;
	fd_set fdSetAll, fdSet;
	timeout.tv_sec = 15;
	timeout.tv_usec = 0;
	FD_ZERO(&fdSetAll);
	FD_SET(iSockID, &fdSetAll);
	if (Router.iStage == 3)
	{
		iRawSockID = getIcmpRawSocket();
		if(iRawSockID<0)
		{
			perror("get raw socket error");
		}
		Router.iRawSockID = iRawSockID;
		FD_SET(iRawSockID, &fdSetAll);
		rou2ExternalAddr.sin_addr.s_addr = inet_addr("192.168.201.2");
		rou2ExternalAddr.sin_family = AF_INET;
		rou2ExternalAddr.sin_port = htons(0);
		socklen_t len = sizeof(rou2ExternalAddr);
		if (-1 == bind(iRawSockID,(struct sockaddr*) &rou2ExternalAddr, len))
		{
			perror("secondRouter_s2 error, bind error");
		}
		iMaxfdpl = (iRawSockID > iSockID) ? (iRawSockID + 1) : (iSockID + 1);
	}
	else if (Router.iStage == 2)
	{
		iMaxfdpl = Router.iSockID + 1;
	}
	while (1)
	{
		fdSet = fdSetAll;
		int iSelect = select(iMaxfdpl, &fdSet, NULL, NULL, &timeout);
		if (iSelect == 0)
		{
			cout << "timeout!" << endl;
			return;
		}
		else
		{
			if(FD_ISSET(iSockID, &fdSet))
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
					int iProtoType = icmpUnpack(buffer, srcAddr, dstAddr, icmp_type);
					if (iProtoType != 1)
					{
						continue;
					}
					int iCheck = packetDstCheck(dstAddr, "10.5.51.0", "255.255.255.0");
					if (iCheck == 1)
					{
						icmpReply_secondRouter(Router.iSockID, buffer, sizeof(buffer), rou1Addr);
					}
					else
					{
						if (Router.iStage == 3)
						{
							oriSrcAddr = srcAddr;
							icmpForward_secondRouter(Router, buffer, sizeof(buffer), rou1Addr, rou2ExternalAddr.sin_addr);
						}
					}
				}
				//icmpReply_primRouter(tun_fd, buffer, nread);
			}
			if (Router.iStage == 3)
			{
				if (FD_ISSET(iRawSockID, &fdSet))
				{
					char buffer2[2048];
					struct sockaddr_in senderAddr;
					struct iovec iov2;
					struct msghdr msg2;
					iov2.iov_base = buffer2;
					iov2.iov_len = sizeof(buffer2);
					msg2.msg_name = &senderAddr;
					msg2.msg_namelen = sizeof(senderAddr);
					msg2.msg_iov = &iov2;
					msg2.msg_iovlen = 1;
					msg2.msg_control = NULL;
					msg2.msg_controllen = 0;
					msg2.msg_flags = 0;
					int err = recvmsg(iRawSockID, &msg2, 0);
					if (err == -1)
					{
						perror("icmpForward_secondRouter error: recvmsg");
					}
					else
					{
						perror("icmpForward_secondRouter success: recvmsg");
						printf(": src address : %s  \n", inet_ntoa(senderAddr.sin_addr));
					}

					u_int8_t icmpType = getIcmpType(buffer);
					cout<<"Raw socket second router icmpType: " << icmpType<<endl;
					if (icmpType == 0)
					{
						icmpForward_log(Router, buffer2, 2048, FromRawSock, ntohs(senderAddr.sin_port)); // last var has no sense in this statement
						printf("orignal src address: %s  \n", inet_ntoa(oriSrcAddr));
						icmpReply_Edit(oriSrcAddr, buffer2, FromRawSock);
						err = sendMsg(Router.iSockID, buffer2, 2048, rou1Addr);
						if (err == -1)
						{
							perror("icmpForward_secondRouter error: sendMsg");
						}
						else
						{
							perror("icmpForward_secondRouter success: sendMsg");
						}
					}
				}
			}
			timeout.tv_sec = 15;
			timeout.tv_usec = 0;
		}
	}
	

}

void secondRouter_s4(cRouter & Router)
{
	char buffer[2048];
	struct sockaddr_in rou1Addr;
	struct timeval timeout;
	struct sockaddr_in rou2ExternalAddr;
	struct in_addr oriSrcAddr;
	int iSockID = Router.iSockID;
	int iRawSockID;
	int iMaxfdpl;
	fd_set fdSetAll, fdSet;
	timeout.tv_sec = 15;
	timeout.tv_usec = 0;
	FD_ZERO(&fdSetAll);
	FD_SET(iSockID, &fdSetAll);
	iRawSockID = getIcmpRawSocket();
	if (iRawSockID<0)
	{
		perror("get raw socket error");
	}
	Router.iRawSockID = iRawSockID;
	FD_SET(iRawSockID, &fdSetAll);
	rou2ExternalAddr.sin_addr.s_addr = inet_addr("192.168.201.2");
	rou2ExternalAddr.sin_family = AF_INET;
	rou2ExternalAddr.sin_port = htons(0);
	socklen_t len = sizeof(rou2ExternalAddr);
	if (-1 == bind(iRawSockID, (struct sockaddr*) &rou2ExternalAddr, len))
	{
		perror("secondRouter_s2 error, bind error");
	}
	iMaxfdpl = (iRawSockID > iSockID) ? (iRawSockID + 1) : (iSockID + 1);
	while (1)
	{
		fdSet = fdSetAll;
		int iSelect = select(iMaxfdpl, &fdSet, NULL, NULL, &timeout);
		if (iSelect == 0)
		{
			cout << "timeout!" << endl;
			return;
		}
		else
		{
			if (FD_ISSET(iSockID, &fdSet))
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
					int iProtoType = icmpUnpack(buffer, srcAddr, dstAddr, icmp_type);
					if (iProtoType == 1)
					{	
						printf("Second Router Read a ICMP packet \n");
						int iIcmpType = getIcmpType(buffer);
						if (iIcmpType != 8)
						{
							return;
						}
						int iCheck = packetDstCheck(dstAddr, "10.5.51.0", "255.255.255.0");
						if (iCheck == 1)
						{
							icmpReply_secondRouter(Router.iSockID, buffer, sizeof(buffer), rou1Addr);
						}
						else
						{
							oriSrcAddr = srcAddr;
							icmpForward_secondRouter(Router, buffer, sizeof(buffer), rou1Addr, rou2ExternalAddr.sin_addr);
						}
					}
					else if (iProtoType == 253)
					{
						printf("Second Router Read a Control Message packet \n");
						octane_control octMsg;
						int iSeqno = octaneUnpack(buffer, &octMsg);
						cout << "iSeqno: " << iSeqno << endl;
						if (Router.m_unAckBuffer.find(iSeqno) == Router.m_unAckBuffer.end())
						{
							string sLog = "router: " + to_string(Router.iRouterID) + Router.m_rouFlowTable.insert(octMsg);
							Router.vLog.push_back(sLog);
							if (Router.m_MsgCount.find(iSeqno) == Router.m_MsgCount.end())
							{
								Router.m_MsgCount[iSeqno] = 1;
							}
							else
							{
								int iTemp = Router.m_MsgCount[iSeqno];
								Router.m_MsgCount[iSeqno] = iTemp + 1;
							}
							if (Router.m_MsgCount[iSeqno] == Router.m_iDropAfter)
							{
								Router.m_MsgCount.erase(iSeqno);
								Router.m_unAckBuffer[iSeqno] = octMsg;
							}
							octaneReply_Edit(buffer);
							sendMsg(Router.iSockID, buffer, sizeof(buffer), rou1Addr);
						}
					}
				}
				//icmpReply_primRouter(tun_fd, buffer, nread);
			}
			if (FD_ISSET(iRawSockID, &fdSet))
			{
				char buffer2[2048];
				struct sockaddr_in senderAddr;
				struct iovec iov2;
				struct msghdr msg2;
				iov2.iov_base = buffer2;
				iov2.iov_len = sizeof(buffer2);
				msg2.msg_name = &senderAddr;
				msg2.msg_namelen = sizeof(senderAddr);
				msg2.msg_iov = &iov2;
				msg2.msg_iovlen = 1;
				msg2.msg_control = NULL;
				msg2.msg_controllen = 0;
				msg2.msg_flags = 0;
				int err = recvmsg(iRawSockID, &msg2, 0);
				if (err == -1)
				{
					perror("icmpForward_secondRouter error: recvmsg");
				}
				else
				{
					perror("icmpForward_secondRouter success: recvmsg");
					printf(": src address : %s  \n", inet_ntoa(senderAddr.sin_addr));
				}

				uint8_t icmpType =  getIcmpType(buffer2);

				printf("\n rawsocket rawsocket icmp type: %x \n ",icmpType);

				if (icmpType ==  0)
				{
					icmpForward_log(Router, buffer2, 2048, FromRawSock, ntohs(senderAddr.sin_port)); // last var has no sense in this statement
					printf("orignal src address: %s  \n", inet_ntoa(oriSrcAddr));
					icmpReply_Edit(oriSrcAddr, buffer2, FromRawSock);
					err = sendMsg(Router.iSockID, buffer2, 2048, rou1Addr);
					if (err == -1)
					{
						perror("icmpForward_secondRouter error: sendMsg");
					}
					else
					{
						perror("icmpForward_secondRouter success: sendMsg");
					}
				}
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
							 const struct in_addr addrForReplace)
{
	struct in_addr dstAddr;
	struct in_addr srcAddr;
	struct msghdr msg1;
	struct iovec iov1;
	struct icmphdr icmphdr;
	struct sockaddr_in sockDstAddr;
	u_int8_t icmp_type;
	icmpUnpack(buffer, icmphdr, srcAddr, dstAddr, icmp_type);
	sockDstAddr.sin_addr = dstAddr;
	sockDstAddr.sin_family = AF_INET;


	struct ip * pIpHeader;
	struct icmp * pIcmp;
	pIpHeader = (struct ip *) buffer;
	unsigned int iIpHeaderLen = pIpHeader->ip_hl << 2;
	pIcmp = (struct icmp *)(buffer + iIpHeaderLen);
	short iIcmpTotLen = ntohs(pIpHeader->ip_len) - iIpHeaderLen;

	int iRawSockID = Router.iRawSockID;
	iov1.iov_base = pIcmp;// (char*)&icmphdr;
	iov1.iov_len = iIcmpTotLen;
	msg1.msg_name = &sockDstAddr;
	msg1.msg_namelen = sizeof(sockDstAddr);
	msg1.msg_iov = &iov1;
	msg1.msg_iovlen = 1;
	msg1.msg_control = 0;
	msg1.msg_controllen = 0;
	msg1.msg_flags = 0;
	int err = sendmsg(iRawSockID, &msg1, 0);
	if (err == -1)
	{
		perror("icmpForward_secondRouter error: sendmsg");
	}
	else
	{
		perror("icmpForward_secondRouter success: sendmsg");
		printf(": target dst address : %s  \n", inet_ntoa(sockDstAddr.sin_addr));
	}


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
