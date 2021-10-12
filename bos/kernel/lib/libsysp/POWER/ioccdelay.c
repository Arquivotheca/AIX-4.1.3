static char sccsid[] = "@(#)74	1.4  src/bos/kernel/lib/libsysp/POWER/ioccdelay.c, libsysp, bos411, 9428A410j 4/18/94 17:40:08";
/*
 * COMPONENT_NAME: LIBSYSP
 *
 * FUNCTIONS: iocc_delay(), io_delay()
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/adspace.h>
#include <sys/iocc.h>
#include <sys/syspest.h>
#include <sys/machine.h>
#include <sys/seg.h>
#include <sys/time.h>

/*
 * NAME: iocc_delay
 *
 * FUNCTION:  Provide IOCC delay function
 *
 * NOTES:
 *	This is provided to work around bug in fist pass RSC chips
 *
 * RETURNS: None
 */

void
iocc_delay(
	volatile char *ioccaddr,
	int usecs)
{
	int i;
	register char trash;

	ASSERT((usecs > 0) && (usecs <= 8));
	ASSERT(SEGOFFSET(ioccaddr) == IOCC_DELAY);

	trash = *(ioccaddr + usecs - 1);
}

/*
 * NAME: io_delay
 *
 * FUNCTION:
 *	This function will delay for at least N micro seconds before 
 *	returning.
 *
 * NOTES:
 *	io_delay is limited to a 999,999 microsecond delay.  i.e. less
 *	than 1 second.
 *
 * EXECUTION ENVIRONEMT:
 *	This may execute in the process or interrupt environments.
 *
 * RETURNS:
 *	None
 */
void
io_delay(
	int uSec)			/* number of microseconds to wait */
{
	struct timestruc_t	dtime;	/* delta time			*/
	struct timestruc_t	ctime;	/* current time			*/
	struct timestruc_t	etime;	/* ending time			*/
	ulong			nsec;

	/* get the current time */
	curtime( &ctime );
	if( uSec < uS_PER_SECOND ) {
		dtime.tv_sec = 0;
		dtime.tv_nsec = uSec * NS_PER_uS;

		/* generate an ending time */
		ntimeradd( dtime, ctime, etime ) 

		/* now wait for time to pass */
		while( 1 ) {
			curtime(&ctime);
			if( ntimercmp(ctime, etime, >) )
				break;
		}
	}

}
