static char sccsid[] = "@(#)21	1.1  src/bos/usr/ccs/lib/libprint/close.c, libprint, bos411, 9428A410j 9/30/89 15:37:43";
/*
 * COMPONENT_NAME: libplot
 *
 * FUNCTIONS: closevt, closepl
 *
 * ORIGINS: 4,10,27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */


#include    <stdio.h>
#include    <lprio.h>
extern      struct  lprmode     lprmode;
extern      int     lpmode;
extern      int     printempty;

closepl(){
	if (!printempty)
	    doscreen ();
	fprintf (stdout, "\033A\014\0332");
	fflush (stdout);
	if (lpmode != lprmode.modes) {
	    lprmode.modes = lpmode;
	    ioctl (fileno (stdout), LPRSETV, &lprmode);
	    }
	}

closevt (){
	closepl ();
	}

