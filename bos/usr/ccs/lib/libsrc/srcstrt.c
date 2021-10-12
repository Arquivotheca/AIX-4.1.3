static char sccsid[] = "@(#)34	1.8  src/bos/usr/ccs/lib/libsrc/srcstrt.c, libsrc, bos411, 9428A410j 10/24/91 10:12:50";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controller
 *
 * FUNCTIONS:
 *	srcstrt
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
**    Name:	srcstrt
**    Title:	Send Start Subsystem Request To SRC
** PURPOSE:
**	To send a Start subsystem request packet to SRC and waits for 
**	reply from SRC.
** 
** SYNTAX:
**    srcstrt(host, name, env, parms, restrt, strtfrom)
**    Parameters:
**	i char *host - target host for subsystem request
**	i char *name - subsystem name
**	i char *envs - environment to be setup for a subsystem
**	i char *parms - command line arguments to be passed to the
**		subsystem when it is started.
**	i unsigned int restrt - respawn indicator, allow respawn on abnormal
**		subsystem termination.
**	i int strtfrom - SHELL or SDEAMON
**
** INPUT/OUTPUT SECTION:
**	UDP Socket
**
** PROCESSING:
**	Send start request packet to SRC on the proper host.
**	Wait for SRC start reply packets.
**
** PROGRAMS/FUNCTIONS CALLED:
**
** OTHER:
**	These are the vaild responces from SRC
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

#define PKTSIZE	\
sizeof(startbuf)-sizeof(startbuf.parm)+startbuf.envlen+startbuf.parmlen

srcstrt(host, name, env, parms, restrt, strtfrom)
char *host;
char *name;
char *env;
char *parms;
unsigned int restrt;
int strtfrom;
{
	int  rc;
	int  fd;
	struct src_socket src_socket;

	struct strtreply strtcode;
	struct sockaddr_un src_sock_addr;
	struct sockaddr_un sockaddr;
	int socksz;

	int count;

	struct start  startbuf;              /* start server parameters  */

        short daemnrep;

	/* check the length of the parms and environment passed in */
	{
		int parmlen;
		int envlen;
		parmlen=strlen(parms);
		envlen=strlen(env);
		if (parmlen + envlen >= PARMSIZE+ENVSIZE)
			if(parmlen >= PARMSIZE)
				return(SRC_ARG2BIG);
			else
				return(SRC_ENV2BIG);
	}


	/* setup socket to send request through and get address of SRC
	** if request stays on local host will check to
	**	make sure if local SRC is running
	**/
	fd=srcsockset(&src_socket,&src_sock_addr,host,MAXSOCKBUFSIZE,sizeof(startbuf));
	if(fd < 0)
		return(fd);


	/*------------------------------------------------------------------*/
	/*  Set up the request and send it to the srcmstr daemon.           */
	/*------------------------------------------------------------------*/

	bzero(&startbuf,sizeof(startbuf));
	startbuf.demnreq.action = START;             /* start svr action*/
	startbuf.demnreq.dversion = SRCMSGBASE;
	startbuf.rstrt = restrt;
	strcpy(startbuf.demnreq.subsysname, name);

	/* pass env&parms string lengths */
	startbuf.envlen = (short) strlen(env);
	startbuf.parmlen= (short) strlen(parms);

	/* pass env&parms strings with the start request */
	strcpy(startbuf.parm,parms);
	strcpy((char *)(startbuf.parm+startbuf.parmlen),env);


	/* send start request to the proper SRC daemon */
	rc=srcsendpkt(fd,&startbuf,PKTSIZE,0,&src_sock_addr,src_what_sockaddr_size(&src_sock_addr));
	/* error with sending the request? */
	if(rc<0)
	{
		src_close_socket(&src_socket);
		return(SRC_SOCK);
	}

	/* get the daemon reply start count */
	socksz=sizeof(sockaddr);
	rc=srcrecvpkt(fd,&daemnrep,sizeof(daemnrep),0,&sockaddr,&socksz,REPLYLIMIT);

	/* was there an error receiving the packet? */
	if (rc < 0)
	{
		src_close_socket(&src_socket);
		return(rc);
	}
	/* was there a short error pakcet returned to us */
	else if(daemnrep <= 0)
	{
		src_close_socket(&src_socket);
		return(daemnrep);
	}

	/* collect the start msgs */
	count = (int)daemnrep;
	while (count > 0)
	{
		/* recieve the first start msg */
		socksz=sizeof(sockaddr);
		rc=srcrecvpkt(fd,&strtcode,sizeof(strtcode),0,&sockaddr,&socksz,REPLYLIMIT);
		/* ignore errors on the receipt of a packet */
		if (rc > 0 && strtfrom == SSHELL)
		{
			/* positive pid is really a pid and not an error code */
			if(strtcode.pid > 0)
			{
				char pid[10];
				sprintf(pid,"%d",strtcode.pid);
				srcerr(SRC_BASE, SRC_STRTOK, SSHELL, strtcode.subsysname,pid,0,0);
			}
			else
				srcerr(SUBSYS_BASE, strtcode.pid,SSHELL,strtcode.subsysname, 0, 0, 0);
		}
		count--;
	}
	src_close_socket(&src_socket);

	if(strtfrom == SSHELL)
		return(SRC_OK);
	else
		return(strtcode.pid);
}
