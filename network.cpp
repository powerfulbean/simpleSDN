#include "network.h"

int getUdpSocket()
{
        return socket(AF_INET,SOCK_DGRAM,0);
}

void setTempAddr(const char* pIp, struct sockaddr_in & locAddr)
{
        bzero(&locAddr,sizeof(locAddr));
        locAddr.sin_family = AF_INET;
        locAddr.sin_port = htons(SERV_PORT);
        inet_pton(AF_INET,pIp,&locAddr.sin_addr);
        return;
}

void getDynmcPortSrv(const struct sockaddr_in & locAddr,
                        struct sockaddr_in & outputAddr)
{
        int sockID = getUdpSocket();
        socklen_t len;
        connect(sockID,(struct sockaddr*) &locAddr,sizeof(locAddr));
        len = sizeof(outputAddr);
        getsockname(sockID,(struct sockaddr*) &outputAddr, &len);
        close(sockID);
}


