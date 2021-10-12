static char sccsid[] = "@(#)56	1.23  src/bos/usr/ccs/bin/strings/strings.c, cmdscan, bos412, 9446B 11/15/94 20:13:08";
/*
 * COMPONENT_NAME: (CMDSCAN) strings
 *
 * FUNCTIONS:
 *
 * ORIGINS: 3, 26, 27
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
 */

#define _ILS_MACROS

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <locale.h>
#include <xcoff.h> 
#include <sys/file.h>

#include "strings_msg.h" 
static nl_catd catd;
#define MSGSTR(n,s) catgets(catd, MS_STRINGS, n, s) 
#define	MB_PATH	1
#define SB_PATH	0

/*
 * strings
 */

static struct	xcoffhdr header;
static struct scnhdr scnhdr;
#define BADMAG(X)	(  ( (X).filehdr.f_magic != U802WRMAGIC) && \
			   ( (X).filehdr.f_magic != U802ROMAGIC) && \
			   ( (X).filehdr.f_magic != U800WRMAGIC) && \
			   ( (X).filehdr.f_magic != U800ROMAGIC) && \
			   ( (X).filehdr.f_magic != U802TOCMAGIC) && \
			   ( (X).filehdr.f_magic != U800TOCMAGIC) ) 

#define DEFAULT_USAGE "Usage: strings [ -a | - ] [ -o ] [ -t format ] [ -n # | -# ] [ file ... ]\n"
#define DEFAULT_NVAL_MAX "strings: string length cannot be greater than %d\n"

static void find(long cnt);
static void findMB(long cnt);
static int dirt(int c, int wval );
static char	*infile = "Standard input";
static int	oflg = 0;
static int	dflg = 0;
static int	xflg = 0;
static int	asdata = 0;
static long	offset = 0;
static int	minlength = 4;
static int     status = 0;		/* the value the strings command returns */

main(int argc, char *argv[])
{
	setlocale(LC_ALL, "");
	catd = catopen(MF_STRINGS, NL_CAT_LOCALE);

	argc--, argv++;
	while (argc > 0 && argv[0][0] == '-') {
		register int i;
		if (argv[0][1] == 0)		/* same as -a */
			asdata++;
		else
		if ((argv[0][1]=='-') && (argv[0][2]==0)) {
			argc--, argv++; /* --, so do args */
			break;
		}
		else for (i = 1; argv[0][i] != 0; i++) switch (argv[0][i]) {

		/*
		 * Option to preceed each buffer print by its offset in the
		 * file; in octal.
		 */
		case 'o':
			oflg++;
			dflg=xflg=0;
			break;

		/*
		 * Option to search the entire file, not just the data
		 * section for printable characters.
		 */
		case 'a':
			asdata++;
			break;

		case 't':
			/* if next char is null (just t) check next arg */
			/* look through remaining characters in string  */
			/* if known char (odx) set flags, else error    */
			if (argv[0][++i] == 0) {
				i=0;
				argc--,argv++;
				if (argc <= 0) {
					fprintf(stderr, MSGSTR(USAGE, DEFAULT_USAGE));
					exit(1);
					}
			}
			while (argv[0][i] != 0 )
				switch (argv[0][i++]) {
					case 'o':
						oflg++;
						dflg=xflg=0;
						break;
					case 'd':
						dflg++;
						oflg=xflg=0;
						break;
					case 'x':
						xflg++;
						oflg=dflg=0;
						break;
					default:
						fprintf(stderr, MSGSTR(USAGE, DEFAULT_USAGE));
						exit(1);
				}
			i--; /* backup so that the for loop will exit */
			break;

		case 'n':
			/* if next char is null (just n) check next arg */
			/* look through remaining characters in string  */
			/* make a number out of them, if possible       */
			if (argv[0][++i] == 0) {
				i=0;
				argc--,argv++;
				if (argc <= 0) {
					fprintf(stderr, 
						MSGSTR(USAGE, DEFAULT_USAGE));
					exit(1);
					}
			}
			minlength = 0;
			while (argv[0][i] != 0 ) {
				if (!isdigit((int)argv[0][i])) {
					fprintf(stderr, MSGSTR(USAGE, DEFAULT_USAGE));
					exit(1);
					}
				minlength = minlength * 10 + argv[0][i] - '0';
				i++;
				}
			i--; /* backup so that the for loop will exit */
			break;

		/*
		 * Any other options, other than a number will cause
		 * execution to halt.  A number option (e.g. -2) will
		 * cause the minimum length to be that number.
		 */
		default:
			if (!isdigit((int)argv[0][i])) {
				fprintf(stderr, MSGSTR(USAGE, DEFAULT_USAGE));
				exit(1);
			}
			minlength = argv[0][i] - '0';
			for (i++; isdigit((int)argv[0][i]); i++)
				minlength = minlength * 10 + argv[0][i] - '0';
			i--;
			break;
		}
		/* if user used a larger than max value for 
		 * -n option then exit 
		 */
		if ( minlength > BUFSIZ ) {
			fprintf(stderr, MSGSTR(NVAL_MAX, DEFAULT_NVAL_MAX), BUFSIZ);
			exit(2);
		}

		argc--, argv++;
	}
	do {
		while (argc > 0) {
			if (freopen(argv[0], "r", stdin) == NULL) {
				perror(argv[0]);
				argc--, argv++;
                                status=1;
                                if (argc==0)
                                	exit(1);
			}
			else {
				infile = argv[0];
				argc--, argv++;
				break;
			}
		}
		fseek(stdin, 0, 0);
		if (asdata ||
		    fread((void *)&header, sizeof(header), 1, stdin) != 1 || 
		    BADMAG(header)) {
			fseek(stdin, 0, 0);
			if (MB_CUR_MAX>1)
			{
				findMB((long) 100000000L);
			}
			else
				find((long) 100000000L);
			continue;
		}
		fseek(stdin, (long)(FILHSZ+header.filehdr.f_opthdr+((header.aouthdr.o_sndata-1)*(sizeof scnhdr))), 0);
		fread((void *)&scnhdr,sizeof(scnhdr),1,stdin);
		fseek(stdin, scnhdr.s_scnptr, 0);
		if (MB_CUR_MAX>1)
			findMB(scnhdr.s_size);
		else
			find (scnhdr.s_size);
	} while (argc > 0);
exit(status);
}

/*
 * Function: find
 *
 * Description: Find and print all valid characters from the file that was
 *              opened above.  (Used for single byte locales)
 *             ********************************************************
 *             *Modifications to find() must also be made to findMB().*
 *             ********************************************************
 */

static void
find(long cnt)
{
	register int cc;
	register int c;
	static char buf[BUFSIZ];
	register char *cp;

	cp = buf, cc = 0;
	for (; cnt != 0; cnt--) {
		/*
		 * Get the character and check to see if it is valid.
		 */
		c = getc(stdin);
	
		if (c == '\n' || c == '\0' || cnt == 0)
		{
		/*
		 * If the character read is null or a terminating character,
		 * then, if buf contains more than minlength characters
		 * print the buffer with newline termination, and reset the
		 * buffer.
		 *
		 * Else if the character read is non-printable then
		 * all previous characters are not considered a string and
		 * buf is reset.
		 */
			if (cp > buf && cp[-1] == '\n')
				--cp;
				*cp++ = 0;
			if (cp > &buf[minlength]) {
				if (oflg)
					printf("%7o ", ftell(stdin) - cc - 1);
				if (dflg)
					printf("%7d ", ftell(stdin) - cc - 1);
				if (xflg)
					printf("%7x ", ftell(stdin) - cc - 1);

				puts(buf);
			}
			cp = buf, cc = 0;
		} else if (!dirt(c,SB_PATH)) {
			/*
			 * 
			 * As long as the current line being read is less
			 * than BUFSIZ - 2 (to allow space for a terminating
			 * null when the buffer becomes full) then add the
			 * character to the buffer.
			 *
			 * If the last valid character of a file falls on a
			 * buffer boundary, then the next character will be
			 * EOF, and the above "dirt" condition will occur.
			 */
			if (cp < &buf[BUFSIZ - 2]) {
				*cp++ = (char)c;
					cc++;
			} else {
				/*
				 * If the line read is too long, then
				 * print it out as if a terminating
				 * character were found.
				 */
				*cp++ = (char)c;
				*cp++ = 0;
				if (oflg)
					printf("%7o ", ftell(stdin) - cc - 1);
				if (dflg)
					printf("%7d ", ftell(stdin) - cc - 1);
				if (xflg)
					printf("%7x ", ftell(stdin) - cc - 1);

				fputs(buf,stdout);
				cp = buf, cc = 0;
			}
		} else
			cp = buf, cc = 0;
		/*
		 * If an error condition occurs when reading the input
		 * stream, or we reach end of file, end current search.
		 */
		if (ferror(stdin) || feof(stdin))
			break;
	}
}


/*
 * Function: findMB
 *
 * Description: Find and print all valid characters from the file that was
 *              opened above.  (used for MB locales)
 *             ********************************************************
 *             *Modifications to findMB() must also be made to find().*
 *             ********************************************************
 */

static void
findMB(long cnt)
{
	register int cc;
	int sz;
	char bf[30];
	wint_t c;
	wchar_t buf[BUFSIZ+1];
	wchar_t *cp;
	int dirt_val;

	cp = buf, cc = 0;
	for (; cnt != 0; cnt--) {
		/*
		 * Get the character and check to see if it is valid.
		 */
		c = getwc(stdin); 

		if ( ( c == L'\n' ) || c == L'\0' || cnt == 0 )
		{
		/*
		 * If the character read is null or a terminating character,
		 * then, if buf contains more than minlength characters
		 * print the buffer with newline termination, and reset the
		 * buffer.
		 *
		 * Else if the character read is a non-printable char then
		 * all previous characters are not considered a string and
		 * buf is reset.
		 */
			if (cp > buf && cp[-1] == '\n')
				--cp;
				*cp++ = 0;
			if (cp > &buf[minlength]) {
				if (oflg)
					printf("%7o ", ftell(stdin) - cc - 1);
				if (dflg)
					printf("%7d ", ftell(stdin) - cc - 1);
				if (xflg)
					printf("%7x ", ftell(stdin) - cc - 1);
				printf("%S\n", buf);
			}
			cp = buf, cc = 0;
		} else if ( !dirt(c,MB_PATH)) {
			/*
			 * As long as the current line being read is less
			 * than BUFSIZ - 2 (to allow space for a terminating
			 * null when the buffer becomes full) then add the
			 * character to the buffer.
			 *
			 * If the last valid character of a file falls on a
			 * buffer boundary, then the next character will be
			 * EOF, and the above "dirt" condition will occur.
			 */
			if (cp < &buf[BUFSIZ - 2]) {
				*cp++ = (wchar_t)c;
				if ((sz=wctomb(bf, (wchar_t)c))>1)
					cc += sz;
				else
					cc++;
			} else {
				/*
				 * If the line read is too long, then
				 * print it out as if a terminating
				 * character were found.
				 */
				*cp++ = (wchar_t)c;
				*cp++ = 0;
				if (oflg)
					printf("%7o ", ftell(stdin) - cc - 1);
				if (dflg)
					printf("%7d ", ftell(stdin) - cc - 1);
				if (xflg)
					printf("%7x ", ftell(stdin) - cc - 1);

				fputws(buf,stdout);
				cp = buf, cc = 0;
			}
		} else {
			cp = buf, cc = 0;
			if( c == WEOF ) {
				/* if getwc returned WEOF make sure it's really *
				 * the eof and not just an invalid multibyte ch */
				clearerr(stdin);
				getc(stdin);
			}
		}
			
		/*
		 * If an error condition occurs when reading the input
		 * stream, or we reach end of file, end current search.
		 */
		if (ferror(stdin) || feof(stdin))
			break;
	}
}

/*
 * Function: dirt
 *
 * Description: Static function to see if characters
 *              are valid for placement into the print buffer.
 */

static int
dirt(int c, int wval)
{

	switch (c) 
	{
	case 0177:
		return (1);
	default:
		if (wval)
			return( !(iswprint(c) || iswspace(c)) );
		else
			return( !(isprint(c) || isspace(c)) );
	}
}/* End dirt() */

