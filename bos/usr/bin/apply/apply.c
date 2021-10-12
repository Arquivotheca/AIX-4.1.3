static char sccsid[] = "@(#)62	1.13  src/bos/usr/bin/apply/apply.c, cmdsh, bos41B, 412_41B_sync 12/15/94 12:11:18";
/*
 * COMPONENT_NAME: (CMDSH) Bourne shell and related commands
 *
 * FUNCTIONS:
 *
 * ORIGINS: 26, 27, 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.1
 *
 */
/*
 * NAME:  apply - apply a command to a set of arguments
 *
 * FUNCTION: 
 *      apply [-ac] [-n] command args ...
 *      -ac    changes '%' to be the character of your choice c.
 *      -n     number of arguments to be passed to the commmand.
 * NOTE:
 *	apply echo * == ls
 *	apply -2 cmp A1 B1 A2 B2   compares A's with B's
 *	apply "ln %1 /usr/fred/dir" *  duplicates a directory
 *        If your are having problems getting apply to work quote the command.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <sys/errno.h>

#include <ctype.h>
#include "apply_msg.h" 
static nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_APPLY,n,s) 

void usage();
static char	*cmdp;
#define	NCHARS ARG_MAX	

static wchar_t	cmd[NCHARS];
static wchar_t	blank = L' ';

static long	defargs=1;
#define	DEFARGCHAR	'%'

static wchar_t	argchar=DEFARGCHAR;

static int	nchars;

main(argc, argv)
int argc;
char *argv[];
{
	register int n;

	(void) setlocale (LC_ALL, "");

	catd = catopen(MF_APPLY,NL_CAT_LOCALE);


	while(argc>1 && argv[1][0]=='-'){
		if(argv[1][1]=='a'){
			register int rc;
			rc = mbtowc(&argchar,&argv[1][2],MB_CUR_MAX);
			if (rc < 0) {
				fprintf(stderr, MSGSTR(INVMB,
					 "apply: incomplete or invalid multibyte character string encountered.\n"));
				exit(1);
			}
			if(argchar=='\0')
				argchar=DEFARGCHAR;
		} 
		else { 
			char *end;

			errno = 0;
			defargs = strtol(&argv[1][1], &end, 10);
			if (errno) {
				fprintf(stderr, "apply: %s: %s\n",
					&argv[1][1], strerror(errno));
				exit(1);
			}
			if (*end != NULL)
				usage();
			if(defargs < 0) {
				fprintf(stderr, MSGSTR(BADNUM,
					 "apply: specify a positive number\n"));	
				exit(1);
			}
		}
		--argc; ++argv;
	}
	if(argc<2)
		usage();

	argc -= 2;
	cmdp = argv[1];
	argv += 2;
	while(n=docmd(argc, argv)){
		argc -= n;
		argv += n;
	}
	exit(0);
}

/*
 * NAME: addc
 *
 * FUNCTION: add character to command
 */
static wchar_t addc(c)
char **c;
{
	wchar_t wc;
	int mbcnt;

	if(nchars >= NCHARS){
		fprintf(stderr, MSGSTR(TOOLONG, "apply: command too long\n"));
		exit(1);
	}
	mbcnt = ((mbcnt = mbtowc(&wc, *c, MB_CUR_MAX)) > 0) ? mbcnt : 1;
	nchars += mbcnt;
	*c += mbcnt;
	return (wc);
}

/*
 * NAME: addarg
 *
 * FUNCTION: check length of s and copy to t.
 */
static wchar_t *addarg(s, t)
	char *s;
	register wchar_t *t;
{
	while(*t = addc(&s))
		*t++;
	return(t);
}

/*
 * NAME: docmd
 *
 * FUNCTION:  get command from argv
 */
static docmd(argc, argv)
	char *argv[];
{
	char *p;
	wchar_t *q;
	register max, i;
	int gotit;
	if(argc<=0)
		return(0);
	nchars = 0;
	max = 0;
	gotit = 0;
	p = cmdp;
	q = cmd;
	while(*q = addc(&p)){
		if(*q++!=argchar || *p<'1' || '9'<*p)
			continue;
		if((i= *p++-'1') > max)
			max = i;
		if(i>=argc){
	Toofew:
			fprintf(stderr, MSGSTR(TOOFEWARGS, "apply: expecting argument(s) after `%s'\n"), argv[argc-1]); /*MSG*/
			exit(1);
		}
		q = addarg(argv[i], q-1);
		gotit++;
	}
	if(defargs!=0 && gotit==0){
		if(defargs>argc)
			goto Toofew;
		for(i=0; i<defargs; i++){
			if (nchars >= NCHARS) {
				fprintf(stderr, MSGSTR(TOOLONG,
				 "apply: command exceed %d bytes long\n"), NCHARS);
				exit(1);
			}
			/* null byte is replaced by a blank. nchars has been 
                           incremented previously. */ 
			*q++ = blank;
			q = addarg(argv[i], q);
		}
	}
	i = eXec(cmd);
	if(i == 127){
		fprintf(stderr, MSGSTR(NOSHELL, "apply: no shell!\n")); /*MSG*/
		exit(1);
	}
	return(max==0? (defargs==0? 1 : defargs) : max+1);
}

/*
 * NAME: eXec
 * FUNCTION: execute command s
 */
static eXec(s)
wchar_t *s;
{
	char tmpbuf[NCHARS];
	int status, pid, w;
	char *shell = getenv("SHELL");

	wcstombs(tmpbuf,s,sizeof(tmpbuf));
	if ((pid = fork()) == 0) {

		execl(shell ? shell : "/usr/bin/sh", "sh", "-c", tmpbuf, 0);
		return(127);
	}
	if(pid == -1){
		fprintf(stderr, MSGSTR(NOFORK, "apply: can't fork\n")); /*MSG*/
		exit(1);
	}
	while ((w = wait(&status)) != pid && w != -1)
		;
	if (w == -1)
		status = -1;
	return(status);
}

static void
usage()
{
	fprintf(stderr, MSGSTR(USAGE,
		 "usage: apply [-aCharacter] [-Number] Command Argument...\n"));
	exit(1);
}
