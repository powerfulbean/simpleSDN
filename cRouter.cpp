#include "cRouter.h"

	
void cRouter::readConfigFile(char* filePath)
{
	ifstream infile(filePath);
	vector<string> vReadBuffer;
	string sTemp;
	while (infile.good()){
		getline(infile,sTemp);
		vReadBuffer.push_back(sTemp);
	}
	infile.close();
	for(int i=0;i<vReadBuffer.size();i++)
	{	
		bool flag =true;
		string sNum;
		vector<string> output;
		if(parser(vReadBuffer[i],output))
		{
			for(int i=0;i<output.size();i++)
			{
				//cout<<output[i]<<endl;
				if(output[i++]=="stage")
				{
					iStage = stoi(output[i]);
				}
				else if( output[i++] == "num_routers")
				{
					iRouteNum = stoi(output[i]);
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
		//	cout<<i<<iWordHead<<endl;	
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
        socklen_t len2 = sizeof(rou1Addr);
        sendto(sockID2,sPid.data(),sPid.size()*sizeof(char),0,(struct sockaddr*)&rou1Addr,len2);
       // sendto(sockID2,msg.data(),msg.size()*sizeof(char),0,(struct sockaddr*)&rou1Addr,len2);
        getsockname(sockID2,(struct sockaddr*) &rou2Addr, &len);
        char pRou1Addr[16];
        unsigned int iRou1Port;
        inet_ntop(AF_INET,&rou1Addr.sin_addr,pRou1Addr,sizeof(pRou1Addr));// translate the router 1 ip address to ascii
        iRou1Port = ntohs(rou1Addr.sin_port);//translate the router 1 port  to ascii
	unsigned int iRou2Port;
        char pRou2Addr[16];
        inet_ntop(AF_INET,&rou2Addr.sin_addr,pRou2Addr,sizeof(pRou2Addr));
        iRou2Port = ntohs(rou2Addr.sin_port);
        Router.iRouterID = 1;
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

void primaryRouter(const int sockID,cRouter & Router, const sockaddr_in rou1Addr)
{
       struct sockaddr_in rou2Addr;
       socklen_t len;
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
