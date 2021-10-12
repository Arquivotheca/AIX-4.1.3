static char sccsid[] = "@(#)19	1.10  src/bos/usr/ccs/lib/libsrc/setupsocket.c, libsrc, bos411, 9428A410j 4/29/91 16:51:55";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controler
 *
 * FUNCTIONS:
 *	src_setup_socket
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


/*
** IDENTIFICATION:
**    Name:	src_setup_socket
**    Title:	Setup Socket
** PURPOSE:
**	To create a socket and assign it an address and port number.
** 
** SYNTAX:
**    src_setup_socket(sun,socktype,bufsize)
**    Parameters:
**	u struct sockaddr_un *sun - address of socket created and real address
**		returned.
**	i int socktype - SOCK_STREAM or SOCK_DGRAM.
**	i int bufsize - socket buffer size requested.
**
** INPUT/OUTPUT SECTION:
**
** PROCESSING:
**
** PROGRAMS/FUNCTIONS CALLED:
**	socket
**	bind
**	getsockname
**	setsockopt
**
** OTHER:
**	Note: if bufsize is zero socket will have the default buffer size
**	for it's send and receive buffers.
**
** RETURNS:
**	int socket or error code
**
**/
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <errno.h>
#include <srcerrno.h>
int src_setup_socket(sun,socktype,ibufsize,obufsize)
struct sockaddr_un *sun;
int socktype;
int ibufsize;
int obufsize;
{
	int sockid;
	int sunsz;

	/* create our socket */
	sockid=socket(sun->sun_family, socktype, 0);
	if(sockid < 0)
		if(errno == ESOCKTNOSUPPORT)
			return(SRC_NOINET);
		else
			return(SRC_SOCK);

	/* do we want to change the buffer size of the socket? */
	if(ibufsize > 0)
		setsockopt(sockid, SOL_SOCKET, SO_RCVBUF, (char *)&ibufsize, sizeof (ibufsize));
	if(obufsize > 0)
		setsockopt(sockid, SOL_SOCKET, SO_SNDBUF, (char *)&obufsize, sizeof (obufsize));

	/* give our socket a home address */
	sunsz=src_what_sockaddr_size(sun);

	if(sun->sun_family == AF_INET && ((struct sockaddr_in *)sun)->sin_port==0)
	{
		if(bindresvport(sockid,(struct sockaddr_in *)0) < 0)
			if(errno==EACCES)
				return(SRC_INVALID_USER_ROOT);
			else
				return(SRC_NO_RESV_PORT);
	}
	else if(bind(sockid, sun, sunsz) < 0)
	{
		close(sockid);
		if(errno==EACCES)
			return(SRC_INVALID_USER);
		/* address already in use */
		return(SRC_SOCK);
	}

	/* get the whole name of the socket */
	if(sun->sun_family==AF_INET && getsockname(sockid, sun, &sunsz) < 0)
	{
		/* could not get it's name */
		close(sockid);
		return(SRC_SOCK);
	}

	return(sockid);
}
