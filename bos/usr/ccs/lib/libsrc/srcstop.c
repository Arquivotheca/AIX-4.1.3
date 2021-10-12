static char sccsid[] = "@(#)33	1.10  src/bos/usr/ccs/lib/libsrc/srcstop.c, libsrc, bos411, 9428A410j 2/2/94 12:38:16";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controller
 *
 * FUNCTIONS:
 *	srcstop
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregate modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1984,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*
** IDENTIFICATION:
**    Name:	srcstop
**    Title:	Send Stop Subsystem Request To SRC
** PURPOSE:
**	To send a stop subsystem request packet to SRC and waits for a 
**	reply from SRC, the subsystem, or both.
** 
** SYNTAX:
**    srcstop(host, name, svr_pid, stoptype, replen, svrreply, stopfrom)
**    Parameters:
**	i char *host - target host for subsystem request
**	i char *name - subsystem name
**	i int svr_pid - subsystem pid
**	i short stoptype - normal, forced, or cancel 
**	i int stopfrom - SSHELL or SDAEMON
**	o short *replen - reply buffer length
**	o char *svrreply - reply buffer space
**
** INPUT/OUTPUT SECTION:
**	UDP Socket
**
** PROCESSING:
**	Send stop request packet to SRC on the proper host.
**	Wait for SRC/subsystem stop reply packets.
**
** PROGRAMS/FUNCTIONS CALLED:
**
** OTHER:
**	These are the vaild responces from SRC/subsystem
**		short error packet
**		srvreply error packet
**		srvreply responce packet
**
** RETURNS:
**	SRC_OK on success
**	Error code on failure 
**
**/
#include "src.h"
#include <netinet/in.h>
#include "src10.h"
#include "srcsocket.h"

srcstop(host, name, svr_pid, stoptype, replen, svrreply, stopfrom)
char *host;
char *name;
int svr_pid;
short stoptype;
short *replen;
char *svrreply;
int   stopfrom;
{
	int     rc;
	int     count;
	int     msgs;
	struct svrreply *replyptr;

	/* comunication file descriptor (socket) */
	int fd;
	struct src_socket src_socket;

	struct sockaddr_un src_sock_addr;
	struct sockaddr_un sockaddr;
	int sockaddrsz;
	struct srcrep rtlreply;

	struct stopstat stopbuf;

	replyptr = (struct svrreply *)svrreply;

	/* no stop type specified is a normal stop */
	if (stoptype == 0)
		stoptype = NORMAL;

	/* must have a valid stop type */
	if (stoptype != NORMAL && stoptype != CANCEL && stoptype != FORCED)
		return(SRC_PARM);


	/* setup socket to send request through and get address of SRC
	** if request stays on local host will check to
	**	make sure if local SRC is running
	**/
	fd=srcsockset(&src_socket,&src_sock_addr,host,MAXSOCKBUFSIZE,sizeof(stopbuf));
	if(fd<0)
		return(fd);

	/* build the stop subsystem request to be send to SRC */
	bzero(&stopbuf,sizeof(stopbuf));
	stopbuf.demnreq.action = STOP;
	stopbuf.demnreq.dversion = SRCMSGBASE;
	stopbuf.parm1 = stoptype;
	stopbuf.demnreq.pid = svr_pid;
	strcpy(stopbuf.demnreq.subsysname, name);

	/* Forward stop request to proper SRC daemon */
	rc=srcsendpkt(fd,&stopbuf,sizeof(stopbuf),0,&src_sock_addr,src_what_sockaddr_size(&src_sock_addr));
	if(rc<0)
	{
		src_close_socket(&src_socket);
		return(SRC_SOCK);
	}

	/* get count of subsystems from SRC daemon */
	sockaddrsz=sizeof(sockaddr);
	rc=srcrecvpkt(fd,svrreply,(int)(*replen),0,&sockaddr,&sockaddrsz,REPLYLIMIT);
	if(rc<0)
	{
		src_close_socket(&src_socket);
		return(rc);
	}

	/* short error packet was returned as status */
	if (replyptr->rtncode < 0)
	{
		src_close_socket(&src_socket);
		return((int) replyptr->rtncode);
	}

	/* receive all the stop responces */
	for(count=(int)replyptr->rtncode,msgs=0; msgs < count; msgs++)
	{
		/* receive the stop status could be from whom?
		**	SRC daemon
		**	subsystem
		**/
		sockaddrsz=sizeof(sockaddr);
		rc=srcrecvpkt(fd,&rtlreply,sizeof(rtlreply),0,&sockaddr,&sockaddrsz,REPLYLIMIT);

		/* error on receipt of packet? */
		if(rc<0)
		{
			src_close_socket(&src_socket);
			return(rc);
		}

		/* was a short error packet received? */
		if(rc==sizeof(short))
			rc=(int)*(short *)&rtlreply;
		/* might there be an error pakcet returning? */
		else if (rtlreply.svrreply.rtncode < SRC_OK) 
			rc = rtlreply.svrreply.rtncode;
		else 
		/* the stop of the subsystem was ok */
			rc = SRC_STPOK;

		/* print the error msg if we are from SSHELL commands */
		if (stopfrom == SSHELL)
		{
			srcerr(SUBSYS_BASE,rc,SSHELL,rtlreply.svrreply.objname,0,0,rtlreply.svrreply.rtnmsg);
	        	rc = SRC_OK;
		}
	}
	src_close_socket(&src_socket);
	return(rc);
}                                               /* END srcstop          */
