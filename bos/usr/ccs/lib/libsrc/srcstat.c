static char sccsid[] = "@(#)30	1.7  src/bos/usr/ccs/lib/libsrc/srcstat.c, libsrc, bos411, 9428A410j 2/26/91 14:54:54";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controller
 *
 * FUNCTIONS:
 *	srcstat
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregate modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1984,1989,1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*
** IDENTIFICATION:
**    Name:	srcstat
**    Title:	Send Status Request To SRC
** PURPOSE:
**	To send a status request packet to SRC and wait for the
**	receipt of a reply from SRC.
** 
** SYNTAX:
**    srcstat(host, name, svr_pid, replen,svvreply)
**    Parameters:
**	i char *host - target host for subsystem request
**	i char *name - subsystem name
**	i int svr_pid - subsystem pid
**	u short *replen - max reply buffer size
**	o char *svrreply - buffer for status reply from SRC.
**	u int *continued - continuation
**
** INPUT/OUTPUT SECTION:
**	UDP socket
**
** PROCESSING:
**	Send status request packet to SRC on the proper host.
**	Wait for a reply packet from SRC.
**
** PROGRAMS/FUNCTIONS CALLED:
**
** OTHER:
**	These are the vaild responces from the SRC
**		short error packet
**		statcode error packet
**		statcode responce packet
**	Note: there is no continuation support from this function.
**
**	Status that SRC will report:
**		active, inoperative, stoping, and warned to stop
**
** RETURNS:
**	error code or size of reply packet .
**
**/

#include "src.h"
#include <netinet/in.h>
#include "src10.h"
#include "srcsocket.h"

srcstat(host, name, svr_pid, replen, svrreply, continued)
char *host;
char *name;
int  svr_pid;
short *replen;
char *svrreply;
int *continued;
{
	static int fd; /* send&receive socket */
	int rc; 
	static struct src_socket src_socket={0,0};

	struct sockaddr_un src_sock_addr;
	struct sockaddr_un sockaddr;
	int sockaddrsz;

	if(*continued==NEWREQUEST)
	{
		struct stopstat statbuf;

		if(src_socket.open)
			src_close_socket(&src_socket);
	
		/* setup socket to send request through and get address of SRC
		** if request stays on local host will check to
		**	make sure if local SRC is running
		**/
		fd=srcsockset(&src_socket,&src_sock_addr,host,MAXSOCKBUFSIZE,sizeof(statbuf));
		if(fd<0)
			return(fd);
	
		/* build status request packet to send to SRC */
		bzero(&statbuf,sizeof(statbuf));
		statbuf.demnreq.action=STATUS;
		statbuf.demnreq.dversion=SRCMSGBASE;
		statbuf.parm1 = SHORTSTAT;
		statbuf.parm2 = *replen;
		statbuf.demnreq.pid = svr_pid;
		strcpy(statbuf.demnreq.subsysname,name);
	
		/* send status request to proper src daemon */
		rc=srcsendpkt(fd,&statbuf,sizeof(statbuf),0,&src_sock_addr,src_what_sockaddr_size(&src_sock_addr));
		if(rc <= 0)
		{
			src_close_socket(&src_socket);
			return(SRC_SOCK);
		}
	}
	else if(!src_socket.open)
		return(SRC_NOCONTINUE);

	/* receive status reply from src daemon */
	sockaddrsz=sizeof(sockaddr);
	rc=srcrecvpkt(fd,svrreply,(int)*replen,0,&sockaddr,&sockaddrsz,REPLYLIMIT);
	if (rc < 0)
	{
		src_close_socket(&src_socket);
		return(rc);
	}

	/* was a short error packet received? */
	if(rc == sizeof(short))
	{
		src_close_socket(&src_socket);
		return((int)*(short *)svrreply);
	}
	
	*continued=(int)((struct srchdr *)svrreply)->cont;
	if(*continued==END)
		src_close_socket(&src_socket);
		
	/* SRC reply packet was received */
	*replen=rc;
	return(SRC_OK);
}
