#pragma once
#include "cRouter.h"
void primaryRouter(int sockID, cRouter & Router,
	sockaddr_in &rou2Addr);
void secondRouter(cRouter & Router, const struct sockaddr_in rou1Addr, struct sockaddr_in &rou2Addr);

int icmpForward_log(cRouter & Router, char * buffer, unsigned int iSize, int flag, int iPort);
void icmpReply_primRouter(int tun_fd, char* buffer, int nread);
void icmpReply_secondRouter(int iSockID, char* buffer, unsigned int iSize,const struct sockaddr_in rou1Addr);
void icmpForward_secondRouter(cRouter & Router, char* buffer, unsigned int iSize,
	       	 const struct in_addr addrForReplace);
void icmpForward_secondRouter(cRouter & Router, char* buffer, unsigned int iSize, const struct sockaddr_in rou1Addr,
	const struct in_addr addrForReplace);
void tcpForward_secondRouter(cRouter & Router, char* buffer, unsigned int iSize,
	const struct in_addr addrForReplace);
void primaryRouter_s2(cRouter & Router,struct sockaddr_in &rou2Addr);
void secondRouter_s2(cRouter & Router);
void primaryRouter_s4(cRouter & Router, struct sockaddr_in &rou2Addr);
void secondRouter_s4(cRouter & Router);
void primaryRouter_s5(cRouter & Router, struct sockaddr_in &rou2Addr);
void secondRouter_s5(cRouter & Router);
void primaryRouter_s6(cRouter & Router);
void secondRouter_s6(cRouter & Router);
void primaryRouter_s7(cRouter & Router);
void primaryRouter_reg(const int sockID, cRouter & Router);
void secondRouter_reqReg(cRouter & Router, const sockaddr_in rou1Addr,int iRouterID);
void primaryRouter_s9(cRouter & Router);
void primaryRouter_s10(cRouter & Router);

//write Router Port to /tmp/captive.conf
void primaryRouter_savePort(cRouter & Router);
void writeFile(string sFileName, const vector<string> & content);

int octaneRulesController(const flow_entry entry,cRouter Router, char* buffer, int iSize, struct sockaddr_in rou1Addr, struct in_addr rou2Sin_addr);

void stage1(cRouter &Router,
	struct sockaddr_in & rou1Addr,
	struct sockaddr_in & rou2Addr);

void stage2(cRouter &Router,
	struct sockaddr_in & rou1Addr,
	struct sockaddr_in & rou2Addr);

void stage4(cRouter &Router,
	struct sockaddr_in & rou1Addr,
	struct sockaddr_in & rou2Addr);

void stage5(cRouter &Router,
	struct sockaddr_in & rou1Addr,
	struct sockaddr_in & rou2Addr);

void stage6(cRouter &Router,
	struct sockaddr_in & rou1Addr,
	struct sockaddr_in & rou2Addr);

void stage7(cRouter &Router,
	struct sockaddr_in & rou1Addr,
	struct sockaddr_in & rou2Addr);

void stage9(cRouter &Router,
	struct sockaddr_in & rou1Addr,
	struct sockaddr_in & rou2Addr);

void stage10(cRouter &Router,
	struct sockaddr_in & rou1Addr,
	struct sockaddr_in & rou2Addr);

int packetDstCheck(struct in_addr &srcAddr, string targetDst, string  mask);

class cOctaneTimer : public TimerCallback
{
public:
	octane_control m_iOctaneMsg;
	int m_iSeq;
	int Expire();
	sockaddr_in m_rou2Addr;
	int m_iSockID = -1;

	cOctaneTimer(int iSockID, sockaddr_in & rou2Addr, octane_control  msg, int iSeq) : m_iOctaneMsg(msg), m_iSeq(iSeq), m_iSockID(iSockID)
	{
		m_rou2Addr.sin_family = AF_INET;
		m_rou2Addr.sin_addr.s_addr = rou2Addr.sin_addr.s_addr;
		m_rou2Addr.sin_port = rou2Addr.sin_port;
	}
};


