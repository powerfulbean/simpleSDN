#pragma once
#include "cRouter.h"
#include "sample_tunnel.h"
void primaryRouter(int sockID, cRouter & Router,
	sockaddr_in &rou2Addr);
void secondRouter(cRouter & Router, const sockaddr_in rou1Addr);

void icmpReply(int tun_fd, char* buffer, int nread);

void primaryRouter_s2(int sockID, cRouter & Router,
	sockaddr_in &rou2Addr);
void secondRouter_s2(cRouter & Router);

void stage1(cRouter &Router);
void stage2(cRouter &Router);
