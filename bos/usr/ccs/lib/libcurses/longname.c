#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)88  1.6  src/bos/usr/ccs/lib/libcurses/longname.c, libcurses, bos411, 9428A410j 9/3/93 14:45:32";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: longname
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

/* #ident	"@(#)curses:screen/longname.c	1.6"		*/



/* This routine returns the long name of the terminal. */

char *
longname()
{
    extern	char	ttytype[], *strrchr();
    register	char	*cp = strrchr(ttytype, '|');

    if (cp)
	return (++cp);
    else
	return (ttytype);
}
