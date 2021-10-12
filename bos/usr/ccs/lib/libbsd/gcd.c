static char sccsid[] = "@(#)68	1.2  src/bos/usr/ccs/lib/libbsd/gcd.c, libbsd, bos411, 9428A410j 6/16/90 00:59:47";
/*
 * COMPONENT_NAME: (LIBBSD)  Berkeley Compatibility Library
 *
 * FUNCTIONS: gcd, invert
 *
 * ORIGINS: 26 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include "mp.h"

/*
 * NAME: gcd 
 * FUNCTION:  returns the greatest common denominator of a and b in c
 */
gcd(a,b,c)
MINT *a,*b,*c;
{	
	MINT x,y,z,w;

	x.len=y.len=z.len=w.len=0;
	move(a,&x);
	move(b,&y);
	while(y.len!=0)
	{	mdiv(&x,&y,&w,&z);
		move(&y,&x);
		move(&z,&y);
	}
	move(&x,c);
	xfree(&x);
	xfree(&y);
	xfree(&z);
	xfree(&w);
	return;
}

/*
 * NAME: invert
 * FUNCTION:  computes c such that a*b mod b = 1, for  a and b relatively prime
 */
invert(a, b, c)
MINT *a, *b, *c;
{	
	MINT x, y, z, w, Anew, Aold;
	int i = 0;

	x.len = y.len = z.len = w.len = Aold.len = 0;
	Anew.len = 1;
	Anew.val = xalloc(1, "invert");
	*Anew.val = 1;
	move(b, &x);
	move(a, &y);
	while(y.len != 0)
	{	mdiv(&x, &y, &w, &z);
		move(&Anew, &x);
		mult(&w, &Anew, &Anew);
		madd(&Anew, &Aold, &Anew);
		move(&x, &Aold);
		move(&y, &x);
		move(&z, &y);
		i++;
	}
	move(&Aold, c);
	if( (i&01) == 0) msub(b, c, c);
	xfree(&x);
	xfree(&y);
	xfree(&z);
	xfree(&w);
	xfree(&Aold);
	xfree(&Anew);
}
