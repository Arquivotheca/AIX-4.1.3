static char sccsid[] = "@(#)63	1.1  src/bos/usr/lib/nls/loc/imk/kfep/KIMutil.c, libkr, bos411, 9428A410j 5/25/92 15:41:07";
/*
 * COMPONENT_NAME :	(libKR) - AIX Input Method
 *
 * FUNCTIONS :		KIMutil.c
 *
 * ORIGINS :		27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp.  1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/******************************************************************
 *
 *  Component:    Korean IM FEP
 *
 *  Module:       KIMutil.c
 *
 *  Description:  Korean Input Method Fep Conveience Routines.
 *
 *  Functions:    itoa()
 *                reverse()
 *		  atks()
 *
 *  History:      5/20/90  Initial Creation.
 *
 ******************************************************************/

#include <string.h>

/************************************/
/*				    */
/*  Converts a integer into string. */
/*				    */
/************************************/
void	itoa(int n, unsigned char s[], int *len) 
{
register int	i=0, sign;
extern void	reverse();

	if ((sign=n) < 0) n = -n;
	do {	/* generate digits in reverse order */
		s[i++] = n % 10 + '0';
	} while((n/=10)>0);	/* delete it */
	if (sign < 0) s[i++] = '-';
	s[i] = '\0';
	*len = i;
	reverse(s);
}

/**********************/
/*		      */
/*  Reverse a string. */
/*		      */
/**********************/
void 	reverse(unsigned char s[]) {
register int c, i, j;
for(i=0, j=strlen(s)-1; i <j; i++, j--) {
	c = s[i];
	s[i] = s[j];
	s[j] = c;
}
}

/********************************************/
/*				            */
/*  Converts a ascii string into KS string. */
/*				            */
/********************************************/
static unsigned short int ks_tbl[]= {
	0xa3b0, 0xa3b1, 0xa3b2, 0xa3b3, 0xa3b4,
	0xa3b5, 0xa3b6, 0xa3b7, 0xa3b8, 0xa3b9
};
unsigned char   *atks(unsigned char s[], int len) {
register int 	i=0, cp;
extern unsigned char *malloc();
unsigned char	*d;

	d = (unsigned char*)malloc(len*2);
	while(i < len) {
	  if (s[i] >= '0' && s[i] <= '9') {
		cp = s[i]-'0';
		memcpy((unsigned char*)(d+(i*2)), 
		(unsigned char*)&ks_tbl[cp], 2);
		i++;
	  }
	  else return (NULL);
	}
	return (unsigned char*)d;
}
