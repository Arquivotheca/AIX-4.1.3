static char sccsid[] = "@(#)98	1.4  src/bos/kernext/disp/ped/diag/midexch.c, peddd, bos411, 9428A410j 3/31/94 21:32:49";
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: pio_exc_on
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/syspest.h>
#include <sys/types.h>
#include <sys/except.h>
#include <sys/adspace.h> /* for the io_att() & io_det() macros          */
#include <mid_dd_trace.h>	      /* Debug / Trace controls, macros  */


MID_MODULE (midexch) ;  
#define    dbg_middd   dbg_midexch



/*---------------------------------------------------------------------------
 *--------------------------------------------------------------------------*/


void
pio_exc_on(int exception, label_t *jumpbuf)
{
	  int 	rc ;


	/*----------
	  We setup the exception jumpbuf, then if we take an exception,
	  if it is not an IO exception, we pass it to the next handler
	  on the list. If it is ours, we print an error.
	  ----------*/

	BUGLPR(dbg_middd, 0, ("Exception %X occurred.\n", exception));

	if (exception == EXCEPT_IO)
	{
		BUGLPR(dbg_middd, 0,("IO exception occurred.\n"));

	}
	else {
		BUGLPR(dbg_middd, 0,("Not an IO exception.\n"));
		longjmpx(jumpbuf);
	}


	return;
}
