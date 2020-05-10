/* $Id: socket.c 1.1 1995/01/01 07:11:14 cthuang Exp $
 *
 * This module has been modified by Radim Kolar for OS/2 emx
 */

/***********************************************************************
  module:       socket.c
  program:      popclient
  SCCS ID:      @(#)socket.c    1.5  4/1/94
  programmer:   Virginia Tech Computing Center
  compiler:     DEC RISC C compiler (Ultrix 4.1)
  environment:  DEC Ultrix 4.3 
  description:  UNIX sockets code.
 ***********************************************************************/
 
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>

int Socket(const char *host, int clientPort)
{
    int sock;
    unsigned long inaddr;
    struct sockaddr_in ad;
    struct hostent *hp;
    
    memset(&ad, 0, sizeof(ad));
    ad.sin_family = AF_INET;

    inaddr = inet_addr(host);
    if (inaddr != INADDR_NONE)
        memcpy(&ad.sin_addr, &inaddr, sizeof(inaddr));
    else
    {
        hp = gethostbyname(host);
        if (hp == NULL)
            return -1;
        memcpy(&ad.sin_addr, hp->h_addr, hp->h_length);
    }
    ad.sin_port = htons(clientPort);
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        return sock;
    if (Connect(sock, (struct sockaddr *)&ad, sizeof(ad)) < 0)
        return -1;
    return sock;
}

int Connect(int sockfd, const struct sockaddr * name, size_t len)
{
    fcntl(sockfd,F_SETFL,fcntl(sockfd,F_GETFL,0)|O_NONBLOCK);
    int connected = connect(sockfd, name, len);
    int ret = -1;
    if (connected != 0 )
    {
        if(errno != EINPROGRESS)
            printf("connect error :%s\n",strerror(errno));
        else
        {
            struct timeval tm = {0, 30000};
            fd_set wset,rset;
            FD_ZERO(&wset);
            FD_ZERO(&rset);
            FD_SET(sockfd,&wset);
            FD_SET(sockfd,&rset);
            int res = select(sockfd+1,&rset,&wset,NULL,&tm);
            if(res < 0)
            {
                if (errno != EINTR)
                    printf("connect error :%s\n",strerror(errno));
            }
            else if(res == 0)
            {
                printf("connect time out\n");
            }
            else if (1 == res)
            {
                if(FD_ISSET(sockfd,&wset))
                {
                    //printf("connect succeed.\n");
                    fcntl(sockfd,F_SETFL,fcntl(sockfd,F_GETFL,0) & ~O_NONBLOCK);
                    ret = 0;
                }
                else
                {
                    printf("other error when select:%s\n",strerror(errno));
                }
            }
        }
    }
    return ret;
}
