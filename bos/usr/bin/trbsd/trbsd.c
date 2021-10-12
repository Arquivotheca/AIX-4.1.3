static char sccsid[] = "@(#)15	1.14.1.7  src/bos/usr/bin/trbsd/trbsd.c, cmdfiles, bos411, 9428A410j 4/5/94 14:15:29";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 * 
 * FUNCTIONS: trbsd
 *
 * ORIGINS: 18, 26, 27, 71
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
 *  RCSfile: trbsd.c,v  Revision: 1.4.2.2  (OSF) Date: 90/10/11 20:49:28
 *
 */
#include <stdio.h>
#include <locale.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/localedef.h>
#include <sys/lc_core.h>
#include <sys/limits.h>
#include <string.h>
#include <patlocal.h>

/* trbsd - transliterate data stream */

#include "trbsd_msg.h"
#define hdl __lc_collate
nl_catd catd;
#define MSGSTR(Num, Str) catgets(catd, MS_TRBSD, Num, Str)

/* Used to determine whether current locale is SBCS or MBCS .*/
int mb_cur_max; /* max number of bytes per character in current locale */
int mbcodeset;  /* 0=current locale SBCS, 1=current locale MBCS */

/* Non-portable: locale database for LC_COLLATE information.
 * Minimum and maximum wchar_t values for process codes in
 * the current locale are  __OBJ_DATA(__lc_collate)->co_wc_min  and
 * __OBJ_DATA(__lc_collate)->co_wc_max  respectively.
 */
wchar_t co_wc_min, co_wc_max;
int howmanychars;
unsigned char lc_ctype[NL_LANGMAX+1], lc_collate[NL_LANGMAX+1];
int POSIXlc_ctype, POSIXlc_collate;
int ASCIIpath;	/* 1=(mbcodeset==0 || Aflag), 0=otherwise */

/* Define STRIPNULLS so the BSD version of  tr   will strip NULL characters.
 * This is a known bug that has become expected behavior. To defeat automatic
 * stripping of NULLs, define STRIPNULLS 0 .
 */
#define STRIPNULLS 1

	/* Behavior preserving constants from obsolete NLchar.h */
#define NLCHARMAX 257

typedef	int		CHAR;
int endchar;	/* endchar is one greater than the largest process code */

unsigned long int EOS;

int dflag = 0;
int sflag = 0;
int cflag = 0;
int Aflag = 0;
char *prog;		/* store pointer to name of program */
CHAR save = 0;
CHAR code[NLCHARMAX];
char squeez[NLCHARMAX];
CHAR vect[NLCHARMAX];
wchar_t wsave;
wchar_t *wcode;
int *wsqueez;
wchar_t *wvect;

/* Character Range Table Element: for non-ASCIIpath locales, a character
 * range is represented by a Character Range Table that is ordered by
 * the unique collating weight in unique
 */
struct crte {	int	unique;			/* unique collating weight of pc */
		wchar_t pc;			/* Process code of character */
	    };

struct string { int last, max; 		/* last = most recently generated ascii char */
					/* max  = end of current ascii range in progress */
		wchar_t wlast,wmax; 	/* wchar_t versions of last, max for non-ascii */
		struct crte *rip;	/* non-ascii character Range In Progress */
		int nrip;		/* index into rip when generating a range */
		int nripmax;		/* number of characters in rip range */
		unsigned char *p; 	/* source string pointer */
	       } string1, string2;

void remove_escapes( char * );  /* Removes \seq (replace by byte value) */
wint_t next();
wint_t nextc();
int intcmp(const void * i1, const void * i2);	/* Collation comparison for ranges */


/*
 * NAME: tr [-cdsA] [string1 [string2]]
 * FUNCTION:  translate characters
 *  FLAGS:
 *   -c       complements the set of characters in string1 with respect to 
 *            string2
 *   -d       deletes all input characters in string1
 *   -s       squeezes all strings of repeated output characters that are in 
 *            string2 to single characters
 *   -A       uses the ASCII collation order.  works on byte by byte basis.
 */
main(argc,argv)
char **argv;
{
	register i;
	int j;
	register d;
	int c;
	CHAR *compl;
	wchar_t *wcompl;
	int lastd;
	wint_t wci;
	wchar_t wc,wd,wlastd;
	char mbc[MB_LEN_MAX];

	(void) setlocale( LC_ALL, "" );
	catd = catopen(MF_TRBSD, NL_CAT_LOCALE);

	string1.last = string2.last = (unsigned char)0;
	string1.max = string2.max = (unsigned char)0;
	string1.wlast = string2.wlast = (wchar_t)0;
	string1.wmax = string2.wmax = (wchar_t)0;
	string1.p = string2.p = "";

	prog=argv[0];			/* store name of program */
	if(--argc>0) {
		argv++;
		if(*argv[0]=='-'&&argv[0][1]!=0) {
			while(*++argv[0])
				switch(*argv[0]) {
				case 'c':
					cflag++;
					continue;
				case 'd':
					dflag++;
					continue;
				case 's':
					sflag++;
					continue;
				case 'A':
					Aflag++;
					/* endchar = 256; */
					/* this value is set to 256 within */
					/* the ASCIIpath path later on.    */
					/* no need to set it twice.        */
					continue;
                                default:
                                        fprintf(stderr,
                                        MSGSTR(BADUSE,"usage:\t%s  [-cdsA] [String1 [String2]]\n"),prog );
                                        exit(1);

				}
			argc--;
			argv++;
		}
	}
	if(argc>0) {
		remove_escapes(argv[0]);
		string1.p = argv[0];
	}
	if(argc>1) {
		remove_escapes(argv[1]);
		string2.p = argv[1];
	}

	mb_cur_max = MB_CUR_MAX;
	mbcodeset = (mb_cur_max > 1?1:0);

	strcpy(lc_ctype , setlocale(LC_CTYPE,NULL));
	strcpy(lc_collate , setlocale(LC_COLLATE,NULL));
	POSIXlc_ctype =   ( strcmp(lc_ctype,"C")==0
			|| strcmp(lc_ctype,"POSIX")==0 );
	POSIXlc_collate =   ( strcmp(lc_collate,"C")==0
			|| strcmp(lc_collate,"POSIX")==0 );

	ASCIIpath = ( (Aflag || (POSIXlc_ctype && POSIXlc_collate)) ? 1 : 0 );
#ifdef DEBUG_LOCALEPATH
fprintf(stderr,"trbsd: ASCIIpath=%d\n",ASCIIpath);
#endif

	if (ASCIIpath) {
	/* Translate in C locale */
		EOS = NLCHARMAX;
		endchar = 256;	/* using C locale, so 256 is 1 greater than */
				/* max process code			    */
		if(cflag) {
			for(i=0; i<endchar; i++)
				vect[i] = 0;
			while ((c = (int)next(&string1)) != EOS) {
				vect[c] = 1;
			}
			j = 0;
			for(i=0; i<endchar; i++)
				if(vect[i]==0) vect[j++] = i;
			vect[j] = EOS;
			compl = vect;
		}
		for(i=0; i<endchar; i++) {
			code[i] = EOS;
			squeez[i] = 0;
		}
		lastd = 0;
		for(;;){
	
			if(cflag) c = *compl++;
			else c = (int)next(&string1);
			if(c==EOS) break;
			d = (int)next(&string2);
		 	if (d==EOS) 
				if (sflag && !lastd) d = c;
       	                	else d = lastd;
			else lastd = d;
			code[c] = d;
			squeez[d] = 1;
		}
		while ((d = (int)next(&string2)) != EOS)
			squeez[d] = 1;
		for(i=0;i<endchar;i++) {
			if(code[i]==EOS) code[i] = i;
			else if(dflag) code[i] = EOS;
		}

		/* Read and process standard input */
		for (;;) {
			/* Get next input byte */
			if ((c = getchar()) == EOF)
				break;
#ifdef STRIPNULLS
			if(c == 0) continue;
#endif
			if ((c = code[c]) != EOS)
				if(!sflag || c!=save || !squeez[c]) {
					save = c;
					putchar(c);
				}
		}
	} else {
	/* Translate in non-C locale */
		/* Get bounds on wchar_t values for process codes.*/
		co_wc_min = __OBJ_DATA(__lc_collate)->co_wc_min;
		co_wc_max = __OBJ_DATA(__lc_collate)->co_wc_max;
		endchar = co_wc_max + 1;
		EOS = co_wc_max+1;
		howmanychars = (co_wc_max - co_wc_min + 1);
		wvect = (wchar_t *)malloc(howmanychars*sizeof(wint_t));
		wcode = (wchar_t *)malloc(howmanychars*sizeof(wint_t));
		wsqueez = (int *)malloc(howmanychars*sizeof(int));
		string1.rip = (struct crte *)malloc(howmanychars*sizeof(struct crte));
		string2.rip = (struct crte *)malloc(howmanychars*sizeof(struct crte));

		wsave = (wchar_t)0;
		if(cflag) {
			for(wci=0; wci<howmanychars; wci++)
				wvect[wci] = (wchar_t)0;

			while ((wci = next(&string1)) != EOS) {
				wvect[wci - co_wc_min] = 1;
			}
			j = 0;
			for(i=0; i<howmanychars; i++)
				if(wvect[i]==0) wvect[j++] = (wchar_t)i;
			wvect[j] = EOS;
			wcompl = wvect;
		}
		for(i=0; i<howmanychars; i++) {
			wcode[i] = EOS;
			wsqueez[i] = 0;
		}
		wlastd = (wchar_t)0;
		for(;;){

			if(cflag) wc = *wcompl++;
			else wc = (wchar_t)next(&string1);
			if(wc==EOS) break;
			wd = (wchar_t)next(&string2);
		 	if (wd==EOS) 
				if (sflag && !wlastd) wd = wc;
       	                	else wd = wlastd;
			else wlastd = wd;
			wcode[wc-co_wc_min] = wd;
			wsqueez[wd-co_wc_min] = 1;
		}
		while ((wd = (wchar_t)next(&string2)) != EOS)
			wsqueez[wd-co_wc_min] = 1;
		for(i=0;i<howmanychars;i++) {
			if(wcode[i]==EOS) wcode[i] = (wchar_t)i;
			else if(dflag) wcode[i] = EOS;
		}

		while ((wci=fgetwc(stdin)) != WEOF) {
			wc = (wchar_t)wci;
#ifdef STRIPNULLS
			if(wc == (wchar_t)0) continue;
#endif
			if ((wc = wcode[wc-co_wc_min]) != EOS)
				if(!sflag || wc!=wsave || !wsqueez[wc]) {
					wsave = wc;
					putwchar(wc+co_wc_min);
				}
		}
	}
	exit(0);
/* NOTREACHED */
}

wint_t
next(s)
struct string *s;
{
	wint_t	wci;
	char	*dummy1=NULL;
	int	dummy2;
	int	max_ucw;

#ifdef DEBUG_LOCALEPATH
fprintf(stderr,"trbsd:next(s): *s->p='%c' s->p=\"%s\"\n",*s->p,s->p);
#endif
again:
	if (ASCIIpath) {
		/* next() for C locale */
		if(s->max) {
			/* Continue generating single byte range as unsigned char */
			if(s->last++ < s->max)
				return((wint_t)s->last);
			s->max = s->last = 0;
		}
		if(s->last && *s->p=='-') {
			/* Start generating single byte range as int */
			nextc(s);
			s->max = (int)nextc(s);
			if(s->max==0) {
				s->p--;
				return('-');
			}
			if(s->max < s->last)  {
				s->last = s->max-1;
				return('-');
			}
			goto again;
		}
		return((wint_t)(s->last = (int)nextc(s)));
	} else {
		/* next() for non-C locale */
		if (s->wmax) {
		 	/* (Start or) Continue generating range from collated table */
			if(s->nrip <= s->nripmax) {
#ifdef DEBUG_LOCALEPATH
fprintf(stderr,"trbsd:next:Continue range, return 0x%.4x\n",s->rip[(s->nrip)+1].pc);
#endif
				return( (s->wlast = s->rip[++s->nrip].pc) );
			}
			s->wmax = s->wlast = (wchar_t)0;
		}
		if (s->wlast && *s->p=='-') {
			/* Initialize for generating character range. */
			/* Get upper bound character */
			nextc(s);
			s->wmax = (wchar_t)nextc(s);
#ifdef DEBUG_LOCALEPATH
fprintf(stderr,"trbsd:next:range s->wlast=0x%.4x - s->wmax=0x%.4x\n",s->wlast,s->wmax);
#endif
			if(s->wmax==0) {
#ifdef DEBUG_LOCALEPATH
fprintf(stderr,"trbsd:next:No range end, return '-'\n");
#endif
				s->p--;
				return((wint_t)'-');
			}
			/* Get unique collating weight for lower bound character */
			s->rip[0].unique=__wcuniqcollwgt(s->wlast);
			s->rip[0].pc = s->wlast;
			/* Get unique collating weight for upper bound character */
			max_ucw=__wcuniqcollwgt(s->wmax);

			if (s->rip[0].unique  > max_ucw) {
#ifdef DEBUG_LOCALEPATH
fprintf(stderr,"trbsd:next:Range ends out of order, return '-'\n");
#endif
				s->nripmax = 0;
				s->rip[1].pc = s->wmax; 
				return((wint_t)'-');
			}
			s->nrip = 0;
			s->nripmax = 1;
			/* Find all of the process codes in the current locale
			 * that collate between the ends of the rangei and put
			 * them in the range table.
			 */
			for (wci=(unsigned int)co_wc_min;wci<=(unsigned int)co_wc_max;wci++) {

				s->rip[s->nripmax].unique=__wcuniqcollwgt((wchar_t)wci);
				/* If the process code collates between the ends of
				 * this range, add it to the Range Table.
				 */
				if ((s->rip[s->nripmax].unique > s->rip[0].unique) &&
				    (s->rip[s->nripmax].unique <= max_ucw)) {
					s->rip[s->nripmax++].pc = (wchar_t)wci;
					}
			}
			s->nripmax--;
			/* Sort the Range Table into collation order */
			qsort(&s->rip[1],s->nripmax,sizeof(struct crte),intcmp);
			/* Go back and return the first character in the range */
			goto again;
		}
#ifdef DEBUG_LOCALEPATH
fprintf(stderr,"trbsd:next:Normal return from nextc(s)\n");
#endif
		return((wint_t)(s->wlast = nextc(s)));
	}
}

wint_t
nextc(s)
struct string *s;
{
	register i, n;
	unsigned char c;
	char * msgstr;
	wint_t rc;
	wchar_t wc;
	int chrlen;

	if (ASCIIpath) {
		/* Get next String character as single byte */
		wc = (wchar_t)*s->p++;
	} else {
		/* Get next String character in multibyte locale */
		if ((chrlen = mbtowc(&wc,s->p,mb_cur_max)) > 0) {
			s->p += chrlen;
		} else {
			s->p++;
			wc = (wchar_t)0;
		}
	}
	if (wc == (wchar_t)L'\0') {
		--s->p;
		return ((wint_t)EOS);
	}
	if(wc==(wchar_t)L'\\') {
		i = n = 0;
		while(i<3 && (c = *s->p)>=(unsigned char)'0' 
			  && c<=(unsigned char)'7') {
			n = n*8 + (int)(c - (unsigned char)'0');
			i++;
			s->p++;
		}
		if(i>0) {
			wc = (wchar_t)n;
			if ((unsigned int)wc >= endchar) {
                                fprintf(stderr, 
                                  MSGSTR(BADVAL,"%s: Bad octal value\n"),prog );
				exit(1);
			}
		}
		else {
			/* Deliver next character as is */
			if (ASCIIpath)
				wc = (wchar_t)*s->p++;
			else if ((chrlen = mbtowc(&wc,(unsigned char *)s->p,mb_cur_max)) > 0)
				s->p += chrlen;
			else {
				s->p++;
				wc = (wchar_t)0;
			}
		}
#ifdef STRIPNULLS
		if(wc==0) *--s->p = 0;
#endif
	}
	return((wint_t)wc);
}

/* NAME		intcmp
 * FUNCTION	Compares unique collating elements in Range Table for qsort().
 *
 * compares the ints pointed to by the arguments.
 * This means that the structure pointers passed in must start with
 * the unique collating weight.
 */
int intcmp(const void * i1, const void * i2)

{
	return(*(int *)i1 - *(int *)i2);

}


/*
 * remove_escapes - takes \seq and replace with the actual character value.
 *              \seq can be a 1 to 3 digit octal quantity or {abfnrtv\}
 *
 *              This prevents problems when trying to extract multibyte
 *              characters (entered in octal) from the translation strings
 *
 * Note:        the translation can be done in place, as the result is
 *              guaranteed to be no larger than the source.
 */

void
remove_escapes( char *s ){
    char *d = s;                /* Position in destination of next byte */
    int i,n;
    int mb_cur_max = MB_CUR_MAX;

    while (*s) {                /* For each byte of the string */
        switch (*s) {
          default:
            i = mblen(s, mb_cur_max);
            if (i < 0) i=1;     /* Not a MB char - just move one byte */

            while(i--) { *d++ = *s++; }; /* Move the character */
            break;

          case '\\':
            switch (*++s) {
              case '0':
              case '1':
              case '2':
              case '3':
              case '4':
              case '5':
              case '6':
              case '7':
                        i = n = 0;
                        while (i<3 && *s >= '0' && *s <= '7') {
                            n = n*8 + (*s++ - '0');
                            i++;
                        }
                        *d++ = n;       break;

              case 'a': *d++ = '\a';    s++; break;
              case 'b': *d++ = '\b';    s++; break;
              case 'f': *d++ = '\f';    s++; break;
              case 'n': *d++ = '\n';    s++; break;
              case 'r': *d++ = '\r';    s++; break;
              case 't': *d++ = '\t';    s++; break;
              case 'v': *d++ = '\v';    s++; break;
              case '\\':*d++ = '\\';    s++; break;
              default:  *d++ = *s++;    break;
            }
        }                       /*     switch */
    }                           /* while (*s) */
    *d = '\0';
}

