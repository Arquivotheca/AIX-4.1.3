static char sccsid[] = "@(#)22	1.6  src/bos/usr/ccs/lib/libcur/ecbpls.c, libcur, bos411, 9428A410j 5/14/91 17:01:04";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: ecbpls
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

/*
 * NAME:                ecbpls
 *
 * FUNCTION:            Create and initialize a new panel structure
 *
 * EXECUTION ENVIRONMENT:
 *
 *   PARAMETERS:        pr - panel size in rows
 *                      pc - panel size in columns
 *                      or - origin of panel on display upper left
 *                           corner row coordinate
 *                      oc - origin of panel on display upper left
 *                           corner column coordinate
 *                      tt - pointer to panel title string, will be
 *                           shown in top border, centered. Should be
 *                           null if no border or no title wanted.
 *                      dt - division type code, indicates the dim.
 *                           along which this panel is to be divided
 *                      bd - boundry definition code, boundry for
 *                           panel, yes or no.
 *                      pn - pointer to first PANE which defines the
 *                           divisions of this panel. If null panel
 *                           not divided and all actions apply only
 *                           to the panel structures.
 *
 *   INITIAL CONDITIONS While all parameters should be as defined,
 *                      they are not checked and are not used until a
 *                      callto ecdfpl. Thus an application may modify
 *                      valuesput into this structure until that time
 *
 *   FINAL CONDITIONS:
 *     NORMAL:          new PANEL structure created and address ret
 *
 *     ABNORMAL:        ERR returned and no structure created.
 *
 * EXTERNAL REFERENCES: calloc
 *
 * RETURNED VALUES:     address of panel structure if no error, minus
 *                      1 if unable to allocate storage.
 */

PANEL   *ecbpls(pr, pc, or, oc, tt, dt, bd, pn)
short int   pr;			/* rows in panel	      */
short int   pc;			/* columns in panel	      */
short int   or;			/* upper row on display       */
short int   oc;			/* left column on display     */
char    *tt;                    /* title string pointer       */
char    dt;			/* division direction code    */
char    bd;			/* borders for panel code     */
PANE    *pn;                    /* first division pane        */

{				/* begin function	      */

    PANEL  *plptr;              /* new panel struct address   */
#ifdef OLD_STYLE	/* Apr/23/91 by Tz */
    char   *calloc ();		/* storage allocate routine   */
#endif

				/* executable code           */

    plptr = (PANEL *) calloc(1, sizeof(PANEL));
				/* get storage for panel      */

    if (plptr == NULL) {	/* if storage not obtained    */
	return(PANEL *)(-1);	/* return error 	      */
    }

    else {
	plptr->p_depth = pr;	/* fill in the structure      */
	plptr->p_width = pc;
	plptr->orow = or;
	plptr->ocol = oc;
	plptr->title = tt;
	plptr->divty = dt;
	plptr->bordr = bd;
	plptr->dpane = pn;

	return plptr;		/* return structure addr      */
    }

}				/* end of function	      */
