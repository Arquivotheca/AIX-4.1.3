static char sccsid[] = "@(#)59	1.2  src/bos/usr/bin/src/cmds/srcalrm.c, cmdsrc, bos411, 9428A410j 6/15/90 23:35:43";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controller
 *
 * FUNCTIONS:
 *	srcalrm
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
#include <signal.h>
#include "src11.h"
#include "src10.h"
#include "srcmstr.h"

extern int alarmset;
extern long nextalarm;
extern unsigned int alarm();
extern long time();

void srcalrm()
{
	/*-----------------------------*/
	/* LOCAL VARIABLES             */
	/*-----------------------------*/
	int    rc                 ;     /* Return code variable        */
	struct activsvr *prevptr  ;     /* Ptr to previous aste        */
	struct activsvr *ptr      ;     /* Ptr to an   actve svr entry */
	struct activsvr *nextptr   ;     /* Ptr to next actve svr entry */
	long   now                ;     /* current time                */
	int    svrpid             ;     /* local copy of svr pid in tbl*/

	long secstoalarm;

	long newtime;

#define MAXEARLY (long)0x7fffffff

	/* is there a subsystem out there with a current alarm */
	if(!alarmset)
		return;

	(void) sigblock(BLOCKMASK);	/* block signals */
	now = time((long *)0);		/* get current time */
	secstoalarm=MAXEARLY;

	for(ptr = frstaste, prevptr = NULL;ptr != NULL;)
	{
		/* have we been warned to stop? */
		if (ptr->state == SRCWARN)
		{
			/* get time remaining until this subsystems alarm */
			newtime=(ptr->warntime + ptr->subsys->waittime) - now;

			nextptr = ptr->next;	/* save next pointer */

			/* has an alarm gone off for this subsystem */
			if(newtime <= 0)
			{
				/* save subsystem pid to kill */
				svrpid = ptr->svr_pid;

				/* remove subsystem from active list */
				srcdsvr(ptr, prevptr);

				/* kill the subsystems process group */
				kill( -(svrpid), SIGKILL);
			}
			else if(secstoalarm > newtime)
			{
				secstoalarm = newtime;
				nextalarm=newtime + now;
				prevptr = ptr;
			}

			ptr = nextptr;
		}
		else
		{
			prevptr = ptr;
			ptr = ptr->next;
		}
	}

	if(alarmset)
		alarm((unsigned int)secstoalarm);

	(void) sigsetmask(0) ;               /* release signals */
	return;
}                                       /* end of srcalrm()             */
