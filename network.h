#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h> 
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

#define OCTANE_PROTOCOL_NUM 253
#define SERV_PORT 80
#define FromTunnel 5511
#define FromUdp 5512
#define FromRawSock 5513

using namespace std;

struct octane_control {
	uint8_t octane_action;
	uint8_t octane_flags;
	uint16_t octane_seqno;
	uint32_t octane_source_ip;
	uint32_t octane_dest_ip;
	uint16_t octane_source_port; 
	uint16_t octane_dest_port; 
	uint16_t octane_protocol; 
	uint16_t octane_port;

	octane_control(uint8_t action, uint8_t flags, uint16_t seqno, uint32_t src_ip, uint32_t dst_ip,
		uint16_t src_port, uint16_t dst_port, uint16_t protocol, uint16_t octPort = 0) :
		octane_action(action), octane_flags(flags), octane_seqno(seqno), octane_source_ip(src_ip),
		octane_dest_ip(dst_ip), octane_source_port(src_port), octane_dest_port(dst_port),
		octane_protocol(protocol), octane_port(octPort) {};
	octane_control() {};
	bool operator< (const octane_control key2) const;
};


int getUdpSocket();
int getIcmpRawSocket();
int getRawSocket(int protocol);
void setTempAddr(const char* pIp,struct sockaddr_in & locAddr);
//void setAddr(const char* pIp, int iPort, struct sockaddr_in & locAddr);
void getDynmcPortSrv(const struct sockaddr_in & locAddr,
                        struct sockaddr_in & outputAddr);
int sendMsg(int sockID2,const char* buf, unsigned int iSize,
		const struct sockaddr_in rou1Addr);
int recvMsg(int sockID2, char *buf, unsigned int iSize,
struct sockaddr_in & rou2Addr);

void icmpReply_Edit(char* buffer);
void ipChangeProtocol(char* buffer,int iProtocol);
struct in_addr icmpReply_Edit(struct in_addr AddrForReplace, char* buffer, int iFlag);
int icmpUnpack(char* buffer, struct in_addr &srcAddr, struct in_addr &dstAddr, u_int8_t &icmp_type);
int icmpUnpack(char* buffer, struct icmphdr &icmphdr, struct in_addr &srcAddr, struct in_addr &dstAddr, u_int8_t &icmp_type);
int octaneUnpack(char* buffer, struct octane_control *pOctane);

// the output endian of ipUnpack is network endian
int ipUnpack(const char* buffer, uint32_t &sSrc_addr, uint32_t &sDst_addr, uint16_t &sSrc_port, uint16_t &sDst_port, u_int8_t &ip_type);
u_int8_t getIcmpType(char* buffer);
void octaneReply_Edit(char* buffer);

void buildIpPacket(char* buffer, unsigned int iBufferSize, int iProtocol, string sSrcAddr, string sDstAddr, char* payload, unsigned int iPayloadSize);