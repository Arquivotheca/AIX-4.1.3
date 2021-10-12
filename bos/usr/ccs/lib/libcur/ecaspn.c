static char sccsid[] = "@(#)00	1.5  src/bos/usr/ccs/lib/libcur/ecaspn.c, libcur, bos411, 9428A410j 6/16/90 01:37:57";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: ecaspn
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
 * NAME:                ecaspn
 *
 * FUNCTION:            Specify which of the p-spaces for a pane is
 *                      to be the active p-space.
 *
 * EXECUTION ENVIRONMENT:
 *
 *   PARAMETERS:        pane - pointer to PANE structure
 *                      win - pointer to win to be the active win
 *
 *   INITIAL CONDITIONS win must appear in one of the aux p-space
 *                      structures for the argument pane
 *
 *   FINAL CONDITIONS:
 *     NORMAL:          OK returned, win is active p-space for pane
 *                      Display frame for panel updated if needed.
 *
 *     ABNORMAL:        ERR - returned, p-space for pane unchanged
 *
 * EXTERNAL REFERENCES: calloc
 *
 * RETURNED VALUES:     OK - no error
 *                      ERR - win not associated with pane
 */

int     ecaspn (pn, pw)		/* begin function		 */
register
	    PANE    *pn;        /* pointer to pane being accessed */

register
	    WINDOW  *pw;        /* pointer to p-space (window)   */

{				/* begin function code		 */

    register
		PANEPS  *pa;    /* pointer to aux structure      */

    register
    int     i;			/* index to data vectors         */
    WINDOW  *vp;                /* view data structure           */

    if (pn->w_win != pw) {	/* if changing the pspace        */
	for (pa = pn->exps;	/* follow chain from pane        */
		pa != NULL;	/* until end of chain            */
		pa = pa->extnxt) {/* follow to next in chain       */
	    if (pa->extps == pw)
		break;		/* exit if correct entry found   */
	}
	if (pa == NULL)
	    return ERR;		/* if end chain found - error    */

	pn->w_win = pa->extps;	/* set active p-space pointer    */
	pn->pnvsid = pa->expvsid;/* set v.s. id in pane           */
    }

#ifdef	PCWS

    if (PS) {			/* if terminal has p-spaces	 */
	dfcnt = 0;		/* init display frame struct	 */
	pl = pn->hpanl->fpane;	/* init pointer to first pane	 */
	do {			/* repeat until end of chain	 */
	    ds.pnt++ = pl->pnvsid;/* add vsid to display frame	 */
	    pl = pl->nxtpn;	/* step to next pane		 */
	} while (pl != pn->hpanl->fpane);
				/* loop to end of panes 	 */
	write df		/* write display frame to term.  */
    }

    else			/* for local or ASCII terminal   */
# endif
    {

	for (i = 0, vp = pn->v_win;/* index through the view        */
		i < vp->_maxy;	/* step through rows             */
		i++) {
	    vp->_y[i] = pw->_y[i];/* set view to point to new data */
	    vp->_a[i] = pw->_a[i];/* - and new attribute codes     */
	}
	vp->_view = pw;		/* set pointer to p-space        */

	ecpspn(pn);		/* assure that cursor is visible */
	touchwin(vp);		/* assure complete paint         */
	ecpnmodf(pn);		/* flag pane as modified         */
    }

    return OK;			/* return - no error              */
}				/* end code ecaspn		 */
