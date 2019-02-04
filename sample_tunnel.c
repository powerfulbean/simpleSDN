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


#include "sample_tunnel.h"

/**************************************************************************
 * tun_alloc: allocates or reconnects to a tun/tap device. 
 * copy from from simpletun.c
 * refer to http://backreference.org/2010/03/26/tuntap-interface-tutorial/ for more info 
 **************************************************************************/

int tun_alloc(char *dev, int flags) 
{
    struct ifreq ifr;
    int fd, err;
    char *clonedev = (char*)"/dev/net/tun";

    if( (fd = open(clonedev , O_RDWR)) < 0 ) 
    {
	perror("Opening /dev/net/tun");
	return fd;
    }

    memset(&ifr, 0, sizeof(ifr));

    ifr.ifr_flags = flags;

    if (*dev) 
    {
	strncpy(ifr.ifr_name, dev, IFNAMSIZ);
    }

    if( (err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0 ) 
    {
	perror("ioctl(TUNSETIFF)");
	close(fd);
	return err;
    }

    strcpy(dev, ifr.ifr_name);
    return fd;
}

// the function set_tunnel_reader() is built by Jin Dou based on tunnel_reader provided by the author of this file
int set_tunnel_reader()
{
    char tun_name[IFNAMSIZ];

    /* Connect to the tunnel interface (make sure you create the tunnel interface first) */
    strcpy(tun_name, "tun1");
    int tun_fd = tun_alloc(tun_name, IFF_TUN | IFF_NO_PI);

    if(tun_fd < 0)
    {
        perror("Open tunnel interface");
        exit(1);
    }
   return tun_fd;
}

// the function read_tunnel is built by Jin Dou based on tunnel_reader provided by the author of this file
int read_tunnel(int tun_fd, char *buffer,size_t iBufSize)
{
	int nread = read(tun_fd,buffer,iBufSize);
	//printf("buffer size %d\n",iBufSize);
        if(nread < 0)
        {
            perror("Reading from tunnel interface");
            close(tun_fd);
        }
	return nread;
}


int tunnel_reader()
{
    char tun_name[IFNAMSIZ];
    char buffer[2048];

    /* Connect to the tunnel interface (make sure you create the tunnel interface first) */
    strcpy(tun_name, "tun1");
    int tun_fd = tun_alloc(tun_name, IFF_TUN | IFF_NO_PI); 

    if(tun_fd < 0)
    {
	perror("Open tunnel interface");
	exit(1);
    }


    /*
     * This loop reads packets from the tunnle interface.
     *
     * You will need to REWRITE this loop into a select loop,
     * so that it can talk both to the tun interface,
     * AND to the router.
     *
     * You will also probably want to do other setup in the
     * main() routine.
     */
    while(1) 
    {
	/* Now read data coming from the tunnel */
        int nread = read(tun_fd,buffer,sizeof(buffer));
	printf("buffer size %d\n",sizeof(buffer));
	if(nread < 0) 
	{
	    perror("Reading from tunnel interface");
	    close(tun_fd);
	    exit(1);
	}
	else
	{
	    printf("Read a packet from tunnel, packet length:%d\n", nread);
	    /* Do whatever with the data, function to manipulate the data here */
		
	    /*
	     * For project A, you will need to add code to forward received packet 
	     * to router via UDP socket.
	     * And when you get icmp echo reply packet from router, you need to write
	     * it back to the tunnel interface
	     */
	    
	}
    }
}

/**************************************************************************
 * cwrite: write routine that checks for errors and exits if an error is  *
 *         returned.													  *
 * this function is provided by											  *	
 * https://backreference.org/2010/03/26/tuntap-interface-tutorial/        *
 * (C) 2010 Davide Brini.                                                 *
 *                                                                        *
 * DISCLAIMER AND WARNING: this is all work in progress. The code is      *
 * ugly, the algorithms are naive, error checking and input validation    *
 * are very basic, and of course there can be bugs. If that's not enough, *
 * the program has not been thoroughly tested, so it might even fail at   *
 * the few simple things it should be supposed to do right.               *
 * Needless to say, I take no responsibility whatsoever for what the      *
 * program might do. The program has been written mostly for learning     *
 * purposes, and can be used in the hope that is useful, but everything   *
 * is to be taken "as is" and without any kind of warranty, implicit or   *
 * explicit. See the file LICENSE for further details.                    *
 *************************************************************************/
int cwrite(int fd, char *buf, int n){
  
  int nwrite;

  if((nwrite=write(fd, buf, n)) < 0){
    perror("Writing data");
    exit(1);
  }
  return nwrite;
}


// int main(int argc, char** argv)
// {
	// /*
	 // * For a real proja, you will want to do some setup here.
	 // */
	// tunnel_reader();
// }
//

