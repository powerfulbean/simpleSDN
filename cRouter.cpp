#include "cRouter.h"

int cRouter::stageEngine()
{
	int iStageLen = 2;
	iConfigReg += 1;
	if(iConfigReg*iStageLen<=vConfig.size())
	{       
		iStage = stoi(vConfig[(iConfigReg-1)*2]);
		iRouteNum = stoi(vConfig[(iConfigReg-1)*2+1]);
}
	else
	{
		return 0;
	}
	return iConfigReg;
}
	
void cRouter::readConfigFile(char* filePath)
{
	ifstream infile(filePath);
	vector<string> vReadBuffer;// every "string" in vReadBuffer is one line of content  in the file
	string sTemp;
	while (infile.good())
	{
		getline(infile,sTemp);
		vReadBuffer.push_back(sTemp);
	}
	infile.close();
	for(int i=0;i<vReadBuffer.size();i++)
	{	
		bool flag =true;
		string sNum;
		vector<string> output;
		if(parser(vReadBuffer[i],output)) // every string in the output is a word in one string of ReadBuffer 
		{
			for(int i=0;i<output.size();i++)
			{
			//	vector<string> vRecord;
				if(output[i]=="stage")
				{
					i++;
					//iStage = stoi(output[i]);
					vConfig.push_back(output[i]);
				}
				else if( output[i] == "num_routers")
				{
					i++;
					//iRouteNum = stoi(output[i]);
					vConfig.push_back(output[i]);
				}
			}
		}
	}
	
}

void cRouter::writeLogFile(vector<string> vLog)
{
	string sFilePath;
	string sStageNum = to_string(iStage);
	string sRouterID = to_string(iRouterID);
	sFilePath = "stage"+sStageNum+"."+"r"+sRouterID+".out";
	ofstream outFile(sFilePath);
	if(outFile.is_open())
	{
		for(int i=0;i<vLog.size();i++)
		{
			outFile<<vLog[i]<<"\n";
		}
	}
	else
	{
		cout<<"writeLogFile error: cannot open the file!"<<endl;
	}
}


int cRouter::parser(const string &target, vector<string> &output)
{
	int iCount=0;
	bool wordHeadFoundFlag = false;
	//bool wordTailFoundFlag = false;
	int iWordHead=0;
	int iWordTail=0;
	for(int i=0;i<target.size();i++)
	{
		if(target[i]==' ' || target[i]==9)
		{
			;
		}
		else if(target[i]== '#')
		{
			break;
		}
		else if((target[i]>='A' && target[i]<='Z')||
			(target[i]>='a' && target[i]<='z')||
			(target[i]>='0' && target[i]<='9')||
		        (target[i]=='_'))
		{
			if(wordHeadFoundFlag == false)
			{
				iWordHead = i;
				iWordTail = i;
				wordHeadFoundFlag = true;		
			}
			else if(wordHeadFoundFlag == true)
			{
				iWordTail = i;
			}
		}

		if(wordHeadFoundFlag == true)
		{
			//cout<<i<<iWordHead<<endl;	
			if(i!=iWordTail || i==target.size()-1)
			{
				if(iWordHead == iWordTail)
				{
					string temp;
					temp.push_back(target[iWordHead]);
					output.push_back(temp);
				}
				else
				{	
					string subString(target.begin()+iWordHead
							,target.begin()+iWordTail+1);
					// must +1 to include the tail
					output.push_back(subString);
				}
				iCount++;
				wordHeadFoundFlag = false;
			}	
		}
	}
	return iCount;
}


void secondRouter(cRouter & Router, const sockaddr_in rou1Addr)
{
	vector<string> vLog;
        int sockID2 = getUdpSocket();
        struct sockaddr_in rou2Addr;
        socklen_t len = sizeof(rou2Addr);
        string sPid = to_string(getpid());
        string msg = "router: 1, pid: xxxx";
        sendMsg(sockID2,sPid.data(),sPid.size()*sizeof(char),rou1Addr);
       // sendto(sockID2,msg.data(),msg.size()*sizeof(char),0,(struct sockaddr*)&rou1Addr,len2);
        getsockname(sockID2,(struct sockaddr*) &rou2Addr, &len);
	unsigned int iRou2Port;
        char pRou2Addr[16];
        inet_ntop(AF_INET,&rou2Addr.sin_addr,pRou2Addr,sizeof(pRou2Addr));
        iRou2Port = ntohs(rou2Addr.sin_port);
        Router.iRouterID = 1;
        Router.iPortNum = iRou2Port;
        cout<<"router2: local address: "<<pRou2Addr<<endl;
        cout<<"router2: local  port: "<<iRou2Port<<endl;
        cout<<endl;
        string temp2 = "router 1, pid: "+ sPid +", port: " + to_string(iRou2Port);
        vLog.push_back(temp2);
        string temp3 = "router 1 closed";
        vLog.push_back(temp3);

        Router.writeLogFile(vLog);
	exit(0);
}

void primaryRouter(const int sockID,cRouter & Router, const sockaddr_in rou1Addr)
{
        vector<string> vLog;
        struct sockaddr_in rou2Addr;
        string temp = "primary port: "+to_string(Router.iPortNum);
        vLog.push_back(temp);
        char buf[1024], pRou2Addr[16];
        memset(buf,0,1024);
        recvMsg(sockID, buf, 1024,rou2Addr);
        string sMsgRecv(buf);
        inet_ntop(AF_INET,&rou2Addr.sin_addr,pRou2Addr,sizeof(pRou2Addr));
        int iRou2Port = ntohs(rou2Addr.sin_port);
        string temp2 = "router 1, pid: "+ sMsgRecv +", port: " + to_string(iRou2Port);
        vLog.push_back(temp2);
        string temp3 = "router 0 closed";
        vLog.push_back(temp3);
        Router.writeLogFile(vLog);		
}

void stage1(cRouter &Router)
{
        int sockID;
        socklen_t len;
        struct sockaddr_in rou1Addr,locAddr;
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
        cout<<"primary address:"<<pRou1Addr<<endl;
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
		Router.iFPID = fPid;
        }
        else
        {
                cout<<"child process pid: "<<fPid<<endl<<endl;
		Router.iFPID = fPid;
                primaryRouter(sockID,Router,rou1Addr);
		waitpid(fPid,NULL,0);
        }
}

void stage2(cRouter &Router)
{
	cout<<"stage 2:\n";
	cout<<"iStage: "<<Router.iStage<<endl;
	cout<<"iRouteNum: "<<Router.iRouteNum<<endl;
}
