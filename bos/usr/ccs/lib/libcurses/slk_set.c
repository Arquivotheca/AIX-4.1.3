#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)87  1.1  src/bos/usr/ccs/lib/libcurses/slk_set.c, libcurses, bos411, 9428A410j 9/3/93 15:13:01";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: slk_set
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

/* #ident	"@(#)curses:screen/slk_set.c	1.4"		*/



#include	"curses_inc.h"

/*
 * Set a soft label.
 *
 * n:	label number
 * lab:	the string
 * f:	0, 1, 2 for left, center, right-justification
 */

slk_set(n, lab, f)
int	n;
char	*lab;
int	f;
{
    register	SLK_MAP	*slk = SP->slk;
    register	int	len, slklen = slk->_len, left;
    char		*cp, nlab[LABLEN + 1];

    if ((slk == NULL) || f < 0 || f > 2 || n < 1 || n > slk->_num)
	return (ERR);

    /* 0-indexing internally */
    n--;

    if (lab == NULL)
	lab = "";

    /* chop lengthy label */
    if ((len = strlen(lab)) > slklen)
	lab[len = slklen] = '\0';

    /* make the new display label */
    for (cp = nlab + slklen - 1; cp >= nlab; cp--)
	*cp = ' ';
    nlab[slklen] = '\0';
    if (f == 0)
	left = 0;
    else
	left = (slklen - len) / ((f == 1) ? 2 : 1);

    (void) memcpy(nlab + left, lab, len);

    if (strcmp(slk->_ldis[n], nlab) != 0)
    {
	(void) memcpy(slk->_lval[n], lab, len + 1);
	(void) memcpy(slk->_ldis[n], nlab, slklen + 1);
	slk->_changed = slk->_lch[n] = TRUE;
    }

    return (OK);
}
