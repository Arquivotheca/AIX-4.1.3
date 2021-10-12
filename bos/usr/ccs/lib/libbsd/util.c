static char sccsid[] = "@(#)76	1.7  src/bos/usr/ccs/lib/libbsd/util.c, libbsd, bos411, 9428A410j 11/10/93 15:19:09";
/*
 * COMPONENT_NAME: (LIBBSD)  Berkeley Compatibility Library
 *
 * FUNCTIONS: move, xalloc, fatal, xfree, mcan, itom, mcmp, xtoi, xtom, itox,
 *	      mtox, mfree 
 *
 * ORIGINS: 26 27 10
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * "@(#)util.c 1.2 88/05/16 4.0NFSSRC SMI; from 5.1 (Berkeley) 4/30/85";
 */

void mfree();
#ifdef lint
int xv_oid;
#endif
#include <stdio.h>
#include <stdlib.h>
#include "mp.h"
#include "libbsd_msg.h"

/*
 * NAME: move
 * FUNCTION: copies a to b
 */
move(a,b)
MINT *a,*b;
{	
	int i,j;

	xfree(b);
	b->len=a->len;
	if((i=a->len)<0) i = -i;
	if(i==0) return;
	b->val=xalloc(i,"move");
	for(j=0;j<i;j++)
		b->val[j]=a->val[j];
	return;
}

dummy(){}


/* Allocate the number of shorts given by nint.  s is a string used only
 * when debugging to indicate why this routine is being called.  Returns
 * the pointer to the allocated buffer or NULL if error.  The buffer is 
 * zeroed to maintain compatibility with the old malloc().
 */
short *xalloc(nint,s)
char *s;
{
	short *i;
	nl_catd catd;
	int n;

	/* allocate nint+2 shorts - which must be converted to a # of bytes */
	nint +=2;
	i=(short *)malloc((size_t)(sizeof(short)*(unsigned)nint));

#ifdef DBG
	if(dbg) fprintf(stderr, "%s: %o\n",s,i);
#endif
	if(i!=NULL)
	{
		for(n=0; n<nint; n++)	/* clear a # of shorts */
			*(i+n) = 0;
		return(i);
	}
	else
	{
		catd = catopen(MF_LIBBSD, NL_CAT_LOCALE);
		fatal(catgets(catd, MS_LIBBSD, M_NOSPACE,
						"xalloc: no free space"));
		catclose(catd);
		return(0);
	}
}

fatal(s)
char *s;
{
	fprintf(stderr,"%s\n",s);
	VOID fflush(stdout);
	sleep(2);
	abort();
}

xfree(c)
MINT *c;
{
#ifdef DBG
	if(dbg) fprintf(stderr, "xfree ");
#endif
	if(c->len==0) return;
	shfree(c->val);
	c->len=0;
	return;
}

mcan(a)
MINT *a;
{	
	int i,j;

	if((i=a->len)==0) return;
	else if(i<0) i= -i;
	for(j=i;j>0 && a->val[j-1]==0;j--);
	if(j==i) return;
	if(j==0)
	{	xfree(a);
		return;
	}
	if(a->len > 0) a->len=j;
	else a->len = -j;
}

/*
 * NAME: itom
 * FUNCTION:  pointer to MINT can be initialized using this function.
 */
MINT *itom(n)
{	
	MINT *a;

	a=(MINT *)xalloc(2,"itom");
	if(n>0)
	{	a->len=1;
		a->val=xalloc(1,"itom1");
		*a->val=n;
		return(a);
	}
	else if(n<0)
	{	a->len = -1;
		a->val=xalloc(1,"itom2");
		*a->val= -n;
		return(a);
	}
	else
	{	a->len=0;
		return(a);
	}
}

/*
 * NAME: mcmp
 * FUNCTION:  returns a negative, zero or positive integer value when a is
 * less than, equal to, or greater than b.
 */
mcmp(a,b)
MINT *a,*b;
{	
	MINT c;
	int res;

	if(a->len!=b->len) return(a->len-b->len);
	c.len=0;
	msub(a,b,&c);
	res=c.len;
	xfree(&c);
	return(res);
}

/*
 * Convert hex digit to binary value
 */
static int
xtoi(c)
	char c;
{
	if (c >= '0' && c <= '9') {
		return(c - '0');
	} else if (c >= 'a' && c <= 'f') {
		return(c - 'a' + 10);
	} else {
		return(-1);
	}
}



/*
 * Convert hex key to MINT key
 */
MINT *
xtom(key)
	char *key;
{	
	int digit;
	MINT *m = itom(0);
	MINT *d;
	MINT *sixteen;

	sixteen = itom(16);
	for (; *key; key++) {
		digit = xtoi(*key);
		if (digit < 0) {
			return(NULL);
		}
		d = itom(digit);
		mult(m,sixteen,m);
		madd(m,d,m);
		mfree(d);
	}
	mfree(sixteen);
	return(m);
}

static char
itox(d)
	short d;
{
	d &= 15;
	if (d < 10) {
		return('0' + d);
	} else {
		return('a' - 10 + d);
	}
}

/*
 * Convert MINT key to hex key
 */
char *
mtox(key)
	MINT *key;
{
	MINT *m = itom(0);
	MINT *zero = itom(0);
	short r;
	char *p;
	char c;
	char *s;
	char *hex;
	int size;

#	define BASEBITS	(8*sizeof(short) - 1)

	if (key->len >= 0) {
		size = key->len;
	} else {	
		size = -key->len; 
	}
	hex = malloc((size_t) ((size * BASEBITS + 3)) / 4 + 1);
	if (hex == NULL) {
		return(NULL);
	}
	move(key,m);
	p = hex;
	do {
		sdiv(m,16,m,&r);
		*p++ = itox(r);
	} while (mcmp(m,zero) != 0);	
	mfree(m);
	mfree(zero);

	*p = 0;
	for (p--, s = hex; s < p; s++, p--) {
		c = *p;
		*p = *s;
		*s = c;
	}
	return(hex);
}

/*
 * Deallocate a multiple precision integer
 */
void
mfree(a)
	MINT *a;
{
	xfree(a);
	free((void *)a);
}
