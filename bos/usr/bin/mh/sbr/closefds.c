static char sccsid[] = "@(#)41	1.4  src/bos/usr/bin/mh/sbr/closefds.c, cmdmh, bos411, 9428A410j 11/6/93 13:17:50";
/* 
 * COMPONENT_NAME: CMDMH closefds.c
 * 
 * FUNCTIONS: closefds 
 *
 * ORIGINS: 10  26  27  28  35 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/* closefds.c - close-up fd:s */

#include "mh.h"
#include <stdio.h>
extern nl_catd catd;
/*
#ifndef	BSD42
#include <stdio.h>
#endif	not BSD42
*/


void	closefds (i)
register int	i;
{
	int cat_fd = -1;
#ifndef	BSD42
    int     nbits = _NFILE;
#else	BSD42
    int     nbits = getdtablesize ();
#endif	BSD42

   if ( catd != NULL )
	cat_fd = fileno(catd->_fd ) ;
    for (; i < nbits; i++)
#ifdef	OVERHEAD
	if (i != fd_def && i != fd_ctx)
#endif	OVERHEAD
		/* do not close msg catlog fd */
	    if ( i != cat_fd  )
	       (void) close (i);
}
