static char sccsid[] = "@(#)89	1.6  src/bos/usr/ccs/lib/libcur/ecadpn.c, libcur, bos411, 9428A410j 5/14/91 17:01:01";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: ecadpn
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

#ifdef OLD_STYLE /* Apr/23/91 by Tz */
char   *calloc ();		/* storage allocate function	 */
#endif

/*
 * NAME:                ecadpn
 *
 * FUNCTION:            Add the specified pspace to those which can
 *                      be used with the specified pane.
 *
 * EXECUTION ENVIRONMENT:
 *
 *   PARAMETERS:        pane - pointer to pane to be modified
 *                      win - window to be added to those for pane
 *
 *   INITIAL CONDITIONS pane and win must be valid pointers to
 *                      structures of the appropriate type.
 *
 *   FINAL CONDITIONS:
 *
 *     NORMAL:          block added to chain of blocks for the pane
 *                      each block represents a window which can
 *                      be presented in the pane.
 *
 *     ABNORMAL:        if unable to allocate storage for block
 *                      ERR is returned and the win cannot be used
 *                      with the pane
 *
 * EXTERNAL REFERENCES: calloc
 *
 * RETURNED VALUES:     OK - no error
 *                      ERR - error, unable to allocate work space
 */

int     ecadpn (pn, pw)		/* begin function		 */
register
	    PANE    *pn;        /* Parameter is pane pointer     */

register
	    WINDOW  *pw;        /* Parameter pointer to window   */

{				/* begin function code		 */

    register
		PANEPS  *pa;    /* pointer to pane aux struct    */

    if (pn->exps == NULL) {	/* if chain is now null 	 */
	if ((pa = (PANEPS *) calloc(1, sizeof(PANEPS))) == NULL)
	    return ERR;		/* if unable to allocate - ERR	 */

	pn->exps = pa;		/* set chain from PANE		 */
	pa->extps = pn->w_win;	/* set win pointer in new block  */

	pa->expvsid = pn->pnvsid;/* set vsid into new block	 */
    }

    if ((pa = (PANEPS *) calloc(1, sizeof(PANEPS))) == NULL)
	return ERR;		/* if unable to allocate - ERR	 */

    pa->extnxt = pn->exps;	/* chain new block to old header */
    (pa->extnxt)->extprv = pa;	/* - and old back to new         */
    pn->exps = pa;		/* add new block to head of chn  */
    pa->extps = pw;		/* set window pointer in block	 */

#ifdef	PCWS

    if (PS) {			/* if terminal has external ps	 */
	vsdef.vsid = (++vsid);	/* get next screen id		 */
	pa->vsid = vsid;
	vsdef.row = pw.maxy;	/* set size for ps in vsdef	 */
	vsdef.col = pw.maxx;	/* 				 */
	vsdef.? ? ? ?		/* other vsdef parameters	 */
	    write		/* write vsdef to terminal	 */
    }
#endif

    return OK;			/* return to caller - no error	 */

}				/* end function ecadpn		 */
