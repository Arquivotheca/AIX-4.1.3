static char sccsid[] = "@(#)89	1.8  src/bos/usr/ccs/lib/libs/libsmsg.c, libs, bos411, 9428A410j 12/8/93 11:28:11";
/*
 * COMPONENT_NAME: (LIBS) security library functions
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
#include "libs_msg.h"

static	nl_catd	libscatd = NULL;

/*
 * NAME: setlibsmsg, getlibsmsg, endlibsmsg
 *                                                                    
 * FUNCTION: set, get, and end libs message catalogue 
 *                                                                    
 * EXECUTION ENVIRONMENT: called by the routines in libs
 *
 * 	The idea is that the invoker of any security library routine
 *	will not have to manage the message catalogue. The catalogue
 *	is opened automatically by getlibsmsg() and closed by endlibsmsg.
 *	However, the call to endlibsmsg must be made by the invoker.
 *
 */  
static int
setlibsmsg ()
{
	if (libscatd == NULL)
		libscatd = catopen (MF_LIBS, NL_CAT_LOCALE);
}
char *
getlibsmsg (num, str)
int	num;
char	*str;
{
	if (libscatd == NULL)
		setlibsmsg ();
	return (catgets (libscatd, MS_LIBS, num, str));
}
int
endlibsmsg ()
{
	if (libscatd)
	{
		catclose (libscatd);
		libscatd = NULL;
	}
}
