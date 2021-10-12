static char sccsid[] = "@(#)90	1.7  src/bos/usr/ccs/lib/libcur/ecrfpn.c, libcur, bos411, 9428A410j 6/16/90 01:39:30";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: ecrfpn
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

static
char    rfpnact = FALSE;	/* switch - TRUE when this      */
				/* routine is recursively       */
				/* invoking itself              */

/*
 * NAME:                ecrfpn
 *
 * FUNCTION:            Assure that any changes made to the specified
 *                      pane are visible on the display.
 *
 * EXECUTION ENVIRONMENT:
 *
 *   PARAMETERS:        pn - pointer to pane structure
 *
 *   INITIAL CONDITIONS Pane pn must be a well defined pane in the
 *                      structures.
 *
 *   FINAL CONDITIONS:  pane is updated on the display.
 *     NORMAL:
 *
 *     ABNORMAL:
 *
 * EXTERNAL REFERENCES: ecrefpan, ecrfpl, wrefresh
 *
 * RETURNED VALUES:     OK - always
 */

int     ecrfpn (pn)		/* begin function definition    */
	PANE    *pn;            /* pointer to pane structure    */

{

    PANE    *pnr;               /* pane pointer when recursing  */

#ifdef PCWS

    if (PW) {			/* if terminal has external ws  */
	writeps(pn);		/* go write changed lines of ps */
    }

    else {			/* terminal has no external ws  */

#endif

	if ((pn->hpanl)->apane == pn) {/* if argument pane is active   */
	    ecpspn(pn);		/* ensure cursor is showing     */
	}

	if (pn->pnobsc == Pobscn) {/* if pane is not obscured      */

	    if (!rfpnact) {	/* if not recursively active    */
		rfpnact = TRUE;	/* set flag - recursively active */

		for (pnr = pn->hscr;/* refresh all panes linked     */
			pnr != pn && pnr != NULL;
				/* - for horizontal scroll if   */
			pnr = pnr->hscr) {
				/* - any such panes             */
		    ecrfpn(pnr);/* refresh the linked pane      */
		}

		for (pnr = pn->vscr;/* refresh all panes linked     */
			pnr != pn && pnr != NULL;
				/* - for vertical scroll if     */
			pnr = pnr->vscr) {
				/* - any such panes             */
		    ecrfpn(pnr);/* refresh the linked pane      */
		}

		rfpnact = FALSE;/* done with recursive calls    */
	    }
	    ecrefpan(pn->v_win);/* go update change vectors     */
	    wrefresh(pn->v_win);/* go refresh the display       */
	}
	else {
	    ecrfpl(pn->hpanl);	/* if needed refresh the panel  */
	    wmove(stdscr,
		    pn->v_win->_cury + pn->v_win->_begy,
		    pn->v_win->_curx + pn->v_win->_begx);
				/* set cursor position          */
	    clearok(stdscr, FALSE);/* do not allow clear screen    */
	    wrefresh(stdscr);	/* go move cursor as needed     */
	}

#ifdef PCWS
    }
#endif

    return OK;
}				/* end function ecrfpn          */
