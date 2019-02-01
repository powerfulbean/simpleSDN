#include "cRouter.h"

int main(int argc, char * argv[])
{
	char* configFilePath;
	int sockID;
	socklen_t len;
	struct sockaddr_in rou1Addr,locAddr;
	cRouter Router;
	for (int i=1;i<argc;i++)
	{
		configFilePath = argv[i];
	}
	Router.readConfigFile(configFilePath);
	setTempAddr("127.0.0.1",locAddr);
	getDynmcPortSrv(locAddr,rou1Addr);
	sockID = getUdpSocket();
	char pRou1Addr[16];
	unsigned int iRou1Port;
	inet_ntop(AF_INET,&rou1Addr.sin_addr,pRou1Addr,sizeof(pRou1Addr));// translate the router 1 ip address to ascii
	iRou1Port = ntohs(rou1Addr.sin_port);//translate the router 1 port  to ascii
	Router.iPortNum = iRou1Port;
	int iBind = bind(sockID,(struct sockaddr*)&rou1Addr,sizeof(rou1Addr));
	if(iBind<0)
	{
		perror("bind error");
	}
	cout<<"local address:"<<pRou1Addr<<endl;
	cout<<"primary port: "<<iRou1Port<<endl;
	
	
	pid_t fPid;
	fPid = fork();
	if(fPid<0)
	{
		cout<<" fork : error"<<endl;
	}
	else if (fPid==0)
	{
		secondRouter(Router, rou1Addr);
	}
	else
	{
		cout<<"child process pid: "<<fPid<<endl;
		primaryRouter(sockID,Router,rou1Addr);	
	}	
}
