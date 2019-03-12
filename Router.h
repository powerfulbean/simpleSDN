#pragma once
#include "cRouter.h"
void primaryRouter(int sockID, cRouter & Router,
	sockaddr_in &rou2Addr);
void secondRouter(cRouter & Router, const struct sockaddr_in rou1Addr, struct sockaddr_in &rou2Addr);

int icmpForward_log(cRouter & Router, char * buffer, unsigned int iSize, int flag, int iPort);
void icmpReply_primRouter(int tun_fd, char* buffer, int nread);
void icmpReply_secondRouter(int iSockID, char* buffer, unsigned int iSize,const struct sockaddr_in rou1Addr);
void icmpForward_secondRouter(cRouter & Router, char* buffer, unsigned int iSize,const struct sockaddr_in rou1Addr,
	       	 const struct in_addr addrForReplace);

void primaryRouter_s2(cRouter & Router,struct sockaddr_in &rou2Addr);
void secondRouter_s2(cRouter & Router);
void primaryRouter_s4(cRouter & Router, struct sockaddr_in &rou2Addr);
void secondRouter_s4(cRouter & Router);
void primaryRouter_s5(cRouter & Router, struct sockaddr_in &rou2Addr);
void secondRouter_s5(cRouter & Router);
void primaryRouter_s6(cRouter & Router);
void secondRouter_s6(cRouter & Router);
void primaryRouter_reg(const int sockID, cRouter & Router);
void secondRouter_reqReg(cRouter & Router, const sockaddr_in rou1Addr,int iRouterID);


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

int packetDstCheck(struct in_addr &srcAddr, string targetDst, string  mask);

class cOctaneTimer : public TimerCallback
{
public:
	octane_control m_iOctaneMsg;
	int m_iSeq;
	int Expire();
	sockaddr_in m_rou2Addr;
	int m_iSockID;

	cOctaneTimer(int iSockID, sockaddr_in &rou2Addr, octane_control  msg, int iSeq) : m_iSockID(iSockID), m_iOctaneMsg(msg), m_iSeq(iSeq), m_rou2Addr(rou2Addr){};
};
