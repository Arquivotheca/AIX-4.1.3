#ifndef LD_128
static char sccsid[] = "@(#)48  1.87  src/bos/usr/ccs/lib/libc/doscan.c, libcio, bos41J, 9521B_all 5/19/95 15:27:19";
#endif /* LD_128 */
/*
 * COMPONENT_NAME: (LIBCIO) Standard C Library I/O Functions 
 *
 * FUNCTIONS: _doscan, nf, NLnan_doscan, getcc, ungetcc 
 *
 * ORIGINS: 3, 27 
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
 */

#define _ILS_MACROS
#ifdef _THREAD_SAFE
# ifndef _STDIO_UNLOCK_CHAR_IO
# define _STDIO_UNLOCK_CHAR_IO
# endif
#include <stdio_lock.h>
#endif /* _THREAD_SAFE */

#include <stdio.h>
#include <stdlib.h>		/* strtold, etc. */
#include <string.h>
#include <ctype.h>
#include <varargs.h>
#include <values.h>
#include <memory.h>
#include <fp.h>
#include <nl_types.h>
#include <langinfo.h>
#include <errno.h>
#include <limits.h>		/* LONGLONG_MIN */

static int number();
static unsigned char *setup();
static int string();
static int reorder();
static wint_t ungetcc();
static wint_t getcc();
static void settab();
static int gettab();
static void inittab();

#define NCHARS		(1 << BITSPERBYTE)
#define MAXBUF	2048	/* to keep track of char bits */
#define GETC(iop)	\
	(((iop->_flag&_IONOFD)&&iop->_cnt<=0)? WEOF: getcc(iop, icp))
#define GETC_SB(iop, c)	\
	if ((iop->_flag&_IONOFD)&&iop->_cnt<=0) c=EOF; else if ((c=getc(iop))!=EOF) (*icp)++;
#define RFREE(s)	if(s) free((void *)s)
#define MBLEN(x)	((mbleng=mblen(x,mb_cur_max)) > 1 ? mbleng : 1)

/*                                                                    
 * FUNCTION: _doscan: common code for scanf, sscanf, fscanf
 *
 * RETURN VALUE DESCRIPTIONS:
 * 	     - returns number of matches
 *	     - EOF on failure
 *
 * NOTES:
 *	With AIX 4 and XPG 4, the IBM extension 'ws' has been replaced with 
 *	X/Open's 'S' and 'wc' has been replaced with 'C'.  The code for 'ws'
 *	and 'wc' is left as is for binary compatibility reasons.
 */

int
#ifdef LD_128
_doscan128(FILE *iop, unsigned char *ofmt, va_list ova_alist)
#else /* must be doscan */
_doscan(FILE *iop, unsigned char *ofmt, va_list ova_alist)
#endif /* LD_128 */
{
	unsigned char *fmt;	/* (possibly new) format string */
	va_list	va_alist; 	/* (possibly reordered) arg. list */
	unsigned char *nfmt;	/* temp. format ptr. if reordering */
	va_list	nva_alist; 	/* temp. args ptr. if reordering */
	extern unsigned char *setup();
	int tab[MAXBUF];
	int ch;
	int rc;
	int mb = MB_CUR_MAX,
	    mbflag = mb > 1;
	wchar_t wc;

	/* nmatch is the number of specifiers that matched something
	 * in the input stream scanned.
	 */
	int nmatch = 0; 
	int len;
	wint_t inchar;
	int stow;
	int rc22=0;			/* was size in two places */
	int size;			/* 64890: ' ' => no size; 'h' => "h";
					 * 'l' => "l" or "L"; 'Z' => "LL",
					 * "ll", "Ll", or "lL" */
	int reorder_res;
	char wlflag = 0;
	char Nflag = 0;
	char Bflag = 0;
	unsigned char *bp;
	/* p82404 - pattern "%1$" was too restrictive */
	char	pattern='$'; /* pattern indicating variable arg. ordering */
	int incount = 0, *icp = &incount;
	int inpeof = 0;

	va_alist = ova_alist;
	nfmt = 0;
	nva_alist = 0;
	fmt = ofmt;

	if ((strchr(ofmt, pattern) != (char *)NULL)) {
	    if ((reorder_res = (reorder (ofmt, ova_alist, &nfmt, &nva_alist)))
		< 0) {
		/* p82331 - fix memory leak */
		RFREE(nva_alist);
		RFREE(nfmt);
		return (EOF);
	    }
	    else 
		if (reorder_res == 0) {  /* p81668 */
		    va_alist = nva_alist;
		    fmt = nfmt;
		}
	}

	/* Main Loop:  Read the format specifications for values to
	 * to be scanned one at a time.  For each specification read
	 * from the input stream the expected type of value.
	 */

	for( ; ; ) {
		if((ch = *fmt++) == '\0')
		{
			RFREE(nva_alist);
			RFREE(nfmt);
			return(nmatch); /* end of format */
		}
		if (isspace(ch) || (mbflag && mbtowc(&wc, fmt-1, mb)>1 && 
		    iswspace(wc))) {
			while (inchar = GETC(iop), iswspace(inchar))
				 ;
			if (inchar == WEOF) {
				inpeof = 1;
				continue;
			}
			if(ungetcc(inchar, iop, icp) != WEOF)
				continue;
			break;
		}

		if (!wlflag)
			if(ch != '%' || (ch = *fmt++) == '%') {
				GETC_SB(iop, inchar);
				if((int) inchar == ch)
					continue;
				if ((int) inchar != EOF)
					(*icp)--;
				if(ungetc((int) inchar, iop) != EOF)
				{
					RFREE(nva_alist);
					RFREE(nfmt);
					return(nmatch); /* failed to match input */
				}
				break;
			}

		/* When stow is 0, the input is read but not written. */
		if(ch == '*') {
			stow = 0;
			ch = *fmt++;
		} else
			stow = 1;

		/* N flag is no-op starting V3.2 */
		if (ch == 'N')
			ch = *fmt++;

		/* B flag is no-op starting V3.2 */
		if (ch == 'B')
			ch = *fmt++;

		/*  If wlflag is not already set, check to see if current
		 *  character is a 'w'.  Set wlflag accordingly and continue
		 *  processing.
		 */

		if (!wlflag)
			if (ch == 'w') {
				wlflag = 'w';
				ch = *fmt++;
			}

		for(len = 0; isdigit(ch); ch = *fmt++)
			len = len * 10 + ch - '0';
		if(len == 0)
			len = MAXINT;

		/* Check for syntax "%[*][w][B][N][<length>]<fmt_desc>"
		 */
		if (ch == 'N' || ch == 'B' || ch == 'w')
			break;

		/* 64890:  64-bit long long int adds "LL", "ll", "Ll", "lL" */
		if (ch == 'h') {
		    size = 'h';		/* indicate got 'h' */
		    ch = *fmt++;	/* advance past 'h' */
		} else if ((ch == 'l') || (ch == 'L')) {
#ifdef LD_128
		    size = ch;
#else
		    size = 'l';		/* indicate got 'l' */
#endif /* LD_128 */
		    ch = *fmt++;	/* advance past 'l' 'L' */
		    if ((ch == 'l') || (ch == 'L')) {
			size = 'Z';	/* indicate got 'LL' */
			ch = *fmt++;	/* advance past 'l' 'L' */
		    };
		} else {
		    size = ' ';		/* No size specified */
		};

		if (ch == 'n') {
			wlflag = 0;
			if (stow) {
				switch(size) {
				case 'h':
					*va_arg(va_alist, short *) = (short)incount;
					break;
				case 'L':
				case 'l':
					*va_arg(va_alist, long *) = (long)incount;
					break;
				case 'Z':     /* 64890: 64-bit long long int */
				    *va_arg(va_alist, long long *)
					= (long long) incount;
				    break;
				case ' ':
				default:
					*va_arg(va_alist, int *) = (int)incount;
				}
				continue;
			}
			else
				continue;
		}

		if (inpeof)
			break;

		/* A call to setup defines scansets. */
		if(ch == '\0' ||
		    ch == '[' && (fmt = setup(fmt, tab)) == NULL)
		{
			RFREE(nva_alist);
			RFREE(nfmt);
			return(EOF); /* unexpected end of format */
		}
			/* this is not documented and support is
			   not guaranteed in future releases    */
		if ((ch != 'N') && (ch != 'B') && 
		    ! ((wlflag && (ch == 's' || ch == 'c' )) 
		      || ch == 'S' || ch == 'C') && isupper(ch)) {
			/* no longer documented */
			ch = _tolower(ch);
		}
		if(ch != 'c' && ch != 'C' && ch != '[') {
			while(inchar = GETC(iop), iswspace(inchar))
				;
			if(ungetcc(inchar, iop, icp) == WEOF)
				break;
		}


		/* if wlflag is true, current character must be in the set
		 * listed below.  If it is not, a format error is reached
		 * and we should stop processing.
		 */
		if (wlflag) {
			switch(ch) {
			case 's':
			case 'c':
			case 'd':
			case 'i':
			case 'o':
			case 'u':
			case 'X':
			case 'x':
			case 'p': 		
			case 'E':
			case 'e':
			case 'f':
			case 'G':
			case 'g':
				break;
			default:
				RFREE(nva_alist);
				RFREE(nfmt);
				return(nmatch!=0 ? nmatch : inpeof ? EOF : 0);
			}
		}

		/* If a character string is specified, then call the
		 * string function; otherwise, call the number function.
		 * Note that "number" is not restricted to handling
		 * digit strings, it must also recognize special IEEE
		 * values (INF, NaNQ, NaNS).
		 */

		inpeof = 0;
		rc = 0;
		if((rc22 = (ch == 'S' || ch == 'C' || ch == 'c' || ch == 's' || ch == '[') ?
		    string(wlflag,Nflag,Bflag,stow,ch,len,tab,iop,icp, &inpeof, &va_alist):
		    (rc = number(wlflag,stow,ch,len,size, iop,icp,&inpeof,&va_alist))) != 0)
			nmatch += stow;

		if (rc == -1) {
			RFREE(nva_alist);
			RFREE(nfmt);
			return(-1);
		}

		/* 'w' qualifiers should affect only the current
		 * descriptor, so we turn them off here.
		 */

		wlflag = 0;

		if(va_alist == NULL) /* end of input */
			break;

		if(rc22 == 0)
			/* failed to match input */
		{
			RFREE(nva_alist);
			RFREE(nfmt);
			return(nmatch != 0 ? nmatch : inpeof ? EOF : 0);
		}
	}
	RFREE(nva_alist);
	RFREE(nfmt);
	return(nmatch != 0 ? nmatch : EOF); /* end of input */
}


/* Functions to read the input stream in an attempt to match incoming
 * data to the current specification from the main loop of _doscan().
 */

/* The arguments are defined as follows:
 *    stow - if set the value being scanned is written to listp
 *    type - the scan conversion character
 *    len - the length of the string to be scanned (MAXINT default)
 *    size - equal to type unless a long (l or h) conversion
 *    iop - pointer to input being scanned
 *    listp - list of pointers where values should be stored
 */
#define NUMBUF_SIZE 256
static int
number(wlflag, stow, type, len, size, iop, icp, eofp, listp)
char wlflag;
int stow, type, len, size;
FILE *iop;
int *icp, *eofp;
va_list *listp;
{
	int next_radix;
	int next_e;
	int next_sign;

	int rc;
	char buffer[NUMBUF_SIZE];
	char *numbuf=buffer;
	char *np=numbuf;
	char *malloced_numbuf=0;
	int buf_index;
	wint_t c;
	wint_t c1;
	int base;
	int digitseen = 0, radixseen = 0, expseen = 0, floater = 0, negflg = 0;
	int zero = 0;
	long lcval = 0;
	long long llval = 0LL;	/* 64890: 64-bit long long int support */
	int t;
	int nanret = 0;
	int pointer = 0;
	char *radix = nl_langinfo(RADIXCHAR);

	/* If there is no radix character defined for this locale, use '.' */
	if (radix[0] == '\0')
		radix = ".";
	switch(type) {
	case 'E':
	case 'e':
	case 'f':
	case 'G':
	case 'g':
		floater++;
	case 'd':
	case 'u':
		base = 10;
		break;
	case 'o':
		base = 8;
		break;
	case 'p':
		pointer = 1;	/* in hex, so fall through to 'X' case */
	case 'X':
	case 'x':
		base = 16;
		break;
	case 'i':
		base = -1;
		break;
	default:
		return(0); /* unrecognized conversion character */
	}
	switch(c = GETC(iop))
	{
	case '-':
		negflg++;
	case '+': /* fall-through */
		len--;
		c = GETC(iop);
		/********
		  if c is WEOF, put it back and handle in the next section
		*********/
		if (c == WEOF) {
			len++;
			ungetcc(c, iop, icp); /* character after [+-] */
			if (negflg) {
				negflg=0;
				c = '-';
			}
			else
				c = '+';
		}
	}

	if( base == 16 && c == '0' ) {   /* Lookahead for 0x or 0X */
		len--;
		c = GETC(iop);
		if(c=='x' || c=='X') {
			len--;
			c = GETC(iop);
			/* check if the next character is an xdigit. if it is not, then put them all back */
			if ( ! iswxdigit(c)) {
				len++;
				ungetcc(c, iop, icp); /* character after (xX) */
				return(0);
			}
		} else  {
			len++;
			ungetcc(c,iop, icp);
			c = '0'; /*restore original value */
		}
	}			
	
 	/* set base if %i format specified */
 	while (base == -1)
 		switch(c) {
 		case '0':
 			if (zero) {
 				base = 8;
 				break;
 			}
 			zero = c;
 			len--;
 			c = GETC(iop);
 			break;
 		case 'x':
 		case 'X':
 			base = 16;
 			len--;
 			c = GETC(iop);
 			break;
 		default:
 			if (zero) {
 				ungetcc(c, iop, icp);
 				len++;
 				c = zero;
 				base = 8;
 			}
 			else
 				base = 10;
 			break;
 		}

	/* First determine which case - either a number or one of the
	 * three special IEEE numbers which begin with either a I, Q
	 * or a S (P34273).  SNaN and QNaN have been replaced with
	 * NaNS and NaNQ.  INF and infinity are still OK.
	 */
	if (wlflag && c == 's')
	    t='S';
	else
	    t=c;

	switch(t) {
	default:
		/* Store the number in numbuf as it's read in.  A blank will
		 * cause a break in the loop.
		 */

		buf_index=1;
		/* want to leave one byte for the null-terminator  */
		/*  so we start counting with 1 so that we reach   */
		/*  256 even though we've only read 255.  That way */
	 	/*  if we need to read 256 bytes, this algorithm   */
		/*  will malloc more space to include the NULL.    */
		/* Problem with buf_index=0 is that if we read 256 */
		/*  bytes exaclty, it will exit the for() loop     */
		/*  and won't malloc more space for the NULL.      */
		next_radix=1;
		next_e=0;
		next_sign=0;
		c1=-1;

		for( ; --len >= 0; *np++ = c, c = GETC(iop))
		{
			if (c == WEOF) {
				/**********
				  If a valid number has been found, just break,
				  otherwise, put back WEOF and go on
				**********/
				if (digitseen || c1 == -1)
					break;
				len++;
 				ungetcc(c, iop, icp);
				c = c1;
			}
			c1 = c;

			/**********
			  Every NUMBUF_SIZE bytes, realloc some more space
			**********/
			if (buf_index && (buf_index % NUMBUF_SIZE == 0)) {
				/**********
				  If this is the first time space ran out, malloc more space
				  and copy the contents of buffer to the malloced space
				**********/
				if (!malloced_numbuf) {
					if ((numbuf = (char *) malloc(NUMBUF_SIZE*2)) == NULL) {
						errno = ENOMEM;
						return -1;
					}
					memcpy(numbuf, buffer, NUMBUF_SIZE);
				}
				else {
					numbuf = (char *) realloc(numbuf, ((buf_index / NUMBUF_SIZE) + 1) * NUMBUF_SIZE);
					if (numbuf == NULL) {
						errno = ENOMEM;
						(void)free(malloced_numbuf);
						return -1;
					}
				}

				/**********
				  set the malloced_numbuf pointer so this space can be free'd
				**********/
				malloced_numbuf = numbuf;
				np = numbuf + buf_index - 1;
			}

			buf_index++;

			if(isdigit(c) || base == 16 && isxdigit(c)) {
				int digit = c - (isdigit(c) ? '0' :
				    isupper(c) ? 'A' - 10 : 'a' - 10);
				if(digit >= base)
					break;
				if(stow && !floater)
				    if (size == 'Z') {	/* 64890: 64-bit long long int */
					llval = base * llval + digit;
				    } else {
					lcval = base * lcval + digit;
				    };
				digitseen++;

				if (! expseen)
					next_e=1;
				else
					next_e=0;

				if (! radixseen)
					next_radix=1;
				else
					next_radix=0;

				next_sign=0;

				continue;
			}
			if(!floater)
				break;

			if(next_sign && (c == '+' || c == '-')) {

				next_sign = 0;
				next_e = 0;
				next_radix = 0;
			continue;
			}

			if(next_radix && c == (wint_t) *radix) {
				int i=1;
				wint_t x;
				if (radix[i])
					x = GETC(iop);
				while(radix[i] && (wint_t)radix[i] == x) {
					i++;
					if (radix[i])
						x = GETC(iop);
					}
				if (radix[i] == '\0') {
/*
 * NOTE: atof() currently does NOT handle multi-byte radix characters, it only
 *	 looks at the first byte of the radix character.  So, to handle this,
 *	 only the first byte of the radix character is put into numbuf[] which
 *	 is then passed on to atof().  If atof() changes to handle a mb radix,
 *	 then this code should change to send the whole string.
 */
					c = radix[0];
					radixseen=1;
					next_radix=0;
					next_e = 1;
					next_sign = 0;
					continue;
				}
				if(ungetcc(x, iop, icp) == WEOF)
					*eofp = 1;	/* end of input */
				digitseen=0;
				break;
			}
			if(next_e && (c == 'e' || c == 'E') && digitseen) {

				digitseen=0;
				radixseen=1;
				expseen=1;

				next_radix=0;
				next_e=0;
				next_sign=1;
				continue;
			}
			break;
		}
		if(stow && digitseen)
			if(floater) {
				*np = '\0';
				switch (size) {
#ifdef LD_128
			              case 'L':
					{
					long double ldval;
					int saveerrno = errno;
					ldval = strtold(numbuf, NULL);
					errno = saveerrno;
					if (negflg)
						ldval = -ldval;
					*va_arg(*listp, long double *) = ldval;
					}
					break;
#endif /* LD_128 */
				      case 'l':
					{
					double dval;
					int saverrno=errno;

					dval = atof(numbuf);
					errno=saverrno;
					if(negflg)
						dval = -dval;
					*va_arg(*listp, double *) = dval;
					}
					break;

				      default:
					/* 64890: LL 'Z' is same as no size */
					{
					float dval;
					int saverrno=errno;

					dval = atof(numbuf);
					errno=saverrno;
					if(negflg)
						dval = -dval;
					*va_arg(*listp, float *) = dval;
					}
				}
			} else {
				/* 
				 suppress possible overflow on 2's-comp 
				 negation
				 */
			    if (negflg) {
				if (size == 'Z') {	/* 64890: 64-bit long long int */
				    if (llval != LONGLONG_MIN)
					llval = -llval;
				} else {	/* size != 'Z' */
				    if (lcval != HIBITL)
					lcval = -lcval;
				};
			    };		/* end if negflg */
				if (!pointer)
					if(size == 'l')
						*va_arg(*listp, long *) = (long)lcval;
					else if(size == 'h')
						*va_arg(*listp, short *) =
						(short)lcval;
					else if (size == 'Z')	/* 64890: 64-bit long
								 * long */
					    *va_arg(*listp, long long *) = (long long) llval;
					else
						*va_arg(*listp, int *) =
						(int)lcval;
				else
				{
					pointer = 0;
					if (size == 'Z')	/* 64890: 64-bit long long */
					    lcval = llval;	/* demote from long long */
					*va_arg(*listp, void **) =
					(void *) lcval;
				}
			}
		if(ungetcc(c, iop, icp) == WEOF)
			*eofp = 1;	/* end of input */
		break;
	/* Check for special IEEE values INF, NaNQ, and NaNS.
	 * When any of these strings are found the next character
	 * is read to be consistent with the for loop that reads
	 * the numbers, digitseen is also incremented because these
	 * are considered legitimate matches.
	 */
	case 'i':
	case 'I':
	  if (floater)
	    {
	      if (nf(iop, icp)) {
		     /* An IEEE infinite has been scanned. */
		     digitseen++;
		     if (stow) {
			/* Explicitly set the INF bit string for double or
			 * float types.
			 */
				switch (size) {
#ifdef LD_128
			              case 'L':
					{
					union {
						long double ld;
						unsigned int i[4];
					} ldval;

					if (negflg) {
						ldval.i[0] = 0xfff00000;
						ldval.i[2] = 0x80000000;
	    		} else {
						ldval.i[0] = 0x7ff00000;
						ldval.i[2] = 0x0;
						}
					ldval.i[1] = 0x0;
					ldval.i[3] = 0x0;
					*va_arg(*listp, long double *) = ldval.ld;
					}
					break;
#endif /* LD_128 */
				      case 'l':
					{
					double ival;
					if (negflg) {
						DBL(ival, 0xfff00000, 0x00000000);
					}
					else
						DBL(ival, 0x7ff00000, 0x00000000);
					*va_arg(*listp, double *) = ival;
					}
					break;
				      default:
					/* 64890: LL 'Z' is same as no size */
					{
					float fival;
					if (negflg)
						VALH(fival) = 0xff800000;
					else
						VALH(fival) = 0x7f800000;
					*va_arg(*listp, float *) = fival;
					}
			} /* switch (size) */
		     } /* if (stow) */
	      } /* if nf() */
	    } /* if (floater) */
	    else
		if(ungetcc(c, iop, icp) == WEOF)
			*eofp = 1; /* end of input */
		break;
	case 'n':
	case 'N':
	  if (floater)
	  {

	   nanret = NLnan_doscan(iop, icp);
	   if (nanret == 1) {
	      digitseen++;

	      if (stow) {

	      /* A NaNQ has been scanned and needs to be stored.  Negative
	       * signs must be given in the actual assignments to take
	       * effect.  Printf of float negative NaNQs yields a NaNQ.
	       */

		switch (size) {
#ifdef LD_128
	              case 'L':
			{
			union {
				long double ld;
				unsigned int i[4];
			} ldval;
			    
			if (negflg) {
				ldval.i[0] = 0xfff80000;
				ldval.i[2] = 0x80000000;
			} else {
				ldval.i[0] = 0x7ff80000;
				ldval.i[2] = 0x0;
				}
			ldval.i[1] = 0x0;
			ldval.i[3] = 0x0;
			*va_arg(*listp, long double *) = ldval.ld;
			}
			break;
#endif /* LD_128 */
		      case 'l':
			{
			double qval;
			if (negflg) {
				DBL(qval, 0xfff80000, 0x00000000);
			}
			else
				DBL(qval, 0x7ff80000, 0x00000000);
			*va_arg(*listp, double *) = qval;
			}
			break;
			
		      default:				
			/* 64890: LL 'Z' is same as no size */
			{
			float fqval;
			if (negflg)
				VALH(fqval) = 0xffc00000;
			else
				VALH(fqval) = 0x7fc00000;
			*va_arg(*listp, float *) = fqval;
			}
		} /* switch (size) */
	      } /* if (stow) */
	   } /* if nanret() == 1 */
	   else if (nanret == 2) {
	      digitseen++;

	      if (stow) {

	      /* A NaNS has been scanned and needs to be stored.
	       */

		switch (size) {
#ifdef LD_128
	              case 'L':
			{
			union {
			long double ld;
			unsigned int i[4];
			} ldval;
			
			if (negflg) {
				ldval.i[0] = 0xfff55555;
				ldval.i[2] = 0x80000000;
			} else {
				ldval.i[0] = 0x7ff55555;
				ldval.i[2] = 0x0;
				}
			ldval.i[1] = 0x55555555;
			ldval.i[3] = 0x0;
			*va_arg(*listp, long double *) = ldval.ld;
			}
			break;
#endif /* LD_128 */
		      case 'l':
			{
			double sval;
			if (negflg) {
				DBL(sval, 0xfff55555, 0x55555555);
			}
			else
				DBL(sval, 0x7ff55555, 0x55555555);
			*va_arg(*listp, double *) = sval;
			}
			break;

		      default:				
			/* 64890: LL 'Z' is same as no size */
			{
			float fsval;
			if (negflg)
				VALH(fsval) = 0xff855555;
			else
				VALH(fsval) = 0x7f855555;
			*va_arg(*listp, float *) = fsval;
			}
		} /* switch (size) */
	      } /* if (stow) */
	   } /* if natret == 2 */
	  } /* if (floater) */
	  else
		if(ungetcc(c, iop, icp) == WEOF)
			*eofp = 1; /* end of input */
		break;
	}

	/**********
	  if numbuf had to be malloced, then free it
	**********/
	if (malloced_numbuf)
		free(malloced_numbuf);

	return(digitseen); /* successful match if non-zero */
}

static int
string(wlflag, Nflag, Bflag, stow, type, len, tab, iop, icp, eofp, listp)
char wlflag, Nflag, Bflag;
int stow, type, len;
int *tab;
FILE *iop;
int *icp, *eofp;
va_list *listp;
{
	char *ptr;
	wint_t Nch;
	char *start;
	char cpsize, cpbuf[MB_LEN_MAX];
	char i;
	int lowlim = 0;
	char Shiftread = 0;
	wchar_t *NLstart;
	wchar_t *NLptr;
	wint_t val;

	if((wlflag=='l') && ((type=='c') || (type == 's')))
		wlflag='\0';

	if (type == 'S' || type == 'C' ||(wlflag &&(type == 's' || type == 'c')))
		NLstart = NLptr = stow ? va_arg(*listp, wchar_t *) : NULL;
	else
		start = ptr = stow ? va_arg(*listp, char *) : NULL;
	if((type == 'c' || type == 'C') && len == MAXINT)
		len = 1;

	if (! ((wlflag && (type == 's' || type == 'c' ))
	      || type == 'S' || type == 'C'))
	    while (((Nch = val = GETC(iop)) != WEOF) &&
	        !(type == 's' && iswspace(Nch) || type == '[' &&
		  gettab(tab, (int) Nch))) {

		    cpsize = wctomb(cpbuf, Nch);

		    lowlim += cpsize - 1;

		    if (stow)
			    for (i=0; i<cpsize; i++)
				    *ptr++ = cpbuf[i];
		    else
			    ptr += cpsize;
    
	    	    if(--len <= lowlim)
	    		    break;
	    }
	else
	    while (((Nch = val = GETC(iop)) != WEOF)
		   && (!iswspace(Nch) || 
		   (((wlflag && type == 'c') || type == 'C') &&
		   iswspace(Nch)))) {
		   if (stow)
			   *NLptr = Nch;
		   NLptr++;
    
		   if(--len <= 0)
			    break;
	    }

	if(val == WEOF || len > lowlim && ungetcc(Nch, iop, icp) == WEOF)
		*eofp = 1; /* end of input */

	if (! ((wlflag && (type == 's' || type == 'c' ))
	      || type == 'S' || type == 'C')) {
		if(ptr == start)
			return(0); /* no match */
		if(stow && type != 'c') /* note wc excluded above */
			*ptr = '\0';
	}
	else {
		if(NLptr == NLstart)
			return(0); /* no match */
		if(stow && ! (type == 'C' || (wlflag && type == 'c')))
			*NLptr = '\0';
	}
	return(1); /* successful match */
}

static unsigned char *
setup(fmt, tab)
unsigned char *fmt;
int *tab;
{
	int b, c, d, t = 0;
	int saveval;
	int mb_rc=0;

	if(*fmt == '^') {
		t++;
		fmt++;
	}
	inittab(tab, t);
	if((c = *fmt) == ']' || c == '-') { /* first char is special */
		settab(tab, c, t);
		fmt++;
	}
	while((c = *fmt++) != ']') {
		int i;
		if(c == '\0')
			return(NULL); /* unexpected end of format */

		/* This is where the scanset range is specified,
		 * b represents the first and d the last character
		 * included in the scanset.
		 */
		if(c == '-' && (d = *fmt) != ']' && (mb_rc=check_mb(fmt, &d)) &&
		    saveval < d) {
			for ( i=saveval; i<=d; i++ )
			    settab(tab, i, t);
			if ( mb_rc > 1 )
			    while ( mb_rc-- )
				fmt++;
			fmt++;
		} else {
			if ((mb_rc = check_mb((fmt - 1), &c)) > 1)
			    while ( --mb_rc )
				fmt++;
			settab(tab, c, t);
			saveval = c;
		}
	}
	return(fmt);
}

/* Check for nf */
static nf(iop, icp)
FILE *iop;
int *icp;
{
/* Checks for input string INF or INFINITY,returning true if match     *
 * found or false in case of no match.If the character after 'F' is an *
 * 'I' continue scanning for INFINITY. Going by the ANSI scanf specs a * 
 * single character push back is sufficient for scanf implementation   *
 * This and the statement ANSI 4.9.6.2 pg 138 line 15 "If conversion   *
 * terminates on a conflicting input character,the offending character *
 * is left unread in the input stream."  implies that we don't have to *
 * push back the already read characters.                              *
 */
        wchar_t ch;


	if (((ch = GETC(iop)) == 'n') || (ch == 'N'))
	{
	   if (((ch = GETC(iop)) == 'f') || (ch == 'F'))
	     if (((ch = GETC(iop)) == 'i') || (ch == 'I'))
	     {
	       if (((ch = GETC(iop)) == 'n') || (ch == 'N'))
	         if (((ch = GETC(iop)) == 'i') || (ch == 'I'))
	           if (((ch = GETC(iop)) == 't') || (ch == 'T'))
	             if (((ch = GETC(iop)) == 'y') || (ch == 'Y'))
			   return 1;
	      }
	      else
	      {
	          ungetcc(ch, iop, icp);
		  return(1);
	      }

	}
	ungetcc(ch, iop, icp);
	return (0);
}

/* Check for aNQ or aNS or aN */
static NLnan_doscan(iop, icp)
FILE *iop;
int *icp;
{
	wchar_t c[3];

	/* Backup if ! aNQ or aNS
	 */

	if (((c[0] = GETC(iop)) == L'a') || (c[0] == L'A'))
	{	if (((c[1] = GETC(iop)) == L'n') || (c[1] == L'N'))
		{	if (((c[2] = GETC(iop)) == L'q') || (c[2] == L'Q')) {
				/* NanQ */
				return 1;
			}
			/*
			  anything else is treated as a NaNS, but if s is not there
			  put the character back
			*/
			else if ((c[2] != (wchar_t)WEOF) && 
				 (c[2] != L's') && 
				 (c[2] != L'S'))
			    ungetcc(c[2], iop, icp);
			return 2;
		}
		ungetcc(c[1], iop, icp);
	}
	ungetcc(c[0], iop, icp);

	return(0);
}


/*
 *	Reorder goes through a format statement containing variably
 *      ordered arguments and reorders the arguments accordingly,
 *      stripping out the x$'s.  Upon success, reorder() returns 0;
 *	the new format string is returned via the argument "bp_new_fmt",
 *	and the new arg pointer is returned via the argument "bp_new_args".
 *	Upon failure, reorder() returns -1; *bp_new_fmt = *bp_new_args = 0.
 */

/* Convert a digit character to the corresponding number */
#define tonumber(x) ((x)-'0')

static
reorder (format, args, bp_new_fmt, bp_new_args)
char   *format;
va_list args;
char		**bp_new_fmt;	/* new format */
va_list		*bp_new_args;	/* new argument list */
{
	/* Order array keeps track of arg variable order ..
            e.g. arg 2 comes before arg 1 */
	int	*order;

	/* New_args is the new args va_list returned to caller */
	char    *new_args;

	/* Arg_index counts number of args (i.e. % occurrences) */
	int     arg_index;

	/* Last_alloc is the last alloced index for the size&order arrays */
	int     last_alloc;

	/* size of pointer ... usually four */
	int	ptr_size;	    

	/* Tot_size counts the actual size of all arguments in bytes */
	int     tot_size;

	/* arg_count is the total number of arguments in format string */
	int	arg_count;

	/* new_fmt is the working pointer in the new format string*/
	/* bp is the beginning pointer to original format string  */
	char   *new_fmt, *bp;

	/* Format code */
	int	fcode;

	/* Return code */
	int	rcode;

	/* Flags - nonzero if corresponding character is in format */
	int     length;         /* l */

	/* work variable(s) */
	int 	i;

	bp = format;
	tot_size = arg_count = 0;
	*bp_new_fmt = *bp_new_args = 0;
	rcode = 0;

	/*
	 *      The format loop interogates all conversion characters to
	 *      determine the order and size of each argument, 
	 *      calculating the tot_size, and filling in new_fmt.
	 */

	*bp_new_fmt = new_fmt = (char *)malloc(strlen(format) + 1);
	last_alloc = 10;
	order = (int *)malloc((size_t)(10 * sizeof(int)));
	if (new_fmt == 0 || order == 0) {
		errno = ENOMEM;
		rcode = -1;
		goto ret;
	}

	for ( ; ; ) {
		while ((*new_fmt = *format) != '\0' && *new_fmt != '%' ) {
			new_fmt++;
			format++;
		}

		fcode = *format;
		if (fcode == '\0') { /* end of format; normal return */
			break;    /* Now do the get_args loop */
			}
		/*
		 *	% has been found.
		 *	First extract digit$ from format and compute arg_index.
		 *	Next parse the format specification.
		 *	Skip, if * is encountered.
		 */

		if ((fcode = *++format) == '*') {
			++new_fmt;
			continue; 
		} else
			if(fcode == '%')
				{ /* Assign it to newformat and go get next char*/
				*++new_fmt=fcode;
				new_fmt++;
				format++;
				continue;
				}
		        else 
			    if (!isdigit(fcode)) {
				/* p81668
				 * The conversion spec is not of the form
				 * %n$.  There is probably a $ in the format
				 * string that is part of a literal, not 
				 * a conversion spec.  Return here and let
				 * _doscan try to sort it out.
				 */
				rcode = 1; 
				goto ret;
			    }
		            else 
				/* p82406
				 * the digit should be a decimal integer; 
				 * a '0' (zero) indicates an octal number
				 * which should fail.
				 */
				if (fcode == '0') {
				    rcode = -1;
				    goto ret;
				}
				else
				    --format;

		fcode = *++format;
		for (arg_index = 0; isdigit(fcode); fcode = *++format)
			arg_index = arg_index * 10 + tonumber(fcode);

		if (arg_index > last_alloc) {
			last_alloc += 10;
			order= (int *)realloc((void *)order, (size_t)(last_alloc * sizeof(int)));
			if (order == 0) {
				rcode = -1;
				goto ret;
			}
		}
		arg_index--;    /* the arrays are zero based */
		order[arg_count] = arg_index;

		for ( ; ; ) { /* Scan the <flags> */
			switch (fcode = *++new_fmt = *++format) {
			case ' ':
			case 'B':
			case 'N':
				continue;
			}
			break;
		}

		/* Scan the field width */
		fcode = *format;
		while (isdigit (fcode)) {
			*new_fmt++ = *format++;
			fcode = *format;
			}

		/* Scan the length modifier */
		length = 0;
		switch (*format) {
		case 'L':
		case 'l':
		    length++;
		    *new_fmt++ = *format++;	/* copy and advance */
		    if ((*format == 'l') || (*format == 'L')) {	/* 64890: 64-bit */
			length++;
			*new_fmt++ = *format++;	/* copy and advance */
		    };
		    break;
		case 'h':
			*new_fmt++ = *format++;
		}

		switch (fcode = *new_fmt++ = *format++) {

		case 'd':
		case 'u':
		case 'o':
		case 'X':
		case 'x':
		case 'E':
		case 'p':
		case 'e':
		case 'F':
		case 'f':
		case 'G':
		case 'g':
		case 'c':
		case 's':
		case 'S':
			break;

		case '[': {
		        /* a string of characters follows ending with a ']' */
			int loop_cnt;
			register mbleng;
			register mb_cur_max = MB_CUR_MAX;

			while (*format != '\0' && *format != ']') {
				loop_cnt = MBLEN(format);
				for (i=0; i < loop_cnt; i++)
					*new_fmt++ = *format++;
			}
			fcode = *new_fmt = *format;
			if (*format == ']') {
				new_fmt++;
				format++;
			}
			break;
		}

		/* case '%':   */
		default:
			break;

		case '\0': { /* unexpected end of format; return error */
			rcode = -1;
			goto ret;
			}

		}
		arg_count++;
	}

	/* Reorder ARGS Loop:  allocate char *new_args;
                               loop through order array to index into
			       size and args and copy XX size bytes
			       from *args[i] to new_args */
	{
	    int     arg_no;         /* argument order used to index in size */
				    /* and pargs                            */

            if(arg_count == 0)  goto ret;

	    ptr_size = sizeof (int *);
	    tot_size = ptr_size * arg_count;
	    *bp_new_args = new_args = malloc((size_t)tot_size);
	    if (new_args == 0) {
		    errno = ENOMEM;
		    rcode = -1;
		    goto ret;
	    }
	    for (arg_no = 0; arg_no < arg_count; arg_no++) {
		    memcpy ((void *)new_args, (void *)(args+(order[arg_no]*ptr_size)), (size_t)ptr_size);
		    new_args += ptr_size;
	    }
ret:
	    if (order != 0)
		    free ((void *)order);
	    return(rcode);
	}
}

/* Routines to keep track of chars read from input stream  */
static wint_t getcc(iop, icp)
FILE *iop;
int *icp;
{
	wint_t wc;
	register mbleng;
	char mb_buf[MB_LEN_MAX+1];

	if ((wc = getwc(iop)) != WEOF) {
		mbleng = wctomb (mb_buf, wc);
		*icp += mbleng;
	}
	return(wc);
}

static wint_t ungetcc(wc, iop, icp)
wint_t wc;
FILE *iop;
int *icp;
{
	register mbleng;
	char mb_buf[MB_LEN_MAX+1];

	if (wc != WEOF) {
		mbleng = wctomb (mb_buf, wc);
		*icp -= mbleng;
	}
	return(ungetwc(wc, iop));
}

/* settab - flags character c in the tab array. This array is a buffer
 *	to keep track of which characters are either accepted or ommited
 *	- used mainly for "%[scanset]" format.
 */
static void
settab(int *tab, int c, int t)
{
    int num, offset=0;
    int tmpmask=0;

    offset = c / 32;
    num = c % 32;
    if ( t != 0 ) {
	tmpmask = 1 << num;
	tab[offset] |= tmpmask;
    }
    else {
	int tmp=-1;
	tmpmask = ( (1 << num) ^ tmp );
	tab[offset] &= tmpmask;
    }

}

/* gettab - queries the tab array for character c. Returns 0 if the
 *	character is accepted and 1 if it is ommited.
 */
static
gettab(int *tab, int c)
{
    int num, offset=0;
    int tmpmask=0;
    int i, sz;

    offset = c / 32;
    num = c % 32;
    tmpmask = 1 << num;
    if ( tab[offset] & tmpmask )
	return(1);
    else
	return(0);
}

/* inittab - initializes the tab array with t ( ususally 0/1 )
 */
static void
inittab(int *tab, int t)
{
    int num, i;

    if (t)
	num = 0;
    else
	num = -1;

    for (i=0; i < MAXBUF ; i++) 
	tab[i] = num;
}

/* check_mb - checks the buffer buf for a valid wide character. If the
 *	next character is a valid wide char ( wclen > 1 ), we will put
 *	the value of the wide character in c and return 1; otherwise,
 *	we return 0.
 */
static
check_mb(char *buf, int *c)
{

    wchar_t	wc;
    int		wclen;

    if ( (wclen = mbtowc(&wc, buf, MB_CUR_MAX)) == -1 ) 
	return(0);
    if (wclen > 0) {
	*c = wc;
	return(wclen);
    }
    return(0);
}
