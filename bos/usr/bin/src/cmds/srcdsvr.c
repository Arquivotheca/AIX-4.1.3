static char sccsid[] = "@(#)70	1.3  src/bos/usr/bin/src/cmds/srcdsvr.c, cmdsrc, bos411, 9428A410j 6/15/90 23:36:07";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controller
 *
 * FUNCTIONS:
 *	srcdsvr
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
#include <signal.h>                      /* signal definitions            */

#include "src11.h"
#include "src10.h"
#include "srcmstr.h"


/*---------------------------------------------------------------------*/
/* Delete server function                                              */
/*---------------------------------------------------------------------*/

extern int alarmset;

void free_activsvr(subsys)
struct activsvr *subsys;
{
	/* free memory allocated for environment string */
	if(subsys->env)
		free((void *)subsys->env);

	/* free memory allocated for subsys args */
	if(subsys->parm)
		free((void *)subsys->parm);
	
	free((void *)subsys);
}

void srcdsvr(ptr, prevptr)
struct activsvr *ptr;     /* Ptr to an   actve svr entry */
struct activsvr *prevptr;     /* Ptr to previous aste        */
{
	struct activsvr *next     ;          /* Ptr to next actve svr entry */
	extern void delvalidsubsys();

	/* there are no active subsystems to delete */
	if (frstaste == NULL)
		return;

	(void) sigblock(BLOCKMASK);            /* block signals           */

	/* first subsystem on the list? */
	if (frstaste == ptr)
		frstaste = ptr->next;
	else
		prevptr->next = ptr->next;

	/* last subsystem on the list? */
	if (ptr == lastaste)
	{
		lastaste = prevptr;
		if (ptr == curraste)
			curraste = prevptr;
	}

	/* one of our instances of this subsystem has died */
	((struct validsubsys *)ptr->subsys)->forked--;

	/* delete this subsystem entry? */
	delvalidsubsys((struct validsubsys *)ptr->subsys);

	/* one less subsystem that has an alarm? */
	if(ptr->state == SRCWARN)
		alarmset--;

	/* free the active subsystem entry */
	free_activsvr(ptr);	
	(void) sigsetmask(0);                /* release signals        */
	return;
} 					/* end of delsvr()              */

