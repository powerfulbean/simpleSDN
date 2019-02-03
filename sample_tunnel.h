/* sample.cpp */
/*
 * sample code to open tunnel interface and to read data from it.
 */

/*
* This code "USC CSci551 FA2012 Projects A and B" is
* Copyright (C) 2012 by Zi Hu.
* All rights reserved.
*
* This program is released ONLY for the purposes of Fall 2012 CSci551
* students who wish to use it as part of their project assignments.
* Use for another other purpose requires prior written approval by
* Zi Hu.
*
* Use in CSci551 is permitted only provided that ALL copyright notices
* are maintained and that this code is distinguished from new
* (student-added) code as much as possible.  We new services to be
* placed in separate (new) files as much as possible.  If you add
* significant code to existing files, identify your new code with
* comments.
*
* As per class assignments, use of any code OTHER than this provided
* code requires explicit approval, ahead of time, by the professor.
*
*/

#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/if_tun.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/ip.h>
/**************************************************************************
 * tun_alloc: allocates or reconnects to a tun/tap device. 
 * copy from from simpletun.c
 * refer to http://backreference.org/2010/03/26/tuntap-interface-tutorial/ for more info 
 **************************************************************************/

 #ifdef __cplusplus
extern "C" {
#endif

int tun_alloc(char *dev, int flags);

int tunnel_reader();


/**************************************************************************
 * cwrite: write routine that checks for errors and exits if an error is  *
 *         returned.                                                      *
 **************************************************************************/
int cwrite(int fd, char *buf, int n);

int set_tunnel_reader();


int read_tunnel(int tun_fd, char *buffer,int iBufSize);
#ifdef __cplusplus
};
#endif

