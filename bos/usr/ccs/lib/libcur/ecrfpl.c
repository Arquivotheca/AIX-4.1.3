static char sccsid[] = "@(#)79	1.6  src/bos/usr/ccs/lib/libcur/ecrfpl.c, libcur, bos411, 9428A410j 6/16/90 01:39:25";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: ecrfpl, ecrfplp, ecuntw
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

int     ECRFCLR;		/* global integer/switch         */

/*
 * NAME:                ecrfpl
 *
 * FUNCTION:            ensure that all the data for the specified
 *                      panel is displayed on the terminal
 *
 * EXECUTION ENVIRONMENT:
 *
 *   PARAMETERS:        pnl - pointer to panel structure to refresh
 *                            if NULL the refresh will start with
 *                            the bottom most panel which is flagged
 *                            as having been modified.
 *
 *                      The global variable (integer) ECRFCLR must
 *                      be set to one of the following values:
 *
 *                            PLRFCLR - clear screen, refresh display
 *                                      from argument panel up
 *                                      for use after remove with arg
 *                                      of bottom panel in stack
 *                            PLRFNCL - do not clear, refresh display
 *                                      from argument panel up
 *                                      (default) for use if change
 *                                      to panel but not the set of
 *                                      panels.
 *                            PLRFSTD - move the updated panel to
 *                                      stdscr, do not refresh disp.
 *
 * RETURNED VALUES:     OK
 */

int     ecrfpl (pnl)			/* refresh panel function        */
	PANEL   *pnl;          		/* pointer to panel structure    */

{								/* begin mainline code		 */

#ifdef	PCWS
    if (PW) {                   /* if terminal has outboard
				   					workspaces 		 */
	for (cp = pnl->fpane;;) {	/* begin with first pane	 */
	    ecwrpn(cp);				/* write p-space to terminal	 */
	    cp = cp->nxtpn;			/* step to next pane in chain	 */
	    if ((cp == NULL) || (cp = pnl->fpane)) {
								/* if back to start of chain     */
		break;					/* exit when all panes processed */
	    }
	}							/* end - for all panes		 */
    }				/* end - if pcws type terminal	 */
    else {
#endif
	if (pnl == NULL) {	/* if null argument find changes */
	    for (pnl = _botpl;	/* start at bottom panel         */
		    pnl != NULL && pnl->plmodf == Pmodfn;
				/* while in active panels and no */
				/* - change found                */
		    pnl = pnl->p_over);
				/* stepping to next panel        */
	    if (pnl == NULL && ECRFCLR != PLRFCLR)
		return OK;	/* if no change needed return    */
	}

	if (ECRFCLR == PLRFCLR) {
				/* if request to clear           */

	    pnl = _botpl;	/* start refr from bottom        */
	    werase(stdscr);	/* clear the standard screen     */
	    touchwin(stdscr);	/* ensure it is all processed    */
	}

	while (pnl != NULL) {	/* until all needed panels proc  */
	    if (pnl->plobsc == Pobscn &&
				/* if panel is not covered and   */
		    ECRFCLR != PLRFCLR) {
				/* - clearing was not requested  */
		ecrfplp(pnl);	/* move data to panel p-space    */
		clearok(pnl->p_win, FALSE);
				/* do not clear screen           */
		wrefresh(pnl->p_win);/* update the display            */
		do {
		    pnl = pnl->p_over;/* look up the panel stack       */
		} while (pnl != NULL &&
				/* until top of stack or         */
			pnl->plmodf == Pmodfn);
				/* - a changed panel is found    */
	    }			/* end refr top w/o clear req    */
	    else {
		for (pnl = ((ECRFCLR == PLRFCLR) ? _botpl : pnl);
				/* step through panels to top    */
			pnl != NULL;/* - link from top will be null  */
			pnl = pnl->p_over) {
				/* - follow links to top         */
		    ecrfplp(pnl);/* update p-space for the panel  */
		    overwrite(pnl->p_win, stdscr);
				/* move data to stdscr           */
		    ecuntw(pnl->p_win);
				/* remove modification flags     */
		    if (ECRFCLR == PLRFSTD)
			break;	/* quit now if move to stdscr req */
		}
		if (ECRFCLR != PLRFSTD) {
				/* if not move only request do
				   display refresh             */
		    clearok(stdscr, FALSE);
				/* prevent clearing screen       */
		    wrefresh(stdscr);/* refresh the display           */
		}

	    }			/* end else refr not top panel   */
				/* - or clear requested          */

	}			/* end - while pnl not null      */

	eclocsh();		/* ensure locator is visible     */

	ECRFCLR = PLRFNCL;	/* reset clear required flag     */

#ifdef	PCWS
    }
#endif

    return(OK);			/* return to caller              */
}				/* end of mainline ecrfpl.	 */

/*
 * NAME:                ecrfplp
 *
 * FUNCTION:            ensure that the data in the p-space for a pnl
 *                      reflect the current data in all panes of pnl
 *
 * EXECUTION ENVIRONMENT:
 *
 *   PARAMETERS:        pnl - pointer to panel structure to refresh
 *
 *   INITIAL CONDITIONS
 *
 *   FINAL CONDITIONS:
 *     NORMAL:
 *
 *     ABNORMAL:
 *
 * EXTERNAL REFERENCES:
 *
 * RETURNED VALUES:     OK
 */

int     ecrfplp (pnl)		/* refresh panel p-space	 */
	PANEL   *pnl;           /* argument is panel pointer     */


{

    PANE    *cp;                /* work pane pointer             */


    if (pnl->apane != NULL) {	/* if panel has active pane	 */
	ecpspn(pnl->apane);	/* position pane to show cursor  */
    }

    for (cp = pnl->fpane;;) {	/* step through panes in panel	 */
	ecrefpan(cp->v_win);	/* ensure refresh deltas correct */
	overwrite(cp->v_win, pnl->p_win);
				/* move data to panel pspace	 */
	ecuntw(cp->v_win);	/* remove modification flags     */
	cp = cp->nxtpn;		/* step to next pane in chain	 */
	if ((cp == NULL) || (cp == pnl->fpane)) {
				/* if back to starting pane	 */
	    break;		/* exit from loop		 */
	}
    }				/* end - loop thru all panes	 */
    touchwin(pnl->p_win);	/* touch to assure borders refr  */
    pnl->plmodf = Pmodfn;	/* set flag no modificaton pendin */

}				/* end ecrfplp function 	 */

/*
 * NAME:                ecuntw
 *
 * FUNCTION:            set change flags for all rows of argument
 *                      windot to nochange
 *
 * EXECUTION ENVIRONMENT:
 *
 *   PARAMETERS:        win - window structure to modify
 *
 *   INITIAL CONDITIONS
 *
 *   FINAL CONDITIONS:
 *     NORMAL:
 *
 *     ABNORMAL:
 *
 * EXTERNAL REFERENCES:
 *
 * RETURNED VALUES:     OK
 */

static
int     ecuntw (win)		/* untouch the window            */
	WINDOW  *win;           /* argument is window pointer    */


{

    register
    int     ix;			/* row number index             */

    register
    short  *dp;			/* pointer to delta flag        */

    for (ix = win->_maxy - 1,	/* initialize index to row cnt  */
	    dp = &(win->_firstch[ix]);/* init pointer to last change  */
				/* flag for the window          */
	    ix >= 0;		/* loop backward to first row   */
	    ix--, dp--) {	/* decrement index and pointer  */
	*dp = _NOCHANGE;	/* set the first changed flag   */
    }				/* for each row to no change    */

    return;			/* return to caller             */

}				/* end function ecuntw          */
