static char sccsid[] = "@(#)63	1.4  src/bos/usr/bin/src/cmds/srcdreq.c, cmdsrc, bos411, 9428A410j 6/15/90 23:35:53";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controller
 *
 * FUNCTIONS:
 *	srcdrqt
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


#include "src.h"
#include "src11.h"
#include "src10.h"
#include "srcmstr.h"


void srcdrqt ()
{
	extern int  dsvrsnd();
	extern struct activsvr *srcpassfind();   /* find subsystem */
	extern union ibuf ibuf;
	extern union obuf obuf;

	struct activsvr *ptr;     /* Ptr to an   actve svr tbl entry*/
	int rc;


	/* find non-inoperative copy of the subsystem */
	ptr=srcpassfind(ibuf.demnreq.subsysname,ibuf.demnreq.pid,frstaste);
	if(ptr != NULL)
	{
		/* are we to search for another instance? */
		if (ibuf.demnreq.subsysname[0] != 0)
		{
			/* find second instance of the subsystem to stop */
			if (srcpassfind(ibuf.demnreq.subsysname,ibuf.demnreq.pid, ptr->next) != NULL)
			{
				/* found several occurances of the subsystem
				** we can forward a packet to only one subsys
				**/
				shortrep(&retaddr,SRC_WICH);
				return;
			}
			/* no there is only one copy */
		}
		/* no there is only one copy */
	}
	else
	{
		/* no copies of the subsystem running */
		shortrep(&retaddr,SRC_NSVR);
		return;
	}

	/* there was just one occurance of the subystem (best case) */

	/* can't send a message to a signal oriented subsystem */
	if (ptr->subsys->contact == SRCSIGNAL)
	{
		shortrep(&retaddr,SRC_CONT);
		return;
	}

	/* can't forward message to stoping subsystem */
	if  (ptr->state == SRCWARN) {
		shortrep(&retaddr,SRC_STPG);
		return;
	}

	/* forward that packet to the subsystem */

	/* fill up the packet with it's little data */
	obuf.svrreq.srchdr.cont = END ;
	obuf.svrstp.srchdr.dversion = ibuf.demnreq.dversion;
	memcpy(&obuf.svrreq.srchdr.retaddr,&retaddr,sizeof(struct sockaddr_un));
	memcpy(obuf.svrreq.req,ibuf.sndreq.req,(int)ibuf.sndreq.reqlen);
	/* we can send both by ICPMSGQUEUES or SOCKETS to the subsystem */
	if (ptr->subsys->contact == SRCIPC)
	{
		/* message queue comunication */
		obuf.svrreq.mtype = (long)ptr->subsys->svrmtype;
		rc=dsvrsnd(ptr->subsys->svrkey, &obuf, (int)(ibuf.sndreq.reqlen + sizeof(struct srchdr)));
		if(rc<0)
			shortrep(&retaddr,rc);
	}
	else
	{
		/* socket comunication */
		rc=dsubsyspkt(&ptr->sun,&obuf.svrreq.srchdr,(int)(ibuf.sndreq.reqlen + sizeof(struct srchdr)));
		if(rc<0)
			shortrep(&retaddr,rc);
	}

	return;
}
