#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#define SERV_PORT 80

int getUdpSocket();
void setTempAddr(const char* pIp,struct sockaddr_in & locAddr);
void getDynmcPortSrv(const struct sockaddr_in & locAddr,
                        struct sockaddr_in & outputAddr);
void sendMsg(int sockID2, struct sockaddr_in rou1Addr,
		struct sockaddr_in & rou2Addr);

