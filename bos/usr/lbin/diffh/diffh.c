static char sccsid[] = "@(#)49  1.10  src/bos/usr/lbin/diffh/diffh.c, cmdfiles, bos411, 9428A410j 4/11/94 19:29:38";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: diff -h
 *
 * ORIGINS: 3, 18, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
/*
 * @OSF_COPYRIGHT@
static char rcsid[] = "RCSfile: diffh.c,v  Revision: 2.4  (OSF) Date: 90/10/07 16:24:51 ";
 */
/*
 *  diffh is diff -h
 *  does a fast, half-hearted job.  It works only when changed stretches are
 *  short and well separated, but does work on files of unlimited length.
 */                                                                   
/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */
#define _ILS_MACROS
#include <stdio.h>
#include <locale.h>
#include <stdlib.h>
#include <unistd.h>

#include <ctype.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <dirent.h>

#include <nl_types.h>
#include "diff_msg.h"
nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_DIFF,Num,Str)

#define C 3
#define RANGE 30
#define LEN 255
#define INF 16384

char *text[2][RANGE];
long lineno[2] = {1, 1};	/*no. of 1st stored line in each file*/
int ntext[2];		/*number of stored lines in each*/
long n0,n1;		/*scan pointer in each*/
int bflag;
int debug = 0;
FILE *file[2];

	/* return pointer to line n of file f*/
char *getl(f,n)
long n;
{
	register char *t;
	register delta, nt;
again:
	delta = n - lineno[f];
	nt = ntext[f];
	if(delta<0)
		progerr("1");
	if(delta<nt)
		return(text[f][delta]);
	if(delta>nt)
		progerr("2");
	if(nt>=RANGE)
		progerr("3");
	if(feof(file[f]))
		return(NULL);
	t = text[f][nt];
	if(t==0) {
		t = text[f][nt] = malloc((size_t)(LEN+1));
		if(t==NULL)
			if(hardsynch())
				goto again;
			else
				progerr("5");
	}
	t = fgets(t,LEN,file[f]);
	if(t!=NULL)
		ntext[f]++;
	return(t);
}

	/*remove thru line n of file f from storage*/
clrl(f,n)
long n;
{
	register i,j;
	j = n-lineno[f]+1;
	for(i=0;i+j<ntext[f];i++)
		movstr(text[f][i+j],text[f][i]);
	lineno[f] = n+1;
	ntext[f] -= j;
}

movstr(s,t)
register char *s, *t;
{
	while(*t++= *s++)
		continue;
}

main(argc,argv)
char **argv;
{
	char *s0,*s1;
	FILE *dopen();
	register int status = 0;
	extern char *optarg;
	extern int optind, optopt, opterr;
	int c;
	unsigned char ucc; /* c disguised as char */
	int badopt;

	(void) setlocale(LC_ALL,"");

	catd = catopen(MF_DIFF, NL_CAT_LOCALE);
	
	/* Usage as separate command would be "diffh [-b] File1 File2". But there
	 * is no separate documented diffh command. It is invoked by  diffreg  for
	 * a "diff -h" command. All error messages for illegal options or option
	 * combinations are assumed to be issued by  diff  or  diffreg.  diffh 
	 * ignores all options except  -b . Option processing by getopt() appears
	 * to be superfluous but is necessary to detect  -b  option because diffreg
	 * passes the  diff  argument list instead of just  bflag . Usage message
	 * provided in case a user finds it and tries to invoke it.
	 */
	badopt = 0;
	while ((c = getopt(argc,argv,"bc:C:D:efhilnrsS:tw")) != -1) {
		ucc = (unsigned char)c;
		switch (ucc) {
		case 'b':
			bflag++;
			break;
			/* diff options illegal in combination with -h */
		case 'c':
		case 'C':
		case 'D':
		case 'e':
		case 'f':
		case 'n':
			badopt++;
			break;
			/* diff options ignored in combination with -h */
		case 'h':
		case 'i':
		case 'l':
		case 'r':
		case 's':
		case 'S':
		case 't':
		case 'w':
			break;
			/* Illegal options for diff */
		default:
			badopt++;
			break;
		} /* switch (ucc) */
	} /* while ((c=getopt...*/
	if (optind++ == argc || argc>optind && argv[optind+1] == NULL) {
		file[0] = dopen(argv[optind-1],argv[optind]);
		file[1] = dopen(argv[optind],argv[optind-1]);
	} else {
		fprintf(stderr,MSGSTR(EARGS2,"diffh: must have 2 file arguments\n"));
		badopt++;
	}
	if (badopt) {
		fprintf(stderr,MSGSTR(DIFFHUSE, "Usage: diffh [-b] File1 File2\n"));
		exit(2);
	}
	

	for(;;) {
		s0 = getl(0,++n0);
		s1 = getl(1,++n1);
		if(s0==NULL||s1==NULL)
			break;
		if(cmp(s0,s1)!=0) {
			if(!easysynch()&&!hardsynch())
				progerr("5");
			status = 1;
		} else {
			clrl(0,n0);
			clrl(1,n1);
		}
	}
	if(s0==NULL&&s1==NULL)
		exit(status);
	if(s0==NULL)
		output(-1,INF);
	if(s1==NULL)
		output(INF,-1);
	exit(1);
}

	/* synch on C successive matches*/
easysynch()
{
	int i,j;
	register k,m;
	char *s0,*s1;
	for(i=j=1;i<RANGE&&j<RANGE;i++,j++) {
		s0 = getl(0,n0+i);
		if(s0==NULL)
			return(output(INF,INF));
		for(k=C-1;k<j;k++) {
			for(m=0;m<C;m++)
				if(cmp(getl(0,n0+i-m),
					getl(1,n1+k-m))!=0)
					goto cont1;
			return(output(i-C,k-C));
cont1:			;
		}
		s1 = getl(1,n1+j);
		if(s1==NULL)
			return(output(INF,INF));
		for(k=C-1;k<=i;k++) {
			for(m=0;m<C;m++)
				if(cmp(getl(0,n0+k-m),
					getl(1,n1+j-m))!=0)
					goto cont2;
			return(output(k-C,j-C));
cont2:			;
		}
	}
	return(0);
}

output(a,b)
{
	register i;
	char *s;
	if(a<0)
		change(n0-1,0,n1,b,"a");
	else if(b<0)
		change(n0,a,n1-1,0,"d");
	else
		change(n0,a,n1,b,"c");
	for(i=0;i<=a;i++) {
		s = getl(0,n0+i);
		if(s==NULL)
			break;
		printf("< %s",s);
		clrl(0,n0+i);
	}
	n0 += i-1;
	if(a>=0&&b>=0)
		printf("---\n");
	for(i=0;i<=b;i++) {
		s = getl(1,n1+i);
		if(s==NULL)
			break;
		printf("> %s",s);
		clrl(1,n1+i);
	}
	n1 += i-1;
	return(1);
}

change(a,b,c,d,s)
long a,c;
char *s;
{
	range(a,b);
	printf("%s",s);
	range(c,d);
	printf("\n");
}

range(a,b)
long a;
{
	if(b==INF)
		printf("%ld,$",a);
	else if(b==0)
		printf("%ld",a);
	else
		printf("%ld,%ld",a,a+b);
}

cmp(s,t)
char *s,*t;
{
	if(debug)
		printf("%s:%s\n",s,t);
	for(;;){
		if (bflag && isspace((int)*s) && isspace((int)*t)) {
				/* Skip corresponding blocks of 1 or more spaces */
			s++;
			while(isspace((int)*s))
				s++;
			t++;
			while(isspace((int)*t))
				t++;
		}
		if(*s!=*t||*s==0)
			break;
		s++;
		t++;
	}
	return(*s-*t);
}

FILE *dopen(f1,f2)
char *f1,*f2;
{
	FILE *f;
	char b[100],*bptr,*eptr;
	struct stat statbuf;
	if(cmp(f1,"-")==0)
		if(cmp(f2,"-")==0)
			error(MSGSTR(ESTDINS,"can't do - -"),"");
		else
			return(stdin);
	if(stat(f1,&statbuf)==-1) {
		fprintf(stderr,MSGSTR(EACCESS2,"diffh: Can't access %s\n"),f1);
		exit(2);
	}
	if((statbuf.st_mode&S_IFMT)==S_IFDIR) {
		for(bptr=b;*bptr= *f1++;bptr++) ;
		*bptr++ = '/';
		for(eptr=f2;*eptr;eptr++)
			if(*eptr=='/'&&eptr[1]!=0&&eptr[1]!='/')
				f2 = eptr+1;
		while(*bptr++= *f2++) ;
		f1 = b;
	}
	f = fopen(f1,"r");
	if(f==NULL) {
		fprintf(stderr,MSGSTR(EOPEN3,"diffh: cannot open %s\n"),f1);
		exit(2);
	}
	return(f);
}


progerr(s)
char *s;
{
	error(MSGSTR(EPROGERR,"program error "),s);
}

error(s,t)
char *s,*t;
{
	fprintf(stderr,"diffh: %s%s\n",s,t);
	exit(2);
}

	/*stub for resychronization beyond limits of text buf*/
hardsynch()
{
	change(n0,INF,n1,INF,"c");
	printf(MSGSTR(ECHANGE,"---change record omitted\n"));
	error(MSGSTR(ERESYNC,"diffh: can't resynchronize"),"");
	return(0);
}
