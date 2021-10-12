static char sccsid[] = "@(#)44	1.9  src/bos/usr/ccs/lib/libcur/ecdfpl.c, libcur, bos411, 9428A410j 6/16/90 01:38:14";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: ecdfpl, nc
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

/*
 * NAME:                ecdfpl
 *
 * FUNCTION:            Create all the auxiliary structures used with
 *                      the panel specified
 *
 * EXECUTION ENVIRONMENT:
 *
 *   PARAMETERS:        pnl - pointer to panel structure
 *                      mpwom - boolean TRUE indicates do not
 *                              create p-space for panes
 *
 *   INITIAL CONDITIONS panel structure must be fully defined and all
 *                      links to panes and between panes must be
 *                      filled with valid values. If the application
 *                      wishes to pre-allocate the p-space, the
 *                      w_win pointer must be set in the pane blocks
 *
 *   FINAL CONDITIONS:
 *     NORMAL:          The pointers in the pane and panel which are
 *                      defined as pointers to p-space windows will
 *                      be filled in with valid values, if needed the
 *                      pointers to view window structures will also
 *                      be filled in.
 *
 *     ABNORMAL:        Some window structures may be created but
 *                      must not be assumed to be present.
 *
 * EXTERNAL REFERENCES: newwin, newview, drawbox, mvwin,
 *
 * RETURNED VALUES:     zero if normal, minus 1 (ERR) if error
 */

#include        "cur99.h"
#include        "cur05.h"

int     vsid = 0;		/* virtual screen ident code */

int     nca[] = {		/* pointers to box characters */
    1,				/* code 0000 char 'horiz'    */
    1,				/* code 000r char 'horiz'    */
    1,				/* code 00l0 char 'horiz'    */
    1,				/* code 00lr char 'horiz'    */
    3,				/* code 0t00 char 'vert'     */
    5,				/* code 0t0r char 'bot left' */
    4,				/* code 0tl0 char 'bot right' */
    8,				/* code 0tlr char 'bot tee'  */
    3,				/* code b000 char 'vert'     */
    0,				/* code b00r char 'top left' */
    2,				/* code b0l0 char 'top right' */
    6,				/* code b0lr char 'top tee'  */
    3,				/* code bt00 char 'vert'     */
    9,				/* code bt0r char 'left tee' */
    7,				/* code btl0 char 'right tee' */
    10
};				/* code btlr char 'cross'    */


int     ecdfpl (pnl, nowin)	/* define entry point        */
	PANEL   *pnl;           /* argument is panel pointer */
char    nowin;			/* TRUE - do not allocate    */
				/*        p-space for panes  */
				/* FALSE- allocate p-space   */
				/*        if null in block   */

{				/* begin program body	     */

				/* local declarations       */
    PANE    *cp;                /* current pane pointer      */
    int     adj;		/* adjustment 0 if no border */
				/* -- 1 if border           */
    int     adj2;		/* double value of adj       */
    int     x1,
            x2,
            y1,
            y2;			/* work row/column of corners */

    if (pnl->p_depth > LINES)	/* assure valid width and    */
	pnl->p_depth = LINES;	/* - depth values in struct  */

    if (pnl->p_width > COLS)
	pnl->p_width = COLS;

    if ((pnl->orow < 0) ||	/* if origin is not valid set */
	    (pnl->orow >= LINES))
	pnl->orow = (LINES - pnl->p_depth) / 2;
				/* - origin for center       */

    if ((pnl->ocol < 0) ||	/*                           */
	    (pnl->ocol >= COLS))
	pnl->ocol = (COLS - pnl->p_width) / 2;
				/*                           */

    pnl->p_win = newwin(pnl->p_depth,/* create window for panel   */
	    pnl->p_width,	/* -- use size and origin    */
	    pnl->orow,		/* -- from the panel struct  */
	    pnl->ocol);

    if (pnl->p_win == NULL)
	return ERR;		/* if no room for wind - ERR */

    if (pnl->bordr == Pbordry) {/* if needed draw box on pane */
	wcolorout(pnl->p_win, Bxa);/* set attribute for box     */
	cbox(pnl->p_win);	/* draw box in panel p-space */
	wcolorend(pnl->p_win);	/* reset normal attribute    */
    }

    for (cp = pnl->fpane;;) {	/* step thru panes exit is   */
				/* -- explicit at loop end   */


	adj = ((cp->bordr == Pbordry) ? 1 : 0);
				/* adj = 1 if border else 0  */
	adj2 = adj + adj;	/* double adj for size adjust */

	if (cp->w_win == NULL) {
	    if (nowin == TRUE) {/* if no allocate requested  */
		cp->w_win = ecblks();/* use blank p-space         */
	    }
	    else {
		cp->w_win = newwin(/* create pspace for pane    */
			(cp->w_depth >= cp->v_depth - adj2 ?
			    cp->w_depth : cp->v_depth - adj2),
			(cp->w_width >= cp->v_width - adj2 ?
			    cp->w_width : cp->v_width - adj2),
			(pnl->orow + cp->orow + adj),
			(pnl->ocol + cp->ocol + adj));
				/* size is max of specified  */
				/* -- pspace size and view   */
				/* -- size, origin is panel  */
				/* -- origin + origin in pnl */
				/* -- plus adj for border    */
	    }
	    if (cp->w_win == NULL)
		return ERR;	/* if error on allocate quit */


	    cp->alloc = Pallocy;/* flag to indicate w_win    */
				/* - created here            */
	}
	else {                  /* if p-space pre-allocated assure
				   correct origin   */
	    (cp->w_win)->_begy = (pnl->orow + cp->orow + adj);
	    (cp->w_win)->_begx = (pnl->ocol + cp->ocol + adj);
	    cp->alloc = Pallocn;/* flag to indicate w_win    */
				/* - not created here        */
	}


#ifdef	PCWS
	if (!PW)		/* if not defining on PCWS   */
# endif
	{
	    getyx(cp->w_win, y1, x1);/* save current cursor locat */
	    wmove(cp->w_win, 0, 0);/* force view to top left    */

	    cp->v_win = newview(cp->w_win,/* define a window for view  */
		    cp->v_depth - adj2,
				/* -- on p-space, size of    */
		    cp->v_width - adj2);
				/* -- specified view minus   */
				/* -- borders if any         */
	    if (cp->v_win == NULL)
		return ERR;	/* if error on allocate quit */
	    wmove(cp->w_win, y1, x1);/* reset cursor position     */

	    mvwin(cp->w_win,	/* now move to real display  */
		    (pnl->p_win)->_begy,/* -- coordinates	     */
		    (pnl->p_win)->_begx);/* 			     */

	    if (adj != 0) {	/* if border is specified    */
		wcolorout(pnl->p_win, Bxa);
				/* set attribute for box     */
				/* draw box in panel p-space */
				/* at the view origin        */
				/* with full view size       */
		drawbox(pnl->p_win,
			cp->orow,
			cp->ocol,
			cp->v_depth,
			cp->v_width);
		wcolorend(pnl->p_win);/* reset normal attribute    */
	    }

	}
#ifdef	PCWS
	else {
	    cp->v_win = (++vsid);/* incre vir scr id for pane */
	    vscnt++;		/* count virtual screens     */
	}
#endif

	cp = cp->nxtpn;		/* step pointer to next pane */
	if ((cp == NULL) || (cp == pnl->fpane)) {
				/* if at end of chain        */
	    break;		/* at end - exit from loop   */
	}
    }

				/* begin second pass on panes */
    for (cp = pnl->fpane;;) {	/* initialize for scan	     */
#ifdef	PCWS
	if (!PW)		/* not defining for PCWS     */
# endif
	{
	    if (cp->bordr == Pbordry) {/* if border for this pane   */
		y1 = cp->orow;	/* top row		     */
		y2 = cp->orow + cp->v_depth - 1;
				/* last row		     */
		x1 = cp->ocol;	/* first column 	     */
		x2 = cp->ocol + cp->v_width - 1;
				/* last column		     */

		wcolorout(pnl->p_win, Bxa);
				/* set attribute for box     */
		nc(y1, x1, pnl);/* determine all new corners */
		nc(y2, x1, pnl);
		nc(y1, x2, pnl);
		nc(y2, x2, pnl);
		wcolorend(pnl->p_win);/* reset normal attribute    */
	    }
	}

#ifdef	PCWS
	else {
	    if (dfp == NULL) {  /* if display frame area not
				   defined yet	     */
		dfp = calloc(sizeof(dfpstr) + sizeof(short) * vsct);
				/* allocate storage for df   */
		pnl->dfid = (++vsid);/* assign df id 	     */
		dfp->dfid = vsid;/* move to df structure too  */
		dfp->leng = sizeof(? ? ? ?);/* size of header	     */
		dfp->type = '????'/* type code                 */
		    dfp->pcnt = vsct;/* number of screens	     */
		nvsp = &dfp->fvs;/* init pointer to vs field  */
	    }

	    dvs.leng = sizeof(dvs);/* set size of structure     */
	    dvs.hdln = sizeof(dvs.? ?);/* size of header	     */
	    dvs.hdty = '????';	/* header type code          */
	    dvs.vsdf = '????';	/* set block data type       */
	    dvs.vsor = cp->orow + pnl->orow + adj;
				/* origin row                */
	    dvs.vsoc = cp->ocol + pnl->ocol + adj;
				/* origin column             */
	    dvs.vsnr = cp->w_depth - adj2;/* size in rows              */
	    dvs.vsnc = cp->w_width - adj2;/* size in columns           */
	    ? ? dvs.vslh = (cp->hscr)->v_win;
				/* vsid for hor scroll	     */
	    oops null pointer for link
		? ? dvs.vslv = (cp->vscr)->v_win;
				/* vsid for ver scroll	     */

	    write(term, dvs, sizeof(dvs));/* sent to terminal	     */
	}
#endif
	cp = cp->nxtpn;		/* step pointer to next pane */
	if ((cp == NULL) || (cp == pnl->fpane)) {
				/* if at end of chain        */
	    break;		/* at end - exit from loop   */
	}
    }				/* end - loop through panes  */

    if (pnl->bordr == Pbordry && pnl->title != NULL) {
				/* if title specified w/ bord */
				/* NLS supp-use display length */
	x1 = NLstrdlen(pnl->title);/* get title length          */
	if (x1 < pnl->p_width) {/* if title will fit         */
	    mvwaddstr(pnl->p_win, 0,(pnl->p_width - x1) / 2, pnl->title);
	}			/* put title in panel        */
    }				/* end if title              */

#ifdef	PCWS
    if (PW)			/* if defining on PCWS	     */
	write(term, dfp, sizeof(dfpb));/* define the dialog frame   */
#endif

    return(OK);
}				/* end ecdfpl main line      */



/*
 * NAME:                nc
 *
 * FUNCTION: y = row, x = col in panel pspace for border corner
 *      based on four neighbors change corner character as needed
 */

int     nc (y, x, pl)
int     y;			/* row of corner character   */
int     x;			/* column of corner character */
PANEL   *pl;                    /* address of panel struct   */

{				/* begin function	     */

    int     cn = 0;		/* neighbor code	     */
    char    ch;			/* current character	     */
    WINDOW  *pnlp;              /* panel window address      */

    pnlp = pl->p_win;		/* get pointer to window     */


    if (y != 0) {		/* if not at top row	     */
	wmove(pnlp, y - 1, x);
	ch = winch(pnlp);
	cn += ((ch == ' ') || (ch == '\0')) ? 0 : 4;
				/* char above coded as 4     */

    }

    if (x != 0) {		/* if not at first column    */
	wmove(pnlp, y, x - 1);
	ch = winch(pnlp);
	cn += ((ch == ' ') || (ch == '\0')) ? 0 : 2;
				/* char left  coded as 2     */
    }

    if (y < (pnlp)->_maxy - 1) {/* if not at last row	     */
	wmove(pnlp, y + 1, x);
	ch = winch(pnlp);
	cn += ((ch == ' ') || (ch == '\0')) ? 0 : 8;
				/* char below coded as 8     */
    }

    if (x < (pnlp)->_maxx - 1) {/* if not at last column     */
	wmove(pnlp, y, x + 1);
	ch = winch(pnlp);
	cn += ((ch == ' ') || (ch == '\0')) ? 0 : 1;
				/* char right coded as 1     */
    }

    mvwaddch(pnlp, y, x, BX[nca[cn]]);
				/* put correct char in panel */
    return(OK);

}				/* end of function nc	     */
