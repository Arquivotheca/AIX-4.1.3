static char sccsid[] = "@(#)14  1.16  src/bos/usr/bin/uniq/uniq.c, cmdfiles, bos412, 9446C 11/14/94 16:47:15";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: uniq
 *
 * ORIGINS: 3, 18, 26, 27
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
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 Release 1.0
 *
 * static char rcsid[] = "RCSfile: uniq.c,v  Revision: 2.5  (OSF) Date: 90/10/07 17:17:33 ";
 */
#define _ILS_MACROS
#include <stdio.h>
#include <locale.h>
#include <ctype.h>
#include <limits.h>
#include <sys/types.h>
#include <stdlib.h>
#include "uniq_msg.h"

#define MSGSTR(n,s)	catgets(catd,MS_UNIQ,n,s)

/* For <blank> detection: define macros for isblank() for SBCS and
 * iswblank() for wchar with MBCS. POSIX defines function of -f option
 * in terms of <blank> character class. Standard bindings do not provide
 * isblank() or iswblank(), so uniq must provide its own.
 */
#define isblank(c)  	is_wctype(c, _ISBLANK)
#define iswblank(wc) 	is_wctype(wc, _ISBLANK)

static int mbcodeset;  /* 0=current locale SBCS, 1=current locale MBCS */

static nl_catd catd;

static int	fields = 0;
static int	letters = 0;
static int	repeat = 0;
static char	*skip();
static int	cflag = 0;
static int	dflag = 0;
static int	uflag = 0;
static char 	*infile;	/* InFile name string if any */
static char 	*outfile;       /* OutFile name string if any */

/*
 * NAME: uniq [-c | -d | -u] [-f fields] [-s chars] [-num] [+num]
 *            [input_file [output_file]]
 *                                                                    
 * FUNCTION: Deletes repeated lines in a file.  Repeated lines must be
 *           in consecutive lines in order for them to be found.
 *         -c     Precedes each output line with the number of times it
 *                appear in the file.
 *         -d     Displays only the repeated lines.
 *         -u     Displays only the unrepeated lines.
 *	   -f num Skips over the first num fields.
 *         -num   Equivalent to  -f num.
 *         -s num Skips over the first num characters.
 *         +num   Equivalent to  -s num.
 *                                                                    
 */  
main(argc, argv)
int argc;
char *argv[];
{
	static char b1[LINE_MAX+1], b2[LINE_MAX+1];
	register char *b1ptr, *b2ptr, *pb1, *pb2, *tmp;
	char *arg;	/* next option or operand string */
	int c;		/* option character */
	int needtoskip = 0; /* flag used to check if skip is needed */
	int numval;	/* numeric value of option value */
	char *cp;	/* for processing numeric option values */
	char optname[] = "-?";  /* for diagnostics */

	(void) setlocale(LC_ALL,"");
	catd = catopen(MF_UNIQ, NL_CAT_LOCALE);
	mbcodeset = (MB_CUR_MAX > 1);

		/* Command parsing: follow POSIX guidelines and also
		 * allow old POSIX "obsolescent" versions.
		 */
	do {
		/* Two-stage processing of each potential option character:
		 * 1. If it is the first character of an obsolescent option
		 *    that does not follow getopt() conventions, process it
		 *    manually and update getopt() pointers to next possible
		 *    option character.
		 * 2. Otherwise process it through getopt() for normal
		 *    option processing.
		 */
		if(optind >= argc) break;
		arg = argv[optind];
		c = arg[0];
		if ( c == '+') {
			/* Part 1 of 3: Obsolescent +m option */
			cp = &arg[1];
			numval = (int)strtoul(cp,&cp,10);
			if (numval < 0)
				OutOfRange(c);
			else if (numval > LINE_MAX)
				numval=LINE_MAX;
			if (*cp == '\0')
				letters = numval;			
			else badnumb("+");
			optind++;
		} else if (c == '-' && strlen(arg) > 1 && iswdigit((wchar_t)arg[1])) {
			/* Part 2 of 3: Obsolescent -n option */
			cp = &arg[1];
			numval = (int)strtoul(cp,&cp,10);
			if (numval < 0)
				OutOfRange(c);
			else if (numval > LINE_MAX)
				numval=LINE_MAX;
			if (*cp == '\0')
				fields = numval;			
			else badnumb("-");
			optind++;
		} else {
			/* Part 3 of 3: Normal POSIX command syntax option */
			/* Implemented to POSIX 1003.2/Draft 10 conventions*/
			c = getopt(argc,argv,"cdf:us:");
			switch (c) {
				case 'c':
					cflag = 1;
					break;
				case 'd':
					dflag = 1;
					break;
				case 'u':
					uflag = 1;
					break;
				case 'f':
				case 's':
					cp = optarg;
					numval = (int)strtoul(cp,&cp,10);
					if (numval < 0)
						OutOfRange(c);
					else if (numval > LINE_MAX)
						numval=LINE_MAX;
					if (*cp == '\0')
						if (c == (int)'f')
							fields = numval;
						else letters = numval;
					else {
						optname[1] = (char)c;
						badnumb(optname);
					}
					break;
				case -1:
					break;
				default:
					(void) usage();
			} /* end switch(c) */
		} /* end POSIX syntax option */
	} while (c != -1); /* end do */

	if (cflag + dflag + uflag > 1)
		(void) usage();
	infile = outfile = "";
	switch(argc-optind) {
		case 2:
			outfile = argv[optind+1];
		case 1:
			infile  = argv[optind];
			break;
		case 0: break;
		default:
			fprintf(stderr,MSGSTR(BADOPT,"uniq: too many files\n"));
			(void) usage();	/* Too many File operands */
	} /* switch(argc-optind) */

#ifdef DEBUG_OPTS
fprintf(stderr,"uniq option dump: fields=%d  letters=%d  InFile=\"%s\"  OutFile=\"%s\"\n",fields,letters,infile,outfile);
#endif

		/* Open named InFile for standard input if specified.*/
	if (strcmp(infile,"-") != 0 && strlen(infile)) {
		if (strcmp(infile,outfile) == 0) {
			fprintf(stderr,MSGSTR(SAMEFIL,
			"uniq: input and output files can not be the same.\n"));
			exit(1);
		}
		if (freopen(infile,"r",stdin)==NULL) {
			fprintf(stderr,MSGSTR(OPERR,"uniq: cannot open %s\n"),infile);
			exit(1);
		}
	}
		/* Open named OutFile for standard output if specified.*/
	if (strlen(outfile))
		if (freopen(outfile,"w",stdout) == NULL) {
			fprintf(stderr,MSGSTR(CRERR,"uniq: cannot create %s\n"),outfile);
			exit(1);
		} 

	b1ptr = b1;
	b2ptr = b2;
	needtoskip = (fields || letters);
	if(!gline(b1ptr)) nontext(infile);
	pb1 = needtoskip ? skip(b1ptr) : b1ptr;
	while (gline(b2ptr)) {
		pb2 = needtoskip ? skip(b2ptr) : b2ptr;
		if(strcmp(pb1,pb2)) {
			pline(b1ptr);     /* print current line */
			repeat = 0;
			tmp = b1ptr;
			b1ptr = b2ptr;
			b2ptr = tmp;
			pb1 = pb2;
		} 
		else repeat++;
	}
	pline(b1ptr);
	exit(0);
}

/*
 * NAME: gline
 *                                                                    
 * FUNCTION: get the next line from the file checking for EOF.
 *                                                                    
 * RETURN VALUE DESCRIPTION:
 *         0 - unsuccessful
 *         1 - successful 
 */  
static gline(buf)
register char *buf;
{
	register char *bufmax;

	bufmax = buf + LINE_MAX - 1;
	if (!fgets(buf,LINE_MAX+1,stdin)) return(0);
	while(*buf != '\n' && buf < bufmax) {
		if (*buf == '\0') nontext(infile);
		buf++;
	}
	if (buf == bufmax && *buf != '\n') nontext(infile);
	return(1);
}

/*
 * NAME: pline
 *                                                                    
 * FUNCTION: check mode and print current line if needed.
 */  
static pline(buf)
char *buf;
{
	if (uflag) {
		if (repeat) {
			repeat = 0;
			return;
		}
	}
	else if (dflag) {
		if (!repeat) return;
	}
	else if (cflag) printf("%4d ",repeat+1);
	repeat = 0;
	fputs(buf,stdout);
}

/*
 * NAME: skip
 *                                                                    
 * FUNCTION:  Skip num fields or num characters.
 *
 * RETURN VALUE DESCRIPTION:  return the rest of the string.
 *   
 */  
static char *
skip(s)
char *s;
{
	int nf, nl;
	wchar_t ws;
	wchar_t *wsp = &ws;
	int chrlen;

	nf = nl = 0;
	if (mbcodeset) { /* skip fields (-f) and letters (-s) in MBCS */
		while(nf++ < fields) {
			while ((chrlen=mbtowc(wsp,s,MB_CUR_MAX))>0 && ws != L'\0' && iswblank(ws))
				s += chrlen;
			while ((chrlen=mbtowc(wsp,s,MB_CUR_MAX))>0 && ws != L'\0' &&!iswblank(ws))
				s += chrlen;
		}
		while(nl++ < letters) {
			chrlen=mbtowc(wsp,s,MB_CUR_MAX);
			if (chrlen==0 || ws == L'\0')
				break;
			else s += chrlen;
		}
	} else {	 /* skip fields (-f) and letters (-s) in SBCS */
		while(nf++ < fields) {
			while (*s != '\0' && isblank(*s))
				s++;
			while (*s != '\0' && !isblank(*s))
				s++;
		}
		while(nl++ < letters && *s != 0) 
				s++;
	}
	return(s);
}

static nontext(s)
char *s;
{
	fprintf(stderr,MSGSTR(NONTEXT,"uniq: %s is not a text file.\n"),s);
	exit(1);
}

static badnumb(c)
char *c;
{
	fprintf(stderr,MSGSTR(BADNUM,"uniq: bad number for option %s\n"),c);
	(void) usage();
}

static usage()
{
	fprintf(stderr,MSGSTR(USAGE,
		"Usage: uniq [-c | -d | -u] [-f Fields] [-s Chars] [-Fields] [+Chars] [Input_file [Output_file]]\n"));
	exit(1);
}

static OutOfRange(c)
char	c;
{
	fprintf(stderr,MSGSTR(OUTOFRANGE,"uniq: The %c option requires a positive integer for the fields parameter.\n"),c);
	exit(1);
}
