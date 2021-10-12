static char sccsid[] = "@(#)22	1.7  src/bos/usr/ccs/lib/libcur/ecloct.c, libcur, bos411, 9428A410j 6/16/90 01:38:56";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: eclocmv, eclocst, eclocsh, eclocrs
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include        "cur99.h"
#include        "cur05.h"

/*      global externs declared in the CUR05.h file, defined here       */

PANEL   *LC_PNL = NULL;         /* panel containing locator     */
PANE    *LC_PANE = NULL;        /* pane containing locator      */
int     LC_ROW = 0;		/* row in LC_PAN for locator    */
int     LC_COL = 0;		/* column in LC_PAN for locator */

/*	Static data areas - global to this set of functions only     */

static
int     LOC_ROW = 0;		/* Row containing locator       */

static
int     LOC_COL = 0;		/* Column containing locator    */

static
int     CLOC_ROW = 0;		/* current row locator shown in */

static
int     CLOC_COL = 0;		/* current col locator shown in */

static
char    HAVE_LOC = FALSE;	/* Have any locator inputs been */
				/* - seen yet.                  */

/*
 * NAME:                eclocmv
 *
 * FUNCTION:            Adjust locator position based on change
 *                      distance for cursor.
 *
 * EXECUTION ENVIRONMENT:
 *
 *   PARAMETERS:        dy - integer, movement for locator vertically
 *                           positive is down
 *                      dx - integer, movement for locator
 *                           horizontally, positive is to right
 *
 *   INITIAL CONDITIONS none
 *
 *   FINAL CONDITIONS:
 *     NORMAL:          locator static variables updated, if delta
 *                      would move outside display stop at boundry
 *
 *     ABNORMAL:        n/a
 *
 * EXTERNAL REFERENCES: ROWS, COLS
 *
 * RETURNED VALUES:     OK - always
 */

int     eclocmv (dy, dx)	/* define function		 */
int     dy;			/* change in row for locator	 */
int     dx;			/* change in column for loc.	 */

{				/* begin function eclocmv	 */
    if ((dy != 0) || (dx != 0)) {/* if delta is not null         */
	HAVE_LOC = TRUE;	/* flag - locator position valid */

	LOC_ROW += dy;		/* calc new row 		 */
	if (LOC_ROW < 0)	/* if not valid, force to	 */
	    LOC_ROW = 0;	/* - margin value		 */
	else
	    if (LOC_ROW >= LINES)/* same for upper limit 	 */
		LOC_ROW = LINES - 1;/* 				 */

	LOC_COL += dx;		/* calc new column		 */
	if (LOC_COL < 0)	/* if not valid, force to	 */
	    LOC_COL = 0;	/* - margin value		 */
	else
	    if (LOC_COL >= COLS)/* same for upper limit 	 */
		LOC_COL = COLS - 1;/* 				 */

	if ((LOC_ROW == LINES - 1) &&/* if moved to lower right      */
		(LOC_COL == COLS - 1) &&
		AM != NULL) {
	    LOC_COL--;		/* decrement column by one      */
	}

    }
    return(OK);			/* return to caller		 */
}				/* end function eclocmv 	 */

/*
 * NAME:                eclocst
 *
 * FUNCTION:            Move locator to specified position.
 *
 * EXECUTION ENVIRONMENT:
 *
 *   PARAMETERS:        ly - integer, row for locator cursor
 *                      lx - integer, column for locator cursor
 *
 *   INITIAL CONDITIONS none
 *
 *   FINAL CONDITIONS:
 *     NORMAL:          locator static variables updated, if
 *                      location is off screen set to boundry
 *
 *     ABNORMAL:        n/a
 *
 * EXTERNAL REFERENCES: ROWS, COLS
 *
 * RETURNED VALUES:     OK - always
 */

extern char SHOW_LOC;

int     eclocst (ly, lx)	/* define function		 */
int     ly;			/* row for locator cursor	 */
int     lx;			/* column for locator cursor	 */


{				/* begin function eclocst	 */

    HAVE_LOC = TRUE;		/* flag - locator position valid */

    LOC_ROW = ly;		/* set new row			 */
    if (LOC_ROW < 0)		/* if not valid, force to	 */
	LOC_ROW = 0;		/* - margin value		 */
    else
	if (LOC_ROW >= LINES)	/* same for upper limit 	 */
	    LOC_ROW = LINES - 1;/* 				 */

    LOC_COL = lx;		/* calc new column		 */
    if (LOC_COL < 0)		/* if not valid, force to	 */
	LOC_COL = 0;		/* - margin value		 */
    else
	if (LOC_COL >= COLS)	/* same for upper limit 	 */
	    LOC_COL = COLS - 1;	/* 				 */

    if ((LOC_ROW == LINES - 1) &&/* if moved to lower right      */
	    (LOC_COL == COLS - 1) &&
	    AM != NULL) {
	LOC_COL--;		/* decrement column by one      */
    }

    if (!SHOW_LOC) {		/* initialize variables         */
	CLOC_ROW = LOC_ROW;
	CLOC_COL = LOC_COL;
    }

    return(OK);			/* return to caller		 */
}				/* end function eclocst 	 */

/*
 * NAME:                eclocsh
 *
 * FUNCTION:            Present locator on display.
 *
 * EXECUTION ENVIRONMENT:
 *
 *   PARAMETERS:        none
 *
 *   INITIAL CONDITIONS none
 *
 *   FINAL CONDITIONS:
 *     NORMAL:          Locator displayed on terminal.
 *
 *     ABNORMAL:        n/a
 *
 * EXTERNAL REFERENCES: ROWS, COLS
 *
 * RETURNED VALUES:     OK - always
 */

int     eclocsh () {            /* define function - no parms,
				   begin function eclocsh	 */

    int     cy,
            cx;			/* text cursor location         */
    int     ay,
            ax;			/* actual terminal cursor loc   */


    if (HAVE_LOC && SHOW_LOC) {	/* if locator input encountered */
	getyx(curscr, cy, cx);	/* get current text cursor loc  */
	ay = cy;		/* make working copy            */
	ax = cx;

	if ((CLOC_ROW != LOC_ROW) |/* if locator is moving         */
		(CLOC_COL != LOC_COL)) {
	    domvcur(ay, ax, CLOC_ROW, CLOC_COL);
				/* move terminal cursor to old  */
				/* - locator cursor location    */

	    chg_attr_mode(curscr->_csbp, curscr->_a[CLOC_ROW][CLOC_COL]);
				/* assure attr is that for char */

	    curscr->_csbp = curscr->_a[CLOC_ROW][CLOC_COL];
				/* change 'current attr code'   */

	    eciopc(curscr->_y[CLOC_ROW][CLOC_COL]);
				/* write original char          */
	    ay = CLOC_ROW;	/* update actual cursor loc     */
	    ax = CLOC_COL + 1;

	    CLOC_ROW = LOC_ROW;	/* change current locator locn  */
	    CLOC_COL = LOC_COL;
	}			/* end - if locator moved       */

	domvcur(ay, ax, CLOC_ROW, CLOC_COL);
				/* move terminal cursor to      */
				/* - locator cursor location    */

	chg_attr_mode(curscr->_csbp, REVERSE ^ curscr->_a[CLOC_ROW][CLOC_COL]);
				/* assure attr is that for char */

	curscr->_csbp = REVERSE ^ curscr->_a[CLOC_ROW][CLOC_COL];
				/* change 'current attr code'   */

	eciopc(curscr->_y[CLOC_ROW][CLOC_COL]);
				/* write original char          */
	ay = CLOC_ROW;		/* update actual cursor loc     */
	ax = CLOC_COL + 1;

	domvcur(ay, ax, cy, cx);/* move terminal cursor to      */
    /* - locator text location      */

	eciofl(stdout);		/* make it visible on display   */

    }				/* end - if locator defined     */

    return(OK);			/* return to caller		 */
}				/* end function eclocsh 	 */

/*
 * NAME:                eclocrs
 *
 * FUNCTION:            Resolve locator position into panel/pane
 *
 * EXECUTION ENVIRONMENT:
 *
 *   PARAMETERS:        none
 *
 *   INITIAL CONDITIONS none
 *
 *   FINAL CONDITIONS:
 *     NORMAL:          Global variables LC_PNL, LC_PANE, LC_ROW
 *                      LC_COL set to indicate locator position
 *
 *     ABNORMAL:        LC_PANE is NULL if failure
 *
 * EXTERNAL REFERENCES: _toppl
 *
 * RETURNED VALUES:     OK - if locator in a pane
 *                      ERR - if locator outside all panes or on a
 *                            pane boundry or if locator not valid
 */

int     eclocrs () {            /* define function - no parms,
				   begin function eclocsh	 */

    register
		PANEL   *pl;    /* panel pointer for search      */

    register
		PANE    *pn;    /* pane pointer for search       */

    int     L_row_pl;		/* locator row relative to pnl	 */
    int     L_col_pl;		/* locator column in panel	 */

    int     pnadj;		/* adjustment for borders       */
    int     pnadj2;

    LC_PNL = (PANEL *) NULL;	/* clear the globals		 */
    LC_PANE = (PANE *) NULL;
    if (HAVE_LOC) {		/* if locator input encountered */
	for (pl = _toppl; pl != NULL; pl = pl->p_under) {
				/* step through the panel chain */
	    if (CLOC_ROW >= pl->orow &&/* check if locator within the  */
		    CLOC_COL >= pl->ocol &&
				/* - bounds of the panel        */
		    CLOC_ROW < pl->orow + pl->p_depth &&
		    CLOC_COL < pl->ocol + pl->p_width) {
		LC_PNL = pl;	/* set panel pointer		 */
		pn = pl->fpane;	/* init pane pointer		 */
		L_row_pl = CLOC_ROW - pl->orow;
				/* calculate locator row/col    */
		L_col_pl = CLOC_COL - pl->ocol;
				/* - relative to panel          */

		do {
		    pnadj = ((pn->bordr == Pbordry) ? 1 : 0);
		    pnadj2 = pnadj + pnadj;
				/* adjustment if borders on pnae */

				/* check if locator is in pane  */
		    if (L_row_pl >= pn->orow + pnadj &&
			    L_col_pl >= pn->ocol + pnadj &&
			    L_row_pl <= pn->orow + pn->v_depth - pnadj2 &&
			    L_col_pl <= pn->ocol + pn->v_width - pnadj2) {
			LC_PANE = pn;/* save pane containing locator */
			LC_ROW = L_row_pl - pn->orow - pnadj +
						pn->v_win->_winy;
			LC_COL = L_col_pl - pn->ocol - pnadj +
						pn->v_win->_winx;
				/* row/col = disp rel to panel       */
				/*   minus disp of view in panel */
				/*   plus disp of view on window */

			return OK;/* early exit if found pane	 */
		    }		/* end - if locator in pane	 */
		    pn = pn->nxtpn;
				/* step to next pane in panel	 */
		} while (pn != pl->fpane);
				/* end do while still in panel	 */
		break;		/* was in panel not in pane	 */
				/* - don't consider more panels */
				/* - because they're covered    */

	    }			/* end if locator in panel	 */

	}			/* end for loop on panels	 */
    }				/* end - if locator valid	 */

    return(ERR);		/* return to caller - not res.	 */
}				/* end function eclocrs 	 */
