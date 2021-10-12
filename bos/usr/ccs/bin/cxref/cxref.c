static char     sccsid[] = "@(#)48      1.16.1.12  src/bos/usr/ccs/bin/cxref/cxref.c, cmdprog, bos411, 9428A410j 5/11/94 15:20:58";
/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: main, callsys, dexit, doxasref, doxref, getnumb, m4file, mktmpfl,
 *            prntflnm, prntfnc, prntlino, prntwrd, prtsort, quitmsg,
 *            quitperror, scanline, setsig, sigout, sortfile, tmpscan, tunlink
 *
 * ORIGINS: 3 10 27 32 65 71
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.1
 */
/*static char rcsid[] = "RCSfile: cxref.c,v Revision: 1.5  (OSF) Date: 90/10/07 17:39:35 ";*/

/* 
 * Modified: July 91: would sometimes truncate sjis/ujis file names (P27179).
 */

#define _ILS_MACROS
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <nl_types.h>
#include <mbstr.h>
#include <sys/param.h>
#include <string.h>
#include <signal.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/localedef.h>
#include <ctype.h>

#if 0
# include <nl_types.h>
# include <ctype.h>
#endif

#include "cxref_msg.h"
#define MSGSTR(Num, Str) catgets(catd, MS_CXREF, Num, Str)
nl_catd catd;

#define EXITFATAL 0100  /* Fatal error occurred */

#define MAXWORD (BUFSIZ / 2)
#define MAXRFW MAXWORD
#define MAXRFL MAXWORD

#define NUMFILES 200
#define MAXARGS 520
#define MAXDEFS 500             /* maximum number of preprocessor arguments */
#define BASIZ   45              /* funny */
#define TTYOUT  35              /* numbers */
#define YES     1
#define NO      0
#define LETTER  'a'
#define DIGIT   '0'
#define CPPTYPE 1
#define CTYPE 0

#define prntwrd(word) printf("\n%-13s ", word)
#define prntflnm(fname) printf("%-20s ", fname)
#define prntfnc(fnc) printf("%-10s ", fnc)

#define USAGE MSGSTR(M_MSG_19, "Usage: cxref [-cst] [-o File] [-w [Number]]\n\t[[-D Name[=Definition]] [-U Name] [I Directory] [-qOption]] ...\n\t[-NdNumber] [-NlNumber] [-NnNumber] [-NtNumber] File ...\n")

extern char     *mktemp();

char    *cfile = 0;             /* current file name */
char    *tmp1;                  /* contains preprocessor result */
char    *tmp2;                  /* contains cpp and xpass cross-reference */
char    *tmp3;                  /* contains transformed tmp2 */
char    *tmp4;                  /* contains sorted output */

char    *clfunc = "--";         /* clears function name for ext. variables */

char    xflnm[MAXWORD];         /* saves current filename */
char    funcname[MAXWORD];      /* saves current function name */
char    sv1[MAXRFL];            /* save areas used for output printing */
char    sv2[MAXRFL];
char    sv3[MAXRFL];
char    sv4[MAXRFL];
char    buf[BUFSIZ];

/* ----------- these paths are to be checked for accuracy---------- */
/* ----------- and some final path for these utilities should be */
/* ------------ decided and if neccessary changed to reflect the */
/* ------------ correct path before installing cxref --------------*/

#ifdef  DEBUG
static char     xcpp[] = "./cpp";
static char     xref[] = "./xpass";
static char     sort[] = "./sort";
static char     xlC[] = "./xlC";
#else
static char     xcpp[] = "/lib/cpp";
static char     xref[] = "/usr/lib/xpass";
static char     sort[] = "/bin/sort";
static char     xlC[] = "/usr/bin/xlC";
#endif  DEBUG

char    *arv[MAXARGS], *predefs[MAXDEFS];

int     cflg = NO;              /* prints all given files as one cxref listing */
int     silent = NO;            /* print filename before cxref? */
int     tmpmade = NO;           /* indicates if temp file has been made */
int     inafunc = NO;           /* in a function? */
int     fprnt = YES;            /* first printed line? */
int     addlino = NO;           /* add line number to line?? */
int     ddbgflg = NO;           /* debugging? */
int     idbgflg = NO;
int     bdbgflg = NO;
int     tdbgflg = NO;
int     edbgflg = NO;
int     xdbgflg = NO;
int     Nflag = NO;
int     LSIZE = TTYOUT;         /* default size */
int     lsize = 0;              /* scanline: keeps track of line length */
int     sblk = 0;               /* saves first block number of each new function */
int     fsp = 0;                /* format determined by prntflnm */
int     defs = 0;               /* number of -I, -D and -U flags */
int     defslen = 0;            /* length of -I, -D and -U flags */
char    *Nstr;
int     operandflg = NO;

int     nword;                  /* prntlino: new word? */
int     nflnm;                  /* new filename? */
int     nfnc;                   /* new function name? */
int     nlino;                  /* new line number? */

int     callsys(char f[], char *v[], int redirect);
void    dexit(void);
int     doxref(int);
int     getnumb(char *);
void    mktmpfl(void);
void    prntlino (char *ns);
void    prtsort(void);
void    quitmsg(const char *message, ...);
void    quitperror(char *str);
void    scanline(char *line);
void    setsig(void);
void    sortfile(void);
void    tmpscan(char s[], char ns[], FILE *tfp);
void    tunlink(void);

struct filestr {
        char *name;
        int  type;
};

int
main(int argc, char *argv[])
{

        int i, j;
        int errflag = 0;
        int numfiles = NUMFILES;
        struct filestr *files = NULL;
        int filepos = 0;

        setlocale(LC_ALL, "");
        catd = catopen(MF_CXREF, NL_CAT_LOCALE);

        if (argc == 1) {
                fprintf(stderr, USAGE);
                exit(1);
        }

        files = (struct filestr *)calloc(sizeof(struct filestr), numfiles);
        if (!files)
                quitmsg(MSGSTR(M_MSG_23, "There is not enough memory available.\n"));
        
        mktmpfl();      /* make temp files */

        while (--argc > 0) {
                ++argv;
                sblk = 0;
                if (!strcmp(*argv, "--")) {
                        operandflg = YES;
                        continue;
                }
                else if ((**argv == '-') && (!operandflg)){
                    while (* ++ *argv) {
                        switch (**argv) {
                                /* prints cross reference for all files combined */
                        case 'c':
                                cflg = YES;
                                continue;

                        case 'd':
                                ddbgflg = YES;
                                continue;

                        case 'i':
                                idbgflg = YES;
                                continue;

                        case 'b':
                                bdbgflg = YES;
                                continue;

                        case 'T':
                                tdbgflg = YES;
                                continue;

                        case 'e':
                                edbgflg = YES;
                                continue;

                        case 'x':
                                xdbgflg = YES;
                                continue;

                        case 'D':
                        case 'U':
                        case 'I':
                        case 'q':
                                if (defs >= MAXDEFS) {
                                        quitmsg(MSGSTR(M_MSG_21, "There cannot be more than %d preprocessor options.\n"), MAXDEFS);
                                        /*NOTREACHED*/
                                }
                                defslen += strlen(predefs[defs++] = -- * argv);
                                /* space between flag and arg */
                                if (strlen(*argv) == 2) {
                                        if (argc == 1)
                                        {
                                                fprintf(stderr, USAGE);
                                                exit(1);
                                        }
                                        defslen += strlen(predefs[defs++] = *++argv);
                                        --argc;
                                }
                                break;

                                /* length option when printing on terminal */
                        case 't':
                        case 'w':
                                /* LSIZE = # of columns left for line numbers */
                                /* BASIZ = # of columns up to start of line numbers -1 */
                                /* LSIZE + BASIZ must be >= 51 */
                                if (strlen(*argv) > 1)
                                        ++ * argv;
                                else {
                                        if (argc == 1)
                                        {
                                                fprintf(stderr, USAGE);
                                                exit(1);
                                        }
                                        argv++;
                                        argc--;
                                }
                                LSIZE = 0;
                                {
                                        int c;
                                        while ((c = **argv) >= '0' && c <= '9')
                                        {
                                                LSIZE = LSIZE * 10 + c - '0';
                                                ++ * argv;
                                        }
                                        if (c != '\0')
                                        {
                                                fprintf(stderr, USAGE);
                                                exit(1);
                                        }
                                }
                                LSIZE = LSIZE - BASIZ;
                                if (LSIZE <= 5) {
                                        fprintf(stderr, MSGSTR(M_MSG_25, "Warning, invalid argument for -w option, 80 assumed.\n"));
                                        LSIZE = TTYOUT;
                                }
                                break;

                        case 's':
                                silent = YES;
                                continue;

                                /* output file */
                        case 'o':
                                /* May 14/90 - GR: cxref now accepts -o filename
                                  as well as -ofilename configuration */
                                if (strlen(-- * argv) > 2) {
                                        if (freopen((*argv) + 2, "w", stdout) == NULL) {
                                                quitperror((*argv) + 2);
                                                /*NOTREACHED*/
                                        }
                                } else {
                                        if (freopen(*++argv, "w", stdout) == NULL) {
                                                quitperror(*argv);
                                                /*NOTREACHED*/
                                        }
                                        argc--;
                                }
                                break;

                        case 'N':
                                if (Nflag) {
                                        /* Allocate enough space for
                                         * original string, plus new
                                         * string, and Null the new
                                         * string won't include the N
                                         */
                                        Nstr = (char *) realloc(Nstr, (strlen(Nstr) + strlen(*argv)) * sizeof(char));
                                        strcat(Nstr, ++ * argv);
                                } else {
                                        Nflag = YES;
                                        /* Allocate enough space for Null and - */
                                        Nstr = (char *) malloc( (2 + strlen(*argv)) * sizeof(char));
                                        strcpy(Nstr, "-");
                                        strcat(Nstr, *argv);
                                }
                                break;

                        default:
                                fprintf(stderr, MSGSTR(M_MSG_22, "%s is not a recognized flag\n" ), *argv);
                                fprintf(stderr, USAGE);
                                exit(1);
                                break;
                        }
                        break;
                    }
                    continue;
                } else if (strcmp(*argv + strlen(*argv) - 2, ".C") == 0) {
                        /* C++ file */
                        files[filepos].name = *argv;
                        files[filepos].type = CPPTYPE;
                        filepos += 1;
                } else if (strcmp(*argv + strlen(*argv) - 2, ".c") == 0) {
                        files[filepos].name = *argv;
                        files[filepos].type = CTYPE;
                        filepos += 1;
                } else {
                        fprintf(stderr, MSGSTR(M_MSG_20,"File %s must have a .c or .C extension.\n"), *argv);
                        errflag = 1;
                        continue;
                }
                if (filepos >= numfiles)
                {
                        numfiles += NUMFILES;
                        files = (struct filestr *)realloc(files, sizeof(struct filestr) * numfiles);
                        if (!files)
                                quitmsg(MSGSTR(M_MSG_23, "There is not enough memory available.\n"));
                }
        }


        if (cflg == NO) {
                for (j = 0; j<filepos; j++)
                {
                        cfile = files[j].name;
                        if (silent == NO)
                                printf("%s:\n\n", cfile);
                        if(doxref(files[j].type))
                        {
                                errflag=1;
                                continue;
                        }
                        sortfile();     /* sorts temp file */
                        prtsort();      /* print sorted temp file when -c option is not used */
                        tunlink();      /* forget everything */
                        mktmpfl();      /* and start over */
                }
        }
        else {
                int dosort = filepos;
                for (j = 0; j<filepos; j++)
                {
                        cfile = files[j].name;
                        if (silent == NO)
                                printf("%s:\n\n", cfile);
                        if(doxref(files[j].type))
                        {
                                errflag=1;
                                dosort--;
                                continue;
                        }
                }
                if(dosort)
                {
                        sortfile();     /* sorts temp file */
                        prtsort();      /* print sorted temp file when -c option is used */
                }
        };
        tunlink();       /* unlink temp files */
        if (!filepos)
        {
                fprintf(stderr, USAGE);
                exit(1);
        }
        return(errflag);
}


/*
 * This routine calls the program "xpass" which parses
 * the input files, breaking the file down into
 * a list with the variables, their definitions and references,
 * the beginning and ending function block numbers,
 * and the names of the functions---The output is stored on a
 * temporary file and then read.  The file is then scanned
 * by tmpscan(), which handles the processing of the variables
 *
 * For C++ files, xlC -qcxref:tempfile is invoked.
 */

int
doxref(int isCplusplus)
{

        register int    i, n;
        FILE * fp, *tfp;
        char    cccmd[MAXPATHLEN];
        char    line[BUFSIZ], s1[MAXRFW], s2[MAXRFW];

        /* set up cc command line to do preprocessing */
        n = 0;
        if (isCplusplus) {
            /* ensure that the xlC command is present */
            struct stat xlCstat;

            if (stat (xlC, &xlCstat) != 0) {
                quitmsg(MSGSTR(M_MSG_18,
                    "The %s command is not available to process C++ files.\n"),
                        xlC);
                /*NOTREACHED*/
            }

            arv[n++] = xlC;
            arv[n++] = "-c";
            arv[n++] = "-U__STR__";
            arv[n++] = "-U__MATH__";
            arv[n++] = strcat(strcpy(cccmd, "-qcxref:"), tmp2);

            for (i = 0; i < defs; i++)
                    arv[n++] = predefs[i];
            if (operandflg == YES)
                    arv[n++] = "--";
            arv[n++] = cfile;                   /* infile */
            arv[n] = 0;

            if (callsys(xlC, arv, NO) > 1) {
                    fprintf(stderr, MSGSTR(M_MSG_17, "The command %s failed on %s.\n"), xlC, cfile);
                    return(1);
                    /*NOTREACHED*/
            }
        } else {
            arv[n++] = xcpp;
            arv[n++] = "-E";
            arv[n++] = "-U__STR__";
            arv[n++] = "-U__MATH__";
            arv[n++] = "-D_IBMR2";
            arv[n++] = "-D_AIX";
            arv[n++] = "-D_POWER";

            arv[n++] = strcat(strcpy(cccmd, "-W"), tmp2);

            for (i = 0; i < defs; i++)
                    arv[n++] = predefs[i];
            if (operandflg == YES)
                    arv[n++] = "--";
            arv[n++] = cfile;                   /* infile */
            /*arv[n++] = tmp1;           outfile */
            arv[n] = 0;

            if ( callsys(xcpp, arv, YES) > 0) {
                    fprintf(stderr, MSGSTR(M_MSG_6, "cpp failed on %s!\n"), cfile);
                    return(1);
                    /*NOTREACHED*/
            }

            i = 0;
            arv[i++] = "xpass";
            if (ddbgflg)
                    arv[i++] = "-d";
            if (idbgflg)
                    arv[i++] = "-I";
            if (bdbgflg)
                    arv[i++] = "-b";
            if (tdbgflg)
                    arv[i++] = "-t";
            if (edbgflg)
                    arv[i++] = "-e";
            if (xdbgflg)
                    arv[i++] = "-x";
            if (Nflag)
                    arv[i++] = Nstr;
            arv[i++] = tmp1;
            arv[i++] = tmp2;
            arv[i] = 0;

            if ( callsys(xref, arv, NO) > 0) {
                    fprintf(stderr, MSGSTR(M_MSG_7, "xpass failed on %s!\n"), cfile);
                    return(1);
                    /*NOTREACHED*/
            }
        }

        /* open temp file produced by "xpass" for reading */
        if ((fp = fopen(tmp2, "r")) == NULL) {
                quitperror(tmp2);
                /*NOTREACHED*/
        }

        setbuf(fp, buf);
        /*
         * open a new temp file for writing
         * the output from tmpscan()
         */
        if ((tfp = fopen(tmp3, "a")) == NULL) {
                quitperror(tmp3);
                /*NOTREACHED*/
        }

        /*
         * read a line from tempfile 2,
         * break it down and scan the
         * line in tmpscan()
         */
        while (fgets(line, BUFSIZ, fp) != NULL) {
                if (line[0] == '"') {
                        char    *p = line;
                        strcpy(xflnm, p + 1);           /* remove first quote */
                        p = mbsrchr (xflnm, '"');       /* remove last quote */
                        if (p)
                                *p = '\0';
                        continue;
                }
                sscanf(line, "%[^\t]\t%s", s1, s2);
                tmpscan(s1, s2, tfp);
        }
        fclose(tfp);
        fclose(fp);
        return(0);
}


/*
 * general purpose routine which does a fork
 * and executes what was passed to it--
 */

int
callsys(char f[], char *v[], int redirect)
{
        register int    pid, w;
        int     status;

#ifdef  DEBUG
        int     i;

        fprintf (stderr, "Executing command: %s ", f);
        for (i = 1; v[i]; i++) {
                fprintf (stderr, "%s ", v[i]);
        }
        fprintf (stderr, "\n");
#endif  DEBUG

        /* flag to redirect stdout to tmp1 */
        if (redirect)
                fflush(stdout);

        if ((pid = fork()) == 0) {
                /* only the child gets here */
                if (redirect) {
                        if (freopen(tmp1, "w", stdout) == NULL) {
                                quitperror(tmp1);
                                /*NOTREACHED*/
                        }
                }
                execvp(f, v);
                quitperror(f);
                /*NOTREACHED*/

        } else if (pid == -1) {
                /* fork failed - tell user */
                quitperror(MSGSTR(M_MSG_8, "cxref"));
                /*NOTREACHED*/
        }

        /*
         * loop while calling "wait" to wait for the child.
         * "wait" returns the pid of the child when it returns or
         * -1 if the child can not be found.
         */
        while (((w = wait(&status)) != pid) && (w != -1))
                ;

        if (w == -1) {
                /* child was lost - inform user */
                quitperror(f);
                /*NOTREACHED*/
        }

        /* check system return status */
        if (((w = status & 0x7f) != 0) && (w != SIGALRM)) {
                /* don't complain if the user interrupted us */
                if (w != SIGINT) {
                        fprintf(stderr, MSGSTR(M_MSG_15, "Fatal error in %s"), f);
                        perror(" ");
                };
                /* remove temporary files */
                dexit();
                /*NOTREACHED*/
        };

        /* return child status only */
        return((status >> 8) & 0xff);
}


/*
 * general purpose routine which returns
 * the numerical value found in a string
 */

int
getnumb(char *ap)
{
        register int    n, c;

        n = 0;
        while ((c = *ap++) >= '0' && c <= '9')
                n = n * 10 + c - '0';

        return(n);
}


/* this routine parses the output of "xpass"*/
void
tmpscan(char s[], char ns[], FILE *tfp)
{

        /*
         * D--variable defined;
         * R--variable referenced;
         * F--function name;
         * B--block(function ) begins;
         * E--block(function) ends
         */

        register int    lino = 0;
        char    star;

        /*
         * look for definitions and references of external variables;
         * look for function names and beginning block numbers
         */

        if (inafunc == NO) {
                switch (*s++) {
                case 'D':
                        star = '*';
                        goto ahead1;

                case 'R':
                        star = ' ';
ahead1:
                        lino = getnumb(ns);
                        fprintf(tfp, "%s\t%s\t%s\t%5d\t%c\n", s, xflnm, clfunc, lino, star);
                        break;

                case 'F':
                        strcpy(funcname, s);
                        star = '$';
                        fprintf(tfp, "%s\t%c\n", s, star);
                        break;

                case 'B':
                        inafunc = YES;
                        sblk = getnumb(s);
                        break;

                default:
                        quitmsg(MSGSTR(M_MSG_9, "SWITCH ERROR IN TMPSCAN: inafunc = no\n" ));
                        /*NOTREACHED*/
                }
        } else {
                /*
                 * in a function:  handles local variables
                 * and looks for the end of the function
                 */

                switch (*s++) {
                case 'R':
                        star = ' ';
                        goto ahead2;
                        /* No Break Needed */

                case 'D':
                        star = '*';
ahead2:
                        lino = getnumb(ns);
                        fprintf(tfp, "%s\t%s\t%s\t%5d\t%c\n", s, xflnm, funcname, lino, star);
                        break;

                case 'B':
                        break;

                case 'E':
                        lino = getnumb(s);
                        /*
                         * lino is used to hold the ending block
                         * number at this point
                         *
                         * if the ending block number is the
                         * same as the beginning block number
                         * of the function, indicate that the
                         * next variable found will be external.
                         */

                        if (sblk == lino) {
                                inafunc = NO;
                        }
                        break;

                case 'F':
                        star = '$';
                        fprintf(tfp, "%s\t%c\n", s, star);
                        break;

                default:
                        quitmsg(MSGSTR(M_MSG_10, "SWITCH ERROR IN TMPSCAN: inafunc = yes\n"));
                        /*NOTREACHED*/
                };
        };
}


void
mktmpfl(void)
{
#ifdef  DEBUG
        tmp1 = "./XR1";
        tmp2 = "./XR2";
        tmp3 = "./XR3";
        tmp4 = "./XR4";
#else
        static char     str1[] = "/usr/tmp/xr1XXXXXX";
        static char     str2[] = "/usr/tmp/xr2XXXXXX";
        static char     str3[] = "/usr/tmp/xr3XXXXXX";
        static char     str4[] = "/usr/tmp/xr4XXXXXX";

        /* make temporary files */

        tmp1 = mktemp(str1);
        tmp2 = mktemp(str2);            /* output of "xpass" */
        tmp3 = mktemp(str3);            /* holds output of tmpscan() routine */
        tmp4 = mktemp(str4);            /* holds output of tempfile 3*/
        tmpmade = YES;                  /* indicates temporary files have been made */
        setsig();
#endif  DEBUG

        /* C++ driver uses -q option, which will lower case all the option */
        /* make tmp2 lower case now, to avoid problems later */
        {
            char *p = tmp2;
            while (*p)
            {
                if (isupper(*p))
                    *p = tolower(*p);
                p++;
            }
        }
}


/* unlink temporary files */
void
tunlink(void)
{
        if (tmpmade == YES) {   /* if tempfiles exist */
                unlink(tmp1);
                unlink(tmp2);
                unlink(tmp3);
                unlink(tmp4);
        };
}


/* remove temporary files and exit with error status */
void
dexit(void)
{
        tunlink();
        exit(EXITFATAL);
        /*NOTREACHED*/
}


void
quitperror(char *str)
{
        perror(str);
        dexit();
        /*NOTREACHED*/
}


void
quitmsg(const char *fmt, ...)
{
        va_list args;

        va_start(args, fmt);
        vfprintf (stderr, fmt, args);
        va_end(args);

        dexit();
        /*NOTREACHED*/
}


/* set up check on signals */
void
setsig(void)
{
        void    sigout(void);

        if (isatty(1)) {
                if (signal(SIGHUP, SIG_IGN) == SIG_DFL)
                        signal(SIGHUP, (void (*)(int))sigout);
                if (signal(SIGINT, SIG_IGN) == SIG_DFL)
                        signal(SIGINT, (void (*)(int))sigout);
        } else {
                signal(SIGHUP, SIG_IGN);
                signal(SIGINT, SIG_IGN);
        };

        signal(SIGQUIT, (void (*)(int))sigout);
        signal(SIGTERM, (void (*)(int))sigout);
}


/* signal caught; unlink tmp files */
void
sigout(void)
{
        tunlink();
        signal(SIGHUP, SIG_IGN);
        signal(SIGINT, SIG_IGN);
        signal(SIGTERM, SIG_IGN);
        dexit();
        /*NOTREACHED*/
}


/* sorts temp file 3 --- stores on 4 */
void
sortfile(void)
{
        register int    status;

        arv[0] = sort;
        arv[1] = "-t\t";
        arv[2] = "-o";
        arv[3] = tmp4;
        arv[4] = tmp3;
        arv[5] = 0;
        /* execute sort */

        if ((status = callsys(sort, arv, NO)) > 0) {
                quitmsg(MSGSTR(M_MSG_11, "Sort failed with status %d\n"), status);
                /*NOTREACHED*/
        }
}


/* prints sorted files and formats output for cxref listing */
void
prtsort(void)
{
        FILE * fp;
        char    line[BUFSIZ];

        /* open tempfile of sorted data */
        if ((fp = fopen(tmp4, "r")) == NULL) {
                quitmsg(MSGSTR(M_MSG_12, "CAN'T OPEN %s\n"), tmp4);
                /*NOTREACHED*/
        }

        if (silent == YES) {    /* assume called by "oprl" */
                fprintf(stdout, MSGSTR(M_MSG_24, "SYMBOL        FILE                 FUNCTION   LINE\n"));
        }

        while (fgets(line, BUFSIZ, fp) != NULL) {
                scanline(line); /* routine to format output */
        }

        fprnt = YES;    /* reinitialize for next file */
        fclose(fp);
        putc('\n', stdout);
}

char *to_tab (char *ptr, char *buffer)
{
    for (;;) {
	switch (*ptr) {
	    case '\0':
		*buffer = '\0';
		return ptr;
	    
	    case '\t':
	    case '\n':
		*buffer = '\0';
		return ptr+1;
	    
	    default:
		*buffer++ = *ptr++;
		break;
	}
    }
}


char *skip_blanks (char *ptr)
{
    for (;;) {
	switch (*ptr) {
	    case ' ':
		ptr++;
		break;
	    
	    default:
		return ptr;
		break;
	}
    }
}


/* formats what is to be printed on the output */
void
scanline(char *line)
{
        register char   *sptr1;
        register int    och, nch;
        char    s1[MAXRFL], s2[MAXRFL], s3[MAXRFL], s4[MAXRFL], s5[MAXRFL];

        /*
         * break down line into variable name, filename,
         * function name, and line number
         */
	sptr1 = to_tab(line, s1);
	sptr1 = to_tab(sptr1, s2);
	sptr1 = to_tab(sptr1, s3);
	sptr1 = skip_blanks(sptr1);
	sptr1 = to_tab(sptr1, s4);
	(void) to_tab(sptr1, s5);

        /* function name */
        if (strcmp(s2, "$") == 0) {
                if (strcmp(sv1, s1) != 0) {
                        strcpy(sv1, s1);
                        if (strchr (s1, '('))
                            printf("\n%s", s1);   /* already has '()' */
                        else
                            printf("\n%s()", s1);   /* append '()' to name */
                        *sv2 = *sv3 = *sv4 = '\0';
                        fprnt = NO;
                };
                return;
        }

        /* variable defined at this line number */
        if (strcmp(s5, "*") == 0) {
                *s5 = '\0';
                sptr1 = s4;
                och = '*';
                /* prepend a star '*' */
                for ( nch = *sptr1; *sptr1 = och; nch = *++sptr1)
                        och = nch;
        }

        /* if first line--copy the line to a save area */
        if (fprnt == YES) {
                prntwrd( strcpy(sv1, s1) );
                prntflnm( strcpy(sv2, s2) );
                prntfnc( strcpy(sv3, s3) );
                prntlino( strcpy(sv4, s4) );
                fprnt = NO;
                return;
        } else {
                /*
                 * this part checks to see what variables have changed
                 */
                if (strcmp(sv1, s1) != 0) {
                        nword = nflnm = nfnc = nlino = YES;
                } else {
                        nword = NO;
                        if (strcmp(sv2, s2) != 0) {
                                nflnm = nfnc = nlino = YES;
                        } else {
                                nflnm = NO;
                                if (strcmp(sv3, s3) != 0) {
                                        nfnc = nlino = YES;
                                } else {
                                        nfnc = NO;
                                        nlino = (strcmp(sv4, s4) != 0) ? YES : NO;
                                        if (nlino == YES) {
                                                /*
                                                 * everything is the same
                                                 * except line number
                                                 * add new line number
                                                 */
                                                addlino = YES;
                                                prntlino( strcpy(sv4, s4) );
                                        };
                                        /*
                                         * Want to return if we get to
                                         * this point. Case 1: nlino
                                         * is NO, then entire line is
                                         * same as previous one.
                                         * Case 2: only line number is
                                         * different, add new line number
                                         */
                                        return;
                                }
                        }
                }
        }

        /*
         * either the word, filename or function name
         * are different; this part of the routine handles
         * what has changed...
         */

        addlino = NO;
        lsize = 0;
        if (nword == YES) {
                /* word different--print line */
                prntwrd( strcpy(sv1, s1) );
                prntflnm( strcpy(sv2, s2) );
                prntfnc( strcpy(sv3, s3) );
                prntlino( strcpy(sv4, s4) );
                return;
        } else {
                printf("\n%-14s", "");
                if (nflnm == YES) {
                        /*
                         * different filename---new name,
                         * function name and line number are
                         * printed and saved
                         */
                        prntflnm( strcpy(sv2, s2) );
                        prntfnc( strcpy(sv3, s3) );
                        prntlino( strcpy(sv4, s4) );
                        return;
                } else {
                        prntflnm(s2);
                        if (nfnc == YES) {
                                /*
                                 * different function name---new name
                                 * is printed with line number;
                                 * name and line are saved
                                 */
                                prntfnc( strcpy(sv3, s3) );
                                prntlino( strcpy(sv4, s4) );
                        }
                }
        }
}

/* formats line numbers */
void
prntlino(char *ns)
{
        register int    lino, i;
        char    star;

        i = lino = 0;

        if (*ns == '*') {
                star = '*';
                ns++;   /* get past star */
        } else {
                star = ' ';
        }

        lino = getnumb(ns);
        if (lino < 10)  /* keeps track of line width */
                lsize += (i = 3);
        else if ((lino >= 10) && (lino < 100))
                lsize += (i = 4);
        else if ((lino >= 100) && (lino < 1000))
                lsize += (i = 5);
        else if ((lino >= 1000) && (lino < 10000))
                lsize += (i = 6);
        else /* lino > 10000 */
                lsize += (i = 7);

        if (addlino == YES) {
                if (lsize <= LSIZE) {
                        /* line length not exceeded--print line number */
                        fprintf(stdout, " %c%d", star, lino);
                } else {
                        /* line to long---format lines overflow */
                        fprintf(stdout, "\n%-46s%c%d", "", star, lino);
                        lsize = i;
                }
                addlino = NO;
        } else {
                fprintf(stdout, "%c%d", star, lino);
        }
}


