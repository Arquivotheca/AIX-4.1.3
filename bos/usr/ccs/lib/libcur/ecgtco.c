static char sccsid[] = "@(#)00	1.6  src/bos/usr/ccs/lib/libcur/ecgtco.c, libcur, bos411, 9428A410j 6/16/90 01:38:43";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: ecgtco, ecstco
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

/*
 * NAME:                ecgtco
 *
 * FUNCTION:            Return the pane relative coordinate for
 *                      the given presentation space coordinate
 *
 * EXECUTION ENVIRONMENT:
 *
 *   PARAMETERS:        pane - pointer to PANE to be positioned
 *                      pnrw - row in p-space to be resolved
 *                      pnco - col in p-space to be resolved
 *                      relrw- pointer to integer to receive row disp
 *                      relco- pointer to integer to receive col disp
 *
 *   INITIAL CONDITIONS pane valid pane pointer
 *
 *   FINAL CONDITIONS:
 *     NORMAL:          Integers pointed to by final args are updated
 *                      with relative value for row and column in
 *                      the presented view of the p-space
 *
 *
 *     ABNORMAL:        no change to arguments
 *
 * EXTERNAL REFERENCES: none
 *
 * RETURNED VALUES:     OK - position ok, ERR position outisde p-sp
 */

ecgtco(pn, py, px, ry, rx)	/* function invocation          */
PANE    *pn;                    /* arg 1 - PANE pointer          */
int     py;			/* arg 2 - row coord in p-space */
int     px;			/* arg 3 - col coord in p-space */
int    *ry;			/* arg 4 - Ptr to view row disp */
int    *rx;			/* arg 5 - ptr to view col disp */

{				/* begin function ecgtco        */

    register
		WINDOW  *pw;    /* local window pointer         */

    pw = pn->w_win;		/* get pointer to p-space str   */

    if (py < 0 ||		/* check if coordinate is valid */
	    py >= pw->_maxy ||
	    px < 0 ||
	    px >= pw->_maxx) {
	return(ERR);		/* if not valid return error    */
    }

    pw = pn->v_win;		/* get pointer to view          */

    *ry = py - pw->_winy;	/* calculate disp of coord rel  */
    *rx = px - pw->_winx;	/* - to the view                */

    return(OK);			/* return to caller - no error  */
}

/*
 * NAME:                ecstco
 *
 * FUNCTION:            Set pane position to place a p-space coord
 *                      at a given view relative location.
 *
 * EXECUTION ENVIRONMENT:
 *
 *   PARAMETERS:        pane - pointer to PANE to be positioned
 *                      pnrw - row in p-space to be resolved
 *                      pnco - col in p-space to be resolved
 *                      relrw- relative row in view for pnrw
 *                      relco- relative col in view for pnco
 *
 *   INITIAL CONDITIONS pane valid pane pointer
 *
 *   FINAL CONDITIONS:
 *     NORMAL:          The view for pane will be positioned to place
 *                      pnrw/pnco as close as possible to relrw/relco
 *
 *     ABNORMAL:        no change to arguments
 *
 * EXTERNAL REFERENCES: ecscpn
 *
 * RETURNED VALUES:     OK - always
 */

ecstco(pn, py, px, ry, rx)	/* function invocation          */
PANE   *pn;                     /* arg 1 - PANE pointer          */
int     py;			/* arg 2 - row coord in p-space */
int     px;			/* arg 3 - col coord in p-space */
int     ry;			/* arg 4 - Ptr to view row disp */
int     rx;			/* arg 5 - ptr to view col disp */

{				/* begin function ecstco        */

    return(
	    ecscpn(pn,		/* scroll the pane              */
		py - pn->v_win->_winy - ry,
				/* calculate disp from current  */
		px - pn->v_win->_winx - rx)
				/* - location to requested loc  */
	);
}
				/* end function ecstco          */
