static char sccsid[] = "@(#)79	1.3  src/bos/usr/ccs/lib/libc/cuexit.c, libcproc, bos412, 9446C 11/17/94 09:27:24";
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: atexit, exit
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>  /* for mon.h */
#include <mon.h>        /* for _mondata.proftype definition */
#include <sys/limits.h> /* for getting ATEXIT_MAX */
#include <sys/errno.h>  /* for setting errno */

#include "ts_supp.h"	

#ifdef _THREAD_SAFE
#include "rec_mutex.h"
extern struct rec_mutex _exit_rmutex;
#endif /* _THREAD_SAFE */

/*
 * NAME: atexit
 *
 * FUNCTION: Special program termination sequence.  atexit() may be used
 *	     to register up to 32 functions that are to be executed before
 *           normal program termination
 *
 * PARAMETERS: 
 *  	     *func() - pointer to function to be executed before normal
 *	   	       program termination
 *
 * RETURN VALUE DESCRIPTIONS:
 *	     - 0 if registration succeeds
 *	     - 1 if registration does not succeed
 *
 */


static struct atexitarray {
	void (*func)(void);	
} ex_actions[ATEXIT_MAX];

static int action_cnt;

int	
atexit(void (*func)(void))
{
	int rc;
	if (func == NULL) {
		errno = EFAULT;
		return(1);
	}

	TS_LOCK(&_exit_rmutex);
	if (action_cnt < ATEXIT_MAX) {
		ex_actions[action_cnt].func = func;
		action_cnt++;
		rc=0;
	} else {
		errno = EINVAL;
		rc=1;
	}
	TS_UNLOCK(&_exit_rmutex);
	return(rc);
}

/*
 * NAME: exit
 *
 * FUNCTION: Normal program termination
 *
 * PARAMETERS: 
 *	     int code - status of exit
 *
 */

void
exit(int status)
{
	extern struct monglobal _mondata; /* profiling global control data */
	static struct monglobal *z=&_mondata; /* pointer to them */

	/*********
	  don't bother unlocking as it would cause a race to get to the _exit
	*********/
	TS_LOCK(&_exit_rmutex);
	if ( z->prof_type != 0 ){    /* if any profiling active */
		monitor((caddr_t)0); /* stop and cleanup profiling */
	}

	while (action_cnt > 0)
		(*ex_actions[--action_cnt].func)();
	_cleanup();
	_exit(status);
}
