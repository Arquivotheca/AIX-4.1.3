static char sccsid[] = "@(#)68	1.7  src/bos/usr/bin/src/cmds/srcdstp.c, cmdsrc, bos411, 9428A410j 2/26/91 14:51:10";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controller
 *
 * FUNCTIONS:
 *	srcdstp,dsvrsnd,dsubsyspkt
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



#include "src.h"                         /* IPC request/reply struct  */
#include <signal.h>                      /* signal defines                */
#include <sys/msg.h>
#include "netinet/in.h"
#include "src11.h"                       /* subsystem profile                */
#include "src10.h"                       /* IPC request/reply   structure */

#include "srcmstr.h"

#define  SRC_ONE    1

/*--------------------------------*/
/* EXTERNAL ROUTINES              */
/*--------------------------------*/
extern union ibuf ibuf;         /* ipc queue input buffer   */
extern union obuf obuf;         /* ipc queue output buffer  */
extern int  errno     ;         /* Extern for errno               */
extern int  dmsgsnd() ;         /* SRC's call to IPC msg send     */
extern int  dsvrsnd() ;         /* send an ipc msg to svr         */
extern void dmsgxrcv();         /* SRC's call to IPC msg receive  */
extern long time()    ;         /* UNIX time function             */
extern unsigned alarm();        /* UNIX set alarm function        */

/*--------------------------------*/
/* GLOBAL   VARIABLES for  SRC    */
/*--------------------------------*/
extern int alarmset;
extern long nextalarm;


int srcdstp(ptr)
/* pointer to active server table entry of the subsystem to stop */
struct activsvr *ptr;
{
	int rc;

	(void) sigblock(BLOCKMASK);

	/* we will not accept another stop request when
	** 	1. we are already (not cancel) stoping 
	**		and the new request is not a cancel
	**	2. we have already received a stop cancel
	*/
	if (((ptr->state == SRCSTPG) && (ibuf.stopstat.parm1 != CANCEL))
	    || (ptr->state == SRCWARN))
	{
		(void) sigsetmask(0);
		return(SRC_STPG);
	}
	(void) sigsetmask(0);


	/* Stop Cancel requested ? */
	if (ibuf.stopstat.parm1 == CANCEL)
	{

		(void) sigblock(BLOCKMASK);

		/* send a TERMINATE signal*/
		kill(ptr->svr_pid, (int)SIGTERM);

		/* set time warning was sent */
		ptr->warntime = time( (long *) 0);

		/* new state of subsystem */
		ptr->state = SRCWARN;

		(void) sigsetmask(0);

		/* what is our time to next alarm */
		if(alarmset == 0 || nextalarm == 0 || ptr->warntime + ptr->subsys->waittime < nextalarm)
		{
			nextalarm=ptr->warntime+ptr->subsys->waittime;
			if(ptr->subsys->waittime > 0)
				/* set warn time from object */
				alarm((unsigned int)ptr->subsys->waittime);
			else
				/* can't have a warntime of zero */
				alarm((unsigned int)1);
		}
		alarmset++;

		return(SRC_OK) ;
	}

	/* Forced or Normal subsystem stop */

	if (ptr->subsys->contact == SRCSIGNAL)
	{
		(void) sigblock(BLOCKMASK);
		/* what stop signal do we send forced or normal */
		if (ibuf.stopstat.parm1 == FORCED)
			/* FORCED */
			rc = kill(ptr->svr_pid,(int)ptr->subsys->sigforce);
		else
			/* NORMAL */
			rc = kill(ptr->svr_pid,(int)ptr->subsys->signorm);

		if (rc < 0)
		{
			switch (errno)
			{
			case ESRCH:               /* search failed for pid*/
				rc = SRC_SVND ;
				break;

			case EINVAL:              /* invalid signal       */
				if (ibuf.stopstat.parm1 == FORCED)
					rc = SRC_BADFSIG;
				else
					rc = SRC_BADNSIG;
				break;


			default:                  /* not superusr*/
				rc = SRC_NOTROOT;
				break;
			} 
		}
		else
			/* remember we are stoping */
			ptr->state = SRCSTPG;
		(void) sigsetmask(0);             /* release signals  */
		return(rc);
	}

	/* we will comunicate the stop request to the subystem using
	** sockets or message queues
	**/

	/* create our stop packet */
	(void) sigblock(BLOCKMASK);
	obuf.svrstp.subreq.object   = SUBSYSTEM ;
	obuf.svrstp.subreq.action   = STOP   ;
	obuf.svrstp.subreq.parm1    = ibuf.stopstat.parm1 ;
	obuf.svrstp.subreq.parm2    = ibuf.stopstat.parm2 ;
	strcpy(obuf.svrstp.subreq.objname,ptr->subsys->subsysname);
	/* place return address on packet being sent */
	memcpy(&obuf.svrstp.srchdr.retaddr,&retaddr,sizeof(struct sockaddr_un));
	obuf.svrstp.srchdr.dversion = ibuf.demnreq.dversion;
	obuf.svrstp.srchdr.cont = END;

	/* comunication by message queues */
	if (ptr->subsys->contact == SRCIPC)
	{
		/* set subsys mtype(key by which subsys will read) */
		obuf.svrstp.mtype = ptr->subsys->svrmtype;
		rc = dsvrsnd(ptr->subsys->svrkey,&obuf,(sizeof(struct srcreq)));
		if (rc == SRC_OK)
		{
			ptr->state = SRCSTPG; /* remember the stop request */
			rc = SRC_ONE;
		}
	}
	/* comunication by sockets */
	else if (ptr->subsys->contact == SRCSOCKET)
	{
		rc=dsubsyspkt(&ptr->sun,&obuf.svrstp.srchdr,sizeof(struct srcreq));
		if (rc > 0)
		{
			ptr->state = SRCSTPG; /* remember the stop request */
			rc = SRC_ONE;
		}
	}

	(void) sigsetmask(0);                      /* release signals   */

	return(rc);
}

/*----------------------------------------------------------------------*/
/* DSVRSND function                                                     */
/*----------------------------------------------------------------------*/
int dsvrsnd(svrkey, msg, size)
key_t  svrkey;
struct msgbuf *msg;
int    size;
{
	int svrqid;
	int rc;

	/* server not active */
	rc = SRC_SVND;

	/* get the subsystems message queue */
	do {
		svrqid = msgget(svrkey,MSG_R);
	} while(svrqid == -1 && errno == EINTR);

	/* log any error with msgget */
	logiferr(svrqid,0,0,SRC_MSGQ,errno);

	/* send the message if the subsystem's msg que is really there */
	if (svrqid != -1)
	{
		/* msg que is there */
		rc = dmsgsnd(svrqid, msg, size);
		/* server not active */
		if(rc < 0)
			rc = SRC_SVND;
	}

	return(rc);
}
/* send a socket packet to the subsystem */
int dsubsyspkt(sun,buf,bufsz)
char *sun;
char *buf;
int bufsz;
{
	return(dsrcsendpkt(sun,buf,bufsz));
}
