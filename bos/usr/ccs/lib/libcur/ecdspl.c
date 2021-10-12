static char sccsid[] = "@(#)67	1.6  src/bos/usr/ccs/lib/libcur/ecdspl.c, libcur, bos411, 9428A410j 6/16/90 01:38:24";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: ecdspl, ecdspln
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
 * NAME:                ecdspl
 *
 * FUNCTION =           Release all structures associated with the
 *                      specified panel (pspaces, panes, panel)
 *
 * EXECUTION ENVIRONMENT:
 *
 *   PARAMETERS:        pl - Panel pointer
 *
 *   INITIAL CONDITIONS pl must be a pointer to a valid panel stuct
 *
 *   FINAL CONDITIONS:
 *     NORMAL:          The panel and associated pane structures
 *                      along with all window and view structures
 *                      will be released.
 *
 *     ABNORMAL:
 *
 * EXTERNAL REFERENCES: ecrlpl, cfree
 *
 * RETURNED VALUES:     OK - always
 */

int     ecdspl (pl)		/* begin ecdspl - destroy panel */
	PANEL   *pl;            /* pointer to panel to destroy  */

{
    ecrlpl(pl);			/* release windows/views etc    */
    ecdspln(pl->dpane);		/* release pane structures      */
    cfree(pl);			/* release the panel structure  */
    return OK;			/* return to caller             */
}				/* end ecdspl                   */

/*
 * NAME:                ecdspln
 *
 * FUNCTION:            Release all pane structures from argument
 *                      down in the pane tree
 *
 * EXECUTION ENVIRONMENT:
 *
 *   PARAMETERS:        pn - Pane  pointer
 *
 *   INITIAL CONDITIONS pn must be a pointer to a valid pane  stuct
 *                      all associated windows, views and extra
 *                      p-space blocks must have been released
 *
 *   FINAL CONDITIONS:
 *     NORMAL:          The pane and all linked panes will be
 *                      released.
 *
 *     ABNORMAL:
 *
 * EXTERNAL REFERENCES: cfree
 *
 * RETURNED VALUES:     OK - always
 *
*/

int     ecdspln (pn)		/* release pane structures      */
	PANE    *pn;            /* pointer to pane to be dropped */

{

    PANE    *tmp;               /* work pointer                 */

    while (pn != NULL) {	/* continue until end of links  */
	if (pn->divd != NULL)	/* if there are divisions       */
	    ecdspln(pn->divd);	/* - release them first         */

	tmp = pn->divs;		/* get link to neighbor pane    */
	cfree(pn);		/* release pane structure       */
	pn = tmp;		/* set ptr to next pane if any  */
    }				/* end - while not null         */

    return OK;			/* return to caller             */

}				/* end ecdspln                  */
