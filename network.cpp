#include "network.h"

int getUdpSocket()
{
        return socket(AF_INET,SOCK_DGRAM,0);
}

int getIcmpRawSocket()
{
	return socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
}

int getRawSocket(int protocol)
{
	if (protocol == 6)
	{
		return socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
	}
	return socket(AF_INET, SOCK_RAW, protocol);
}

void setTempAddr(const char* pIp, struct sockaddr_in & locAddr)
{
        bzero(&locAddr,sizeof(locAddr));
        locAddr.sin_family = AF_INET;
        locAddr.sin_port = htons(SERV_PORT);
        inet_pton(AF_INET,pIp,&locAddr.sin_addr);
        return;
}

void getDynmcPortSrv(const struct sockaddr_in & locAddr,
                        struct sockaddr_in & outputAddr)
{
        int sockID = getUdpSocket();
        socklen_t len;
        connect(sockID,(struct sockaddr*) &locAddr,sizeof(locAddr));
        len = sizeof(outputAddr);
        getsockname(sockID,(struct sockaddr*) &outputAddr, &len);
        close(sockID);
}

int sendMsg(int sockID2,const char* buf,unsigned int iSize,
	      const struct sockaddr_in rou1Addr)
{
	socklen_t len = sizeof(rou1Addr);
	int err = sendto(sockID2,buf,iSize,0,(struct sockaddr*)&rou1Addr,len);
	if (err == -1)
	{
		perror("sendMsg error");
	}
	char pRou1Addr[16];
        //unsigned int iRou1Port;
        inet_ntop(AF_INET,&rou1Addr.sin_addr,pRou1Addr,sizeof(pRou1Addr));// translate the router 1 ip address to ascii
        //iRou1Port = ntohs(rou1Addr.sin_port);//translate the router 1 port  to ascii
	//	cout << endl;
	//	cout<<"sendMsg: receiver  address:"<<pRou1Addr<<endl;
    //    cout<<"sendMsg: receiver port: "<<iRou1Port<<endl;
	//cout<<"sendMsg: end"<<endl<<endl;
	return err;
}
	
int  recvMsg(int sockID, char *buf, unsigned int iSize,
                struct sockaddr_in & rou2Addr)
{
	socklen_t len;
	char pRou2Addr[16];
	len = sizeof(rou2Addr);
	int count = recvfrom(sockID, buf, 1024,0,(struct sockaddr*)&rou2Addr,&len);
        //      cout<<"udp receive"<<endl;
        if(count==-1)
        { cout<<"receive data fail!";}
        else
        {
		//printf("recvMsg: sender said %s \n", buf);
        }
	//int iRou2Port = ntohs(rou2Addr.sin_port);
	inet_ntop(AF_INET,&rou2Addr.sin_addr,pRou2Addr,sizeof(pRou2Addr));
	//cout << endl;
	//cout<<"recvMsg: sender address:"<<pRou2Addr<<endl;
    //    cout<<"recvMsg: sender port: "<<iRou2Port<<endl;
    //    cout<<"recvMsg: end"<<endl;
	//cout<<endl;
	return count;
}


int octaneUnpack(char* buffer, struct octane_control *pOutOctane)
{
	struct ip * pIpHeader;
	struct octane_control * pOctane;
	//char *pA;
	pIpHeader = (struct ip *) buffer;

	unsigned int iIpHeaderLen = pIpHeader->ip_hl << 2;
	short iOctaneTotLen;
	if (iIpHeaderLen == 0)
	{
		iIpHeaderLen = 20;
	}

	pOctane = (struct octane_control *)(buffer + iIpHeaderLen);
	if (pIpHeader->ip_len == 0)
	{
		iOctaneTotLen = 20;
	}
	else
	{
		iOctaneTotLen = ntohs(pIpHeader->ip_len) - iIpHeaderLen; // this part is learnt from
	} 
	memcpy(pOutOctane, pOctane, iOctaneTotLen);
	return pOctane->octane_seqno;
}

int  recvMsgSafe(int sockID, char *buffer, unsigned int iSize,
	struct sockaddr_in & rou2Addr)
{
	struct iovec iov2;
	struct msghdr msg2;
	iov2.iov_base = buffer;
	iov2.iov_len = iSize;
	msg2.msg_name = &rou2Addr;
	msg2.msg_namelen = sizeof(rou2Addr);
	msg2.msg_iov = &iov2;
	msg2.msg_iovlen = 1;
	msg2.msg_control = NULL;
	msg2.msg_controllen = 0;
	msg2.msg_flags = 0;
	int err = recvmsg(sockID, &msg2, 0);
	if (err == -1)
	{
		perror("primRouter_tcpRaw error: recvmsg");
	}
	else
	{
		perror("primRouter_tcpRaw success: recvmsg");
		printf(": msglen : %d  \n", err);
	}
	return err;
}

void octaneReply_Edit(char* buffer)
{
	struct ip * pIpHeader;
	struct octane_control * pOctane;
	pIpHeader = (struct ip *) buffer;

	if (pIpHeader->ip_p != OCTANE_PROTOCOL_NUM)
	{

		return;
	}

	unsigned int iIpHeaderLen = pIpHeader->ip_hl << 2;
	pOctane = (struct octane_control *)(buffer + iIpHeaderLen);
	pOctane->octane_flags = 1;
	return;
}

/* About the function:
	icmpUnpack(char* buffer, struct in_addr &srcAddr, struct in_addr &dstAddr, u_int8_t &icmp_type)
	was built based on the itroduction of ipheader length calculation by "zthgreat"
	" https://blog.csdn.net/u014634338/article/details/48951345"
*/

int icmpUnpack(char* buffer, struct in_addr &srcAddr, struct in_addr &dstAddr, u_int8_t &icmp_type)
{
	struct ip * pIpHeader;
	struct icmp * pIcmp;
	pIpHeader = (struct ip *) buffer;

	if (pIpHeader->ip_p != 1)
	{
		if (pIpHeader->ip_p == 6)
		{
			srcAddr = pIpHeader->ip_src;
			dstAddr = pIpHeader->ip_dst;
		}
		return pIpHeader->ip_p;
	}

	srcAddr = pIpHeader->ip_src;
	dstAddr = pIpHeader->ip_dst;

	unsigned int iIpHeaderLen = pIpHeader->ip_hl << 2;
	pIcmp = (struct icmp *)(buffer + iIpHeaderLen);

	// get ICMP_echoReply
	icmp_type = pIcmp->icmp_type;
	//printf("src address: %s  ", inet_ntoa(pIpHeader->ip_src));
	//printf("dst address: %s  ", inet_ntoa(pIpHeader->ip_dst));
	//printf("service type: %d  ", pIpHeader->ip_p);
	printf("icmp type: %d", pIcmp->icmp_type);
	cout<<endl;

	return 1;
}

int icmpUnpack(char* buffer, struct icmphdr &icmphdr, struct in_addr &srcAddr, struct in_addr &dstAddr, u_int8_t &icmp_type)
{
	struct ip * pIpHeader;
	struct icmp * pIcmp;
	pIpHeader = (struct ip *) buffer;

	if (pIpHeader->ip_p != 1)
	{
		return 0;
	}

	srcAddr = pIpHeader->ip_src;
	dstAddr = pIpHeader->ip_dst;

	unsigned int iIpHeaderLen = pIpHeader->ip_hl << 2;
	pIcmp = (struct icmp *)(buffer + iIpHeaderLen);
	memcpy(&icmphdr, pIcmp, sizeof(icmphdr));
	// get ICMP_echoReply
	icmp_type = pIcmp->icmp_type;
	//printf("src address: %s  ", inet_ntoa(pIpHeader->ip_src));
	//printf("dst address: %s  ", inet_ntoa(pIpHeader->ip_dst));
	//printf("service type: %d  ", pIpHeader->ip_p);
	printf("icmp type: %d", pIcmp->icmp_type);
	//cout << endl;

	return 1;
}

int tcpUnpack(char* buffer) // return dst port
{
	struct ip * pIpHeader;
	struct tcphdr * pTcp;
	pIpHeader = (struct ip *) buffer;

	if (pIpHeader->ip_p != 6)
	{
		return 0;
	}

	unsigned int iIpHeaderLen = pIpHeader->ip_hl << 2;
	pTcp = (struct tcphdr *)(buffer + iIpHeaderLen);

	return ntohs(pTcp->dest);
}
int tcpUnpackSrc(char* buffer) // return src port
{
	struct ip * pIpHeader;
	struct tcphdr * pTcp;
	pIpHeader = (struct ip *) buffer;

	if (pIpHeader->ip_p != 6)
	{
		return 0;
	}

	unsigned int iIpHeaderLen = pIpHeader->ip_hl << 2;
	pTcp = (struct tcphdr *)(buffer + iIpHeaderLen);

	return ntohs(pTcp->source);
}

void buildIpPacket(char* buffer, unsigned int iBufferSize,int iProtocol, string sSrcAddr, string sDstAddr, char* payload, unsigned int iPayloadSize)
{

	int iIP4_HDRLEN = 20;
	if (iBufferSize < iIP4_HDRLEN + iPayloadSize)
	{
		cout << "buildIpPacket error: the buffer is too small! \n";
		return;
	}
	struct ip IpHeader;
	IpHeader.ip_hl = iIP4_HDRLEN / sizeof(uint32_t);
	IpHeader.ip_v = 4;
	IpHeader.ip_tos = 0;
	IpHeader.ip_len = htons(iIP4_HDRLEN + iPayloadSize);
	IpHeader.ip_id = htons(0);
	IpHeader.ip_off = 0;
	IpHeader.ip_p = iProtocol;
	IpHeader.ip_src.s_addr = inet_addr(sSrcAddr.data());
	IpHeader.ip_dst.s_addr = inet_addr(sDstAddr.data());
	IpHeader.ip_ttl = 255;
	IpHeader.ip_sum = checksum((char*)&IpHeader, iIP4_HDRLEN);
	
	memcpy(buffer, &IpHeader, iIP4_HDRLEN * sizeof(uint8_t));
	memcpy(buffer + iPayloadSize, payload, iPayloadSize * sizeof(uint8_t));

	return;

}

int ipUnpack(const char* buffer, uint32_t &sSrc_addr, uint32_t &sDst_addr, uint16_t &sSrc_port, uint16_t &sDst_port, u_int8_t &ip_type)
{
	struct ip * pIpHeader;
	pIpHeader = (struct ip *) buffer;

	ip_type = pIpHeader->ip_p;
	sSrc_addr = pIpHeader->ip_src.s_addr;
	sDst_addr = pIpHeader->ip_dst.s_addr;

	if (ip_type == 1)
	{
		sSrc_port = htons(0xFFFF);
		sDst_port = htons(0xFFFF);
		cout << "network.cpp: ipUnpack: it is a ICMP packet: port: " << to_string(sDst_port);
	}
	else if (ip_type == 6)
	{
		struct tcphdr * pTcp;
		unsigned int iIpHeaderLen = pIpHeader->ip_hl << 2;
		pTcp = (struct tcphdr *)(buffer + iIpHeaderLen);
		sSrc_port = pTcp->source;
		sDst_port = pTcp->dest;
	}
	return 1;
}
/* About the function:
void icmpReply_Edit(char* buffer)
was built based on the itroduction of netinet/ip.h ipheader on
"https://www.ibm.com/developerworks/cn/linux/network/ping/index.html"
*/

void icmpReply_Edit(char* buffer)
{
        struct ip * pIpHeader;
	struct icmp * pIcmp;
	pIpHeader = (struct ip *) buffer;

	if (pIpHeader->ip_p != 1)
	{
		return;
	}

	printf("src address: %s  ",inet_ntoa(pIpHeader->ip_src));
        printf("dst address: %s  ",inet_ntoa(pIpHeader->ip_dst));
        printf("service type: %d  ",pIpHeader->ip_p);

	unsigned int iIpHeaderLen = pIpHeader->ip_hl<<2;
	pIcmp = (struct icmp *)(buffer + iIpHeaderLen);

	// edit ICMP_echoReply
	pIcmp->icmp_type = ICMP_ECHOREPLY;
	pIcmp->icmp_cksum = 0;
	short iIcmpTotLen = ntohs(pIpHeader->ip_len) - iIpHeaderLen; // this part is learnt from 
	pIcmp->icmp_cksum = checksum((char *)pIcmp, iIcmpTotLen); // the checksum funtion was provided by cs551 class moodle

	// edit IP packet
	struct in_addr tempAddr = pIpHeader->ip_dst;
	pIpHeader->ip_dst = pIpHeader->ip_src;
	pIpHeader->ip_src = tempAddr;
	pIpHeader->ip_sum = 0;
	pIpHeader->ip_sum = checksum((char*)pIpHeader, iIpHeaderLen);

		
        printf("src address: %s  ",inet_ntoa(pIpHeader->ip_src));
        printf("dst address: %s  ",inet_ntoa(pIpHeader->ip_dst));
        printf("service type: %d  \n",pIpHeader->ip_p);
	
}

int tcpReply_Edit(struct in_addr oriSrcAddr, char* buffer3)
{
	// recal checksum of tcp
	char psdBuffer[65535] = { 0 };
	struct ip * pIpHeader;
	struct tcphdr * pTcp, *pTcp_psd;
	struct psdhdr* pPsd;
	pIpHeader = (struct ip *) buffer3;
	unsigned int iIpHeaderLen = pIpHeader->ip_hl << 2;
	pTcp = (struct tcphdr *)(buffer3 + iIpHeaderLen);
	short iTcpTotLen = ntohs(pIpHeader->ip_len) - iIpHeaderLen;
	pTcp_psd = (struct tcphdr *)(psdBuffer + sizeof(struct psdhdr));
	pPsd = (struct psdhdr *) psdBuffer;

	// edit IP packet
	pIpHeader->ip_dst = oriSrcAddr;
	pIpHeader->ip_sum = 0;
	pIpHeader->ip_sum = checksum((char*)pIpHeader, iIpHeaderLen);

	// calculate check sum
	pPsd->saddr = pIpHeader->ip_src.s_addr;//pIpHeader->ip_src.s_addr;
	pPsd->daddr = oriSrcAddr.s_addr;
	pPsd->mbz = 0;
	pPsd->protocol = pIpHeader->ip_p;
	pPsd->tcpl = htons(iTcpTotLen); //htons(sizeof(struct tcphdr));
	//cout << endl <<"ip len: "<<ntohs(pIpHeader->ip_len)<< " tcp len: " << iTcpTotLen << endl;
	//printf("tcpEdit ori check sum 1: %x \n", pTcp->check);
	pTcp->check = 0;
	memcpy(pTcp_psd, pTcp, iTcpTotLen);
	//printf("tcpEdit ori check sum equal: %x == %x ? \n", pTcp->check, pTcp_psd->check);
	//cout << endl << "psdhdr len: " << sizeof(struct psdhdr) << " " << sizeof(psdhdr) << endl;
	pTcp->check = checksum(psdBuffer, iTcpTotLen + sizeof(struct psdhdr));
	//printf("tcpEdit ori check sum 2: %x \n", pTcp->check);
	// end of recal tcp checksum
	cout << endl << "tcpEdit: msgLen is: " << ntohs(pIpHeader->ip_len) << endl;
	return ntohs(pIpHeader->ip_len);
}

void ipChangeProtocol(char* buffer, int iProtocol)
{
	struct ip * pIpHeader;
	pIpHeader = (struct ip *) buffer;
	unsigned int iIpHeaderLen = pIpHeader->ip_hl << 2;
															  // edit IP packet
	struct in_addr tempAddr = pIpHeader->ip_dst;
	pIpHeader->ip_dst = pIpHeader->ip_src;
	pIpHeader->ip_src = tempAddr;
	pIpHeader->ip_p = iProtocol;
	pIpHeader->ip_sum = 0;
	pIpHeader->ip_sum = checksum((char*)pIpHeader, iIpHeaderLen);
	printf("service type after changing: %d  \n", pIpHeader->ip_p);
}

struct in_addr icmpReply_Edit(struct in_addr AddrForReplace, char* buffer, int iFlag)// actually edit the IP packet
{
	struct ip * pIpHeader;
	//struct icmp * pIcmp;
	struct in_addr replacedAddr;
	pIpHeader = (struct ip *) buffer;

	if (pIpHeader->ip_p != 1)
	{

		return pIpHeader->ip_src;
	}

	printf("src address: %s  ", inet_ntoa(pIpHeader->ip_src));
	printf("dst address: %s  ", inet_ntoa(pIpHeader->ip_dst));
	printf("service type: %d  ", pIpHeader->ip_p);

	unsigned int iIpHeaderLen = pIpHeader->ip_hl << 2;

	if (iFlag == FromUdp)
	{
		// edit IP packet
		replacedAddr = pIpHeader->ip_src;
		pIpHeader->ip_src = AddrForReplace;
		pIpHeader->ip_sum = 0;
		pIpHeader->ip_sum = checksum((char*)pIpHeader, iIpHeaderLen);


		printf("src after replacement address: %s  ", inet_ntoa(pIpHeader->ip_src));
		printf("dst address: %s  ", inet_ntoa(pIpHeader->ip_dst));
		printf("service type: %d  \n", pIpHeader->ip_p);
	}
	else if (iFlag == FromRawSock)
	{
		// edit IP packet
		replacedAddr = pIpHeader->ip_dst;
		pIpHeader->ip_dst = AddrForReplace;
		pIpHeader->ip_sum = 0;
		pIpHeader->ip_sum = checksum((char*)pIpHeader, iIpHeaderLen);


		printf("src address: %s  ", inet_ntoa(pIpHeader->ip_src));
		printf("dst after replacement address: %s  ", inet_ntoa(pIpHeader->ip_dst));
		printf("service type: %d  \n", pIpHeader->ip_p);
	}
	
	return replacedAddr;
}

u_int8_t getIcmpType(char* buffer)
{
	struct ip * pIpHeader;
	struct icmp * pIcmp;
	pIpHeader = (struct ip *) buffer;

	if (pIpHeader->ip_p != 1)
	{
		//printf("\n not  icmp %x ",pIpHeader->ip_p);
		printf("\n not  icmp %d \n ",pIpHeader->ip_p);
		return -1;
	}

	unsigned int iIpHeaderLen = pIpHeader->ip_hl << 2;
	pIcmp = (struct icmp *)(buffer + iIpHeaderLen);

	return pIcmp->icmp_type;
}

bool octane_control::operator< (const octane_control key2) const
{
	if (octane_source_ip != key2.octane_source_ip  && octane_source_ip != htonl(0xFFFFFFFF))
	{
		return octane_source_ip < key2.octane_source_ip;
	}
	else
	{
		if (octane_source_port != key2.octane_source_port && octane_source_port != htons(0xFFFF))
		{
			return octane_source_port < key2.octane_source_port;
		}
		else
		{
			if (octane_dest_ip != key2.octane_dest_ip && octane_dest_ip != htonl(0xFFFFFFFF))
			{
				return octane_dest_ip < key2.octane_dest_ip;
			}
			else
			{
				if (octane_dest_port != key2.octane_dest_port && octane_dest_port != htons(0xFFFF))
				{
					return octane_dest_port < key2.octane_dest_port;
				}
				else
				{
					if (octane_action < key2.octane_action)
					{
						return octane_action < key2.octane_action;
					}
					else
					{
						return octane_port < key2.octane_port;
					}
				}
			}
		}
	}
}