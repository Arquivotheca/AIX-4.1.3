#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)72  1.1  src/bos/usr/ccs/lib/libcurses/V3.vidattr.c, libcurses, bos411, 9428A410j 9/3/93 15:00:04";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: *		vidattr
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

/* #ident	"@(#)curses:screen/V3.vidattr.c	1.2"		*/



#include	"curses_inc.h"
extern	int	_outchar();

#ifdef	_VR3_COMPAT_CODE
#undef	vidattr

vidattr(a)
_ochtype	a;
{
    vidupdate(_FROM_OCHTYPE(a), cur_term->sgr_mode, _outchar);
    return (OK);
}
#endif	/* _VR3_COMPAT_CODE */
