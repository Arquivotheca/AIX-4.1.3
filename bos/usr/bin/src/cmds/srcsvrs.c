static char sccsid[] = "@(#)79	1.7  src/bos/usr/bin/src/cmds/srcsvrs.c, cmdsrc, bos411, 9428A410j 2/26/91 14:51:20";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controller
 *
 * FUNCTIONS:
 *	srcsvrs
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


#include "src.h"
#include <signal.h>
#include "src11.h"                       /* subsystem profile                */
#include "src10.h"                       /* RTL request/reply   structure */
#include "srcmstr.h"

#define STATLEN(numstatcd) \
(sizeof(struct srchdr)+(sizeof(struct statcode)*numstatcd))

#define MAXSTATCD(buffsize) \
((buffsize-sizeof(struct srchdr))/sizeof(struct statcode))

#define SNDSTAT \
{ \
dsrcsendpkt(&retaddr,optr,STATLEN(curstatcd)); \
statptr=(struct statcode *)(optr + sizeof(struct srchdr)); \
curstatcd=0; \
} \

void srcsvrs ()
{
	/*------------------------------*/
	/* EXTERNAL VARIABLES/FUNCTIONS */
	/*------------------------------*/
	extern union ibuf ibuf;         /* ipc queue input buffer   */
	extern union obuf obuf;         /* ipc queue output buffer  */
	extern char *malloc();

	/*------------------------------*/
	/* LOCAL VARIABLES              */
	/*------------------------------*/

	unsigned statlen          ;     /* length of src subsystem profile */
	int    rc                 ;     /* Return code variable         */
	struct activsvr *ptr      ;     /* Ptr to an actve svr tbl entry*/
	int counter               ;     /* all subsystems character        */
	int    found              ;     /* found boolean                */
	struct statcode *statptr  ;     /* ptr to each status entry     */
	char *optr;     /* ptr to output status buffer  */
	static char hdr1[256];
	static char hdr2[256];
	static int gothdr=0;
	int maxstatcd;
	int curstatcd;

	struct validsubsys *vptr;

	(void) sigblock(BLOCKMASK);                /* block signals    */

	/* Get status of all subsystems or allsubsystems in a group */

	/* get size of active svr table */

	/* status by pid? */
	if(ibuf.demnreq.pid == 0)
	{
		/* status by all,group,or subsystem */
		for (vptr = frstsubsys, counter = 0;  vptr != (struct validsubsys *) 0; vptr = vptr->nsubsys)
		{
			/* all subystems */
			if(isall()
			    /* group of subsystems */
			    || isgroup(vptr->subsys.grpname)
			    /* single subsystem */
			    || issubsys(vptr->subsys.subsysname,0))
			{
				if(vptr->forked >= 1)
					counter=counter+vptr->forked;
				else if(vptr->subsys.display || issubsys(vptr->subsys.subsysname,0))
					counter++;
			}
		}
	}
	else
	{
		/* find by pid */
		for(ptr = frstaste; ptr != NULL && ptr->svr_pid != ibuf.demnreq.pid; ptr = ptr->next) ;

		if (ptr != NULL)
			counter = 1;
		else
			counter = 0;
	}

	if(MAXSTATCD(ibuf.stopstat.parm2) >= MAXSTATCD(SRCPKTMAX))
		maxstatcd=MAXSTATCD(SRCPKTMAX);
	else
		maxstatcd=MAXSTATCD(ibuf.stopstat.parm2);


	statlen=STATLEN(maxstatcd);

	if (statlen > (unsigned)SRCPKTMAX) {
		shortrep(&retaddr,SRC_MMRY);
		(void) sigsetmask(0);                   /* release signals    */
		return;
	}

	optr  =  malloc(statlen); /* get status buffer      */
	if (optr ==  NULL)
	{
		shortrep(&retaddr,SRC_MMRY);
		(void) sigsetmask(0);
		return;
	}
	/* STATUS BUFFER ALLOCATED*/
	(void) memset(optr, 0, statlen);
	((struct srchdr *)optr)->cont=STATCONTINUED;

	statptr = (struct statcode *) (optr + sizeof(struct srchdr));
	if(!gothdr)
	{
		srcstathdr(hdr1,hdr2);
		gothdr=1;
	}
	strcpy(statptr->objname,hdr1);
	strcpy(statptr->objtext,hdr2);
	statptr++;
	curstatcd=1;
	for(ptr = frstaste; ptr != NULL  && counter > 0; ptr = ptr->next)
	{
		if(curstatcd==maxstatcd) SNDSTAT;

		/* current subsystem not targeted for status? */
		if(!isall() && !isgroup(ptr->subsys->grpname) && !issubsys(ptr->subsys->subsysname,ptr->svr_pid))
			continue;

		counter--;
		curstatcd++;

		/* this is a subsystem status response */
		statptr->objtype=SUBSYSTEM;

		/* format the bulk of the text
		**	1 group name
		**	2 pid
		**/
		sprintf(statptr->objtext,"%-16s %-5d  ",ptr->subsys->grpname,ptr->svr_pid);

		/* format the subsystem name */
		sprintf(statptr->objname," %-16s",ptr->subsys->subsysname);
		statptr->status =  ptr->state ;         /* put svr status in buf  */
		statptr++ ;                             /* increment buf ptr  */
	}

	/* give status for those inactive subsystems */
	for (vptr = frstsubsys; counter > 0 && vptr != (struct validsubsys *) 0; vptr = vptr->nsubsys)
	{
		if(curstatcd==maxstatcd) SNDSTAT;

		/* current subsystem not targeted for inoperative status? */
		if(vptr->forked || ((!isall() && !isgroup(vptr->subsys.grpname) || !vptr->subsys.display) && !issubsys(vptr->subsys.subsysname,0)))
			continue;

		statptr->objtype = SUBSYSTEM;

		/* format the group name */
		sprintf(statptr->objtext,"%-16s        ",vptr->subsys.grpname);

		/* format the subsystem name */
		sprintf(statptr->objname," %-16s",vptr->subsys.subsysname);
		statptr->status=SRCINOP;
		statptr++;
		counter--;
		curstatcd++;
	}
	(void) sigsetmask(0);
	if(curstatcd) SNDSTAT;

	((struct srchdr *)optr)->cont=END;
	dsrcsendpkt(&retaddr,optr,sizeof(struct srchdr));
	(void) free ( optr );

	return;
}
