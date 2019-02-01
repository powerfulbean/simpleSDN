#include "projectA.h"

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
		vector<string> vLog;
		struct sockaddr_in rou2Addr;
		socklen_t len = sizeof(rou2Addr);
		string sPid = to_string(getpid());
		Router.iRouterID = 1;
		int sockID2 = socket(AF_INET,SOCK_DGRAM,0);
        	string msg = "router: 1, pid: xxxx";
        	socklen_t len2 = sizeof(rou1Addr);
      		sendto(sockID2,sPid.data(),sPid.size()*sizeof(char),0,(struct sockaddr*)&rou1Addr,len2);
	//	sendto(sockID2,msg.data(),msg.size()*sizeof(char),0,(struct sockaddr*)&rou1Addr,len2);
        	getsockname(sockID2,(struct sockaddr*) &rou2Addr, &len);
        	unsigned int iRou2Port;
        	char pRou2Addr[16];
        	inet_ntop(AF_INET,&rou2Addr.sin_addr,pRou2Addr,sizeof(pRou2Addr));
        	iRou2Port = ntohs(rou2Addr.sin_port);
		Router.iPortNum = iRou2Port;
        	cout<<"router2: local address: "<<pRou2Addr<<endl;
        	cout<<"router2: local  port: "<<iRou2Port<<endl;
        	cout<<"router2: receiver  address:"<<pRou1Addr<<endl;
        	cout<<"router2: receiver port: "<<iRou1Port<<endl;
		cout<<endl;
		string temp2 = "router 1, pid: "+ sPid +", port: " + to_string(iRou2Port);
		vLog.push_back(temp2);
		string temp3 = "router 1 closed";
		vLog.push_back(temp3);

		Router.writeLogFile(vLog);

	}
	else
	{
		struct sockaddr_in rou2Addr;
		cout<<"child process pid: "<<fPid<<endl;
		vector<string> vLog;
		string temp = "primary port: "+to_string(Router.iPortNum);
		vLog.push_back(temp);
		char buf[1024], pRou2Addr[16];
                memset(buf,0,1024);
                len = sizeof(rou2Addr);
                int count = recvfrom(sockID, buf, 1024,0,(struct sockaddr*)&rou2Addr,&len);
        //      cout<<"udp receive"<<endl;
                if(count==-1)
                { cout<<"receive data fail!";}
                else
                {
                        printf("client %s \n", buf);
                }
		string sMsgRecv(buf);
                inet_ntop(AF_INET,&rou2Addr.sin_addr,pRou2Addr,sizeof(pRou2Addr));
                int iRou2Port = ntohs(rou2Addr.sin_port);
                cout<<"router1: sender address:"<<pRou2Addr<<endl;
                cout<<"router1: sender port: "<<iRou2Port<<endl;
		cout<<endl;
		string temp2 = "router 1, pid: "+ sMsgRecv +", port: " + to_string(iRou2Port);
		vLog.push_back(temp2);
		string temp3 = "router 0 closed";
		vLog.push_back(temp3);
		Router.writeLogFile(vLog);
	}	
}
