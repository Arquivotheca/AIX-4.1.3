#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)94  1.4  src/bos/usr/ccs/lib/libcurses/termerr.c, libcurses, bos411, 9428A410j 4/15/94 17:00:47";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: termerr
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

/* #ident	"@(#)curses:screen/termerr.c	1.3"		*/


#include 	"curses_inc.h"
#include	<locale.h>
#include	"libcurses_msg.h"

nl_catd		catd;

void
termerr()
{
    catd = catopen (MF_LIBCURSES, NL_CAT_LOCALE);

    switch (term_errno) {
	case UNACCESSIBLE:
			(void) fprintf(stderr, catgets (catd, MS_LIBCURSES, 
			NOACCESS, "/usr/share/lib/terminfo is unaccessible.\n"));
			break;
	case NO_TERMINAL:
			(void) fprintf(stderr, catgets (catd, MS_LIBCURSES, 
			NOTERM, "Terminal %s is unknown.\n"), term_parm_err);
			break;
	case CORRUPTED: 
			(void) fprintf(stderr, catgets (catd, MS_LIBCURSES, 
			CORRUPT, "corrupted terminfo entry.\n"));
			break;
	case ENTRY_TOO_LONG:
			(void) fprintf(stderr, catgets (catd, MS_LIBCURSES, 
			LONGTERM, "terminfo entry is too long.\n"));
			break;
	case TERMINFO_TOO_LONG:  
			(void) fprintf(stderr, catgets (catd, MS_LIBCURSES, 
			LONGPATH, "terminfo pathname for device exceeds 512 characters.\n"));
			break;
	case TERM_BAD_MALLOC: 
#ifdef DEBUG
			(void) fprintf(stderr, "malloc returned NULL in function %s.\n", term_parm_err);
#else   /* no debug */
			(void) fprintf(stderr, catgets (catd, MS_LIBCURSES, 
			MALLOC, "malloc returned NULL.\n"));
#endif  /* DEBUG */
			break;
	case NOT_READABLE: 
			(void) fprintf(stderr, catgets (catd, MS_LIBCURSES, 
			NOTREAD, "Unable to read terminfo file for %s terminal.\n"), term_parm_err);
			break;
    }

	catclose(catd);
}
