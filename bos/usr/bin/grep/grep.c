static char sccsid[] = "@(#)46	1.57  src/bos/usr/bin/grep/grep.c, cmdscan, bos41J, 9519B_all 5/11/95 15:12:18";
/*
 * COMPONENT_NAME: (CMDSCAN) commands that scan files
 *
 * FUNCTIONS:
 *
 * ORIGINS: 3, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
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

/*
 * grep -- print lines matching (or not matching) a pattern
 *
 *	status returns:
 *		0 - ok, and some matches
 *		1 - ok, but no matches
 *		2 - some error
 */

#define _ILS_MACROS

#include <stdio.h>
#include <string.h>
#include <macros.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <locale.h>
#include "grep_msg.h"
#include <regex.h>
#include <fcntl.h>
#include <sys/mode.h>

static nl_catd	catd;
#define MSGSTR(n,s)	catgets(catd,MS_GREP,n,s)
#define MAXB MB_CUR_MAX

static int mb_cur_max;		/* const. value after start of program. */

static int regstat;
static int regflags = 0; 	/* regular expression flags */

/*
 * linked list needed for the fflag
 * because it's possible to have more than
 * 1 pattern in a file
 */
struct regexpr {
	struct regexpr *next;
	regex_t regpreg;  	/* regular expression structure */
};

static struct regexpr *s1;
static struct regexpr *head;
static struct regexpr *tail;

static int wordcnt = 0;	/* current number of words in linked list 	  */
#define MAXWORDS 2048   /* Initial maximum number of patterns allowed     */
int     maxwords=0;     /* current maximum number of patterns allowed     */
			/* this number is increased, and the arrays which */
			/* use it are extended in the function            */
			/*  increase_maxwords() 			  */

#define ESIZE	(256*5)
#define BSIZE	512
#define BSPACE 10000
#define NUMEXPS 20
#define LUL 8		/* Length of Null padding for Fast grep mode      */
#define CTABSIZE 256	/* Size of character indexed tables               */
#define BMOFF 3		/* Boyer Moore table offset                       */
#define ZERO '\0'
#define LF   '\n'


static int     fastflag;               /* fast grep flag            */
static int     vfflag;                 /* very fast grep flag       */
static int     mfflag;                 /* medium fast grep flag     */
static int     mb_fflag;               /* fast grep for mb locales  */

static long	lnum;			/* current line number       */
static int	nflag;			/* Number lines              */
static int	iflag;			/* ignore case.              */
static int	bflag;			/* display block numbers     */
static int	lflag;			/* list file names           */
static int	cflag;			/* count line matches        */
static int	hflag = 0;		/* suppress prnt. filenames  */
static int	vflag;			/* the NOT case              */
static int	sflag;			/* silent flag               */
static int	qflag;			/* quiet flag                */
static int	wflag;			/* match word only           */
static int	eflag = 0;		/* use next expression       */
static int 	xflag;			/* match an entire string    */
static int 	Fflag = 0;		/* match fixed expression    */
static int 	Eflag = 0;		/* extended regular express  */
static int 	fflag = 0;		/* take pattern from file    */
static int	nfile;			/* how many files to look at */
static long	tln;			/* total lines that matched  */
static int	nsucc=0;		/* Return code.  success?    */
static long    blkno1;			/* block number              */
static int	wgrep=0;		/* flag: which grep--grep, egrep, or fgrep   */
				/* grep=0, egrep=1, fgrep=2		     */
				/* if nonzero, use egrep's and fgrep's	     */
				/*  -s flag of displaying only error essages */
				/*  as opposed to grep's suppressing error   */
				/*  messages of nonexistant or unreadable    */
				/*  files. 	 		   	     */

#define GREP    0
#define EGREP   1
#define FGREP   2


static struct {int id; char msg[400]; }    /* Message id and default message text */
	usages[] = {
		{USAGE, "usage: %s [-E|-F] [-c|-l|-q] [-insvxbhwy] [-p[parasep]] -e pattern_list...\n\t[-f pattern_file...] [file...]\nusage: %s [-E|-F] [-c|-l|-q] [-insvxbhwy] [-p[parasep]] [-e pattern_list...]\n\t-f pattern_file... [file...]\nusage: %s [-E|-F] [-c|-l|-q] [-insvxbhwy] [-p[parasep]] pattern_list [file...]\n"},

		{EUSAGE, "usage: %s [-hisvwxy] [[-bn]|[-c|-l|-q]] [-p[parasep]] -e pattern_list...\n\t[-f pattern_file...] [file...]\nusage: %s [-hisvwxy] [[-bn]|[-c|-l|-q]] [-p[parasep]] [-e pattern_list...]\n\t-f pattern_file... [file...]\nusage: %s [-hisvwxy] [[-bn]|[-c|-l|-q]] [-p[parasep]] pattern_list [file...]\n"},

		{FUSAGE, "usage: %s [-hisvwxy] [[-bn]|[-c|-l|-q]] [-p[parasep]] -e pattern_list...\n\t[-f pattern_file...] [file...]\nusage: %s [-hisvwxy] [[-bn]|[-c|-l|-q]] [-p[parasep]] [-e pattern_list...]\n\t-f pattern_file... [file...]\nusage: %s [-hisvwxy] [[-bn]|[-c|-l|-q]] [-p[parasep]] pattern_list [file...]\n"},
		};

				/* Is it grep, egrep, or fgrep?  	     */
				/* For error displaying purposes.	     */
				/*  (egrep = "grep -E", fgrep = "grep -F")   */
static char		*callname;
static char		perrormsgstr[] = {"egrep malloc "}; /* error msg for perror() */
static char		*perrormsg = perrormsgstr;
static char    	*endspace;
static char 		*escape();
static long    	lnum1;
static unsigned        lspace;
static int     	pflag;
static char    	*space=0;
static int ii;
static regex_t 	*terms[NUMEXPS + 1];   /* paragraph delimiters */

static size_t nmatch = 1;			/* used when checking for words */
static regmatch_t  match_offsets[1];		/* used when checking for words */
static regmatch_t *match_ptr;			/* used when checking for words */

static void execute(char *);
static void fastexecute(char *); 
static int  check_list(char *, char *);
static void succeed(char *);
static void prntregerr(int, regex_t *);
static void errexit(char *, char *);
static int  wordmatch(regmatch_t *offsets, char *);
static void insertpatterns(char *);
static void getpreg(regex_t *, char *);
static void checkfile(char *);
static void fast_insertpatterns(char *);
static void umatch(int fd, char * comstr, char * cmap, int strleng, int * strmj,
 	           char * file);
static int  umatch2(char * line, int leng);
static void increase_maxwords(int);

static FILE *wordf;
static FILE *cur_input;                     /* current input file */

#ifdef GREP_DEBUG
static FILE *dbf;                           /* file for debug information  */
static int debug_value = 0;
#endif

static char **epatterns;

static int     str_num = 0;   /* number of input strings                     */
static int    *str_len;	      /* array of input string lengths               */
static char  **strcmap;       /* array of ptrs to inp str char maps          */
static char  **str_inp;       /* array of ptrs to input strings              */
static int   **strmj;         /* array of ptrs to input str match jump table */

static char    possible_mb[CTABSIZE]; /* true if entry is 1st byte of pattrn */

main(int argc, char *argv[])
{
	int c;
	char *arg ;
	int errflg = 0;
	char *ffiles[OPEN_MAX];
	char buf[LINE_MAX+1];
	char *p;
	char *bop,		/* begining of pattern */
	     *eop,		/* end of pattern      */
	     *tcp;		/* temp. char ptr.     */

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_GREP,NL_CAT_LOCALE);

#ifdef GREP_DEBUG
	dbf = fopen("/tmp/tmp.db", "w"); /* debug output */
#endif

	mb_cur_max = MB_CUR_MAX;

	if (p = strrchr(argv[0],'/')) argv[0] = p + 1;

	callname = argv[0];

	if (mb_cur_max != 1)
		fastflag = 0;	/* can't use fastexecute()	*/
	else
		fastflag = 1;	/* maybe use fastexecute()	*/

	switch (*callname) {
	   case 'e' :
		wgrep = 1;
		Eflag = 1;
		break;
		
	   case 'f' :
		*perrormsg= 'f';
		wgrep = 2;
		Fflag = 1;
		break;
		
	   default  :           /* assume it is grep */
		perrormsg++;
	}

increase_maxwords(MAXWORDS);

#ifdef GREP_DEBUG
	while ((c=getopt(argc, argv, "bhlciynsqvwxd:e:p:f:FE")) != EOF)
	   switch(c) {
		case 'd':  /* tmp debug flag */
			debug_value = atoi(optarg);
			fprintf(dbf, " debug_value = %d\n", debug_value);
			break;
#else
	while ((c=getopt(argc, argv, "bhlciynsqvwxe:p:f:FE")) != EOF)
	   switch (c) {
#endif
		case 'v':
			vflag = 1;	/* restrict to {0,1}	*/
			break;

		case 'i':
		case 'y':
			iflag = 1;
			break;

		case 'c':
			cflag = 1;
			break;

		case 'n':
			nflag = 1;
			/* fastflag = 0; */
			break;

		case 'b':
			bflag = 1;
			/* fastflag = 0; */
			break;


		case 'h':
			hflag = 1;
			break;

		case 's':
			sflag = 1;
			break;

		case 'q':
			qflag = 1;
			break;

		case 'l':
			lflag = 1;
			break;

		case 'w':
			wflag = 1;
			continue;

		case 'e':
			epatterns[eflag++] = optarg;
			continue;

		case 'p':
			fastflag = 0;
			if (pflag >= NUMEXPS)
				errexit(MSGSTR(PARG,
				"too many `p' arguments\n"), (char *) NULL);

			if ((terms[pflag] = (regex_t *)malloc(sizeof(regex_t))) == NULL)
			{
				perror (perrormsg);
				exit (2);
			}
		/* getopt allows no arguments after a minus argument or a
		   mandatory argument after a minus argument, but no optional
			   argument, so work around this problem */

			if (optarg != argv[optind-1]) {
				if ( (regstat = regcomp(terms[pflag], optarg, 0) ) != 0 )
					prntregerr(regstat, terms[pflag]);
			} else {
				if ( (regstat = regcomp(terms[pflag], "^$", 0) ) != 0 )
					prntregerr(regstat,terms[pflag]);
				else
					--optind;
			}
			++pflag;
			if (space == 0)
				if ((space = malloc((size_t)(lspace = BSPACE + LINE_MAX + 1))) == NULL)
				{
					perror (perrormsg);
					exit (2);
				}
			break;

		case 'x':
			xflag = 1;
			break;
		case 'F':
			if (wgrep == GREP) {
				Fflag = 1;
				Eflag = 0;
			}

			else	/* usage statement if not grep */
				errflg = 1;
			break;
		case 'f':
			ffiles[fflag++] = optarg;
			checkfile(optarg);
			continue;
		case 'E':
			if (wgrep == GREP) {
				Eflag = 1;
				Fflag = 0;
			}
			else	/* usage statement if not grep */
				errflg = 1;
			break;
		case '?':
			errflg = 1;
	} /* end of input options parsing */

	if (cflag && lflag)		/* lflag takes precedence over cflag */
		cflag = 0;		/* re: defect 121506 & XPG4 p. 378   */

	argc -= optind;

	if(errflg || (argc <= 0 && !eflag && !fflag)) {
		fprintf(stderr, MSGSTR(usages[wgrep].id, usages[wgrep].msg), callname,
				callname, callname);
		exit(2);
	}

	if ( !eflag  && !fflag ) {
		epatterns[eflag++] = argv[optind++];
		nfile = --argc;
	 }
	else
		nfile = argc;

	argv = &argv[optind];

	if (Eflag)
		regflags |= REG_EXTENDED;

	if (iflag)
		regflags |= REG_ICASE;

	vfflag = fastflag && !iflag;
	if (nflag || bflag)
        	fastflag = 0;	/* keep fastflag like it was */

	mb_fflag = !iflag && !pflag && !wflag && !xflag && (mb_cur_max>1);
	if (mb_fflag) {
		int i;
		for (i=0;i<CTABSIZE;i++);
			possible_mb[i]=0;
		}

	if (fflag)
		for (ii = 0; ii < fflag; ii++) {
	      		if ((wordf = fopen(ffiles[ii], "r"))==NULL)
			{
			   fprintf(stderr,MSGSTR(OPERR, "%s: can't open %s\n"),
				 callname, ffiles[ii]);
			   exit(2);
			}
	      		while (fgets(buf, LINE_MAX+1, wordf) != NULL) {
		 		if (vfflag || mb_fflag)
                      			fast_insertpatterns(buf);
		 		insertpatterns(buf);
	      		}
	      		fclose( wordf);
	   	}

	if (eflag)
		for (ii = 0; ii < eflag; ii++) {
			if (vfflag || mb_fflag)
				 fast_insertpatterns(epatterns[ii]);
			insertpatterns(epatterns[ii]);
		}

	vfflag = vfflag && !wflag && !xflag;
	mfflag = vfflag;
	if (nflag || vflag || (str_num > 1))
		vfflag = 0;

	/* don't need to allocate "space" if vfflag is true */
	if ((space == 0) && vfflag == 0)
		if ((space = malloc((size_t)(lspace = LINE_MAX + 1 + LUL))) == NULL)
		{
		   perror (perrormsg);
		   exit (2);
		}
	endspace = space + lspace - LINE_MAX - LUL;

	if (argc<=0) {
		if (fastflag || mfflag || mb_fflag)
			fastexecute((char *)NULL);
		else
			execute((char *)NULL);
        }
        else while (--argc >= 0) {
		if (fastflag || mfflag || mb_fflag)
			fastexecute(*argv++);
		else
			execute(*argv++);
	}

        /* Defect 44200 */
        errno = 0;
        fclose(stdout);
        if (errno == ENOSPC || errno == EDQUOT)
        {
                perror(callname);
                nsucc = 2;
        }
	exit(nsucc == 2 ? 2 : (nsucc == 0));  /* two means it's an error */
}

/*
 * NAME: fastexecute
 *
 * FUNCTION:    Fast grep - print each line which matches user pattern.
 *
 * RETURN VALUE: void
 *
 */

static void
fastexecute(char *file)
{
        FILE	*temp;		/* input stream      */
	int     fleng;          /* fgets line length */
	int     rc;             /* fgets return code */
	int     fd;		/* input streams file descriptor */
	int	was_long = 0;	/* are we finishing off a long line? */

        if (file) {
                if ((temp = fopen(file, "r")) == NULL) {
                        if (!sflag || wgrep)   /* if wgrep==0, use grep's -s */
				fprintf(stderr
					, (MSGSTR(OPERR
						  , "%s: can't open %s\n"))
					, callname, file);
			nsucc = (nsucc==1 && qflag) ? 1 : 2;
                        return;
		}
	} else {
                temp = stdin;
		if (bflag) {
			/* this could be supported by replacing ftell() usage */
			/* with manual byte counts, but would slow grep down  */
			fprintf(stderr
				, MSGSTR(BANDSTDIN,
                       "%s: The -b flag may not be used with stdin or pipes.\n")
                                 , callname);
			nsucc = (nsucc==1 && qflag) ? 1 : 2;
			return;
		}
	}

	cur_input = temp;	/* for `lflag'		*/

	/*
	 * this is the essence of fast execute
	 */
	if (vfflag) {
		fd = fileno(temp);
		umatch(fd, str_inp[0], strcmap[0], str_len[0], strmj[0], file);
	}
	else {
		lnum1 = 1;
		if (mfflag || mb_fflag) { /* list of simple input strings */
			while (fgets(space, LINE_MAX+1, temp) != NULL) {

				fleng = strlen(space);
				if ((space[fleng - 1] != '\n') && 
                                            (fleng == LINE_MAX)) {

					if (was_long)
						continue;

					nsucc = (nsucc==1 && qflag) ? 1 : 2;

					fprintf(stderr, MSGSTR(LINELENERR,
                                   "%s: Maximum line length of %d exceeded.\n"),
					callname, LINE_MAX);
						
					was_long = 1;
					continue;
				}
				
				if (was_long) {
					was_long = 0;
					continue;
				}		

				space[fleng - 1] = '\0';
				memset(space + fleng, '\0', LUL);

				if (mb_fflag)
					rc = mb_umatch(space, fleng - 1);
				else
					rc = umatch2(space, fleng - 1);
				if (vflag ^ rc) {
					if (bflag)
						blkno1=(ftell(temp)-1)/BSIZE;
					succeed(file);
				}
				lnum1++;
			}
		}
		else {
			while (fgets(space, LINE_MAX+1, temp) != NULL) {
				
				if ((space[strlen(space) - 1] != '\n') &&
                                              (strlen(space) == LINE_MAX)) {
					if (was_long)
						continue;

                                        nsucc = (nsucc==1 && qflag) ? 1 : 2;

                                        fprintf(stderr, MSGSTR(LINELENERR,
                                   "%s: Maximum line length of %d exceeded.\n"),
                                        callname, LINE_MAX);

                                        was_long = 1;
                                        continue;
                                }

                                if (was_long) {
                                        was_long = 0;
                                        continue;
                                }

				if (vflag ^ check_list(file, space))
					succeed(file);
			}
		}
	}

	/*
	 * common flag processing at EOF
	 */
	if (cflag && !qflag) {
		if (nfile>1 && !hflag)
			printf("%s:"
			       , file ? file:MSGSTR(STDIN,"(standard input)"));
		printf("%ld\n", tln);
		tln = 0;
	}

	/* close the file */
        if (file) {
                fclose(temp);
        }
}

/*
 * NAME: execute
 *
 * FUNCTION:	For each file in the agument list, search each line
 *		for characters matching the user's pattern.  Multibyte
 *		charcters are converted if the iflag is set.
 *
 * RETURN VALUE: void
 *
 */

static void
execute(char *file)
{
	register char *p1;
	register int c;
	int match;
	int i;
	char *cp;
	FILE *temp;
	int chklline = 0;
	int nlflag = 0;
	int was_long = 0;	/* are we finishing off a long line? */
	char *linebuf,
	     *lbuffer,
	     linebuffer[LINE_MAX * 2  +  1];

	lbuffer = linebuffer;
	if (file) {
		if ((temp = fopen(file, "r")) == NULL) {
                        if (!sflag || wgrep)   /* if wgrep==0, use grep's -s */
				fprintf(stderr
					, (MSGSTR(OPERR
						  , "%s: can't open %s\n"))
					, callname, file);
			nsucc = (nsucc==1 && qflag)? 1:2;
			return;
	        }
	} else {
		temp = stdin;
		if (bflag) {
			/* this could be supported by replacing ftell() usage */
			/* with manual byte counts, but would slow grep down  */
			fprintf(stderr
				, MSGSTR(BANDSTDIN,
                       "%s: The -b flag may not be used with stdin or pipes.\n")
                                 , callname);
			nsucc = (nsucc==1 && qflag)? 1:2;
			return;
		}
	}

	cur_input = temp;	/* for `lflag'		*/

	lnum = 0;
	tln = 0;
	match = 0;
	cp = space;
	lnum1 = 0;
	/* If empty file, we are done. In case of c print zero */
	if ((c = getc(temp)) == EOF) {
		if (!cflag) {
			/* close the file */
			if (file) {
				fclose(temp);
			}
			return;
		}
		else {
			if (!qflag) {
				if (nfile>1 && !hflag)
					printf("%s:", file? file:MSGSTR(STDIN,"(standard input)"));
				printf("%ld\n", tln);
			}
			/* close the file */
			if (file) {
				fclose(temp);
			}
			return;
		}
	} else
		ungetc(c, temp);
	for (;;) {
next:           lnum++;
		p1 = cp;
		linebuf = lbuffer;
		while (((c = getc(temp)) != '\n') && (c != '\0')) {
			if (c == EOF) {
				if (pflag && cp != space && (match ^ vflag)
				    && !chklline && !was_long)
					succeed(file);
				/* Force last line with no newline to be checked
				 * for match . If last line has a newline, then
				 * check has already been made.
				 */
				if (chklline++ == 0 && !nlflag && !was_long)
					goto checklast;
				if (cflag && !qflag) {
					if (nfile>1 && !hflag)
						printf("%s:", file? file:MSGSTR(STDIN,"(standard input)"));
					printf("%ld\n", tln);
				}
				/* close the file */
				if (file) {
					fclose(temp);
				}
				return;
			}
			nlflag = 0;
			*p1++ = c;
			*linebuf++ = c;

		  	if (p1 - cp >= LINE_MAX ) { /* Test max char limit*/

				nsucc = (nsucc==1 && qflag) ? 1 : 2;

				fprintf(stderr, MSGSTR(LINELENERR,
				   "%s: Maximum line length of %d exceeded.\n"),
					callname, LINE_MAX);

				/* eat up remainder of long line */
				while (((c = getc(temp)) != '\n') && 
					(c != '\0') && (c != EOF)) ;
				
				if (c == EOF) {
					ungetc(c, temp);
					was_long=1;
				}
				goto next;
			}
		}

checklast:	nlflag++;
		*p1 = '\0';
		*linebuf = '\0';
		if (lnum1 == 0) {
			lnum1 = lnum;
			if (bflag)
				blkno1 = (ftell(temp) - 1) / BSIZE;
		}
		if (pflag) {
			for (i = 0; terms[i]; ++i) {
				if ( (regstat = regexec( terms[i], lbuffer, (size_t) 0, (regmatch_t *) NULL, 0) ) == 0 ) {
                                       *cp = 0; *lbuffer = 0;
					if (match ^ vflag && cp != space)
						succeed(file);
					cp = space;
					lnum1 = 0;
					match = 0;
					goto next;
				}
			}
		}

		if (!match)
			match = check_list(file, lbuffer);

		if (pflag) {
			*p1++ = '\n';
			*linebuf = '\n';
		}
		*p1 = 0;
		*linebuf = 0;

		if (!pflag) {
			if (vflag ^ match)
				succeed(file);
			lnum1 = 0;
			match = 0;
		} else {
			cp = p1;
			linebuf = lbuffer;
			if (cp >= endspace) {
				fprintf(stderr,(MSGSTR(OVFLO,
					"paragraph space overflow\n")), file);
				*cp = 0; *lbuffer = 0;
				if (match ^ vflag && cp != space)
					succeed(file);
				cp = space;
				lnum1 = 0;
				match = 0;
				goto next;
			}
		}
	}
	/* NOT REACHED */
}

/*
 * NAME: succeed
 *
 * FUNCTION:	A line was selected (note that this is *not* the same
 *		thing as saying that a match was found).  Now print
 *		out all the information	the user requested such as
 *		block number, file name, inc line count (printed at
 *		end only), or do nothing.
 *
 * Note: If a selection was made and the q flag is on, then the exit status should
 *  	 be zero even if an access or read error was detected on an earlier
 *	 file, so mark nsucc as having matched (i.e. set it to 1).  Also the -q
 *	 option specifies that nothing be printed to the standard output.
 *
 * (Globals:)
 *	nsucc is set to one if there hasn't been any previous errors.
 *	tln  is incremented.
 *
 */

static void
succeed(char *file)
{
	nsucc = (qflag || nsucc != 2) ? 1 : 2;

	if (cflag) {
		tln++;
		return;
	}
	if (lflag) {
		if (!qflag)
			printf("%s\n", file? file:MSGSTR(STDIN,"(standard input)"));
		fseek(cur_input, 0l, 2);
		return;
	}
	if (qflag || sflag && wgrep)	/* f/egrep, -s displays ONLY error */
		return;			/*   messages.			   */

	if (nfile > 1 && !hflag)
		printf("%s:", file? file:MSGSTR(STDIN,"(standard input)"));
	if (bflag)
		printf("%ld:", blkno1);
	if (nflag)
		printf("%ld:", lnum1);
	if (pflag && (nfile > 1 || nflag || bflag))
		putchar('\n');

	puts(space);
}

/* NAME: prntregerr
 *
 * FUNCTION:    Print the error message produced by regerror().
 *
 */
static void
prntregerr(int regstat, regex_t *preg)
{
	char *err_msg_buff;
	size_t sobuff;     /* size of buffer needed */

	sobuff = regerror(regstat, preg, 0, 0);
	err_msg_buff = (char *)malloc(sizeof(char)*sobuff);
	sobuff = regerror(regstat, preg, err_msg_buff, sobuff);

	fprintf(stderr, "%s\n", err_msg_buff);
	exit(2);
}

/*
 * NAME: errexit
 *
 * FUNCTION: Error printed  and program terminates.
 *
 * RETURN VALUE:  Terminates with exit code 2
 *		
 */
static void
errexit(char *s, char *f)
{
	fprintf(stderr, s, callname, f);
	exit(2);
}

/*
 * NAME: check_list
 *
 * FUNCTION:  This function will check to see if ANY of the patterns matches
 *              the input string (instring).
 *
 * RETURN VALUE: 1 = Matched.
 *               0 = Did not match.
 */
static
int
check_list(char *file, char *instring)
{
	char *tmpptr;

	s1 = head;
	if ((tmpptr = strchr(instring,'\n')) != NULL)
		*tmpptr++ = '\0';
	if (wflag) {
		match_ptr = match_offsets;
		nmatch = 1;
		tmpptr=malloc(strlen(instring)+2);
		tmpptr[0] = '\0';
		strcpy(++tmpptr,instring);
		instring = tmpptr;
		}
	else {
		match_ptr = (regmatch_t *) NULL;
		nmatch    = 0;
		}
	while (s1 != NULL) {
		tmpptr=instring;
		while (1) {/*** Exit points are inside while loop body ***/
			regstat=regexec(&s1->regpreg,tmpptr,nmatch,match_offsets,0);
			if (!regstat && (!wflag || wordmatch(match_offsets, tmpptr)))
				return(1);
			if (regstat)    /*** by now, we must be looking for
					     whole words but if ***/
				break;  /*** &s1->regpreg not found in this
					     line.  Try next pattern. ***/
			tmpptr= &tmpptr[match_offsets[0].rm_eo];  /*** if found,
					     then it's not a word need to go
					     past it 'n try again ***/
		}
		s1 = s1->next;
	}
	return (0);
}

/*
 * NAME: wordmatch
 *
 * FUNCTION:  Tests to see if the substring that was matched is a word.
 *            A word is a substring that is delimited on the left by
 *            either the begining of line or a non-alphanumeric char-
 *            acater and on the right by either the end of line or
 *            a non-alphanumeric character.
 *
 * RETURN VALUE:  0 - No match.  Is not a "word".
 *		  1 - Matched.  The substring found matches the pattern
 *			being sought and is therefor a word.
 *		
 * NOTES:  This function relies on offsets[] which gets its values from
 *	   	regexec().  regexec() is only required to do this for
 *		regular expressions but the current implementation will
 *		do it regardless.  In case of problems with word matching
 *		when using extended regular experssions, check to see if
 *		offsets[] is getting its values correctly.
 */

static int
wordmatch(regmatch_t offsets[], char *string)
{
	int s1 = offsets[0].rm_so;  	/* start of substr. matched	*/
	int s2 = offsets[0].rm_eo;	/* start of remainder of string	*/
	char *p1, *p2;
	int  tmplen, bol=0, eol=0;
	wchar_t pmbc, nmbc;

	if (MB_CUR_MAX == 1)  		/* for single-byte locales	*/
		return(	(!isalnum(string[s1-1])) &&
			(!isalnum(string[s2])) );

					/* for multi-byte locales	*/
	pmbc = nmbc = (wchar_t) 0;
	p1 = p2 = &string[s1];

	if (string[s1-1]) {		/* find previous mb character	*/
		p1 = (p1 - MAXB >= string)? p1 - MAXB : string;
		for(; p1 + (tmplen = mblen(p1, MAXB)) < p2;
                                                p1 += (tmplen>0)? tmplen : 1);
		mbtowc(&pmbc, p1, MAXB);
	} else  			/* at begining of line		*/
		bol++;

	p2 = &string[s2];
	if (*p2 == '\0')		/* at end of line 		*/
		eol++;
	else
		mbtowc(&nmbc, p2, MAXB);
		
	return ((bol || !iswalnum(pmbc)) && (eol || !iswalnum(nmbc)));
}

/*
 * NAME: insertpatterns
 *
 * FUNCTION:  Inserts an entry into the linked list which contains the patterns
 *              that may be matched by the input.  This list is used when more
 *              than one pattern will be supplied by the user.  This will
 *              happen when either the -f is used specifying a file of patterns,
 *              or when a list of patterns is quoted at the command line and
 *              separated by newlines.
 *              e.g.:
 *                      $ grep 'hello
 *                      world
 *                      3rd pattern' inputfile
 *              or
 *                      $ grep -e 'hello
 *                      world
 *                      3rd pattern' inputfile
 *              or
 *                      $ grep -f patternfile inputfile
 *
 *              Note:  insertpatterns() takes care of taking each of the
 *                      <newline> separated patterns and inserting them as
 *                      individual entries in the linked list.
 *
 * RETURN VALUE:  None.  Any errors that may occur are fatal.
 *
 */
static void
insertpatterns(char *buffer)
{
	char *ptr;
	int  firsttime=1;       /* make sure null patterns are processed */
	int  newln = 0;         /* newline flag */

	while (*buffer || firsttime) {
		firsttime=0;
		if (++wordcnt >= maxwords)
			increase_maxwords(maxwords*4);
		if ( (s1 = (struct regexpr*) malloc ( sizeof( struct regexpr))) == NULL) {
			perror(perrormsg);
			exit(2);
		}
		if (head == NULL) {
			head = s1;
			tail = s1;
		} else {
			tail->next = s1;
			tail = s1;
		}
		s1->next = NULL;

		if ((ptr = strchr(buffer,'\n')) != NULL) {
			*ptr++ = '\0';
			if (*ptr == '\n') {
				/* null pattern */
				buffer = ptr++;
				vfflag=mfflag=0; /* ensure old path is taken */
				wflag=0;  /* since null pattern is specified */
			       /* entire file must be matched, disable wflag */
				}
		}

		if (strlen(buffer)==0)	/* null pattern echos entire file */
			wflag=0;	/* so disable wflag               */

		getpreg(&s1->regpreg, buffer);
		buffer = ptr;
	}
}

/*
 * NAME: fast_insertpatterns
 *
 * FUNCTION:  Inserts an entry into list which contains the patterns
 *            that may be matched by the input.
 *
 *            Builds data structures(str_len, strcmap, str_inp, strmj) used
 *            by umatch and umatch2 routines. Also modifies vfflag to control
 *            when fast path can be taken.
 *
 * RETURN VALUE:  None.  Any errors that may occur are fatal.
 */
static void
fast_insertpatterns(char *buffer)
{
	char c, *tptr, *tmap;
	int i, k, sleng, leng, m, n;
	int *mj;       /* current line of match jump table                 */
	int brex = 0;  /* 1 if buffer has basic regular expression char    */
	int erex = 0;  /* 1 if buffer has extended regular expression char */

	leng = strlen(buffer);

	if ((*buffer==0) || (*buffer=='\n')) {   
	/* either a null string, or blank line, let mfflag handle it */
	   vfflag=0;
           mfflag=0;
	   mb_fflag=0;
           return;
        }

	while (*buffer) {

	   if (mb_fflag)
		possible_mb[*buffer]=1;

	   if (str_num >= maxwords)
			increase_maxwords(maxwords*4);
	   if ((tptr = (char *) malloc(leng + 1)) == NULL) {
	      perror(perrormsg);
	      exit(2);
	   }
	   str_inp[str_num] = tptr;

	   while ((*buffer != '\n') && (*buffer != '\0')) {
	      if (!mb_fflag && !isprint(*buffer)) {
		 vfflag = 0;
		 return;
	      }
	      if (!mb_fflag)
		      switch(*buffer) {
			 case '.' :
			 case '[' :
			 case '\\':
			 case '*' :
			 case '^' :
			 case '$' :
			    brex = 1;
			    erex = 1;
			    break;
			 case '(' :
			 case '+' :
			 case '?' :
			 case '{' :
			 case '|' :
			    erex = 1;
			    break;
			 default:
			    break;
		      }
		else {
		      int len;
	  	      len=mblen(buffer,mb_cur_max);
		      if (len==1)
		      switch(*buffer) {
			 case '.' :
			 case '[' :
			 case '\\':
			 case '*' :
			 case '^' :
			 case '$' :
			    brex = 1;
			    erex = 1;
			    break;
			 case '(' :
			 case '+' :
			 case '?' :
			 case '{' :
			 case '|' :
			    erex = 1;
			    break;
			 default:
			    break;
		        }
			while (--len>0)
			      *tptr++ = *buffer++;
		}
		
	      *tptr++ = *buffer++;
	   }

	   if (Eflag && erex) {
		vfflag = 0;
		mb_fflag = 0;
		}
	   else
		 if ((Fflag == 0) && brex) {
			vfflag = 0;
			mb_fflag = 0;
			}

	   if (!vfflag && !mb_fflag)
		return;

	   *tptr = '\0';
	   if (*buffer == '\n')
		buffer++;
	   sleng = str_len[str_num] = strlen(str_inp[str_num]);

	   if (sleng > 253) {	/* can't handle patterns larger than 253 */
	      vfflag = 0;
              mfflag = 0;
	      mb_fflag=0;
              return;
            }

	   if (mb_fflag) {
		str_num++;
		continue;
		}
		
	   if ((tmap = (char *)malloc(CTABSIZE)) == NULL) {
	      perror (perrormsg);
	      exit (2);
	   }
	   strcmap[str_num] = tmap;

	   if ((strmj[str_num]=(int *)malloc(sleng *sizeof(int) + 1)) == NULL) {
	      perror(perrormsg);
	      exit(2);
	   }
	   mj = strmj[str_num];

	   memset(tmap, '\0', CTABSIZE);

	   for (k = 0; k < sleng; k++) {
	      tmap[c = str_inp[str_num][k]] = k + BMOFF;
	   }
	   tmap[0] = 1;     /* NULL */
	   tmap['\n'] = 2;  /* LF */

	   /* Boyer Moore MatchJump calculation */
	   mj[0] = 1;
	   tptr = str_inp[str_num];
	   for (m = 1; m < sleng; m++) {
	      n = 0;
	      for (i = 1; (i < sleng) && (n <= m); i++) {
		 if (n < m) {
		    if (tptr[sleng - 1 - i] == tptr[sleng - 1 - n])
                       n++;
		    else {
		       i = i - n;
		       n = 0;
		    }
		 }
		 else { /* n == m */
		    if (tptr[sleng - 1 - i] != tptr[sleng - 1 - n])
                        n++;
		    else {
	 	        i = i - n;
		        n = 0;
		    }
		 }
	      }
	      mj[m] = i - n + m;
	   }

#ifdef GREP_DEBUG
	   /* show MatchJump */
	   if (debug_value > 0) {
	      fprintf(dbf, "<%s>", str_inp[str_num]);
	      for (k = 0; k < sleng; k++) {
		 fprintf(dbf, " %d", mj[sleng - 1 - k]);
	      }

			fputc('\n',dbf);
	   }
#endif
	   str_num++;
	}
}

/*
 * NAME: getpreg
 *
 * FUNCTION:  This function is an interface to regcomp().  It takes a null
 *		terminated pattern and compiles it.  Note that if there are
 *		any <newline>'s in the pattern, the pattern will be taken to be
 *		the string up to the character just before the first <newline>.
 *	      If the -F option (fixed string) is specified, all special
 *		characters are escaped before sending the string to regcomp().
 *	      If the -x option was specified, a circumflex (^) is added at the
 *		begining of the pattern and a dollar sign ($) is added at the
 *		end so that it will only match if the entire line is used up in
 *		the match.
 *
 * RETURN VALUE:  none.  Any errors that may occure are fatal.
 *		
 */
static void
getpreg(regex_t *localpreg, char *localpattern)
{
	int	locali;
	char	protectedbuf[LINE_MAX * 2 + 1],
		*protbufptr = &protectedbuf[0],
		*ptr;

	if ((ptr = strchr(localpattern, '\n')) != NULL)
		*ptr = '\0';

	if (xflag || Fflag) {
		if (xflag)
			*protbufptr++ = '^';

		ptr = localpattern;
		while(*ptr) {
			if (Fflag)
				switch(*ptr) {
					case '$' :
					case '\\':
					case '[' :
					case '*' :
					case '?' :
					case '+' :
					case '.' :
					case '^' :
						*protbufptr++ = '\\';
				}
			
			locali = mblen(ptr, mb_cur_max);
			if (locali<1)
				locali = 1;
			for (; locali; locali--)
				*protbufptr++ = *ptr++;
		}

		if (xflag)
			*protbufptr++ = '$';

		*protbufptr = '\0';
		localpattern = protectedbuf;
	}

	if ((regstat = regcomp(localpreg, localpattern, regflags) ) != 0)
		prntregerr(regstat, localpreg);
}

/*
 * NAME: checkfile
 *
 * FUNCTION:  This function checks to see if a file exists by trying to open
 *		it for read access.  Note that it only checks to see if it is
 *		there and immediately closes it or exits with an error message.
 *
 * RETURN VALUE:  none.  Any errors that may occure are fatal.
 *		
 */
static void
checkfile(char *file)
{
	struct stat statb;

	if (stat(file, &statb) < 0) {
	   fprintf(stderr,MSGSTR(OPERR, "%s: can't open %s\n"), callname, file);
	   exit(2);
	} else if (!(S_ISREG(statb.st_mode) || S_ISFIFO(statb.st_mode))) {
	   fprintf(stderr,MSGSTR(FMODE, "%s: %s must be a regular or fifo file\n"), callname, file);
	   exit(2);
	} else if (S_ISREG(statb.st_mode) && (statb.st_size == 0)) {
	   fprintf(stderr,MSGSTR(NOPAT, "%s: missing pattern in %s\n"), callname, file);
	   exit(2);
	}
}


/* increase_maxwords()								*/
/* increase the memory allocation used to support more matching patterns        */
/* The variable maxwords is changed to the specified value.  New memory         */
/* is allocated for epatterns, str_len, strcmap, strinp, and strmj.             */
/* Old values in these arrays are copied into the newly allocated memory,       */
/* and the old arrays are freed.                                                */

static void increase_maxwords(int newlimit)
{
	int i;
	char **new_epatterns;
	int  *new_str_len;
	char **new_strcmap;
	char **new_str_inp;
	int  **new_strmj;

	/* allocate new memory */
	if (((new_epatterns=malloc(newlimit*sizeof(char *)))==NULL)
		|| ((new_str_len=malloc(newlimit*sizeof(int)))==NULL)
		|| ((new_strcmap=malloc(newlimit*sizeof(char *)))==NULL)
		|| ((new_str_inp=malloc(newlimit*sizeof(char *)))==NULL)
		|| ((new_strmj=malloc(newlimit*sizeof(int *)))==NULL)) {
			perror(callname);
			exit(2);
			}
		
	/* if old data exists, copy it and free memory */
	if (maxwords!=0) {
		for (i=0;i<maxwords;i++) {
			new_epatterns[i]=epatterns[i];
			new_str_len[i]=str_len[i];
			new_strcmap[i]=strcmap[i];
			new_str_inp[i]=str_inp[i];
			new_strmj[i]=strmj[i];
			}

		free(epatterns);
		free(str_len);
		free(strcmap);
		free(str_inp);
		free(strmj);
		}

	/* assign new pointers */
	epatterns=new_epatterns;
	str_len=new_str_len;
	strcmap=new_strcmap;
	str_inp=new_str_inp;
	strmj=new_strmj;
	
	maxwords = newlimit;
} /* increase_maxwords() */


/* buffer management */
static char blk0[PAGE_SIZE + LUL + 2];		/* input buffer            */
static char blk1[PAGE_SIZE + LUL + 2];		/* input buffer            */
static char * bptr;				/* ptr to last read buffer */
static int btotb;				/* total bytes read        */
static int bcnt, bcnt0, bcnt1;			/* size reads              */
static int b_eof;				/* End Of File             */
static char * bend;				/* End of Block            */

/*
 * NAME: bback
 *
 * FUNCTION: Provides capability to backup from the most recently read
 *           raw I/O buffer to the previously read buffer.
 *
 * PARAMETERS: *s - char ptr into 1 of the 2 I/O buffers(blk0, blk1)
 *             n  - amount to back up.
 *
 * RETURN VALUE: pointer into the other, previously read buffer.
 *
 */
static char * bback(char *s, int n)
{
	char * rtn;

	if (btotb <= PAGE_SIZE)  /* still in first block of data */
		return(0);
	if (bptr == blk0)
		rtn = blk1 + bcnt1 - n;
	else
		rtn = blk0 + bcnt0 - n;
	return(rtn);
}

/* this macro ensures that enough data is read in so that	*/
/* *s references valid data					*/

#define FULLBREAD(fd,s)  do {s=bread(fd,s);} while ((s>=bend) && (b_eof==0));

/*
 * NAME: bread
 *
 * FUNCTION: Provides low level I/O handling.  Reads data in quickly using
 *           two buffers.  If the current block is full, then read data
 *           the other buffer.  Otherwise, attempt to append data to the 
 *           current buffer.  (appending solves problems with partial
 *           reads inherent to some pipe operations.)
 *
 * PARAMETERS: fd - file descriptor of file to read.
 *             *s - ptr to current buffer.
 *
 * RETURN VALUE: Pointer into the newly read data at the correct offset.
 *
 */
static char * bread(int fd, char * s)
{
	int new_offset;		/* distance into new block */
	int current_count;	/* number of bytes read in this time */
	int i;

	if (bptr == blk1) {
		if (bcnt >= PAGE_SIZE) { /* full read new block */
			new_offset = s - bptr - bcnt;
			bcnt = 0;
			bcnt0 = current_count = read(fd, blk0, PAGE_SIZE);
			bptr = blk0;
			s = bptr + new_offset;
			}
		else { /* partial read old block */
			current_count = read(fd, blk1+bcnt, PAGE_SIZE-bcnt);
			bcnt1 += current_count;
			}
		}
	else {
		if (bcnt >= PAGE_SIZE) { /* full read new block */
			new_offset = s - bptr - bcnt;
			bcnt = 0;
			bcnt1 = current_count = read(fd, blk1, PAGE_SIZE);
			bptr = blk1;
			s = bptr + new_offset;
			}
		else { /* partial read old block */
			current_count = read(fd, blk0+bcnt, PAGE_SIZE-bcnt);
			bcnt0 += current_count;
			}
		}

	bcnt += current_count;
	bend = bptr + bcnt;
	btotb += current_count;

	if (current_count > 0)
		memset(bend, '\0', LUL);
	else
		bptr = 0;

	if (current_count <= 0) {
		b_eof = 1;
		if (current_count < 0 ) {
			perror (callname);
			nsucc = 2;
		}
	}
	return(s);
}

/*
 * NAME: umatch
 *
 * FUNCTION: Provides a faster grep path, for a limited set of input
 *           parameters. The primary restrictions are that there be only
 *           1 input string and that the string be simple, i.e. isprint
 *           characters only.
 *           The speed is achieved in two ways, 1) I/O is handled in a
 *           more basic fashion and 2) the Boyer-Moore string matching
 *           algorithm is used, so that not all characters in the input
 *           file need to be looked at.
 *           Reference: Computer Algorithms, Sara Basse, 1988, Addison
 *           Wesley, pp. 220-228.
 *
 * GLOBAL VARIABLES: nflag, xflag, lflag, space.
 *
 * PARAMETERS:
 *       fd     - File descriptor of file to be searched.
 *       comstr - Single input string to be searched for.
 *       cmap   - Character indexed table. Non zero entries in the table
 *                denote characters in the input string as well as the
 *                NULL character. The specific value of an entry denotes
 *                the last position in the input string that the given
 *                character occur at.
 *       strleng- The length of the input string.
 *       mj     - A table, built in fast_insertpatterns, that specifies how far
 *                forward it is possible to move after a partial string
 *                match. The move is such that it is guaranteed that a
 *                match to the target string will not be entirely skipped.
 *       file   - Name of input file being seached.
 *
 * RETURN VALUE: void.
 *       The succeed routine is called for each line matched.
 */
static void
umatch(int fd, char * comstr, char * cmap, int strleng, int * mj, char * file)
{
	char c, buf[2 * LINE_MAX + 2 + 1];
	int cpos, d, dd, m, i;
	char * s, * e;
	int ilen, ifrt;

	lnum1 = 1; /* global from grep */
	bptr = NULL;
	bcnt = btotb = b_eof = 0;

	cmap[LF] = 0; /* -n support by MedFast */

	s = 0;
	bcnt = PAGE_SIZE;    /* force buffer initialization in bread() */
	bptr = (char *)-bcnt;/* ensure s points to start of buffer...  */
	if (bptr == blk1)	
		bptr++,bcnt--;
	FULLBREAD(fd, s);    /* first read of file */
	s += strleng - 1;    /* earliest end-of-string */

	while (b_eof == 0) {
		if (strleng <= LUL)         /* NULL pad detects buf overflow */
			while (!cmap[*s])
				s += strleng;
		else
			while ((s < bend) && !cmap[*s])
				s += strleng;

		if (s >= bend) {          /* overflow ? */
			FULLBREAD(fd, s); /* read next buffer */
			continue;
		}

		cpos = cmap[*s];
		if (cpos < BMOFF)   /* NULL && !overflow ?? */
			s += strleng;
		else
			s += strleng - 1 - (cpos - BMOFF); /* new end point */

		if (s >= bend)  {          /* buf overflow ? */
			FULLBREAD(fd, s);  /* read next buffer */
			continue;
		}

		e = s; /* match end point */

		m = 0;  /* match backward */
		while (((c = *s) == comstr[strleng - m - 1])) {
			m++;
			if (m == strleng) /* match found */
				break;
			if (s == bptr)
				s = bback(s, 1);
			else
				s--;
		}

		if (m < strleng) { /* Boyer-Moore MatchJump */
		/* After a partial match, compute maximum jump forward(d).
		 * d = max(charJump, matchJump) from the present position(s).
		 * charJump is based on the actual file character(*s) that was
		 * found to be a mismatch.  matchJump(mj[]) is a table 
		 * built in fast_insertpatterns. It is only a function
		 * of how far you got in matching the input string
		 */
			c = cmap[*s];
			if (c != 0)
				d = strleng - 1 - (c - BMOFF); /* charJump */
			else
				d = strleng;
			dd = d;
			if (d < mj[m])
				d = mj[m];  /* maxJump */
			s = e - m + d;
                            /* Conceptually s += d. However this
			     * form is to avoid problem when match starts in
			     * 1 buffer and backs into the previous buffer.
			     */
			continue;
		}

      /* Reconstruct Matched Line. Backward from the end of the string(e) */
      /* Back phases stops at LF, Beginning of File, or Line too long */

		s = e;
		i = LINE_MAX;
		while ((i > 0) && ((c = *s) != LF)) {
			buf[--i] = c;
			if (s == bptr) {
				s = bback(s, 1);
				if (s == 0)   /* beginning of file */
					break;
			}
			else
				s--;
		}
		ifrt = i;

      /* Reconstruct line forward from end of string match(e) */
      /*   stops at LF, End of File, or Line too long */

 		i = LINE_MAX - 1;
		s = e;
		while ((c = *s) != LF) {
			if (i < 2 * LINE_MAX)
				buf[i++] = c;
			s++;
			if (s >= bend) {
			        FULLBREAD(fd, s);
				if (b_eof == 1)
					break;
			}
		}

		/* Output matched line */
		ilen = i - ifrt;
		if (ilen <= LINE_MAX) {
			buf[i] = ZERO;  /* LF no! ZERO yes! */
			blkno1 = (btotb - bcnt + (s - bptr)) / BSIZE;
			space = &buf[ifrt];
			succeed(file);
			lnum1++;
			if (lflag == 1)
				return;
		}
		else {
                        nsucc = (nsucc==1 && qflag) ? 1 : 2;

			fprintf(stderr, MSGSTR(LINELENERR,
                                   "%s: Maximum line length of %d exceeded.\n"),
                                   callname, LINE_MAX);
		}
		s += strleng;
	} /* EOF loop */
}

/*
 * NAME: umatch2
 *
 * FUNCTION: Provides grep performance intermediate between umatch and the
 *           normal grep path.
 *           umatch2 can handle multiple simple input strings. Also
 *           normal I/O is used, i.e. fgets to get the next line.
 *
 * GLOBAL VARIABLES: str_inp, strcmap, str_len, strmj.
 *
 * PARAMETERS:
 *       line   - The present line being seached for a match.
 *       lleng  - Length of line.
 *
 * RETURN VALUE: Pointer into the newly read buffer at the correct offset.
 *
 */
static int
umatch2(char * line, int lleng)
{
	char c, * s, * str, * cmap;
	int sleng, ii, d, dd, m, cpos;
	char * lend;
	int * mj;

	for (ii = 0; ii < str_num; ii++) {
		str   = str_inp[ii];
		cmap  = strcmap[ii];
		sleng = str_len[ii];
		mj    = strmj[ii];

		s = line + sleng - 1;
		lend = s + lleng;

		while (s < lend) {
			if (sleng <= LUL)   /* uses ZERO pad to escape */
				while (!cmap[*s])
					s += sleng;
			else
				while ((s < lend) && !cmap[*s])
					s += sleng;
			if (s >= lend)
				break;

			cpos = cmap[*s];
			if (cpos < BMOFF) {  /* LF, ZERO */
				if (*s == LF)
					break;
				s += sleng;
				continue; /* ZERO case */
			}
			s = s + sleng - 1 - (cpos - BMOFF);
			if (s >= lend)
				break;

			m = 0;
			while (str[sleng - 1 - m] == *s) {
				m++;
				if (m == sleng)
					return(1);
				s--;
			}

			/* MatchJump table. */
			c = cmap[*s];
			if (c != 0)
				d = sleng - 1 - (c - BMOFF);
			else
				d = sleng;
			if (d < mj[m])
				d = mj[m];
			dd = d;
			s += d;
		}
	}
	return(0);
}



/*
 * NAME: mb_umatch
 *
 * FUNCTION: Provides grep performance faster than the normal calling
 * 	     regexec()
 *	     
 *           mb_umatch() can handle simple patterns
 *
 * GLOBAL VARIABLES: str_inp, str_len, str_num
 *
 * PARAMETERS:
 *       line   - The present line being seached for a match.
 *       lleng  - Length of line.
 *
 * RETURN VALUE: 0 or 1 depending on existance of pattern in line
 *
 */
static int
mb_umatch(char * line, int lleng)
{
char *pline,*p1,*p2;
int  pats=0;
int  len;

pline=line;
while (*pline) {
	if (possible_mb[*pline]) {
		pats=0;
		while (pats<str_num) {
			p1=pline;
			p2=str_inp[pats];
			while (*p1==*p2 && *p2)
				p1++,p2++;
			if (*p2==0)
				return (1);
			pats++;
			}
		}
	len=mblen(pline,mb_cur_max);
	pline+=(len<1?1:len);
	}
return (0);
}
