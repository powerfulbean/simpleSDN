#pragma once
#include "cRouter.h"
#include "sample_tunnel.h"
void primaryRouter(int sockID, cRouter & Router,
	sockaddr_in &rou2Addr);
void secondRouter(cRouter & Router, const struct sockaddr_in rou1Addr, struct sockaddr_in &rou2Addr);

void icmpReply_primRouter(int tun_fd, char* buffer, int nread);
void icmpReply_secondRouter(int iSockID, char* buffer, size_t iSize,const struct sockaddr_in rou1Addr);

void primaryRouter_s2(cRouter & Router,struct sockaddr_in &rou2Addr);
void secondRouter_s2(cRouter & Router);

void stage1(cRouter &Router,
	struct sockaddr_in & rou1Addr,
	struct sockaddr_in & rou2Addr);

void stage2(cRouter &Router,
	struct sockaddr_in & rou1Addr,
	struct sockaddr_in & rou2Addr);
