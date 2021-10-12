static char sccsid[] = "@(#)25	1.9  src/bos/usr/ccs/lib/libc/alarm.c, libctime, bos411, 9428A410j 3/4/91 15:58:00";
/*
 * COMPONENT_NAME: (LIBCTIME) Standard C Library Time Management Functions 
 *
 * FUNCTIONS: alarm 
 *
 * ORIGINS: 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include	<sys/types.h>
#include	<sys/errno.h>	/* declaration of errno variable	*/
#include	<sys/time.h>	/* to define the interval timer labels	*/
#include	<sys/events.h>	/* to define the POSIX delivery mechanis*/
#include	<sys/timer.h>	/* to define the corresponding timerid's*/

/*
 * NAME:  alarm
 *                                                                    
 * FUNCTION:  Set the value of an interval timer.
 *                                                                    
 * EXECUTION ENVIRONMENT:  
 *	
 *	This routine may only be called by a process.
 * 
 * NOTES:  
 *
 * DATA STRUCTURES:  
 *
 * RETURN VALUE DESCRIPTION:  
 *
 * EXTERNAL PROCEDURES CALLED: 
 */  
unsigned int
alarm(seconds)
register unsigned int seconds;
{
	register int rv;		/* return value from sys. calls	*/
	register timer_t timerid;	/* timer to issue request for	*/
	struct itimerstruc_t value;	/* time for alarm to occur	*/
	struct itimerstruc_t ovalue;	/* time left for pending alarm	*/
	int saverrno=errno;		/* In case incinterval fails */

	value.it_value.tv_sec = seconds;
	value.it_value.tv_nsec = 0;
	value.it_interval.tv_sec = 0;
	value.it_interval.tv_nsec = 0;
	ovalue.it_value.tv_sec = 0;
	ovalue.it_value.tv_nsec = 0;
	ovalue.it_interval.tv_sec = 0;
	ovalue.it_interval.tv_nsec = 0;

	rv = incinterval(TIMERID_ALRM, &value, &ovalue);
	if(rv == 0) {
		if(ovalue.it_value.tv_nsec >= 500000000)  {
			rv = ovalue.it_value.tv_sec + 1;
		}
		else  {
			rv = ovalue.it_value.tv_sec;
		}
		if((rv == 0) && ovalue.it_value.tv_nsec) {
			rv++;
		}
		return(rv);
	}
	else  {
		if(errno == EINVAL)  {
			timerid = gettimerid(TIMERID_ALRM, DELIVERY_SIGNALS);
			rv = incinterval(timerid, &value, &ovalue);
			if(rv == 0) {
				if(ovalue.it_value.tv_nsec >= 500000000)  {
					rv = ovalue.it_value.tv_sec + 1;
				}
				else  {
					rv = ovalue.it_value.tv_sec;
				}
				if((rv == 0) && ovalue.it_value.tv_nsec) {
					rv++;
				}
				errno=saverrno;
				return(rv);
			}
		}
	}
}
