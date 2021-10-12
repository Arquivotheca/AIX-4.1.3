static char sccsid[] = "@(#)00	1.11  src/bos/usr/ccs/lib/libcur/newwin.c, libcur, bos411, 9428A410j 5/14/91 17:01:49";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: newwin, subwin, _makenew
 *
 * ORIGINS: 10, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include        "cur99.h"

/*
 * NAME:                newwin
 *
 * FUNCTION: This routine allocates the space for a
 *      new window and sets up all the defaults associated with that type
 *      of window.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      newwin(num_lines, num_cols, begy, begx), where 'num_lines'
 *      & 'num_cols' are the size of the window, and 'begy' & 'begx' are
 *      the glass starting coordinates of the window.
 *
 * EXTERNAL REFERENCES: calloc(), malloc(), cfree(), delwin(), free()
 *
 * DATA STRUCTURES:     WINDOW (struct _win_st)
 *
 * RETURNS:             normal -> <WINDOW *>    error -> ERR
 */

WINDOW *
newwin(num_lines, num_cols, begy, begx)
int     num_lines,
        num_cols,
        begy,
        begx;
{
    register    WINDOW  *win;
    register    NLSCHAR *sp;
    register    ATTR    *ap;
    register int    i;
    register unsigned   numl,
                        numc;
#ifdef OLD_STYLE	/* by Tz Apr/23/91 */
    char    *calloc ();
#endif
    WINDOW  *_makenew();

    numl = (unsigned) num_lines;
    numc = (unsigned) num_cols;

    if (numl == 0)
	numl = LINES - begy;
    if (numc == 0)
	numc = COLS - begx;
    if ((win = _makenew(numl, numc, begy, begx)) == NULL)
	return((WINDOW *) ERR);
    for (i = 0; i < numl; i++) {
	if ((win->_y[i] =
		((NLSCHAR *) calloc(numc, sizeof(NLSCHAR)))) == NULL) {
	    win->_maxy = i;	/* quick & cheap for delwin() */
	    delwin(win);	/* free all previously allocated space */
	    return((WINDOW *) ERR);
	}
	else
	    if ((win->_a[i] = ((ATTR *)calloc(numc, sizeof(ATTR)))) == NULL) {
		win->_maxy = i;	/* quick & cheap for delwin() */
		cfree((char *) win->_y[i]);/* free last one of _y */
		delwin(win);	/* free all previously allocated space */
		return((WINDOW *) ERR);
	    }
	else
	    for (sp = win->_y[i], ap = win->_a[i]; sp < win->_y[i] + numc;
		    *sp++ = NLSBLANK, *ap++ = NORMAL);
    }				/* end: for(i = 0;... */
    return win;
}


/*
 * NAME:                subwin
 *
 * FUNCTION: This routine allocates the space for a
 *      sub window and sets up all the defaults associated with that type
 *      of window.
 */

WINDOW *
subwin(orig, num_lines, num_cols, begy, begx)
register    WINDOW  *orig;
int     num_lines,
        num_cols,
        begy,
        begx;
{
    register    WINDOW  *win;
    register unsigned   numl,
                        numc;
    register int    i,
                    j,
                    k;
    WINDOW  *_makenew();

    numl = (unsigned) num_lines;
    numc = (unsigned) num_cols;

/*
 * make sure window fits inside the original one
 */
    if (begy < orig->_begy || begx < orig->_begx
				/* origin and size must */
	    || begy + numl > orig->_maxy + orig->_begy
				/* - fit inside base   */
	    || begx + numc > orig->_maxx + orig->_begx)
				/* - window, check with */
				/* - display locations */
	return((WINDOW *) ERR);

				/* if size is null then */
				/* - calculate size to */
				/* - end of base win   */
				/* - note- if base was */
				/* - off display sub   */
				/* - will be also      */
    if (numl == 0)
	numl = orig->_maxy + orig->_begy - begy;
    if (numc == 0)
	numc = orig->_maxx + orig->_begx - begx;
    if ((win = _makenew(numl, numc, begy, begx)) == NULL)
	return((WINDOW *) ERR);

    j = begy - orig->_begy;	/* origin in base win  */
    k = begx - orig->_begx;	/* - data of subwin    */

    for (i = 0; i < numl; i++, j++) {
	win->_y[i] = &orig->_y[j][k];
	win->_a[i] = &orig->_a[j][k];
    }
    win->_flags |= _SUBWIN;
    return win;
}


/*
 * NAME:                _makenew
 *
 * FUNCTION: This routine sets up a window buffer and returns a pointer to
 * it.
 */

WINDOW *
_makenew(num_lines, num_cols, begy, begx)
register unsigned   num_lines,
                    num_cols;
register int    begy,
                begx;
{
    register    WINDOW  *win;
    register int    i;
#ifdef OLD_STYLE /* by Tz Apr/23/91 */
    char    *calloc ();
    WINDOW  *malloc();
#endif

#define MAXSHORT 0x7fff

    if (((int) num_lines < 0) ||        /* if size is invalid return    */
	((int) num_lines > MAXSHORT) || /* with null for error code     */
	((int) num_cols < 0) ||
	((int) num_cols > MAXSHORT))
	return(NULL);

    if ((win = (WINDOW *) malloc(sizeof(WINDOW))) == NULL)
	return(NULL);
    if ((win->_y =
	((NLSCHAR **) calloc(num_lines, sizeof(NLSCHAR *)))) == NULL) {
	free(win);
	return(NULL);
    }
    if ((win->_a = ((ATTR **) calloc(num_lines, sizeof(ATTR *)))) == NULL) {
	free(win);
	cfree(win->_y);
	return(NULL);
    }
    if ((win->_firstch =
		((short *) calloc(num_lines, sizeof(short)))) == NULL) {
	free(win);
	cfree(win->_y);
	cfree(win->_a);
	return(NULL);
    }
    if ((win->_lastch =
		((short *) calloc(num_lines, sizeof(short)))) == NULL) {
	free(win);
	cfree(win->_y);
	cfree(win->_a);
	cfree(win->_firstch);
	return(NULL);
    }

    win->_cury = win->_curx = win->_flags = 0;
    win->_clear = (num_lines == LINES && num_cols == COLS);
    win->_maxy = num_lines;
    win->_maxx = num_cols;
    win->_begy = begy;
    win->_begx = begx;
    win->_scroll = win->_leave = FALSE;
    win->_tmarg = 0;
    win->_bmarg = num_lines - 1;
    win->_csbp = NORMAL;
    win->_view = (WINDOW *) NULL;

    for (i = 0; i < num_lines; i++)
	win->_firstch[i] = win->_lastch[i] = _NOCHANGE;
    if (begx + num_cols == COLS) {
	win->_flags |= _ENDLINE;
	if (begx == 0 && num_lines == LINES && begy == 0)
	    win->_flags |= _FULLWIN;
    }
/* if the window includes the lower right-hand corner, it's a _SCROLLWIN */
    if ((begy + num_lines >= LINES) && (begx + num_cols >= COLS)
	    && (begy < LINES) && (begx < COLS))
	win->_flags |= _SCROLLWIN;

    return win;
}
