static char sccsid[] = "@(#)71  1.19  src/bos/usr/bin/cmp/cmp.c, cmdfiles, bos412, 9446C 11/14/94 16:47:33";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: cmp
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
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 * 
 * OSF/1 1.1
 *
 */

#include	<stdio.h>
#include	<ctype.h>
#include	<locale.h>
#include	<sys/stat.h>
#include "cmp_msg.h"
#define	MSGSTR(Num, Str) catgets(catd,MS_CMP,Num,Str)
#define USAGE_MESSAGE() \
	fprintf(stderr,MSGSTR(USG,"usage: cmp [-l | -s] file1 file2\n"));\
	exit(2)

static nl_catd catd;

/*
 * NAME: cmp [-l | -s] file1 file2 
 *                                                                    
 * FUNCTION: Compares two files
 *                                                                    
 * NOTES:   Compares two files sending the differences to standard out.
 *          If a '-' is given for file1 or file2, then cmp reads from      
 *          standard in for that file.  Two hyphens cannot be entered for
 *          both input files.  The default output is that cmp displays
 *          nothing if the files are the same.  If the files differ, cmp
 *          displays the byte and line number at which the first 
 *          difference occurs.
 *          -l    Displays, for each difference, the byte number in
 *                decimal and the differing bytes in octal.
 *          -s    Returns only an exit value.
 *
 * RETURN VALUE DESCRIPTION: 
 *          0  indicates identical files.
 *          1  indicates different files.
 *          2  indicates inaccessible files or a missing argument.
 */  
main(argc, argv)
int argc;
char **argv;
{
	FILE	*file1, *file2;             /* the files to be compared */
	struct	stat buf1, buf2;
	int 	ch;
	int	c1, c2;           /* current charcters from file1 and file2 */
	char 	file1_buf[LINE_MAX+1];
	char 	file2_buf[LINE_MAX+1];
	long	line = 1;                   /* line number */
	long	byte = 0;                   /* byte number */
	int	dflg = 0;                   /* difference flag */
	int	lflg = 0;                   /* long output flag */
	int	sflg = 0;                   /* short output flag */

	(void) setlocale(LC_ALL,"");

	catd = catopen(MF_CMP, NL_CAT_LOCALE);

	while ((ch = getopt(argc, argv, "ls")) != EOF)
		switch(ch) {
		case 'l':		/* print all differences */
			lflg++;
			break;
		case 's':		/* silent run */
			sflg++;
			break;
		default:
			USAGE_MESSAGE();	
		}
	if (lflg && sflg) {			     /* mutually exclusive */
		USAGE_MESSAGE();
	}

	argv += optind;
	argc -= optind;

	if (argc != 2){				     /* ensure appropriate */
		USAGE_MESSAGE();		     /* no. of arguments */ 
	}

	if (!strcmp(argv[0], "-"))                   /* file1 from stdin */
		file1 = stdin;
	else if((file1 = fopen(argv[0], "r")) == NULL) { /* bad file */
		fprintf(stderr, MSGSTR(NOTOPEN, "cmp: cannot open %s\n"), argv[0]);
		exit(2);
	}

	if (!strcmp(argv[1], "-"))                   /* file2 from stdin */	
		file2 = stdin;
	else if((file2 = fopen(argv[1], "r")) == NULL) { /* bad file */
		fprintf(stderr, MSGSTR(NOTOPEN, "cmp: cannot open %s\n"), argv[1]);
		exit(2);
	}

	/* both files can't be stdin */
	if (file1 == stdin && file2 == stdin) {	    
		fprintf(stderr, MSGSTR(BADFILES, "File1 and File2 can not both refer to stdin.\n"));
		USAGE_MESSAGE();				   
	}

	if (sflg && file1 != stdin && file2 != stdin) {
		if (stat(argv[0], &buf1) || stat(argv[1], &buf2)) {
			perror("cmp");
			exit(2);
		}
		if (buf1.st_size != buf2.st_size)
			exit(1);
	}

	while (TRUE) {
                c1 = getc(file1);
                c2 = getc(file2);
                byte++;
                if (c1 == c2) {
                        if (c1 == '\n')
                                line++;
                        if (c1 == EOF)
                                exit(dflg ? 1 : 0);
                } else {
			if (sflg)
				exit(1);
                        if (c1 == EOF)
                                earg(argv[0]); /* file1 shorter than file2 */
                        if (c2 == EOF)
                                earg(argv[1]); /* file2 shorter than file1 */
			if (lflg == 0) {       /* default output, exit */
				printf(MSGSTR(DIFF,
					"%s %s differ: char %d, line %d\n"),
					argv[0], argv[1], byte, line);
				exit(1);
			}
                        dflg = 1;
                        printf("%6ld %3o %3o\n", byte, c1, c2);
                }
        } /* end of while */
}

static
earg(char *arg)
{
        fprintf(stderr, MSGSTR(EOFMSG,"cmp: EOF on %s\n"), arg);
        exit(1);
}
