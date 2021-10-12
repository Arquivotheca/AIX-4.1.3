static char sccsid[] = "@(#)74	1.3  src/bos/usr/ccs/lib/libbsd/mult.c, libbsd, bos411, 9428A410j 7/26/91 14:15:04";
/*
 * COMPONENT_NAME: (LIBBSD)  Berkeley Compatibility Library
 *
 * FUNCTIONS: mult, tradd
 *
 * ORIGINS: 26 27 10
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp.  1989
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
 * NAME: mult
 * FUNCTION: assigns c the product of a and b.
 */
mult(a,b,c)
struct mint *a,*b,*c;
{	
	struct mint x,y,z;
	int sign = 1;

	x.val=a->val;
	y.val=b->val;
	z.len=0;

	if(a->len<0)
	{	x.len= -a->len;
		sign= -sign;
	}
	else	x.len=a->len;
	if(b->len<0)
	{	y.len= -b->len;
		sign= -sign;
	}
	else	y.len=b->len;

	if(x.len<y.len) m_mult(&y,&x,&z);
	else m_mult(&x,&y,&z);

	if(sign<0) z.len= -z.len;
	move(&z,c);
	xfree(&z);

	return;
}


#define S2 x=a->val[j];
#define S3 x=x*b->val[i-j];
#define S4 tradd(&carry,&sum,x);
#define S5 c->val[i]=sum.yy.low&077777;
#define S6 sum.xx=sum.xx>>15;
#define S7 sum.yy.high=carry;

m_mult(a,b,c)
struct mint *a,*b,*c;
{	
	long x;
	union {long xx; struct half yy;} sum;
	int carry;
	int i,j;

	c->val=xalloc(a->len+b->len,"m_mult");
	sum.xx=0;
	for(i=0;i<b->len;i++)
	{	carry=0;
		for(j=0;j<i+1;j++)
		{	S2
			S3
			S4
		}
		S5
		S6
		S7
	}
	for(;i<a->len;i++)
	{	carry=0;
		for(j=i-b->len+1;j<i+1;j++)
		{	S2
			S3
			S4
		}
		S5
		S6
		S7
	}
	for(;i<a->len+b->len;i++)
	{	carry=0;
		for(j=i-b->len+1;j<a->len;j++)
		{	S2
			S3
			S4
		}
		S5
		S6
		S7
	}
	if(c->val[i-1]!=0)
		c->len=a->len+b->len;
	else	c->len=a->len+b->len-1;
	return;
}

tradd(a,b,c)
long c;
int *a;
union g {long xx; struct half yy;} *b;
{
	b->xx= b->xx+c;
	if(b->yy.high&0100000)
	{	b->yy.high= b->yy.high&077777;
		*a += 1;
	}
	return;
}
