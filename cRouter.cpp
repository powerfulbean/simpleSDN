#include "cRouter.h"

int cRouter::stageEngine()
{
	//int iStageLen = 2;
	if(iConfigReg<=vConfig.size())
	{       
		iStage = stoi(vConfig[(nextConfig() -1)]);
		switch (stageToCase(iStage))
		{
		case 0:
			iRouteNum = stoi(vConfig[(nextConfig() - 1)]);
			break;
		case 1:
			iRouteNum = stoi(vConfig[(nextConfig() - 1)]);
			m_iDropAfter = stoi(vConfig[(nextConfig() - 1)]);
			break;
		default:
			break;
		}
		
	}
	else
	{
		return 0;
	}
	return iConfigReg;
}

int cRouter::nextConfig()
{
	if (iConfigReg > vConfig.size())
	{
		cout << "Error: stage�� " << iStage << "need more legal line in the configuration file" << endl;
		return 0;
	}
	int iCurrent = iConfigReg;
	iConfigReg++;
	return iCurrent;
}

int cRouter::stageToCase(int iStage)
{
	if (iStage <= 3)
	{
		return 0;
	}
	else if (iStage > 3 && iStage <= 4)
	{
		return 1;
	}
	return -1;
}
	
void cRouter::close()
{
	string temp3 = "router " + to_string(iRouterID) +" closed";
	vLog.push_back(temp3);
	writeLogFile();
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
				else if (output[i] == "drop_after")
				{
					i++;
					//iRouteNum = stoi(output[i]);
					vConfig.push_back(output[i]);
				}
			}
		}
	}
	
}

void cRouter::writeLogFile()
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
	outFile.close();
}


int cRouter::parser(const string &target, vector<string> &output)
{
	int iCount=0;
	bool wordHeadFoundFlag = false;
	//bool wordTailFoundFlag = false;
	int iWordHead=0;
	int iWordTail=0;
	bool bEndFlag = false;
	for(int i=0;i<target.size();i++)
	{
		if(target[i]==' ' || target[i]==9)
		{
			;
		}
		else if(target[i]== '#')
		{
			bEndFlag = true;
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
		if ( bEndFlag == true)
		{
			break;
		}
	}
	return iCount;
}

