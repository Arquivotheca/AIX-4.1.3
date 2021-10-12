static char sccsid[] = "@(#)46	1.14  src/bos/usr/bin/diff/diff.c, cmdfiles, bos412, 9446C 11/14/94 16:48:14";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: diff
 *
 * ORIGINS: 3, 18, 26, 27, 71
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
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 */

#include <stdio.h>
#include <locale.h>
#include <stdlib.h>

#include <ctype.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include "diff.h"

nl_catd catd;

/* Determine whether current locale is SBCS or MBCS for diffdir and diffreg.*/
static int mb_cur_max; /* max number of bytes per character in current locale */
int mbcodeset;  /* 0=current locale SBCS, 1=current locale MBCS */
/* Let diffdir and diffreg implement "wclen(wc)" by "wctomb(ignore_mb,wc)" */
char ignore_mb[MB_LEN_MAX];

/*
 * Output format options
 */
int        opt;
int        tflag=0;                        /* expand tabs on output */

/*
 * Algorithm related options
 */
int        hflag=0;                    /* -h, use halfhearted DIFFH */
int        bflag=0;                    /* ignore blanks in comparisons */
int        wflag=0;                    /* totally ignore blanks in comparisons*/
int        iflag=0;                    /* ignore case in comparisons */


/*
 * Options on hierarchical diffs.
 */
int        lflag=0;                      /* long output format with header */
int        rflag=0;                      /* recursively trace directories */
int        sflag=0;                      /* announce files which are same */
char       *begin = NULL;                /* do file only if name >= this */

/*
 * Variables for -I D_IFDEF option.
 */
int        wantelses;                    /* -E */
char       *def1;                        /* String for -1 */
char       *def2;                        /* String for -2 */

/*
 * Variables for -c context option.
 */
int        context;                /* lines of context to be printed */

/*
 * State for exit status.
 */
int        status;
int 	   oldstatus;
char        *tempfile;        /* used when comparing against std input */

/*
 * Variables for diffdir.
 */
char        **diffargv;        /* option list to pass to recursive diffs */
char	*slinkcmpsbuf1;		/* Allocate symbolic link buffers outside recursion */
char	*slinkcmpsbuf2;

/*
 * Input file names.
 * With diffdir, file1 and file2 are allocated BUFSIZ space,
 * and padded with a '/', and then efile0 and efile1 point after
 * the '/'.
 */
char        *file1, *file2, *efile1, *efile2;
struct        stat stb1, stb2;
/*
 * This is allocated early, and used
 * to reset the free storage pointer to effect space compaction.
 */
char        *dummy;
char        *talloc(), *ralloc();
char        *savestr();
int        done();

/*
 * Variable for passing NAME_MAX to diffreg and diffdir
 */
int	name_max;

/*
 * NAME:   diff - driver and subroutines
 * FUNCTION:  compare two files
 *   FLAGS:   
 *    Options when comparing directories are:
 *    -l       long output format; each text file diff is piped through pr to 
 *             paginate it, other differences are remembered and summarized 
 *             after all text file differences are reported.
 *   -r        causes application of diff recursively to common subdirectories
 *   -s        casuses diff to report files which are the same, which are 
 *             otherwise not memtioned. 
 *   -Sname    starts a directory diff in the middle beginning with file name.
 *   The following options are mutually exclusive: 
 *   -e        produces a script for the command ed
 *   -f        produces a script similar to -e but in the opposite order
 *   -n        produces a script similar to that of -e, but in the opposite 
 *             order and with a count of changed lines on each insert or 
 *             delete command.
 *   -c        produces a diff with lines of context
 *   -h        does a fast, half-hearted job.
 *   -Dstring  cause diff to create a merged version of file1 and file2,
 *             with C preprocessor controls included.
 *  Other flags
 *  -b         causes trailing blanks (spaces and tabs) to be ignored, and
 *             other strings of blanks to compare equal.
 *  -w         causes whitespace (blanks and tabs) to be totally ignored.
 *  -i         ignores the case of letters
 *  -t         will expand tabs in output lines
 */


#ifdef DIFF
char        diff[] = DIFF;
#else
char        diff[] = "/usr/bin/diff";
#endif
#ifdef DIFFH
char        diffh[] = DIFFH;
#else
char        diffh[] = "/usr/lbin/diffh";
#endif
#ifdef PR
char        pr[] = PR;
#else
char        pr[] = "/usr/bin/pr";
#endif

main(argc, argv)
        int argc;
        char **argv;
{
        register char *argp;
	extern char *optarg;
	extern int optind, optopt, opterr;
	int c; /* char for getopt() value */
	unsigned char ucc; /* c disguised as char */
	unsigned char uco; /* optopt disguised as char */
	char **ep;
	int badopt;
	int mb_cur_max; /* max number of bytes per character in current locale */

        (void) setlocale(LC_ALL,"");

        catd = catopen(MF_DIFF, NL_CAT_LOCALE);
	mb_cur_max = MB_CUR_MAX;
	mbcodeset = (mb_cur_max > 1?1:0);

        def1 = "FILE1"; def2 = "FILE2";
	oldstatus = status = 0;
        diffargv = argv;
	badopt = 0;
			/* Option -c is allowed to have traditional optional argument.
			 * This version of option handling uses POSIX 1003.2/D10 getopt().
			 * This version will not work with the 1003.2/D11 version of
			 * getopt() because missing-argument conventions are different.
			 */
	opterr = 0;
	while ((c = getopt(argc,argv,"bcC:D:efhilnrsS:tw")) != -1) {
		ucc = (unsigned char)c;
		switch (ucc) {
		case 'b':
                        bflag = 1;
			break;
		case 'c':
                       	opt = DI_CONTEXT;
			context = 3;
			break;
		case 'C':
			/* optarg must point to a positive numeric argument */
			ep = &optarg;
			if ( optarg != NULL
			     && ((context = (int)strtoul(optarg,ep,10)) >= 0)
			     && (ep == NULL || *ep == NULL || **ep=='\0')) {
                        	opt = DI_CONTEXT;
			} else {
				fprintf(stderr,MSGSTR(EBADCNT3,
					"diff: -%c: bad count\n"),ucc);
				status=2;
				badopt++;
			}
			break;

		case 'D':
			/* optarg must point to valid -D argument */
                        wantelses = 1;
                        def1 = "";
                        opt = DI_IFDEF;
                        def2 = optarg;
			break;
		case 'e':
                        opt = DI_EDIT;
			break;
		case 'f':
                        opt = DI_REVERSE;
			break;
		case 'h':
                        hflag = 1;
			break;
		case 'i':
                        iflag = 1;
			break;
		case 'l':
                        lflag = 1;
			break;
		case 'n':
                        opt = DI_NREVERSE;
			break;
		case 'r':
                        rflag = 1;
			break;
		case 's':
                        sflag = 1;
			break;
		case 'S':
			/* optarg points to -S argument. To disable -S effects below top
			 * level operands, use empty string optarg as convention for
			 * recursive call out of diffdir, and ignore -S in that case.
			 * (Same effect as if user said  diff -S "" .)
			 * Method: remember -S argument here and mark it null in argv[].
			 * When argv is passed in recursive call (via diffargv) it will
			 * look like  -S "" .
			 */
			if (strcmp(optarg,"")!= 0) {
				begin = strcpy((char *)malloc(strlen(optarg)+2),optarg);
				*optarg = '\0';
			}
			break;
		case 't':
                        tflag = 1;
			break;
		case 'w':
                        wflag = 1;
			break;
		case '?': /* Mimic getopt() error messages where appropriate */ 
			uco = (unsigned char)optopt;
			switch (uco) {
			case 'D': /* Omitted String in [-D String] : missing argument */
			case 'C': /* Omitted Number in [-C Number] : missing argument */
			case 'S': /* Omitted File in [-S File] : missing argument */
				fprintf(stderr,MSGSTR(EOPTARG,
					"diff: option requires an argument -- %c\n"),uco);
				status=2;
				badopt++;
				break;
			default: /* Anything else: illegal option */
				fprintf(stderr,MSGSTR(EILLOPT,
					"diff: illegal option -- %c\n"),uco);
				status=2;
				badopt++;
				break;

			} /* switch (uco) */
			break;
		} /* switch(ucc) */
	} /* while ((c = getopt ... */
		/* Check for exactly two File or Directory operands
		 * (allowing for mystery machinations by getopt())
		 */
	if ((argc - optind) == 2) {
        	file1 = argv[optind  ];
        	file2 = argv[optind+1];
	} else {
                fprintf(stderr,MSGSTR(ETWO, 
                        "diff: two filename arguments required\n"));
				status=2;
                badopt++;
        }
        if (hflag && opt) {
                fprintf(stderr,MSGSTR(EBAD, 
                        "diff: -h doesn't support -e, -f, -n, -c, or -D\n"));
				status=2;
                badopt++;
        }

	if (badopt) {
		fprintf(stderr,MSGSTR(USAGE,
"Usage: diff [-bcitw] [[-C Lines|-D String|-e|-f|-n]|[-h]] File1 File2\n\
       diff [-bcilrstw] [[-C Lines|-e|-f|-n]|[-h]] [-S File] Directory1 Directory2\n"
));
		status=2;
		exit(status);
	}

        if (!strcmp(file1, "-"))
                stb1.st_mode = S_IFREG;
        else if (stat(file1, &stb1) < 0) {
					 fputs("diff: ",stderr);
                perror(file1);
		status=2;
                done();
        }
        if (!strcmp(file2, "-"))
                stb2.st_mode = S_IFREG;
        else if (stat(file2, &stb2) < 0) {
					 fputs("diff: ",stderr);
                perror(file2);
		status=2;
                done();
        }

	/* Get NAME_MAX value if at least one operand is a regular file */
	if (   ((name_max=pathconf(file1,_PC_NAME_MAX)) == -1)
	    && ((name_max=pathconf(file2,_PC_NAME_MAX)) == -1)) {
		if ( strcmp(file1,"-")==0 && strcmp(file2,"-")==0)
			fprintf(stderr,MSGSTR(DSTDINS,
			"diff: can't specify - -\n"));
		else 
			fprintf(stderr,MSGSTR(ENOPATHMAX,
			"diff: Cannot get _PC_NAME_MAX.\n"));
		status=2;
		done();
	}

        if ((stb1.st_mode & S_IFMT) == S_IFDIR &&
            (stb2.st_mode & S_IFMT) == S_IFDIR) {
		if ( (slinkcmpsbuf1 = (char *)malloc(name_max+2)) == NULL || 
		     (slinkcmpsbuf2 = (char *)malloc(name_max+2)) == NULL ) {
			fprintf(stderr,MSGSTR(ESLINK,
				"diff: Cannot allocate symbolic link space.\n"));
			status=2;
			done();
		};
                diffdir(argv);
        } else
                diffreg();
        done();
}

/* save a copy of string cp */
char *
savestr(cp)
register char *cp;
{
        register char *dp;

	dp = (char *) malloc((size_t)(strlen(cp)+1));
        if (dp == 0) {
                fprintf(stderr,MSGSTR(EMEM,"diff: ran out of memory\n"));
                done();
        }
        strcpy(dp, cp);
        return (dp);
}

/* return the integer with the minimun value */
min(a,b)
        int a,b;
{

        return (a < b ? a : b);
}

/* return the integer with the maxinum value */
max(a,b)
        int a,b;
{

        return (a > b ? a : b);
}

/* delete tempfile and exit */
done()
{
        unlink(tempfile);
        exit(status);
}

/*   malloc with test */
char *
talloc(n)
{
        register char *p;
        p = malloc((size_t)n);
        if(p!=NULL)
                return(p);
        noroom();
}

char *
ralloc(p,n)        /*compacting reallocation */
char *p;
{
        register char *q;
        q = realloc((void *)p, (size_t)n);
        if(q==NULL)
                noroom();
        return(q);
}

/* out of memory, print error message and exit */
static
noroom()
{
        fprintf(stderr,MSGSTR(ESIZE,"diff: files too big, try -h\n"));
        done();
}
