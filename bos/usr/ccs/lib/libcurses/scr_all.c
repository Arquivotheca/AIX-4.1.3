#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)68  1.1  src/bos/usr/ccs/lib/libcurses/scr_all.c, libcurses, bos411, 9428A410j 9/3/93 15:11:38";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: _scr_all
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

/* #ident	"@(#)curses:screen/scr_all.c	1.3"		*/



#include	"curses_inc.h"

/* Set <screen> idea of the screen image to that stored in "file". */

_scr_all(file,which)
char	*file;
int	which;
{
    int		rv;
    FILE	*filep;

    if ((filep = fopen(file,"r")) == NULL)
	return (ERR);
    rv = scr_reset(filep,which);
    fclose(filep);
    return (rv);
}
