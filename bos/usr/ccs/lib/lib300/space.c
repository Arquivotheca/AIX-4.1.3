static char sccsid[] = "@(#)50	1.1  src/bos/usr/ccs/lib/lib300/space.c, libt300, bos411, 9428A410j 9/30/89 15:42:23";
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

# include "con.h"
extern float deltx;
extern float delty;
space(x0,y0,x1,y1){
	botx = -2047.;
	boty = -2047.;
	obotx = x0;
	oboty = y0;
	scalex = deltx/(x1-x0);
	scaley = delty/(y1-y0);
}
