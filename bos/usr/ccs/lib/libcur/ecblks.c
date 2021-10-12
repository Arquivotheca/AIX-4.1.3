static char sccsid[] = "@(#)11	1.6  src/bos/usr/ccs/lib/libcur/ecblks.c, libcur, bos411, 9428A410j 6/16/90 01:38:01";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: ecblks
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

static
	WINDOW  *blkwin = NULL; /* address of window            */

/*
 * NAME:                ecblks
 *
 * FUNCTION:            Create a window which points to a blank
 *                      p-space and reuses memory for the data area
 *
 * EXECUTION ENVIRONMENT:
 *
 *   PARAMETERS:        none
 *
 *   INITIAL CONDITIONS none
 *
 *   FINAL CONDITIONS:
 *     NORMAL:          new p-space created and address saved
 *
 *     ABNORMAL:        no p-space and null returned
 *
 * EXTERNAL REFERENCES: newwin
 *
 * RETURNED VALUES:     pointer to p-space created or NULL if error
 */

WINDOW  *ecblks() {             /* begin function definition    */

    int     i;			/* index variable               */

    if (blkwin == NULL) {	/* for first call               */
	if ((blkwin = newwin(0, 0, 0, 0)) != NULL) {
				/* if new window created ok     */
	    for (i = 1; i < blkwin->_maxy; i++) {
				/* step through rows of window  */
		cfree(blkwin->_y[i]);/* release data row             */
		cfree(blkwin->_a[i]);/* release attr row             */
		blkwin->_y[i] = blkwin->_y[0];
				/* point data to row zero       */
		blkwin->_a[i] = blkwin->_a[0];
				/* point attr to row zero       */
	    }			/* end loop thru rows           */
	}			/* end - if newwin ok           */
    }				/* end - if pointer null        */
    return blkwin;		/* return pointer to blank win  */
}				/* end function ecblks          */
