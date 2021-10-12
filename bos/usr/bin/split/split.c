static char sccsid[] = "@(#)80	1.18  src/bos/usr/bin/split/split.c, cmdfiles, bos412, 9446C 11/14/94 16:49:33";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: split
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
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>
#include "split_msg.h"

static nl_catd catd;
#define MSGSTR(Num, Str) catgets(catd, MS_SPLIT, Num, Str)
#define	atosize_t(str)	((size_t) strtoul(str, NULL, 10))

#define NUM_ALPHA 26
static size_t	count = 1000;			/* Default Line Count per POSIX */
static int	suffix_len = 2;
static int	max_outf;
static char	fname[PATH_MAX+1];
static char	*ofil;
static FILE	*is;
static FILE	*os;

static void	split_by_bytes(void);
static void	split_by_lines(void);
static FILE	*open_file(int);
static void	close_file(FILE *f);
static size_t	str2num(char *);
static void	usage(void);

/*
 * NAME: split [-l line_count] [-a suffix_length] [file [name]]
 *       split -b number[k|m] [-a suffix_length] [file [name]]
 *
 * obsolete:  split [-line_count] [-a suffix_length] [file [name]]
 *                                                                    
 * FUNCTION: Splits a file into pieces.
 *       -a suffix_length  The length of the suffix.  Default = 2.
 *       -b number[k|m]    The max number of bytes for each output file.
 *                         k = Kilobytes and m = Megabytes.
 *       -l line_count     The max number of lines for each output file.
 *                         Default = 1000.
 *       file              Input file name.  Use "-" to specify stdin.
 *       name              Alternate prefix.  (Must not exceed PATH_MAX-1
 *                         bytes when combined with suffix.)
 * 
 * RETURN VALUES:  0 if no errors
 *                 1 if error
 */  

main(int argc, char *argv[])
{
	int ch, i, numargs;
	int bflag = 0, lflag = 0;
	char *ifil;

	setlocale(LC_ALL, "");
	catd = catopen(MF_SPLIT, NL_CAT_LOCALE);

	ch = '\0';
	while (ch != EOF) {
		if (optind >= argc)
		    break;
		ch = argv[optind][0];
		if ((ch == '-') && (isdigit(argv[optind][1]))) {
			count = atosize_t(argv[optind]+1);
			if (bflag)
				usage();
			lflag++;
			optind++;
		} else if ((ch = getopt(argc, argv, "a:b:l:")) != EOF) {
			switch(ch) {
				case 'a':
					suffix_len = atoi(optarg);
					if (suffix_len < 0)
						usage();
					break;
				case 'b':
					count = str2num(optarg);
					if (lflag)
						usage();
					bflag++;
					break;
				case 'l':
					count = atosize_t(optarg);
					if (bflag)
						usage();
					lflag++;
					break;
				default:
					usage();
			}
		}
	}
	if (count < 1) {
		if (bflag)
			fprintf(stderr, MSGSTR(INVALNUMB,
			"Invalid number of bytes specified\n."));
		else
			fprintf(stderr, MSGSTR(INVALNUML,
			"Invalid number of lines specified\n."));
		usage();
	}

	numargs = argc - optind;
	if ((numargs < 0) || (numargs > 2))
		usage();
					/* No args or "-", use stdin */
	if ((numargs < 1) || (!strcmp("-", argv[optind])))
		ifil = NULL;
	else
		ifil = argv[optind];
					/* 2nd arg, if given, is a prefix */
	if (numargs == 2)
		ofil = argv[optind+1];
	else
		ofil = "x";

	if(ifil == NULL)
		is = stdin;
	else          /* open file */
		if((is=fopen(ifil,"r")) == NULL) {
			fprintf(stderr,MSGSTR(INPOPNER, "cannot open input\n"));
			exit(1);
		}

	if((strlen(ofil) + suffix_len) > PATH_MAX) {
		fprintf(stderr, MSGSTR(OUTNMSIZ, "more than %d characters in output file name\n"),PATH_MAX);
		exit(1);
	}

	for(i=0, max_outf=1; i<suffix_len; i++) {
		if (max_outf > (INT_MAX/NUM_ALPHA)) {
			max_outf = INT_MAX;
			break;
		}
		max_outf *= NUM_ALPHA;
	}

	if (bflag)
		split_by_bytes();
	else
		split_by_lines();

	fclose(is);
	exit(0);
} /* main */


static void
split_by_bytes(void)
{
	int done, fnumber;
	size_t i, rcount, numread;
	char buffer[BUFSIZ];

	for (done = 0, fnumber = 0; !done; fnumber++)
	{
		os = NULL;
		for(i=0; i<count; i += numread)	/* break file */
		{
			rcount = ((count - i) < BUFSIZ) ? count - i : BUFSIZ;
			if ((numread = fread(buffer, 1, rcount, is)) <= 0) {
				done = 1;
				break; /* break out of inner loop */
			}
			/* open file only after we have read something */
			if (os == NULL)
				os = open_file(fnumber);
			if (fwrite(buffer, 1, numread, os) != numread) {
				perror("split");
				exit(1);
			}
		}
		close_file(os);
	}
	return;
} /* split_by_bytes */


static void
split_by_lines(void)
{
	int done, fnumber;
	size_t i;
	char buffer[LINE_MAX + 1];

	for (done = 0, fnumber = 0; !done; fnumber++)
	{
		os = NULL;
		for(i=0; i<count; i++)		/* break file */
		{
			if (fgets(buffer, LINE_MAX + 1, is) == NULL) {
				done = 1;
				break; /* break out of inner loop */
			}
			/* open file only after we have read something */
			if (os == NULL)
				os = open_file(fnumber);
			if (fputs(buffer, os) == EOF) {
				perror("split");
				exit(1);
			}
		}
		close_file(os);
	}
	return;
} /* split_by_lines */


static FILE *
open_file(int n)
{
	int i, j;
	FILE *f;

	if(n >= max_outf) {
		fprintf(stderr, MSGSTR(ABRTSPLT, "more than %d output files needed, aborting split\n"), max_outf);
		exit(1);
	}
	for(i=0; ofil[i]; i++)
		fname[i] = ofil[i];
	for(j=suffix_len-1; j>=0; j--) {
		fname[i+j] = n%NUM_ALPHA + 'a';
		n /= NUM_ALPHA;
	}
	i += suffix_len;
	fname[i] = '\0';
	if((f=fopen(fname,"w")) == NULL) {
		perror("split");
		exit(1);
	}
	return f;
} /* open_file */


static void
close_file(FILE *f)
{
	if ((f != NULL) && (fclose(f) == EOF)) {
		perror("split");
		exit(1);
	}
	return;
} /* close_file */


static size_t
str2num(char *str)
{
	char *endptr;
	size_t val, mult;

	val = (size_t) strtoul(str, &endptr, 10);

	switch(*endptr) {
		case '\0':
			mult = 1; break;
		case 'k':
			mult = 1024;
			break;
		case 'm':
			mult = 1048576;
			break;
		default:
			fprintf(stderr, MSGSTR(INVALMULT,
			"Invalid multiplier specified.\n"));
			usage();
	}
	return (val * mult);
} /* str2num */


static void
usage(void)
{
	fprintf(stderr,MSGSTR(USAGE, "usage: split [-l line_count] [-a suffix_length] [file [name]]\n   or: split -b number[k|m] [-a suffix_length] [file [name]]\n"));
	exit(1);
} /* usage */
