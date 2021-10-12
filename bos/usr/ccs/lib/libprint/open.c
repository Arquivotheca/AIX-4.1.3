static char sccsid[] = "@(#)30	1.1  src/bos/usr/ccs/lib/libprint/open.c, libprint, bos411, 9428A410j 9/30/89 15:38:49";
/*
 * COMPONENT_NAME: libplot
 *
 * FUNCTIONS: openvt, openpl
 *
 * ORIGINS: 4,10,27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */


#include    <stdio.h>
#include    <lprio.h>
#define     SIZEACR     480
#define     SIZEDOWN    (72 * 6)
#define     SIZEBITS    (SIZEACR / 16 * SIZEDOWN)
extern      short   bitarea[SIZEBITS];
extern      int     printempty;
extern      struct  lprmode     lprmode;
extern      int     lpmode;

openpl ()
{
	register    short   *iptr;

	printempty = 1;
	ioctl (fileno (stdout), LPRGETV, &lprmode);
	lpmode = lprmode.modes;
	lprmode.modes = PLOT;
	ioctl (fileno (stdout), LPRSETV, &lprmode);

	fprintf (stdout, "\033A\010\0332");
	for (iptr = &bitarea[SIZEBITS - 1]; iptr >= bitarea; iptr--)
	    *iptr = 0;
	}

openvt () {
	openpl ();
}
