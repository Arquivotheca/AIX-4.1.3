static char sccsid[] = "@(#)12	1.6  src/bos/usr/ccs/lib/libcur/ecrmpl.c, libcur, bos411, 9428A410j 6/16/90 01:39:39";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: ecrmpl
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
 * NAME:                ecrmpl
 *
 * FUNCTION:            Remove a panel from the display and refresh
 *                      display of any panels which had been obscured
 *
 * EXECUTION ENVIRONMENT:
 *
 *   PARAMETERS:        pl - pointer to panel to be removed from disp
 *
 *   INITIAL CONDITIONS Panel should be on list of displayed panels
 *
 *   FINAL CONDITIONS:
 *     NORMAL:          Panel will be removed and display refreshed
 *
 *     ABNORMAL:        n/a
 *
 * EXTERNAL REFERENCES:
 *
 * RETURNED VALUES:     OK - always
 */

int     ecrmpl (pl)		/* remove a panel               */
	PANEL   *pl;            /* pointer to panel to remove   */

{				/* begin function ecrmpl        */

    register
		PANEL   *plo;   /* pointer to panel over arg    */
    register
		PANEL   *plu;   /* pointer to panel under arg   */
    extern
	    PANEL   *_titlpl;   /* title panel pointer          */

#ifdef PCWS
    if (!PS) {			/* if no external p-space       */
#endif


	plo = pl->p_over;	/* extract pointer up           */
	plu = pl->p_under;	/* extract pointer down         */

	if (plu != NULL) {	/* if there is a panel under    */
	    plu->p_over = plo;	/* change its pointer up stack  */
	    if (plo && plo == _titlpl) {
				/* if the panel over this is    */
				/* - is the title panel         */
		if (plu->apane != NULL) {
				/* and new top pnl has active pn */
		    ecactp(plu->apane, TRUE);
				/* show that as active pane     */
		}
	    }
	}

	if (plo != NULL) {	/* if there is a panel over     */
	    plo->p_under = plu;	/* change its pointer down stack */
	}

	if (pl == _toppl) {	/* if arg is top panel          */
	    _toppl = plu;	/* correct that pointer         */
	    if (_toppl != NULL && _toppl->apane != NULL)
		ecactp(_toppl->apane, TRUE);
				/* if new top has active pane   */
				/* - make that pane active      */
	}

	if (pl == _botpl) {	/* if arg is bottom panel       */
	    _botpl = plo;	/* correct bottom pointer       */
	}

	if (plo != NULL) {	/* if there was a pane above    */
	    ecobsc(plo);	/* refresh overlap flags from it */
	}
	else {
	    if (_toppl != NULL)	/* else if new top              */
		ecobsc(_toppl);	/* refresh overlap from top     */
	}
	pl->p_under = pl->p_over = NULL;
				/* clear links in panel         */

	ECRFCLR = PLRFCLR;	/* indicate next refresh must be */
				/* - to cleared screen          */

#ifdef PCWS
    }
    else {
	termdfid = pl->dfid;	/* set dfid in parm for deact   */
	write termdactd		/* write the deactivate to term */
    }
#endif


    return OK;
}				/* end function ecrmpl          */
