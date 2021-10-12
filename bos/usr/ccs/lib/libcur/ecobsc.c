static char sccsid[] = "@(#)33	1.6  src/bos/usr/ccs/lib/libcur/ecobsc.c, libcur, bos411, 9428A410j 6/16/90 01:39:02";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: ecobsc
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
 * NAME:                ecobsc
 *
 * FUNCTION:            Set the flag indicating whether a panel or
 *                      pane is obscured by other panel on the screen
 *
 * EXECUTION ENVIRONMENT:
 *
 *   PARAMETERS:        pl - pointer to topmost panel to consider
 *
 *   INITIAL CONDITIONS Panel pl must be valid and in chain of active
 *                      panels.
 *
 *   FINAL CONDITIONS:  obsc flag set/reset for panels and panes
 *     NORMAL:
 *
 *     ABNORMAL:
 *
 * EXTERNAL REFERENCES: none
 *
 * RETURNED VALUES:     OK - always
 */

int     ecobsc (pl)		/* begin function definition    */
	PANEL   *pl;            /* pointer to panel structure   */

{

    PANEL   *px, *py;           /* work panel structure ptrs    */

    PANE    *pn;                /* Work Pane structure pointer  */

    WINDOW  *pxw, *pyw, *pnw;   /* window pointers for px,py pn */

    for (px = pl;		/* work down list of panels     */
	    px != NULL;		/* stop at bottom of chain      */
	    px = px->p_under) {	/* follow chain                 */
	px->plobsc = Pobscn;	/* reset obscured flag in panel */

	pxw = px->p_win;	/* save window pointer          */

	pn = px->fpane;		/* initialize pointer to 1st pn */
	do {			/* repeat processing on panes   */
	    pn->pnobsc = Pobscn;/* clear obscured flag          */
	    pn = pn->nxtpn;	/* step to next pane            */
	} while (pn != px->fpane);/* until back to top            */

	for (py = px->p_over;	/* search up the panels         */
		py != NULL;	/* to top of panel stack        */
		py = py->p_over) {
	    pyw = py->p_win;	/* save py window pointer       */

	    if (pxw->_begx < pyw->_begx + pyw->_maxx &&
		    pxw->_begy < pyw->_begy + pyw->_maxy &&
		    pxw->_begx + pxw->_maxx > pyw->_begx &&
		    pxw->_begy + pxw->_maxy > pyw->_begy) {
				/* If px overlaps with py       */
		px->plobsc = Pobscy;/* flag panel px as overlapped  */

		pn = px->fpane;	/* initialize to first px pane  */
		do {
		    pnw = pn->v_win;/* get pane vie pointer         */
		    if (pnw->_begx < pyw->_begx + pyw->_maxx &&
			    pnw->_begy < pyw->_begy + pyw->_maxy &&
			    pnw->_begx + pnw->_maxx > pyw->_begx &&
			    pnw->_begy + pnw->_maxy > pyw->_begy) {
				/* if pane overlapped by py     */
			pn->pnobsc = Pobscy;
				/* flag pane as obscured        */
		    }
		    pn = pn->nxtpn;/* step to next pane            */
		} while (pn != px->fpane);
				/* end loop thru px panes       */
	    }			/* end overlap found in px/py   */
	}			/* end search up for overlap    */
    }				/* end loop to bottom of panels */

    return OK;			/* return to caller - OK        */

}				/* end function ecobsc          */
