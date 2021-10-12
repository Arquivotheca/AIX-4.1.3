static char sccsid[] = "@(#)48	1.5  src/bos/usr/ccs/bin/structure/0.alloc.c, cmdprog, bos411, 9428A410j 3/9/94 13:17:32";
/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: balloc, bfree, challoc, chfree, error, faterr, freegraf, galloc,
	      morespace, reuse, strerr, talloc
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

int routbeg;
extern int debug;
struct coreblk	{struct coreblk *nxtblk;
			int blksize;
			int nxtfree;
			int *blk;
			};

long space;
challoc(n)
int n;
	{
	int i;
	i = malloc(n);
	if(i) { space += n; return(i); }
	fprintf(stderr,MSGSTR(ALLOCSPC, "alloc out of space\n")); /*MSG*/
	fprintf(stderr,MSGSTR(TOTSPAC, "total space alloc'ed = %ld\n"),space); /*MSG*/
	fprintf(stderr,MSGSTR(MOREBYTE, "%d more bytes requested\n"),n); /*MSG*/
	exit(1);
	}


chfree(p,n)
int *p,n;
	{
	ASSERT(p,chfree);
	space -= n;
	free(p);
	}


struct coreblk *tcore, *gcore;
int tblksize=12, gblksize=300;


balloc(n,p,size)		/* allocate n bytes from coreblk *p */
int n,size;		/* use specifies where called */
struct coreblk **p;
	{
	int i;
	struct coreblk *q;
	n = (n+sizeof(i)-1)/sizeof(i);	/* convert bytes to wds to ensure ints always at wd boundaries */
	for (q = *p; ; q = q->nxtblk)
		{
		if (!q)
			{
			q = (struct coreblk *) morespace(n,p,size);
			break;
			}
		if (q-> blksize - q->nxtfree >= n)  break;
		}
	i = q->nxtfree;
	q ->nxtfree += n;
	return( (int) &(q->blk)[i]);
	}

talloc(n)		/* allocate from line-by-line storage area */
int n;
	{return(balloc(n,&tcore,tblksize)); }

galloc(n)		/* allocate from graph storage area */
int n;
	{
	return(balloc(n,&gcore,gblksize));
	}

reuse(p)		/* set nxtfree so coreblk can be reused */
struct coreblk *p;
	{
	for (; p; p=p->nxtblk)  p->nxtfree = 0;  
	}

bfree(p)		/* free coreblk p */
struct coreblk *p;
	{
	if (!p) return;
	bfree(p->nxtblk);
	p->nxtblk = 0;
	free(p);
	}


morespace(n,p,size)		/* get at least n more wds for coreblk *p */
int n,size;
struct coreblk **p;
	{struct coreblk *q;
	int t,i;

	t = n<size?size:n;
	q = (struct coreblk *) malloc(i=t*sizeof(*(q->blk))+sizeof(*q));
	if(!q){
		error(MSGSTR(COLOUTSPC, ": alloc out of space"),"",""); /*MSG*/
		fprintf(stderr,MSGSTR(SPCEQ, "space = %ld\n"),space); /*MSG*/
		fprintf(stderr,MSGSTR(MOREBYTE, "%d more bytes requested\n"),n); /*MSG*/
		exit(1);
		}
	space += i;
	q->nxtblk = *p;
	*p = q;
	q -> blksize = t;
	q-> nxtfree = 0;
	q->blk = (int *)(q + 1);
	return((int) q);
	}




freegraf()
	{
	bfree(gcore);
	gcore = 0;


	}









error(mess1, mess2, mess3)
char *mess1, *mess2, *mess3;
	{
	static lastbeg;
#ifdef MSG
	char savem1[200], savem2[200], savem3[200];
	strcpy(savem1, mess1); /* save messages...*/
	strcpy(savem2, mess2);
	strcpy(savem3, mess3);
#endif MSG
	if (lastbeg != routbeg)
		{
		fprintf(stderr,MSGSTR(ROUTBEG, "routine beginning on line %d:\n"),routbeg); /*MSG*/
		lastbeg = routbeg;
		}
	if (strcmp(mess1, "")) {
#if defined(KJI) || defined(NLS)
#ifdef MSG
	NLfprintf(stderr,MSGSTR(ERRORS, "error %s %s %s\n"),savem1, savem2, savem3); /*MSG*/
#else MSG
	NLfprintf(stderr,MSGSTR(ERRORS, "error %s %s %s\n"),mess1, mess2, mess3); /*MSG*/
#endif MSG
#else KJI || NLS
#ifdef MSG
	fprintf(stderr,MSGSTR(ERRORS, "error %s %s %s\n"),savem1, savem2, savem3); /*MSG*/
#else MSG
	fprintf(stderr,MSGSTR(ERRORS, "error %s %s %s\n"),mess1, mess2, mess3); /*MSG*/
#endif MSG
#endif KJI || NLS
	}
	}


faterr(mess1, mess2, mess3)
char *mess1, *mess2, *mess3;
	{
	error(mess1, mess2, mess3);
	exit(1);
	}


strerr(mess1, mess2, mess3)
char *mess1, *mess2, *mess3;
	{
	error(MSGSTR(STRUCTERR, "struct error: "),mess1, mess2); /*MSG*/
	}
