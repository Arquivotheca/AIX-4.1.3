#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)06  1.4  src/bos/usr/ccs/lib/libcurses/curserr.c, libcurses, bos411, 9428A410j 4/15/94 16:57:04";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: curserr
 *		
 *
 *   ORIGINS: 27, 4
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
#endif /* _POWER_PROLOG_ */


/*
 *	Copyright (c) 1984 AT&T	
 *	  All Rights Reserved 

 *	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
 *	The copyright notice above does not evidence any 
 *	actual or intended publication of such source code.
 */

/* #ident	"@(#)curses:screen/curserr.c	1.5"		*/


#include 	"curses_inc.h"
#include	<locale.h>
#include	"libcurses_msg.h"

nl_catd		catd;

void
curserr()
{
    catd = catopen (MF_LIBCURSES, NL_CAT_LOCALE);

    switch (curs_errno) {
	case CURS_BAD_MALLOC: 
#ifdef DEBUG
			(void) fprintf(stderr, "malloc returned NULL in function %s.\n", curs_parm_err);
#else   /* no debug */
			(void) fprintf(stderr, catgets (catd, MS_LIBCURSES, 
			MALLOC, "malloc returned NULL.\n"));
#endif  /* DEBUG */
			break;
	case STUPID:  
			(void) fprintf(stderr, catgets (catd, MS_LIBCURSES, 
			NOTERM, "Terminal %s is unknown.\n"), curs_parm_err);
			break;
	case UNKNOWN:  
			(void) fprintf(stderr, catgets (catd, MS_LIBCURSES, 
			SPECTERM, "Please provide a more specific terminal type than %s.\n"), curs_parm_err);
			break;
    }

	catclose(catd);
}
