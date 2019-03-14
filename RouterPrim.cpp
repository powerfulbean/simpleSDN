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

void primaryRouter_s4_old(cRouter & Router, sockaddr_in &rou2Addr)
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
	bool bRefreshTimeout = true;
	Timers timersManager;
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
				bRefreshTimeout = true;
				memset(buffer, 0, 2048);
				int nread = read_tunnel(tun_fd, buffer, sizeof(buffer));

				flow_entry entry(buffer);
				string sCheck = Router.m_rouFlowTable.flowCheck(entry);

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

						if (sCheck.size() != 0)
						{
							string sLog = "router: " + to_string(Router.iRouterID) + sCheck;
							cout << endl << sLog << endl;
						}
						else
						{
							// create a orctane message for this primary router 
							Router.createOctaneMsg(localMsg, buffer, sizeof(buffer), 1, ntohs(rou2Addr.sin_port), false);
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
								//Router.m_unAckBuffer[iSeqno] = msg1;
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
								
								// Add timer and register the handle number
								/*cOctaneTimer octaneTimer1(Router.iSockID, rou2Addr, msg1, iSeqno1);
								cOctaneTimer octaneTimer2(Router.iSockID, rou2Addr, msg1_re, iSeqno2);
								timersManager.AddTimer(2000, &octaneTimer1);
								timersManager.AddTimer(2000, &octaneTimer2);
								Router.m_unAckBuffer[iSeqno1] = msg1;
								Router.m_unAckBuffer[iSeqno2] = msg1_re;*/
							}
							Router.printUnAckBuffer();
						}
						sendMsg(Router.iSockID, buffer, sizeof(buffer), rou2Addr);
					}
					else
					{
						bRefreshTimeout = false;
					}
				}
			}
			if (FD_ISSET(iSockID, &fdSet))
			{
				bRefreshTimeout = true;
				memset(buffer, 0, 2048);
				struct sockaddr_in rou2Addr;
				int nread = recvMsg(Router.iSockID, buffer, sizeof(buffer), rou2Addr);

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
					printf("Read a packet from secondary router, packet length:%d\n", nread);
					int iIcmpProtocol = icmpForward_log(Router, buffer, sizeof(buffer), FromUdp, ntohs(rou2Addr.sin_port));
					if (iIcmpProtocol == 1)// its a icmp pscket
					{
						cwrite(tun_fd, buffer, nread);// send packet back to tunnel
													  //sendMsg(Router.iSockID, buffer, sizeof(buffer), rou1Addr);
													  //icmpReply_primRouter(tun_fd, buffer, nread);
					}
					else if (iIcmpProtocol == OCTANE_PROTOCOL_NUM)
					{
						// check seqno and remove related record from the unack_buffer
						octane_control octMsg;
						int iSeqno = octaneUnpack(buffer, &octMsg);
						if (octMsg.octane_flags == 1)
						{
							// place for code to remove the related timer
							/*       */
							Router.m_unAckBuffer.erase(iSeqno);
							Router.printUnAckBuffer();
						}
					}
					else
					{
						bRefreshTimeout = false;
					}
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

void primaryRouter_s4(cRouter & Router, sockaddr_in &rou2Addr)
{
	int tun_fd = set_tunnel_reader();
	int iSockID = Router.iSockID;
	struct timeval idelTimeout;
	idelTimeout.tv_sec = 15;
	idelTimeout.tv_usec = 0;
	struct timeval timeout;
	timeout.tv_sec = 15;
	timeout.tv_usec = 0;
	bool bRefreshTimeout = true;
	Timers timersManager;
	fd_set fdSetAll, fdSet;
	FD_ZERO(&fdSetAll);
	FD_SET(tun_fd, &fdSetAll);
	FD_SET(iSockID, &fdSetAll);
	int iMaxfdpl = (tun_fd > iSockID) ? (tun_fd + 1) : (iSockID + 1);
	while (1)
	{
		bool bEventTimeout = false;
		fdSet = fdSetAll;
		struct timeval temp;
		temp.tv_sec = timeout.tv_sec;
		temp.tv_usec = timeout.tv_usec;
		timersManager.NextTimerTime(&timeout);
		if (timeout.tv_sec == 0 && timeout.tv_usec == 0) {
			cout << endl << " The timer at the head on the queue has expired " << endl;
			timersManager.ExecuteNextTimer();
			bEventTimeout = true;
		}
		if (timeout.tv_sec == MAXVALUE && timeout.tv_usec == 0) {
			cout << endl << "There are no timers in the event queue" << endl;
			timeout.tv_sec = temp.tv_sec;
			timeout.tv_usec = temp.tv_usec;
			bEventTimeout = false;
		}
		else
		{
			bEventTimeout = true;
		}
		int iSelect = select(iMaxfdpl, &fdSet, NULL, NULL, &timeout);
		if (iSelect == 0)
		{
			if (bEventTimeout == true)
			{
				cout <<endl<< "event time out!" << endl;
				// start of using the code of test-app.cc provided by csci551.
				// Timer expired, Hence process it 
				timersManager.ExecuteNextTimer();
				// Execute all timers that have expired.
				timersManager.NextTimerTime(&timeout);
				while (timeout.tv_sec == 0 && timeout.tv_usec == 0)
				{
					// Timer at the head of the queue has expired 
					timersManager.ExecuteNextTimer();
					timersManager.NextTimerTime(&timeout);
				}
				// end of using the code of test-app.cc provided by csci551.
				timeout.tv_sec = 15;
				timeout.tv_usec = 0;
			}
			else
			{
				cout << "timeout!" << endl;
				return;
			}
			
		}
		char buffer[2048];
		if (FD_ISSET(tun_fd, &fdSet))
		{
			bRefreshTimeout = true;
			memset(buffer, 0, 2048);
			int nread = read_tunnel(tun_fd, buffer, sizeof(buffer));

			flow_entry entry(buffer);
			string sCheck = Router.m_rouFlowTable.flowCheck(entry);

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
					struct octane_control localMsg, msg1, msg1_re;
					struct in_addr srcAddr, dstAddr;
					u_int8_t icmp_type;

					if (sCheck.size() != 0)
					{
						string sLog = "router: " + to_string(Router.iRouterID) + sCheck;
						cout << endl << sLog << endl;
					}
					else
					{
						// create a orctane message for this primary router 
						Router.createOctaneMsg(localMsg, buffer, sizeof(buffer), 1, ntohs(rou2Addr.sin_port), false);
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
							
							// Add timer and register the handle number																		   // Add timer and register the handle number
							cOctaneTimer * octaneTimer = new cOctaneTimer(Router.iSockID, rou2Addr, msg1, iSeqno);
							handle t1 = timersManager.AddTimer(2000, octaneTimer);
							Router.m_unAckBuffer[iSeqno] = t1;
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

							// Add timer and register the handle number
							cOctaneTimer *octaneTimer1 = new cOctaneTimer(Router.iSockID, rou2Addr, msg1, iSeqno1);
							cOctaneTimer *octaneTimer2 = new cOctaneTimer(Router.iSockID, rou2Addr, msg1_re, iSeqno2);
							handle t1 = timersManager.AddTimer(2000, octaneTimer1);
							handle t2 = timersManager.AddTimer(2000, octaneTimer2);
							Router.m_unAckBuffer[iSeqno1] = t1;
							Router.m_unAckBuffer[iSeqno2] = t2;
						}
						Router.printUnAckBuffer();
					}
					sendMsg(Router.iSockID, buffer, sizeof(buffer), rou2Addr);
				}
				else
				{
					bRefreshTimeout = false;
				}
			}
		}
		if (FD_ISSET(iSockID, &fdSet))
		{
			bRefreshTimeout = true;
			memset(buffer, 0, 2048);
			struct sockaddr_in rou2Addr;
			int nread = recvMsg(Router.iSockID, buffer, sizeof(buffer), rou2Addr);

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
				printf("Read a packet from secondary router, packet length:%d\n", nread);
				int iIcmpProtocol = icmpForward_log(Router, buffer, sizeof(buffer), FromUdp, ntohs(rou2Addr.sin_port));
				if (iIcmpProtocol == 1)// its a icmp pscket
				{
					cwrite(tun_fd, buffer, nread);// send packet back to tunnel
													//sendMsg(Router.iSockID, buffer, sizeof(buffer), rou1Addr);
													//icmpReply_primRouter(tun_fd, buffer, nread);
				}
				else if (iIcmpProtocol == OCTANE_PROTOCOL_NUM)
				{
					// check seqno and remove related record from the unack_buffer
					octane_control octMsg;
					int iSeqno = octaneUnpack(buffer, &octMsg);
					if (octMsg.octane_flags == 1)
					{
						// place for code to remove the related timer
						int iRmvHandle = Router.m_unAckBuffer[iSeqno];
						timersManager.RemoveTimer(iRmvHandle);
						Router.m_unAckBuffer.erase(iSeqno);
						Router.printUnAckBuffer();
					}
				}
				else
				{
					bRefreshTimeout = false;
				}
			}
		}
		if (bRefreshTimeout == true)
		{
			timeout.tv_sec = 15;
			timeout.tv_usec = 0;
		}
	}
}

void primaryRouter_s5(cRouter & Router, sockaddr_in &rou2Addr)
{
	int tun_fd = set_tunnel_reader();
	int iSockID = Router.iSockID;
	struct timeval idelTimeout;
	idelTimeout.tv_sec = 15;
	idelTimeout.tv_usec = 0;
	struct timeval timeout;
	timeout.tv_sec = 15;
	timeout.tv_usec = 0;
	bool bRefreshTimeout = true;
	Timers timersManager;
	fd_set fdSetAll, fdSet;
	FD_ZERO(&fdSetAll);
	FD_SET(tun_fd, &fdSetAll);
	FD_SET(iSockID, &fdSetAll);
	int iMaxfdpl = (tun_fd > iSockID) ? (tun_fd + 1) : (iSockID + 1);
	while (1)
	{
		bool bEventTimeout = false;
		fdSet = fdSetAll;
		struct timeval temp;
		temp.tv_sec = timeout.tv_sec;
		temp.tv_usec = timeout.tv_usec;
		timersManager.NextTimerTime(&timeout);
		if (timeout.tv_sec == 0 && timeout.tv_usec == 0) {
			cout << endl << " The timer at the head on the queue has expired " << endl;
			timersManager.ExecuteNextTimer();
			bEventTimeout = true;
		}
		if (timeout.tv_sec == MAXVALUE && timeout.tv_usec == 0) {
			cout << endl << "There are no timers in the event queue" << endl;
			timeout.tv_sec = temp.tv_sec;
			timeout.tv_usec = temp.tv_usec;
			bEventTimeout = false;
		}
		else
		{
			bEventTimeout = true;
		}
		int iSelect = select(iMaxfdpl, &fdSet, NULL, NULL, &timeout);
		if (iSelect == 0)
		{
			if (bEventTimeout == true)
			{
				cout << endl << "event time out!" << endl;
				// start of using the code of test-app.cc provided by csci551.
				// Timer expired, Hence process it 
				timersManager.ExecuteNextTimer();
				// Execute all timers that have expired.
				timersManager.NextTimerTime(&timeout);
				while (timeout.tv_sec == 0 && timeout.tv_usec == 0)
				{
					// Timer at the head of the queue has expired 
					timersManager.ExecuteNextTimer();
					timersManager.NextTimerTime(&timeout);
				}
				// end of using the code of test-app.cc provided by csci551.
				timeout.tv_sec = 15;
				timeout.tv_usec = 0;
			}
			else
			{
				cout << "timeout!" << endl;
				return;
			}

		}
		char buffer[2048];
		if (FD_ISSET(tun_fd, &fdSet))
		{
			bRefreshTimeout = true;
			memset(buffer, 0, 2048);
			int nread = read_tunnel(tun_fd, buffer, sizeof(buffer));

			flow_entry entry(buffer);
			string sCheck = Router.m_rouFlowTable.flowCheck(entry);

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
					struct octane_control localMsg, msg1, msg1_re;
					struct in_addr srcAddr, dstAddr;
					u_int8_t icmp_type;

					if (sCheck.size() != 0)
					{
						string sLog = "router: " + to_string(Router.iRouterID) + sCheck;
						cout << endl << sLog << endl;
						Router.vLog.push_back(sLog);
					}
					else
					{
						// create a orctane message for this primary router 
						Router.createOctaneMsg(localMsg, buffer, sizeof(buffer), 1, ntohs(rou2Addr.sin_port), false);
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
							//int iCheck2 = packetDstCheck(dstAddr, "10.5.51.4", "255.255.255.255");
							int iSeqno;
							//if (iCheck2 == 1)
							/*{
								iSeqno = Router.createOctaneMsg(msg1, buffer, sizeof(buffer), 3, -1);
							}
							else*/
							{
								iSeqno = Router.createOctaneMsg(msg1, buffer, sizeof(buffer), 2, -1);
							}
							char octaneIpBuffer[2048];
							memset(octaneIpBuffer, 0, 2048);
							string localAddr = "127.0.0.1";
							buildIpPacket(octaneIpBuffer, sizeof(octaneIpBuffer), 253, localAddr, localAddr, (char *)&msg1, sizeof(msg1));
							sendMsg(Router.iSockID, octaneIpBuffer, sizeof(octaneIpBuffer), rou2Addr); // send control message

																									   // Add timer and register the handle number																		   // Add timer and register the handle number
							cOctaneTimer * octaneTimer = new cOctaneTimer(Router.iSockID, rou2Addr, msg1, iSeqno);
							handle t1 = timersManager.AddTimer(2000, octaneTimer);
							Router.m_unAckBuffer[iSeqno] = t1;
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

																											// Add timer and register the handle number
							cOctaneTimer *octaneTimer1 = new cOctaneTimer(Router.iSockID, rou2Addr, msg1, iSeqno1);
							cOctaneTimer *octaneTimer2 = new cOctaneTimer(Router.iSockID, rou2Addr, msg1_re, iSeqno2);
							handle t1 = timersManager.AddTimer(2000, octaneTimer1);
							handle t2 = timersManager.AddTimer(2000, octaneTimer2);
							Router.m_unAckBuffer[iSeqno1] = t1;
							Router.m_unAckBuffer[iSeqno2] = t2;
						}
						Router.printUnAckBuffer();
					}
					sendMsg(Router.iSockID, buffer, sizeof(buffer), rou2Addr);
				}
				else
				{
					bRefreshTimeout = false;
				}
			}
		}
		if (FD_ISSET(iSockID, &fdSet))
		{
			bRefreshTimeout = true;
			memset(buffer, 0, 2048);
			struct sockaddr_in rou2Addr;
			int nread = recvMsg(Router.iSockID, buffer, sizeof(buffer), rou2Addr);

			flow_entry entry(buffer);
			string sCheck = Router.m_rouFlowTable.flowCheck(entry);
			
			if (nread < 0)
			{
				exit(1);
			}
			else
			{
				printf("Read a packet from secondary router, packet length:%d\n", nread);
				int iIcmpProtocol = icmpForward_log(Router, buffer, sizeof(buffer), FromUdp, ntohs(rou2Addr.sin_port));
				if (sCheck.size() != 0)
				{
					string sLog = "router: " + to_string(Router.iRouterID) + sCheck;
					cout << endl << sLog << endl;
					Router.vLog.push_back(sLog);
				}
				if (iIcmpProtocol == 1)// its a icmp pscket
				{
					cwrite(tun_fd, buffer, nread);// send packet back to tunnel
												  //sendMsg(Router.iSockID, buffer, sizeof(buffer), rou1Addr);
												  //icmpReply_primRouter(tun_fd, buffer, nread);
				}
				else if (iIcmpProtocol == OCTANE_PROTOCOL_NUM)
				{
					// check seqno and remove related record from the unack_buffer
					octane_control octMsg;
					int iSeqno = octaneUnpack(buffer, &octMsg);
					if (octMsg.octane_flags == 1)
					{
						// place for code to remove the related timer
						int iRmvHandle = Router.m_unAckBuffer[iSeqno];
						timersManager.RemoveTimer(iRmvHandle);
						Router.m_unAckBuffer.erase(iSeqno);
						Router.printUnAckBuffer();
					}
				}
				else
				{
					bRefreshTimeout = false;
				}
			}
		}
		if (bRefreshTimeout == true)
		{
			timeout.tv_sec = 15;
			timeout.tv_usec = 0;
		}
	}
}

void primaryRouter_s6(cRouter & Router)
{
	int tun_fd = set_tunnel_reader();
	int iSockID = Router.iSockID;
	struct timeval idelTimeout;
	idelTimeout.tv_sec = 15;
	idelTimeout.tv_usec = 0;
	struct timeval timeout;
	timeout.tv_sec = 15;
	timeout.tv_usec = 0;
	bool bRefreshTimeout = true;
	Timers timersManager;
	fd_set fdSetAll, fdSet;
	FD_ZERO(&fdSetAll);
	FD_SET(tun_fd, &fdSetAll);
	FD_SET(iSockID, &fdSetAll);
	int iMaxfdpl = (tun_fd > iSockID) ? (tun_fd + 1) : (iSockID + 1);

	// install reply rules on secondary Router
	//uint32_t src_ip = inet_addr("127.0.0.1");
	//uint32_t dst_ip = inet_addr("127.0.0.1");
	//uint16_t src_port = htons(0xFFFF);
	//uint16_t dst_port1 = htons(0xFFFF);
	//uint16_t dst_port2 = htons(0xFFFF);
	//uint16_t secondRouter1Port = htons(Router.m_mChildPort.begin()->second);
	//map<int, int>::iterator it;
	//it = Router.m_mChildPort.begin();
	//it++;
	//uint16_t secondRouter2Port = htons(it->second);
	//octane_control second1RouterMsg1(2, 0, Router.m_iSeqnoCnt++, src_ip, dst_ip, src_port, dst_port1, OCTANE_PROTOCOL_NUM, secondRouter1Port);
	//octane_control second1RouterMsg2(2, 0, Router.m_iSeqnoCnt++, src_ip, dst_ip, src_port, dst_port2, OCTANE_PROTOCOL_NUM, secondRouter2Port);
	//octane_control second1RouterMsg1Local(1, 0, 0, src_ip, dst_ip, src_port, dst_port1, OCTANE_PROTOCOL_NUM, 0);
	//octane_control second1RouterMsg2Local(1, 0, 0, src_ip, dst_ip, src_port, dst_port2, OCTANE_PROTOCOL_NUM, 0);
	uint16_t secondRouter1Port = htons(Router.m_mChildPort.begin()->second);
	map<int, int>::iterator it;
	it = Router.m_mChildPort.begin();
	it++;
	uint16_t secondRouter2Port = htons(it->second);

	while (1)
	{
		bool bEventTimeout = false;
		fdSet = fdSetAll;
		struct timeval temp;
		temp.tv_sec = timeout.tv_sec;
		temp.tv_usec = timeout.tv_usec;
		timersManager.NextTimerTime(&timeout);
		if (timeout.tv_sec == 0 && timeout.tv_usec == 0) {
			cout << endl << " The timer at the head on the queue has expired " << endl;
			timersManager.ExecuteNextTimer();
			bEventTimeout = true;
		}
		if (timeout.tv_sec == MAXVALUE && timeout.tv_usec == 0) {
			cout << endl << "There are no timers in the event queue" << endl;
			timeout.tv_sec = temp.tv_sec;
			timeout.tv_usec = temp.tv_usec;
			bEventTimeout = false;
		}
		else
		{
			bEventTimeout = true;
		}
		int iSelect = select(iMaxfdpl, &fdSet, NULL, NULL, &timeout);
		if (iSelect == 0)
		{
			if (bEventTimeout == true)
			{
				cout << endl << "event time out!" << endl;
				// start of using the code of test-app.cc provided by csci551.
				// Timer expired, Hence process it 
				timersManager.ExecuteNextTimer();
				// Execute all timers that have expired.
				timersManager.NextTimerTime(&timeout);
				while (timeout.tv_sec == 0 && timeout.tv_usec == 0)
				{
					// Timer at the head of the queue has expired 
					timersManager.ExecuteNextTimer();
					timersManager.NextTimerTime(&timeout);
				}
				// end of using the code of test-app.cc provided by csci551.
				timeout.tv_sec = 15;
				timeout.tv_usec = 0;
			}
			else
			{
				cout << "timeout!" << endl;
				return;
			}

		}
		char buffer[2048];
		if (FD_ISSET(tun_fd, &fdSet))
		{
			struct sockaddr_in rou2Addr;
			struct sockaddr_in targetAddr;
			setTempAddr("127.0.0.1", rou2Addr);
			rou2Addr.sin_port = htons(Router.m_mChildPort.begin()->second);
			bRefreshTimeout = true;
			memset(buffer, 0, 2048);
			int nread = read_tunnel(tun_fd, buffer, sizeof(buffer));

			flow_entry entry(buffer);
			string sCheck = Router.m_rouFlowTable.flowCheck(entry);

			if (nread < 0)
			{
				exit(1);
			}
			else
			{
				printf("Read a packet from tunnel, packet length:%d\n", nread);
				int a = icmpForward_log(Router, buffer, sizeof(buffer), FromTunnel, ntohs(rou2Addr.sin_port));// for FromTunnel "port" is not useful
				if (a == 1) // it is a ICMP packet
				{
					printf("Prim Router Read a ICMP packet \n", nread);
					struct octane_control localMsg, msg1, msg1_re;
					struct in_addr srcAddr, dstAddr;
					u_int8_t icmp_type;
					int iProtocolType = icmpUnpack(buffer, srcAddr, dstAddr, icmp_type);
					int iCheck = packetDstCheck(dstAddr, "10.5.51.11", "255.255.255.255");
					int iCheck2 = packetDstCheck(dstAddr, "10.5.51.12", "255.255.255.255");
					if (iCheck == 1 || iCheck2 == 1)
					{
						targetAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
						targetAddr.sin_family = AF_INET;
						if (iCheck == 1)
						{
							targetAddr.sin_port = secondRouter1Port;
						}
						else
						{
							targetAddr.sin_port = secondRouter2Port;
						}
					}
					else
					{
						targetAddr.sin_port = secondRouter1Port;
					}
					if (sCheck.size() != 0)
					{
						string sLog = "router: " + to_string(Router.iRouterID) + sCheck;
						cout << endl << sLog << endl;
						Router.vLog.push_back(sLog);
					}
					else
					{
						// create a orctane message for this primary router 
						Router.createOctaneMsg(localMsg, buffer, sizeof(buffer), 1, ntohs(targetAddr.sin_port), false);
						//insert rules in flow_table and get the respective log
						vector<string> tempLog = Router.m_rouFlowTable.dbInsert(localMsg);
						for (int i = 0; i < tempLog.size(); i++)
						{
							string sLog = "router: " + to_string(Router.iRouterID) + tempLog[i];
							Router.vLog.push_back(sLog);
						}
						int iCheckDef = packetDstCheck(dstAddr, "10.5.51.0", "255.255.255.0");
						if (iCheckDef == 1)
						{
							int iSeqno;
							iSeqno = Router.createOctaneMsg(msg1, buffer, sizeof(buffer), 2, -1);
							char octaneIpBuffer[2048];
							memset(octaneIpBuffer, 0, 2048);
							string localAddr = "127.0.0.1";
							buildIpPacket(octaneIpBuffer, sizeof(octaneIpBuffer), 253, localAddr, localAddr, (char *)&msg1, sizeof(msg1));
							sendMsg(Router.iSockID, octaneIpBuffer, sizeof(octaneIpBuffer), targetAddr); // send control message
																										 // Add timer and register the handle number
							cOctaneTimer * octaneTimer = new cOctaneTimer(Router.iSockID, targetAddr, msg1, iSeqno);
							handle t1 = timersManager.AddTimer(2000, octaneTimer);
							Router.m_unAckBuffer[iSeqno] = t1;
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

							// Add timer and register the handle number
							cOctaneTimer *octaneTimer1 = new cOctaneTimer(Router.iSockID, rou2Addr, msg1, iSeqno1);
							cOctaneTimer *octaneTimer2 = new cOctaneTimer(Router.iSockID, rou2Addr, msg1_re, iSeqno2);
							handle t1 = timersManager.AddTimer(2000, octaneTimer1);
							handle t2 = timersManager.AddTimer(2000, octaneTimer2);
							Router.m_unAckBuffer[iSeqno1] = t1;
							Router.m_unAckBuffer[iSeqno2] = t2;
						}
						Router.printUnAckBuffer();
					}
					sendMsg(Router.iSockID, buffer, sizeof(buffer), targetAddr);
				}
				else if (a == 6)
				{
					int iProtocolType = tcpUnpack(buffer);
					targetAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
					targetAddr.sin_family = AF_INET;
					if (iProtocolType == 80)
					{
						targetAddr.sin_port = secondRouter1Port;
					}
					else
					{
						targetAddr.sin_port = secondRouter2Port;
					}
					if (sCheck.size() != 0)
					{
						string sLog = "router: " + to_string(Router.iRouterID) + sCheck;
						cout << endl << sLog << endl;
						Router.vLog.push_back(sLog);
					}
					else
					{
						;
					}
				}
				else
				{
					bRefreshTimeout = false;
				}
			}
		}
		if (FD_ISSET(iSockID, &fdSet))
		{
			bRefreshTimeout = true;
			memset(buffer, 0, 2048);
			struct sockaddr_in rou2Addr;
			int nread = recvMsg(Router.iSockID, buffer, sizeof(buffer), rou2Addr);

			flow_entry entry(buffer);
			string sCheck = Router.m_rouFlowTable.flowCheck(entry);
			
			if (nread < 0)
			{
				exit(1);
			}
			else
			{
				printf("Read a packet from secondary router, packet length:%d\n", nread);
				int iIcmpProtocol = icmpForward_log(Router, buffer, sizeof(buffer), FromUdp, ntohs(rou2Addr.sin_port));
				if (sCheck.size() != 0)
				{
					string sLog = "router: " + to_string(Router.iRouterID) + sCheck;
					cout << endl << sLog << endl;
					Router.vLog.push_back(sLog);
				}
				if (iIcmpProtocol == 1 || iIcmpProtocol == 6)// its a icmp or TCP pscket
				{
					cwrite(tun_fd, buffer, nread);// send packet back to tunnel
												  //sendMsg(Router.iSockID, buffer, sizeof(buffer), rou1Addr);
												  //icmpReply_primRouter(tun_fd, buffer, nread);
				}
				else if (iIcmpProtocol == OCTANE_PROTOCOL_NUM)
				{
					// check seqno and remove related record from the unack_buffer
					octane_control octMsg;
					int iSeqno = octaneUnpack(buffer, &octMsg);
					if (octMsg.octane_flags == 1)
					{
						// place for code to remove the related timer
						int iRmvHandle = Router.m_unAckBuffer[iSeqno];
						timersManager.RemoveTimer(iRmvHandle);
						Router.m_unAckBuffer.erase(iSeqno);
						Router.printUnAckBuffer();
					}
				}
				else
				{
					bRefreshTimeout = false;
				}
			}
		}
		if (bRefreshTimeout == true)
		{
			timeout.tv_sec = 15;
			timeout.tv_usec = 0;
		}
	}
}

int cOctaneTimer::Expire()// use the code of test-app.cc provided by csci551.
{
	struct timeval tv; 

	getTime(&tv);
	fprintf(stderr, "Timer of Seq number %d has expired! Time %d.%06d\n",
		m_iSeq, (int)tv.tv_sec, (int)tv.tv_usec);
	fflush(NULL);

	char octaneIpBuffer[2048];
	memset(octaneIpBuffer, 0, 2048);
	string localAddr = "127.0.0.1";
	buildIpPacket(octaneIpBuffer, sizeof(octaneIpBuffer), OCTANE_PROTOCOL_NUM, localAddr, localAddr, (char *)&m_iOctaneMsg, sizeof(m_iOctaneMsg));
	sendMsg(m_iSockID, octaneIpBuffer, sizeof(octaneIpBuffer), m_rou2Addr);// send control message

	return TimerCallback::RESCHEDULE_SAME;
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

void stage5(cRouter &Router,
	struct sockaddr_in & rou1Addr,
	struct sockaddr_in & rou2Addr)
{
	stage1(Router, rou1Addr, rou2Addr);
	if (Router.iFPID == 0) // if it is secondary router
	{
		secondRouter_s5(Router);
	}
	else// if it is primary router
	{
		primaryRouter_s5(Router, rou2Addr);
	}
}

void stage6(cRouter &Router,
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
	for (int i = 0; i < Router.iRouteNum ; i++)
	{
		if (Router.iFPID != 0)
		{
			fPid = fork();
			if (fPid<0)
			{
				cout << " fork : error" << endl;
			}
			else if (fPid == 0)
			{
				secondRouter_reqReg(Router, rou1Addr, i+1);
				Router.iFPID = fPid;
			}
			else
			{
				//cout << "child process pid: " << fPid << endl << endl;
				Router.iFPID = fPid;
				primaryRouter_reg(sockID, Router);
			}
		}
	}

	if (Router.iFPID != 0)
	{
		vector<string> &vLog = Router.vLog;
		string temp = "primary port: " + to_string(Router.iPortNum);
		vLog.push_back(temp);
		int cnt = 0;
		for (auto i : Router.m_mChildPort)
		{
			cout << "pid: " << i.first << "; port: " << i.second << endl;
			string temp2 = "router: " + to_string(cnt+1) + ", pid: " + to_string(i.first) + ", port: " + to_string(i.second);
			vLog.push_back(temp2);
			cnt++;
		}
		
	}
	
	

	if (Router.iFPID == 0) // if it is secondary router
	{
		secondRouter_s6(Router);
	}
	else// if it is primary router
	{
		primaryRouter_s6(Router);
	}
}



void primaryRouter_reg(const int sockID, cRouter & Router)
{
	struct sockaddr_in rou2Addr;
	Router.iSockID = sockID;
	vector<string> &vLog = Router.vLog;
	
	char buf[1024], pRou2Addr[16];
	memset(buf, 0, 1024);
	recvMsg(sockID, buf, 1024, rou2Addr);
	string sMsgRecv(buf);
	inet_ntop(AF_INET, &rou2Addr.sin_addr, pRou2Addr, sizeof(pRou2Addr));
	int iRou2Port = ntohs(rou2Addr.sin_port);
	Router.m_mChildPort[stoi(sMsgRecv)] = iRou2Port;
	
}
