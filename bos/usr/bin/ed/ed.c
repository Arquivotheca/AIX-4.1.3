static char sccsid[] = "@(#)66	1.62  src/bos/usr/bin/ed/ed.c, cmdedit, bos41J, 9519A_all 5/3/95 16:01:06";
/*
 * COMPONENT_NAME: (CMDEDIT) ed.c
 *
 * FUNCTIONS: main, address, append, blkio, chktime, clear, commands, compsub,
 * crblock, crinit, delete, dosub, eclose, eopen, error, error1, execute,
 * exfile, expnd, filecopy, filename, fspec, gdelete, getblock, getchr,
 * getcopy, getfile, getime, getkey, getline, getsub, gettty, global, globaln,
 * init, join, lenchk, makekey, mkfunny, move, newline, newtime, nonzero, numb,
 * onhup, onintr, onpipe, place, putchr, putd, putfile, putline, puts, quit,
 * rdelete, red, reverse, save, setall, setdot, setnoaddr, stdtab, substitute
 * targ, tincr, tlist, tstd, undo, unixc, re_compile, re_error, mktmpfunny
 *
 * ORIGINS: 3, 10, 18, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1984, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *      Unpublished work.
 *      (c) Copyright INTERACTIVE Systems Corp. 1983, 1985
 *      Licensed Material - Program property of INTERACTIVE Systems Corp.
 *      All rights reserved.
 *
 *      RESTRICTED RIGHTS
 *      These programs are supplied under a license.  They may be used or
 *      copied only as permitted under such license agreement.  Any
 *      Authorized copy must contain the above notice and this restricted
 *      rights notice.  Disclosure of the programs is strictly prohibited
 *      unless otherwise provided in the license agreement.
 *
 *      Encryption used to be done by recognizing that text had high-order
 *      bits on.  Now that can't be the method; not clear how to solve this.
 *
 *      Present version does not support encryption.
 *
 *
 * (c) Copyright 1990 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.0
 */

#undef CRYPT
/*
** Editor
*/

#define _ILS_MACROS
#include <locale.h>
#include "ed_msg.h"
#include <unistd.h>
#include <sys/limits.h>
#include <string.h>
#include <ctype.h>
static nl_catd catd;

static char    *msgtab[] =
{
        "write or open on pipe failed",                 /*  0 */
        "warning: expecting `w'",                       /*  1 */
        "mark not lower case",                          /*  2 */
        "cannot open input file",                       /*  3 */
        "PWB spec problem",                             /*  4 */
        "nothing to undo",                              /*  5 */
        "restricted shell",                             /*  6 */
        "cannot create output file",                    /*  7 */
        "filesystem out of space!",                     /*  8 */
        "cannot open file",                             /*  9 */
        "cannot link",                                  /* 10 */
        "Range endpoint too large",                     /* 11 */
        "unknown command",                              /* 12 */
        "search string not found",                      /* 13 */
        "-",                                            /* 14 */
        "line out of range",                            /* 15 */
        "bad number",                                   /* 16 */
        "bad range",                                    /* 17 */
        "Illegal address count",                        /* 18 */
        "incomplete global expression",                 /* 19 */
        "illegal suffix",                               /* 20 */
        "illegal or missing filename",                  /* 21 */
        "no space after command",                       /* 22 */
        "fork failed - try again",                      /* 23 */
        "maximum of 64 characters in file names",       /* 24 */
        "`\\digit' out of range",                       /* 25 */
        "interrupt",                                    /* 26 */
        "line too long",                                /* 27 */
        "illegal character in input file",              /* 28 */
        "write error",                                  /* 29 */
        "out of memory for append",                     /* 30 */
        "-",                                            /* 31 */
        "I/O error on temp file",                       /* 32 */
        "multiple globals not allowed",                 /* 33 */
        "global too long",                              /* 34 */
        "no match",                                     /* 35 */
        "illegal or missing delimiter",                 /* 36 */
        "-",                                            /* 37 */
        "replacement string too long",                  /* 38 */
        "illegal move destination",                     /* 39 */
        "-",                                            /* 40 */
        "no remembered search string",                  /* 41 */
        "'\\( \\)' imbalance",                          /* 42 */
        "Too many `\\(' s",                             /* 43 */
        "more than 2 numbers given",                    /* 44 */
        "'\\}' expected",                               /* 45 */
        "first number exceeds second",                  /* 46 */
        "incomplete substitute",                        /* 47 */
        "newline unexpected",                           /* 48 */
        "'[ ]' imbalance",                              /* 49 */
        "regular expression overflow",                  /* 50 */
        "regular expression error",                     /* 51 */
        "command expected",                             /* 52 */
        "a, i, or c not allowed in G",                  /* 53 */
        "end of line expected",                         /* 54 */
        "no remembered replacement string",             /* 55 */
        "no remembered command",                        /* 56 */
        "illegal redirection",                          /* 57 */
        "possible concurrent update",                   /* 58 */
        "that command confuses yed",                    /* 59 */
        "the x command has become X (upper case)",      /* 60 */
        "Warning: 'w' may destroy input file (due to `illegal char' read earlier)", /* 61 */
        "Caution: 'q' may lose data in buffer; 'w' may destroy input file", /* 62 */
        0
};

static int     peekc;
int     getchr(void);
void    error1(int);

/*
 * Define some macros for the regular expression
 * routines to use for input and error stuff.
 */

#include <stdlib.h>
#include <ctype.h>
#include <regex.h>
#include <stdio.h>
#include <sys/file.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
/* Following line included while bringing OSF code into AIX environment */
#include <sys/statfs.h>
#include <termio.h>
#include <setjmp.h>

/* macro for message catalog access */
#define MSGSTR(num, str)	catgets(catd, MS_ED, num, str)
#define PUTM(num)  if (num >= 0) { \
		msg_ptr = MSGSTR(num+1, msgtab[num]); \
		puts(msg_ptr); \
	}

#define MAXLINES 32756
#define FNSIZE PATH_MAX+1
#define LBSIZE  LINE_MAX+2
#define GBSIZE  LINE_MAX+2
#define GLOBAL_SUB -1	/* global substitution	*/
#define KSIZE   9
/* Though ed supports lines up to LINE_MAX bytes, all IO is done in 512 byte */
/* chunks.  IO is limited to reading and writing of the current file buffer  */
/* and is performed in the routines getblk() and blkio().		     */
#define IOBLKSIZE 512

#define READ    0
#define WRITE   1

#define PRNT    02

static int     Xqt = 0;
static int     lastc;
static char    savedfile[FNSIZE+1];
static char    file[FNSIZE+1];
static char    origfile[FNSIZE+1];	/* holds name of originally edited file */
static char    funny[LBSIZE];
static unsigned char    linebuf[LBSIZE];

static char   	expbuf[LBSIZE];	/* Buffer for input regular expression	*/
static regex_t		preg;		/* Holds compiled regular expression	*/
static regmatch_t 	pmatch[_REG_SUBEXP_MAX + 1];	/* Holds substring offsets for subexpressions	*/

static unsigned char    rhsbuf[LBSIZE];

typedef struct position {
	unsigned int	bno;		/* Block number		*/
	unsigned short	off;		/* Offset within block	*/
	unsigned short	flags;		/* Flags		*/
#define MARK_1		0x1
} POS;

struct  lin     {
	POS cur;
	POS sav;
};
typedef struct lin *LINE;

char    *getenv(const char *);
void    *calloc(size_t, size_t);
LINE    address(void);
unsigned char    *getline(POS);
unsigned char    *getblock(POS, int);
unsigned char    *place(register unsigned char *, register unsigned char *, register unsigned char *);
int	statfs(char *, struct statfs *);
int	fsync(int);
pid_t   wait(int *stat_loc);

char    *mktemp(char *);
static struct  stat Fl, Tf;
#ifndef RESEARCH
static int     Short = 0;
static int     oldmask; /* No umask while writing */
#endif
static jmp_buf savej;
#ifdef  NULLS
int     nulls;  /* Null count */
#endif
static long    ccount;

struct  Fspec   {
	char    Ftabs[22];
	char    Fdel;
	unsigned char   Flim;
	char    Fmov;
	char    Ffill;
};
static struct  Fspec   fss;

static int     errcnt=0;



static int     maxlines;
static LINE    zero;
static LINE    dot;
static LINE    dol;
static LINE    endcore;
static LINE    fendcore;
static LINE    addr1;
static LINE    addr2;
static LINE    savdol, savdot;
static int     globflg;
static int     initflg;
static unsigned char    genbuf[LBSIZE];
static long    count;
static unsigned char    *nextip;
static unsigned char    *linebp;
static int     ninbuf;
static int     io;
static void    (*oldhup)(int);
static void    (*oldquit)(int), (*oldpipe)(int);
static int     vflag = 1;
static int     yflag;
#ifdef TRACE
char	*tracefile;
FILE	*trace;
#endif	
#ifdef CRYPT
int     xflag;
int     xtflag;
int     kflag;
#endif
static int     hflag;
static int     xcode = -1;
#ifdef CRYPT
char    key[KSIZE + 1];
char    crbuf[IOBLKSIZE];
char    tperm[768];
char    perm[768];
#endif
static int     col;
static char    *globp;
static int     tfile = -1;
static POS     tline;
static char    *tfname;
static unsigned char    ibuff[IOBLKSIZE];
static int     iblock = -1;
static unsigned char    obuff[IOBLKSIZE];
static int     oblock = -1;
static int     ichanged;
static int     nleft;
static POS     names[26];
static int     anymarks;
static POS     subnewa;
static int     fchange;
static int     nline;
static int     fflg, shflg;
/* Define the length of the prompt string */
#define	PRLEN	16
static char    prompt[PRLEN+1] = "*";
static int     rflg;
static int     readflg;
static int     eflg;
static int     ncflg;
static int     listn;
static int     listf;
static int     pflag;
static int     flag28 = 0; /* Prevents write after a partial read */
static int     save28 = 0; /* Flag whether buffer empty at start of read */
static long    savtime;
static char    *name = "SHELL";
static char	*rshell1 = "/usr/bin/rsh";
static char	*rshell2 = "/usr/bin/Rsh";
static char    *val;
static char    *home;

#ifdef	_SECURITY
char	*aclp;			/* holds acl information		*/
char	*acl_get();
#endif

static int	scommand;	/* Indicates when a substitute command is in progress	*/
static char	*offset_base;	/* Stores the current base for Regular Expression offsets */
static char 	*expend;	/* Stores the end element position in the expression buffer */

void    init(void);
void    commands(void);
void    quit(void);  
void    setdot(void);
void    newline(void);
void    save(void);
void    append(int (*)(void), LINE);
void    delete(void);
void    setnoaddr(void);
int     error(register);
void    filename(register);
void    setall(void);
void    nonzero(void);
void    exfile(void);
int     gettty(void);
int     getfile(void);
void    putfile(void);
void    unixcom(void);
void    rdelete(LINE, LINE);
void    gdelete(void);
POS     putline(void);
void 	blkio(int, char *, int (*)(int, char *, unsigned int));
void    global(int);
void    join(void);
void    substitute(int);
int     compsub(void);
int     getsub(void);
void    dosub(void);
void    move(int);
void    reverse(register LINE, register LINE);
int     getcopy(void);
int     execute(int, LINE);
void    putd(void);
int     puts(const char *);
void    putchr(int);
void	putwchr(wchar_t);
#ifdef CRYPT
void    crblock(char *, char *, int *, long *);
void    getkey(void);
int     crinit(char *, char *);
void    makekey(char *, char *);
#endif
void    globaln(int);
int     eopen(char *, int);
void    eclose(int);
void    mkfunny(void);
void    mktmpfunny(void);
void    getime(void);
void    chktime(void);
void    newtime(void);
void    red(char *);
int     fspec(char *, struct Fspec *, int);
int     numb(void);
void    targ(struct Fspec *);
void    tincr(int, struct Fspec *);
void    tstd(struct Fspec *);
void    tlist(struct Fspec *);
int     expnd(char *, char *,int *, struct Fspec *);
void    clear(struct Fspec *);
int     lenchk(char *, struct Fspec *);
int     stdtab(char *, char *);
void    undo(void);
void	re_compile(wchar_t);
void	re_error(int);
void	onpipe(void);

static void onpipe(void)
{
	(void) error(0);
}

main(int argc, char **argv)
{
	register        char *p1, *p2;
	extern  void onintr(void),  onhup(void);
	void     (*oldintr)(int);
	int	c, n, len;

	(void) setlocale(LC_ALL,"");		/* required by NLS environment tests */
	catd = catopen(MF_ED, NL_CAT_LOCALE);

#ifdef  STANDALONE
	if (argv[0][0] == '\0')
		argc = getargv("ed",&argv,0);
#endif

	oldquit = signal(SIGQUIT, SIG_IGN);
	oldhup = signal(SIGHUP, SIG_IGN);
	oldintr = signal(SIGINT, SIG_IGN);
	oldpipe = signal(SIGPIPE, (void (*)(int))onpipe);
	if (((int)signal(SIGTERM, SIG_IGN)&01) == 0)
		(void) signal(SIGTERM, (void (*)(int))quit);

	expend = &expbuf[LBSIZE];
	p1 = *argv;
	while(*p1++);
	while(--p1 >= *argv)
		if(*p1 == '/')
			break;
	*argv = p1 + 1;
	/* if SHELL set in environment and is /usr/bin/rsh or /usr/bin/Rsh, */
	/* then set rflg 						    */
	if((val = getenv(name)) != NULL)
		if ((strcmp(val, rshell1) == 0) || (strcmp(val, rshell2) == 0))
			rflg++;
	if (**argv == 'r')
		rflg++;
	home = getenv("HOME");
	c = 0;
	while (c != -1) {
		if ((strcmp(argv[optind], "-")) == 0)  {
			vflag = 0;
			optind++;
		} else switch(c = getopt(argc, argv, "sp:qxTy")) {
			case -1:
				/* done parsing */
				break;
			case 's':
				vflag = 0;
				break;

			case 'p':
				(void) strncpy(prompt, optarg, PRLEN);
				/* Ensure that the prompt string has not been 	*/
				/* truncated in the middle of a multi-byte character */
				if (strlen(prompt) >= PRLEN) {
					n = PRLEN - 1;
					while (((len = mblen(&prompt[n], MB_CUR_MAX)) == -1) || (n + len > PRLEN)) 
						n--;
					prompt[n+len] = '\0';
				}
				shflg = 1;
				break;

			case 'q':
				signal(SIGQUIT, SIG_DFL);
				vflag = 1;
				break;

#ifdef CRYPT
			case 'x':
				xflag = 1;
				break;
#endif

#ifdef TRACE
			case 'T':
				tracefile = "trace";
				trace = fopen(tracefile, "w");
				break;
#endif
			case 'y':
				yflag = 03;
				break;
			default:
				puts(MSGSTR(ED_USAGE, 
					"usage: ed [-p string] [-s|-] [file]\n"));
				exit(1);
		}
	}
	argc -= optind;
	argv += optind;
#ifdef CRYPT
	if(xflag){
		getkey();
		kflag = crinit(key, perm);
	}
#endif

	if (argc>0) {
		p1 = *argv;
		if(strlen(p1) > FNSIZE) {
			puts(MSGSTR(M_EDFILE, "file name too long"));
			exit(2);
		}
		p2 = savedfile;
		while (*p2++ = *p1++);
		globp = "r";
		fflg++;
	}
	else    /* editing with no file so set savtime to 0 */
		savtime = 0;
	eflg++;
	maxlines = MAXLINES;
	while (!(fendcore = (LINE)malloc(maxlines*sizeof(struct lin))))
		maxlines-=1024;
	tfname = mktemp("/var/tmp/eXXXXXX");
	init();
	if (((int)oldintr&01) == 0)
		(void) signal(SIGINT, (void (*)(int))onintr);
	if (((int)oldhup&01) == 0)
		(void) signal(SIGHUP, (void (*)(int))onhup);
	preg.re_comp = 0;
	(void) setjmp(savej);
	commands();
	quit();
        exit(0);
}

#ifndef OLD
/* filecopy: copy file ifp to file ofp */
static void filecopy(FILE *ifp, FILE *ofp)
{
	int c;

#ifdef TRACE
	if (trace)
		fprintf(trace, "in filecopy\n");
#endif

	while ((c = getc(ifp)) != EOF)
		putc(c,ofp);
}
#endif

static void commands(void)
{
/*	int getfile(), gettty();  */
	char	*msg_ptr;		/* pointer to message text */
	register LINE a1;
	register c;
	register char *p1, *p2;
	struct statfs sb;
	struct termio	tty;
	int n;
        int tmplistf, tmplistn;
#ifndef OLD
	FILE *outfi;
	FILE *tmpfi;
#endif

	for (;;) {
	scommand = 0;	/* Initialises and clears this flag	*/
	if ( pflag ) {
		pflag = 0;
		addr1 = addr2 = dot;
		goto print;
	}
	if (shflg && globp==0)
		(void) write(1, prompt, strlen(prompt));
	addr1 = 0;
	addr2 = 0;
	c=getchr();
	while((c == ' ') || (c == '\t'))
		c=getchr();
	if(c == ',') {
		addr1 = zero + 1;
		addr2 = dol;
		c = getchr();
	} else if(c == ';') {
		addr1 = dot;
		addr2 = dol;
		c = getchr();
	} else {
		peekc = c;
		do {
			addr1 = addr2;
			if ((a1 = address())==0) {
				c = getchr();
				break;
			}
			addr2 = a1;
			if ((c=getchr()) == ';') {
				c = ',';
				dot = a1;
			}
		} while (c==',');
		if (addr1==0)
			addr1 = addr2;
	}
	while((c == ' ') || (c == '\t'))
		c=getchr();
	switch(c) {

	case 'a':
		setdot();
		newline();
		if (!globflg) save();
		append(gettty, addr2);
		continue;

	case 'c':
		delete();
		append(gettty, addr1-1);
		/* XPG4 - If no new lines are inserted, then the current */
		/* line becomes the line after the lines deleted.        */
		if ((dot == addr1-1) && (addr1 <= dol))
			dot = addr1;  
		continue;

	case 'd':
		delete();
		continue;

	case 'E':
		fchange = 0;
		c = 'e';
	case 'e':
		fflg++;
		setnoaddr();
		if (vflag && fchange) {
			fchange = 0;
			(void) error(1);
		}
		filename(c);
		eflg++;
		init();
		addr2 = zero;
		goto caseread;

	case 'f':
		setnoaddr();
		filename(c);
		if (!ncflg)  /* there is a filename */
			getime();
		else
			ncflg--;
		puts(savedfile);
		continue;

	case 'g':
		global(1);
		continue;
	case 'G':
		globaln(1);
		continue;

	case 'h':
		newline();
		setnoaddr();
		PUTM(xcode);
		continue;

	case 'H':
		newline();
		setnoaddr();
		if(!hflag) {
			hflag = 1;
			PUTM(xcode);
		}
		else
			hflag = 0;
		continue;

	case 'i':
		setdot();
		nonzero();
		newline();
		if (!globflg) save();
		append(gettty, addr2-1);
		if (dot == addr2-1)
			dot += 1;
		continue;


	case 'j':
		if (addr2==0) {
			addr1 = dot;
			addr2 = dot+1;
		}
		setdot();
		newline();
		nonzero();
		if (!globflg) save();
		join();
		continue;

	case 'k':
		if ((c = getchr()) < 'a' || c > 'z')
			(void) error(2);
		newline();
		setdot();
		nonzero();
		names[c-'a'] = addr2->cur;
		anymarks |= 01;
		continue;

	case 'm':
		move(0);
		continue;

	case '\n':
		if (addr2==0)
			addr2 = dot+1;
		addr1 = addr2;
		goto print;

	case 'n':
		listn++;
		newline();
		goto print;

	case 'l':
		listf++;
	case 'p':
		newline();
	print:
		setdot();
		nonzero();
		a1 = addr1;
		do {
			if (listn) {
				count = a1 - zero;
				putd();
				putchr('\t');
			}
			puts((char *)getline((a1++)->cur));
		}
		while (a1 <= addr2);
		dot = addr2;
		pflag = 0;
		listn = 0;
		listf = 0;
		continue;

	case 'Q':
		fchange = 0;
	case 'q':
		setnoaddr();
		newline();
		quit();

	case 'r':
		filename(c);
	caseread:
		readflg = 1;
		save28 = (dol != fendcore);
		if ((io = eopen(file, 0)) < 0) {
			lastc = '\n';
			/* if first entering editor and file does not exist */
			/* set saved access time to 0 */
			if (eflg) {
				savtime = 0;
				eflg  = 0;
			}
			(void) error(3);
		}
		/* get last mod time of file */
		/* eflg - entered editor with ed or e  */
		if (eflg) {
			eflg = 0;
			getime();
			(void) strcpy(origfile,file);
		}
#ifdef _SECURITY
		/* get up to date acl of original file		*/
		aclp = acl_get(origfile);
		/* put acl on temporary file in case of crash	*/
		acl_put(tfname,aclp,0);
#endif
		setall();
		ninbuf = 0;
		n = zero != dol;
#ifdef NULLS
		nulls = 0;
#endif
		if (!globflg && (c == 'r')) save();
		append(getfile, addr2);
		exfile();
		readflg = 0;
		fchange = n;
		continue;

	case 's':
		scommand = 1;
		setdot();
		nonzero();
		if (!globflg) save();
		substitute(globp!=0);
		continue;

	case 't':
		move(1);
		continue;

	case 'u':
                setnoaddr();
		setdot();
		newline();
		if (!initflg) undo();
		else (void) error(5);
		fchange = 1;
		continue;

	case 'v':
		global(0);
		continue;
	case 'V':
		globaln(0);
		continue;

	case 'w':
		if (flag28) {
			flag28 = 0;
			fchange = 0;
			(void) error(61);
		}
		setall();
		if((zero != dol) && (addr1 <= zero || addr2 > dol))
			(void) error(15);
		filename(c);
		if(Xqt) {
			io = eopen(file, 1);
			n = 1;  /* set n so newtime will not execute */
		} else {
			(void) fstat(tfile, &Tf);
			if(stat(file, &Fl) < 0) {
				if((io = open(file, O_CREAT|O_RDWR, 0666)) < 0)
					(void) error(7);
				(void) fstat(io, &Fl);
				Fl.st_mtime = 0;
				(void) close(io);
			}
			else {
#ifndef RESEARCH
				oldmask = umask(0);
#endif
			}
#ifndef RESEARCH
                        (void) statfs(file, &sb);
                        if(!Short &&
                            sb.f_blocks <
                           ((Tf.st_size / sb.f_bsize) +
                            (50000 / sb.f_bsize))) {
                                Short = 1;
                                (void) error(8);
                        }
                        Short = 0;
#endif
			if(/*Fl.st_nlink == 1 &&*/ (Fl.st_mode & S_IFMT) == S_IFREG)
			{       if (close(open(file, 1)) < 0)
					(void) error(9);
				p1 = savedfile;
				p2 = file;
#ifdef TRACE
				if (trace)
					fprintf(trace, "origfile = %s, p1 = %s, p2 = %s\n",origfile,p1,p2);
#endif
				if (!(n=strcmp(p1, p2)))
					chktime();
				mkfunny();
/***MBROWN
				if ((io = creat(funny, Fl.st_mode)) >= 0) {
***/
                                if ((io = open(funny, O_CREAT|O_RDWR, Fl.st_mode)) < 0)
					mktmpfunny();
				if (io >= 0 || (io = open(funny, O_CREAT|O_RDWR, Fl.st_mode)) >= 0) {
#ifdef OLD
					chown(funny, Fl.st_uid, Fl.st_gid);
					chmod(funny, Fl.st_mode);
#endif
#ifdef TRACE
				if (trace)
					fprintf(trace, "funny = %s\n", funny);
#endif
					putfile();
					exfile();
#ifdef OLD
					unlink(file);
					if (link(funny, file))
						(void) error(10);
					unlink(funny);
#else
					if ((outfi = fopen(file,"w")) == NULL || (tmpfi = fopen(funny,"r")) == NULL)
						(void) error(7);
					else
						filecopy(tmpfi,outfi);
					fclose(outfi);
					fclose(tmpfi);
					unlink(funny);
#ifdef TRACE
				if (trace)
					fprintf(trace, "after fclose(outfi)\n");
#endif
#endif

					/* if filenames are the same */
					if (!n)
						newtime();
					/* check if entire buffer was written */
					fchange = ((addr1==zero || addr1==zero+1) && addr2==dol)?0:fchange;
					continue;
				}
				else
					(void) error(7);
			}
			else   n = 1;   /* set n so newtime will not execute*/
			if((io = open(file, O_CREAT|O_RDWR, 0666)) < 0)
				(void) error(7);
		}
		putfile();
		exfile();
		if (!n) newtime();
		fchange = ((addr1==zero||addr1==zero+1)&&addr2==dol)?0:fchange;
#ifdef _SECURITY
		/* get up to date acl of original file		*/
		aclp = acl_get(origfile);
		/* put acl on file to be written		*/
		acl_put(file,aclp,0);
#endif
		continue;

#ifdef CRYPT
	case 'X':
		setnoaddr();
		newline();
		xflag = 1;
		getkey();
		kflag = crinit(key, perm);
		continue;
#endif


	case '=':
		setall();
		newline();
                tmplistf = listf;
                tmplistn = listn;
                listf = 0;
                listn = 0;
		count = (addr2-zero)&077777;
		putd();
		putchr('\n');
                listf = tmplistf;
                listn = tmplistn;
		continue;

	case '!':
		unixcom();
		continue;

	case EOF:
		return;

	case 'P':
		if (yflag)
			(void) error(59);
		setnoaddr();
		newline();
		if (shflg)
			shflg = 0;
		else
			shflg++;
		continue;
	}
#ifdef CRYPT
	if (c == 'x')
		(void) error(60);
	else
#endif
		(void) error(12);
	}
}

static LINE
address(void)
{
	register minus, c;
	register LINE a1;
	int n, relerr;
	char	tmp;
	wchar_t	wch;

	minus = 0;
	a1 = 0;
	for (;;) {
		c = getchr();
		if ('0'<=c && c<='9') {
			n = 0;
			do {
				n *= 10;
				n += c - '0';
			} while ((c = getchr())>='0' && c<='9');
			peekc = c;
			if (a1==0)
				a1 = zero;
			if (minus<0)
				n = -n;
			a1 += n;
			minus = 0;
			continue;
		}
		relerr = 0;
		if (a1 || minus)
			relerr++;
		switch(c) {
		case ' ':
		case '\t':
			continue;

		case '+':
			minus++;
			if (a1==0)
				a1 = dot;
			continue;

		case '-':
		case '^':
			minus--;
			if (a1==0)
				a1 = dot;
			continue;

		case '?':
		case '/':
			tmp = c;
			if (mbtowc(&wch, &tmp, MB_CUR_MAX) != 1)
				error(51);
			re_compile(wch);  /* Compile the Regular Expression */
			a1 = dot;
			for (;;) {
				if (c=='/') {
					a1++;
					if (a1 > dol)
						a1 = zero;
				} else {
					a1--;
					if (a1 < zero)
						a1 = dol;
				}
				if (execute(0, a1))
					break;
				if (a1==dot)
					(void) error(13);
			}
			break;

		case '$':
			a1 = dol;
			break;

		case '.':
			a1 = dot;
			break;

		case '\'':
			if ((c = getchr()) < 'a' || c > 'z')
				(void) error(2);
			for (a1=zero+1; a1<=dol; a1++)
				if ((names[c-'a'].bno == a1->cur.bno) &&
				    (names[c-'a'].off == a1->cur.off))
					break;
			break;

		case 'y' & 037:
			if(yflag) {
				newline();
				setnoaddr();
				yflag ^= 01;
				continue;
			}

		default:
			peekc = c;
			if (a1==0)
				return(0);
			a1 += minus;
			if (a1<zero || a1>dol)
				(void) error(15);
			return(a1);
		}
		if (relerr)
			(void) error(16);
	}
}

static void setdot(void)
{
	if (addr2 == 0)
		addr1 = addr2 = dot;
	if (addr1 > addr2)
		(void) error(17);
}

static void setall(void)
{
	if (addr2==0) {
		addr1 = zero+1;
		addr2 = dol;
		if (dol==zero)
			addr1 = zero;
	}
	setdot();
}

static void setnoaddr(void)
{
	if (addr2)
		(void) error(18);
}

static void nonzero(void)
{
	if (addr1<=zero || addr2>dol)
		(void) error(15);
}

static void newline(void)
{
	register c;

	c = getchr();
	if ( c == 'p' || c == 'l' || c == 'n' ) {
		pflag++;
		if ( c == 'l') listf++;
		if ( c == 'n') listn++;
		c = getchr();
	}
	if ( c != '\n')
		(void) error(20);
}

static void filename(register comm)
{
	register char *p1, *p2;
	register c;
	register i = 0;

	count = 0;
	c = getchr();
	if (c=='\n' || c==EOF) {
		p1 = savedfile;
		if (*p1==0 && comm!='f')
			(void) error(21);
		/* ncflg set means do not get mod time of file */
		/* since no filename followed f */
		if (comm == 'f')
			ncflg++;
		p2 = file;
		while (*p2++ = *p1++);
		red(savedfile);
		return;
	}
	if (c!=' ')
		(void) error(22);
	while ((c = getchr()) == ' ');
	if(c == '!')
		++Xqt, c = getchr();
	if (c=='\n')
		(void) error(21);
	p1 = file;
	do {
		if(++i >= FNSIZE)
			(void) error(24);
		*p1++ = c;
		if(c==EOF || (c==' ' && !Xqt))
			(void) error(21);
	} while ((c = getchr()) != '\n');
	*p1++ = 0;
	if(Xqt)
		if (comm=='f') {
			--Xqt;
			(void) error(57);
		}
		else
			return;
	if (savedfile[0]==0 || comm=='e' || comm=='f') {
		p1 = savedfile;
		p2 = file;
		while (*p1++ = *p2++);
	}
	red(file);
}

static void exfile(void)
{
#ifdef NULLS
	register c;
#endif

#ifndef RESEARCH
	if(oldmask) {
		umask(oldmask);
		oldmask = 0;
	}
#endif
	eclose(io);
	io = -1;
	if (vflag) {
		putd();
		putchr('\n');
#ifdef NULLS
		if(nulls) {
			c = count;
			count = nulls;
			nulls = 0;
			putd();
			puts(MSGSTR(M_EDNULLS,
                		" nulls replaced by '\\0'"));
			/* Leave space at beginning, message follows a number */
			count = c;
		}
#endif
	}
}

static void onintr(void)
{
	(void) signal(SIGINT, (void (*)(int))onintr);
	putchr('\n');
	lastc = '\n';
	if (*funny) 
		(void) unlink(funny); /* remove tmp file */
	/* if interruped a read, only part of file may be in buffer */
	if ( readflg ) {
		puts(MSGSTR(M_EDINCREAD,
			"\007read may be incomplete - beware!\007"));
		fchange = 0;
	}
	(void) error(26);
}

static void onhup(void)
{
	(void) signal(SIGINT, SIG_IGN);
	(void) signal(SIGHUP, SIG_IGN);
	/* if there are lines in file and file was */
	/* not written since last update, save in ed.hup, or $HOME/ed.hup */
	if (dol > zero && fchange == 1) {
		addr1 = zero+1;
		addr2 = dol;
		io = open("ed.hup", O_CREAT|O_RDWR, 0666);
		if(io < 0 && home) {
			char    *fn;

			fn = calloc(strlen(home) + 8, sizeof(char));
			if(fn) {
				(void) strcpy(fn, home);
				(void) strcat(fn, "/ed.hup");
				io = open(fn, O_CREAT|O_RDWR, 0666);
				free(fn);
			}
		}
		if (io)
			putfile();
	}
#ifdef _SECURITY
	/* get up to date acl of original file		*/
	aclp = acl_get(origfile);
	/* put acl on file to be written		*/
	acl_put(io,aclp,0);
#endif
	fchange = 0;
	quit();
}

/*
 * Checks the error status returned from the regcomp() routine and invokes the
 * error() function with the appropriate error code.
 *
 */
static void re_error(int regcomp_status)
{
	switch(regcomp_status) {

	case REG_ESUBREG:
			(void) error(25);
			break;

	case REG_EBRACK:
			(void) error(49);
			break;

	default:
			(void) error(51);
			break;
	}
}

static int error(register code)
{
	char	*msg_ptr;		/* pointer to message text */
	register c;

	if (code == 28 && save28 == 0) {
		fchange = 0; 
		flag28++;
	}
	readflg = 0;
	++errcnt;
	listf = listn = 0;
	pflag = 0;
#ifndef RESEARCH
	if(oldmask) {
		umask(oldmask);
		oldmask = 0;
	}
#endif
#ifdef NULLS    /* Not really nulls, but close enough */
	/* This is a bug because of buffering */
	if(code == 28) /* illegal char. */
		putd();
#endif
	putchr('?');
	if(code == 3)   /* Cant open file */
		puts(file);
	else
		putchr('\n');
	count = 0;
	(void) lseek(0, (long)0, 2);
	if (globp)
		lastc = '\n';
	globp = 0;
	peekc = lastc;
	if(lastc)
		while ((c = getchr()) != '\n' && c != EOF);
	if (io) {
		eclose(io);
		io = -1;
	}
	xcode = code;
	if(hflag)
		PUTM(xcode);
	if(code==4)return(0);   /* Non-fatal error. */
	longjmp(savej, 1);
	return(1);
}


static int getchr(void)
{
	unsigned char c;

	if (lastc=peekc) {
		peekc = 0;
		return(lastc);
	}
	if (globp) {
		if ((lastc = *globp++) != 0)
			return(lastc);
		globp = 0;
		return(EOF);
	}
	if (read(0, &c, 1) <= 0)
		return(lastc = EOF);
	lastc = (int)c; 
	return(lastc);
}

static int gettty(void)
{
	register c;
	register char *gf;
	register unsigned char *p;

	p = linebuf;
	gf = globp;
	while ((c = getchr()) != '\n') {
		if (c==EOF) {
			if (gf)
				peekc = c;
			return(c);
		}
		if (c == 0)
			continue;
		*p++ = c;
		if (p >= &linebuf[LBSIZE-2])
			(void) error(27);
	}
	*p++ = 0;
	if (linebuf[0]=='.' && linebuf[1]==0)
		return(EOF);

/*      This is not allowed per XPG4/POSIX 
 *	if (linebuf[0]=='\\' && linebuf[1]=='.' && linebuf[2]==0) {
 *		linebuf[0] = '.';
 *		linebuf[1] = 0;
 *	}
 */
	return(0);
}

static int getfile(void)
{
	char	*msg_ptr;		/* pointer to message text */
	register c;
	register unsigned char *lp, *fp;

	lp = linebuf;
	fp = nextip;
	do {
		if (--ninbuf < 0) {
			if ((ninbuf = read(io, genbuf, LBSIZE)-1) < 0)
				return(EOF);
			fp = genbuf;
		}
		if (lp >= &linebuf[LBSIZE]) {
			lastc = '\n';
			(void) error(27);
		}
		if ((*lp++ = c = *fp++) == 0) { 
#ifdef NULLS
			lp[-1] = '\\';
			*lp++ = '0';
			nulls++;
#else
			lp--;
			continue;
#endif
		}
		count++;
	} while (c != '\n');
	*--lp = 0;
	nextip = fp;
	if (fss.Ffill && fss.Flim && lenchk(linebuf,&fss) < 0) {
		msg_ptr =  MSGSTR(M_EDLINE,
			"line too long: lno = ");
		(void) write(1, msg_ptr, strlen(msg_ptr));

		ccount = count;
		count = (++dot-zero)&077777;
		dot--;
		putd();
		count = ccount;
		putchr('\n');
	}
	return(0);
}

static void putfile(void)
{
	char	*msg_ptr;		/* pointer to message text */
	int n;
	LINE a1;
	register unsigned char *fp, *lp;
	register nib;

	nib = IOBLKSIZE;
	fp = genbuf;
	a1 = addr1;
	do {
		lp = getline((a1++)->cur);
		if (fss.Ffill && fss.Flim && lenchk(linebuf,&fss) < 0) {
			msg_ptr =  MSGSTR(M_EDLINE,
				"line too long: lno = ");
			(void) write(1, msg_ptr, strlen(msg_ptr));
			ccount = count;
			count = (++dot-zero)&077777;
			dot--;
			putd();
			count = ccount;
			putchr('\n');
		}
		for (;;) {
			if (--nib < 0) {
				n = fp-genbuf;
#ifdef CRYPT
				if(kflag)
					crblock(perm, genbuf, n, count-n);
#endif
				if(write(io, genbuf, n) != n)
					(void) error(29);
				nib = 511;
				fp = genbuf;
			}
			if (dol == zero)
				break; /* Allow write of null file */
			count++;
			if ((*fp++ = *lp++) == 0) {
				fp[-1] = '\n';
				break;
			}
		}
	} while (a1 <= addr2);
	n = fp-genbuf;
#ifdef CRYPT
	if(kflag)
		crblock(perm, genbuf, n, count-n);
#endif
	if(write(io, genbuf, n) != n || (Xqt ? 0 : fsync(io)))
		(void) error(29);
}

static void append(int (*f10)(void), LINE a)
{
	register LINE a1, a2, rdot, tmp;
	POS tl;

	nline = 0;
	dot = a;
	while ((*f10)() == 0) {
		if (dol >= endcore) {
			maxlines += 1024;
			if (tmp = (LINE)realloc(fendcore,maxlines*sizeof(struct lin))) {
				/* adjust line stuff to new buffer */
				if (tmp != fendcore) {
					zero   += tmp - fendcore;
					dot    += tmp - fendcore;
					dol    += tmp - fendcore;
					addr1  += tmp - fendcore;
					addr2  += tmp - fendcore;
					savdol += tmp - fendcore;
					savdot += tmp - fendcore;
					a      += tmp - fendcore;
					fendcore = tmp;
				}
				endcore = fendcore + maxlines - 1;
			} else {
				maxlines-=1024;
				lastc = '\n';
				(void) error(30);
			}
		}
		tl = putline();
		nline++;
		a1 = ++dol;
		a2 = a1+1;
		rdot = ++dot;
		while (a1 > rdot)
			(--a2)->cur = (--a1)->cur;
		rdot->cur = tl;
	}
}

static void unixcom(void)
{
	void (*savint)(int);
	register  pid, rpid;
	int retcode;
	static char savcmd[LBSIZE];     /* last command */
	char curcmd[LBSIZE];            /* current command */
	char *psavcmd, *pcurcmd, *psavedfile;
	register c, shflag=0;
	int	i, len;
	char	str[MB_LEN_MAX];
		
	setnoaddr();
	if(rflg)
		(void) error(6);
	pcurcmd = curcmd;
	/* A '!' found in beginning of command is replaced with the */
	/* saved command. A '%' found in command is replaced with   */
	/* the current filename					    */
	c = getchr();
	if (c == '!') {
		if (savcmd[0]==0)
			(void) error(56);
		else {
			psavcmd = savcmd;
			while (*pcurcmd++ = *psavcmd++);
			--pcurcmd;
			shflag = 1;
		}
	} else
		 peekc = c;  /* put c back */

	for (;;) {
		if ((c = getchr()) == '\n') 	/* end of command */
			break;
		else if (c == '%') {		
			/* insert current filename into command string */
			if (savedfile[0]==0)
				/* no remembered filename */
				(void) error(21);
			else {
				psavedfile = savedfile;
				while (*pcurcmd++ = *psavedfile++);
				--pcurcmd;
				shflag = 1;
			}
			continue;
		} else if (c == '\\') {
			/* '\\' has special meaning only if preceding a '%' */
			if ((c = getchr()) != '%') 
				*pcurcmd++ = '\\';
			else {
				*pcurcmd++ = c;
				continue;
			}
		} 
		/* Copy multi-byte character to pcurcmd */
		len = 1;
		str[0] = c;
		while (mblen(str, MB_CUR_MAX) != len) {
			if (++len > MB_CUR_MAX)
				(void) error(20);
			str[len-1] = getchr();
		}
		for (i=0; i<len; i++)
			*pcurcmd++ = str[i];
		if (pcurcmd >= &curcmd[LBSIZE - 1])
			(void) error(27);
	}
	*pcurcmd++ = 0;
	if (shflag == 1)
		puts(curcmd);
	/* save command */
	(void) strcpy(savcmd,curcmd);

	if ((pid = fork()) == 0) {
		(void) signal(SIGHUP, (void (*)(int))oldhup);
		(void) signal(SIGQUIT, (void (*)(int))oldquit);
		(void) execlp("/usr/bin/sh", "sh", "-c", curcmd, (char *) 0);
		exit(0100);
	}
	savint = signal(SIGINT, SIG_IGN);
	while ((rpid = wait(&retcode)) != pid && rpid != -1);
	(void) signal(SIGINT, (void (*)(int))savint);
	if (vflag) puts("!");
}

static void quit(void)
{
	if (vflag && fchange) {
		fchange = 0;
		/* For case where user reads in BOTH a good file & a bad file */
		if (flag28) {
			flag28 = 0;
			(void) error(62);
		}
		(void) error(1);
	}
	(void) unlink(tfname);
#ifdef TRACE
	(void) fclose(trace);
#endif
	exit(errcnt? 2: 0);
}

static void delete(void)
{
	setdot();
	newline();
	nonzero();
	if (!globflg) save();
	rdelete(addr1, addr2);
}

static void rdelete(LINE ad1, LINE ad2)
{
	register LINE a1, a2, a3;

	a1 = ad1;
	a2 = ad2+1;
	a3 = dol;
	dol -= a2 - a1;
	do
		(a1++)->cur = (a2++)->cur;
	while (a2 <= a3);
	a1 = ad1;
	if (a1 > dol)
		a1 = dol;
	dot = a1;
	fchange = 1;
}

static void gdelete(void)
{
	register LINE a1, a2, a3;

	a3 = dol;
	for (a1=zero+1; (a1->cur.flags&MARK_1)==0; a1++)
		if (a1>=a3)
			return;
	for (a2=a1+1; a2<=a3;) {
		if ((a2->cur.flags&MARK_1) == MARK_1) {
			a2++;
			dot = a1;
		} else
			(a1++)->cur = (a2++)->cur;
	}
	dol = a1-1;
	if (dot>dol)
		dot = dol;
	fchange = 1;
}


/*
 * Obtains a pattern string for Regular Expression compilation and compiles
 * the pattern 
 *
 */
static void re_compile(wchar_t eof)
{
	char *temp;
	register c;
	int status;
	int	i, len;
	wchar_t wch;
	char	str[MB_LEN_MAX];

	status = 0;
	temp = expbuf;
	for (;;) {
		if ((c = getchr()) == '\n')
			break;
		len = 1;
		if (c == '\\') {
			*temp++ = c;
			str[0] = getchr();
			while (mbtowc(&wch, str, MB_CUR_MAX) != len) {
				if (++len > MB_CUR_MAX)
					(void) error(51);
				str[len-1] = getchr();
			}
		} else {
			str[0] = c;
			while (mbtowc(&wch, str, MB_CUR_MAX) != len) {
				if (++len > MB_CUR_MAX)
					(void) error(51);
				str[len-1] = getchr();
			}
			if (wch == eof)
				break;
		}
		for (i=0; i<len; i++)
			*temp++ = str[i];
		if (temp > expend)
			(void) error(50);
	}
	*temp = '\0';
	if (c == '\n') {
		if (scommand)	/* Substitute expression incomplete */
			(void) error1(36);
		else	/* Remember for line display execution in commands() */
			peekc = c;  
	}
	if (*expbuf != '\0') {
		if (preg.re_comp != 0)
			regfree(&preg);
		if ((status = regcomp(&preg, expbuf, 0)) != 0)
			(void) re_error(status);
	}
	else {
		if (preg.re_comp == 0)
			(void) error(41);
	}
}
 

static unsigned char *
getline(POS tl)
{
	register unsigned char *bp, *lp;
	register nl;

	lp = linebuf;
	bp = getblock(tl, READ);
	nl = nleft;
	while (*lp++ = *bp++)
		if (--nl == 0) {
			tl.bno++;
			tl.off = 0;
			bp = getblock(tl, READ);
			nl = nleft;
		}
	return(linebuf);
}

static POS putline(void)
{
	register unsigned char *bp, *lp;
	register nl;
	POS tl;

	fchange = 1;
	lp = linebuf;
	tl = tline;
	bp = getblock(tline, WRITE);
	nl = nleft;
	while (*bp = *lp++) {
		tline.off ++;
		if (*bp++ == '\n') {
			*--bp = 0;
			linebp = lp;
			break;
		}
		if (--nl == 0) {
			tline.bno++;
			tline.off = 0;
			bp = getblock(tline, WRITE);
			nl = nleft;
		}
	}
	tline.off ++;
	if (tline.off == IOBLKSIZE) {
		tline.bno ++;
		tline.off = 0;
	}
	return tl;
}

static unsigned char *
getblock(POS atl, int iof)
{
#ifdef CRYPT
	register char *p1, *p2;
	register int n;
#endif
	int bno = atl.bno;
	int off = atl.off;

	nleft = IOBLKSIZE - off;
	if (bno==iblock) {
		ichanged |= iof;
		return(ibuff+off);
	}
	if (bno==oblock)
		return(obuff+off);
	if (iof==READ) {
		if (ichanged) {
#ifdef CRYPT
			if(xtflag
)
				crblock(tperm, ibuff, IOBLKSIZE, (long)0);
#endif
			blkio(iblock, ibuff, write);
		}
		ichanged = 0;
		iblock = bno;
		blkio(bno, ibuff, read);
#ifdef CRYPT
		if(xtflag)
			crblock(tperm, ibuff, IOBLKSIZE, (long)0);
#endif
		return(ibuff+off);
	}
	if (oblock>=0) {
#ifdef CRYPT
		if(xtflag) {
			p1 = obuff;
			p2 = crbuf;
			n = IOBLKSIZE;
			while(n--)
				*p2++ = *p1++;
			crblock(tperm, crbuf, IOBLKSIZE, (long)0);
			blkio(oblock, crbuf, write);
		} else
#endif
			blkio(oblock, obuff, write);
	}
	oblock = bno;
	return(obuff+off);
}

static void blkio(int b, char *buf, int (*iofcn)(int, char *, unsigned int))
{

	(void) lseek(tfile, (long)b<<9, 0);
	if ((*iofcn)(tfile, buf, IOBLKSIZE) != IOBLKSIZE) {
		if(dol != zero) 
			(void) error(32); /* Bypass this if writing null file */
	}
}

static void init(void)
{
	register POS *markp;
	int omask;

	(void) close(tfile);
	tline.bno = 0;
	tline.off = 0;
	tline.flags = 0;
	for (markp = names; markp < &names[26]; markp++) {
		markp->bno = 0;
		markp->off = 0;
		markp->flags = 0;
	}
	subnewa.bno = 0;
	subnewa.off = 0;
	subnewa.flags = 0;
	anymarks = 0;
	iblock = -1;
	oblock = -1;
	ichanged = 0;
	initflg = 1;
	omask = umask(0);
	(void) close(open(tfname, O_CREAT|O_RDWR, 0600));
	umask(omask);
	tfile = open(tfname, 2);
#ifdef CRYPT
	if(xflag) {
		xtflag = 1;
		makekey(key, tperm);
	}
#endif
	dot = zero = dol = savdot = savdol = fendcore;
	flag28 = save28 = 0;
	endcore = fendcore + maxlines - 1;
}

static void global(int k)
{
	register char *gp;
	register c;
	register LINE a1;
	int i, len;
	wchar_t wch;
	char globuf[GBSIZE], str[MB_LEN_MAX];

	if (globp)
		(void) error(33);
	setall();
	nonzero();
	if ((c=getchr())=='\n')
		(void) error(19);
	save();
	len = 1;
	str[0] = c;
	while (mbtowc(&wch, str, MB_CUR_MAX) != len) {
		if (++len > MB_CUR_MAX)
			(void) error(19);
		str[len-1] = getchr();
	}	
	re_compile(wch);	/* Compile the Regular Expression	*/
	gp = globuf;
	for (;;) {
		if ((c = getchr()) == '\n') 
			break;
		if (c == EOF)
			(void) error(19);
		if (c == '\\') {
			/* '\\' has special meaning only if preceding a '\n' */
			c = getchr();
			if (c != '\n')
				*gp++ = '\\';
			else {
				*gp++ = c;
				if (gp >= &globuf[GBSIZE-2])
					(void) error(34);
				continue;
			}
		}
		/* The processing of this stream is done on a 		*/
		/* multi-byte character by character basis instead of 	*/
		/* byte by byte entirely because '\\' is not in the 	*/
		/* unique code-point range 				*/
		len = 1;
		str[0] = c;
		while (mblen(str, MB_CUR_MAX) != len) {
			if (++len > MB_CUR_MAX)
				(void) error(19);
			str[len-1] = getchr();
		}
		for (i=0; i<len; i++)
			*gp++ = str[i];
		if (gp >= &globuf[GBSIZE-2])
			(void) error(34);
	}
	if (gp == globuf)
		*gp++ = 'p';
	*gp++ = '\n';
	*gp++ = 0;
	for (a1=zero; a1<=dol; a1++) {
		a1->cur.flags &= ~MARK_1;
		if (a1>=addr1 && a1<=addr2 && execute(0, a1)==k)
			a1->cur.flags |= MARK_1;
	}
	/*
	 * Special case: g/.../d (avoid n^2 algorithm)
	 */
	if (globuf[0]=='d' && globuf[1]=='\n' && globuf[2]=='\0') {
		gdelete();
		return;
	}
	for (a1=zero; a1<=dol; a1++) {
		if (a1->cur.flags & MARK_1) {
			a1->cur.flags &= ~MARK_1;
			dot = a1;
			globp = globuf;
			globflg = 1;
			commands();
			globflg = 0;
			a1 = zero;
		}
	}
}

static void join(void)
{
	register unsigned char *gp, *lp;
	register LINE a1;

	if (addr1 == addr2) return;
	gp = genbuf;
	for (a1=addr1; a1<=addr2; a1++) {
		lp = getline(a1->cur);
		while (*gp = *lp++)
			if (gp++ >= &genbuf[LBSIZE-2])
				(void) error(27);
	}
	lp = linebuf;
	gp = genbuf;
	while (*lp++ = *gp++);
	addr1->cur = putline();
	if (addr1<addr2)
		rdelete(addr1+1, addr2);
	dot = addr1;
}

static void substitute(int inglob)
{
	register gsubf, nl;
	register LINE a1, saved_fendcore;
	POS *markp;
	int gf, n;

	gsubf = compsub();
	for (a1 = addr1; a1 <= addr2; a1++) {
		gf = n = 0;
		do {
			if (execute(gf++, a1) == 0)
				break;
			if (gsubf == GLOBAL_SUB || gsubf == gf) {
				n++;
				dosub();
			} else
				offset_base += pmatch[0].rm_eo;
			/* if matched null string, increment offset_base */
			if (pmatch[0].rm_so == pmatch[0].rm_eo)
				offset_base++;
		} while (*offset_base != '\0' && (gsubf == GLOBAL_SUB || gsubf > gf));
		if (n == 0)
			continue;
		inglob |= 01;
		subnewa = putline();
		subnewa.flags = a1->cur.flags;
		if (anymarks) {
			for (markp = names; markp < &names[26]; markp++)
				if ((markp->bno == a1->cur.bno) &&
				    (markp->off == a1->cur.off))
					*markp = subnewa;
		}
		a1->cur = subnewa;
		saved_fendcore = fendcore;
		append(getsub, a1);
		if (fendcore != saved_fendcore) {
			a1 += fendcore - saved_fendcore;
		}
		nl = nline;
		a1 += nl;
		addr2 += nl;
	}
	if (inglob==0)
		(void) error(35);
}

static short remem[LBSIZE] = {-1};

static int compsub(void)
{
	register c;
	register char *p;
	wchar_t	seof, wch;
	int n, i, len;
	char str[MB_LEN_MAX];

	if ((c = getchr()) == '\n' || c == ' ')
		(void) error(36);
	len = 1;
	str[0] = c;
	while (mbtowc(&seof, str, MB_CUR_MAX) != len) {
		if (++len > MB_CUR_MAX)
			(void) error(36);
		str[len-1] = getchr();
	}
	re_compile(seof);  /* Compile the Regular Expression	*/
	p = rhsbuf;
	for (;;) {
		len = 1;
		if ((c = getchr()) == '\\') {
				/* copy '\\' into p and read next char */
			*p++ = c;
			c = getchr();
			if (c >= preg.re_nsub + '1' && c < '9') 
				(void) error(25);
			str[0] = c;
			while (mblen(str, MB_CUR_MAX) != len) {
				if (++len > MB_CUR_MAX)
					(void) error(51);
				str[len-1] = getchr();
			}	
		} else if (c == '\n') {
			if (globp && globp[0]) {
				*p++ = '\\';
				str[0] = c;
			} else {
				peekc = c;
				pflag++;
				break;
			}
		} else {
			/* read in a multi-byte character */
			str[0] = c;
			while (mbtowc(&wch, str, MB_CUR_MAX) != len) {
				if (++len > MB_CUR_MAX)
					(void) error(51);
				str[len-1] = getchr();
			}	
			if (wch == seof)
				break;
		}
		for (i=0; i<len; i++)
			*p++ = str[i];
		if (p >= &rhsbuf[LBSIZE])
			(void) error(38);
	}
	*p++ = 0;
	if(rhsbuf[0] == '%' && rhsbuf[1] == 0)
		(remem[0] != -1) ? (void) strcpy(rhsbuf, (char *)remem) : (void) error(55);
	else
		(void) strcpy((char *)remem, rhsbuf);
	for (n=0; (c = getchr()) >= '0' && c <= '9';)
		n = n * 10 + c - '0';
	peekc = c;
	if (n == 0)
		if (peekc == 'g') {
			peekc = 0;
			n = GLOBAL_SUB;
		} else
			n = 1;
	newline();
	return(n);
}

static int getsub(void)
{
	register unsigned char *p1, *p2;

	p1 = linebuf;
	if ((p2 = linebp) == 0)
		return(EOF);
	while (*p1++ = *p2++);
	linebp = 0;
	return(0);
}

static void dosub(void)
{
	register unsigned char *lp, *sp, *rp;
	char 	c;
	int	len;

	lp = linebuf;
	sp = genbuf;
	rp = rhsbuf;

	while (lp < (offset_base + pmatch[0].rm_so))
		*sp++ = *lp++;
	while (c = *rp) {
		if (c == '&') {
			rp++;
			sp = place(sp, (offset_base + pmatch[0].rm_so), 
					(offset_base + pmatch[0].rm_eo));
			continue;
		} else if (c == '\\') { 
			c = *++rp;	/* discard '\' */
			if(c >= '1' && c < preg.re_nsub + '1') {
				rp++;
				sp = place(sp, (offset_base + pmatch[c-'0'].rm_so),
						(offset_base +  pmatch[c-'0'].rm_eo));
				continue;
			}
		}
		if ((len = mblen(rp, MB_CUR_MAX)) == -1 ) 
			(void) error(28);     /* illegal multi-byte character */
		while (len--) 
			*sp++ = *rp++;
		if (sp >= &genbuf[LBSIZE]) 
			(void) error(27);
	}
	lp = offset_base + pmatch[0].rm_eo;
	offset_base = sp - genbuf + linebuf;
	while (*sp++ = *lp++)
		if (sp >= &genbuf[LBSIZE]) 
			(void) error(27);
	lp = linebuf;
	sp = genbuf;
	while (*lp++ = *sp++);
}

static unsigned char *
place(register unsigned char *sp, register unsigned char *l1, register unsigned char *l2)
{

	while (l1 < l2) {
		*sp++ = *l1++;
		if (sp >= &genbuf[LBSIZE])
			(void) error(27);
	}
	return(sp);
}

static void move(int cflag)
{
	register LINE adt, ad1, ad2, saved_fendcore;
	int getcopy(void);

	setdot();
	nonzero();
	if ((adt = address())==0)
		(void) error(39);
	newline();
	if (!globflg) save();
	if (cflag) {
		ad1 = dol;
		saved_fendcore = fendcore;
		append(getcopy, ad1++);
		if (saved_fendcore != fendcore) {
			ad1 += fendcore - saved_fendcore;
			adt += fendcore - saved_fendcore;
		}
		ad2 = dol;
	} else {
		ad2 = addr2;
		for (ad1 = addr1; ad1 <= ad2;)
			(ad1++)->cur.flags &= ~MARK_1;
		ad1 = addr1;
	}
	ad2++;
	if (adt<ad1) {
		dot = adt + (ad2-ad1);
		if ((++adt)==ad1)
			return;
		reverse(adt, ad1);
		reverse(ad1, ad2);
		reverse(adt, ad2);
	} else if (adt >= ad2) {
		dot = adt++;
		reverse(ad1, ad2);
		reverse(ad2, adt);
		reverse(ad1, adt);
	} else
		(void) error(39);
	fchange = 1;
}

static void reverse(register LINE a1, register LINE a2)
{
	POS t;

	for (;;) {
		t = (--a2)->cur;
		if (a2 <= a1)
			return;
		a2->cur = a1->cur;
		(a1++)->cur = t;
	}
}

static int getcopy(void)
{

	if (addr1 > addr2)
		return(EOF);
	(void) getline((addr1++)->cur);
	return(0);
}


static void error1(int code)
{
	if (preg.re_comp != 0) {
		regfree(&preg);
		preg.re_comp = 0;
	}
	(void) error(code);
}

static int execute(int gf, LINE addr)
{
	register unsigned char *p1;
	int status;

	if (gf) {
		p1 = offset_base;
		status = regexec(&preg, p1, (preg.re_nsub + 1), pmatch, REG_NOTBOL); 
	} else {
		if (addr==zero)
			return(0);
		p1 = getline(addr->cur);
		status = regexec(&preg, p1, (preg.re_nsub + 1), pmatch, 0); 
		offset_base = linebuf;
	}
	if (status == 0)
		return(1);
	else
		return(0);
}


static void putd(void)
{
	register r;

	r = (int)(count%10);
	count /= 10;
	if (count)
		putd();
	putchr(r + '0');
}

static int puts(const char *s)
{
	wchar_t wp;
	char	*msg_ptr;		/* pointer to message text */
	register char *sp = (char *)s;
	int sz, i, ch_len;

	if (fss.Ffill && (listf == 0)) {
		if ((i = expnd(sp,funny,&sz,&fss)) == -1) {
			(void) write(1,funny,fss.Flim & 0377); putchr('\n');
			msg_ptr =  MSGSTR(M_EDTOOLONG,
			    "too long");
			(void) write(1, msg_ptr, strlen(msg_ptr));
		}
		else
			(void) write(1,funny,sz);
		putchr('\n');
		if (i == -2)
		{
			msg_ptr =  MSGSTR(M_EDTAB,
			    "tab count\n");
			(void) write(1, msg_ptr, strlen(msg_ptr));
		}
		return(0);
	}
	col = 0;
	while (*sp) {
		if (listf) {
			if ((ch_len = mbtowc(&wp, sp, MB_CUR_MAX)) < 1) 
				(void) error(28);	/* illegal character */
			else if (ch_len == 1)
				putchr(*sp++);
			else {
				sp += ch_len;
				putwchr(wp);
			}
		} else
			putchr(*sp++);
	}
	putchr('\n');
        return(1);
}

static char    line[70];
static char    *linp = line;

static void putchr(int ac)
{
	register char *lp;
	register c;
	short len;
	int chrwid;
	char tmp;
	wchar_t wch;

	lp = linp;
	c = ac;
	if (listf && c != '\n') {
		if (!isprint(c))
			chrwid = 4;
		else {
			tmp = c;
			(void) mbtowc(&wch, &tmp, MB_CUR_MAX);
			chrwid = wcwidth(wch);
			if (chrwid < 0)
				chrwid = 1;
		}
		if (col+chrwid >= 72) {
			col = 0;
			*lp++ = '\\';
			*lp++ = '\n';
		}
		col += chrwid;
		if (c == '\a') {
			*lp++ = '\\';
			*lp++ = 'a';
			col -= 2;
		} else if (c == '\b') {
			*lp++ = '\\';
			*lp++ = 'b';
			col -= 2;
		} else if (c == '\f') {
			*lp++ = '\\';
			*lp++ = 'f';
			col -= 2;
		} else if (c == '\r') {
			*lp++ = '\\';
			*lp++ = 'r';
			col -= 2;
		} else if (c == '\t') {
			*lp++ = '\\';
			*lp++ = 't';
			col -= 2;
		} else if (c == '\v') {
			*lp++ = '\\';
			*lp++ = 'v';
			col -= 2;
		} else if (c == '\\') {
			*lp++ = '\\';
			*lp++ = '\\';
			col -= 2;
		} else if (!isprint(c)) {
			(void) sprintf(lp, "\\%03o", c);
			lp += 4;
		} else
			*lp++ = c;
	} else if (listf && c == '\n') {
		*lp++ = '$';	/* XPG4 - lines must end with a $ */
		*lp++ = c;
	} else
		*lp++ = c;
	if(c == '\n' || lp >= &line[65]) {
		linp = line;
		len = lp - line;
		if(yflag & 01)
			(void) write(1, (char *) &len, sizeof(len));
		(void) write(1, line, len);
		return;
	}
	linp = lp;
}

static void putwchr(wchar_t ac)
{
	register char *lp;
	char	buf[MB_LEN_MAX], *p;
	wchar_t c;
	short len;

	lp = linp;
	c = ac;
	if (listf) {
		if (!iswprint(c)) {
			p = &buf[0];
			len = wctomb(p, c);
			while (len--) {
				if (col + 4 >= 72) {
					col = 0;
					*lp++ = '\\';
					*lp++ = '\n';
				}
				(void) sprintf(lp, "\\%03o", *p++);
				col += 4;
				lp += 4;
			}
		} else {
			len = wcwidth(c);
			if (len < 0)
				len = 1;
			if (col	+ len >= 72) {
				col = 0;
				*lp++ = '\\';
				*lp++ = '\n';
			}
			col += len;
			lp += wctomb(lp, c);
		}
	} else
		lp += wctomb(lp, c);
	if(lp >= &line[65]) {
		linp = line;
		len = lp - line;
		if(yflag & 01)
			(void) write(1, (char *) &len, sizeof(len));
		(void) write(1, line, len);
		return;
	}
	linp = lp;
}
 
#ifdef CRYPT
void crblock(char *permp, char *buf, int nchar, long startn)
{
	register char   *p1;
	register int n1, n2;
	register char   *t1, *t2, *t3;

	t1 = permp;
	t2 = &permp[256];
	t3 = &permp[IOBLKSIZE];

	n1 = (int)(startn&0377);
	n2 = (int)((startn>>8)&0377);
	p1 = buf;
	while(nchar--) {
		*p1 = t2[(t3[(t1[(*p1+n1)&0377]+n2)&0377]-n2)&0377]-n1;
		n1++;
		if(n1==256){
			n1 = 0;
			n2++;
			if(n2==256) n2 = 0;
		}
		p1++;
	}
}

void getkey(void)
{
	char	*msg_ptr;		/* pointer to message text */
	struct termio b;
	int save;


	/* int (*sig)(); changed to void for ANSI compatibility */
	int (*sig)();
	char *p;
	int c;

	sig = signal(SIGINT, SIG_IGN);
	ioctl(0, TCGETA, &b);
	save = b.c_lflag;
	b.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);
	ioctl(0, TCSETA, &b);
	msg_ptr =  MSGSTR(M_EDKEY,
		    "Enter file encryption key: ");
	(void) write(1, msg_ptr, strlen(msg_ptr));
	p = key;
	while(((c=getchr()) != EOF) && (c!='\n')) {
		if(p < &key[KSIZE])
			*p++ = c;
	}
	*p = 0;
	(void) write(1, "\n", 1);
	b.c_lflag = save;
	ioctl(0, TCSETA, &b);
	(void) signal(SIGINT, (void (*)(int))sig);
	/* this line used to say "return(key[0] != 0);" */
}

/*
 * Besides initializing the encryption machine, this routine
 * returns 0 if the key is null, and 1 if it is non-null.
 */
void crinit(char *keyp, char *permp)
{
	register char *t1, *t2, *t3;
	int ic, i, k, temp, pf[2];
	unsigned random;
	char buf[13];
	long seed;

	if (yflag)
		(void) error(59);
	t1 = permp;
	t2 = &permp[256];
	t3 = &permp[IOBLKSIZE];

	if (*keyp == 0)
		return(0);

	(void) strncpy(buf, keyp, 8);
	while (*keyp)
		*keyp++ = '\0';
	buf[8] = buf[0];
	buf[9] = buf[1];
	if(pipe(pf) < 0)
		(void) error(0);
	i = fork();
	if(i == -1)
		(void) error(23);
	if(i == 0) {
		(void) close(0);
		(void) close(1);
		(void) dup(pf[0]);
		(void) dup(pf[1]);
		(void) execl("/usr/ccs/bin/makekey", "-", (char *) 0, (char *) 0, (char *) 0);
		exit(2);
	}
	(void) write(pf[1], buf, 10);
	if (wait((int *) 0)== -1 || read(pf[0], buf, 13)!=13) {
		puts(MSGSTR(M_EDGENKEY,
		    "crypt: cannot generate key"));
		exit(2);
	}
	(void) close(pf[0]);
	(void) close(pf[1]);
	seed = 123;
	for (i=0; i<13; i++)
		seed = seed*buf[i] + i;
	for(i=0;i<256;i++) {
		t1[i] = i;
		t3[i] = 0;
	}
	for(i=0;i<256;i++) {
		seed = 5*seed + buf[i%13];
		random = (int)(seed % 65521);
		k = 256-1 -i;
		ic = (random&0377)%(k+1);
		random >>= 8;
		temp = t1[k];
		t1[k] = t1[ic];
		t1[ic] = temp;
		if(t3[k]!=0) continue;
		ic = (random&0377) % k;
		while(t3[ic]!=0) ic = (ic+1) % k;
		t3[k] = ic;
		t3[ic] = k;
	}
	for(i=0;i<256;i++)
		t2[t1[i]&0377] = i;
	return(1);
}

void makekey(char *a, char *b)
{
	register int i;
	long gorp;
	char temp[KSIZE + 1];

	for(i = 0; i < KSIZE; i++)
		temp[i] = *a++;
	time(&gorp);
	gorp += getpid();

	for(i = 0; i < 4; i++)
		temp[i] ^= (char)((gorp>>(8*i))&0377);

	i = crinit(temp, b);
}
#endif

static void globaln(int k)
{
	register char *gp;
	register c;
	register LINE a1;
	int  i, len, nfirst;
	wchar_t wch;
	char globuf[GBSIZE], str[MB_LEN_MAX];

	if (yflag)
		(void) error(59);
	if (globp)
		(void) error(33);
	setall();
	nonzero();
	if ((c=getchr())=='\n')
		(void) error(19);
	save();
	len = 1;
	str[0] = c;
	while (mbtowc(&wch, str, MB_CUR_MAX) != len) {
		if (++len > MB_CUR_MAX)
			(void) error(19);
		str[len-1] = getchr();
	}	
	re_compile(wch);	/* Compile the Regular Expression	*/
	for (a1=zero; a1<=dol; a1++) {
		a1->cur.flags &= ~MARK_1;
		if (a1>=addr1 && a1<=addr2 && execute(0, a1)==k)
			a1->cur.flags |= MARK_1;
	}
	nfirst = 0;
	newline();
	for (a1=zero; a1<=dol; a1++) {
		if (a1->cur.flags & MARK_1) {
			a1->cur.flags &= ~MARK_1;
			dot = a1;
			puts((char *)getline(a1->cur));
			if ((c=getchr()) == EOF)
				(void) error(52);
			if(c=='a' || c=='i' || c=='c')
				(void) error(53);
			if (c == '\n') {
				a1 = zero;
				continue;
			}
			if (c != '&') {
				gp = globuf;
				/* c is first byte of a multi-byte character */
				len = 1;
				str[0] = c;
				while (mblen(str, MB_CUR_MAX) != len) {
					if (++len > MB_CUR_MAX)
						(void) error(52);
					str[len-1] = getchr();
				}
				for (i=0; i<len; i++)
					*gp++ = str[i];
				while ((c = getchr()) != '\n') {
					if (c == '\\') {
						if ((c = getchr()) != '\n')
							*gp++ = '\\';
						else {
							*gp++ = c;
							if (gp >= &globuf[GBSIZE-2])
								(void) error(34);
							continue;
						}
					}
					/* c is first byte of a multi-byte character */
					len = 1;
					str[0] = c;
					while (mblen(str, MB_CUR_MAX) != len) {
						if (++len > MB_CUR_MAX)
							(void) error(52);
						str[len-1] = getchr();
					}
					for (i=0; i<len; i++)
						*gp++ = str[i];
					if (gp >= &globuf[GBSIZE-2])
						(void) error(34);
				}
				*gp++ = '\n';
				*gp++ = 0;
				nfirst = 1;
			}
			else
				if ((c=getchr()) != '\n')
					(void) error(54);
			globp = globuf;
			if (nfirst) {
				globflg = 1;
				commands();
				globflg = 0;
			}
			else (void) error(56);
			globp = 0;
			a1 = zero;
		}
	}
}

static int eopen(char *string, int rw)
{
#define w_or_r(a,b) (rw?a:b)
	int pf[2];
	int i;
	int fio;
	int chcount;    /* # of char read. */
#ifdef CRYPT
	int crflag;
	char *fp;

	crflag = 0;      /* Is file encrypted flag; 1=yes. */
#endif
	if (rflg) {     /* restricted shell */
		if (Xqt) {
			Xqt = 0;
			(void) error(6);
		}
	}
	if(!Xqt) {
		if((fio=open(string, rw)) >= 0) {
			if (fflg) {
				chcount = read(fio,funny,LBSIZE);
#ifdef CRYPT
/* Verify that line just read IS an encrypted file. */
/* This seems to be a heuristic; if the user specified -x on a file */
/* that is not encrypted, ed will politely ignore it.  Under NLS,   */
/* the file will be assumed to be encrypted if it contains NLS chars. */
/* Not knowing the properties of encryption it is hard to know a better */
/* way to do this analysis. */
			fp = funny; /* Set fp to start of buffer. */
			while(fp < &funny[chcount])
				if(*fp++ & 0200)crflag = 1;
/* If is is encrypted, & -x option was used, & key is not null, decode it. */
		if(crflag & xflag & kflag)crblock(perm,funny,chcount,0L);
#endif
				if (fspec(funny,&fss,0) < 0) {
					fss.Ffill = 0;
					fflg = 0;
					(void) error(4);
				}
				(void) lseek(fio,0L,0);
			}
		}
		fflg = 0;
		return(fio);
	}
	if(pipe(pf) < 0)
xerr:           (void) error(0);
	if((i = fork()) == 0) {
		(void) signal(SIGHUP, (void (*)(int))oldhup);
		(void) signal(SIGQUIT, (void (*)(int))oldquit);
		(void) signal(SIGPIPE, (void (*)(int))oldpipe);
		(void) signal(SIGINT, (void (*)(int))0);
		(void) close(w_or_r(pf[1], pf[0]));
		(void) close(w_or_r(0, 1));
		(void) dup(w_or_r(pf[0], pf[1]));
		(void) close(w_or_r(pf[0], pf[1]));
		(void) execlp("/usr/bin/sh", "sh", "-c", string, (char *) 0);
		exit(1);
	}
	if(i == -1)
		goto xerr;
	(void) close(w_or_r(pf[0], pf[1]));
	return w_or_r(pf[1], pf[0]);
}

static void eclose(int f1)
{
	(void) close(f1);
	if(Xqt){
		Xqt = 0; 
		(void) wait((int *) 0);
	}
}
static void mkfunny(void)
{
	register char *p, *p1, *p2;

	p2 = p1 = funny;
	p = file;
	while(*p)
		p++;
	while(*--p  == '/')     /* delete trailing slashes */
		*p = '\0';
	p = file;
	while (*p1++ = *p)
		if (*p++ == '/') p2 = p1;
	p1 = &tfname[10];
	*p2 = '\007';   /* add unprintable char to make funny a unique name */
	while (p1 <= &tfname[16])
		*++p2 = *p1++;
}

static void mktmpfunny(void)
{
	sprintf(funny,"/tmp/%c%s",7,&tfname[10]);
}

static void getime(void) /* get modified time of file and save */
{
	if (stat(file,&Fl) < 0)
		savtime = 0;
	else
		savtime = Fl.st_mtime;
}

static void chktime(void) /* check saved mod time against current mod time */
{
	if (savtime != 0 && Fl.st_mtime != 0) {
		if (savtime != Fl.st_mtime)
			(void) error(58);
	}
}

static void newtime(void) /* get new mod time and save */
{
	(void) stat(file,&Fl);
	savtime = Fl.st_mtime;
}

static void red(char *op) /* restricted - check for '/' in name */
	/* and delete trailing '/' */
{
	register char *p;

	p = op;
	while(*p)
		if(*p++ == '/' && strncmp(op, "/var/tmp/", 5) && rflg) {
			*op = 0;
			(void) error(6);
		}
	/* delete trailing '/' */
	while(p > op) {
		if (*--p == '/')
			*p = '\0';
		else break;
	}
}


static char *fsp, fsprtn;

static int fspec(char line3[], struct Fspec *f2, int up)
{
	struct termio arg;
	register int havespec, n;

	if(!up) clear(f2);

	havespec = fsprtn = 0;
	for(fsp=line3; *fsp && *fsp != '\n'; fsp++)
		switch(*fsp) {

			case '<':       if(havespec) return(-1);
					if(*(fsp+1) == ':') {
						havespec = 1;
						clear(f2);
						if(!ioctl(1, TCGETA, &arg) &&
							((arg.c_oflag&TAB3) == TAB3))
						  f2->Ffill = 1;
						fsp++;
						continue;
					}

			case ' ':       continue;

			case 's':       if(havespec && (n=numb()) >= 0)
						f2->Flim = n;
					continue;

			case 't':       if(havespec) targ(f2);
					continue;

			case 'd':       continue;

			case 'm':       if(havespec)  n = numb();
					continue;

			case 'e':       continue;
			case ':':       if(!havespec) continue;
					if(*(fsp+1) != '>') fsprtn = -1;
					return(fsprtn);

			default:        if(!havespec) continue;
					return(-1);
		}
	return(1);
}

static int numb(void)
{
	register int n;

	n = 0;
	while(*++fsp >= '0' && *fsp <= '9')
		n = 10*n + *fsp-'0';
	fsp--;
	return(n);
}


static void targ(struct Fspec *f3)
{


	if(*++fsp == '-') {
		if(*(fsp+1) >= '0' && *(fsp+1) <= '9') tincr(numb(),f3);
		else tstd(f3);
		return;
	}
	if(*fsp >= '0' && *fsp <= '9') {
		tlist(f3);
		return;
	}
	fsprtn = -1;
	fsp--;
	return;
}


static void tincr(int n, struct Fspec *f4)
{
	register int l, i;

	l = 1;
	for(i=0; i<20; i++)
		f4->Ftabs[i] = l += n;
	f4->Ftabs[i] = 0;
}


static void tstd(struct Fspec *f5)
{
	char std[3];

	std[0] = *++fsp;
	if (*(fsp+1) >= '0' && *(fsp+1) <= '9')  {
						std[1] = *++fsp;
						std[2] = '\0';
	}
	else std[1] = '\0';
	fsprtn = stdtab(std,f5->Ftabs);
	return;
}


static void tlist(struct Fspec *f6)
{
	register int n, last, i;

	fsp--;
	last = i = 0;

	do {
		if((n=numb()) <= last || i >= 20) {
			fsprtn = -1;
			return;
		}
		f6->Ftabs[i++] = last = n;
	} while(*++fsp == ',');

	f6->Ftabs[i] = 0;
	fsp--;
}


static int expnd(char line2[], char buf[], int *sz, struct Fspec *f7)
{
	register char *l, *t;
	register int b;

	l = line2 - 1;
	b = 1;
	t = f7->Ftabs;
	fsprtn = 0;

	while(*++l && *l != '\n' && b < 511) {
		if(*l == '\t') {
			while(*t && b >= *t) t++;
			if (*t == 0) fsprtn = -2;
			do buf[b-1] = ' '; while(++b < *t);
		}
		else buf[b++ - 1] = *l;
	}

	buf[b] = '\0';
	*sz = b;
	if(*l != '\0' && *l != '\n') {
		buf[b-1] = '\n';
		return(-1);
	}
	buf[b-1] = *l;
	if(f7->Flim && b-1 > f7->Flim) return(-1);
	return(fsprtn);
}



static void clear(struct Fspec *f8)
{
	f8->Ftabs[0] = f8->Fdel = f8->Fmov = f8->Ffill = 0;
	f8->Flim = 0;
}


static int lenchk(char line4[], struct Fspec *f9)
{
	register char *l, *t;
	register int b;

	l = line4 - 1;
	b = 1;
	t = f9->Ftabs;

	while(*++l && *l != '\n' && b < 511) {
		if(*l == '\t') {
			while(*t && b >= *t) t++;
			while(++b < *t);
		}
		else b++;
	}

	if((*l!='\0'&&*l!='\n') || (f9->Flim&&b-1>f9->Flim))
		return(-1);
	return(0);
}
#define NTABS 21

/*      stdtabs: standard tabs table
	format: option code letter(s), null, tabs, null */
static char stdtabs[] = {
'a',    0,1,10,16,36,72,0,                      /* IBM 370 Assembler */
'a','2',0,1,10,16,40,72,0,                      /* IBM Assembler alternative*/
'c',    0,1,8,12,16,20,55,0,                    /* COBOL, normal */
'c','2',0,1,6,10,14,49,0,                       /* COBOL, crunched*/
'c','3',0,1,6,10,14,18,22,26,30,34,38,42,46,50,54,58,62,67,0,
'f',    0,1,7,11,15,19,23,0,                    /* FORTRAN */
'p',    0,1,5,9,13,17,21,25,29,33,37,41,45,49,53,57,61,0, /* PL/I */
's',    0,1,10,55,0,                            /* SNOBOL */
'u',    0,1,12,20,44,0,                         /* UNIVAC ASM */
0};
/*      stdtab: return tab list for any "canned" tab option.
	entry: option points to null-terminated option string
		tabvect points to vector to be filled in
	exit: return(0) if legal, tabvect filled, ending with zero
		return(-1) if unknown option
*/

static int stdtab(char option[], char tabvect[NTABS])
{
	char *scan;
	int	n;

	tabvect[0] = 0;
	scan = stdtabs;
	while (*scan) {
		n = strcmp(scan, option);
		/* set scan to character after null */
		scan += (strlen(scan) + 1);
		if (n == 0) {
			(void) strcpy(tabvect, scan);
			break;
		} else 
			while(*scan++) ;    /* skip over tab specs */
	}
/*      later: look up code in /etc/something */
	return(tabvect[0]?0:-1);
}

/* This is called before a buffer modifying command so that the */
/* current array of line ptrs is saved in sav and dot and dol are saved */
static void save(void)
{
	LINE i;

	savdot = dot;
	savdol = dol;
	for (i=zero+1; i<=dol; i++)
		i->sav = i->cur;
	initflg = 0;
}

/* The undo command calls this to restore the previous ptr array sav */
/* and swap with cur - dot and dol are swapped also. This allows user to */
/* undo an undo */
static void undo(void) 
{
	POS tmp;
	LINE i, tmpdot, tmpdol;

	tmpdot = dot; dot = savdot; savdot = tmpdot;
	tmpdol = dol; dol = savdol; savdol = tmpdol;
	/* swap arrays using the greater of dol or savdol as upper limit */
	for (i=zero+1; i<=((dol>savdol) ? dol : savdol); i++) {
		tmp = i->cur;
		i->cur = i->sav;
		i->sav = tmp;
	}
}
