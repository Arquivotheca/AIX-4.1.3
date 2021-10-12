static char sccsid[] = "@(#)72  1.14  src/bos/usr/bin/units/units.c, cmdmisc, bos41B, 9504A 1/4/95 14:12:16";
/*
 * COMPONENT_NAME: (CMDMISC) miscellaneous commands
 *
 * FUNCTIONS: units
 *
 * ORIGINS: 3, 26, 27
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
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */
/*
 *
 *  NAME:     units [-][filename]
 *  OPTIONS:  -         prints the conversion factors.
 *            filename  takes the conversion factors from this file instead
 *                      of from /usr/share/lib/unittab.
 *                                                                     
 *  FUNCTION: Converts units in one measure to equivalent units in
 *            another measure.
 */                                                                    

#include <stdio.h>
#include <sys/signal.h>
#include <stdlib.h>
#include <locale.h>
#include <langinfo.h>
#include "units_msg.h"
static nl_catd catd;

#define MSGSTR(Num, Str) catgets(catd, MS_UNITS, Num, Str)

#define	NDIM	10
#define	NTAB	601
#define MAXRADIX 10	/* maximum number of bytes in radix */
static char	*dfile	= "/usr/share/lib/unittab";      /* the default file. */
static wchar_t	*unames[NDIM];
double	getflt();
int	fperr(void);
void    units();
wchar_t get();
struct	table	*hash();
struct unit
{
	double	factor;
	int dim[NDIM];
};

static struct table
{
	double	factor;
	int dim[NDIM];
	wchar_t	*name;
} table[NTAB];
static wchar_t	names[NTAB*10];
static struct prefix
{
	double	factor;
	char	*pname;
} prefix[] = 
{
	1e-18,	"atto",
	1e-15,	"femto",
	1e-12,	"pico",
	1e-9,	"nano",
	1e-6,	"micro",
	1e-3,	"milli",
	1e-2,	"centi",
	1e-1,	"deci",
	1e1,	"deka",
	1e2,	"hecta",
	1e2,	"hecto",
	1e3,	"kilo",
	1e6,	"mega",
	1e6,	"meg",
	1e9,	"giga",
	1e12,	"tera",
	0.0,	0
};
static FILE	*inp;
static int	fperrc;    /* has a floating-point exception occured? */ 
static wchar_t	peekc = 0;
static int	dumpflg;   /* should the conversion factors be printed? */
static char	*fc_radix;	/* radix in file code */
static wchar_t	wc_radix[MAXRADIX];	/* radix in wide characters */
static int	initflag;
static wchar_t	get_buf[MAXRADIX];	/* buffer used by get & unget */

main(argc, argv)
char *argv[];
int argc;
{
	register int i;
	register char *file;
	struct unit u1, u2;
	double f;
	int cont;


	(void) setlocale (LC_ALL, "");
        catd = catopen(MF_UNITS, NL_CAT_LOCALE);

        /* if more than one argument is given, and that argument is
         * a minus sign, then set the dumpflg.  This flag will later
         * signify to the program that a dump of all the conversion
         * factors is to be printed.                              */
	if(argc>1 && *argv[1]=='-') {
		argc--;
		argv++;
		dumpflg++;
	}
        /* dfile contains the address of /usr/share/lib/unittab and, after
         * this assignment, file contains the address of dfile.      */
	file = dfile;
	/* if no argument is given, then /usr/share/lib/unittab is used.
         * if an argument is given, and that argument is not a minus,
         * then it is used as the name of the file from which the unit 
         * definitions are taken.   
         * If that file does not exist, then "no table" is printed.  */
	if(argc > 1)
		file = argv[1];
	if ((inp = fopen(file, "r")) == NULL) {
		fprintf(stderr,MSGSTR(NOTABLE,"Table, %s, was not found.\n"),file);
		exit(1);
	}

	/* Signal 8 indicates that a floating point exception has     
	 * occured.                                                    */
	signal(SIGFPE, (void (*)(int))fperr);

	/* fetch radix character(s) and convert it(them) to 	       *
	 * process code						       */
	fc_radix = nl_langinfo(RADIXCHAR);
	mbstowcs(wc_radix, fc_radix, sizeof(wc_radix)/sizeof(wchar_t));

	/* While in init() the getflt() subroutine expects the	       *
	 * standard U.S. english radix '.'. The reason for this	       *
	 * is that during initialization getflt() is reading	       *
	 * the hardcoded values in the file "/usr/share/lib/unittab"   *
	 * where '.' is used, because U.S. numbers are already	       *
	 * present, and a uniform radix is required, even if non-U.S.  *
	 * definitions are added there later on. After initialization, *
	 * however, getflt() is reading from standard input and	       *
	 * must be able to interpret the culture specific radix	       *
	 * in use. This is indicated by setting initflag to 1.	       */
	initflag = 0;
	init();
	initflag = 1; /* initialization completed */

	/* This loop reads in the data, calls the procedures which
	 * calculate the answer and prints the answer.  If the 
	 * labels on the input are not compatible, it prints the
	 * word conformability, along with the base units of the two
	 * types.  
	 * NOTE:  the only way to exit this program is by striking a 
	 *        control character(CNTL-D).                           */
        while(1) {
		fperrc = 0;
		printf(MSGSTR(UHAVE, "you have: "));
		if(convr(&u1))
			continue;
		if(fperrc) {
		    fprintf(stderr,MSGSTR(UNDOVFLOW,"underflow or overflow\n"));
			continue;
		}

		do {
			printf(MSGSTR(UWANT, "you want: "));
		} while (convr(&u2) != 0);
		cont = 1;
		for(i=0; i<NDIM; i++) {
			if(u1.dim[i] != u2.dim[i]) {
				if(fperrc) 
	                   fprintf(stderr,MSGSTR(UNDOVFLOW, "underflow or overflow\n"));
				else {
					/* label not compat. */
				printf(MSGSTR(CONFO, "conformability\n"));
					units(&u1);
					units(&u2);
				}
				cont = 0;
				break;
			}
		}
		/* At this point, the factors are in terms of the base
		 * units and the labels have been determined to be 
		 * compatible.  Now, we divide and print the correct
		 * answers. */  
		if(cont) {
			f = u1.factor/u2.factor;
			if(fperrc)
	                   fprintf(stderr,MSGSTR(UNDOVFLOW,"underflow or overflow\n"));
			else {
				printf("\t* %e\n", f);
				printf("\t/ %e\n", 1./f);
			}		
		}
	}
}

/* 
 * NAME: units
 *
 * FUNCTION:  This function is called when either a dump is being printed
 *	      ("units -") or when the labels are not compatible(gal vs. ft).
 *            It prints out the factor and calls a routine which prints
 *            the labels.
 *
 * RETURN VALUE:  none.
 *
 */
static void units(up)
struct unit *up;
{
	register struct unit *p;
	register int f, i;

	p = up;
	printf("\t%e ", p->factor);
	f = 0;
	for(i=0; i<NDIM; i++)
		f |= pu(p->dim[i], i, f);
	if(f&1) {
		putchar('/');
		f = 0;
		for(i=0; i<NDIM; i++)
			f |= pu(-p->dim[i], i, f);
	}
	putchar('\n');
}
   
/* 
 * NAME: pu
 *
 * FUNCTION: Prints the label.
 *
 * RETURN VALUE:  0)  Searching for end of string.
 *                1)  Number less than zero.
 *                2)  Successfully printed.
 *
 */
static pu(u, i, f)
{
	char buff[64];   

	if(u > 0) {
		if(f&2)
			putchar('-'); /* Put hyphen btwn. units. */
		if(unames[i]) {
			wcstombs(buff, unames[i], 64);	 
			printf("%s", buff);			  
			/*printf("%S", unames[i]);		*/
			/*putws(unames[i]);			*/
		} else {
			printf("*%c*", i+'a');
		}
		if(u > 1)
			putchar(u+'0'); /* Print exponent. */
		return(2);
	}
	if(u < 0)  /* Negative exponent. Indicate 2nd pass needed. */
		return(1);
	return(0);
}

/* 
 * NAME: convr
 *
 * FUNCTION: Reads in the factor and the label(ex: 1.2-5 lb) until a
 *           new-line character is found.
 *
 * RETURN VALUE:  0) the label is valid(found in the hash table).
 *                1) the label is not valid.
 */
static convr(up)
struct unit *up;
{
	register struct unit *p;
	register wchar_t c;
	register wchar_t *cp;
	wchar_t name[20];
	int j, den, err;

	p = up;
	for(j=0; j<NDIM; j++)
		p->dim[j] = 0;
	p->factor = getflt();        /* Reads in the factor. */
	if(p->factor == 0.)
		p->factor = 1.0;     /* If no factor is provided, make it 1. */
	err = 0;
	den = 0;
	cp = name;

	while(1) {
		switch(c=get()) {
		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
		case L'-':
		case L'/':
		case L' ':
		case L'\t':
		case L'\n':
			if(cp != name) {
				*cp++ = 0;
				cp = name;
				err |= lookup(cp, p, den, c); /* Label valid? */
			}
			if(c == L'/')
				den++;
			if(c == L'\n')
				return(err);
			break;
		default:
			*cp++ = c;   /* These chars are assumed to be a
				      * valid part of the input. */
		}
	}
}

/* 
 * NAME: lookup
 *
 * FUNCTION:  Finds the label in the hash table, and converts
 *            it to the base label (ex: gal is converted to m3). 
 *            If it can't find the label, it prints "cannot recognize".   
 *
 * RETURN VALUE:  0) the label was found.
 *                1) the label was not found (cannot recognize).
 */
static lookup(name, up, den, c)
wchar_t *name;
struct unit *up;
int den;
wchar_t c;
{
	register struct unit *p;
	register struct table *q;
	register int i, cont;
	char *cp1, *cp2, buff[128];
	wchar_t *wcp2, wch;
	double e;       /* If a prefix was used, "e" contains the multiplier. */

	p = up;
	e = 1.0;

        while(1) {
		q = hash(name);
		if(q->name) {
			while(1) {
				if(den) {
					p->factor /= q->factor*e;
					for(i=0; i<NDIM; i++)
						p->dim[i] -= q->dim[i];
				}
				else {
					p->factor *= q->factor*e;
					for(i=0; i<NDIM; i++)
						p->dim[i] += q->dim[i];
				}
				if(c >= L'2' && c <= L'9') 
					c--;
				else return(0);
			}
		}
		cont = 1;
		/* If a prefix was used, then multiply "e" by the appr. amt. */
		for(i=0; cp1 = prefix[i].pname; i++) {
			int len = 0;

			len = strlen(cp1);
			wcstombs(buff, name, 128);
			cp2 = buff;
			if(strncmp(cp1, cp2, len) == 0) {
				e *= prefix[i].factor;
				name += len;
				cont = 0;
				break;
			}
		}
		if(cont) {
			for(wcp2 = name; *wcp2; wcp2++);
			if(wcp2 > name+1 && *--wcp2 == L's') 
				*wcp2 = 0;
			else {
	                fprintf(stderr,MSGSTR(CANTREC, "cannot recognize %S\n"), name);
				return(1);
			}
		}
	}
}

/* 
 * NAME: equal
 *
 * FUNCTION:  Verifies that two strings are equal.
 *
 * RETURN VALUE:  0) the strings are not equal.
 *                1) the strings are equal.
 */
static equal(s1, s2)
wchar_t *s1, *s2;
{
	register wchar_t *c1, *c2;

	c1 = s1;
	c2 = s2;
	while(*c1++ == *c2)
		if(*c2++ == 0)
			return(1);
	return(0);
}

/* 
 * NAME: init
 *
 * FUNCTION:  This function initializes the hash table: 
 *            1)  Sets name equal to "*x*" where "x" is a character between 
 *	          "a" and "j".
 *            2)  Sets the factor equal to 1.0.
 *            3)  Sets the i'th dimension equal to 1.     
 *            Reads data from the file into the hash table.
 *
 * RETURN VALUE:
 *
 */
static init()
{
	register wchar_t *cp;
	register struct table *tp, *lp;
	int i, j, f, t, cont;
	wchar_t c, *np;
	char buff[64];

	cp = names;                     /* cp contains the address of names,  */
			                /* so when *cp changes, names changes */
	for(i=0; i<NDIM; i++) {
		np = cp;                /* np will take on *cp's values.      */
					/* Now, when *cp changes, both names
					 * and np also change.                */
		*cp++ = L'*';
		*cp++ = i+L'a';
		*cp++ = L'*';
		*cp++ = 0;              /* names and np now equal "*a*"       */
		lp = hash(np);
		lp->name = np;
		lp->factor = 1.0;
		lp->dim[i] = 1;
	}
	lp = hash("");
	lp->name = cp-1;
	lp->factor = 1.0;

	while(1) {
		c = get();
		if(c == 0) {
			/* if the dumpflg was set, then print the conv.
			 * factors.  */
			if(dumpflg) {
				printf(MSGSTR(DUMP, "%d units; %d bytes\n\n"), i, cp-names);
				for(tp = &table[0]; tp < &table[NTAB]; tp++) {
					if(tp->name == 0)
						continue;
					/* printf("%S", tp->name); */
					wcstombs(buff, tp->name, 64);
					printf("%s", buff);
					units((struct unit *)tp);
				}
			} 
			fclose(inp);
			inp = stdin;
			return;
		}
		if(c == L'/') {
			while(c != L'\n' && c != 0)
				c = get();
			continue;	
		}
		if(c == L'\n')
			continue;
		np = cp;
		cont = 1;
		while(c != L' ' && c != L'\t') {
			*cp++ = c;
			c = get();
			/* if we're at the end of the input, then end init. */  
			if (c==0) {
				cont = 0;
				break;
			}
			if(c == L'\n') {
				cont = 0;
				*cp++ = 0;
				tp = hash(np);
				if(tp->name) {
					fprintf(stderr,MSGSTR(REDEF, "redefinition %S\n"), np);
					break;
				}	
				tp->name = np;
				tp->factor = lp->factor;
				for(j=0; j<NDIM; j++)
					tp->dim[j] = lp->dim[j];
				i++;
				break;
			}
		}
		if(cont) {
			*cp++ = 0;
			lp = hash(np);
			if(lp->name) {
				fprintf(stderr,MSGSTR(REDEF, "redefinition %S\n"), np);
				continue;
			}
			convr((struct unit *)lp);
			lp->name = np;
			f = 0;
			i++;
			if(lp->factor != 1.0)
				continue;
			for(j=0; j<NDIM; j++) {
				t = lp->dim[j];
				if(t>1 || (f>0 && t!=0)) {
					cont = 0;
					break;
				}
				if(f==0 && t==1) {
					if(unames[j]) {
						cont = 0;
						break;
					}
					f = j+1;
				}
			}
			if(cont) {
			 	if(f>0)
					unames[f-1] = np;
			}
		}
	}	
}


/* 
 * NAME: getflt
 *
 * FUNCTION:  Reads in a number a character at a time and converts it to 
 *            floating-point.
 *
 * RETURN VALUE:
 *
 */
static double
getflt()
{
	wchar_t c;
	register int i, dp;
	double d, e;
	int f;

	d = 0.;
	dp = 0;		/* dp tracks the placement of the
			 * decimal point.                 */
	do
		c = get();
	while(c == L' ' || c == L'\t');

        /*  If the character that's read in is an integer, then move the
	 *  dec pt of the total to the rt and add the number to it.       */
	while(1) {
		if(c >= L'0' && c <= L'9') {
			d = d*10. + c-L'0';
			if(dp)
				dp++;           
			c = get();
			continue;
		}
		/*  When the dec pt is reched, inc the dp var 	*
		 *  and read in the next character. */
		if( !initflag ) {
			/* we're in the initialization stage, the *
			 * radix may be assumed to be a '.'	*/
			if(c == L'.') {
				dp++;
				c = get();
				continue;	
			}
		} else {
			/* we're reading from standard output.	*
			 * The radix will depend on the locale.	*/

			 wchar_t *cp;

			 for(cp = wc_radix; *cp != L'\0'; cp++) {
				if(*cp != c)
					break;
				c = get();
			 }
			 if(*cp == L'\0') {
				/* found a radix */
				dp++;
				continue;
			 } else {
				/* A byte didn't match the radix. *
				 * Backup any input. This may be  *
				 * a units label of some kind.	  */
				if( cp != wc_radix ) {
					unget(c);
					while( --cp > wc_radix )
						unget(*cp);
					c = *wc_radix;
				}
			 }
		}
		if(dp)
			dp--;

		/* if there is an exp factor, increment the dp counter
		 * by that amount.   */
		if(c == L'+' || c == L'-') {
			f = 0;
			if(c == L'-')
				f++;
			i = 0;
			c = get();
			while(c >= L'0' && c <= L'9') {
				i = i*10 + c-L'0';
				c = get();
			}
			if(f)
				i = -i;
			dp -= i;
		}

		/* Use the value of dp to determine where to place the decimal
		 * within the number.  */
		e = 1.;
		i = dp;
		if(i < 0)
			i = -i;
		while(i--)
			e *= 10.;
		if(dp < 0)
			d *= e; else
			d /= e;
		if(c == L'|')
			return(d/getflt());
		/* peekc = c; */
		unget(c);
		return(d);
	}
}

/* 
 * NAME: get
 *
 * FUNCTION:  This function is used to gather information a character 
 *            at a time.  That info can come from a file or from stdin.
 *
 * RETURN VALUE:  0)  Reached the end of file.
 *                *)  The value read.
 *
 */
static wchar_t get()
{
	register wchar_t c;

	/*****************
	if(c=peekc) {
		peekc = 0;
		return(c);
	}
	*****************/
	if(peekc) {
		c = get_buf[--peekc];
		return(c);
	}
	c = getwc(inp);
	if (c == (wchar_t)WEOF) {
		if (inp == stdin) {
			printf("\n");
			exit(0);
		}
		return(0);
	}
	return(c);
}

/* 
 * NAME: unget
 *
 * FUNCTION:  This function is used to return a character
 *	      to the buffer used by get().
 *
 */

 static unget(c)
 wchar_t c;
 {
	get_buf[peekc++] = c;
 }

/* 
 * NAME: hash
 *
 * FUNCTION:  Provides a valid index into the hash table.
 *
 * RETURN VALUE:  the address of the table.
 *
 */
static struct table *
hash(name)
wchar_t *name;
{
	register struct table *tp;
	register wchar_t *np;
	register /* wint_t */ wchar_t h;

	h = 0;
	np = name;
	while(*np)
		h = h*57 + *np++ - L'0';
	if( ((int)h)<0) h= -(int)h;
	h %= NTAB;
	tp = &table[h];

	while(1)
	{
		if(tp->name == 0)
			return(tp);
		if(equal(name, tp->name))
			return(tp);
		tp++;
		if(tp >= &table[NTAB])
			tp = table;
	}
}

static fperr(void)
{

	signal(SIGFPE, (void (*)(int))fperr);
	fperrc++;
}
