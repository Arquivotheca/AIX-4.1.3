static char sccsid[] = "@(#)16	1.3  src/bos/usr/ccs/lib/libsrc/recvpkt.c, libsrc, bos411, 9428A410j 10/18/90 15:08:52";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controler
 *
 * FUNCTIONS:
 *	srcrecvpkt,recv_timeout
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
**    Name:	srcrecvpkt
**    Title:	Receive Socket Packet
** PURPOSE:
**    To wait for a socket packet to arrive on a specific socket.
** 
** SYNTAX:
**    srcrecvpkt(fd,data,datasz,flags,hostaddr,hostaddrsz,time)
**    Parameters:
**       i int fd - socket to read packet from
**       o char *data - user buffer to store data
**       i int datasz - size of user buffer
**       i int flags - flags to socket
**       o struct sockaddr_in *hostaddr - buffer for address from which 
**		the packet was sent.
**       u int *hostaddrsz - size of the hostaddr buffer
**       i int time - time to wait for packet to arrive
**
** INPUT/OUTPUT SECTION:
**	SOCKET: fd
**
** PROCESSING:
**	When time is non-zero
**		save SIGALRM handler
**		save time to first alarm
**		replace SIGALARM handler
**
**	wait for packet
**
**	When time is non-zero
**		restore original SIGALARM handler
**		restore original time to alarm
**
** PROGRAMS/FUNCTIONS CALLED:
**	recvfrom
**	sigvec
**	recv_timeout
**
** OTHER:
**	NOTE: if 'time' is zero then srcrecvpkt will wait forever.
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
#include <signal.h>
#include <srcerrno.h>

static int timeout;

/* our alarm signal handler will only set a flag */
static void recv_timeout(int sig)
{
	timeout=TRUE;
}

int srcrecvpkt(fd,data,datasz,flags,hostaddr,hostaddrsz,time)
int fd;
char *data;
int datasz;
int flags;
struct sockaddr_in *hostaddr;
int *hostaddrsz;
int time;
{
	int rc;
	struct sigvec newvec, oldvec;
	unsigned int alarm();
	unsigned int otime;

	/* we have not timed out on our read yet */
	timeout=FALSE;

	/* was a time limit specified for the packet recive? */
	if(time!=0)
	{
		/* time limit was specified
		**    1. setup to catch alarm signal
		**    2. save old alarm signal info (time & function)
		**    3. set the alarm 
		**/
		bzero(&newvec,sizeof(newvec));
		newvec.sv_handler=recv_timeout;
		sigvec(SIGALRM,&newvec,&oldvec);
		otime=alarm((unsigned int)time);
	}
	
	do {
		rc=recvfrom(fd,data,datasz,flags,hostaddr,hostaddrsz);
	} while(rc == -1 && errno==EINTR && !timeout);

	/* restore old alarm handler and time */
	if(time!=0)
	{
		sigvec(SIGALRM,&oldvec,(struct sigvec *)0);
		alarm(otime);
	}

	/* error on receipt of packet? */
	if(rc < 0)
		/* did we time out waiting to receive a packet? */
		if(timeout)
			return(SRC_NORPLY);
		/* we had some socket error instead of receiving a packet */
		else
			return(SRC_SOCK);
	return(rc);
}
