static char sccsid[] = "@(#)22	1.1  src/bos/usr/ccs/lib/libprint/cont.c, libprint, bos411, 9428A410j 9/30/89 15:37:50";
/*
 * COMPONENT_NAME: libplot
 *
 * FUNCTIONS: cont
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


extern  float    obotx;
extern  float    oboty;
extern  float    botx;
extern  float    boty;
extern  float    scalex;
extern  float    scaley;
extern  int      scaleflag;
extern  int      printhpos;
extern  int      printvpos;


cont (newx, newy)
int     newx;
int     newy;
{
	register    int     xdistance;      /* length of x in screen units */
	register    int     ydistance;      /* length of y in screen units */
	register    int     xsign;
	register    int     ysign;
	register    int     count;
	register    int     numpts;
	register    int     x;
	register    int     y;

	newx = (newx-obotx)*scalex + botx;
	newy = (newy-oboty)*scaley + boty;

	xdistance = newx - printhpos;
	ydistance = newy - printvpos;
	xsign = xdistance < 0 ? -1 : 1;
	ysign = ydistance < 0 ? -1 : 1;

	numpts = xdistance * xsign;
	if (ydistance * ysign > numpts)
	    numpts = ydistance * ysign;

	setdot (printhpos, printvpos);
	for (count = 1; count <= numpts; count++) {
	    x = printhpos + ((long) xdistance * count) / numpts;
	    y = printvpos + ((long) ydistance * count) / numpts;
	    setdot (x, y);
	    }

	printhpos = newx;
	printvpos = newy;
}
