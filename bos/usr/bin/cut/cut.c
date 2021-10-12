static char sccsid[] = "@(#)01	1.19  src/bos/usr/bin/cut/cut.c, cmdfiles, bos412, 9446C 11/14/94 16:48:10";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: cut
 *
 * ORIGINS: 3, 18, 27, 71
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
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * static char rcsid[] = "RCSfile: cut.c,v  Revision: 1.4  (OSF) Date: 90/10/07 16:22:52"
 */
#define _ILS_MACROS
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <locale.h>
#include <limits.h>
#include "cut_msg.h"

static nl_catd	catd;
#define MSGSTR(Num, Str) catgets(catd,MS_CUT, Num, Str)

#define MAX_ARGS 512

static int mb_cur_max;			/* the maximum no. of bytes for multibyte
				   characters in the current locale */

static int status = 0;			/* exit status code */

static int low[MAX_ARGS];		/* holds low ranges */ 
static int high[MAX_ARGS];		/* holds high ranges */ 

static unsigned char *linebuf;		/* pointer to line buffer for reads/writes */
static wchar_t *wcbuf;	 		/* pointer to wide char buffer for output */  

/*
 * NAME: cut -c list [file1 ...]
 *       cut -f list [-d char] [-s] [file1 ...]
 *       cut -b list [-n] [file1 ...]
 *                                                                    
 * FUNCTION: Writes out selected fields from each line of a file.
 *                                                                    
 * NOTES:
 *     Must specify either the -c, -b  or -f flag.
 *      -c list   specifies the character postions to be written by cut.
 *      -f list   specifies the field postions to be written by cut.
 *         -d char  used with -f list, char denotes a delimiter between fields.
 *         -s       used with -f list, suppresses lines with out delimiters.
 *      -b list   specifies the selected bytes to be written by cut.
 *         -n       used with -b list, does not allow characters to be split.
 */  

main(argc, argv)
int argc; char **argv;
{
	char *arg;
	int mbcodeset;			/* multibyte code set indicator */
	int mbcnt;			/* no. of bytes in a mb char */

	int cflag = 0;			/* 'c' parameter indicator */
	int fflag = 0;			/* 'f' parameter indicator */
	int bflag = 0;			/* 'b' parameter indicator */
	int nflag = 0;			/* 'n' parameter indicator */
	int dflag = 0;			/* 'd' parameter indicator */
	int sflag = 0;			/* 's' parameter indicator */

	int range_ck = 0;		/* range list indicator */	
	int rindex = 0;			/* no. of range elements */
	int start;			/* starting index for searching
					   range arrays */

	int endflag;			/* EOF indicator */
	int keep;			/* keep indicator */	
	int count;			/* byte/char/field counter */
	int bytect;			/* byte counter */
	int rnum;			/* range number to check
					     For '-c': it's the char position
					     For '-f': it's the field position
					     For '-b': it's the position of
					           the last byte of a char */


	wchar_t wdel = '\t';    	/* wide char field delim, default tab */
	int del = '\t';    		/* field delimiter, default tab */
	int c, i, option;
	int temp = 0;
	int delim = 0;			/* field delimiter variable */
	unsigned char *list;		/* list pointer */

	int len;
	int offset, poffset;
	int alloc_len;			/* Size of line buffer (in char) and wide-char
					 * buffer (in wchar_t) from malloc/realloc calls.
					 */			
	unsigned char *lptr;		/* current line buffer pointer */
	unsigned char *plptr;		/* previous line buffer pointer */
	wchar_t *wptr;			/* current wide char pointer */
	wchar_t *pwptr;			/* previous wide char pointer */

	FILE *fp;			/* input file pointer */

	(void) setlocale(LC_ALL,"");
	catd = catopen(MF_CUT, NL_CAT_LOCALE);

	mb_cur_max = MB_CUR_MAX;
	if (mb_cur_max > 1)
		mbcodeset = 1;			/* multibyte locale */
	else
		mbcodeset = 0;			/* single byte locale */
 
/*
 * Get parameters and arguments
 */

	while ((c=getopt(argc,argv,"c:f:b:nd:s")) != EOF) {
		switch((char) c) {
			case 'c': cflag++;
				  list = optarg;
				  option = c;	
				  break;
			case 'f': fflag++;
				  list = optarg;
				  option = c;	
				  break;
			case 'b': bflag++;
				  list = optarg;
				  option = c;	
				  break;
			case 'n': nflag++;
				  break;	
			case 'd': dflag++;
				  if (mbcodeset) {
				      mbcnt = mbtowc(&wdel, optarg, mb_cur_max);
				      if (mbcnt < 1) {
				      	 fprintf(stderr, MSGSTR(NODELIMITER, "cut: invalid delimiter\n"));
					 exit(2);
				      }	
				  }
				  else del = *optarg;
				  break;
			case 's': sflag++;
				  break;
			default:  usage();
		}
	}   /* while c is next valid option */

/*
 * Ensure that only one of the following parameters,
 * 'c', 'f', or 'b' was specified
 */

	if ((cflag && fflag) ||
	    (cflag && bflag) ||
	    (fflag && bflag)) 
		usage();

/*
 * Ensure that either the 'c', 'f', or 'b' parameter was specified
 */

	if (!cflag && !fflag && !bflag) 
		usage();

/*
 * Ensure that any optional parameters entered belong with the entered mandatory parameter
 */

	if ((sflag && !fflag) || (dflag && !fflag) || (nflag && !bflag))
		   usage();

/*
 * Determine selected characters, fields, or bytes.
 */

	do {
		c = *list;
		switch((char) c) {
			case '-': range_ck++;
				  if (rindex >= MAX_ARGS) {
					fprintf (stderr,MSGSTR(MAXARGS,
					    "cut: too many list arguments for %c option\n"),option);
					exit(2);
				  }
				  if (temp == 0)     	 /* hyphen or zero specified for low range */
					low[rindex] = 1;
				  else {
					low[rindex] = temp;
					temp = 0;
				  }
				  break;
			case '\0':
			case '\t':
			case ' ':
			case ',': if (rindex >= MAX_ARGS) {
					fprintf(stderr,MSGSTR(MAXARGS,
					   "cut: too many list arguments for %c option\n"),option);
					exit(2);
				  }
				  if (range_ck) {
					if (temp == 0)		/* hyphen or zero specified for high range */
					    high[rindex] = -1;  /* -1 used to denote "end of line" */
					else
					    high[rindex] = temp;
					if (high[rindex] != -1 && high[rindex] < low[rindex]) {
					    fprintf(stderr,MSGSTR(BADLIST,
				               "cut: bad list for %c option\n"),option);
					    exit(2);
					}
				  }
				  else
				       high[rindex] = low[rindex] = temp;
				  rindex++;
				  range_ck = temp = 0;
				  break;
			default:  if (!isdigit(c)) {
					fprintf(stderr,MSGSTR(BADLIST,
				           "cut: bad list for %c option\n"),option);
					exit(2);
				  }
				  temp = 10*temp + c - '0';
			}
	} while (*list++ != '\0');	/* end of list */ 

/*
 * if multiple ranges, sort the high and
 * low ranges in ascending order
 */

if (rindex > 1)
    sort(rindex);
							
/*
 * allocate storage for reads
 */

alloc_len = LINE_MAX + 1;
linebuf = (unsigned char *) malloc ((size_t) alloc_len);
if (linebuf == NULL) 
     nomem();

/*
 * perform cut processing 
 */

do {	/* for all input files */
	if (optind < argc) {				/* file name arguments specified */
	    arg = argv[optind];
	    if (*arg == '-' && *++arg == 0)  		/* ck for hyphen entered for stdin */
		fp = stdin;
	    else
		fp = fopen(argv[optind], "r");	
	}
	else fp = stdin;				/* no file name arguments, read from stdin */
  
	if (fp == NULL) {
		fprintf(stderr,
		MSGSTR(CANTOPEN, "cut: cannot open : %s\n"), 
		argv[optind]);
		status = 1;
		continue;		/* process next file */
	}

	endflag = 0;
	if (!mbcodeset || (bflag && !nflag)) {
	    
	    /* single byte support: single byte locale or */
	    /* multibyte locale with byte processing only */
	     
	    do { /* for all lines in a file */
		bytect = count = start = rnum = 0;
		lptr = plptr = linebuf - 1;
		do {	/* for all characters in the line */
			c = fgetc(fp);
			if (c == EOF) {
			    endflag = 1;
			    break;
			}

			/* check if require additional storage */
			/* if so, calculate new line maximum and */
		        /* offsets for current and previous pointers */

			if (++bytect >= alloc_len) {
			    alloc_len += LINE_MAX;
			    offset = lptr - linebuf;
			    poffset = plptr - linebuf;
			    linebuf = (unsigned char *) realloc(linebuf, (size_t) alloc_len);	
			    if (linebuf == NULL) 
				nomem();
			    lptr = linebuf + offset;
			    plptr = linebuf + poffset;
			}

			if (!(fflag && delim && c == '\n'))
			    *++lptr = c;

			/* break on newlines for '-c', '-b' options */
			/* or for fields with no delimiters */

			if (c == '\n' && (!fflag || count == 0)) 
			    break;

			if (!fflag)
				rnum++;		/* increment position */

			 /* check range */
			if (c == del || c == '\n' || cflag || bflag) {
			    keep = 0;	
			    count++;
			    if (fflag) rnum = count;
			    if (rnum >= low[start]) 
			        for (i=start; i<rindex; i++) {
				     if (rnum >= low[i]) {
				         if (rnum <= high[i] || high[i] == -1) {
						keep = 1;
						start = i;
					 	break;
				         }
				     }	
			        }
			        if (keep)
			            plptr = lptr;
			        else
			            lptr = plptr;
			}
			delim = (c == del);
		} while (c != '\n');	/* end character processing */
		
		if (!endflag) {
			/* line suppression check */
		    if (!sflag || count > 0) {
			if (lptr==linebuf-1)	/* if no data was kept */
				lptr++;		/* advance ptr to valid space */
		        *lptr = '\0';		/* overwrite newline */
			puts(linebuf);	/* output line */
		    }
		}
	    } while (!endflag);		/* end line processing */
	}				/* end single byte support */
	else {	

	    /* multibyte code set support */
	    /* Allocate wide-char buffer large enough to cover the current
	     * length of the multibyte line buffer: one wide-char per byte.
	     */
	    wcbuf = (wchar_t *) malloc ((size_t) (alloc_len * sizeof(wchar_t)));
	    if (wcbuf == NULL)
		    nomem();

	    do { /* for all lines of a file */
		lptr = fgets(linebuf, alloc_len, fp);
		if (lptr == NULL)		/* EOF/unrecoverable error */
		    break;
		len = (int)strlen(linebuf);

		/* check if require additional storage to read line */
		/* if so, realloc original storage plus additional space */

		while (*(linebuf + len - 1) != '\n') {
		    alloc_len += LINE_MAX;
		    linebuf = (unsigned char *) realloc (linebuf, (size_t) alloc_len);	
		    if (linebuf == NULL)
		    	nomem();

		    /* Reallocate wide-char buffer large enough to cover the new
		     * current length of the line buffer: one wide char per byte.
		     */
		    wcbuf = (wchar_t *) realloc (wcbuf, (size_t)(alloc_len * sizeof(wchar_t)));
		    if (wcbuf == NULL)
		        nomem();

		    lptr = fgets((linebuf + len), LINE_MAX, fp);
		    if (lptr == NULL) {
			endflag = 1;
			break;
		    }	
		    len = (int)strlen(linebuf);	
		}
		if (endflag)
		    break;	

		lptr = linebuf;
		wptr = pwptr = wcbuf-1;
		mbcnt = count = start = rnum = 0;

		do {   /* for all characters in line */
			lptr += mbcnt;		/* increment ptr to next char */
			if (!(fflag && delim && (*lptr == '\n')))
				wptr++;		/* point to next output area */
			mbcnt = mbtowc(wptr,lptr,mb_cur_max);
			if (mbcnt <= 0) 	/* invalid character */
			    mbcnt = 1;		/* treat as one byte */	

			/* break on newlines for '-c', '-b' options */
			/* or for fields with no delimiters */

			if (*wptr == '\n' && (!fflag || count == 0)) 
			    break;

			/* for '-c' and '-b' , increment position counters */

			if (cflag)
				rnum++;
			else if (bflag)
				rnum += mbcnt;

			      /*  Fields will be checked upon occurrence  */
			      /*  of a delimiter or a new line            */
			
			delim = (*wptr == wdel);
			/* Begin list search */
			if (*wptr == wdel || *wptr == '\n' || cflag || bflag) {
			    keep = 0;	
			    count++;
			    if (fflag) rnum = count;
			    if (rnum >= low[start]) 
			        for (i=start; i<rindex; i++) {
				    if (rnum >= low[i]) {
				        if (rnum <= high[i] || high[i] == -1) {
					    keep = 1;
					    start = i;
					    break;
			 	        }
			   	    }
			        }
			    if (keep)
			        pwptr = wptr;
			    else
			        wptr = pwptr;
                        }
	
		} while (*lptr != '\n');        /* end process chars in line */
		
		if (!endflag) {
		    /* line suppression check */
		    if (!sflag || count > 0) {
			if (wptr==wcbuf-1)	/* if no data was kept */
				wptr++;		/* advance ptr to valid space */
		        *wptr = '\0';		/* overwrite newline */
		        putws (wcbuf);
		    }
		}
	    } while (!endflag);		/* process all lines in file, break on EOF */
	}				/* end multibyte code set support */	

fclose(fp);

} while(++optind < argc);	/* process next file */

exit(status);
}

/*
 * NAME: usage
 *
 * FUNCTION: print usage message and exit.
 *
 */
static
usage()
{
	fprintf(stderr, MSGSTR(USAGE,
	  "Usage: cut {-b <list> [-n] | -c <list> | -f <list> [-d <char>] [-s]} file ...\n"));
	exit(1);
}

/*
 * NAME: nomem
 *
 * FUNCTION: print no memory message and exit.
 *
 */
static
nomem()
{
	fprintf(stderr,MSGSTR(NOMEMORY, "cut: out of memory\n"));
	exit(2);
}

/*
 * NAME: sort
 *                                                                    
 * FUNCTION: Sorts the high and low range arrays.
 */  
static
sort(rindex)
int rindex;
{
	int i, j, temp;
	for (i=0; i<(rindex-1); i++)
		for (j=(i+1); j<rindex; j++) {
			if (low[i] > low[j]) {
				temp = low[i];    /* swap lows */
				low[i] = low[j];
				low[j] = temp;
				temp = high[i];   /* swap highs */
				high[i] = high[j];
				high[j] = temp;
			}
		}
}
