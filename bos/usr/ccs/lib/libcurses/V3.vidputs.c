#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)73  1.1  src/bos/usr/ccs/lib/libcurses/V3.vidputs.c, libcurses, bos411, 9428A410j 9/3/93 15:00:08";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: *		vidputs
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

/* #ident	"@(#)curses:screen/V3.vidputs.c	1.2"		*/



#include	"curses_inc.h"
extern	int	_outchar();

#ifdef	_VR3_COMPAT_CODE
#undef	vidputs
vidputs(a, o)
_ochtype	a;
int		(*o)();
{
    vidupdate(_FROM_OCHTYPE(a), cur_term->sgr_mode, o);
    return (OK);
}
#endif	/* _VR3_COMPAT_CODE */
