static char sccsid[] = "@(#)75	1.3  src/bos/usr/ccs/lib/libbsd/pow.c, libbsd, bos411, 9428A410j 7/26/91 14:15:19";
/*
 * COMPONENT_NAME: (LIBBSD)  Berkeley Compatibility Library
 *
 * FUNCTIONS: pow, rpow
 *
 * ORIGINS: 26 27 10
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
 * NAME: pow
 * FUNCTION: calculates in c the value of a raised to power b.
 * The result is reduced modulo m.
 */
pow(a,b,m,c)
MINT *a,*b,*m,*c;
{	
	int i,j,n;
	MINT x,y,*z;

	x.len=y.len=0;

	/* initialize temporary return value */ 
	z = itom(1);

	for(j=0;j<b->len;j++)
	{	n=b->val[b->len-j-1];
		for(i=0;i<15;i++)
		{	mult(z,z,&x);
			mdiv(&x,m,&y,z);
			if((n=n<<1)&0100000)
			{	mult(a,z,&x);
				mdiv(&x,m,&y,z);
			}
		}
	}

	move(z, c);
	xfree(z);	/* free contents of struct */
	shfree(z);	/* free struct itself */

	xfree(&x);
	xfree(&y);
	return;
}

/*
 * NAME: rpow
 * FUNCTION: calculates in c the value of a raised to the power n.
 */
rpow(a,n,c)
MINT *a,*c;
{	
	MINT x,y;
	int i;

	/* initialize exponent */
	x.len=1;
	x.val=xalloc(1,"rpow1");
	*x.val=n;

	/* initialize modulus which must be big enough to contain the result */
	y.len=n*a->len+4;
	y.val=xalloc(y.len,"rpow2");
	for(i=0;i<y.len;i++) y.val[i]=0;
	y.val[y.len-1]=010000;

	xfree(c);
	pow(a,&x,&y,c);
	xfree(&x);
	xfree(&y);
	return;
}
