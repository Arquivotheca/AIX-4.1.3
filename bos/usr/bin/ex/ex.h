/* @(#)78	1.34  src/bos/usr/bin/ex/ex.h, cmdedit, bos41J, 9524G_all 6/12/95 18:30:17 */
/*
 * COMPONENT_NAME: (CMDEDIT) ex.h
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 3, 10, 13, 18, 26, 27
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
 * Copyright (c) 1981 Regents of the University of California 
 * 
 * (c) Copyright 1990 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.0
 */

#ifdef V6
#include <retrofit.h>
#endif

/*
 * Ex version 3 (see exact version in ex_cmds.c, search for /Version/)
 *
 * This file contains most of the declarations common to a large number
 * of routines.  The file ex_vis.h contains declarations
 * which are used only inside the screen editor.
 * The file ex_tune.h contains parameters which can be diddled per installation.
 *
 * The declarations relating to the argument list, regular expressions,
 * the temporary file data structure used by the editor
 * and the data describing terminals are each fairly substantial and
 * are kept in the files ex_{argv,re,temp,tty}.h which
 * we #include separately.
 *
 * If you are going to dig into ex, you should look at the outline of the
 * distribution of the code into files at the beginning of ex.c and ex_v.c.
 * Code which is similar to that of ed is lightly or undocumented in spots
 * (e.g. the regular expression code).  Newer code (e.g. open and visual)
 * is much more carefully documented, and still rough in spots.
 *
 * Please forward bug reports to
 *
 *      Computer Systems Research Group
 *      Computer Science Division, EECS
 *      EVANS HALL
 *      U.C. Berkeley 94720
 *      (415) 642-7780 (project office)
 *
 * or to 4bsd-bugs@Berkeley on the ARPA-net or ucbvax!4bsd-bugs on UUCP.
 * We would particularly like to hear of additional terminal descriptions
 * you add to the terminfo data base.
 */

#define _ILS_MACROS
#include <sys/param.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/times.h>
#include <fcntl.h>
#include <stdio.h>

#define is_dblwid(c)	(wcwidth(c) > 1 ? 1 : 0)

#include <errno.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/stat.h>

#define BUFFERSIZ (2*8192)
/* AIX security enhancement */
#if defined(TVI)
#include <sys/audit.h>
#include <sys/shm.h>
#include <sys/vfs.h>
#include <mon.h>
#endif
/* TCSEC Division C Class C2 */
#ifndef var
#define var     extern
#endif
/*
 *      The following little dance copes with the new USG tty handling.
 *      This stuff has the advantage of considerable flexibility, and
 *      the disadvantage of being incompatible with anything else.
 *      The presence of the symbol USG will indicate the new code:
 *      in this case, we define CBREAK (because we can simulate it exactly),
 *      but we won't actually use it, so we set it to a value that will
 *      probably blow the compilation if we goof up.
 */
/*
 *	Note to future maintainers of this file:
 *	vi does not, and never has, used curses. It uses the underlying
 *	database (terminfo or termcap) but not the high-level routines.
 *	This is primarily historical, but changing it will be time 
 *	consuming, and not necessarily of any benefit. DO NOT change this
 *	file to #include <curses.h>, directly or indirectly, unless you
 *	rip out the high-level routines and actually replace them with
 *	the routines from curses. The only reason this note exists is
 *	that some genius already tried this, and made a complete hash
 *	of this file and most of the others due to name conflicts. It
 *	also breaks "makeoptions". Horton & Joy were no fools, so don't
 *	second-guess them lightly.
 */
#ifdef USG
#include <termio.h>
typedef struct termio SGTTY;
#else
#include <sgtty.h>
typedef struct sgttyb SGTTY;
#endif

/* By defining SINGLE, we avoid dereferencing NULL pointers if
 * there is a problem initializing the terminfo information. This
 * is basically a fix, to deal with the sloppy port from termcap
 * to terminfo.
 */
/* #define SINGLE  Single is a flag in the Makefile */
#include <term.h>
/* #undef SINGLE   */

#if defined(USG) && !defined(CBREAK)
#define CBREAK xxxxx
#endif

extern  int errno;

typedef int     line;

#include "ex_tune.h"
#include "ex_vars.h"
/*
 * Options in the editor are referred to usually by "value(name)" where
 * name is all uppercase, i.e. "value(PROMPT)".  This is actually a macro
 * which expands to a fixed field in a static structure and so generates
 * very little code.  The offsets for the option names in the structure
 * are generated automagically from the structure initializing them in
 * ex_data.c... see the shell script "makeoptions".
 */
struct  option {
        char    *oname;
        char    *oabbrev;
        short   otype;          /* Types -- see below */
        int     odefault;       /* Default value */
        int     ovalue;         /* Current value */
        char    *osvalue;
};

/* The DUMMY constants can have any value */
#define DUMMY_INT 0
#define DUMMY_CHARP ""

#define ONOFF   0
#define NUMERIC 1
#define STRING  2               /* SHELL or DIRECTORY */
#define OTERM   3
#define ONECOL  4               /* need char that is 1 column wide on display */
#define WRAP    5               /* wraptype - word, col, rigid */
#define	CLOSEP	6		/* closing punctuation */
#define value(a)        options[a].ovalue
#define svalue(a)       options[a].osvalue
extern   struct option options[NOPTS + 1];
#ifdef TRACE
#       include <stdio.h>
        var     FILE    *trace;
        var     short    trubble;
        var     short    techoin;
        var     char    tracbuf[BUFFERSIZ];
#endif
#define SBUFSIZ 256     /* small buffer size */
#define WCSIZE(x)		(sizeof((x)) / sizeof(wchar_t))
/*
 * For improved performance, vi optimizes the updating of various terminals
 * that act differently for spaces, tabs and blanks.
 * See lengthy comment in ex_vput.c before vgotab().
 *
 * The QUOTE bit has been eradicated.
 *
 * We need a bunch of out-of-band values to represent what used to be the
 * QUOTE bit.  QUOTE_CTRL_D (^D) is used only in ex_vops2.c, QUOTE_BSP (^H)
 * gets passed through the putchar() routines to flush1(), QUOTE_NL (^J) and
 * QUOTE_CR (^M) are handled within the putchar() routines.  QUOTE_NUL and
 * QUOTE_SP are the only ones used by the pseudo-curses code.
 *
 * QUOTE_BULLET indicates a PARTIALCHAR indicator in the last display column.
 * The PARTIALCHAR feature replaces an is_dblwid(c) in the last
 * display column with a continuation indicator.  The double-wide character
 * is displayed on the next row.
 *
 * is_dblwid must indicate that QUOTE_BULLET and QUOTE_SP
 * display in 1 column.
 */
#define QUOTE_CTRL_D    0xfff0
#define QUOTE_BSP       0xfff1
#define QUOTE_NL        0xfff2
#define QUOTE_CR        0xfff3
#define QUOTE_NUL       0xfff4
#define QUOTE_SP        0xfff5
#define QUOTE_WCD       0xfff6  /* character displays in two columns */
#define QUOTE_BULLET    0xfff7	/* partial char character */
#define ESCAPE  '\033'
#define Ctrl(c) ((c) & 037)
#define ctlof(c)        ((c) ^ (wchar_t)0x40)

/*
 * Miscellaneous random variables used in more than one place
 */
var		wchar_t	WCemptystr[1];	/* a wide char null string */
char    origfile[FNSIZE];	/* holds name of originally edited file */
#if defined(_SECURITY)
char 	*aclp;			/* holds acl information */
/* Define ACL function prototypes */
int	acl_put(char *, char *, int);
int	acl_fput(int, char *, int);
int	acl_set(char *, int, int, int);
int	acl_fset(int, int, int, int);
char	*acl_get(char *);
char	*acl_fget(int);
#endif
/* AIX security enhancement */
#if defined(TVI)
var     short    trusted_input;  /* The trusted input global variable */
#endif
/* TCSEC Division C Class C2 */
var     short    aiflag;         /* Append/change/insert with autoindent */
var     short    anymarks;       /* We have used '[a-z] */
var     int     chng;           /* Warn "No write" */
var     short   cnt_set;        /* User specified a [count] to an ex command */
var     char    *Command;
var     short   defwind;        /* -w# change default window size */
var     int     dirtcnt;        /* When >= MAXDIRT, should sync temporary */
#ifdef SIGTSTP
var     short    dosusp;         /* Do SIGTSTP in visual when ^Z typed */
#endif
var     short    edited;         /* Current file is [Edited] */
var     line    *endcore;       /* pointer to (just beyond) end of allocated memory */
extern  short    endline;        /* Last cmd mode command ended with \n */
var     line    *fendcore;      /* First address in line pointer space */
var     char    file[FNSIZE];   /* Working file name */
var     wchar_t  genbuf[LBSIZE]; /* Working buffer when manipulating linebuf */
var     short    hush;           /* Command line option - was given, hush up! */
var     wchar_t  *globp;         /* (Untyped) input string to command mode */
var     short    holdcm;         /* Don't cursor address */
var     short    inappend;       /* in ex command append mode */
var     short    inglobal;       /* Inside g//... or v//... */
var     wchar_t  *initev;        /* Initial : escape for visual */
var     short    inopen;         /* Inside open or visual */
var     wchar_t  *input;         /* Current position in cmd line input buffer */
var     short    intty;          /* Input is a tty */
var     short   io;             /* General i/o unit (auto-closed on error!) */
extern  int     lastc;          /* Last character ret'd from cmd input */
var     short    laste;          /* Last command was an "e" (or "rec") */
var     wchar_t  lastmac;        /* Last macro called for ** */
var     wchar_t  lasttag[TAGSIZE];       /* Last argument to a tag command */
var     wchar_t  *linebp;        /* Used in substituting in \n */
var     wchar_t  linebuf[LBSIZE];        /* The primary line buffer */
var     int     listf;          /* Command should run in list mode */
var     wchar_t  *loc1;          /* Where re began to match (in linebuf) */
var     wchar_t  *loc2;          /* First char after re match (") */
var     line    names['z'-'a'+2];       /* Mark registers a-z,' */
var     int     notecnt;        /* Count for notify (to visual from cmd) */
var     char    obuf[BUFFERSIZ];   /* Buffer for tty output */
var     int     outfd;          /* output file descriptor */
var     short   oprompt;        /* Saved during source */
var     short   ospeed;         /* Output speed (from gtty) */
var     int     otchng;         /* Backup tchng to find changes in macros */
var     int     peekc;          /* Peek ahead character (cmd mode input) */
var     wchar_t  *pkill[2];      /* Trim for put with ragged (LISP) delete */
var     short    pfast;          /* Have stty -nl'ed to go faster */
var     int     pid;            /* Process id of child */
var     int     ppid;           /* Process id of parent (e.g. main ex proc) */
var     jmp_buf resetlab;       /* For error throws to top level (cmd mode) */
var     int     rpid;           /* Pid returned from wait() */
var     short    ruptible;       /* Interruptible is normal state */
var     short    seenprompt;     /* 1 if have gotten user input */
var     short    shudclob;       /* Have a prompt to clobber (e.g. on ^D) */
var	int	sigwinch_blocked;/* true if sigwinch is currently blocked. */
var     int     status;         /* Status returned from wait() */
var     int     tchng;          /* If nonzero, then [Modified] */
var	int	exit_status;	/* Exit Status */
/* AIX security enhancement */
#if defined(TVI)
extern  int     tfile;          /* Temporary file unit for shared memory version */
#else
extern  short   tfile;          /* Temporary file unit */
#endif
/* TCSEC Division C Class C2 */
var     short    vcatch;         /* Want to catch an error (open/visual) */
var     jmp_buf vreslab;        /* For error throws to a visual catch */
var     short    writing;        /* 1 if in middle of a file write */
var     int     xchng;          /* Suppresses multiple "No writes" in !cmd */
var	long	bsize;		/* Block size for disk i/o */

/*
 * Macros
 */
#define CP(a, b)        (void) wcscpy(a, b)
                        /*
                         * FIXUNDO: do we want to mung undo vars?
                         * Usually yes unless in a macro or global.
                         */
#define FIXUNDO         (inopen >= 0 && (inopen || !inglobal))
#define ckaw()          {if (chng && value(AUTOWRITE) && !value(READONLY)) wop((short)0);}
#define copy(to,from,n) (void)memcpy((char *)(to), (char *)(from), n) 
/* AIX security enhancement */
#if defined(TVI)
#define tvicopy(to,from,c,n) (void)memccpy((char *)(to), (char *)(from), c, n) 
#endif
/* TCSEC Division C Class C2 */
#define eq(a, b)        ((a) && (b) && strcmp(a, b) == 0)
#define WCeq(a, b)      ((a) && (b) && wcscmp(a, b) == 0)
#define getexit(a)      copy(a, resetlab, sizeof (jmp_buf))
#define lastchar()      lastc
#define outchar(c)      (*Outchar)(c)
#define pastwh()        (ignore(skipwh()))
#define pline(no)       (*Pline)(no)
#define reset()         longjmp(resetlab,1)
#define resexit(a)      copy(resetlab, a, sizeof (jmp_buf))
#define setexit()       setjmp(resetlab)
#define setlastchar(c)  lastc = c
#define ungetchar(c)    peekc = c

#define CATCH           vcatch = 1; if (setjmp(vreslab) == 0) {
#define ONERR           } else { vcatch = 0;
#define ENDCATCH        } vcatch = 0;

/*
 * Environment like memory
 */
var     char    altfile[FNSIZE];        /* Alternate file name */
extern  char    direct[ONMSZ];          /* Temp file goes here */
extern  char    shell[ONMSZ];           /* Copied to be settable */
extern	 char	ttytype[ONMSZ];		/* A long and pretty name */
/* buffer to exec shell commands */
var     char    uxb[(UXBSIZE + 2) * sizeof (wchar_t)];
/* nc_uxb for . repeat command because vglobp is read in getbr() */
var     wchar_t  nc_uxb[(UXBSIZE + 2)];
/*
 * The editor data structure for accessing the current file consists
 * of an incore array of pointers into the temporary file tfile.
 * Each pointer is 15 bits (the low bit is used by global) and is
 * padded with zeroes to make an index into the temp file where the
 * actual text of the line is stored.
 *
 * To effect undo, copies of affected lines are saved after the last
 * line considered to be in the buffer, between dol and unddol.
 * During an open or visual, which uses the command mode undo between
 * dol and unddol, a copy of the entire, pre-command buffer state
 * is saved between unddol and truedol.
 */
var     line    *addr1;                 /* First addressed line in a command */
var     line    *addr2;                 /* Second addressed line */
var     line    *dol;                   /* Last line in buffer */
var     line    *dot;                   /* Current line */
var     line    *one;                   /* First line */
var     line    *truedol;               /* End of all lines, including saves */
var     line    *unddol;                /* End of undo saved lines */
var     line    *zero;                  /* Points to empty slot before one */

/*
 * Undo information
 *
 * For most commands we save lines changed by salting them away between
 * dol and unddol before they are changed (i.e. we save the descriptors
 * into the temp file tfile which is never garbage collected).  The
 * lines put here go back after unddel, and to complete the undo
 * we delete the lines [undap1,undap2).
 *
 * Undoing a move is much easier and we treat this as a special case.
 * Similarly undoing a "put" is a special case for although there
 * are lines saved between dol and unddol we don't stick these back
 * into the buffer.
 */
var     short   undkind;
var     line    *unddel;        /* Saved deleted lines go after here */
var     line    *undap1;        /* Beginning of new lines */
var     line    *undap2;        /* New lines end before undap2 */
var     line    *undadot;       /* If we saved all lines, dot reverts here */

#define UNDCHANGE       0
#define UNDMOVE         1
#define UNDALL          2
#define UNDNONE         3
#define UNDPUT          4
/*
 * Function type definitions
 */
#define NOWCSTR   (wchar_t *) 0
#define NOLINE  (line *) 0
/* 
 * Function prototypes
 */

#if defined(FLOCKFILE)
int  flock(int, int);
#endif
void vmoveitup(int, short);
void undvis(void);
void setoutt(void);
void listchar(wchar_t);
void numbline(int);
void normline(void);
void ex_putchar(int);
void termchar(int);
void flush(void);
void flush1(void);
void fgoto(void);
void gotab(int);
void noteinp(void);
void termreset(void);
void draino(void);
void flusho(void);
void putch(int);
void putpad(char *); 
void setout(void);
void lprintf(char *, char *);
void putNFL(void);
void pstart(void);
void pstop(void);
void tostart(void);
void tostop(void);
void vcook(void);
void vraw(void);
void gTTY(int);
void noonl(void);
void global(short);
int substitute (int);
int compsub(int);
void comprhs(int);
int getsub(void);
int dosubcon(short, line *);
int confirmed(line *);
void ugo(int, int);
void dosub(void);
int fixcase(int);
wchar_t *place(register wchar_t *, register wchar_t *, register wchar_t *);
void snote(int, int);
int compile(int, int);
int same(int, int);
int execute(int, line *);
int advance(wchar_t *, wchar_t *, int, int);
wchar_t colval(wchar_t);
int cclass(wchar_t *, wchar_t *, int, short *);
void set(void);
int setend(void);
int any(int, char *);
int anycp(wchar_t *, wchar_t *);
int backtab(int);
void change(void);
int column(wchar_t *);
void comment(void);
void copyw(line *, line *, int);
void copywR(line *, line *, int);
void dingdong(void);
int fixindent(int);
void filioerr(char *);
wchar_t *genindent(int);
void getDOT(void);
line * getmark(int);
void ignnEOF(void);
int iswhite(int);
int junk(int);
void killed(void);
wchar_t * WCstrend(wchar_t *);
char * strend(char *);
void strcLIN(wchar_t *);
void syserror(int);
int tabcol(int, int);
wchar_t * vfindcol(int);
wchar_t *vpastwh(wchar_t *);
int whitecnt(wchar_t *);
void Ignore(char *);
void Ignorf(int (*)());
void markit(line *);
void onhup(void);
void oncore(int);
void onintr(void);
void setrupt(void); 
int preserve(void);
void onsusp(void);
void killcnt(int);
int lineno(line *);
int lineDOL(void);
int lineDOT(void);
void markDOT(void);
void markpr(line *);
int markreg(int);
char * mesg(char *);
void merror(char *, int);
void mwarn(char *, int);
int morelines(void);
void nonzero(void);
int notable(int);
void notempty(void);
void netchHAD(int);
void netchange(int);
void putmark(line *);
int printwid(wchar_t, int);
void putmkl(line *, int);
int qcolumn(wchar_t *, wchar_t *);
int qdistance(wchar_t *, wchar_t *);
void reverse(line *, line *);
void save(line *, line *);
void save12(void);
void saveall(void);
int span(void);
void ex_sync(void);
int skipwh(void);
void smerror(char *, char *);
void fileinit(void);
void cleanup(short);
void getline(line);
line putline(void);
void tlaste(void);
void tflush(void);
void synctmp(void);
void TSYNC(void);
void regio(short, int(*)(int, char *, unsigned int));
void putreg(wchar_t);
int partreg(wchar_t);
void notpart(wchar_t);
void YANKreg(wchar_t);
void YANKline(void);
void regbuf(wchar_t, wchar_t *, int);
void gettmode(void);
void setterm(char *);
void setsize(void);
char * fkey(int);
int cost(char *);
void countnum(char);
void unix0(short);
void filter(int);
void recover(void);
void waitfor(void);
void revocer(void); 
void oop(void);
void vop(void);
void fixzero(void);
void savevis(void);
void vintr(int);
void vsetsiz(int);
void winchk(int);
void vopen(line *, int);
int vreopen(int, int, int);
int vglitchup(int, int);
void vinslin(int, int, int);
void vrollup(int);
void vup(int, int, short);
void moveitup(int, short);
void vscrap(void);
void repaint(wchar_t *);
void vredraw(int);
void vsyncCL(void);
void vsync(int);
void vsyncl(int);
void vreplace(int, int, int);
void sethard(void);
void vdirty(int, int);
void putnl(void);
void putmk1(register line *, int);
void vup1(void);
int ex_printf(char *, ...);

void	init(void);
char	*tailpath(register char *);
int	iownit(char *);
void	setdot(void);
void	setdot1(void);
void	setcount(void);
int	getnum(void);
void	setall(void);
void	setnoaddr(void);
line	*address(wchar_t *);
void	setCNL(void);
void	setNAEOL(void);

void	commands(short, short);

int	cmdreg(void);
int	endcmd(int);
void	eol(void);
void	error(register char *, int);
void	message(register char *, int);
void	erewind(void);
void	fixol(void);
int	exclam(void);
void	makargs(void);
void	next(void);
void	donewline(void);
void	nomore(void);
int	quickly(void);
void	setflav(void);
void	resetflav(void);
void	serror(char *, char *);
void	setflow(void);
int	skipend(void);
int	skipeol(void);
void	tailspec(wchar_t);
void	tail(wchar_t *);
void	tail2of(wchar_t *);
void	tailprim(register wchar_t *, int, short);
void	vcontin(short);
void	vnfl(void);

int	append(int (*)(void), line *);
void	appendnone(void);
void	pargs(void);
void	delete(short);
void	deletenone(void);
void	squish(void);
void	join(int);
void	move(void);
void	move1(int, int, line *);
int	getcopy(void);
int	getput(void);
void	put(void);
void	pragged(short);
void	shift(int, int);
void	tagfind(short);
void	yank(void);
void	zop(int);
void	zop2(register int, register int);
void	plines(line *, register line *, short);
void	pofix(void);
void	undo(short);
void	somechange(void);
void	mapcmd(int, int);
void	cmdmac(char);

void	ignchar(void);
int	ex_getchar(void);
int	getcd(void);
int	peekchar(void);
int	peekcd(void);
int	gettty(void);
int	smunch(register int, wchar_t *);
line	*setin(line *);

void	filename(int);
int	getargs(void);
int	gscan(void);
void	getone(void);
void	rop(int, int);
void	rop2(void);
void	rop3(int);
int	samei(struct stat *, char *);
void	wop(short);
int	edfile(void);
int	getfile(void);
void	putfile(int);
void	source(char *, short);
void	clrstats(void);

int lindent(line *);
void fixech(void);
void vclrech(short);
void vclear(void);
void ungetkey(int);
int noteit(short);
void macpush(wchar_t *, int);
void visdump(char *);
void vudump(char *);
int vgetcnt(void);
int getkey(void);
void cancelalarm(void);
int peekbr(void);
int getesc(void);
int peekkey(void);
int readecho(wchar_t );
void setLAST(void);
void addtext(wchar_t *);
void setBUF(wchar_t *, short *);
void vmain(void);
void prepapp(void);
void vremote(int,  void (*)(int), int);
void vsave(void);
void pp_insert(int,int, int);
void operate(wchar_t, int);
int word(void (*)(void), int);
void eend(void (*)(void));
int wordof(wchar_t, wchar_t *);
int wordch(wchar_t *);
void vUndo(void);
void vshftop(void);
void vfilter(void);
void vrep(int);
void vundo(short);
void vmacchng(short);
void vnoapp(void);
void voOpen(int, int);
void bleep(int, wchar_t *);
void vdoappend(wchar_t *);
int vdcMID(void);
void takeout(wchar_t *, short *);
int ateopr(void);
void vappend(int, int, int);
int back1(void);
wchar_t * vgetline(int, wchar_t *, short *, wchar_t);
int lfind(short, int, void (*)(void), line *);
int lmatchp(line *);
void lsmatch(wchar_t *);
int lnext(void);
int lbrack(int, void (*)(void));
void vnline(wchar_t *);
void vdown(int, int, short);
void vcontext(line *, wchar_t);
void vclean(void);
void vshow(line *, line *);
void vshow(line *, line *);
void vroll(int);
void vjumpto(line *, wchar_t *, wchar_t);
void vigoto(int, int);
void vclrcp(wchar_t *, int);
void endim(void);
void tfixnl(void);
void tracec(wchar_t);
void vclreol(void);
void vsetcurs(wchar_t *);
void vcsync(void);
void vgotoCL(int);
void vgoto(int, int);
void goim(void);
void vupdown(int, wchar_t *);
int vdepth(void);
void vsync1(register int);
void vprepins(void);
void vfixcurs(void);
void vcursbef(wchar_t *);
void vcursat(wchar_t *);
void vcursaft(wchar_t *);
void setDEL(void);
void vshowmode(char *);
void vrepaint(wchar_t *);
void physdc(int, int);
void tvliny(void);
void vclrlin(int, line *);
void vmoveto(line *, wchar_t *, wchar_t);

extern  void     (*Outchar)(int);
extern  void     (*Pline)(int);
extern  void     (*Putchar)(int);
var     void     (*oldhup)(int);
void     (*setlist(int))(int);
void     (*setnumb(int))(int);
char	*longname(void);
/* char    *memcpy(); removed for ansi compliance */

#include <string.h>
char *index(const char *, int);

line    *vback(line *, int);
wchar_t  *vskipwh(wchar_t *);
var     void     (*oldquit)(int);
void    vputch(int);
char *tparm(char *, int, ...);

#define  ignore(a)      (void) a
#       define  ignorf(a)       a
#include "ex_msg.h"
extern     nl_catd catd;
#define MSGSTR(id,ds)      catgets(catd, MS_EX, id, ds)



/* AIX security enhancement */
#if defined(TVI)
void	tvierror(char *, char *);
#endif
/* TCSEC Division C Class C2 */

/* prototypes for NC functions */


/*
 * If we have true Posix, then lets use it for the tty stuff
 */
#ifdef _POSIX_SOURCE
#include <unistd.h>
#ifdef TIOCLGET
#undef TIOCLGET
#endif
#ifdef TIOCSETC
#undef TIOCSETC
#endif
#endif /* _POSIX_SOURCE */

/* The maximum colation value which can be returned from the wcsxfrm() */
/* routine at this time for a single character.   Though very unlikely */
/* this value may need to be increased in the future. -Defect 68702/77628 */
#define MAXCOLVAL 254
