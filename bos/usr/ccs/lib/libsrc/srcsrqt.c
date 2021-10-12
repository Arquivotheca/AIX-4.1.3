static char sccsid[] = "@(#)28	1.11  src/bos/usr/ccs/lib/libsrc/srcsrqt.c, libsrc, bos411, 9428A410j 2/6/92 09:48:27";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controller
 *
 * FUNCTIONS:
 *	srcsrqt
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
**    Name:	srcsrqt
**    Title:	Send Request To Subystem
** PURPOSE:
**	To send a request packet to a subsystem and wait for a reply from
**	the subsystem.
** 
**	Note: if startitallso is turned on then SRC will attempt to start
**	the subsystem if the subsystem is not currently active.
**
** SYNTAX:
**    srcsrqt(host, name, subsyspid,reqlen, subreq, replen, replybuf, startitallso,cont)
**    Parameters:
**	i char *host - target host for subsystem request
**	i char *name - subsystem name
**	i int subsyspid - subsystem pid
**	i short reqlen - requst length
**	i char *subreq - request to be pass on to the subsystem
**	u short *replen - reply buffer length
**	u char *replybuf - reply buffer space
**	i int startitallso - start subsystem if subsystem not already
**		started indicator. will start subsystem if possible
**		when startitallso is non zero
**	u int *cont - continue - NEWREQUEST to send new REQUEST
**
** INPUT/OUTPUT SECTION:
**	UDP Socket
**
** PROCESSING:
**	On initial request forward request packet to SRC on the proper host.
**	While subsystem sends us a continuation message print them out.
**	On receipt of non-continuation message return to caller.
**
**	Note: this function will only return on the receipt of a
**	non-continuation message (or timeout waiting for a packet).
**
**	Note: the socket that this function opens remains so until
**	one of the following occurs
**		- An error packet is received 
**		- An end packet is received
**
** PROGRAMS/FUNCTIONS CALLED:
**
** OTHER:
**	Three responces to continuation can be returned by the subsystem
**		1. Continuation message packet, another packet will follow
**		2. Reply continuation packet, another packet will follow
**		3. End packet, no more packets follow.
**	These are the vaild responces from the subsystem
**		short error packet
**		svrreply error packet
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

#define pktlen sizeof(sndbuf)-sizeof(sndbuf.req)+sndbuf.reqlen

srcsrqt(host, name, subsyspid,reqlen, subreq, replen, replybuf, startitallso, cont)
char *host;
char *name;
int subsyspid;
short reqlen;
char *subreq ;
short *replen;
char *replybuf;
int startitallso;
int *cont;
{
	int rc;

	/* communication socket */
	static int fd;
	static struct src_socket src_socket={0,0};

	struct sockaddr_un src_sock_addr;
	struct sockaddr_un sockaddr;
	int sockaddrsz;

	struct sndreq sndbuf;

	struct srcrep *reqptr;              /* pointer to request       */

	/* first time trough we want to send a request to SRC */
	if (*cont == NEWREQUEST)
	{
		if(reqlen>REQSIZE)
			return(SRC_REQLEN2BIG);

		/* for those times when continue is ignored */
		if(src_socket.open)
			src_close_socket(&src_socket);

		/*-----------------------------------------------------------*/
		/*           Set up and send request to server               */
		/*-----------------------------------------------------------*/

		bzero(&sndbuf,sizeof(sndbuf));
		/* we can do two things when a subsystem does not exist
		**    1. return error to caller that subsystem is not alive
		**    2. start the subsystem and attempt to pass the
		**	 request on to the subsystem then.
		**/
		if(startitallso)
			/* we will try and start it if it aint alive */
			sndbuf.demnreq.action = REQUESTANDSTART;
		else
			/* we wont try and start it if it aint alive */
			sndbuf.demnreq.action = REQUEST;

		sndbuf.demnreq.dversion = SRCMSGBASE;
		sndbuf.demnreq.pid = subsyspid;      /* process id          */
		strcpy(((char *)sndbuf.demnreq.subsysname),name);
		sndbuf.reqlen = reqlen ;           /* input from call     */
		memcpy((void *)sndbuf.req,(void *)subreq,(size_t)reqlen);
		sndbuf.replen = *replen;           /* input from call     */

		/* setup socket to send request through and get address of SRC
		** if request stays on local host will check to
		**	make sure if local SRC is running
		**/
		fd=srcsockset(&src_socket,&src_sock_addr,host,MAXSOCKBUFSIZE,pktlen);
		if(fd<0)
			return(fd);

		rc=srcsendpkt(fd,&sndbuf,pktlen,0,&src_sock_addr,src_what_sockaddr_size(&src_sock_addr));
		if(rc < 0)
		{
			src_close_socket(&src_socket);
			return(SRC_SOCK);
		}
	}
	else if(!src_socket.open)
		return(SRC_NOCONTINUE);

	reqptr = (struct srcrep *) replybuf;

	/* wait and receive packets until 
	**	1. error packet is received.
	**	2. End packet it received.
	**	3. Reply Continuation packet is received
	**/
	do {
		sockaddrsz=sizeof(sockaddr);
		rc=srcrecvpkt(fd,replybuf,*replen,0,&sockaddr,&sockaddrsz,REPLYLIMIT);
		/* was a short error packet received */
		if (rc == sizeof(short)) {                           
			reqptr->srchdr.cont = END;      
			src_close_socket(&src_socket);
			if(name==0 || *name=='\0')
				sprintf(reqptr->svrreply.objname,"%d",subsyspid);
			else
				strcpy(reqptr->svrreply.objname,name);
			return((int)*(short *)replybuf);
		}

		/* grab continuation marker */
		*cont = reqptr->srchdr.cont;

		/* on receipt of a continuation message packet
		** print the message ie. some HAYES modem message or
		** other action informative message
		*/
		if ((rc >= 0) && (*cont == CONTINUED))
			printf("%s\n",reqptr->svrreply.rtnmsg);

	} while (rc >= 0 && *cont == CONTINUED);

	/* when we receive an END packet the socket we have allocated
	** is no longer needed
	**/
	if(*cont == END)
		src_close_socket(&src_socket);

	/* was there an error on receipt of the packet */
	if(rc < 0)
	{
		src_close_socket(&src_socket);
		return(rc);
	}

	/* did we receive subsystem error packet */
	if ((reqptr->svrreply.rtncode) < 0)
	{
		src_close_socket(&src_socket);
		return((int) reqptr->svrreply.rtncode);
	}

	/* we have a subsystem reply packet */
	*replen=rc;
	return(SRC_OK);
}
