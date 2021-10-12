static char sccsid[] = "@(#)35	1.1  src/bos/usr/ccs/lib/libprint/space.c, libprint, bos411, 9428A410j 9/30/89 15:39:26";
/*
 * COMPONENT_NAME: libplot
 *
 * FUNCTIONS: space
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

extern float botx;
extern float boty;
extern float obotx;
extern float oboty;
extern float scalex;
extern float scaley;
extern int scaleflag;


space(x0,y0,x1,y1){
	botx = 0.;
	boty = 0.;
	obotx = x0;
	oboty = y0;
	if(scaleflag)
		return;
	scalex = (double)(SIZEACR)/(x1-x0);
	scaley = (double)(SIZEDOWN)/(y1-y0);
}
