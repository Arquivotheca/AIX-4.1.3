static char sccsid[] = "@(#)23	1.6  src/bos/usr/ccs/lib/libcur/bell.c, libcur, bos411, 9428A410j 6/16/90 01:36:41";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: bell
 *
 * ORIGINS: 3, 10, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include        "cur99.h"

/*
 * NAME:                bell
 *
 * FUNCTION: This routine will beep the speaker or
 *      flash the screen, depending on the parameter sent or what is
 *      actually available.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      bell(type), where 'type' is either a 1 or a 2
 *
 * EXTERNAL REFERENCES: _puts(), fflush()
 *
 * DATA STRUCTURES:     n/a
 *
 * RETURNS:             normal -> OK            error -> ERR
 */

bell(type)
register short  type;
{
    switch (type) {
	case 1: 
	    if (BL)
		_puts(BL)       /* try termcap entry for bell, */
		    else
		_puts("\007")   /* then try <cntrl g>          */
		    break;
	case 2: 
	    if (VB)
		_puts(VB)       /* try termcap entry for visual */
		    else
		return bell(1);	/* bell, then try bell          */
	    break;
	default: 
	    return ERR;
    }

    eciofl(stdout);		/* flush in case of i/o buffering */

    return OK;
}

