static char sccsid[] = "@(#)55	1.6  src/bos/usr/ccs/lib/libcur/ecdppn.c, libcur, bos411, 9428A410j 6/16/90 01:38:20";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: ecdppn
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
 * NAME:                ecdppn
 *
 * FUNCTION:            change the p-space associated with a panel
 *                      by dropping one and adding another, link to
 *                      the view as needed.
 *
 * EXECUTION ENVIRONMENT:
 *
 *   PARAMETERS:        pn - pointer to pane to be modified.
 *                      op - pointer to old p-space to be dropped for
 *                           this pane.
 *                      np - pointer to new p-space to be added to
 *                           this pane.
 *
 *                      either op or np may be null, if op is null
 *                      then np will be added as a valid p-space for
 *                      the pane. If np is null op will be dropped
 *                      as a vaild p-space for this pane.
 *
 *   INITIAL CONDITIONS pn must be a valid pane
 *
 *   FINAL CONDITIONS:
 *     NORMAL:          pane op will be dropped if it is associated
 *                      with pn. np will be added (if not null) as
 *                      a new p-space for the pane pn. If the last
 *                      p-space for a pane is dropped, the pane will
 *                      be linked to the blank p-space.
 *
 *     ABNORMAL:        N/A
 *
 * EXTERNAL REFERENCES: ecadpn, ecblks, ecaspn.
 *
 * RETURNED VALUES:     OK - always
 */

int     ecdppn (pn, op, np)	/* begin function definition    */
	PANE    *pn;            /* pointer to panel structure   */
WINDOW  *op, *np;               /* p-space pointers old / new   */

{

    PANEPS  *pa;                /* pointer to extra p-space str */

    if ((pn->exps) == NULL) {	/* If the chain of extras = null */
	if (pn->w_win == op) {	/* if current p-space is old one */
	    if (np != NULL) {	/* if new is not null           */
		pn->w_win = np;	/* make new, current p-space    */
	    }
	    else {		/* curr=old, new=NULL           */
		pn->w_win = ecblks();/* make current = blank p-space */
	    }
	    ecaspn(pn, pn->w_win);/* link p-space to view         */
	}
	else {			/* current p-space not = old    */
	    if (np != NULL) {	/* if new is not null           */
		ecadpn(pn, np);	/* add new as valid p-space     */
	    }
	}
    }
    else {			/* chain is not null            */
	for (pa = pn->exps;	/* search for block with op     */
		pa != NULL && pa->extps != op;
				/*                              */
		pa = pa->extnxt);/* search forward in list       */

	if (pa != NULL) {       /* op found in block pa
				   DELETE IT !!!!              */
	    if (pa->extnxt != NULL) {
				/* if fwd is not null
				   link fwd blk to pa prev    */
		(pa->extnxt)->extprv = pa->extprv;
	    }
	    if (pa->extprv != NULL) {
				/* if bkd link is not null
				   link prev blk to pa next   */
		(pa->extprv)->extnxt = pa->extnxt;
	    }
	    else {              /* else set link from pane blk to
				   next ext block          */
		pn->exps = pa->extnxt;
	    }
	    cfree(pa);		/* release storage for block    */
	}

	for (pa = pn->exps;	/* search for block with np     */
		pa != NULL && pa->extps != np;
				/*                              */
		pa = pa->extnxt);/* search forward in list       */

	if (pa == NULL) {	/* if no match found            */
	    if (np != NULL) {	/* if new is not null           */
		ecadpn(pn, np);	/* add new as valid p-space     */
	    }
	}

	if (pn->w_win == op) {	/* of old p-space was 'current' */
	    ecaspn(pn, pn->exps->extps);
				/* make new first the 'current' */
	}

	if (pn->exps->extnxt == NULL) {/* if chain length is exactly 1 */
	    cfree(pn->exps);	/* release the block            */
	    pn->exps = NULL;	/* clear link pointer           */
	}			/* end - exactly 1 block left   */

    }				/* end chain not null           */

    return OK;			/* return to caller - OK        */

}				/* end function ecobsc          */
