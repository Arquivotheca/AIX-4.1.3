static char sccsid[] = "@(#)55	1.6  src/bos/usr/ccs/lib/libcur/ecpspn.c, libcur, bos411, 9428A410j 6/16/90 01:39:16";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: ecpspn
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
 * NAME:                ecpspn
 *
 * FUNCTION =           Ensure that the view port for a pane is
 *                      positioned such that the cursor is visible
 *
 * EXECUTION ENVIRONMENT:
 *
 *   PARAMETERS:        pane - pointer to PANE to be positioned
 *
 *   INITIAL CONDITIONS:
 *
 *   FINAL CONDITIONS:
 *     NORMAL:
 *
 *     ABNORMAL:
 *
 * EXTERNAL REFERENCES: malloc
 *
 * RETURNED VALUES
 */

int     ecpspn (pane)		/* define function		 */
	PANE    *pane;          /* argument is pointer to pane   */
{

				/* local declarations           */
    WINDOW  *vw;                /* pointer to view for pane      */
    WINDOW  *pw;                /* pointer to p-space structure */

    int     cr;			/* row containing cursor in pw	 */
    int     cc;			/* column in pw with cursor	 */

    int     hs = 0;		/* horizontal scroll distance	 */
    int     vs = 0;		/* vertical scroll distance	 */

#ifdef	PCWS

    if (!PW) {			/* if not dealing with outboard */
				/* -- workspaces             */
#endif

	pw = pane->w_win;	/* get pointer to p-space struct */
	vw = pane->v_win;	/* also pointer to view struct	 */

	cr = pw->_cury;		/* get current cursor location	 */
	cc = pw->_curx;

	if (cc < vw->_winx) {	/* if cursor to left of view	 */
	    hs = cc - vw->_winx;/* set scroll to left distance	 */
	}
	else
	    if (cc >= vw->_winx + vw->_maxx) {
				/* check if cursor right of view */
		hs = cc - (vw->_winx + vw->_maxx - 1);
				/* calculate scroll right dist. */
	    }

	if (cr < vw->_winy) {	/* if cursor to above view	 */
	    vs = cr - vw->_winy;/* set scroll up distance	 */
	}
	else
	    if (cr >= vw->_winy + vw->_maxy) {
				/* check if cursor below view	 */
		vs = cr - (vw->_winy + vw->_maxy - 1);
				/* calculate scroll down dist.       */
	    }

	if (vs + hs) {		/* if any scroll needed 	 */
	    ecscpn(pane, vs, hs);/* call scroll routine          */
	}

	vw->_cury = pw->_cury - vw->_winy;
				/* calculate new cursor posn	 */
	vw->_curx = pw->_curx - vw->_winx;
				/* - for the viewport		 */

#ifdef	PCWS
    }				/* end if no outboard workspace */
#endif

    return(vs + hs);		/* return zero/false if no move */

}				/* end - function ecpspn	 */
