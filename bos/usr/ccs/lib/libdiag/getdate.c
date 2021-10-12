static char sccsid[] = "@(#)41	1.5  src/bos/usr/ccs/lib/libdiag/getdate.c, libdiag, bos411, 9428A410j 3/8/94 09:41:49";
/*
 * COMPONENT_NAME: (LIBDIAG)  DIAGNOSTICS LIBRARY
 *
 * FUNCTIONS: getdate
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include	<stdio.h>
#include	<sys/types.h>
/*
 * UU Changes: the prototype for the libc function getdate()
 * was added to time.h.  The prototype is within ifdef
 * _ALL_SOURCE.
 */
#undef		_ALL_SOURCE
#include	<time.h>
#define		_ALL_SOURCE

/*
 * NAME: getdate
 *                                                                    
 * FUNCTION:  	This function obtains the current date and time.
 * 		It is returned in the buffer given as input.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This should describe the execution environment for this
 *	procedure. For example, does it execute under a process,
 *	interrupt handler, or both. Can it page fault. How is
 *	it serialized.
 *                                                                   
 * RETURNS: 0
 */  

/*
struct		tm *localtime();
long		time();
*/

 int
getdate(buffer,len)
char *buffer;
int len;

{
	long		time_loc;
	struct tm	*date;

	time_loc = time((long *)0);
	date = localtime(&time_loc);
	strftime(buffer, len, "%c", date);

	return(0);
}
