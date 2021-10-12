static char sccsid[] = "@(#)51	1.2  src/bos/usr/ccs/bin/structure/0.extr.c, cmdprog, bos411, 9428A410j 6/15/90 22:53:45";
/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 26; 27
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
struct lablist	{long labelt;  struct lablist *nxtlab; };
struct lablist *endlab, *errlab, *reflab, *linelabs, *newlab;

int nameline;			/* line number of function/subroutine st., if any */
int stflag;		/* determines whether at beginning or middle of block of straight line code */



int   nlabs, lswnum, swptr, flag,
	 counter, p1, p3, begline, endline, r1,r2, endcom;
long begchar, endchar, comchar;


char *pred, *inc, *prerw, *postrw, *exp, *stcode;

#define maxdo	20	/* max nesting of do loops */
long dostack[maxdo];		/* labels of do nodes */
int doloc[maxdo];		/* loc of do node */
int doptr;


struct list *FMTLST;		/* list of FMTVX's generated */
struct list *ENTLST;		/* list of STLNVX nodes corresponding to entry statements */
long rtnbeg;	/* number of chars up to beginning of current routine */
