static char sccsid[] = "@(#)29	1.1  src/bos/usr/ccs/lib/libprint/move.c, libprint, bos411, 9428A410j 9/30/89 15:38:42";
/*
 * COMPONENT_NAME: libplot
 *
 * FUNCTIONS: move
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


extern  float   obotx;
extern  float   oboty;
extern  float   botx;
extern  float   boty;
extern  float   scalex;
extern  float   scaley;
extern  int     printhpos;
extern  int     printvpos;

move(newx, newy)
int     newx;
int     newy;
{
	printhpos = (newx-obotx)*scalex + botx;
	printvpos = (newy-oboty)*scaley + boty;
}
