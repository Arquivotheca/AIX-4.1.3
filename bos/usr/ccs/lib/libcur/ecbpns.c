static char sccsid[] = "@(#)33	1.7  src/bos/usr/ccs/lib/libcur/ecbpns.c, libcur, bos411, 9428A410j 5/14/91 17:01:08";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: ecbpns
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

#ifdef OLD_STYLE	/* Apr/23/91 by Tz */
char   *calloc ();		/* obtain storage function      */
#endif

/*
 * NAME:                ecbpns
 *
 * FUNCTION:            Build a pane structure for use within a panel
 *                      link to other structures and fill in parms.
 *
 * EXECUTION ENVIRONMENT:
 *
 *   PARAMETERS:        pr - short integer, rows in pspace for pane
 *                      pc - short integer, cols in pspace for pane
 *                      ln - pointer to neighbor pane above or to lef
 *                      ld - link to start of chain for divisions of
 *                           this pane
 *                      dd - code for dimension of this pane along
 *                           which division is to occur, used if and
 *                           only if ld is not null.
 *                      ds - size of this pane, as part of the
 *                           division of a parent pane.
 *                      du - code for type of size spec. in ds
 *                      bd - border code, border on this pane yes/no
 *                      lh - link to pane which is to scroll with
 *                           pane when the pane scrolls horizontally
 *                      lv - link to pane which is to scroll with
 *                           pane when the pane scrolls vertically.
 *
 *   INITIAL CONDITIONS ln must either point to a valid pane struct o
 *                      must be null. The other pointers should be
 *                      valid but this routine is not dependent on
 *                      that fact.
 *
 *   FINAL CONDITIONS:
 *     NORMAL:          new pane structure created and pointer re.
 *                      if ln is not null the pointer 'divs' in the
 *                      structure pointed to by ln will be set to
 *                      point to the new structure
 *
 *     ABNORMAL:        -1  returned to indicate structure not built
 *
 * EXTERNAL REFERENCES: calloc
 *
 * RETURNED VALUES:     Pointer to pane structure created or minus 1
 *                      if unable to create the structure.
 */

PANE    *ecbpns(pr, pc, ln, ld, dd, ds, du, bd, lh, lv)
				/* function invocation format   */
short int   pr;			/* rows in p-space              */
short int   pc;			/* cols in p-space              */
PANE    *ln;                    /* neighbor pane above or left  */
PANE    *ld;                    /* start of divisions of this pn */
char    dd;			/* division direction code      */
short int   ds;			/* size of this pane            */
char    du;			/* units of this pane size      */
char    bd;			/* border on this pane code     */
PANE    *lh;                    /* pane to link for horz scroll */
PANE    *lv;                    /* pane to link for vert scroll */

{				/* begin function               */

    register
		PANE    *pptr;  /* pointer to new pane struct   */

    pptr = (PANE *) calloc(1, sizeof(PANE));
				/* get storage for new struct   */

    if ((int) pptr == 0) {	/* if error on allocate         */
	return(PANE *)(-1);	/* return error code            */
    }

    pptr->w_depth = pr;		/* set parameters into struct   */
    pptr->w_width = pc;
    pptr->divd = ld;
    pptr->divty = dd;
    pptr->divsz = ds;
    pptr->divszu = du;
    pptr->bordr = bd;

    if (lh) {			/* if horiz. scroll link not 0  */
	pptr->hscr = ((lh->hscr) ? (lh->hscr) : lh);
				/* set ptr to others link if not */
				/* - null or to other pane if   */
				/* - null in the other pane link */
	lh->hscr = pptr;	/* link other pane to this pane */
    }

    if (lv) {			/* if vert   scroll link not 0  */
	pptr->vscr = ((lv->vscr) ? (lv->vscr) : lv);
				/* set ptr to others link if not */
				/* - null or to other pane if   */
				/* - null in the other pane link */
	lv->vscr = pptr;	/* link other pane to this pane */
    }

    if (ln) {			/* if neighbor pane ptr not null */
	pptr->divs = ln->divs;	/* get its current link into    */
				/* -- the new structure         */
	ln->divs = pptr;	/* set pointer to new struct in */
				/* -- neighbor struct           */
    }

    return(pptr);		/* return pointer to new struct */

}				/* end of function              */
