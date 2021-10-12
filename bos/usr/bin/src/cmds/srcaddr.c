static char sccsid[] = "@(#)53	1.3.1.1  src/bos/usr/bin/src/cmds/srcaddr.c, cmdsrc, bos411, 9428A410j 10/16/93 11:20:33";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controller
 *
 * FUNCTIONS:
 *	src_invalid_socket_address, src_local_only_operations
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregate modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   
#include <sys/types.h>
#include <sys/socket.h>
#include "srcsocket.h"
#include "src.h"
#include <netdb.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>

static short srcerrno;
#undef shortrep
#define shortrep(sun,errno) \
{ \
	srcerrno=errno; \
	dsrcsendpkt(sun,&srcerrno,sizeof(short)); \
}


static int src_local_only_operations(int operation)
{
	switch(operation)
	{
		case NEWHVN:
		case ADDSUBSYS:
		case DELSUBSYS:
		case ADDINETSOCKET:
		case DELINETSOCKET:
			return(TRUE);
		default:
			return(FALSE);
	}
}

int src_invalid_socket_address(struct sockaddr_un *sun, int operation)
{
	/* local machine or remote? */
	if(sun->sun_family==AF_UNIX)
	{
		struct stat statbuf;

		/* if the operation was a for a dead child it can only
		 * come for srcmstr itself
		 */
		if(operation==NEWHVN)
		{
			if(strcmp(sun->sun_path,SRC_MASTER_AF_UNIX)!=0)
			{
				shortrep(sun,SRC_NOT_SRC_SOCKADDR);
				return(TRUE);
			}
			else
				return(FALSE);
		}

		/* socket address must be one we know about */
		if(strncmp(sun->sun_path,SRC_BASE_DIR_AF_UNIX,strlen(SRC_BASE_DIR_AF_UNIX))!=0)
		{
			shortrep(sun,SRC_NOT_SRC_SOCKADDR);
			return(TRUE);
		}

		/* local requests where we expect the socket file to be
		 * gone by the time we check it.
		 */
		if(src_local_only_operations(operation))
			return(FALSE);

		/* stat the socket file and make sure that the owner
		 * or group is root or system if the socket file goes 
		 * away before we can stat it forget the operation.
		 */
		if(stat(sun->sun_path,&statbuf) == -1)
			return(TRUE);

		/* root or system must have made the request */
		if(statbuf.st_uid!=0 && statbuf.st_gid!=0)
		{
			shortrep(sun,SRC_INVALID_USER);
			return(TRUE);
		}

		/* the socket address must still be a socket when we
		 * check it out
		 */
		if(!S_ISSOCK(statbuf.st_mode))
			return(TRUE);

		/* ok! */
		return(FALSE);
	}
	else
	{
		struct hostent *hostent;
		char host[MAXDNAME];

		/* ignore those operations that can only be generated
		 * on the local node
		 */
		if(src_local_only_operations(operation))
			return(TRUE);

		/* must have come from a root user on the other machine */
		if(ntohs(((struct sockaddr_in *)sun)->sin_port) > 1023)
		{
			shortrep(sun,SRC_INVALID_USER_ROOT);
			return(TRUE);
		}
		
		/*  get the source host name for the operation */
		hostent=gethostbyaddr(&((struct sockaddr_in *)sun)->sin_addr,
			sizeof(struct in_addr),AF_INET);
		if(hostent == (struct hostent *)0)
		{
			shortrep(sun,SRC_INET_INVALID_HOST);
			return(TRUE);
		}

		/* check for /etc/hosts.equiv for the hostname */
		strcpy(host, hostent->h_name);
		if(ruserok(host,0,"root","root")!=0)
		{
			shortrep(sun,SRC_INET_AUTHORIZED_HOST);
			return(TRUE);
		}
			
		/* ok! */
		return(FALSE);
	}
}
