static char sccsid[] = "@(#)93	1.7  src/bos/usr/ccs/lib/libsrc/tellsrc.c, libsrc, bos411, 9428A410j 5/24/91 15:55:43";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controler
 *
 * FUNCTIONS:
 *	tellsrc
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
**    Name:	tellsrc
**    Title:	Tell SRC daemon something has happened
** PURPOSE:
**	To send a message to the SRC daemon informing it that some action has
**	take place or the SRC daemon must do some action.
** 
** SYNTAX:
**    tellsrc(action,subsysname)
**
**    Parameters:
**	i int action - action code to srcmstr
**      i char *subsysname - subsystem name action applies to
**
** INPUT/OUTPUT SECTION:
**	UDP Socket
**
** PROCESSING:
**	Build packet and send it to the localsrc
**
** PROGRAMS/FUNCTIONS CALLED:
**
** OTHER:
**
** RETURNS:
**	Number of bytes sent to SRC on success
**	Error code on failure
**/
#include "src.h"
#include <netinet/in.h>
#include "src10.h"
#include "srcsocket.h"

int tellsrc(action,subsysname)
int action;
char *subsysname;
{

	struct demnreq demnreq;
	int rc;
	int fd;
	struct sockaddr_un src_sock_addr;
	struct src_socket src_socket;

	/* forget it if src is not active right now */
	if(!is_active_srcmstr())
		return(SRC_DMNA);

	/* setup our socket to send to SRC */
	fd=srcsockset(&src_socket,&src_sock_addr,"",0,0);
	if(fd<0)
		return(fd);

	/* place data in demnreq struct to send */
	bzero(&demnreq,sizeof(demnreq));
	demnreq.action=(short)action;
	demnreq.dversion=(short)SRCMSGBASE;
	strcpy(demnreq.subsysname,subsysname);

	/* send our little candy gram along to SRC */
	rc=srcsendpkt(fd,&demnreq,sizeof(struct demnreq),0,&src_sock_addr,sizeof(struct sockaddr_un));

	/* error writing to the socket? */
	logiferr(rc,0,0,SRC_SOCK,errno);

	src_close_socket(&src_socket);

	return(rc);
}
