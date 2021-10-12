static char sccsid[] = "@(#)45	1.7  src/bos/usr/ccs/lib/libcur/deleteln.c, libcur, bos411, 9428A410j 6/16/90 01:37:34";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: wdeleteln, winsdel
 *
 * ORIGINS: 3, 10, 27
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
 * NAME:                wdeleteln
 *
 * FUNCTION: This routine deletes a line from the
 *      window at the current location (line); it shifts all lower lines
 *      up one, and adds a blank & _csbp line at the bottom.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      wdeleteln(win), where 'win' is a pointer to the window.
 *
 * EXTERNAL REFERENCES: none
 *
 * DATA STRUCTURES:     WINDOW (struct _win_st)
 *
 * RETURNS:             normal -> OK            error -> ERR
 */

wdeleteln(w)			/* delete line function         */
WINDOW  *w;
{
    return(winsdel(FALSE, w));	/* invoke real delete function  */
}

/*
 * NAME:                winsdel
 */

winsdel(insreq, win)
char    insreq;			/* true if request is insert    */
				/* else do delete               */
register    WINDOW  *win;
{
    register    WINDOW  *vwin;
    register int    y;
    register    ATTR    *ap;
    register    NLSCHAR *sp;
    NLSCHAR *end;
    int     lastx,
            lasty;
    int     sy;

    vwin = NULL;		/* initialize for no view case  */
    y = win->_cury;		/* set y to current line        */

    if (win->_flags & _ISVIEW) {/* if argument is a viewport    */
	vwin = win;		/* set view pointer             */
	win = vwin->_view;	/* set win to real window       */
	y = vwin->_cury + vwin->_winy;/* calc row in real win to del. */
    }

    else
	if (win->_flags & _HASVIEW) {/* if argument has a view       */
	    vwin = win->_view;	/* set view ptr for that view   */
	}

    sy = y;			/* save start row value         */
    lasty = win->_maxy - 1;	/* get last row value           */
    lastx = win->_maxx - 1;	/* get last column value        */

    if (insreq) {		/* if processing insert request */
	sp = win->_y[lasty];	/* start of line characters     */
	ap = win->_a[lasty];	/* start of line attribute code */

	for (; y < lasty; lasty--) {/* shift all the lines down one */
	    win->_y[lasty] = win->_y[lasty - 1];
	    win->_a[lasty] = win->_a[lasty - 1];
	    win->_firstch[lasty] = 0;/* mark full line as changed    */
	    win->_lastch[lasty] = lastx;
	}
    }

    else {			/* handle delete line here      */

	sp = win->_y[y];	/* start of line characters     */
	ap = win->_a[y];	/* start of line attribute code */

	for (; y < lasty; y++) {/* shift all the lines up one   */
	    win->_y[y] = win->_y[y + 1];
	    win->_a[y] = win->_a[y + 1];
	    win->_firstch[y] = 0;/* mark full line as changed    */
	    win->_lastch[y] = lastx;
	}
    }
				/* note - on exit from above    */
				/* y = lasty in both cases      */
				/* for insert y matches the cury */
				/* value for delete y matches   */
				/* maxy value. thus following   */
				/* will set the pointer for the */
				/* corresponding 'y' row to the */
				/* data area for the line that  */
				/* was dropped from the p-space */

    win->_y[y] = sp;		/* set row pointers to row that */
    win->_a[y] = ap;		/* was dropped ins - last row   */
				/* del - cury                   */
    win->_firstch[y] = 0;	/* flag that row as changed     */
    win->_lastch[y] = lastx;

    for (end = &sp[lastx]; sp <= end;) {
				/* now clear the new line       */
	*ap++ = win->_csbp;	/* set attr to cuurent attr     */
	*sp++ = NLSBLANK;	/* - and data to blank          */
    }

    if (vwin != NULL) {		/* if a view is involved        */

	register int    vy;	/* declared in block to use reg */

	y = sy;			/* get top change row in window */
	vy = y - vwin->_winy;	/* calc top change rel to view  */

	if (vy < 0) {		/* if outside view              */
	    vy = 0;		/* set to view lower bound      */
	    y = vwin->_winy;	/* also for pointer to window   */
	}

	for (; vy < vwin->_maxy;/* while within the view        */
		y++, vy++) {	/* step both row pointers       */
				       /* recalculate view row start   */
	    vwin->_y[vy] = &(win->_y[y][vwin->_winx]);
	    vwin->_a[vy] = &(win->_a[y][vwin->_winx]);
	    vwin->_firstch[vy] = 0;/* mark view as changed         */
	    vwin->_lastch[vy] = vwin->_maxx - 1;
	}
    }
    return OK;
}
