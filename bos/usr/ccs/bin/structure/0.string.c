static char sccsid[] = "@(#)55	1.5  src/bos/usr/ccs/bin/structure/0.string.c, cmdprog, bos411, 9428A410j 3/9/94 13:18:41";
/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: classmatch, concat, copychars, copycs, find, slength, str_copy,
	      str_eq
 *
 * ORIGINS: 26 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include "def.h"
#include "1.defs.h"

str_copy(s,ptr,length)	/* copy s at ptr, return length of s */
char *s, *ptr;
int length;
	{int i;
	for (i = 0; i < length; i++)
		{
		ptr[i] = s[i];
		if (ptr[i] == '\0')
			return(i + 1);
		}
	error("", "", "");
	fprintf(stderr, MSGSTR(STGTOOLNG,
		"error string too long to be copied at given address:\n%s\n"),s);
	exit(1);
	}


find(s,ar,size)
char *s,*ar[];
int size;
	{
	int i;
	for (i=0; i < size; i++)
		{if (str_eq(s, ar[i])) return(i);}
	return(-1);
	}


str_eq(s,t)
char s[],t[];
	{int j;
	for (j = 0; s[j] == t[j]; j++)
		{if (s[j] == '\0') return(1);}
	return(0);
	}


classmatch(c,i)
char c;
int i;
	{switch(i)
		{case _digit:
			if ('0' <= c && c <= '9')  return(1);
			else return(0);

		case _letter:
			if ( ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z'))
				return(1);
			else return(0);

		case _diglet:  return(classmatch(c,_digit)||classmatch(c,_letter) );

		case _arith:
			if (050 <= c && c<= 057)  return(1);
			else return(0);
		case _nl:
			return(c=='\n');
		case _other:
			return(1);
		}
	}


copychars(cbeg,target,n)		/* copy n chars from cbeg to target */
char *cbeg, *target;
int n;
	{
	int i;
	for (i = 0; i < n; i++)
		target[i] = cbeg[i];
	}



copycs(cbeg,target,n)			/* copy n chars from cbeg to target, add '\0' */
char *cbeg, *target;
int n;
	{
	copychars(cbeg,target,n);
	target[n] = '\0';
	}


slength(s)			/* return number of chars in s, not counting '\0' */
char *s;
	{
	int i;
	if (!s) return(-1);
	for (i = 0; s[i] != '\0'; i++);
	return(i);
	}


concat(x,y)			/* allocate space, return xy */
char *x, *y;
	{
	char *temp;
	int i,j;
	i = slength(x);
	j = slength(y);
	temp = (char *)galloc(i + j + 1);
	sprintf(temp,"%s",x);
	sprintf(&temp[i],"%s",y);
	return((int)temp);
	}
