static char sccsid[] = "@(#)23	1.6  src/bos/usr/ccs/lib/libcur/ecscpn.c, libcur, bos411, 9428A410j 6/16/90 01:39:43";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: ecscpn
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
 * NAME:                ecscpn
 *
 * FUNCTION:            Scroll the specified pane and any linked
 *                      panes to the position indicated.
 *
 * EXECUTION ENVIRONMENT:
 *
 *   PARAMETERS:        pane - pointer to PANE to be positioned
 *                      hscr - distance to move view origin horiz.
 *                      vscr - distance to move view origin vert.
 *
 *   INITIAL CONDITIONS
 *
 *   FINAL CONDITIONS:
 *     NORMAL:
 *
 *     ABNORMAL:
 *
 * RETURNED VALUES:     OK
 */

ecscpn(pn, dy, dx)		/* function invocation - scroll */
PANE    *pn;                    /* arg 1 - PANE pointer          */
int     dy;			/* arg 2 - scroll vert delta	 */
int     dx;			/* arg 3 - scroll horz delta	 */

{				/* begin function ecscpn	 */

    PANE    *cp;                /* local pane pointer            */
    int     scd;		/* delta scroll for other panes */

#ifdef	PCWS

    if (!PS) {                  /* only need to work if no
				   outboard p-space		 */
#endif

	vscroll(pn->v_win, dy, dx);/* scroll the requested pane	 */
	ecpnmodf(pn);		/* flag the pane as modified    */

				/* follow horizontal scroll lnks */
	for (cp = pn->hscr;	/* follow the chain from arg	 */
		cp != NULL && cp != pn;
				/* until null or back to start	 */
		cp = cp->hscr) {/* follow each in chain 	 */
	    if ((scd = (pn->v_win)->_winx - (cp->v_win)->_winx) != 0) {
				/* if we must scroll the next pn */
		ecpnmodf(cp);	/* flag the pane as modified &  */
		vscroll(cp->v_win, 0, scd);
				/* request that scroll		 */
	    }
	}


				/* follow vertical scroll links */
	for (cp = pn->vscr;	/* follow the chain from arg	 */
		cp != NULL && cp != pn;
				/* until null or back to start	 */
		cp = cp->vscr) {/* follow each in chain 	 */
	    if ((scd = (pn->v_win)->_winy - (cp->v_win)->_winy) != 0) {
				/* if we must scroll the next pn */
		ecpnmodf(cp);	/* flag the pane as modified &  */
		vscroll(cp->v_win, scd, 0);
				/* request that scroll          */
	    }
	}

#ifdef	PCWS
    }
#endif

    return OK;
}				/* end function ecscpn		 */

