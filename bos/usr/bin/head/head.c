static char sccsid[] = "@(#)51	1.16  src/bos/usr/bin/head/head.c, cmdscan, bos412, 9446B 11/15/94 20:12:18";
/*
 * COMPONENT_NAME: (CMDSCAN) commands that scan files
 *
 * FUNCTIONS:
 *
 * ORIGINS: 3, 26, 27, 71
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
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 * OSF/1 1.1
 */

/*
 *                                                                    
 * FUNCTION:	Displays the first "count" of lines of each of the 
 *		specified files, or of the standard input.  If "count"
 *		is omitted, it defaults to 10.
 *		
 *		Valid usages:
 *			 head [-count | -n lines | -c bytes] [file...]
 *
 *		Note: The -n lines is synonymous to -count, the -n
 *			is a new flag defined by Posix 1003.2.
 *
 */  


#define _ILS_MACROS

#include <stdio.h>
#include <locale.h>
#include <errno.h>
#include <nl_types.h>
#include <ctype.h>
#include "head_msg.h"

#define         MSGSTR(num,str) catgets(catd,MS_HEAD,num,str)  /*MSG*/

static nl_catd         catd;

static void copyout();
static int  getnum();
static void  bytesout();
static void usage();

main(argc, argv)
int argc;
char *argv[];
{
	int linecnt = 10;	/* Default number of file lines to print */
	int numfiles = --argc;  /* Number of files to be printed         */
	char *name;             /* Name of file to be printed            */
	int morethanone = 0;    /* Flag to indicate more than one file   */
	int cflag = 0;		/* Output in bytes			 */
	int endopt = 0;		/* No more options 			 */
	int status = 0;

	(void) setlocale(LC_ALL,"");
	catd = catopen(MF_HEAD, NL_CAT_LOCALE);
	argv++;
	if (argv[0][0] == '+')
		usage();
	/* retrieve "count" if present */
	while (argc > 0 && argv[0][0] == '-' && !endopt) {
		switch (argv[0][1]) {
		   case '?':
			usage();
		   case '-':
			if (argv[0][2] == '\0') {
			    endopt++;
			    argv++;
		       	    argc--;
			    numfiles--;
			} else {
		            fprintf(stderr, MSGSTR(BADNUM,
			            "Badly formed number\n"));
			    usage();
			}		
			break;
		   case 'n':
		        if (argv[0][2] == '\0') {		
			/* handling space separation */
				linecnt = getnum(argv[1]);
				argc-=2;
				argv+=2;
				numfiles-=2;
			} else  {
			    	linecnt = getnum(argv[0] + 2);
			        argc-=1; 
			        argv+=1;
			   	numfiles-=1;
			}
			endopt++;
			break;
		   case 'c':
			cflag++;
			/* handling space separation */
			if (argv[0][2] == '\0') {
			    	linecnt = getnum(argv[1]);
			    	argc-=2;
			    	argv+=2;
			    	numfiles-=2;
			} else {
			    	linecnt = getnum(argv[0] +2);
		       	   	 argc -=1;
			    	argv +=1;
		     	    	numfiles-=1;
			}
		       	endopt++;
		        break;
		   default:
			if (isdigit(argv[0][1])) {
		            	linecnt = getnum(argv[0] + 1);
		 	    	argc--;
			    	argv++;
		            	numfiles--;
		            	endopt++;
			    	break;
			} else
			    	usage();
	       }
	}
    if (argc > 0 && strcmp(argv[0],"--") == 0) {
		argc--;
		argv++;
		numfiles--;
    }
	do {
		/* If filename present, substitute file for stdin */
		if (argc > 0) {
			close(0);
			if (freopen(argv[0], "r", stdin) == NULL) {
				perror(argv[0]);
				status = 1;
				argc--;
				argv++;
				continue;
			} 
			name = argv[0];
			argc--;
			argv++;
		} else /* use stdin */
			name = 0;
		/*
		 * If more than one file to be printed, 
	   	 * put space between files and print name 
                 */
		if (morethanone)
			putchar('\n');
		morethanone++;
		if (numfiles > 1 && name)
			printf("==> %s <==\n", name);
		/* Print out specified number of lines */
		if (cflag)
			bytesout(linecnt);
		else
			copyout(linecnt);
		fflush(stdout);
	} while (argc > 0);
	exit(status);
}

/*
 * Print "count" lines of file 
 */
static void
copyout(cnt)
int cnt;
{
	char lbuf[BUFSIZ];

	while (cnt > 0 && fgets(lbuf,(int)sizeof(lbuf), stdin) != 0) {
		fwrite(lbuf,(int)sizeof(char),strlen(lbuf),stdout);  
		cnt--;
	}
	fflush(stdout);
}

static void
bytesout(cnt)
int cnt;
{
	int buf,tmp;
	while (cnt > 0 && (buf = getc(stdin)) != EOF ) {
		putchar(buf);
		cnt--;
		tmp = buf;
	}
	fflush(stdout);
	if (tmp != '\n')
		fputc('\n',stdout);
}
/*
 * Retrieve from command line number of lines of each file to be printed 
 * Return value: number of lines. Invoke strtol() for better error checking.
 */
static int
getnum(cp)
char *cp;
{
	int i;
	errno = 0;

 	i = strtol(cp, &cp, 0);
        if ( !cp || *cp || errno || (i<0) ) {
                /*
                 * If the string was anything but a numeric sequence, or
                 * there was any conversion error, or the string was negative
                 */

		fprintf(stderr, MSGSTR(BADNUM, "Badly formed number\n"));
		usage();
	}
	return (i);
}

static void
usage()
{
	fprintf(stderr, MSGSTR(USAGE, "usage: head [-count | -c number | -n number] [file...]\n"));
	exit(1);
}
