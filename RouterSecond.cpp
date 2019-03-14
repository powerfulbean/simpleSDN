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
	bool bRefreshTimeout = true;
	struct sockaddr_in rou1Addr;
	struct timeval timeout;
	struct sockaddr_in rou2ExternalAddr;
	struct in_addr oriSrcAddr;
	int iSockID = Router.iSockID;
	int iRawSockID;
	int iMaxfdpl;
	int iCount = 0;
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
				bRefreshTimeout = true;
				char buffer[2048];
				int nread = recvMsg(Router.iSockID, buffer, sizeof(buffer), rou1Addr);

				flow_entry entry(buffer);
				string sCheck = Router.m_rouFlowTable.flowCheck(entry);
				if (sCheck.size() != 0)
				{
					string sLog = "router: " + to_string(Router.iRouterID) + sCheck;
					cout << endl << sLog << endl;
				}

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
						if (Router.m_droppedMsg.find(octMsg) == Router.m_droppedMsg.end())
						{
							string sLog = "router: " + to_string(Router.iRouterID) + Router.m_rouFlowTable.insert(octMsg);
							Router.vLog.push_back(sLog);
							if (Router.m_MsgCount.find(octMsg) == Router.m_MsgCount.end())
							{
								Router.m_MsgCount[octMsg] = 1;
							}
							else
							{
								int iTemp = Router.m_MsgCount[octMsg];
								Router.m_MsgCount[octMsg] = iTemp + 1;
							}
							if (Router.m_MsgCount[octMsg] == Router.m_iDropAfter)
							{
								Router.m_MsgCount.erase(octMsg);
								Router.m_droppedMsg[octMsg] = flow_action(octMsg);
							}
							octaneReply_Edit(buffer);
							//if (iCount++ == 2)
							{
								sendMsg(Router.iSockID, buffer, sizeof(buffer), rou1Addr);
							//	iCount = 0;
							}
							
						}
					}
					else
					{
						bRefreshTimeout = false;
					}
				}
				//icmpReply_primRouter(tun_fd, buffer, nread);
			}
			if (FD_ISSET(iRawSockID, &fdSet))
			{
				bRefreshTimeout = true;
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
					flow_entry entry(buffer2);
					string sCheck = Router.m_rouFlowTable.flowCheck(entry);
					if (sCheck.size() != 0)
					{
						string sLog = "router: " + to_string(Router.iRouterID) + sCheck;
						cout << endl << sLog << endl;
					}
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
				else
				{
					bRefreshTimeout = false;
				}
			}
			if (bRefreshTimeout == true)
			{
				timeout.tv_sec = 15;
				timeout.tv_usec = 0;
			}
		}
	}


}

void secondRouter_s5(cRouter & Router)
{
	char buffer[2048];
	bool bRefreshTimeout = true;
	struct sockaddr_in rou1Addr;
	struct timeval timeout;
	struct sockaddr_in rou2ExternalAddr;
	struct in_addr oriSrcAddr;
	int iSockID = Router.iSockID;
	int iRawSockID;
	int iMaxfdpl;
	int iCount = 0;
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

	string sLog = "router: " + to_string(Router.iRouterID) + Router.m_rouFlowTable.defaultInsert();
	Router.vLog.push_back(sLog);

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
				bRefreshTimeout = true;
				char buffer[2048];
				int nread = recvMsg(Router.iSockID, buffer, sizeof(buffer), rou1Addr);

				flow_entry entry(buffer);
				string sCheck = Router.m_rouFlowTable.flowCheck(entry);

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
						if (sCheck.size() != 0)
						{
							string sLog = "router: " + to_string(Router.iRouterID) + sCheck;
							cout << endl << sLog << endl;
							Router.vLog.push_back(sLog);
						}
					}
					else if (iProtoType == 253)
					{
						printf("Second Router Read a Control Message packet \n");
						octane_control octMsg;
						int iSeqno = octaneUnpack(buffer, &octMsg);
						cout << "iSeqno: " << iSeqno << endl;
						if (Router.m_droppedMsg.find(octMsg) == Router.m_droppedMsg.end())
						{
							string sLog = "router: " + to_string(Router.iRouterID) + Router.m_rouFlowTable.insert(octMsg);
							Router.vLog.push_back(sLog);
							if (Router.m_MsgCount.find(octMsg) == Router.m_MsgCount.end())
							{
								Router.m_MsgCount[octMsg] = 1;
							}
							else
							{
								int iTemp = Router.m_MsgCount[octMsg];
								Router.m_MsgCount[octMsg] = iTemp + 1;
							}
							if (Router.m_MsgCount[octMsg] == Router.m_iDropAfter)
							{
								Router.m_MsgCount.erase(octMsg);
								Router.m_droppedMsg[octMsg] = flow_action(octMsg);
							}
							octaneReply_Edit(buffer);
							//if (iCount++ == 2)
							{
								sendMsg(Router.iSockID, buffer, sizeof(buffer), rou1Addr);
								//	iCount = 0;
							}

						}
					}
					else
					{
						bRefreshTimeout = false;
					}
				}
				//icmpReply_primRouter(tun_fd, buffer, nread);
			}
			if (FD_ISSET(iRawSockID, &fdSet))
			{
				bRefreshTimeout = true;
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

								

				uint8_t icmpType = getIcmpType(buffer2);

				printf("\n rawsocket rawsocket icmp type: %x \n ", icmpType);

				if (icmpType == 0)
				{
					icmpForward_log(Router, buffer2, 2048, FromRawSock, ntohs(senderAddr.sin_port)); // last var has no sense in this statement
					printf("orignal src address: %s  \n", inet_ntoa(oriSrcAddr));
					icmpReply_Edit(oriSrcAddr, buffer2, FromRawSock);

					flow_entry entry(buffer2);
					string sCheck = Router.m_rouFlowTable.flowCheck(entry);
					if (sCheck.size() != 0)
					{
						string sLog = "router: " + to_string(Router.iRouterID) + sCheck;
						cout << endl << sLog << endl;
						Router.vLog.push_back(sLog);
					}

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
				else
				{
					bRefreshTimeout = false;
				}
			}
			if (bRefreshTimeout == true)
			{
				timeout.tv_sec = 15;
				timeout.tv_usec = 0;
			}
		}
	}


}

void secondRouter_s6(cRouter & Router) // target port of  octane_control is host endian (big endian)
{
	char buffer[2048];
	bool bRefreshTimeout = true;
	struct sockaddr_in rou1Addr;
	struct timeval timeout;
	struct sockaddr_in rou2ExternalAddr;
	struct in_addr oriSrcAddr;
	int iSockID = Router.iSockID;
	int iRawSockID;
	int iTcpRawSockID;
	int iMaxfdpl;
	int iCount = 0;
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
	iTcpRawSockID = getRawSocket(6);
	if (iTcpRawSockID<0)
	{
		perror("get raw socket error");
	}
	Router.iRawSockID = iRawSockID;
	Router.m_iTcpRawSocketID = iTcpRawSockID;
	FD_SET(iRawSockID, &fdSetAll);
	FD_SET(iTcpRawSockID, &fdSetAll);
	if (Router.iRouterID == 1)
	{
		rou2ExternalAddr.sin_addr.s_addr = inet_addr("192.168.201.2");
	}
	else if (Router.iRouterID == 2)
	{
		rou2ExternalAddr.sin_addr.s_addr = inet_addr("192.168.202.2");
	}
	rou2ExternalAddr.sin_family = AF_INET;
	rou2ExternalAddr.sin_port = htons(0);
	socklen_t len = sizeof(rou2ExternalAddr);
	if (-1 == bind(iRawSockID, (struct sockaddr*) &rou2ExternalAddr, len))
	{
		perror("secondRouter_s2 error, bind error");
	}
	if (-1 == bind(iTcpRawSockID, (struct sockaddr*) &rou2ExternalAddr, len))
	{
		perror("secondRouter_s2 error, bind error");
	}
	iMaxfdpl = (iRawSockID > iSockID) ? (iRawSockID + 1) : (iSockID + 1);
	iMaxfdpl = (iMaxfdpl -1  > iTcpRawSockID) ? (iMaxfdpl) : (iTcpRawSockID);
	string sLog = "router: " + to_string(Router.iRouterID) + Router.m_rouFlowTable.defaultInsert();
	Router.vLog.push_back(sLog);

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
				bRefreshTimeout = true;
				char buffer[2048];
				int nread = recvMsg(Router.iSockID, buffer, sizeof(buffer), rou1Addr);

				flow_entry entry(buffer);
				string sCheck = Router.m_rouFlowTable.flowCheck(entry);

				if (nread < 0)
				{
					exit(1);
				}
				else
				{
					printf("Read a packet from primary router, packet length:%d\n", nread);
					struct in_addr srcAddr;
					struct in_addr dstAddr;
					u_int8_t icmp_type;
					int iProtoType = icmpUnpack(buffer, srcAddr, dstAddr, icmp_type);
					if (iProtoType == 253)
					{
						printf("Second Router Read a Control Message packet \n");
						octane_control octMsg;
						int iSeqno = octaneUnpack(buffer, &octMsg);
						cout << "iSeqno: " << iSeqno << endl;
						if (Router.m_droppedMsg.find(octMsg) == Router.m_droppedMsg.end())
						{
							string sLog = "router: " + to_string(Router.iRouterID) + Router.m_rouFlowTable.insert(octMsg);
							Router.vLog.push_back(sLog);
							if (Router.m_MsgCount.find(octMsg) == Router.m_MsgCount.end())
							{
								Router.m_MsgCount[octMsg] = 1;
							}
							else
							{
								int iTemp = Router.m_MsgCount[octMsg];
								Router.m_MsgCount[octMsg] = iTemp + 1;
							}
							if (Router.m_MsgCount[octMsg] == Router.m_iDropAfter)
							{
								Router.m_MsgCount.erase(octMsg);
								Router.m_droppedMsg[octMsg] = flow_action(octMsg);
							}
							octaneReply_Edit(buffer);
							//if (iCount++ == 2)
							{
								sendMsg(Router.iSockID, buffer, sizeof(buffer), rou1Addr);
								//	iCount = 0;
							}

						}
					}
					else if (sCheck.size() != 0)
					{
						printf("Second Router Read a ICMP packet \n");
						icmpForward_log(Router, buffer, sizeof(buffer), FromUdp, ntohs(rou1Addr.sin_port));
						
						/*int iCheck = packetDstCheck(dstAddr, "10.5.51.0", "255.255.255.0");
						if (iCheck == 1)
						{
							icmpReply_secondRouter(Router.iSockID, buffer, sizeof(buffer), rou1Addr);
						}
						else
						{
							oriSrcAddr = srcAddr;
							icmpForward_secondRouter(Router, buffer, sizeof(buffer), rou2ExternalAddr.sin_addr);
						}*/

						int iFlag = octaneRulesController(entry, Router, buffer, sizeof(buffer), rou1Addr, rou2ExternalAddr.sin_addr);
						if (iFlag == 0)
						{
							oriSrcAddr = srcAddr;
						}

						string sLog = "router: " + to_string(Router.iRouterID) + sCheck;
						cout << endl << sLog << endl;
						Router.vLog.push_back(sLog);
					}
					else
					{
						bRefreshTimeout = false;
					}
				}
				//icmpReply_primRouter(tun_fd, buffer, nread);
			}
			if (FD_ISSET(iRawSockID, &fdSet))
			{
				bRefreshTimeout = true;
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

								

				uint8_t icmpType = getIcmpType(buffer2);

				printf("\n rawsocket rawsocket icmp type: %x \n ", icmpType);
				printf("orignal src address: %s  \n", inet_ntoa(oriSrcAddr));
				char buffer3[2048];
				memcpy(buffer3, buffer2, sizeof(buffer2));
				icmpReply_Edit(oriSrcAddr, buffer3, FromRawSock);

				flow_entry entry(buffer3);
				string sCheck = Router.m_rouFlowTable.flowCheck(entry);
				if (sCheck.size() != 0)
				{
					icmpForward_log(Router, buffer2, 2048, FromRawSock, ntohs(senderAddr.sin_port)); // last var has no sense in this statement
					string sLog = "router: " + to_string(Router.iRouterID) + sCheck;
					cout << endl << sLog << endl;
					Router.vLog.push_back(sLog);

					//err = sendMsg(Router.iSockID, buffer3, 2048, rou1Addr);
					sockaddr_in tempSockAddr;
					in_addr tempInAddr;
					octaneRulesController(entry, Router, buffer3, 2048, tempSockAddr, tempInAddr);
					if (err == -1)
					{
						perror("icmpForward_secondRouter error: sendMsg");
					}
					else
					{
						perror("icmpForward_secondRouter success: sendMsg");
					}
				}
				else
				{
					bRefreshTimeout = false;
				}
			}
			if (FD_ISSET(iTcpRawSockID, &fdSet))
			{
				cout << endl << "iTCPrawsocket get packet" << endl;
				bRefreshTimeout = true;
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
				int err = recvmsg(iTcpRawSockID, &msg2, 0);
				if (err == -1)
				{
					perror("icmpForward_secondRouter error: recvmsg");
				}
				else
				{
					perror("icmpForward_secondRouter success: recvmsg");
					printf(": src address : %s  \n", inet_ntoa(senderAddr.sin_addr));
				}

				printf("orignal src address: %s  \n", inet_ntoa(oriSrcAddr));
				char buffer3[2048];
				memcpy(buffer3, buffer2, sizeof(buffer2));
				icmpReply_Edit(oriSrcAddr, buffer3, FromRawSock);



				flow_entry entry(buffer3);
				string sCheck = Router.m_rouFlowTable.flowCheck(entry);
				if (sCheck.size() != 0)
				{
					icmpForward_log(Router, buffer2, 2048, FromRawSock, ntohs(senderAddr.sin_port)); // last var has no sense in this statement
					string sLog = "router: " + to_string(Router.iRouterID) + sCheck;
					cout << endl << sLog << endl;
					Router.vLog.push_back(sLog);

					//err = sendMsg(Router.iSockID, buffer3, 2048, rou1Addr);
					sockaddr_in tempSockAddr;
					in_addr tempInAddr;
					octaneRulesController(entry, Router, buffer3, 2048, tempSockAddr, tempInAddr);
					if (err == -1)
					{
						perror("icmpForward_secondRouter error: sendMsg");
					}
					else
					{
						perror("icmpForward_secondRouter success: sendMsg");
					}
				}
				else
				{
					bRefreshTimeout = false;
				}
			}
			if (bRefreshTimeout == true)
			{
				timeout.tv_sec = 15;
				timeout.tv_usec = 0;
			}
		}
	}


}

void secondRouter_reqReg(cRouter & Router, const sockaddr_in rou1Addr, int iRouterID)
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
	Router.iRouterID = iRouterID;
	Router.iPortNum = iRou2Port;
	Router.iSockID = sockID2;
	//cout << "router2: local address: " << pRou2Addr << endl;
	//cout << "router2: local  port: " << iRou2Port << endl;
	//cout << endl;
	string temp2 = "router: " + to_string(iRouterID) + ", pid: " + sPid + ", port: " + to_string(iRou2Port);
	vLog.push_back(temp2);
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

void icmpForward_secondRouter(cRouter & Router, char* buffer, unsigned int iSize,
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

void tcpForward_secondRouter(cRouter & Router, char* buffer, unsigned int iSize,
	const struct in_addr addrForReplace)
{
	struct in_addr dstAddr;
	struct in_addr srcAddr;
	struct msghdr msg1;
	struct iovec iov1;
	struct icmphdr icmphdr;
	struct sockaddr_in sockDstAddr;
	uint32_t sSrc_addr;
	uint32_t sDst_addr;
	uint16_t sSrc_port;
	uint16_t sDst_port;
	uint8_t ip_type;
	char * psdBuffer[2048] = { 0 };
	//icmpUnpack(buffer, icmphdr, srcAddr, dstAddr, icmp_type);
	ipUnpack(buffer, sSrc_addr, sDst_addr, sSrc_port, sDst_port, ip_type);
	sockDstAddr.sin_addr.s_addr = sDst_addr;
	sockDstAddr.sin_family = AF_INET;
	

	struct ip * pIpHeader;
	struct tcphdr * pTcp, *pTcp_psd;
	struct psdhdr* pPsd;
	pIpHeader = (struct ip *) buffer;
	unsigned int iIpHeaderLen = pIpHeader->ip_hl << 2;
	pTcp = (struct tcphdr *)(buffer + iIpHeaderLen);
	short iTcpTotLen = ntohs(pIpHeader->ip_len) - iIpHeaderLen;
	pTcp_psd = (struct tcphdr *)(psdBuffer + sizeof(struct psdhdr));
	pPsd = (struct psdhdr *) psdBuffer;

	// calculate check sum
	pPsd->saddr = pIpHeader->ip_src.s_addr;//inet_addr("192.168.201.2");
	pPsd->daddr = pIpHeader->ip_dst.s_addr;
	pPsd->mbz = 0;
	pPsd->protocol = pIpHeader->ip_p;
	pPsd->tcpl = htons(iTcpTotLen); //htons(sizeof(struct tcphdr));
	cout << endl << "tcp len: " << iTcpTotLen << endl;
	printf("tcp ori check sum: %x \n", pTcp->check);
	pTcp->check = 0;
	memcpy(pTcp_psd, pTcp, iTcpTotLen);
	cout << endl << "psdhdr len: " << sizeof(struct psdhdr)<<" "<< sizeof(psdhdr) << endl;
	pTcp->check = checksum((char*)psdBuffer, iTcpTotLen + sizeof(struct psdhdr));

	int iRawSockID = Router.m_iTcpRawSocketID;
	iov1.iov_base = pTcp;// (char*)&icmphdr;
	iov1.iov_len = iTcpTotLen;
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


int octaneRulesController(const flow_entry entry, cRouter Router, char* buffer, int iSize, struct sockaddr_in rou1Addr, struct in_addr rou2Sin_addr)
{
	const flow_action & action = Router.m_rouFlowTable.m_mTable[entry];
	if (action.m_action == 1)
	{
		// forward
		if (action.m_fwdPort == 0)
		{
			// forward to rawsocket
			if (entry.m_protocol == 1)
			{
				icmpForward_secondRouter(Router, buffer, iSize, rou2Sin_addr);
			}
			else 
			{
				//cout << endl<<"tcp out" << endl;
				tcpForward_secondRouter(Router, buffer, iSize, rou2Sin_addr);
			}
			return 0;
		}
		else
		{
			if (action.m_fwdPort <= 0)
			{
				cout << "wrong target port number!";
				return -2;
			}
			else
			{
				cout << "action: forward, target port: "<< action.m_fwdPort;
			}
			sockaddr_in targetAddr;
			targetAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
			targetAddr.sin_family = AF_INET;
			targetAddr.sin_port = htons(action.m_fwdPort);
			sendMsg(Router.iSockID, buffer, iSize, targetAddr);
			return action.m_fwdPort;
		}
	}
	else if (action.m_action == 2)
	{
		icmpReply_secondRouter(Router.iSockID, buffer, iSize, rou1Addr);
		return -1;
	}
	else if (action.m_action == 3)
	{
		Router.Drop();
		return -1;
	}
	else if (action.m_action == 4)
	{
		//remove
		Router.m_rouFlowTable.m_mTable.erase(entry);
		return -1;
	}
	return -2;
}