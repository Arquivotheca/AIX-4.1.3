static char sccsid[] = "@(#)75	1.1  src/bos/usr/ccs/lib/lib450/line.c, libt450, bos411, 9428A410j 9/30/89 15:47:01";
/*
 * COMPONENT_NAME: libplot
 *
 * FUNCTIONS: line, cont, iline
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

#include "con.h"
line(x0,y0,x1,y1){
	iline(xconv(xsc(x0)),yconv(ysc(y0)),xconv(xsc(x1)),yconv(ysc(y1)));
		return;
}
cont(x0,y0){
	iline(xnow,ynow,xconv(xsc(x0)),yconv(ysc(y0)));
	return;
}
iline(cx0,cy0,cx1,cy1){
	int maxp,tt,j,np;
	char chx,chy;
	float xd,yd;
	float dist2(),sqrt();
		movep(cx0,cy0);
		maxp = sqrt(dist2(cx0,cy0,cx1,cy1))/2.;
		xd = cx1-cx0;
		yd = cy1-cy0;
		if(xd >= 0)chx = RIGHT;
		else chx = LEFT;
		if(yd >= 0)chy = UP;
		else chy = DOWN;
		if(maxp == 0){
			xd=0;
			yd=0;
		}
		else{
			xd /=maxp;
			yd /=maxp;
		}
		inplot();
		for (tt=0; tt<=maxp; tt++){
			j= cx0+xd*tt-xnow;
			xnow += j;
			j = abval(j);
			while(j-- > 0)spew(chx);
			j = cy0+yd*tt-ynow;
			ynow += j;
			j = abval(j);
			while(j-- > 0)spew(chy);
			spew ('.');
		}
		outplot();
		return;
}
