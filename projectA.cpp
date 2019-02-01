#include "projectA.h"
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
