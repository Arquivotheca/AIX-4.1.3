static char sccsid[] = "@(#)%M  1.15  src/bos/usr/bin/adb/output.c, cmdadb, bos411, 9428A410j  5/26/94  16:30:02";
/*
 * COMPONENT_NAME: (CMDADB) assembler debugger
 *
 * FUNCTIONS: adbpr, charpos, convert, flushbuf, iclose, newline, oclose,
 *	      printc, printdate, printflush, printlong, prints, printulong,
 *	      prompt
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*              include file for message texts          */
#include "adb_msg.h" 
extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */
#include "defs.h"
#ifndef _NO_PROTO
#include <stdarg.h>
#else
#include <varargs.h>
#endif

LOCAL char printbuf[MAXLIN];
LOCAL char *printptr = printbuf;
LOCAL char *digitptr;
LOCAL int  pcolm = 0;
LOCAL int  vcolm = 0;

#ifdef _NO_PROTO
LOCAL void printdate();
LOCAL void printlong();
LOCAL void printulong();
LOCAL int  convert();
#else
LOCAL void printdate(time_t);
LOCAL void printlong(long, int);
LOCAL void printulong(unsigned long, int);
LOCAL int  convert(STRING *);
#endif
LOCAL void printflush();

LOCAL void printflush()
{
    if (-1 == write(outfile, printbuf, (unsigned)(printptr - printbuf)))
	perror( "printflush: write()" );
    printptr = printbuf;
}

#ifdef _NO_PROTO
void printc(c)
char c;
#else
void printc(char c)
#endif
{

    if (mkfault)
	return;
    if (c == '\0') {
	;
    } else if (c == '\t') {
	vcolm = (vcolm | 07) + 1;
    } else if (c == ' ') {
	++vcolm;
    } else if (c == '\n') {
	*printptr++ = c;
	printflush();
	pcolm = vcolm = 0;
    } else {
	while ((pcolm | 07) < vcolm) {
	    *printptr++ = '\t';
	    if (printptr >= &printbuf[MAXLIN]) printflush();
	    pcolm = (pcolm | 07) + 1;
	}
	while (pcolm < vcolm) {
	    *printptr++ = ' ';
	    if (printptr >= &printbuf[MAXLIN]) printflush();
	    ++pcolm;
	}
	*printptr++ = c;
	if (printptr >= &printbuf[MAXLIN]) printflush();
	if (c == '\b') {
	    --pcolm;
	    --vcolm;
	} else if (isprint((unsigned char)c)) {
	    ++pcolm;
	    ++vcolm;
	}
    }
}

int charpos()
{
    return (vcolm);
}

void flushbuf()
{
    if (vcolm != 0)
	printc('\n');
}

LOCAL void printdate(tvec)
time_t tvec;
{
    register int i;
    STRING timeptr;
    timeptr = (STRING)ctime(&tvec);
    for (i = 20; i < 24; i++)
	*digitptr++ = *(timeptr + i);
    for (i = 3; i < 19; i++)
	*digitptr++ = *(timeptr + i);
}

void prints(s)
char *s;
{
    adbpr("%s", s);
}

void newline()
{
    printc('\n');
}

LOCAL int convert(cp)
STRING *cp;
{
    char c;
    int  n;

    n = 0;
    while (((c = *(*cp)++) >= '0') && (c <= '9'))
	n = n * 10 + c - '0';
    (*cp)--;
    return (n);
}

LOCAL void printlong(n, base)
long n;
int base;
{
                  /* don't want signed hex, octal or binary */
    if ((n < 0) && (base != 16) && (base != 8) && (base != 2)) { 
	*digitptr++ = '-';
	printulong((unsigned long)-n, base);
    } else {
	printulong((unsigned long)n, base);
    }
}

LOCAL void printulong(n, base)
unsigned long n;
int base;
{
    int d = n % base;
    if ((n /= base) != 0) printulong(n, base);
    *digitptr++ = d + (d < 10 ? '0' : 'a'-10);
}

void iclose()
{
    if (infile != 0) {
	if (-1 == close(infile))
	    perror( "iclose: close()" );
	infile = 0;
    }
}

void oclose()
{
    if (outfile != 1) {
	flushbuf();
	if (-1 == close(outfile))
	    perror( "oclose: close()" );
	outfile = 1;
    }
}

/*
 * The prompt goes to descriptor zero.  This is intended as a feature.
 * This works because the terminal is actually open in RW mode (init does
 * so, and calls dup twice).  It might be more elegant to use a separate
 * descriptor, as returned by open(ttyname(0), 1).
 */
void prompt()
{
    if (infile == 0)
	(void)write(0, promptstr, (unsigned)strlen(promptstr));
}

#ifndef _NO_PROTO
/*VARARGS1*/
void 
adbpr(STRING fptr, ...)
#else
#if STRICT_VARARGS
/*VARARGS*/
void 
adbpr(va_alist) va_dcl
#else
/*VARARGS1*/
void 
adbpr(fptr, va_alist) STRING fptr; va_dcl
#endif
#endif
{
#if STRICT_VARARGS
    STRING  fptr;
#endif
    va_list vptr;
    STRING  s;
    short   width;
    short   prec;
    char    c;
    char    adj;
    int     n;
    char    digits[64];

#ifndef _NO_PROTO
    va_start(vptr, fptr);
#else
    va_start(vptr);
#if STRICT_VARARGS
    fptr = va_arg(vptr, STRING);
#endif
#endif
#define Nextc   (char)va_arg(vptr, int)
#define Nexth   (short)va_arg(vptr, int)
#define Nextuh  (unsigned short)va_arg(vptr, int)
#define Nexti   (int)va_arg(vptr, int)
#define Nextl   va_arg(vptr, long)
#define Nextul  va_arg(vptr, unsigned long)
#define Nexts   va_arg(vptr, STRING)
#if FLOAT
#define Nextf   va_arg(vptr, double)
#endif

    while (c = *fptr++) {
	if (c != '%')
	    printc(c);
	else {
	  if (*fptr == '-' ) {
	     adj = 'l';
	     fptr++;
	  }
	  else 
            if (*fptr == '~' ) {
		adj = '0';
		fptr++;
	    }
	    else
		adj = 'r';
	    width = convert(&fptr);
	    if (*fptr == '.') {
		fptr++;
		prec = convert(&fptr);
	    }
	    else
		prec = -1;
	    digitptr = digits;
	    s = NULL;
	    switch (c = *fptr++) {
	    case 'd':
		printlong((long)Nexth, 10);
		break;
	    case 'u':
		printulong((unsigned long)Nextuh, 10);
		break;
	    case 'o':
		printulong((unsigned long)Nextuh, 8);
		break;
	    case 'x':
		printulong((unsigned long)Nextuh, 16);
		break;
	    case 'r':
		printlong((long)Nexth, radix);
		break;
	    case 'R':
		printlong(Nextl, radix);
		break;
	    case 'Y':
		printdate((time_t)Nextl);
		break;
	    case 'D':
		printlong(Nextl, 10);
		break;
	    case 'U':
		printulong(Nextul, 10);
		break;
	    case 'O':
		printulong(Nextul, 8);
		break;
	    case 'X':
		printulong(Nextul, 16);
		break;
	    case 'Z':
		printulong(Nextul, 16);
		printulong(Nextul, 16);
		break;
	    case 'c':
		printc(Nextc);
		break;
	    case 's':
		s = Nexts;
		break;
#if FLOAT
	    case 'f':
	    case 'F':
		(void)sprintf(s = digits, "%+.16e", Nextf);
		prec = -1;
		break;
#endif
	    case 'g':
		(void)sprintf(s = digits, "%g", Nextf);
		break;
	    case 'a':
		s = reglist[Nexth].rname;
		break;
	    case 'A':
		n = Nexth;
		if (n == 10) {
		    s = "mq";
		} else if (n == 15) {
		    s = "cs";
		} else {
		    printc('s');
		    printc('r');
		    printulong((unsigned long)n, 10);
		}
		break;
	    case 'i':
		printlong((long)Nexth, radix);
		n = Nexth;
		if (n != 0) {
		    register char *rp = reglist[n].rname;
		    *digitptr++ = '(';
		    while (*rp != '\0') *digitptr++ = *rp++;
		    *digitptr++ = ')';
		}
		break;
	    case 'm':
		break;
	    case 'M':
		width = Nexti;
		break;
	    case 'T':
		width = Nexti;
		/* fall through */
	    case 't':
		if (width)
		    width -= charpos() % width;
		break;
	    default:
		/* Should not happen */
		printc(c);
	    }

	    if (s == NULL) {
		*digitptr = '\0';
		s = digits;
	    }
	    n = strlen(s);
	    n = (prec < n && prec >= 0 ? prec : n);
	    width -= n;
	    if (adj == '0') {
		while (width-- > 0)
		    printc('0');
	    }
	    if (adj == 'r') {
		while (width-- > 0)
		    printc(' ');
	    }
	    while (n--)
		printc(*s++);
	    while (width-- > 0 )
		printc(' ');
	    digitptr = digits;
	}
    }
}
