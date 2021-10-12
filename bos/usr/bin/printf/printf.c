static char sccsid[] = "@(#)09	1.14  src/bos/usr/bin/printf/printf.c, cmdposix, bos41J, 9523C_all 6/8/95 10:38:57";
/*
 * COMPONENT_NAME: (CMDPOSIX) new commands required by Posix 1003.2
 *
 * FUNCTIONS: printf
 *
 * ORIGINS: 27, 71
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
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.1
 *
 */

#define _ILS_MACROS

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <nl_types.h>
#include <mbstr.h>
#include <stdlib.h>
#include <stdarg.h>
#include "printf_msg.h"

#define MAXSTR ARG_MAX		/* max string to be output */
static nl_catd catd;
#define MSGSTR(num,str) catgets(catd,MS_PRINTF,num,str)  

static void escwork(char *destin, char *source, int isarg);
static int doargs(char **fmtptr, char *args);
static int finishline(char *fmtptr);
static void p_out(const char *format, int type, ...);
static int convchar(char *, char **);

	/* types for p_out  (0 is default) */
#define	DBL	1

static int error = 0;
static int cflag = 0;
static char outstr[MAXSTR + 1];		/* output string */

main(int argc, char **argv)
{
	char *fmtptr, *restart;			/* pointer to format */
	int did_conv;

	(void) setlocale(LC_ALL,"");
	catd = catopen(MF_PRINTF, NL_CAT_LOCALE);

	/* handle -- argument for XOPEN standards compliance */
        if (strcmp(argv[1], "--") == 0) {
                argv++;
                argc--;
        }

	if (argv[1]) {
		fmtptr = argv[1];
		restart = argv[1];
	}
	else
	   	exit(0);

        /*
         * If no format specification, simply printf format string 
         */

	if (!strchr(fmtptr, '%')) {
		escwork(NULL, fmtptr, 0);
		exit(0);
	}

	/*
	 * Escape sequences have been translated.  Now process 
         * format arguments.
         */

	argv += 2;
	did_conv = 0;
	while (*argv && !cflag){
		int rc;

		errno = 0;

		if ((rc=doargs(&fmtptr, *argv)) == 1) {
				/* ending format string is a string */
			if (!did_conv)
				break;
			fmtptr = restart;
			did_conv = 0;
		} else if (rc == 2)
				/* invalid conversion */
			break;
		else {
			argv++;
			did_conv = 1;
		}
	}

	/*
       	 * Check to see if 'format' is done. if not transfer the
       	 * rest of the 'format' to stdout.
       	 */

	if (!cflag && strlen(fmtptr))
		finishline(fmtptr);
	exit(error);
}



/*
 * 	escwork
 *
 * This routine transforms octal numbers and backslash sequences to the
 * correct character representations and sends the resulting string 
 * to stdout.
 *
 */

void
escwork(char *destin, char *source, int isarg)
/* char *destin;		 	 pointer to destination */
/* char *source;			 pointer to source      */
/* int isarg;				 format/argument flag   */
{
	int mbcnt, j;
	wchar_t wd;	
	char *pos = destin;

	for(; *source; source += (mbcnt > 0) ? mbcnt : 1) {
		mbcnt = mbtowc(&wd, source, MB_CUR_MAX);
		if (mbcnt == 1 && wd == '\\') {
			/*
			 * process escape sequences
			 */
			switch(*++source) {
				case 'a':
					wd = '\a';
					break;
				case 'b':
					wd = '\b';
					break;
				case 'f':
					wd = '\f';
					break;
				case 'n':
					wd = '\n';
					break;
				case 'r':
					wd = '\r';
					break;
				case 't':
					wd = '\t';
					break;
				case 'v':
					wd = '\v';
					break;
				case '\\':
					wd = '\\';
					break;
				case '0': case '1': case '2': case '3':
				case '4': case '5': case '6': case '7':
					j = wd = 0;
					if ((isarg) && (*source == '0'))
						source++;
					while ((*source >= '0' &&
						*source <= '7') && j++ < 3) {
						wd <<= 3;
						wd |= (*source - '0');
						source++;
					}
					source--;
					break;
				case 'c':
					if (isarg) {
						cflag++;
						return;
					}
					/* else fall through */
				default:
					--source;
			}	/* end of switch */	
		}	/* end of if */
		if (destin) {
			mbcnt = wctomb(pos, wd);
			pos += (mbcnt > 0) ? mbcnt : 1;
		} else {
			putwc(wd, stdout);
		}
	}	/* end of for */
	if (destin)
		*pos++ = '\0';
	return;
}
					

/*
 *    doargs
 *
 * This routine does the actual formatting of the input arguments.
 *
 * This routine handles the format of the following form:
 *	%pw.df
 *		p:	prefix, zero or more of {- + # or blank}.
 *		w:	width specifier. It is optional.
 *		.:	decimal.
 *		d:	precision specifier.
 *                      A null digit string is treated as zero. 
 *		f:	format xXioudfeEgGcbs.
 *
 * The minimum set required is "%f".  Note that "%%" prints one "%" in output.
 * 
 * RETURN VALUE DESCRIPTION:	
 *	0 	forms a valid conversion specification.	
 *	1	the ending format string is a string.
 *	2	cannot form a valid conversion; or the string contains
 *		literal % char(s).
 * 
 * NOTE: If a character sequence in the format begins with a % character,
 *  	 but does not form a valid conversion specification, the doargs() 
 *	 will pass the invalid format to the sprintf() and let it handle
 *	 the situation.
 */

int
doargs(char **fmtptr, char *args)
/* char **fmtptr;  		 format string       */
/* char *args; 		 	 argument to process */
{
	char tmpchar, *last;
	char *ptr;			
	char *end;
	long lnum;
	double fnum;
	int percent;			/* flag for "%" */
	int width;                      /* flag for width */

	percent = 0;

	/*
         *   "%" indicates a conversion is about to happen.  This section
         *   parses for a "%"
         */

	for (last = *fmtptr; *last != '\0'; last++) {
		if (!percent) {
			if (*last == '%') {
				percent++;
				tmpchar = *last;
				*last = '\0';
				escwork(NULL, *fmtptr, 0);
				*last = tmpchar;
				*fmtptr = last;
			}
			continue;
		}

		/*
          	 * '%' has been found check the next character for conversion.
         	 */

		switch (*last) {
		case '%':
			percent = 0;
			printf("%%");
			*fmtptr = last+1;
			continue;
		case 'x':
		case 'X':
		case 'd':
		case 'o':
		case 'i':
		case 'u':
			if ((*args == '\'') || (*args == '"'))
				lnum = convchar(args, &ptr);
			else if (*last == 'u')
				lnum = strtoul(args, &ptr, 0);
			else
				lnum = strtol(args, &ptr, 0);
			if (errno) {  /* overflow, underflow or invalid base */
				fprintf(stderr, "printf: %s: %s\n",
                                        args, strerror(errno));	
				error++;
			}
			else if (strcmp(args, ptr) == 0) {
				fprintf(stderr, MSGSTR(BADNUM,
					"printf: %s expected numeric value\n"), args);
				error++;
			}
			else if (*ptr) {
				fprintf(stderr, MSGSTR(NOCOMP,
					"printf: %s not completely converted\n"), args);
				error++;
			}
			tmpchar = *(++last);
			*last = '\0';
			p_out(*fmtptr, 0, lnum);
			*last = tmpchar;
			break;
		case 'f':
		case 'e':
		case 'E':
		case 'g':
		case 'G':
			if ((*args == '\'') || (*args == '"'))
				fnum = (double) convchar(args, &ptr);
			else
				fnum = strtod(args, &ptr);
			/* strtod() handles error situations somewhat different
			   from strtoul() and strtol(), e.g., strtod() will set
			   errno for incomplete conversion, but strtoul() and
			   strtol() will not. The following error test order
			   is used in order to have the same behaviour as for
			   u, d, etc. conversions */  
			if (strcmp(args, ptr) == 0) {
				fprintf(stderr, MSGSTR(BADNUM,
					"printf: %s expected numeric value\n"), args);
				error++;
			}
			else if (*ptr) {
				fprintf(stderr, MSGSTR(NOCOMP,
					"printf: %s not completely converted\n"), args);
				error++;
			}
			else if (errno) { /* overflow, underflow or EDOM */
				fprintf(stderr, "printf: %s: %s\n",
                                        args, strerror(errno));	
				error++;
			}
			tmpchar = *(++last);
			*last = '\0';
			p_out(*fmtptr, DBL, fnum);
			*last = tmpchar;
			break;
		case 'c':
			tmpchar = *(++last);
			*last = '\0';
			p_out(*fmtptr, 0, *args);
			*last = tmpchar;
			break;
		case 's':
			tmpchar = *(++last);
			*last = '\0';
			p_out(*fmtptr, 0, args);
			*last = tmpchar;
			break;
		case 'b':
			*last = 's';
			tmpchar = *(++last);
			*last = '\0';
			escwork(outstr, args, 1);
			p_out(*fmtptr, 0, outstr);
			*last = tmpchar;
			if (cflag)
				return(0);
			break;
		default:	/* 0 flag, width or precision */
			if (isdigit(*last)) {
				width = strtol(last, &ptr, 0);
				if (errno) {
					fprintf(stderr, "printf: %s: %s\n",
                                        	last, strerror(errno));	
					error++;
				}
				if (width > MAXSTR) {
					fprintf(stderr,
						MSGSTR(LONGLINE, "printf: line too long\n"));
					exit(++error);
				}
				last += ptr - last - 1;
				continue;
			}
			switch (*last) {	
			case '-':
			case '+':
			case ' ':
			case '#':
			case '.':
				continue;
			default:
				tmpchar = *(++last);
				*last = '\0';
				p_out(*fmtptr, 0);
				*last = tmpchar;
				break;
			} 
		} 
		*fmtptr = last;
		percent = 0;
		return(0);
	} 	/* end of for */ 
	/* finish parsing the format string */
	escwork(NULL, *fmtptr, 0);
	*fmtptr = last;  /* fmtptr points to the end of format string */
	if (!strchr(*fmtptr, '%')) { /* the ending format string is a string */
		return(1);
	} else {		     /* cannot form a valid conversion */
		return(2);	
	}
} 


/*
 *   finishline
 *
 *	This routine finishes processing the extra format specifications
 *
 *      If a character sequence in the format begins with a % character,
 *      but does not form a valid conversion specification, nothing will 
 *      be written to output string. 
 */

int
finishline(char *fmtptr)
/* char *fmtptr;	         format string      */
{ 
	char tmpchar, *last;
	int percent;				/* flag for "%" */
	int width;
	char *ptr;

	/*
         * Check remaining format for "%".  If no "%", transfer 
         * line to output.  If found "%" replace with null for %s or
         * %c, replace with 0 for all others.
         */

	if (!strchr(fmtptr, '%')) {
		escwork(NULL, fmtptr, 0);
		return(0);
	}

	for (percent = 0, last = fmtptr; *last != '\0'; last++) {
		if (!percent) {
			if (*last == '%') {
				percent++;
				tmpchar = *last;
				*last = '\0';
				escwork(NULL, fmtptr, 0);
				*last = tmpchar;
				fmtptr = last;
			}
			continue;
		}

		/*
                 * OK. '%' has been found check the next character
                 * for conversion.
                 */
		switch (*last) {
		case '%':
			percent = 0;
			printf("%%");
			fmtptr = last+1;
			continue;
		case 'x':
		case 'X':
		case 'd':
		case 'o':
		case 'i':
		case 'u':
			tmpchar = *(++last);
			*last = '\0';
			p_out(fmtptr, 0, 0);
			*last = tmpchar;
			fmtptr = last;
			percent = 0;
			last--;
			break;
		case 'f':
		case 'e':
		case 'E':
		case 'g':
		case 'G':
			tmpchar = *(++last);
			*last = '\0';
			p_out(fmtptr, DBL, (double)0.0);
			*last = tmpchar;
			fmtptr = last;
			percent = 0;
			last--;
			break;
		case 'b':
		case 'c':
		case 's':
			*last = 's';
			tmpchar = *(++last);
			*last = '\0';
			p_out(fmtptr, 0, "");
			*last = tmpchar;
			fmtptr = last;
			percent = 0;
			last--;
			break;
		default:	/* 0 flag, width or precision */
			if (isdigit(*last)) { 
				width = strtol(last, &ptr, 0);
				if (errno) {
				 	fprintf(stderr, "printf: %s: %s\n",
                                                last, strerror(errno));	
					error++;
				}
				if (width > MAXSTR) {
					fprintf(stderr,
						MSGSTR(LONGLINE, "printf: line too long\n"));
					exit(++error);
				}
				last += ptr - last - 1;
				continue;
			}
			switch (*last) {	
			case '-':
			case '+':
			case ' ':
			case '#':
			case '.':
				continue;
			default:
				break;
			} 
		} 
	} 
	escwork(NULL, fmtptr, 0);
	return(0);
} 

/*
 *   p_out 
 *
 *	This routine checks if the current output line is longer than
 * 	ARG_MAX bytes. If so, outputs the current line and exits with
 *	non-zero value; otherwise, do the conversion. If the result
 *	of the conversion has no error, copy the result to the output 
 *	buffer. If the result of the conversion plus the current output
 *	line is longer than ARG_MAX bytes, the output will be truncated.
 *
 */
void
p_out(const char *format, int type, ...)
{
	char tmpstr[MAXSTR + 1];  /* temp. output string for one conversion */
	char *tmpptr;
	int rc = 0;
	va_list ap;

	tmpptr = tmpstr;
	va_start(ap, type);
	switch(type) {
		case DBL:
			rc = sprintf(tmpptr, format, va_arg(ap, double));
			break;
		default:
			rc = sprintf(tmpptr, format, va_arg(ap, void *));
			break;
	}
	if (rc < 0) {
		fprintf(stderr, MSGSTR(SPRTFERR, "printf: bad conversion\n"));
		error++; 
	}
	else if (errno && errno != ERANGE && errno != EINVAL && errno != EDOM) {
	/* strtol(), strtoul() or strtod() should have reported the error if
           errno is ERANGE, EINVAL or EDOM */
		perror("printf");
		error++;
	}
	va_end(ap);
	printf("%s", tmpptr);	
}

/*
 *   convchar
 *
 *	This routine converts a character to it's codeset equivalent.
 */
int
convchar(char *arg, char **ptr)
/* char *arg;			 char to convert */
/* char **ptr;		         end pointer     */
{
	int len, i, val = 0;

	*ptr = arg;
	arg++;
	if ((len = mblen(arg, MB_CUR_MAX)) < 1)
		return 0;
	for(i=0; i<len; i++)
		val = 256*val + ((int) arg[i]);
	*ptr = arg+len;
	return val;
}
