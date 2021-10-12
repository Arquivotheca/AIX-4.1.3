static char sccsid[] = "@(#)81	1.9  src/bos/usr/bin/src/cmds/srcterm.c, cmdsrc, bos411, 9428A410j 6/15/90 23:36:34";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controller
 *
 * FUNCTIONS:
 *	srcterm
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


#include <signal.h>                      /* signal definitions            */
/* #include <fcntl.h>                        file control include file     */
#include "src.h"
#include "src11.h"                       /* server profile structure      */
#include "src10.h"                       /* RTL structures                */
#include "srcmstr.h"
#include "srcsocket.h"

void srcterm ()
{
	extern void exit()    ;         /* Extern for exiting             */
	extern unsigned sleep();        /* Extern for sleep               */
	extern struct sigvec vecarray[] ;  /* struct to init signals  */

	int    rc                 ;     /* Return code variable           */
	struct activsvr *ptr      ;     /* Ptr to an   actve svr tbl entry*/
	struct activsvr *next     ;     /* Ptr to an   actve svr tbl entry*/
	int secs             ;     /* seconds to sleep               */

	struct validsubsys *vptr;
	struct validsubsys *vnext;

	/* close our comunication ports */
	term_src_sockets();

	/* ignore alarms for stop cancels */
	alarm((unsigned)0);
	sigvec(SIGALRM, &vecarray[3], (struct sigvec *) 0);

	/* ignore ofspring death so we are not interupted needlessly
  	**	will use SIG_DFL so src will not wait for children to
  	**	die before we exit
  	**/
	sigvec(SIGCLD, &vecarray[4],(struct sigvec *) 0);

	/* ignore more signals to terminate we already are in the process of
  	** terminating don't want to stack these sigs
  	**/
	sigvec(SIGTERM, &vecarray[3],(struct sigvec *) 0);

	/* warn subsystem to terminate (like a stop cancel) */
	for (ptr = frstaste,secs=0; ptr != NULL; ptr = ptr->next)
	{
		if(ptr->subsys->waittime > (short)secs)
			secs=ptr->subsys->waittime;
		kill( ptr->svr_pid, SIGTERM);
		if(ptr->subsys->contact == SRCSOCKET)
			unlink(ptr->sun.sun_path);
	}

	/* sleep a little bit while we wait for our subsystem's to die
	**  Note: wait longest time to wait for a subsystem
	**/
	sleep((unsigned)secs);


	/* kill subsystems and free active subsystem storage info */
	for (ptr = frstaste; ptr != NULL; ptr = next)
	{
		/* kill process group of the subsystem */
		kill( -(ptr->svr_pid), SIGKILL);
		next = ptr->next;
		(void) free ((char *)ptr);
	}

	/* free the valid subsystem table */
	for(vptr=frstsubsys;vptr!=(struct validsubsys *) 0;vptr=vnext)
	{
		vnext=vptr->nsubsys;
		free((char *)vptr);
	}

	/* remove our semaphore lock */
	unlock_srcmstr();

	/* remove socket directory */
	system(SRC_DESTROY_TMP_SOCKETS);

	(void) exit(0);
}
