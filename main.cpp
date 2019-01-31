#include "projectA.h"

int main(int argc, char * argv[])
{
	char* configFilePath;
	int sockID;
	socklen_t len;
	struct sockaddr_in rou1Addr,rou2Addr,locAddr;
	
	sockID = socket(AF_INET,SOCK_DGRAM,0);

	bzero(&locAddr,sizeof(locAddr));
	locAddr.sin_family = AF_INET;
	locAddr.sin_port = htons(SERV_PORT);
	inet_pton(AF_INET,"127.0.0.1",&locAddr.sin_addr);
	
	connect(sockID,(struct sockaddr*) &locAddr,sizeof(locAddr));
	len = sizeof(rou1Addr);
	getsockname(sockID,(struct sockaddr*) &rou1Addr, &len);
	close(sockID);
	sockID = socket(AF_INET,SOCK_DGRAM,0);
	char pRou1Addr[16], pRou2Addr[16];
	unsigned int iRou1Port,iRou2Port;
	inet_ntop(AF_INET,&rou1Addr.sin_addr,pRou1Addr,sizeof(pRou1Addr));
	iRou1Port = ntohs(rou1Addr.sin_port);
	int iBind = bind(sockID,(struct sockaddr*)&rou1Addr,sizeof(rou1Addr));
	if(iBind<0)
	{
		perror("bind error");
	}
	cout<<"local address:"<<pRou1Addr<<endl;
	cout<<"port: "<<iRou1Port<<endl;
	
	for (int i=1;i<argc;i++)
	{
		configFilePath = argv[i];
	}
	cRouter Router;
	Router.readConfigFile(configFilePath);
	pid_t fPid;
	fPid = fork();
	if(fPid<0)
	{
		cout<<" fork : error"<<endl;
	}
	else if (fPid==0)
	{
		Router.iRouterID = 1;
		vector<string> vLog;
		string temp = "second router"+to_string(getpid());
		vLog.push_back(temp);
		Router.writeLogFile(vLog);
		char buf[1024];
		memset(buf,0,1024);
		len = sizeof(rou2Addr);
		int count = recvfrom(sockID, buf, 1024,0,(struct sockaddr*)&rou2Addr,&len);
		if(count==-1)
		{ cout<<"receive data fail!";}
		else
		{
			printf("client %s \n", buf);
		}
		inet_ntop(AF_INET,&rou2Addr.sin_addr,pRou1Addr,sizeof(pRou1Addr));
		iRou1Port = ntohs(rou2Addr.sin_port);
		cout<<"router1: sender address:"<<pRou1Addr<<endl;
		cout<<"router1: sender port: "<<iRou1Port<<endl;
	}
	else
	{
		vector<string> vLog;
		string temp = "primary router"+to_string(getpid());
		vLog.push_back(temp);
		Router.writeLogFile(vLog);
		int	sockID2 = socket(AF_INET,SOCK_DGRAM,0);
	char buf[1024]="test udp!";
	socklen_t len2 = sizeof(rou1Addr);
	sendto(sockID2,buf,1024,0,(struct sockaddr*)&rou1Addr,len2);
	getsockname(sockID2,(struct sockaddr*) &rou2Addr, &len);
	socklen_t len = sizeof(rou2Addr);
	unsigned int iRou2Port;
	char pRou2Addr[16];
	inet_ntop(AF_INET,&rou2Addr.sin_addr,pRou2Addr,sizeof(pRou2Addr));
	iRou2Port = ntohs(rou2Addr.sin_port);
	cout<<"router2: local address: "<<pRou2Addr<<endl;
	cout<<"router2: local  port: "<<iRou2Port<<endl;
	cout<<"router2: receiver  address:"<<pRou1Addr<<endl;
	cout<<"router2: receiver port: "<<iRou1Port<<endl;

	}	
}
