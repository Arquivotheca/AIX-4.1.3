static char sccsid[] = "@(#)15	1.9  src/bos/usr/bin/tee/tee.c, cmdscan, bos412, 9446B 11/15/94 20:13:04";
/*
 * COMPONENT_NAME: (CMDSCAN) commands that scan files
 *
 * FUNCTIONS:
 *
 * ORIGINS: 3, 26, 27
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
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 */

/*
 *
 * tee-- pipe fitting.	Displays the  output of  a
 *	program and  copies it into a file.
 */                                                                   

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include <stdlib.h>
#include "tee_msg.h"

static nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_TEE,Num,Str) 

static int stash();
static int openf[20] = { 1 };
static int n = 1;
static int t = 0;
static int aflag;

static char in[BUFSIZ];
static char out[BUFSIZ];

main(argc,argv)
char **argv;
{
	int r,w,p;
	struct stat buf;
	int exstat = 0;
	int c;
	register int i;

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_TEE, NL_CAT_LOCALE);
	while((c=getopt(argc, argv, "ai")) != -1) {
		switch(c) {
		case 'a':		/* append mode            */
			aflag++;
			break;
		case 'i':		/* ignore the kill signal */
		case 0: 		/* "-" is Berkely feature same as "-i"
					 * it is not documented 
					 */ 
			signal(SIGINT, SIG_IGN);
			break;
		default:
			fprintf(stderr,MSGSTR(USAGE,"Usage: tee [-ai] [file ...]\n"));
			exstat++;
			exit(exstat);
		}
	}
	argc -= optind;
	argv += optind;
	fstat(1,&buf);
	t = (buf.st_mode&S_IFMT)==S_IFCHR;
	if(lseek(1,0L,SEEK_CUR)==-1&&errno==ESPIPE)
		t++;
	while(argc-->0) {
		openf[n] = open(argv[0],O_WRONLY|O_CREAT|
			(aflag?O_APPEND:O_TRUNC), 0666);
		if(openf[n++] >= 0 && stat(argv[0],&buf)>=0) {
			if((buf.st_mode&S_IFMT)==S_IFCHR)
				t++;
		} else {
			fprintf(stderr,MSGSTR(NO_FILE,"tee: cannot open %s\n"), argv[0]);
			n--;
			exstat++;
		}
		argv++;
	}
	r = w = 0;
	for(;;) {
		for(p=0;p<BUFSIZ;) {
			if(r>=w) {
				if(t>0&&p>0) break;
				w = read(0,in,BUFSIZ);
				r = 0;
				if(w<=0) {
					if (stash(p) < 0 ) 
						     exstat++;
				/*
				 * finished proccessing files, check on closing
				 * files to see if any writes fail from  files 
				 * e.g. nfs files
				 */
					for (i=0; i< n; i++) {	
						if (close(openf[i]) != 0)
				 		 	exstat++;
					}
					exit(exstat);
				}
			}
			out[p++] = in[r++];
		}
		if (stash(p) < 0) 
			exstat++;
	}
}

/*
 * NAME: stash
 *                                                                    
 * FUNCTION: copy the buffer to each of the output files.
 *
 * RETURN VALUE DESCRIPTION: It returns negative number  if any errors occur in 
 * writing, 0 if it succeeds
 */  
static int
stash(p)
{
	int k;
	int i;
	int d;
	int error=0;
	d = t ? 16 : p;
	for(i=0; i<p; i+=d)
		for(k=0;k<n;k++) 
			if (write(openf[k], out+i, d<p-i?d:p-i) == -1)
				 error--;
	return(error);
}
