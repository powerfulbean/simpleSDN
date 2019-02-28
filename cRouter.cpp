#include "cRouter.h"

flow_entry::flow_entry(octane_control msg)
{
	m_dstIp = msg.octane_dest_ip;
	m_dstPort = msg.octane_dest_port;
	m_srcIp = msg.octane_source_ip;
	m_srcPort = msg.octane_source_port;
	m_protocol = msg.octane_protocol;
}

flow_entry flow_entry::reverse()
{
	flow_entry reverseEntry;
	reverseEntry.m_dstIp = m_srcIp;
	reverseEntry.m_dstPort = m_srcPort;
	reverseEntry.m_srcIp = m_dstIp;
	reverseEntry.m_srcPort = m_dstPort;
	reverseEntry.m_protocol = m_protocol;
	return reverseEntry;
}

bool flow_entry::operator< (const flow_entry key2) const
{
	if (m_srcIp != key2.m_srcIp)
	{
		return m_srcIp < key2.m_srcIp;
	}
	else 
	{
		if (m_srcPort != key2.m_srcPort)
		{
			return m_srcPort < key2.m_srcPort;
		}
		else
		{
			if (m_dstIp != key2.m_dstIp)
			{
				return m_dstIp < key2.m_dstIp;
			}
			else
			{
				return m_dstPort < key2.m_dstPort;
			}
		}
	}
}

flow_action::flow_action(octane_control msg)
{
	m_action = msg.octane_action;
	m_fwdPort = msg.octane_port;
}

string flow_table::insert(octane_control msg)
{
	flow_entry entry(msg);
	flow_action action(msg);
	if (contains(entry))
	{
		cout << "flow_table_insert warning: replace existed extry";
	}
	m_mTable[entry] = action;
	struct in_addr src1, dst1;
	src1.s_addr = entry.m_srcIp;
	dst1.s_addr = entry.m_dstIp;
	string output = ", rule installed (" + 
		string(inet_ntoa(src1)) + ", " + to_string(ntohs(entry.m_srcPort)) + ", " +
		string(inet_ntoa(dst1)) + ", " + to_string(ntohs(entry.m_dstPort))+ ", " + to_string(entry.m_protocol) +
		") action " + to_string(action.m_action);
	return output;
}

vector<string> flow_table::dbInsert(octane_control msg, uint16_t newFwdPort)
{
	flow_entry entry(msg);
	flow_action action(msg);
	flow_entry entryRev = entry.reverse();
	flow_action actionRev(msg);
	if (contains(entry))
	{
		cout << "flow_table_insert warning: replace existed extry";
	}
	m_mTable[entry] = action;
	if (newFwdPort != 65535)
	{
		actionRev.m_fwdPort = newFwdPort;
	}
	m_mTable[entryRev] = actionRev;
	vector<string> log;
	struct in_addr src1, src2, dst1, dst2;
	src1.s_addr = entry.m_srcIp;
	src2.s_addr = entryRev.m_srcIp;
	dst1.s_addr = entry.m_dstIp;
	dst2.s_addr = entryRev.m_dstIp;
	string output1 = ", rule installed (" +
		string(inet_ntoa(src1)) + ", " + to_string(ntohs(entry.m_srcPort)) + ", " +
		string(inet_ntoa(dst1)) + ", " + to_string(ntohs(entry.m_dstPort) ) + ", " + to_string(entry.m_protocol) +
		") action " + to_string(action.m_action) ;
	string output2 = ", rule installed (" +
		string(inet_ntoa(src2)) + ", " + to_string(ntohs(entryRev.m_srcPort)) + ", " +
		string(inet_ntoa(dst2)) + ", " + to_string(ntohs(entryRev.m_dstPort)) + ", " + to_string(entryRev.m_protocol) +
		") action " + to_string(actionRev.m_action);
	log.push_back(output1);
	log.push_back(output2);
	return log;
}

string flow_table::find(octane_control msg)
{
	string temp;
	return temp;
}

string flow_table::remove(octane_control msg)
{
	string temp;
	return temp;
}

bool flow_table::contains(octane_control msg)
{
	flow_entry entry(msg);
	map<flow_entry, flow_action>::iterator it;
	it = m_mTable.find(entry);
	if (it == m_mTable.end())
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool flow_table::contains(flow_entry entry)
{
	map<flow_entry, flow_action>::iterator it;
	it = m_mTable.find(entry);
	if (it == m_mTable.end())
	{
		return false;
	}
	else
	{
		return true;
	}
}

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
		cout << "Error: stage£º " << iStage << "need more legal line in the configuration file" << endl;
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

int cRouter::createOctaneMsg(octane_control &msg, const char *buffer, const unsigned int iSize, uint8_t octane_action, uint16_t sTargetPort , bool truelySend )
{
	uint32_t sSrc_addr;
	uint32_t sDst_addr;
	uint16_t sSrc_port;
	uint16_t sDst_port;
	u_int8_t ip_type;


	ipUnpack(buffer, sSrc_addr, sDst_addr, sSrc_port, sDst_port, ip_type);
	int iThisSeqno = m_iSeqnoCnt;
	msg.octane_action = octane_action ;
	msg.octane_flags = 0;
	if (truelySend == true)
	{
		msg.octane_seqno = m_iSeqnoCnt++;
	}
	else
	{
		msg.octane_seqno = 0;
	}
	msg.octane_source_ip = sSrc_addr;
	msg.octane_dest_ip = sDst_addr;
	msg.octane_source_port = sSrc_port;
	msg.octane_dest_port = sDst_port;
	msg.octane_protocol = ip_type;
	msg.octane_port = sTargetPort;

	return iThisSeqno;
}

int cRouter::createReverseOctaneMsg(octane_control &msg,const octane_control oriMsg, uint16_t sTargetPort , bool truelySend)
{
	uint32_t sSrc_addr;
	uint32_t sDst_addr;
	uint16_t sSrc_port;
	uint16_t sDst_port;
	u_int8_t ip_type;
	int iThisSeqno = m_iSeqnoCnt;
	msg.octane_action = oriMsg.octane_action;
	msg.octane_flags = 0;
	if (truelySend == true)
	{
		msg.octane_seqno = m_iSeqnoCnt++;
	}
	else
	{
		msg.octane_seqno = 0;
	}
	msg.octane_source_ip = oriMsg.octane_dest_ip; 
	msg.octane_dest_ip = oriMsg.octane_source_ip;
	msg.octane_source_port = oriMsg.octane_dest_port;
	msg.octane_dest_port = oriMsg.octane_source_port;
	msg.octane_protocol = oriMsg.octane_protocol;
	if (sTargetPort == -1)
	{
		msg.octane_port = oriMsg.octane_port;
	}
	else
	{
		msg.octane_port = sTargetPort;
	}
	return iThisSeqno;
}

void cRouter::printUnAckBuffer()
{
	//Output unAckBuffer
	cout << endl;
	for (auto i : m_unAckBuffer)
	{
		cout << "seqno: " << i.first << "is not acked" <<" current seqno is: "<<m_iSeqnoCnt<< endl;
	}
}