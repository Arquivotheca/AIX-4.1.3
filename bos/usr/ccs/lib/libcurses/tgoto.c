#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)83  1.6  src/bos/usr/ccs/lib/libcurses/tgoto.c, libcurses, bos411, 9428A410j 9/3/93 14:48:31";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: tgoto
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

/* #ident	"@(#)curses:screen/tgoto.c	1.5"		*/



/*
 * tgoto: function included only for upward compatibility with old termcap
 * library.  Assumes exactly two parameters in the wrong order.
 */
extern	char	*tparm();

char	*
tgoto(cap, col, row)
char	*cap;
int	col, row;
{
    char	*cp;

    cp = tparm(cap, row, col);
    return (cp);
}
