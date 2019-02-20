#include "network.h"

int getUdpSocket()
{
        return socket(AF_INET,SOCK_DGRAM,0);
}

int getIcmpRawSocket()
{
	return socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
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

void sendMsg(int sockID2,const char* buf,unsigned int iSize,
	      const struct sockaddr_in rou1Addr)
{
	socklen_t len = sizeof(rou1Addr);
	sendto(sockID2,buf,iSize,0,(struct sockaddr*)&rou1Addr,len);
	char pRou1Addr[16];
        unsigned int iRou1Port;
        inet_ntop(AF_INET,&rou1Addr.sin_addr,pRou1Addr,sizeof(pRou1Addr));// translate the router 1 ip address to ascii
        iRou1Port = ntohs(rou1Addr.sin_port);//translate the router 1 port  to ascii
	cout<<"sendMsg: receiver  address:"<<pRou1Addr<<endl;
        cout<<"sendMsg: receiver port: "<<iRou1Port<<endl;
	cout<<"sendMsg: end"<<endl<<endl;
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
		printf("recvMsg: sender said %s \n", buf);
        }
	int iRou2Port = ntohs(rou2Addr.sin_port);
	inet_ntop(AF_INET,&rou2Addr.sin_addr,pRou2Addr,sizeof(pRou2Addr));
	cout<<"recvMsg: sender address:"<<pRou2Addr<<endl;
        cout<<"recvMsg: sender port: "<<iRou2Port<<endl;
        cout<<"recvMsg: end"<<endl;
	cout<<endl;
	return count;
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
		return 0;
	}

	srcAddr = pIpHeader->ip_src;
	dstAddr = pIpHeader->ip_dst;

	unsigned int iIpHeaderLen = pIpHeader->ip_hl << 2;
	pIcmp = (struct icmp *)(buffer + iIpHeaderLen);

	// get ICMP_echoReply
	icmp_type = pIcmp->icmp_type;
	printf("src address: %s  ", inet_ntoa(pIpHeader->ip_src));
	printf("dst address: %s  ", inet_ntoa(pIpHeader->ip_dst));
	printf("service type: %d  ", pIpHeader->ip_p);
	printf("icmp type: %d", pIcmp->icmp_type);
	cout<<endl;

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
        printf("service type: %d  ",pIpHeader->ip_p);
	
}


struct in_addr icmpReply_Edit(struct in_addr AddrForReplace, char* buffer, int iFlag)
{
	struct ip * pIpHeader;
	struct icmp * pIcmp;
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
		printf("service type: %d  ", pIpHeader->ip_p);
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
		printf("service type: %d  ", pIpHeader->ip_p);
	}
	
	return replacedAddr;
}


