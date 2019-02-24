//jindou
#pragma once
#include "network.h"
#include "sample_tunnel.h"
#include <map>
using namespace std;

struct flow_entry
{
	uint32_t m_srcIp;
	uint32_t m_dstIp;
	uint16_t m_srcPort;
	uint16_t m_dstPort;
	uint16_t m_protocol;

	flow_entry() {};
	flow_entry( uint32_t srcIp, uint16_t srcPort, uint32_t dstIp,
		 uint16_t dstPort, uint16_t protocol) :
		m_srcIp(srcIp), m_srcPort(srcPort), m_dstIp(dstIp), m_dstPort(dstPort),
		m_protocol(protocol) {};
	flow_entry(octane_control msg);
	flow_entry reverse();
	bool operator< (const flow_entry key2) const;
};

struct flow_action
{
	uint16_t m_fwdPort;
	uint8_t m_action;

	flow_action() {};
	flow_action(uint16_t fwdport, uint8_t action = 0) :
		m_fwdPort(fwdport), m_action(action) {};
	flow_action(octane_control msg);
};

class flow_table {
public:
	map<flow_entry, flow_action>  m_mTable;
	
	flow_table() {};
	string insert(octane_control msg);
	vector<string> dbInsert(octane_control msg, uint16_t newFwdPort);
	string find(octane_control msg);
	string remove(octane_control msg);
	bool contains(octane_control msg);
	bool contains(flow_entry entry);
};


//vector<string>:{<stage1>,<num_routers1>,<stage2>,<num_routers2>...<stage4><num_routers4><drop_number>}
class cRouter{
	public:
		int iStage;
		int iRouteNum;
		int iRouterID;
		int iPortNum;
		int iFPID;// store the return value of fork()
		vector<string> vConfig;
		vector<string> vLog;
		
		int iConfigReg;
		int iSockID;
		int iRawSockID;
		int m_iDropAfter;

		cRouter() { iStage = 0; iRouteNum = 0; iRouterID = 0; iConfigReg = 1; iSockID = -1; iRawSockID = -1; m_iDropAfter = 3; m_iSeqnoCnt = 1; }
		void readConfigFile(char* filePath);
		void writeLogFile();
		int parser(const string &temp, vector<string> &output);
		int syntax();
		int stageEngine();
		int stageToCase(int iStage);
		void close();
		int nextConfig();

		// octane part:
		uint16_t m_iSeqnoCnt;
		flow_table m_rouFlowTable;
		map<uint16_t, octane_control> unAckBuffer;
		int m_iOctSockID;

		int createOctaneMsg(octane_control &msg, const char *buffer, const unsigned int iSize, uint8_t octane_action, uint16_t sTargetPort);
		int cRouter::createReverseOctaneMsg(octane_control &msg, const octane_control oriMsg, uint16_t sTargetPort)
};


