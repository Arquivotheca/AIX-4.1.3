static char sccsid[] = "@(#)34	1.7  src/bos/usr/ccs/lib/libcur/ecshpl.c, libcur, bos411, 9428A410j 6/16/90 01:39:47";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: ecshpl, ectitl
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

/*      global/external data areas that form the root of the panel chain*/

PANEL   *_toppl;                /* top panel on display         */
PANEL   *_botpl;                /* bottom panel on display      */
PANEL   *_titlpl;               /* Title panel                  */

/*
 * NAME:                ecshpl
 *
 * FUNCTION =           Show the specified panel at the top of the
 *                      panels on the display
 *
 * EXECUTION ENVIRONMENT:
 *
 *   PARAMETERS:        pnl - pointer to panel which is to be shown
 *
 *   INITIAL CONDITIONS panel indicated by pnl must be properly
 *                      defined with all auxiliary structures in
 *                      valid statesglobal variables indicating panel
 *                      sequence chain must be properly initialized
 *
 *   FINAL CONDITIONS:
 *     NORMAL:          chain of panels will be such that the
 *                      indicated panel is on the 'top' of the chain.
 *                      The display will be refreshed with the
 *                      contents of the panel.
 *     ABNORMAL:        return code is 0, other status undefined.
 *
 * EXTERNAL REFERENCES: ecactp, ecrfpl
 *
 * RETURNED VALUES:     returns 1 if no error during show, else zero
 */

ecshpl(pnl)			/* function - show panel        */
PANEL   *pnl;                   /* argument is ptr to panel      */

{				/* begin mainline ecshpl	 */

    PANE    *ap;                /* hold active pane pointer     */
    PANEL   *wpl;               /* real top panel pointer       */

#ifdef	PCWS
    if (PC) {			/* if terminal has external ws	 */
	build show df block	/* build block for show DF	 */
	    write show df to terminal/* write that block to the term */

    }
    else {			/* if no external p-spaces	 */
#endif
	if (pnl != _toppl) {	/* if argument is not top panel */
	    if (_toppl != NULL && pnl != _titlpl) {
				/* if top panel is not null     */
				/* - and arg is not title       */
		wpl = (_titlpl != NULL ? _titlpl->p_under : _toppl);

		if (wpl != NULL) {
		    if (wpl->apane != NULL) {
				/* if current top has active pn */
			ap = wpl->apane;
				/* get pointer to active pane   */
			ecactp(ap, FALSE);
				/* make that pane not active    */
			wpl->apane = ap;
				/* set that for reactivate later */
		    }
		}
	    }

	    if (pnl->p_under != NULL) {
				/* remove argument from panel, if
				   panel under argument panel */
		(pnl->p_under)->p_over = pnl->p_over;
	    }
	    if (pnl == _botpl) {/* if argument is current bottom */
		_botpl = pnl->p_over;
	    }
	    if (pnl->p_over != NULL) {/* if arg has panel above it    */
		(pnl->p_over)->p_under = pnl->p_under;
	    }

	    pnl->p_over = pnl->p_under = NULL;
				/* clear links in argument       */

				/* add argument to top of links */
	    if (_toppl != NULL) {/* if there is current top pnl  */
		_toppl->p_over = pnl;/* link current top to argument */
		pnl->p_under = _toppl;/* and argument to that panel   */
	    }
	    if (_botpl == NULL) {/* if no current bottom pnl     */
		_botpl = pnl;	/* make current bottom          */
	    }


	    _toppl = pnl;	/* set argument as new top panel */

	    if (pnl->apane != NULL) {/* if panel has active pane     */
		ecactp(pnl->apane, TRUE);
				/* make that pane active        */
	    }
	    pnl->plmodf = Pmodfy;/* flag panel as modified       */
	    ecobsc(pnl);	/* update obscured flags        */
	}			/* end - if not top panel	 */


	if (_titlpl != NULL && pnl != _titlpl) {
				/* if title and not argument    */
	    ecshpl(_titlpl);	/* force title to top           */
	}

#ifdef	PCWS
    }				/* end - else no external ps	 */
#endif
    return OK;			/* return to caller no error	 */
}				/* end function ecshpl		 */

/*
 * NAME:                ectitl
 *
 * FUNCTION:            Create or updated the title for the display
 *
 * EXECUTION ENVIRONMENT:
 *
 *   PARAMETERS:        ts - pointer to title string
 *                      rw - row on which to show title
 *                      cl - column to begin showing title
 *
 *   INITIAL CONDITIONS ts must point to a null terminated character
 *                      string which does not contain any control
 *                      characters
 *
 *   FINAL CONDITIONS:
 *     NORMAL:          Title will be presented on the display
 *
 *     ABNORMAL:        Old title will be removed and nothing shown
 *
 * EXTERNAL REFERENCES: ecbpls, ecbpns, ecdvpl, ecdfpl, ecshpl,
 *                      ecdspl, ecrfpl
 *
 * RETURNED VALUES:     OK - successful
 *                      ERR - any error during process (storage)
 */

int     ectitl (tl, rw, cl)
char   *tl;			/* pointer to title string    */
int     rw;			/* Row to show title on       */
int     cl;			/* column to show title       */

{				/* begin function	      */

    PANE    *ttlpn;             /* pointer to new pane        */
    int     tllen;		/* title length               */

				/* executable code           */

    if (_titlpl != NULL) {	/* if title panel exists now    */
	ecdspl(_titlpl);	/* remove that title panel      */
	_titlpl = NULL;
    }

    if (tl != NULL) {						/* if new title is not null     */
											/* NLS supp-use display length  */
		tllen = NLstrdlen(tl);				/* get length of title      */
		if (tllen != 0) {					/* if length is not null    */

	    	ttlpn = ecbpns(1,				/* build pane - one row     */
		    	tllen,						/* width - title width      */
		  	  	NULL,						/* no neighbor pane         */
		    	NULL,						/* no divisions of this pane*/
		    	Pdivtyv,					/* divide along vertical    */
		    	NULL,						/* size as part of parent   */
		    	Pdivszf,					/* - floating size          */
		    	Pbordrn,					/* no border                */
		    	NULL,						/* no scroll link horizonatl*/
		    	NULL);						/* No scroll link vertical  */

	    	_titlpl = ecbpls(1,				/* build panel - one row    */
		    	(tllen > COLS ? COLS : tllen),
											/* width- min of title and cols*/

		    	((rw >= LINES || rw < 0) ? 1 : rw),
											/* row specified or 1 if bad */

		    	((cl >= COLS || cl < 0) ? (COLS - tllen) / 2 : cl),
											/* col spec or center title */

		    	NULL,						/* title string is null     */
		    	Pdivtyh,					/* divide along horizonatl */
		    	Pbordrn,					/* no borders            */
		    	ttlpn);						/* pane pointer          */

	    	ecdvpl(_titlpl);				/* create division           */
	    	ecdfpl(_titlpl, FALSE);			/* create presentation space */
	    	wcolorout(ttlpn->w_win, Bxa);
											/* set color to match inactive */
											/* - border color */
	    	mvpaddstr(ttlpn, 0, 0, tl);		/* put title in pane */
	    	ecshpl(_titlpl);				/* show the title */

		}							 		/* end if length not zero */
    }										/* end if title ptr not null */
    return(OK);								/* return to caller          */
}											/* end of function	      */
