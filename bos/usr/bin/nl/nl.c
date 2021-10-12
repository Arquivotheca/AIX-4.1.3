static char sccsid[] = "@(#)55	1.20  src/bos/usr/bin/nl/nl.c, cmdfiles, bos412, 9446C 11/14/94 16:49:02";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: nl
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
 *
 */
#define _ILS_MACROS
#include <stdio.h>	/* Include Standard Header File */
#include <stdlib.h>
#include <strings.h>
#include <locale.h>
#include <sys/limits.h>
#include <ctype.h>
#include <regex.h>
#include <errno.h>
#include "nl_msg.h"
static nl_catd catd;
#define MSGSTR(N,S)	catgets(catd,MS_NL,N,S)

#define CONVSIZ		128  /* size of arg array buffer */
#define PATSIZ		128  /* size of buff used for regexp pattern */
#define SEPSIZ		128  /* buf used for separator and/or argv's */

static char nbuf[100];         /* Declare buf size used in convert/pad/cnt routines */

static char header = 'n';	/* 'n' - default line numbering */
static char body   = 't';  	/* 't' - default doesn't number non-graph character */
static char footer = 'n';	/* 'n' - default line numbering */

static int hbf_flags;
static char *hbf_types[3]={&header, &body, &footer};
static regex_t hbf_pats[3];

static char s1[CONVSIZ];       /* Declare the conversion array */
static char format = 'n';      /* Declare the format of numbers to be rt just */
static char delim1[MB_LEN_MAX+1];
static char delim2[MB_LEN_MAX+1];
static char pad = ' ';         /* Declare the default pad for numbers */
static char *s;                /* Declare the temp array for args */
static int width = 6;          /* Declare default width of number */
static int k;                  /* Declare var for return of convert */
static int r;                  /* Declare the arg array ptr for string args */
static int q = 2;              /* Initialize arg pointer to drop 1st 2 chars */
static wchar_t wc;
static int mb_cur_max, 
    regstat;

/*
 * NAME: nl [-h type] [-b type] [-f type] [-v start#] [-i incr] [-p]
 *          [-l num] [-s sep] [-w width] [-n format] [-d delim] file
 *                                                                    
 * FUNCTION:  Numbers lines in a file.  Input must be written in logical
 *     pages.  Each logic page has a header, body and footer section
 *     ( you can have empty sections).  To mark the different sections
 *     the first line of the section must contain delimiters only. Default:
 *     Line contents    Start of
 *     \:\:\:           Header
 *     \:\:             Body
 *     \:               Footer
 *
 *     FLAGS:
 *      -b type     chooses which body section lines to number
 *                  a   number all lines
 *                  t   do not number non-graph character (default)
 *                  n   do not number any lines
 *                  ppattern   number only lines containing pattern
 *      -d x x      Usess xx as the delimiter for the start of logical
 *                  page sections.
 *      -f type     Chooses which logical page footer lines to number
 *                  same types as in -b.
 *      -h type     Chooses which logical page header line to number
 *                  same types as in -b.
 *      -i num      Increments logical page line numbers by num.
 *                  default value of num is 1.
 *      -l num      num is the number of blank lines to count as one.
 *                  can only be used in documnets where the -ba flag
 *                  is used.
 *      -n format   Uses format as the line numbering format.
 *                  ln  left justified, leading zeroes suppressed.
 *                  rn  right justified, leading zeroes suppressed. (default)
 *                  rz  right justified, leading zeroes kept.
 *      -p          does not restart numbering a logicial page delimiters.
 *      -s sep      separates the text from its line number by the sep
 *                  character. default sep = \t.
 *      -v num      sets the initial logical page line number to num.
 *      -w num      uses num as the number of characters in the line number.
 *                  default num=6.
 */  
main(argc,argv)
int argc;
char *argv[];
{
	int j;
	int i = 0;
	char *p;
	char line[LINE_MAX+1];
	char tempchr;	/* Temporary holding variable. */
	int temp;	/* another temp holding variable */
	char swtch = 'n';
	char cont = 'n';
	char prevsect;
	char type;
	int cnt;	/* line counter */
	char sep[SEPSIZ];
	char pat[PATSIZ];
	int startcnt=1;
	int increment=1;
	int blank=1;
	int blankctr = 0;
	int c;
	int lcv;
	FILE *iptr=stdin;
	FILE *optr=stdout;
	int len1,len2,seccnt;		/* temp. vars. */
	char *ct;
	sep[0] = '\t';
	sep[1] = '\0';

	(void) setlocale(LC_ALL,"");     /* set tables up for current lang */
	catd = catopen(MF_NL, NL_CAT_LOCALE);

	mb_cur_max = MB_CUR_MAX;

	delim1[0] = '\\'; delim1[1] = '\0';	/* Default delimiters. */
	delim2[0] = ':';  delim2[1] = '\0'; 	/* Default delimiters. */

/*		DO WHILE THERE IS AN ARGUMENT
		CHECK ARG, SET IF GOOD, ERR IF BAD	*/

	while((c=getopt(argc, argv, ":h:b:f:v:i:pl:s:w:n:d:"))!=EOF) {
		hbf_flags = 2;
		switch(c) {
		case 'h':      /* header lines */
			hbf_flags--;
		case 'b':      /* body lines */
			hbf_flags--;
		case 'f':     /* footer lines */
			settypes(c, argv);
			break;
		case 'p':     /* do not restart numbering */
			cont = 'y';
			break;
		case 'v': /* set initial logical page number */
		case 'i': /* set line incrementor value */
		case 'w': /* set num of spaces used for line number */
		case 'l': /* set num of blank lines to count as one */
			temp = (int)strtoul(optarg,&optarg,10); 
			if (*optarg != '\0') {
				fprintf(stderr,MSGSTR(INVALNUMB,"nl: Invalid number for option %c.\n"),c); /*MSG*/
				exit(1);
			}
			switch(c) {
				case 'v': 
					if (temp < 0 || temp > SHRT_MAX)
						OutOfRange(c,0,SHRT_MAX);
					else
						startcnt = temp;
					break;
				case 'i':
					if (temp < 1 || temp > 250)
						OutOfRange(c,1,250);
					else
						increment = temp;
					break;
				case 'w':
					if (temp < 1 || temp > 20)
						OutOfRange(c,1,20);
					else
						width = temp;
					break;
				case 'l':
					if (temp < 1 || temp > 250)
						OutOfRange(c,1,250);
					else
						blank = temp;
					break;
				}
			break;
		case 'n':   /* numbering format */
			if (strcmp(optarg, "ln")==0) {
				format = 'l';
				continue;
				}
			if (strcmp(optarg, "rn")==0) {
				format = 'r';
				continue;
				}
			if (strcmp(optarg, "rz")==0) {
				format = 'z';
				continue;
				}
			if (argv[optind-1][0]!='-') {
				*(optarg-1) = ' ';
				optmsg(argv[optind-2]);	/* fatal error */
				}
			optmsg(argv[optind-1]);		/* fatal error */
			break;
		case 's':     /* set separator character */
			if (strlen(optarg)>127) {
				fprintf(stderr,MSGSTR(SEPTOOLONG, "SEPARATOR TOO LONG - PROCESSING TERMINATED\n")); /*MSG*/
				exit(1);
				}
			strcpy(sep, optarg);
			break;
		case ':':
			fprintf(stderr,MSGSTR(MISSARG,"nl: The option '%c' requires a parameter.\n"), optopt);
			exit(1);
			break;
		case 'd': /* set delimiter */
			if((lcv=mblen(optarg, mb_cur_max))>0)
				strncpy(delim1, optarg, lcv);
			else {
				fprintf(stderr,MSGSTR(INVDELIM1, "Invalid 1st delimiter.\n"));
				exit(1);
				}
			optarg += lcv;
			if (*optarg == '\0')	/* only one delimiter */
				break;
			if((lcv=mblen(optarg, mb_cur_max))>0)
				strncpy(delim2, optarg, lcv);
			else {
				fprintf(stderr,MSGSTR(INVDELIM2, "Invalid 2nd delimiter.\n"));
				exit(1);
				}
			break;
		default:
			optmsg(argv[optind-1]);
			break;
		}
	}

	if (*argv[optind]!='\0' && (iptr = fopen(argv[optind],"r")) == NULL)  {
		fprintf(stderr,MSGSTR( CANTOPEN, "CANNOT OPEN FILE %s\n"), argv[optind]); /*MSG*/
		exit(1);
		}

	/* On first pass only, set line counter (cnt) = startcnt and set
	   the initial type to body (not header, body, nor footer) */

	cnt = startcnt; 
	type = body;
	prevsect = '\0';

/*		DO WHILE THERE IS INPUT
		CHECK TO SEE IF LINE IS NUMBERED,
		IF SO, CALCULATE NUM, PRINT NUM,
		THEN OUTPUT SEPARATOR CHAR AND LINE	*/

	while (( p = fgets(line,(int)sizeof(line),iptr)) != NULL) {
		ct = p;
		for(seccnt = 0; seccnt < 3; seccnt++) {
			len1=mblen(ct, mb_cur_max);
			len2=mblen(ct+len1, mb_cur_max);
			if ((strncmp(ct, delim1, len1)==0) &&
			    (strncmp(ct+len1, delim2, len2)==0))
				ct += len1 + len2;
			else
				break;
			}
		if (seccnt>0 && *ct == '\n') {
			swtch = 'y';
			switch (seccnt) {
			case 1:
				if ((prevsect != 'h') && (prevsect != 'b') && (cont != 'y'))
					cnt = startcnt;
				prevsect = 'f';
				type = footer;
				break;
			case 2:
				if ((prevsect != 'h') && (cont != 'y'))
					cnt = startcnt;
				prevsect = 'b';
				type = body;
				break;
			case 3:
				if ( cont != 'y' )
					cnt = startcnt;
				prevsect = 'h';
				type = header;
				break;
			default:
				break;
			}
		}
		if (swtch == 'y') {
			swtch = 'n';
			fputc('\n',optr);
		} else {
			hbf_flags = 2;
			switch(type) {    /* check if line is numbered */
			case 'n':
				npad(width,sep);
				break;
			case 't':
				if (p[0] != '\n') {
					if (*hbf_types[0] == 't' || *hbf_types[1] == 't' || *hbf_types[2] == 't') {
						while (*p != '\n') {
							if (mb_cur_max != 1) {
								mbtowc(&wc, p, mb_cur_max);
								if (!iswprint(wc))
									goto nonumber;
								p += mblen(p, mb_cur_max);
							} else {
								if (!isprint(*p))
									goto nonumber;
								p++;
							}
						}
					}
					pnum(cnt,sep);
					cnt+=increment;
				} else 
nonumber:
					npad(width,sep);
				break;
			case 'a':
				if (p[0] == '\n') {
					blankctr++;
					if (blank == blankctr) {
						blankctr = 0;
						pnum(cnt,sep);
						cnt+=increment;
					} else
						npad(width,sep);
				} else {
					blankctr = 0;
					pnum(cnt,sep);
					cnt+=increment;
				}
					break;
			case 'h':
				hbf_flags--;
			case 'b': 
				hbf_flags--;
			case 'f':
				if (ct=strchr(p, '\n'))
					*ct = '\0';
				if((regexec(&hbf_pats[hbf_flags], p, (size_t) 0, (regmatch_t *) NULL, 0))==0) {
					pnum(cnt,sep);
					cnt+=increment;
				} else
					npad(width,sep);
				*ct = '\n';
				break;
			}

			fputs(line,optr);
		}	/* Closing brace of "else" (~ line 307). */
	}	/* Closing brace of "while". */
	fclose(iptr);
}

/*
 * NAME: pnum
 *                                                                    
 * FUNCTION: convert integer to string.
 */  
static pnum(n,sep)
int	n;
char *	sep;
{
	int	i;

		if (format == 'z') {
			pad = '0';
		}
	for ( i = 0; i < width; i++)
		nbuf[i] = pad;
		num(n,width - 1);
	if (format == 'l') {
		while (nbuf[0]==' ') {
			for ( i = 0; i < width; i++)
				nbuf[i] = nbuf[i+1];
			nbuf[width-1] = ' ';
		}
	}
		printf("%s%s",nbuf,sep);
}

/*
 * NAME: num
 *                                                                    
 * FUNCTION: 	Convert integer to character.
 *              IF NUM > 10, THEN USE THIS CALCULATE ROUTINE
 */  
static num(v,p)
int v,p;
{
	if (v < 10)
		nbuf[p] = v + '0' ;
	else {
		nbuf[p] = (v % 10) + '0' ;
		if (p>0) num(v / 10,p - 1);
	}
}

/*
 * NAME: npad
 *                                                                    
 * FUNCTION: 	CALCULATE LENGTH OF NUM/TEXT SEPARATOR
 */  
static npad(width,sep)
	int	width;
	char *	sep;
{
	int i;

	pad = ' ';
	for ( i = 0; i < width; i++)
		nbuf[i] = pad;

	fputs(nbuf,stdout);

	if(sep[0] == '\t')
		fputc('\t',stdout);
	else
	for(i=0; i < strlen(sep); i++)
		fputc(' ',stdout);
}

/*
 * NAME: optmsg
 *                                                                    
 * FUNCTION: print option error massage.
 */  
static optmsg(option)
char *option;
{
	fprintf(stderr,MSGSTR(OPTINVAL,"nl: illegal option: %s\nUsage: nl [-b Type] [-d Delimiter1 Delimiter2] [-f Type] [-h Type]\n\t[-i Number] [-l Number] [-n Format] [-p] [-s Separator]\n\t[-v Number] [-w Number] [File]\n"),option); /*MSG*/
	exit(1);
}

static settypes(class, argv)
char class;		/* 'h', 'b', or 'f' */
char **argv;
{
	switch(*optarg) {
	case 'n':
	case 't':
	case 'a':
		*hbf_types[hbf_flags] = *optarg;
		break;
	case 'p':
		*hbf_types[hbf_flags] = class;
		if ((regstat=regcomp(&hbf_pats[hbf_flags], optarg+1, 0))!=0)
			prntregerr(regstat, &hbf_pats[hbf_flags]);
		break;
	default:
		switch(hbf_flags) {
			case 'h':
				printf(MSGSTR( HEADER1, "HEADER: ")); /*MSG*/
				break;
			case 'b':
				printf(MSGSTR( BODY1, "BODY: ")); /*MSG*/
				break;
			case 'f':
				printf(MSGSTR( FOOTER1, "FOOTER: ")); /*MSG*/
				break;
			}

		if (argv[optind-1][0]!='-') {
			*(optarg-2) = ' ';
			optmsg(argv[optind-2]);		/* fatal error */
			}
		optmsg(argv[optind-1]);			/* fatal error */
	}
}

static prntregerr(regstat, preg)
int regstat;
regex_t *preg;
{
	char *err_msg_buff;
	size_t sobuff;     /* size of buffer needed */

	sobuff = regerror(regstat, preg, 0, 0);
	err_msg_buff = (char *)malloc(sizeof(char)*sobuff);
	sobuff = regerror(regstat, preg, err_msg_buff, sobuff);

	fprintf(stderr, "%s\n", err_msg_buff);
	exit(2);
}

static OutOfRange(char c, int min, int max)
{
	fprintf(stderr,MSGSTR(OUTOFRANGE,"nl: Number is out of range for option %c.\n\tThe range of the number parameter is from %d to %d.\n"),c,min,max); /*MSG*/
	exit(1);
}
