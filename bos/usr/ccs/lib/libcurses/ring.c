#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)66  1.1  src/bos/usr/ccs/lib/libcurses/ring.c, libcurses, bos411, 9428A410j 9/3/93 15:11:30";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: _ring
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

/* #ident	"@(#)curses:screen/ring.c	1.5"		*/



#include	"curses_inc.h"

void
_ring(bf)
bool	bf;
{
    extern	int	_outch();
    static	char	offsets[2] = {45 /* flash_screen */, 1 /* bell */ };
    char	**str_array = (char **) cur_strs;
#ifdef	DEBUG
    if (outf)
	fprintf(outf, "_ring().\n");
#endif	/* DEBUG */
    _PUTS(str_array[offsets[bf]] ? str_array[offsets[bf]] : str_array[offsets[1 - bf]], 0);
    (void) fflush(SP->term_file);
    if (_INPUTPENDING)
	(void) doupdate();
}
