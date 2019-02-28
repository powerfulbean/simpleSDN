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
	string temp2 = "router: 1, pid: " + sMsgRecv + ", port: " + to_string(iRou2Port);
	vLog.push_back(temp2);
}

void primaryRouter_s2(cRouter & Router, sockaddr_in &rou2Addr)
{
	int tun_fd = set_tunnel_reader();
	int iSockID = Router.iSockID;
	
	struct timeval timeout;
	fd_set fdSetAll, fdSet;
	timeout.tv_sec = 15;
	timeout.tv_usec = 0;
	FD_ZERO(&fdSetAll);
	FD_SET(tun_fd, &fdSetAll);
	FD_SET(iSockID, &fdSetAll);
	int iMaxfdpl = (tun_fd > iSockID) ? (tun_fd + 1) : (iSockID + 1);
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
			char buffer[2048];
			if (FD_ISSET(tun_fd, &fdSet))
			{
				memset(buffer, 0, 2048);
				int nread = read_tunnel(tun_fd, buffer, sizeof(buffer));
				if (nread < 0)
				{
					exit(1);
				}
				else
				{
					printf("Read a packet from tunnel, packet length:%d\n", nread);
					int a = icmpForward_log(Router, buffer, sizeof(buffer), FromTunnel, ntohs(rou2Addr.sin_port));
					if (a != 1)
					{
						continue;
					}
					sendMsg(Router.iSockID, buffer, sizeof(buffer), rou2Addr);
					//icmpReply_primRouter(tun_fd, buffer, nread);
				}
			}
			if (FD_ISSET(iSockID, &fdSet))
			{
				memset(buffer, 0, 2048);
				struct sockaddr_in rou2Addr;
				int nread = recvMsg(Router.iSockID, buffer, sizeof(buffer), rou2Addr);
				if (nread < 0)
				{
					exit(1);
				}
				else
				{
					printf("Read a packet from secondary router, packet length:%d\n", nread);
					icmpForward_log(Router, buffer, sizeof(buffer), FromUdp, ntohs(rou2Addr.sin_port));
					cwrite(tun_fd, buffer, nread);// send packet back to tunnel
												  //sendMsg(Router.iSockID, buffer, sizeof(buffer), rou1Addr);
												  //icmpReply_primRouter(tun_fd, buffer, nread);
				}
			}
			timeout.tv_sec = 15;
			timeout.tv_usec = 0;
		}
	}
	
}

void primaryRouter_s4(cRouter & Router, sockaddr_in &rou2Addr)
{
	int tun_fd = set_tunnel_reader();
	int iSockID = Router.iSockID;
	//int iOctSockID = getRawSocket(253);
	/*if (iOctSockID == -1)
	{
		perror("get OctaneSocket error!");
	}*/
	//Router.m_iOctSockID = iOctSockID;
	struct timeval timeout;
	fd_set fdSetAll, fdSet;
	timeout.tv_sec = 15;
	timeout.tv_usec = 0;
	FD_ZERO(&fdSetAll);
	FD_SET(tun_fd, &fdSetAll);
	FD_SET(iSockID, &fdSetAll);
	//FD_SET(iOctSockID, &fdSetAll);
	int iMaxfdpl = (tun_fd > iSockID) ? (tun_fd + 1) : (iSockID + 1);
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
			char buffer[2048];
			if (FD_ISSET(tun_fd, &fdSet))
			{
				memset(buffer, 0, 2048);
				int nread = read_tunnel(tun_fd, buffer, sizeof(buffer));
				if (nread < 0)
				{
					exit(1);
				}
				else
				{
					printf("Read a packet from tunnel, packet length:%d\n", nread);
					int a = icmpForward_log(Router, buffer, sizeof(buffer), FromTunnel, ntohs(rou2Addr.sin_port));
					if (a == 1) // it is a ICMP packet
					{
						printf("Prim Router Read a ICMP packet \n", nread);
						struct octane_control localMsg, msg1,msg1_re;
						struct in_addr srcAddr, dstAddr;
						u_int8_t icmp_type;
						// create a orctane message for this primary router 
						Router.createOctaneMsg(localMsg, buffer, sizeof(buffer), 1, ntohs(rou2Addr.sin_port),false);
						//insert rules in flow_table and get the respective log
						vector<string> tempLog = Router.m_rouFlowTable.dbInsert(localMsg);
						for (int i = 0; i < tempLog.size(); i++)
						{
							string sLog = "router: " + to_string(Router.iRouterID) + tempLog[i];
							Router.vLog.push_back(sLog);
						}
						int iProtocolType = icmpUnpack(buffer, srcAddr, dstAddr, icmp_type);
						int iCheck = packetDstCheck(dstAddr, "10.5.51.0", "255.255.255.0");
						if (iCheck == 1)
						{
							int iCheck2 = packetDstCheck(dstAddr, "10.5.51.4", "255.255.255.255");
							int iSeqno;
							if (iCheck2 == 1)
							{
								iSeqno = Router.createOctaneMsg(msg1, buffer, sizeof(buffer), 3, -1);
							}
							else
							{
								iSeqno = Router.createOctaneMsg(msg1, buffer, sizeof(buffer), 2, -1);
							}
							char octaneIpBuffer[2048];
							memset(octaneIpBuffer, 0, 2048);
							string localAddr = "127.0.0.1";
							buildIpPacket(octaneIpBuffer, sizeof(octaneIpBuffer), 253, localAddr, localAddr, (char *)&msg1, sizeof(msg1));
							sendMsg(Router.iSockID, octaneIpBuffer, sizeof(octaneIpBuffer), rou2Addr); // send control message
							Router.m_unAckBuffer[iSeqno] = msg1;
						}
						else
						{
							int iSeqno1 = Router.createOctaneMsg(msg1, buffer, sizeof(buffer), 1, 0);
							int iSeqno2 = Router.createReverseOctaneMsg(msg1_re, msg1, Router.iPortNum);
							char octaneIpBuffer[2048];
							char octaneIpBufferRev[2048];
							memset(octaneIpBuffer, 0, 2048);
							memset(octaneIpBufferRev, 0, 2048);
							string localAddr = "127.0.0.1";
							buildIpPacket(octaneIpBuffer, sizeof(octaneIpBuffer), 253, localAddr, localAddr, (char *)&msg1, sizeof(msg1));
							buildIpPacket(octaneIpBufferRev, sizeof(octaneIpBufferRev), 253, localAddr, localAddr, (char *)&msg1_re, sizeof(msg1_re));
							sendMsg(Router.iSockID, octaneIpBuffer, sizeof(octaneIpBuffer), rou2Addr);// send control message
							sendMsg(Router.iSockID, octaneIpBufferRev, sizeof(octaneIpBufferRev), rou2Addr);// send control message
							Router.m_unAckBuffer[iSeqno1] = msg1;
							Router.m_unAckBuffer[iSeqno2] = msg1_re;
						}
						Router.printUnAckBuffer();
						sendMsg(Router.iSockID, buffer, sizeof(buffer), rou2Addr);
					}

				}
			}
			if (FD_ISSET(iSockID, &fdSet))
			{
				memset(buffer, 0, 2048);
				struct sockaddr_in rou2Addr;
				int nread = recvMsg(Router.iSockID, buffer, sizeof(buffer), rou2Addr);
				if (nread < 0)
				{
					exit(1);
				}
				else
				{
					printf("Read a packet from secondary router, packet length:%d\n", nread);
					icmpForward_log(Router, buffer, sizeof(buffer), FromUdp, ntohs(rou2Addr.sin_port));
					cwrite(tun_fd, buffer, nread);// send packet back to tunnel
												  //sendMsg(Router.iSockID, buffer, sizeof(buffer), rou1Addr);
												  //icmpReply_primRouter(tun_fd, buffer, nread);
				}
			}
			timeout.tv_sec = 15;
			timeout.tv_usec = 0;
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
		return a;
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
	else if (flag == FromRawSock)
	{
		string sLog = "ICMP from raw sock, src: " + sSrcAddr + ", dst : " + sDstAddr + ", type : " + sIcmp_type;
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
		secondRouter_s2(Router);
	}
	else// if it is primary router
	{
		//tunnel_reader();
		primaryRouter_s2(Router, rou2Addr);
	}
}

void stage4(cRouter &Router,
	struct sockaddr_in & rou1Addr,
	struct sockaddr_in & rou2Addr)
{
	stage1(Router, rou1Addr, rou2Addr);
	if (Router.iFPID == 0) // if it is secondary router
	{
		secondRouter_s4(Router);
	}
	else// if it is primary router
	{
		primaryRouter_s4(Router, rou2Addr);
	}
}

