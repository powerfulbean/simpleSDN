#pragma once
#include "cRouter.h"

void primaryRouter(int sockID, cRouter & Router,
	sockaddr_in &rou2Addr);
void secondRouter(cRouter & Router, const sockaddr_in rou1Addr);

void primaryRouter_s2(int sockID, cRouter & Router,
	sockaddr_in &rou2Addr);
void secondRouter_s2(cRouter & Router, const sockaddr_in rou1Addr);

void stage1(cRouter &Router);
void stage2(cRouter &Router);