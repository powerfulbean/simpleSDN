#include "network.h"

int getUdpSocket()
{
        return socket(AF_INET,SOCK_DGRAM,0);
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
	
void recvMsg(int sockID, char *buf, unsigned int iSize,
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
}


