static char sccsid[] = "@(#)66	1.7  src/bos/usr/ccs/lib/libcur/ecrefpan.c, libcur, bos411, 9428A410j 6/16/90 01:39:21";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: ecrefpan
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

/*
 * NAME:                ecrefpan
 *
 * FUNCTION:            Fill in the pane update vectors with values
 *                      for the corresponding underlying pspace
 *
 * EXECUTION ENVIRONMENT:
 *
 *   PARAMETERS:        view - pointer to window structure for a
 *                             viewport.
 *
 *   INITIAL CONDITIONS View must point to a viewport window struct.
 *
 *   FINAL CONDITIONS:
 *     NORMAL:          update arrays in view reflect changes made
 *                      -- in underlying window, update arrays for
 *                      -- the pspace modified as if data in view is
 *                      -- not a change (i.e. has been shown)
 *
 *     ABNORMAL:        ERR returned if not a view structure
 *
 * EXTERNAL REFERENCES: none
 *
 * RETURNED VALUES:     TRUE if any change found for this view.
 *                      FALSE if nochanges found for the view.
 */

char    ecrefpan (vwin)
	WINDOW  *vwin;          /* pointer to view for pane     */

{

    WINDOW  *pwin;              /* pointer to window for pspace */

    int     pxmax;		/* max x of view on pspace      */
    int     pxmin;		/* min x of view on pspace      */
    int     vy;			/* curr row in view             */
    int     py;			/* curr row in pspace           */
    char    chgfnd = FALSE;	/* any change found yet         */

    pwin = vwin->_view;		/* get pointer to pspace wind   */

    pxmin = vwin->_winx;	/* get minimum col of view in ps */
    pxmax = pxmin + vwin->_maxx - 1;/* calc last col of view in ps  */

    for (vy = 0; vy < vwin->_maxy; vy++) {
				/* for each row of view         */
	py = vy + vwin->_winy;	/* calculate corres. row in ps  */

	if (pwin->_firstch[py] != _NOCHANGE) {
				/* if any change in row         */
	    if (pwin->_firstch[py] <= pxmax &&
				/* if the change overlaps with  */
		    pwin->_lastch[py] >= pxmin) {
				/* -- the viewport area         */
		chgfnd = TRUE;	/* set flag found change        */

		if (vwin->_firstch[vy] == _NOCHANGE) {
				/* if new change for view line  */
		    vwin->_lastch[vy] = -1;
				/* set end value so any valid   */
		}		/* value causes it to be reset  */

		if (pwin->_firstch[py] < pxmin) {
				/* if change begins left of view */
		    vwin->_firstch[vy] = 0;
				/* left of view change is start */
		    if (pwin->_lastch[py] > pxmax) {
				/* if end of ch is right of view */
			vwin->_lastch[vy] = vwin->_maxx - 1;
				/* end of view change is right  */
				/* -- of view, no change can be */
				/* -- made to pspace change ptrs */
		    }
		    else {	/* start to left  end within    */
			if (vwin->_lastch[vy] < pwin->_lastch[py] - pxmin)
			    vwin->_lastch[vy] = pwin->_lastch[py] - pxmin;
				/* end of change translated to  */
				/* -- view coordinates          */
			pwin->_lastch[py] = pxmin - 1;
				/* change now ends at right of  */
				/* -- view                      */
		    }
		}
		else {          /* change starts at or right of  --
				   view left edge            */
		    if (vwin->_firstch[vy] > pwin->_firstch[py] - pxmin ||
			    vwin->_firstch[vy] == _NOCHANGE)
			vwin->_firstch[vy] = pwin->_firstch[py] - pxmin;
				/* translate start to view coord */
		    if (pwin->_lastch[py] > pxmax) {
				/* if change extends to right of  --
				   the view                  */
			vwin->_lastch[vy] = vwin->_maxx - 1;
				/* end of change is edge of view */
			pwin->_firstch[py] = pxmax + 1;
				/* update pspace chg to start at */
				/* -- view right edge           */
		    }
		    else {	/* chg starts and ends in view  */
			if (vwin->_lastch[vy] < pwin->_lastch[py] - pxmin)
			    vwin->_lastch[vy] = pwin->_lastch[py] - pxmin;
			pwin->_firstch[py] = _NOCHANGE;
				/* all of change shown now      */
		    }
		}
	    }
	}
    }
    return(chgfnd);		/* return flag for any changes  */
				/* -- for the view              */
}				/* end of function              */
