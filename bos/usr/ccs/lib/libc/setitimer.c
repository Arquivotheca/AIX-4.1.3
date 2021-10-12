static char sccsid[] = "@(#)57	1.14  src/bos/usr/ccs/lib/libc/setitimer.c, libctime, bos411, 9428A410j 5/23/94 17:43:48";
/*
 * COMPONENT_NAME: (LIBCTIME) Standard C Library Time Management Functions 
 *
 * FUNCTIONS: setitimer 
 *
 * ORIGINS: 26, 27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include	<sys/types.h>
#include	<sys/errno.h>	/* declaration of errno variable	*/
#include	<sys/time.h>	/* to define the interval timer labels	*/
#include	<sys/timer.h>	/* to define the corresponding timerid's*/
#include	<sys/events.h>	/* to define POSIX delivery mechanisms	*/

/*
 * NAME:  setitimer
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
 * RETURN VALUE DESCRIPTION:  0 upon successful completion, -1 upon error
 *	with errno set as follows:
 *	EINVAL:	the value parameter specified a time that was too large
 *	EINVAL:	the which parameter does not specify a valid interval timer
 *	EFAULT:	the value parameter specified a bad address
 *
 * EXTERNAL PROCEDURES CALLED: 
 */  
int
setitimer(int which, const struct itimerval *value, struct itimerval *ovalue)
{
	register int rv;		/* return value from sys. calls	*/
	register int abs;		/* an absolute value passed in?	*/
	register timer_t timerid;	/* parameter to POSIX sys. calls*/
	int	 saverrno = errno;
	struct itimerstruc_t sys_ival,sys_oval,*sys_ivalp;
				        /* Params to POSIX sys calls*/

	if ( value != NULL )  {  /* Verify input parameter if not NULL */

		if((value->it_value.tv_usec >= 1000000)  ||
	   		(value->it_value.tv_usec < 0)  ||
	   		(value->it_interval.tv_usec >= 1000000)  ||
	   		(value->it_interval.tv_usec < 0))  {
			errno = EINVAL;
			return(-1);
		}

		/*
	 	*  Convert the microseconds specified to nanoseconds like 
	 	*  incinterval() expects.
	 	*/
		sys_ival.it_value.tv_sec  = value->it_value.tv_sec ;
		sys_ival.it_value.tv_nsec = value->it_value.tv_usec * 1000;

		sys_ival.it_interval.tv_sec=  value->it_interval.tv_sec ;	
		sys_ival.it_interval.tv_nsec= value->it_interval.tv_usec * 1000;

		sys_ivalp=&sys_ival;
	}    
	else
		sys_ivalp=NULL;

	switch(which)  {
		case ITIMER_REAL:
		case ITIMER_REAL1:
			abs = 0;
			break;
		case ITIMER_PROF:
		case ITIMER_VIRTUAL:
		case ITIMER_VIRT:
			abs = 1;
			break;
		default:
			errno = EINVAL;
			return(-1);
	}

	if(abs)  {
		rv = absinterval((timer_t)which, sys_ivalp, &sys_oval);
	}
	else  {
		rv = incinterval((timer_t)which, sys_ivalp, &sys_oval);
	}

	if(rv == 0) {
		if(ovalue == NULL)
			return(0);
  		ovalue->it_value.tv_sec = 
				sys_oval.it_value.tv_sec ;
        	ovalue->it_value.tv_usec = 
				sys_oval.it_value.tv_nsec / 1000;

        	ovalue->it_interval.tv_sec = 
				sys_oval.it_interval.tv_sec ;             
        	ovalue->it_interval.tv_usec = 
				sys_oval.it_interval.tv_nsec / 1000;

		return(0);
	}
	else  {
		if(errno == EINVAL)  {
			timerid = gettimerid((timer_t)which, 
						DELIVERY_SIGNALS);
			if(abs)  {
				rv = absinterval(timerid, sys_ivalp, &sys_oval);
			}
			else  {
				rv = incinterval(timerid, sys_ivalp, &sys_oval);
			}
			if(rv == 0) {
				errno = saverrno;
				if(ovalue == NULL)
					return(0);
      				ovalue->it_value.tv_sec = 
					 sys_oval.it_value.tv_sec ;
        			ovalue->it_value.tv_usec = 
					 sys_oval.it_value.tv_nsec / 1000;

        			ovalue->it_interval.tv_sec = 
					 sys_oval.it_interval.tv_sec ;             
        			ovalue->it_interval.tv_usec = 
					 sys_oval.it_interval.tv_nsec / 1000;

				return(0);
			}
		}
	}

	return(-1);
}
