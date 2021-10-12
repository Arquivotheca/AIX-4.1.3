static char sccsid[] = "@(#)44	1.20  src/bos/usr/bin/wc/wc.c, cmdfiles, bos41J, 9521A_all 5/23/95 18:12:32";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: wc
 *
 * ORIGINS: 3, 18, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 Release 1.0
 *
 * static char rcsid[] = "RCSfile: wc.c,v Revision: 2.5  (OSF) Date: 90/10/07 17:25:43"
 */
#define _ILS_MACROS
#include	<stdio.h>
#include	<locale.h>
#include 	"wc_msg.h"
#include	<stdlib.h>
#include	<errno.h>
#include        <ctype.h>
#define MAXPARMS 2
#define MSGSTR(Num,Str) catgets(catd,MS_WC,Num,Str)

#define NOTASCIISPACE(c)  ( ((c) > 32) || ((c)<9) || (((c)>13) && ((c)<32)) )

static nl_catd catd;

static unsigned char buf[BUFSIZ+1];     /* buffer for file/stdin reads */

static wchar_t wc;		    /* used for wide char processing */
static int mb_cur_max; 	    /* the maximum no. of bytes for multibyte
			       characters in the current locale */
static int cflag = 0;              /* 'c' parameter */
static int kflag = 0;              /* 'k' parameter */
static int lflag = 0;              /* 'l' parameter */
static int mflag = 0;              /* 'm' parameter */
static int wflag = 0;              /* 'w' parameter */
/*
 * NAME: wc [-c | -m] [-lw] [File ...]
 *       wc -k [-c [-lw]] [File ...]
 *                                                                    
 * FUNCTION: Counts the number of lines, words and characters in a file.
 *           -k       counts actual characters, not bytes.
 *           -c       counts bytes only. 
 *           -l       counts lines only.
 *           -m       counts characters only.
 *           -w       counts words only.
 */  
main(argc,argv)
int argc;
char **argv;
{
	unsigned char *p1, *p2;
	unsigned char *curp, *cure;	/* current and endpointers in the 
					   buffer */
	int bytesread;			/* no. of bytes read from disk */
	int mbcnt;			/* no. of bytes in a character */
	int fd = 0;			/* file descripter, default stdin */
	int leftover;			/* no. of bytes left over in the  
					   buffer that need remaining bytes
					   for conversion to wide char */
	int filect = 0;			/* file count */
	long wordct;			/* word count */
	long twordct;			/* total word count */
	long linect;			/* line count */
	long tlinect;			/* total line count */
	long bytect;			/* byte count */
	long tbytect;			/* total byte count */
	long charct;			/* actual character count */
	long tcharct;			/* total character count */
	int c;
	int token;
	int status = 0;
	int invalidmb;

	int mbcodeset;			/* Boolean flag to indicate if 
					   current code set is a multibyte
					   code set. */
	char * loc_val;
	int asciipath;

	(void) setlocale(LC_ALL,"");

	catd = catopen(MF_WC, NL_CAT_LOCALE);
					
	while ((c=getopt(argc,argv,"cklmw")) != EOF) {
		switch (c) {
		      case 'k':
				kflag++;
				break;
		      case 'm':		
				mflag++;
				break;
		      case 'l':	
				lflag++;
				break;
		      case 'w':
				wflag++;
				break;
		      case 'c':
				cflag++;
				break;
		      default:
				usage();
		}
	}
	if (cflag && mflag) usage();

	mb_cur_max = MB_CUR_MAX;	/* max no. of bytes in a multibyte
					   char for current locale. */
	if (mb_cur_max > 1) mbcodeset = 1;
	else mbcodeset = 0;

        /* if we are in C or POSIX locales, we can make various */
        /* assumptions to speed things up.                      */
        loc_val =  setlocale(LC_CTYPE,NULL);
        if ( strcmp(loc_val,"C")==0 || ((*loc_val=='P')
					 && (strcmp(loc_val,"POSIX")==0)) )
                asciipath=1;
        else
                asciipath=0;

	do {
		if(optind < argc) {
		    if ((fd=open(argv[optind],0)) < 0) {
			fprintf(stderr,MSGSTR(CANTOPEN,   
				"wc: cannot open %s\n"), argv[optind]); 
			status = 2;
			continue;
		    }
	            else filect++;	
		}
		p1 = p2 = buf;
		linect = 0;
		wordct = 0;
		bytect = 0;
		token = 0;
		charct = 0;
		invalidmb = 0;

  /* count lines, words and characters but check options before printing */

		if (mbcodeset) { 		/* I18N support */
		    leftover = 0;
		    for(;;) {
			bytesread = read(fd, buf+leftover, BUFSIZ-leftover);
			if (bytesread <= 0) 
				break;
			buf[leftover+bytesread] = '\0';  /* protect partial
							    reads */
			bytect += bytesread;
			curp = buf;
			cure = buf + bytesread + leftover;
			leftover = 0;
			for (;curp < cure;) {
			    /* convert to wide character */
			    mbcnt = mbtowc(&wc, curp, mb_cur_max);
                            if (mbcnt <= 0) {
                                if (mbcnt == 0) {
                                     /* null string, handle exception */
                                     mbcnt = 1;
                                }
                                else if (cure-curp >= mb_cur_max) {
                                        /* invalid multibyte.  handle */
                                        /* one byte at a time */
					fprintf(stderr,"wc: %s: %s\n",
						argv[optind],strerror(errno));
					invalidmb = 1;		
					filect--;
					break;
				}
				else { /* needs more data from next read */
                                        leftover = cure - curp;
                                        strncpy(buf, curp, leftover);
                                        break;
				}	
			    }
			    curp += mbcnt;
			    charct++;

			    /* count real characters */
			    if (!iswspace(wc)) {
				if (!token) {
				 	wordct++;
					token++;
				}
				continue;
			    }
			    token = 0;
			    if (wc == L'\n') linect++;
			} 		/* end wide char conversion */
			if (invalidmb) break;
		    }  			/* end read more bytes */
		    if (invalidmb) continue;
		}
		else { 				/* single byte support */
		    for(;;) {
			if(p1 >= p2) {
				p1 = buf;
				c = read(fd, p1, BUFSIZ);
				if(c <= 0)
					break;
				bytect += c;
				charct += c;
				p2 = p1 + c;
			}
			c = *p1++;

			if ((asciipath && NOTASCIISPACE(c)) ||
				(!asciipath && !isspace(c))) {
				if (!token) {
					wordct++;
					token++;
				}
				continue;
			}
			token = 0;
			if (c == '\n')
				linect++;
		    }      /* end for loop */
		}          /* end dual path for I18N */

		/* print lines, words, chars/bytes */

		wcp(linect, wordct, kflag || mflag ? charct : bytect);

		if (filect) 
		    printf(" %s\n", argv[optind]);
		else
			 fputc('\n',stdout);

		close(fd);
		tlinect += linect;
		twordct += wordct;
		tbytect += bytect;
		tcharct += charct;
	} while(++optind<argc);		/* process next file */

	if(filect >= 2) {		/* print totals for multiple files */
		wcp(tlinect, twordct, kflag || mflag ? tcharct : tbytect);
		printf(" %s", MSGSTR(TOTAL,"total\n"));   
	}
	exit(status);
}

/*
 * NAME: wcp
 *                                                                    
 * FUNCTION: check options then print out the requested numbers.
 *           Note:  the pointer used by wcp is external to wcp.      
 */  
static wcp(linect, wordct, cbct)
long linect, wordct, cbct;
{
	if (lflag + wflag + cflag + mflag == 0)   /* default mode "lwc" */
		printf(" %7ld %7ld %7ld", linect, wordct, cbct);
	else {
		if (lflag)
	   		printf(" %7ld", linect);
		if (wflag)
	   		printf(" %7ld", wordct);
		if (cflag || mflag)
	   		printf(" %7ld", cbct);
	}
}

static usage()
{
	fprintf(stderr,MSGSTR(USAGE,"Usage: wc [-c | -m][-lw] [File ...]\n\
       wc -k [-c [-lw]] [File ...]\n"));
	exit(2);
}

