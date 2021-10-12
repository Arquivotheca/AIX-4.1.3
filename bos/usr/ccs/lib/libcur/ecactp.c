static char sccsid[] = "@(#)78	1.7  src/bos/usr/ccs/lib/libcur/ecactp.c, libcur, bos411, 9428A410j 6/16/90 01:37:47";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: ecactp, ecactpc, ecactpd
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
 * NAME:                ecactp
 *
 * FUNCTION:            make the specified pane the active pane in
 *                      its panel.
 *
 * EXECUTION ENVIRONMENT:
 *
 *   PARAMETERS:        pane - pointer to pane to be made active.
 *                      state- boolean, if true change pane to active
 *                             state, else change to inactive state
 *
 *   INITIAL CONDITIONS the specified pane must be defined within a
 *                      defined panel.
 *
 *   FINAL CONDITIONS:
 *                      The border characters for the specified pane
 *                      are changed to the active or inactive state
 *                      as indicated by the state parameter. If
 *                      anotherpane had been active and state was
 *                      true the prior active pane will be inactive.
 *
 * EXTERNAL REFERENCES: malloc
 *
 * RETURNED VALUES: zero if any error, else 1
 */

int     ecactp (pn, mkact)	/* pane and select state     */
	PANE    *pn;            /* pane to be changed        */
char    mkact;			/* new state - true = active */

{				/* begin ecactp              */

    PANEL   *pl;                /* home panel for arg pane   */

    pl = pn->hpanl;		/* get home panel pointer    */

    if (mkact) {		/* if request is make active */
	if (pl->apane != NULL && pl->apane != pn)
	    ecactpc(pl->apane, FALSE);/* inactivate prior active pn */
	ecactpc(pn, TRUE);	/* make argument pane active */
	pl->apane = pn;		/* set as active pane        */
    }
    else {			/* else request is incativate */
	ecactpc(pn, FALSE);	/* change argument to inact  */
	if (pl->apane == pn) {	/* if pane had been active pn */
	    pl->apane = NULL;	/* clear the pointer         */
	}
    }

    return OK;			/* return to caller          */
}				/* end ecactp                */

/*
 * NAME:                ecactpc (local)
 *
 * FUNCTION:            update border for a pane to state indicated
 *
 * EXECUTION ENVIRONMENT:
 *
 *   PARAMETERS:        pane - pointer to pane to be made active.
 *                      state- boolean, if true change pane to active
 *                             state, else change to inactive state
 *
 *   INITIAL CONDITIONS the specified pane must be defined within a
 *                      a defined panel.
 *
 *   FINAL CONDITIONS:
 *     NORMAL:          The border characters for the specified pane
 *                      are changed to the active or inactive state
 *                      as indicated by the state parameter.
 *
 *     ABNORMAL:
 *
 * EXTERNAL REFERENCES: chgattr
 * RETURNED VALUES:     zero if any error, else 1
 */

int     ecactpc (pn, stact)	/* pane and select state     */
	PANE    *pn;            /* pane to be changed        */
char    stact;			/* new state - true = active */

{				/* begin ecactp              */

    WINDOW  *plw;               /* window for panel borders  */

				/* pane location in panel    */
    int     ptop;		/* top row of pane           */
    int     pbot;		/* bottom row of pane        */
    int     plft;		/* left column of pane       */
    int     prig;		/* right column of pane      */
    int     row;		/* work pointer into array   */

    char    *obx;               /* old box char string       */
    ATTR    oat;                /* old attribute code        */
    char    *nbx;               /* new box char string       */
    ATTR    nat;                /* new attribute code        */

    NLSCHAR *cp;                /* ptr to char data in window */
    ATTR    *ap;                /* ptr to attr code in wind. */

    ATTR    ocsbp;              /* old standout pattern      */

    if (pn->bordr == Pbordry) {	/* if there is a border      */
	ptop = pn->orow;	/* extract border location   */
	plft = pn->ocol;	/*                           */
	prig = plft + pn->v_width - 1;/*                           */
	pbot = ptop + pn->v_depth - 1;/*                           */

	plw = pn->hpanl->p_win;	/* locate panel p-space      */
	ecpnmodf(pn);		/* indicate modification made */
	ocsbp = plw->_csbp;	/* save old display attr.    */

	if (stact) {		/* if new state is active    */
	    obx = BX;		/* change from inactive      */
	    oat = (ATTR) Bxa;	/*                           */
	    nbx = BY;		/* - to active set of chars  */
	    nat = (ATTR) Bya;	/* - and their attribute     */
	}
	else {
	    obx = BY;		/* else change from active   */
	    oat = (ATTR) Bya;	/*                           */
	    nbx = BX;		/* - to inactive set of chars */
	    nat = (ATTR) Bxa;	/* - and their attribute     */
	}
				   /* change top border         */
	for (cp = &plw->_y[ptop][plft],/* first character to change */
		ap = &plw->_a[ptop][plft];/* - and first attribute     */
		cp <= &plw->_y[ptop][prig];
				/* - until last char to chg  */
		cp++, ap++)	/* - steping pointers        */
	    ecactpd(cp, ap, obx, oat, nbx, nat);
				/* go change character       */

				/* change bottom border      */
	for (cp = &plw->_y[pbot][plft],/* first character to change */
		ap = &plw->_a[pbot][plft];/* - and first attribute     */
		cp <= &plw->_y[pbot][prig];
				/* - until last char to chg  */
		cp++, ap++)	/* - steping pointers        */
	    ecactpd(cp, ap, obx, oat, nbx, nat);
				/* go change character       */

	for (row = ptop + 1;	/* change side borders       */
		row < pbot;
		row++) {
	    ecactpd(&plw->_y[row][plft],/* change left border char   */
		    &plw->_a[row][plft],
		    obx,
		    oat,
		    nbx,
		    nat);

	    ecactpd(&plw->_y[row][prig],/* change right border char  */
		    &plw->_a[row][prig],
		    obx,
		    oat,
		    nbx,
		    nat);

	}

	for (row = ptop;	/* loop through the change   */
		row <= pbot;	/* - vectors and flag the box */
		row++) {
	    if (plw->_firstch[row] = _NOCHANGE) {
				/* if no prior change flag   */
		plw->_firstch[row] = plft;/* set change for box        */
		plw->_lastch[row] = prig;
	    }
	    else {		/* if prior change keep limit */
		if (plw->_firstch[row] > plft)
		    plw->_firstch[row] = plft;
		if (plw->_lastch[row] < prig)
		    plw->_lastch[row] = prig;
	    }

	}
	plw->_csbp = ocsbp;	/* restore old attribute code */

    }				/* end if border on pane     */

    return OK;			/* return to caller          */
}				/* end function ecactpc      */

/*
 * NAME:                ecactpd (local)
 *
 * FUNCTION:            find matching character and change to new
 *
 * EXECUTION ENVIRONMENT:
 *
 *   PARAMETERS:        oc - pointer to old border character
 *                      oa - pointer to old border attribute
 *                      bo - pointer to string of old box characters
 *                      ba - char attr of old box char
 *                      bn - pointer to string of new box characters
 *                      na - character attribute code value
 *
 *   INITIAL CONDITIONS bo and bn must point to strings of equal
 *                      leng with chars in corresponding positions
 *                      having equivalent functions.
 *
 *   FINAL CONDITIONS:
 *     NORMAL:          if *oc occurs in bo, the corresponding char
 *                      from bn will be used to replace *oc and *oa
 *                      will be set to na.
 *
 *     ABNORMAL:
 *
 * EXTERNAL REFERENCES: none
 *
 * RETURNED VALUES:     OK - always
 */

int     ecactpd (oc, oa, ob, ba, nb, na)/* char, box attributes      */
	NLSCHAR *oc;            /* ptr to current character  */
ATTR   *oa;                     /* ptr to current attribute  */
char   *ob;			/* ptr to string of old box  */
				/* - characters              */
ATTR   ba;                      /* old attribute code        */
char   *nb;			/* ptr to string of new box  */
				/* - characters              */
ATTR   na;                      /* new attribute code        */

{				/* begin ecactpd             */
				/* search old box for match  */
    while (*oc != (NLSCHAR)(*ob) && *ob != '\0')
	ob++, nb++;		/* update both box string ptr */

    if (*ob && *oa == ba) {	/* if match found            */
	*oc = (NLSCHAR)(*nb);	/* change character and      */
	*oa = na;		/* - attribute code          */
    }
}				/* end ecactpd               */
