static char sccsid[] = "@(#)34	1.1  src/bos/usr/ccs/lib/libprint/setdot.c, libprint, bos411, 9428A410j 9/30/89 15:39:18";
/*
 * COMPONENT_NAME: libplot
 *
 * FUNCTIONS: setdot
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


#define SIZEACR     480
#define SIZEDOWN    (72 * 6)
#define SIZEBITS    (SIZEACR / 16 * SIZEDOWN)
extern  short   bitarea[SIZEBITS];
extern  int     printempty;


setdot (x, y)
register    int     x;
register    int     y;
{
	if (x < 0 || x >= SIZEACR || y < 0 || y >= SIZEDOWN)
	    return;

	printempty = 0;
	y = SIZEDOWN - 1 - y;
	bitarea[y * (SIZEACR / 16) + (x >> 4)] |= 1 << (15 - (x & 017));
	}
