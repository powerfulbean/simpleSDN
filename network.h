#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <wait.h>

#include "icmp_checksum.h"


#define SERV_PORT 80
#define FromTunnel 5511
#define FromUdp 5512

using namespace std;

int getUdpSocket();
void setTempAddr(const char* pIp,struct sockaddr_in & locAddr);
void getDynmcPortSrv(const struct sockaddr_in & locAddr,
                        struct sockaddr_in & outputAddr);
void sendMsg(int sockID2,const char* buf, unsigned int iSize,
		const struct sockaddr_in rou1Addr);
void recvMsg(int sockID2, char *buf, unsigned int iSize,
struct sockaddr_in & rou2Addr);

void icmpReply_Edit(char* buffer);
int icmpUnpack(char* buffer, struct in_addr &srcAddr, struct in_addr &dstAddr, u_int8_t &icmp_type);
