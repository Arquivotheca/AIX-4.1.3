static char sccsid[] = "@(#)97	1.17  src/bos/usr/bin/tab/tab.c, cmdfiles, bos411, 9428A410j 4/12/94 16:44:53";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: tab, untab
 *
 * ORIGINS: 26, 27
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
#include <locale.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/limits.h>
#include <sys/param.h>
#include "tab_msg.h"

nl_catd catd;
#define MSGSTR(Num, Str) catgets(catd, MS_TAB, Num, Str)

static int savederrno;	/* making the catgets() call might munge errno; */
extern int errno;

static dofile();
static err();
static resig();
typedef char bool;

/****** rename() is now a system call for DS ******
#define rename(old,new) (link(old, new), unlink(old))
****************/

#if IS1
#define _IORW _IOSHOW
#endif

static struct stat statb;

static char    tmpproto[] = "tabaXXXXXX";
static char    cp[PATH_MAX], TmpFile[PATH_MAX + sizeof tmpproto];
static char    exitcode = 0;
static bool    eflag=FALSE, tabbing = TRUE, disable, rmtmp;
static char    gotsig;

/*
 * NAME: tab [-e] [file ...]
 *       untab [file ..]
 *                                                                    
 * FUNCTION: tab-reads a file and replaces spaces with tabs and
 *               then it writes the result back to file
 *               -e  replaces only those spaces at the beginning of a 
 *                   line up to the first nonspace character.
 *           untab-reads a file and replaces tabs with spaces and then
 *                   it writes the result back to file
 *                                                                    
 * NOTES:  If no file is specified then standard in is read and 
 *         the results is written to standard out.
 */  
main (int argc, char *argv[])
{
    int  	c;
    char 	*vp, *cmd;
    int  	n;
    char 	*optstring = "e";
    bool 	didfile = FALSE;

/* AIX security enhancement */
    char	*aclp;		/* pointer for acl_fget() and acl_put() */
    char	*acl_fget();
/* TCSEC DAC mechanism */

#if IS1
    extern _protection;
#endif
    extern sigcatch(int);

	setlocale(LC_ALL, "");
	catd = catopen(MF_TAB, NL_CAT_LOCALE);

    /****for (n = NSIG; --n > 0;) signal(n, sigcatch);** p28104, dewey */
	signal(SIGHUP, (void (*)(int))sigcatch); /* p28104, dewey */
	signal(SIGINT, (void (*)(int))sigcatch); /* p28104, dewey */
	signal(SIGQUIT, (void (*)(int))sigcatch); /* p28104, dewey */
	signal(SIGPIPE, (void (*)(int))sigcatch); /* p28104, dewey */
	signal(SIGTERM, (void (*)(int))sigcatch); /* p28104, dewey */

    if (strstr(argv[0], "untab") != NULL)
    {
	optstring = "";
	tabbing = FALSE;
    }
    else
    {
	tabbing = TRUE;
    }

    while (( c = getopt(argc, argv, optstring)) != -1) {
	switch(c) {
   	case 'e': eflag = TRUE;
		  break;		 
	default:  printf("default\n");  if (tabbing == FALSE) {
			fprintf(stderr,MSGSTR(UNTABUSAGE,"untab [file...]\n"));
			exit(1);
		  } else {
			fprintf(stderr,MSGSTR(TABUSAGE,"tab [-e] [file...]\n"));
			exit(1);
		  }
	}
    }				
	
	while (optind < argc) {
	vp = argv[optind++];
	didfile = TRUE;

	if (strlen(vp) > PATH_MAX) {
	    savederrno = errno;
	    err(vp, MSGSTR( NAMLENERR, "Name too long"));
	    continue;
	}
	strcpy(TmpFile, vp);
	if ((cmd = strrchr(TmpFile, '/')) != NULL)
		cmd++;
	else
		cmd = TmpFile;
	strcpy(cmd, tmpproto);
	strcpy(cp, TmpFile);
	mktemp(cp);

	
	if (freopen(vp, "r", stdin) == NULL) {
	    savederrno = errno;
	
    err(vp, (char *) NULL);
	    continue;
	}

	fstat((int)fileno(stdin), &statb);

/* AIX security enhancement */
	/* get acl information so we can copy it to vp */
	aclp = acl_fget( fileno(stdin) );
/* TCSEC DAC mechanism */

	if ((statb.st_mode & S_IFMT) != S_IFREG) {
	    savederrno = errno;
	    err(vp, MSGSTR( NOTAFILE, "Not a file"));
	    continue;
	}

#if IS1
	_protection = statb.st_mode;
#endif
	rmtmp = TRUE;
	if (freopen(TmpFile, "w", stdout) == NULL) {
	    rmtmp = FALSE;
	    savederrno = errno;
	    err(TmpFile, (char *) NULL);
	    continue;
	}
	dofile();
	fclose(stdout);
	disable = TRUE;
	unlink(vp);
	rename(TmpFile, vp);
#ifndef IS1
/* AIX security enhancement */
	/* inherit acl information (includes the upper and lower mode bits). */
	/* 1==free aclp */
	acl_put(vp, aclp, 1);
/* TCSEC DAC mechanism */
#endif
	rmtmp = FALSE;
	disable = FALSE;
	if (gotsig) resig(gotsig);
    }
    if (!didfile) dofile();         /* filter */

    exit(exitcode);
} /* end main */

/*
 * NAME: err
 *                                                                    
 * FUNCTION: print error messages
 */  
static err (s1, s2) char   *s1, *s2; {
    if (s2)
	fprintf(stderr,"%s: %s\n", s1, s2);
    else {
	errno = savederrno;
	perror(s1);
    }
	++exitcode; 
}

/*
 * NAME: sigcatch
 *                                                                    
 * FUNCTION: on interrupts clean-up temp files
 */  
sigcatch(int sig) {
    signal(sig, (void (*)(int))sigcatch);
    if (!disable) {
	if (rmtmp) {
	    unlink(TmpFile);
	}
	resig(sig);    /* send sig to process again */
    }
    gotsig = sig;
}

/*
 * NAME: resig
 *                                                                    
 * FUNCTION: send sig to current process
 */  
static resig(sig) {
    signal(sig, SIG_DFL);
    kill(getpid(), sig);
}

/*
 * NAME: dofile
 *                                                                    
 * FUNCTION: change spaces to tabs or change tabs to spaces
 */  
static dofile () {
    int    c;
    int    inpos, outpos,outc;
    bool   leading;
    register int multibyte, len;
    int    disp_width;
	
#if IS1
    setbuf(stdout, TRUE);
#endif

    inpos = outpos = outc = 0;
    leading = eflag;
    multibyte = MB_CUR_MAX != 1;

    while (1) {
	if (multibyte) {
	    c = getwchar();
	    if (c == WEOF)
		break;
	} else {
	    c = getchar();
	    if (c == EOF)
		break;
	}
	switch (c) {
	    case '\t':
		if (!eflag) {
		    if (tabbing) {      /* adding tabs */
			putchar(c);
			outpos+=8; 
			outpos&=~07;
			inpos+=8; 
			inpos&=~07;
			outc = 0;
			break;
		    }
		    c = ' ';           /* removing tabs */
		    do {
			putchar(c);
			inpos++;
			outpos++;
		    } while(outpos % 8);
		    break;
		}
		putchar(c);    /* adding tabs at the beginning only */
		outpos+=8; 
		outpos&=~07;
		inpos+=8; 
		inpos&=~07;
		break;
	    case ' ':
		if (!eflag) {
		    if (tabbing) {      /* adding tabs */
			inpos++;
			if (inpos%8 != 0) {
				outc++;
				continue;
			}
			if (outpos < inpos) {
				c = '\t'; 
				outc = 0;
				putchar(c);
				outpos = inpos;
				break;
			}
			break;
		    }
		    else {          /* removing tabs */
			putchar(c);
			inpos++;
			outpos++;
			break;
		    }        /* adding tabs at the beginning only */
		}
		inpos++;
		if (inpos%8 != 0) {
			outc++;
			continue;
		}
		if (outpos < inpos) {
		    c = '\t'; 
		    putchar(c);
		    outc = 0;
		    outpos = inpos;
		    break;
		}
		break;
	    case '\f':
		if (outc > 0) {
		   while (outc-- > 0) putchar(' ');
		   outc = 0;
                }
		putchar(c);
		break;
	    case '\n':
		if (outc > 0) {
		   while (outc-- > 0) putchar(' ');
		   outc = 0;
                }
		putchar(c);
		inpos = outpos = 0;	
		break;
	    default:
		if (outc > 0) {
		   while (outc-- > 0){
			 putchar(' ');
			 outpos++;
		   }
		   outc = 0;
                }
 		if (leading) {
		    if (multibyte)
			putwchar(c);
		    else
			putchar(c);
		    putline();
    		    inpos = outpos = 0;
    		    leading = eflag;
		    break;
		}
		if (multibyte) {
		    putwchar(c);
		    disp_width = (len = wcwidth(c)) == -1 ? 1 : len;
		    inpos += disp_width;
		    outpos += disp_width;
		} else {
		    putchar(c);
		    inpos++; outpos++;
		}
		break;
	}
    } /* end while */
} /* end dofile */

/*
 * NAME: putline
 * FUNCTION: when  -e option is giving and the first nonspace is found 
 *     put out the rest of the line with out any changes
 */
putline()
{
	int c;
	
	while ((c = getchar()) >= 0 && c != '\f' && c != '\n')
        	putchar(c);
	putchar(c);
}


