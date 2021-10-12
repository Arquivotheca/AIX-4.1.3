static char sccsid[] = "@(#)53	1.23  src/bos/usr/bin/look/look.c, cmdscan, bos411, 9431A411a 8/4/94 15:10:16";
/*
 * COMPONENT_NAME: (CMDSCAN) commands that scan files
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
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 * 
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 * 
 * OSF/1 1.1
 * 
 * $RCSfile: look.c,v $ $Revision: 2.8.2.2 $ (OSF) $Date: 92/02/18 20:26:54 $
 */

#define _ILS_MACROS

#include <stdio.h>
#include <locale.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

#include "look_msg.h" 

nl_catd         catd;
#define MSGSTR(n,s) catgets(catd, MS_LOOK, n, s) 

static void canon();

FILE *dfile;

char *filenam  = "/usr/share/dict/words";

int fold=0;
int dict=0;
int tab;
#define		BIG_BUF		250
char entry[BIG_BUF];
char word [BIG_BUF];
char key  [BIG_BUF];
size_t mb_cur_max;
wchar_t wc;

main(argc,argv)
char **argv;
int argc;
{
	register int c;
	long top,bot,mid;
	int flag;

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_LOOK, NL_CAT_LOCALE);
	mb_cur_max = MB_CUR_MAX;

	while((flag = getopt(argc, argv, "dft:")) != -1) {
	    switch( flag ) {
	      case 'd':
		dict++;
		break;

	      case 'f':
		fold++;
		break;

	      case 't':
		if (mb_cur_max > 1) {
		    c=mbtowc(&wc, optarg, mb_cur_max);
		    tab = wc;
		} else {
		    tab = *optarg;
		    c=1;
		}
		break;

	      case '?':
		usage();
	    }
	}
	
	argc -= optind-1;
	argv += optind-1;

	if(argc<=1)
		usage();	

	if(argc==2) {		/* No 'file' given */
		dict++;
		fold++;
	}
	else
		filenam = argv[2];
	dfile = fopen(filenam,"r");
	if(dfile==NULL) {
		fprintf(stderr,MSGSTR(CANTOPEN, "look: can't open %s\n"),filenam); /*MSG*/
		exit(2);
	}
	canon(argv[1],key);
	bot = 0;
	fseek(dfile,0L,SEEK_END);
	top = ftell(dfile);
	for(;;) {
		mid = (top+bot)/2;
		fseek(dfile,mid,SEEK_SET);
		do {
			c = getc(dfile);
			mid++;
		} while(c!=EOF && c!='\n');
		if(!getword(entry))
			break;
		canon(entry,word);
		switch(compare(key,word)) {
		case -2:
		case -1:
		case 0:
			if(top<=mid)
				break;
			top = mid;
			continue;
		case 1:
		case 2:
			bot = mid;
			continue;
		}
		break;
	}
	fseek(dfile,bot,SEEK_SET);
	while(ftell(dfile)<top) {
		if(!getword(entry))
			return;
		canon(entry,word);
		switch(compare(key,word)) {
		case -2:
			return;
		case -1:
		case 0:
			puts(entry);
			break;
		case 1:
		case 2:
			continue;
		}
		break;
	}
	while(getword(entry)) {
		canon(entry,word);
		switch(compare(key,word)) {
		case -1:
		case 0:
			puts(entry);
			continue;
		case 2:	/* Handle cases like: key=abc lines=(ABC, abc, abc).
			 * Without this case the program never sees "abc" 
			 */
			continue;
		}
		break;
	}
	exit(0);
}

usage() 
{
	fprintf(stderr,MSGSTR(USAGE, "Usage: look [-d -f] String [File...]\n")); /*MSG*/
	exit(2);
}
/*
 *  NAME:  compare
 *
 *  FUNCTION:  string compare two strings.
 *	      
 *  RETURN VALUE:  	 0   - strings are equal
 *			 1   - t < s
 *			-1   - s < t
 *			-2   - t < s
 *			 2   - s < t
 */

compare(s,t)
register char *s,*t;
{
	int r;

	for (;;) {
		if (!*s && !*t)
			return(0);
		if (!*s)
			return(-1);
		if (!*t)
			return(1);
		r = strncmp(s, t, strlen(s));
		if (r < 0)
			return(-2);
		else if (r > 0)
			return(2);
		else /* r == 0 */
			return(0);
	}
}

/*
 *  NAME:  	getword
 *
 *  FUNCTION:  	Read up until a new line character or EOF is found from
 *		file "dfile".  Characters read in are placed in memory
 *		pointed to by the first parameter.
 *
 *  RETURN VALUE: 	0 is end-of-file
 *			1 a string has been read in
 */

getword(w)
char *w;
{
	register int c;
	int len=BIG_BUF;
	for(;;) {
		c = getc(dfile);
		if(c==EOF)
			return(0);
		if(c=='\n')
			break;
		if (--len == 0) {
			printf (MSGSTR(TOOLONG,"Line too long.  Truncating\n"));
			break;
		}
			
		*w++ = c;
	}
	*w = 0;
	return(1);
}

/*
 *  NAME:  canon
 *
 *  FUNCTION:	Take a given string and set the pointer (new)
 *		to the first word (null terminates it).
 *
 *  RETURN VALUE:  None
 *
 */

static void
canon(old,new)
char *old,*new;
{
	int len=BIG_BUF;
	int mbcnt;
	wchar_t wc;
	int c;

	if (mb_cur_max > 1) {
		for(;;) {
			if ((mbcnt = mbtowc(&wc, old, mb_cur_max)) == -1) {
				*new++ = *old++;
				len--;
				continue;
			}
			if(wc=='\0'||wc==tab) {
				*new = '\0';
				break;
			}
			if ((len-=mbcnt) <= 0) {
				printf (MSGSTR(WORDFILE,"Wordfile line too long. Truncating\n"));
				*new = '\0';
				break;
			}
			if(dict) {
	
				if(!iswalnum(wc)) {
					old+=mbcnt;
					continue;
				}
			}
			if(fold) {
				if(iswupper(wc))
					wctomb(new, (wchar_t) towlower(wc));
				else
					strncpy(new, old, mbcnt);
			} else
				strncpy(new, old, mbcnt);
			old+=mbcnt;
			new+=mbcnt;
		}
	} else {
		for(;;) {
			*new = c = *old++;

			if(c==0||c==tab) {
				*new = '\0';
				break;
			}
			if (--len <= 0) {
				printf (MSGSTR(WORDFILE,"Wordfile line too long. Truncating\n"));
				*new = '\0';
				break;
			}
			if(dict) {
				if(!isalnum(c))
					continue;
			}
			if(fold) {
				if(isupper(c))
					*new = tolower(c);
			}
			new++;
		}
	}
}

