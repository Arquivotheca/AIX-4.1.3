#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)69  1.1  src/bos/usr/ccs/lib/libcurses/V3.newterm.c, libcurses, bos411, 9428A410j 9/3/93 14:59:54";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: *		newterm
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

/* #ident	"@(#)curses:screen/V3.newterm.c	1.1"		*/



#include	"curses_inc.h"
extern	int	_outchar();

#ifdef	_VR3_COMPAT_CODE
extern	void	_update_old_y_area();

#undef	newterm
SCREEN	*
newterm(type, outfptr, infptr)
char	*type;
FILE	*outfptr, *infptr;
{
    _y16update = _update_old_y_area;
    return (newscreen(type, 0, 0, 0, outfptr, infptr));
}
#endif	/* _VR3_COMPAT_CODE */
