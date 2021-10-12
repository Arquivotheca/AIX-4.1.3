static char sccsid[] = "@(#)78	1.8  src/bos/usr/ccs/lib/libcur/ecdvpl.c, libcur, bos411, 9428A410j 6/16/90 01:38:29";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: ecdvpl, ecdvpn
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

#include "cur99.h"
#include "cur05.h"

static
	PANEL   *homepl;        /* static/global homp panel  */
				/* - address                 */
/*
 * NAME:                ecdvpl
 *
 * FUNCTION:            Based on the division and neighbor pointers
 *                      and division size specifications in the
 *                      panes set the size and position information
 *                      for panel
 *
 * EXECUTION ENVIRONMENT:
 *
 *   PARAMETERS:        pnlptr - pointer to the PANEL structure to
 *                               be divided.
 *
 *   INITIAL CONDITIONS The panel must contain a non-null pointer to
 *                      the first pane division string.  The
 *                      structure implied by the pointers in the
 *                      panes must be that of a tree with leaves of
 *                      the tree having a null pointer for 'divd', a
 *                      pane which is not a leaf of the tree will
 *                      indicate its divisions through the 'divd'
 *                      pointer.  Each complete set of divisions of
 *                      a pane is the set of panes implied by the
 *                      'divs' chain.  No pane may appear in the
 *                      tree more than once.
 *
 *   FINAL CONDITIONS:
 *     NORMAL:          All pane sizes and origins relative to the
 *                      panel will be filled in.  The next and
 *                      previous pane pointers in the pane blocks
 *                      will be set to link the leaf panes together.
 *
 *     ABNORMAL:        Return code minus 1 indicates  the structure
 *                      contained at least one of the following
 *
 *                              Total of fixed sizes of divisions
 *                              exceeded available space
 *
 *                              Total of fractional sizes exceeded 1
 *
 *                              Floating pane sizes could not be at
 *                              least one character in size.
 *
 * EXTERNAL REFERENCES: N/A
 *
 * RETURNED VALUES:     return code of OK if successful or ERR
 */

int     ecdvpl (pnl)		/* divide panel function     */
register
	    PANEL   *pnl;       /* argument is panel pointer */

{				/* begin function body	     */

    PANE    *cp;                /* pane for first division   */
    PANE    *rpane;             /* pane pointer returned by  */
				/* -- ecdvpn local function  */
    PANE    *ecdvpn();          /* local function divide pane */
    int     sz;			/* size of dimension to div  */
    int     ssz;		/* size of dimension across  */
				/* -- divide dimension      */
    char    dty;		/* initial division type code */


    if ((pnl->p_depth > LINES) ||/* assure that size is valid */
	    (pnl->p_depth <= 0))
	pnl->p_depth = LINES;

    if ((pnl->p_width > COLS) ||
	    (pnl->p_width <= 0))
	pnl->p_width = COLS;

    if (pnl->orow == -1)	/* if origin is -1 set the   */
	pnl->orow = LINES - pnl->p_depth;/* - origin for bottom/right */

    if (pnl->ocol == -1)	/*                           */
	pnl->ocol = COLS - pnl->p_width;/*                           */

    if ((pnl->orow < 0) ||	/* if origin is not valid set */
	    (pnl->orow >= LINES))
	pnl->orow = (LINES - pnl->p_depth) / 2;
				/* - origin for center       */

    if ((pnl->ocol < 0) ||	/*                           */
	    (pnl->ocol >= COLS))
	pnl->ocol = (COLS - pnl->p_width) / 2;
				/*                           */


    if (pnl->p_depth + pnl->orow > LINES) {
				/* if definition extends past */
				/* -- display bottom             */
	pnl->p_depth = LINES - pnl->orow;/* change size to fit	     */
    }

    if (pnl->p_width + pnl->ocol > COLS) {
				/* if definition extends past */
				/* -- display right edge     */
	pnl->p_width = COLS - pnl->ocol;/* change size to fit        */
    }

    if (pnl->dpane != NULL) {	/* if division is specified  */
	for (cp = pnl->dpane, dty = pnl->divty;
				/* start at first division   */
		cp->divs == NULL &&/* but bypass all initial    */
		cp->divd != NULL;/* - panes that have no      */
		cp = cp->divd) {/* - neighbor panes and are
				   themselves divided      */
	    dty = cp->divty;	/* - no need to check them   */
	}			/* - pick up the division    */
				/* - direction code on the   */
				/* - way down                */

	if (dty == Pdivtyv) {	/* if divide vertical dim    */
	    sz = pnl->p_depth;	/* set divide for vertical   */
	    ssz = pnl->p_width;
	}
	else {
	    sz = pnl->p_width;	/* else use horizontal dim   */
	    ssz = pnl->p_depth;
	}

	homepl = pnl;		/* save pnl pointer in global */
	rpane = ecdvpn(sz,	/* size of dim to divide     */
		ssz,		/* size across divide direct */
		0, 0,		/* origin in panel for div.  */
		cp,		/* first pane of division    */
		dty,		/* direction of divide flag  */
		pnl->bordr,	/* border in parent flag     */
		cp,		/* previous pane pointer     */
		cp);		/* next pane pointer         */
	if ((int) rpane != NULL) {/* if valid pointer returned */
	    pnl->fpane = rpane;	/* set first pane ptr for pnl */
	    return OK;		/* set return code to OK     */
	}
    }

    return ERR;			/* return error code if any  */

}				/* end of function	     */

/*
 * NAME:                ecdvpn
 *
 * FUNCTION: Divide the specified pane as indicated
 *
 * EXECUTION ENVIRONMENT:
 *
 *                      sz - size of dimension being divided including
 *                           any border characters
 *                      ssz- size across division dimension (static
 *                           size) including any border
 *                      yo,xo - row/column origin in panel of pane
 *                           being divided
 *                      pane - pointer to first PANE in division set
 *                      divdr - flag indicating division direction
 *                           horizontal or vertical
 *                      bordr - flag indicating if parent pane has
 *                           borders defined for it.
 *                      prev - previous pane to chain to
 *                      next - next pane to chain to
 */

PANE    *ecdvpn(sz, ssz, yo, xo, dpane, divdr, pbord, Prev, Next)
				/* divide a pane tree.	     */
int     sz;			/* size of dimension to div. */
int     ssz;			/* size of static dimension  */
int     yo;			/* origin row of parent pane */
int     xo;			/* origin col of parent pane */
PANE    *dpane;                 /* pane to start division    */
char    divdr;			/* direction code for div    */
char    pbord;			/* border flag for parent    */
PANE    *Prev;                  /* previous pane pointer     */
PANE    *Next;                  /* next pane pointer         */

{				/* begin function	     */

				/* local declarations       */

    int     avsz;		/* available size for div    */
    PANE    *cp;                /* current working pane ptr  */
    char    prbor;		/* previous pane had border  */

    int     pass;		/* pass counter 	     */
#define Pass1 1
#define Pass2 2
#define Pass3 3
#define Pass4 4
#define Pass5 5

    int     frtot;		/* total allocated to fr pns */
    int     frcnt;		/* fraction pane count	     */
    int     flcnt;		/* count of float panes      */
    int     flsz;		/* size for float panes      */
    int     flrm;		/* remainder for float panes */
    int     yc,
            xc;			/* row,col current origin    */
    int     dsz;		/* pass on divide dimension  */
    int     asz;		/* pass on static dimension  */

    PANE    *dpntr;             /* division result pointer   */
    PANE    *retcd;             /* return code to caller     */

    int     *ptcdv;             /* current origin coordinate */
				/* -- along division        */
    short int  *ptpdd;		/* dimension being divided   */
				/* -- in PANE structure      */
    short int  *ptpds;		/* static dimension in PANE  */
				/* -- structure             */
    short int  *ptpod;		/* origin along division     */
				/* -- in PANE structure      */
    short int  *ptpos;		/* origin on static dimension */
				/* -- in PANE structure      */

    for (pass = Pass1; pass <= Pass5; pass++) {
				/* for each of the passes    */
	switch (pass) {		/* initialization for pass   */
	    case Pass1: 
		avsz = sz;	/* init total size	     */
		if (pbord == Pbordry) {/* if parent had borders     */
		    avsz -= 2;	/* decrement available size  */
		    prbor = TRUE;/* set flag - previous bord. */
		}
		else {
		    prbor = FALSE;/* else no previous border   */
		}
		break;
	    case Pass2: 
		break;
	    case Pass3: 
		frtot =		/* initialize total for fract */
		    frcnt =	/* init cnt of fract panes   */
		    flcnt = 0;	/* init cnt of floating panes */
		break;
	    case Pass4: 
		if (flcnt != 0) {/* if some floating panes    */
		    flsz = avsz / flcnt;/* size for each pane	     */
		    flrm = avsz % flcnt;/* remainder to distribute   */
		    if (flsz <= 0) {
			return((PANE *) NULL);
		    }		/* if no room return error   */
		}
		else
		    if (frcnt > 0) {
			flsz = avsz / frcnt;
				/* size to add to each fr pn */
			flrm = avsz % frcnt;
				/* remainder to distribute   */
		    }
		else
		    if (avsz > 0) {
				/* if left over and no place to put
				   it               */
			return((PANE *) NULL);
				/* -- error code return      */
		    }
		break;
	    case Pass5: 
		yc = yo;	/* init coordinate for pane  */
		xc = xo;	/* -- to parent coordinate   */
		if (pbord == Pbordry) {/* init prev. border flag    */
		    prbor = TRUE;/* parent/prev had border    */
		}
		else {
		    prbor = FALSE;/* parent/prev no border     */
		}
		break;
	}

	for (cp = dpane; cp != NULL; cp = cp->divs) {
				/* loop thru panes in set    */
	    switch (pass) {	/* processing for each pane  */
		case Pass1: 	/* determine space for assgn */
		    cp->hpanl = homepl;/* set link back to home pnl */
		    if (cp->bordr == Pbordry) {
				/* if border for this pane   */
			if (prbor) {/* if prev pane had border   */
			    avsz--;/* decrement for new border  */
			}
			else {
			    avsz -= 2;/* decr for both borders     */
			}
			prbor = TRUE;/* set flag for this pane    */
		    }
		    else {	/* no border this pane	     */
			prbor = FALSE;/* set flag for this pane    */
		    }
		    break;
		case Pass2: 	/* assign size for fixed pn  */
		    if (cp->divszu == Pdivszc) {
				/* if constant size pane     */
			cp->v_depth = cp->divsz;
				/* hold assigned size as row */
			avsz -= cp->divsz;/* adjust free size left     */
			if (cp->bordr == Pbordry) {
				/* if border for this pane   */
			    cp->v_depth += 2;
				/* adjust pane size for bord */
			}
			if (avsz < 0) {/* if error exit now !	     */
			    return((PANE *) NULL);
				/* -- error code return      */
			}
		    }
		    break;
		case Pass3: 	/* process proportional panes */
		    if (cp->divszu == Pdivszp) {
				/* if proportion size	     */
			cp->v_depth =
				((long)((long) cp->divsz * avsz)) / 10000;
				/* calculate size from fract */
			frtot += cp->v_depth;
				/* add into total allocated  */
				/* -- for fraction panes     */
			if (cp->bordr == Pbordry) {
				/* if border on this pane    */
			    cp->v_depth += 2;
				/* adjust size for borders   */
			}
			frcnt++;/* count fraction panes      */
			if (frtot > avsz) {
				/* if error exit now !	     */
			    return((PANE *) NULL);
				/* -- error code for return  */
			}

		    }
		    else
			if (cp->divszu == Pdivszf) {
				/* if floating size	     */
			    flcnt++;/* increment floating count  */
			}
		    break;
		case Pass4: 	/* process floating panes    */
		    if (cp->divszu == Pdivszf) {
				/* if floating pane	     */
			cp->v_depth = flsz;
				/* assign normal float size  */
			if (flrm > 0) {/* if remainder after div    */
			    cp->v_depth++;/* add a position to size    */
			    flrm--;/* decrement remainder	     */
			}
			if (cp->bordr == Pbordry) {
				/* if border on this pane    */
			    cp->v_depth += 2;
				/* adjust size for borders   */
			}
		    }
		    else
			if ((cp->divszu == Pdivszp) && (flcnt == 0)) {
				/* if distributing over fr   */
			    cp->v_depth += flsz;
			    if (flrm > 0) {
				/* if remainder after div    */
				cp->v_depth++;
				/* add a position to size    */
				flrm--;/* decrement remainder	     */
			    }
			}
		    break;
		case Pass5: 	/* assign origin and extent  */
		    cp->orow = yc;/* initialize to current     */
		    cp->ocol = xc;/* -- origin		     */
		    if (divdr == Pdivtyv) {
				/* if dividing vertical dim  */
			ptcdv = &yc;/* coordinate along division */
			ptpdd = &cp->v_depth;
				/* dim along division	     */
			ptpds = &cp->v_width;
				/* dim that is 'static'      */
			ptpod = &cp->orow;/* origin along division dim */
			ptpos = &cp->ocol;/* origin along static dim   */
		    }
		    else {	/* if dividing horiz dimen.  */
			ptcdv = &xc;/* coordinate along division */
			ptpdd = &cp->v_width;
				/* dim along division	     */
			ptpds = &cp->v_depth;
				/* dim that is 'static'      */
			ptpod = &cp->ocol;/* origin along division dim */
			ptpos = &cp->orow;/* origin along static dim   */
		    }
		    *ptcdv += cp->v_depth;/* increment origin by size  */
		    *ptpdd = cp->v_depth;/* move size to proper slot  */
		    *ptpds = ssz;/* set fixed size, across div */
				/* -- i.e. static dimension  */
		    if (cp->bordr == Pbordry) {
				/* if border for this pane   */
			(*ptcdv)--;/* decrement next origin to  */
				/* leave 'in border'         */
			prbor = TRUE;/* remember this border      */
		    }
		    else {
			if (prbor) {
				/* if previous had border but this
				   does not	     */
			    (*ptpod)++;/* step origin along division */
			    (*ptcdv)++;/* increment next origin also */
			}
			if (pbord == Pbordry) {
				/* if parent had border -- but
				   this does not      */
			    (*ptpos)++;/* shift pane away from bord */
			    (*ptpds) -= 2;/* reduce size to fit	     */
			}
			prbor = FALSE;/* remember border state     */
		    }
		    cp->nxtpn = Next;/* link pane to surrounding  */
		    cp->prvpn = Prev;/* -- panes		     */
		    Prev->nxtpn = cp;
		    Next->prvpn = cp;
		    Prev = cp;	/* set up for next pane link */
		    break;
	    }			/* end of switch - in loop   */
	}			/* end of loop thru panes    */

	switch (pass) {		/* post processing for pass  */
	    case Pass1: 
		if (prbor && (pbord == Pbordry)) {
				/* if last pane had border -- and
				   parent had border  */
		    avsz++;	/* increment available size  */
		}
		break;
	    case Pass2: 
		break;
	    case Pass3: 
		avsz -= frtot;	/* calculate size for float  */
		break;
	    case Pass4: 
		break;
	    case Pass5: 
		break;
	}			/* end switch - post proc    */
    }				/* end multiple pass loop    */

    retcd = dpane;		/* init return code to 1st pn */

    for (cp = dpane; cp != NULL; cp = cp->divs) {
				/* step thru panes again     */
	if (cp->divd != NULL) {	/* if pane is divided	     */
	    if (cp->divty == Pdivtyv) {/* if divide along vertical  */
		dsz = cp->v_depth;/* pass rows as div dim      */
		asz = cp->v_width;/* and cols as static dim    */
	    }
	    else {		/* for horiz divide	     */
		dsz = cp->v_width;/* pass cols as div dim      */
		asz = cp->v_depth;/* and rows as static dim    */
	    }
	    dpntr = ecdvpn(dsz,	/* size along division	     */
		    asz,	/* static size		     */
		    cp->orow,	/* origin row of pane	     */
		    cp->ocol,	/* origin column of pane     */
		    cp->divd,	/* pane to start division    */
		    cp->divty,	/* direction of divide	     */
		    cp->bordr,	/* border for parent pane    */
		    cp->prvpn,	/* previous pane to link to  */
		    cp->nxtpn);	/* next pane to link to      */
	    if (dpntr == (PANE *) NULL) {/* if error in divide        */
		return dpntr;	/* return now with error     */
	    }
	    else {
		if (cp == dpane) {/* if pane was first of set  */
		    retcd = dpntr;/* update return pointer     */
		}
	    }
	}			/* end if dividing pane      */
    }				/* end loop thru panes	     */
    return retcd;		/* normal end - ptr to 1st dv */
}				/* end routine ecdvpn	     */
