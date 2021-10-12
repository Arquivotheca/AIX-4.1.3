static char sccsid[] = "@(#)27	1.5  src/bos/usr/ccs/lib/libsrc/srcsrpy.c, libsrc, bos411, 9428A410j 6/16/90 02:37:14";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controller
 *
 * FUNCTIONS:
 *	srcsrpy
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregate modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1984,1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*
** IDENTIFICATION:
**    Name:	srcsrpy
**    Title:	Send Subsystem Reply
** PURPOSE:
**	To send a subsystem reply to a request forwarded by SRC to the
**	subsystem.
**
**	Note: srcsrpy creates it's own socket to send reply on.
** 
** SYNTAX:
**    srcsrpy (srchdr,p_pkt,p_pkt_sz,continued)
**    Parameters:
**      i struct srchdr *srchdr - pointer to srchdr that contains address 
**			for reply
**      u char   *p_pkt	- pointer to reply packet.
**      i int    p_pkt_sz - size of reply packet.
**      i ushort continued - continuation indicator if not set to END
**			there are more packets that follow.
**
** INPUT/OUTPUT SECTION:
**	UDP Socket
**
** PROCESSING:
**	create a new socket to send reply on.
**	place continued in the srchdr structure of the packet to be returned.
**	send reply on socket using return address stored in srchdr pointed
**	   to by the srchdr parameter.
**
** PROGRAMS/FUNCTIONS CALLED:
**	srcsendpkt
**	srcelog
**
** OTHER:
**	p_pkt points to the start of the return packet. Since we are using
**	sockets there is no mtype.
**
** RETURNS:
**	SRC_OK on success 
**	Error code on failure.
**
**/

#include "src.h"
#include "srcsocket.h"
#include <netinet/in.h>

int srcsrpy (srchdr,p_pkt,p_pkt_sz,continued)
struct srchdr *srchdr;
char   *p_pkt;
int    p_pkt_sz;
ushort continued;
{
	int rc;
	struct src_socket src_socket;
	int retaddrsz;

	/* The reply packet must be two bytes long or at least the
	 * size of a srchdr structure but not too long
	 */
	if((p_pkt_sz != sizeof(short) && p_pkt_sz < sizeof(struct srchdr)) || p_pkt_sz > SRCPKTMAX)
		return(SRC_REPLYSZ);

	/* setup a socket to send reply on */
	bzero(&src_socket,sizeof(src_socket));
	if(srchdr->retaddr.sun_family == AF_UNIX)
		srcafunixsockaddr(&src_socket.sun,1);
	else
		src_socket.sun.sun_family=AF_INET;

	src_socket.sock_id=src_setup_socket(&src_socket.sun,SOCK_DGRAM,0,p_pkt_sz);

	/* tell caller that we could not setup a socket for the return packet */
	if(src_socket.sock_id < 0)
		return(SRC_SOCK);

	src_socket.open=1;

	/* set or continuation marker
	**	don't have a continuation marker on a 'short' reply
	**/
	if(p_pkt_sz != sizeof(short))
		((struct srchdr *)p_pkt)->cont = continued;

	/* send response packet */
	rc=srcsendpkt(src_socket.sock_id,p_pkt,p_pkt_sz,0,&(srchdr->retaddr),src_what_sockaddr_size(&(srchdr->retaddr)));

	/* close that good old socket so it can be used again some day */
	src_close_socket(&src_socket);

	/* did we fail in sending our packet? */
	if(rc<0)
		return(SRC_SOCK);

	/* return our packet write status */
	return(SRC_OK);
}
