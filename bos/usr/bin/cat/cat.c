static char sccsid[] = "@(#)33	1.34  src/bos/usr/bin/cat/cat.c, cmdscan, bos41J, 9521A_all 5/23/95 17:07:32";
/*
 * COMPONENT_NAME: (CMDSCAN) commands that scan files
 *
 * FUNCTIONS:
 *
 * ORIGINS: 3, 26, 27, 71
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
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.1
 */

#define _ILS_MACROS

#include	<stdio.h>
#include	<stdlib.h>
#include        <fcntl.h>
#include        <sys/types.h>
#include        <sys/stat.h>
#include 	<locale.h>
#include	<errno.h>
#include	<nl_types.h>
#include 	<wchar.h>
#include	<ctype.h>
#include	"cat_msg.h"

#define 	BUFSIZE BUFSIZ	  /* BUFSIZ buffer for all others */
#define MSGSTR(num,str) catgets(catd,MS_CAT,num,str)  /*MSG*/
static nl_catd catd;

static char	buffer[BUFSIZE];
 
static int	silent = 0;
static int	bflag = 0;			/* Don't count blank lines.  */
static int	eflag = 0;			/* Put $ at the end of lines */
static int	nflag = 0;			/* Number lines.             */
static int	sflag = 0;			/* Squeeze blank lines to one*/
static int	tflag = 0;			/* display tabs as ^I.       */
static int	vflag = 0;			/* display all control chars.*/

static int	spaced = 0;
static int	inline = 0;
static void copyopt();
static void copyopt_ascii();
static void copyopt_mb();

/*
 * There are just a few changes to make cat faster under certain
 * circumstances. The standard code (scat) is called, if the option
 * are used or input to cat is stdin. Switching between standard 
 * and the fast code is done by 'main()'!                           
 */
 

/*
 * NAME: fcat
 *                                                                    
 * FUNCTION:
 *		Faster cat when no options are specified (except
 *		-q for quiet mode).
 *                                                                    
 * RETURN VALUE DESCRIPTION: 
 *
 *		Exits with status 0 if successful, 2 if failure.
 */  

static void
fcat(argc,argv)
int argc;
char **argv;
{
        char *file;
        register int cnt, fd, args;
        int status = 0;
        int dev, ino = -1;
        struct stat statb;
	int tmpI;

        if (fstat((int)fileno(stdout), &statb) < 0) {
		if (!silent)
                        fprintf(stderr, MSGSTR(ESTATOUT,"cat: cannot stat stdout\n"));
          	exit(status = 2);
        }
 
        statb.st_mode &= S_IFMT;
        if (statb.st_mode != S_IFCHR && statb.st_mode != S_IFBLK) {
        	dev = statb.st_dev;
          	ino = statb.st_ino;
        }
        for (args = 1; args < argc; args++) {
                if ((argv[args][0] == '-') && (argv[args][1] == 'q')) {
			silent++;
			continue;
		}
                file = argv[args];
                if (((fd = open(file,O_RDONLY)) == -1) && (!silent)) {
			fprintf(stderr,MSGSTR(EOPEN,"cat: cannot open %s\n"),file);
                        status = 2;
                        continue;
                }
                if (fstat(fd, &statb) < 0) {
                	if (!silent)
                                fprintf(stderr,MSGSTR(ESTAT,"cat: cannot stat %s\n"),file);
                      	status = 2;
                      	continue;
                }
                while ((cnt = read(fd, buffer, BUFSIZE)) > 0) {
                	if ( cnt != (tmpI = write(fileno(stdout), buffer, cnt)) ) {
				if (!silent) {
					fprintf(stderr,MSGSTR(EOUTPUT,"cat: output error\n"));
					perror("");
				}
				status = 2;
				break;
			}
		}
		if (cnt<0) {
			fputs("cat: ",stderr);
			perror(file);
			status = 2;
			/* we need to close the input file, so fall through */
  		} 
                if ((close(fd) != 0) && (!silent)) {
                	fprintf(stderr,MSGSTR(ECLOSE,"cat: close error\n"));
                    	status = 2;
                }
        }

        /* Defect 44200 */
        errno = 0;
        fclose(stdout);
        if (errno == ENOSPC || errno == EDQUOT)
        {
                perror("cat");
                status = 2;
        }
	exit(status);
}
 

/*
 * NAME: scat
 *                                                                    
 * FUNCTION: 
 *		Concatenates files together.  If options besides -q are
 *		used, this code will handle it.  Otherwise fcat is called.
 *                                                                    
 * RETURN VALUE DESCRIPTION: 
 *			    
 *		Exit 0 upon successfull completion, two otherwise.
 */  
static void
scat(argc, argv)
int argc;
char **argv;
{
        FILE *fi;
        register int c;
	int	stdinflg = 0;
        int     status = 0;
        int     dev, ino = -1;
        struct  stat statb;
	int	asciipath;
	char	* loc_val;
	int	uflag=0;
 
#ifdef STANDALONE
        if (argv[0][0] == '\0')
                argc = getargv("cat", &argv, 0);
#endif
	while ((c = getopt(argc, argv, "uSsrqvbnte")) != EOF ) {
		switch (c) {
                	case 'u':
#ifndef STANDALONE
                        	setbuf(stdout, (char *) NULL);
				uflag=1;
#endif
                        	continue;
			case 'S':		/* squeeze */
			case 'r':
				sflag++;
				continue;
			case 's':
                	case 'q':
                        	silent++;
                        	continue;
			case 'v':
				vflag++;
				continue;
			case 'b':
				bflag++;
			case 'n':
				nflag++;
				continue;
			case 't':
				tflag++;
				vflag++;
				continue;
			case 'e':
				eflag++;
				vflag++;
				continue;
			case '?':
				if (!silent)
					fprintf(stderr,MSGSTR(EUSAGE,"usage: cat [-urSsq] [-n[b]] [-v[te]] [-|file] ...\n"));
				exit(2);
                }

        	break;
        }

        if (fstat((int)fileno(stdout), &statb) < 0) {
                if (!silent)
                        fprintf(stderr, MSGSTR(ESTATOUT,"cat: cannot stat stdout\n"));
                exit(2);
        }

	/* if we are in C or POSIX locales, we can make various	*/
	/* assumptions to speed things up.			*/
        loc_val =  setlocale(LC_CTYPE,NULL);
	if ( strcmp(loc_val,"C")==0 || ((*loc_val=='P') && (strcmp(loc_val,"POSIX")==0)) )
		asciipath=1;
	else
		asciipath=0;	
	
        statb.st_mode &= S_IFMT;
        if (statb.st_mode != S_IFCHR && statb.st_mode != S_IFBLK) {
                dev = statb.st_dev;
                ino = statb.st_ino;
        }
	if (optind == argc) {
		argc++;
		stdinflg++;
        }
	for (argv = &argv[optind];
	     optind < argc && !ferror(stdout); optind++, argv++) {
		if (stdinflg || (*argv)[0] == '-' && (*argv)[1] == '\0')
                        fi = stdin;
                else {
                        if ((fi = fopen(*argv, "r")) == NULL) {
                                if (!silent)
					fprintf(stderr,MSGSTR(EOPEN,"cat: cannot open %s\n"),*argv);
                                status = 2;
                                continue;
                        }
                }
                if (fstat((int)fileno(fi), &statb) < 0) {
                        if (!silent)
                                fprintf(stderr,MSGSTR(ESTAT,"cat: cannot stat %s\n"),*argv);
                        status = 2;
                        continue;
                }

		if (vflag||nflag||sflag) {
			if (asciipath)
				copyopt_ascii(fi);
			else
				if (MB_CUR_MAX==1)
					copyopt(fi);
				else
					copyopt_mb(fi);
		}
		else 
		if (uflag) {
			while ((c = getc(fi)) != EOF)
                                if (putchar(c) == EOF && ferror(stdout)) {
                                        if (!silent) {
                                                fprintf(stderr,MSGSTR(EOUTPUT,"cat: output error\n"));
                                                perror("");
                                        }
                                        status = 2;
                                        break;
                                }
                }
		else
		{
			int cnt, tmpI;
			while ((cnt = read(fileno(fi), buffer, BUFSIZE)) > 0) {
				if ( cnt != (tmpI = write(fileno(stdout), buffer, cnt)) ) {
					if (!silent) {
						fprintf(stderr,MSGSTR(EOUTPUT,"cat: output error\n"));
						perror("");
					}
					status = 2;
					break;
				}
			}
			if (cnt<0) {
				fputs("cat: ",stderr);
				perror(*argv);
				status = 2;
			} 
		}

                if (fi != stdin) {
                        fflush(stdout);
			if ((fclose(fi) != 0) && (!silent))
				fprintf(stderr,MSGSTR(ECLOSE,"cat: close error\n"));
		}
        }

        fflush(stdout);
        if (ferror(stdout)) {
                if (!silent)
                        fprintf(stderr,MSGSTR(EOUTPUT,"cat: output error\n"));
                status = 2;
        }

        /* Defect 44200 */
        errno = 0;
        fclose(stdout);
        if (errno == ENOSPC || errno == EDQUOT)
        {
                perror("cat");
                status = 2;
        }
        exit(status);
}


/*
 * NAME: cat
 *
 * FUNCTION:
 * 	Depending on the option passed, main() switches between the
 * 	standard cat (scat) and the faster version (fcat). 
 *
 * RETURN:
 *	fcat and scat do not return.
 *                                                                    
 */  
main(argc, argv)
int    argc;
char **argv;
{
        int i;

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_CAT, NL_CAT_LOCALE);

        for (i = 1; i < argc ; i++)
        	if ((argv[i][0] == '-') && (argv[i][1] == '\0')) {
               		i = -1;
                	break;
            	}
        if ((i != -1) && ((argc >= 2) && (argv[1][0] != '-')) ||
           ((argc > 2) && (argv[1][1] == 'q') && (argv[1][2] == '\0') &&
           (argv[2][0] != '-')))
        	fcat(argc, argv);
        else 
            	scat(argc, argv);

	exit(3);  /* should never get here */
}

/*
 * NAME: copyopt
 *                                                                    
 * FUNCTION: 
 *	Read file f, character by character and copy it to standard output.
 *	Translate special characters and number lines as specified by
 *	flags:
 *		-n	number lines.
 *		-b	don't count spaces.
 *		-s	squeeze multiple blank lines together.
 *		-q	don't print out errors.
 *		-t	print tabs as ^I.
 *		-v	print all control characters.
 *		-e	signify endofline with a $
 *                                                                    
 * RETURN VALUE DESCRIPTION: 
 *		Subroutine, nothing returned.
 *
 * modifications to copyopt() must also be made to copyopt_ascii()
 * and vis versa.
 */  

static void
copyopt(f)
	register FILE *f;
{
	static lno = 1;
	register int c;

      while ((c=getc(f)) != EOF)
      {
	if (c == '\n') {
		if (inline == 0) {
			if (sflag && spaced)
				continue;
			spaced = 1;
		}
		if (nflag && bflag==0 && inline == 0)
			printf("%6d\t", lno++);
		if (eflag)
			putchar('$');
		putchar('\n');
		inline = 0;
		continue;
	}
	if (nflag && inline == 0)
		printf("%6d\t", lno++);
	inline = 1;
	if (vflag) {
		if (isprint(c))
			putchar(c);
		else if (iscntrl(c)) {
			if (c == '\t' && tflag == 0)
				putchar(c);
			else
				printf("^%c", c == '\177'
				      	? '?' : c | 0100);
		}
		else if (iscntrl(c = toascii(c)))
			printf("M-^%c", c == '\177'
					? '?' : c | 0100);
		else
			printf("M-%c", c);
	} else
		putchar(c);
	spaced = 0;
      }
}

static void
copyopt_mb(f)
	register FILE *f;
{
	static lno = 1;
	wint_t c;

      while ((c=getwc(f)) != WEOF)
      {
	if (c == L'\n') {
		if (inline == 0) {
			if (sflag && spaced)
				continue;
			spaced = 1;
		}
		if (nflag && bflag==0 && inline == 0)
			printf("%6d\t", lno++);
		if (eflag)
			putchar('$');
		putchar('\n');
		inline = 0;
		continue;
	}
	if (nflag && inline == 0)
		printf("%6d\t", lno++);
	inline = 1;
	if (vflag) {
		if (iswprint(c))
			putwchar(c);
		else if (iswcntrl(c)) {
			if (c == L'\t' && tflag == 0)
				putwchar(c);
			else
				printf("^%C", c == L'\177'
				      	? '?' : c | 0100);
		}
		else if (iswcntrl(c = (wchar_t)toascii(c)))
			printf("M-^%C", c == '\177'
					? '?' : c | 0100);
		else
			printf("M-%C", c);
	} else
		putwchar(c);
	spaced = 0;
      }
}


static void
copyopt_ascii(f)
	register FILE *f;
{
	static lno = 1;
	register int c;

      while ((c=getc(f)) != EOF)
      {
	if (c == '\n') {
		if (inline == 0) {
			if (sflag && spaced)
				continue;
			spaced = 1;
		}
		if (nflag && bflag==0 && inline == 0)
			printf("%6d\t", lno++);
		if (eflag)
			putchar('$');
		putchar('\n');
		inline = 0;
		continue;
	}
	if (nflag && inline == 0)
		printf("%6d\t", lno++);
	inline = 1;
	if (vflag) {
		if (c < ' ') {
			if (c == '\t' && tflag == 0)
				putchar(c);
			else
				printf("^%c", c | 0100);
			}
		else if (c < 0177)
			putchar(c);
		else if (c > 0177) {
			fputs("M-",stdout);
			c &= 0177;
			if (c < ' ')
				printf("^%c", c | 0100);
			else if (c == 0177)
				fputs("^?",stdout);
			else
				putchar(c);
			}
		else if (c == 0177)
			fputs("^?",stdout);
	} else
		putchar(c);
	spaced = 0;
      }
}

