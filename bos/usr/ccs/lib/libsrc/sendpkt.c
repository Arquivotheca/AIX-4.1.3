static char sccsid[] = "@(#)17	1.3  src/bos/usr/ccs/lib/libsrc/sendpkt.c, libsrc, bos411, 9428A410j 6/16/90 02:35:30";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controler
 *
 * FUNCTIONS:
 *	srcsendpkt
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
**    Name:	srcsendpkt
**    Title:	Send Packet
** PURPOSE:
**	To send a packet to a specific address and do it until we are
**	not interupted.
** 
** SYNTAX:
**    srcsendpkt(fd,data,datasz,flags,hostaddr,hostaddrsz)
**    Parameters:
**	i int fd - socket to send packet on
**	i char *data - packet to send
**	i int datasz - size of the packet to send
**	i int flags - flags to sendto
**	i struct sockaddr_in *hostaddr - address to send packet to
**	i int hostaddrsz - size of address to send packet to
**
** INPUT/OUTPUT SECTION:
**	SOCKET: fd
**
** PROCESSING:
**	keep sending packet while we are interupted.
**
** PROGRAMS/FUNCTIONS CALLED:
**	sendto
**
** OTHER:
**
** RETURNS:
**	int error code or number of bytes sent
**
**/
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <srcerrno.h>
srcsendpkt(fd,data,datasz,flags,hostaddr,hostaddrsz)
int fd;
char *data;
int datasz;
int flags;
struct sockaddr_in *hostaddr;
int hostaddrsz;
{
	int rc;
	do {
		rc=sendto(fd,data,datasz,flags,hostaddr,hostaddrsz);
	} while(rc == -1 && errno==EINTR);

	if(rc == -1)
		return(SRC_SOCK);
	else
		return(rc);
}
