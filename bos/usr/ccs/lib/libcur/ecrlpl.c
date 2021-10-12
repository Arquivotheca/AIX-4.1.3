static char sccsid[] = "@(#)01  1.6  src/bos/usr/ccs/lib/libcur/ecrlpl.c, libcur, bos411, 9428A410j 6/16/90 01:39:34";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: ecrlpl
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
 * NAME:                ecrlpl
 *
 * FUNCTION:            Release and free all auxiliary data structs
 *                      for this panel (pspaces) retain all panel and
 *                      and pane structures.
 *
 * EXECUTION ENVIRONMENT:
 *
 *   PARAMETERS:        pl - pointer to Panel to be released
 *
 *   INITIAL CONDITIONS pl must indicate a valid panel structure
 *
 *   FINAL CONDITIONS:
 *     NORMAL:          all widow and view structures associated
 *                      with the panel or its panes will be released
 *                      including all extra p-spaces
 *
 *     ABNORMAL:
 *
 * EXTERNAL REFERENCES: ecdppn, delwin
 *
 * RETURNED VALUES:     OK - always
 */

int     ecrlpl (pl)		/* Function ecrlpl              */
	PANEL   *pl;            /* pointer to panel to release  */

{

    PANE    *pn;                /* pane pointer                 */
    WINDOW  *pw;                /* Window pointer               */

    pn = pl->fpane;		/* initialize ptr to first pane */

    if (pl->p_under != NULL || pl->p_over != NULL || pl == _toppl)
	ecrmpl(pl);		/* remove from display first    */

    do {			/* repeat until end of pane chn */
	while (pn->exps != NULL) {/* while extra p-spaces defined */
	    pw = (pn->exps)->extps;/* get window address           */
	    ecdppn(pn, pw, NULL);/* go release that window       */
	    if (pn->alloc != Pallocn)
		delwin(pw);
				/* release the p-space itself   */
				/* - if if ecdfpl allocated any */
	}

	if (pn->v_win != NULL) {/* if view is defined           */
	    delwin(pn->v_win);	/* release view structure       */
	    pn->v_win = NULL;	/* clear pointer                */
	}

	if ((pn->w_win != NULL) &&/* if p-space is defined        */
		(pn->alloc != Pallocn)) {
				/* - and allocate was done by   */
				/* - ecdfpl then                */
	    delwin(pn->w_win);	/* release p-space structure    */
	    pn->w_win = NULL;	/* clear pointer                */
	}

	pn = pn->nxtpn;		/* step ptr to next pane        */

    } while (pn != pl->fpane);	/* repeat if not back to start  */

    if (pl->p_win != NULL) {	/* if window defined for panel  */
	delwin(pl->p_win);	/* release that window          */
	pl->p_win = NULL;	/* clear the pointer            */
    }

    return OK;			/* return to caller             */
}				/* end ecrlpl                   */
