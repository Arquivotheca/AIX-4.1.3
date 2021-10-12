#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)69  1.1  src/bos/usr/ccs/lib/libcurses/scr_dump.c, libcurses, bos411, 9428A410j 9/3/93 15:11:42";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: scr_dump
 *		
 *
 *   ORIGINS: 4
 *
 *                    SOURCE MATERIALS
 */
#endif /* _POWER_PROLOG_ */


/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/* #ident	"@(#)curses:screen/scr_dump.c	1.7"		*/



#include	"curses_inc.h"

/*
 * Dump a screen image to a file. This routine and scr_reset
 * can be used to communicate the screen image across processes.
 */

scr_dump(file)
char	*file;
{
    int		rv;
    FILE	*filep;

    if ((filep = fopen(file,"w")) == NULL)
    {
#ifdef	DEBUG
	if (outf)
	    (void) fprintf (outf, "scr_dump: cannot open \"%s\".\n", file);
#endif	/* DEBUG */
	return (ERR);
    }
    rv = scr_ll_dump(filep);
    fclose(filep);
    return (rv);
}
