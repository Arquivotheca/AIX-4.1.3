static char sccsid[] = "@(#)26	1.7  src/bos/usr/ccs/lib/libsrc/srcsockset.c, libsrc, bos411, 9428A410j 6/16/90 02:37:07";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controler
 *
 * FUNCTIONS:
 *	srcsockset
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregate modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*
** IDENTIFICATION:
**    Name:	srcsockset
**    Title:	Setup Socket To Send to SRC
** PURPOSE:
**	Create a socket and return address of SRC's socket.
** 
** SYNTAX:
**    srcsockset(src_sock_addr,host)
**    Parameters:
**	o struct sockaddr_un *src_sock_addr;
**	i char *host;
**	i int ibufsize;
**	i int obufsize;
**
** INPUT/OUTPUT SECTION:
**
** PROCESSING:
**	Get address of SRC's Socket.
**	If address is local
**		Check that SRC is already running if not return error
**	Create socket to send message on.
**
** PROGRAMS/FUNCTIONS CALLED:
**	srcsockaddr
**	gethostname
**	src_setup_socket
**
** OTHER:
**
** RETURNS:
**	int socket id or error code
**
**/
#include "src.h"
#include "srcsocket.h"
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>
int srcsockset(src_socket,src_sock_addr,host,ibufsize,obufsize)
struct src_socket *src_socket;
struct sockaddr_un *src_sock_addr;
char *host;
int ibufsize;
int obufsize;
{
	int fd;
	struct sockaddr_in sin;

	/* was a host entered? */
	if(*host != '\0')
	{
		char hostname[HOSTSIZE];
		int rc;

		/* does the host and port exist? */
		rc=srcsockaddr(src_sock_addr,host);
		if (rc < 0)
			return(rc);

		gethostname(hostname,HOSTSIZE);
		srcsockaddr(&sin,hostname);
	}

	/* host specified check to see if it is the local host */
	if(*host == '\0' || memcmp((void *)&sin,(void *)src_sock_addr,(size_t)sizeof(sin))==0)
	{
		/* local host must have srcmstr already active */
		if(!active_srcmstr())
			return(SRC_DMNA);

		/* set up the local af_unix socket */
		srcafunixsockaddr(&src_socket->sun,1);
		srcafunixsockaddr(src_sock_addr,0);
		fd=src_setup_socket(&src_socket->sun,SOCK_DGRAM,ibufsize,obufsize);
	}
	else
	{
		/* set up the local internet socket */
		bzero(&sin,sizeof(sin));
		sin.sin_family=AF_INET;
		fd=src_setup_socket(&sin,SOCK_DGRAM,ibufsize,obufsize);
		memcpy((void *)&src_socket->sun,(void *)&sin,(size_t)sizeof(sin));
	}

	/* failed to setup socket */
	if (fd < 0)
		return(fd);

	src_socket->sock_id=fd;
	src_socket->open=1;

	return(fd);
}
