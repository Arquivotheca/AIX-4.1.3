/* @(#)41	1.26  src/bos/usr/bin/csh/sh.h, cmdcsh, bos411, 9428A410j 5/4/94 14:19:10 */
/*
 * COMPONENT_NAME: CMDCSH  c shell(csh)
 *
 * FUNCTIONS:
 *
 * ORIGINS:  10,26,27,18,71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1994
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
 *
 *
 * Bill Joy, UC Berkeley
 * October, 1978; May 1980
 *
 * Jim Kulp, IIASA, Laxenburg Austria
 * April, 1980
 */

#define _ILS_MACROS
typedef	char bool;

#ifdef BSD_LINE_DISC
#include <sgtty.h>
#else
#include <termio.h>
#include <termios.h>
#endif

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <dirent.h>
#include <sys/param.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/times.h>
#include "local.h"
#include "csh_msg.h"
#include <ctype.h>

extern nl_catd catd;

#define MSGSTR(num,str) catgets(catd,MS_CSH,num,str)   
#define	EQ(a, b)	(strcmp(a, b) == 0)

int mb_cur_max;

wctype_t iswblank_handle;
#define iswblank(c) iswctype(c,iswblank_handle)


/* Global flags */

/* added from CMU csh for command-line expansion & editing */
#ifdef CMDEDIT
bool    cmdedit;                /* Use cmu_tenex routine for input */
#endif

bool	chkstop;		/* Warned of stopped jobs... allow exit */
bool	didcch;			/* Have closed unused fd's for child */
bool	didfds;			/* Have setup i/o fd's for child */
bool	doneinp;		/* EOF indicator after reset from readc */
bool	exiterr;		/* Exit if error or non-zero exit status */
bool	child;			/* Child shell ... errors cause exit */
bool	haderr;			/* Reset was because of an error */
bool	intty;			/* Input is a tty */
bool	intact;			/* We are interactive... therefore prompt */
bool	justpr;			/* Just print because of :p hist mod */
bool	loginsh;		/* We are a loginsh -> .login/.logout */
bool	neednote;		/* Need to pnotify() */
bool	noexec;			/* Don't execute, just syntax check */
bool	pjobs;			/* want to print jobs if interrupted */
bool	setintr;		/* Set interrupts on/off -> Wait intr... */
bool	timflg;			/* Time the next waited for command */
bool	filec;			/* Doing filename expansion */

/* Global i/o info */
uchar_t	*arginp;		/* Argument input for sh -c and internal `xx` */
int	onelflg;		/* 2 -> need line for -t, 1 -> exit on read */
uchar_t	*file;			/* Name of shell file for $0 */

uchar_t	*err;			/* Error message from scanner/parser */
uchar_t	*shtemp;		/* Temp name for << shell files in /tmp */
struct timeval	time0;		/* Time at which the shell started */

struct rusage ru0;

/* Miscellany */
uchar_t	*doldol;		/* Character pid for $$ */
uid_t	uid;			/* Invokers uid */
time_t	chktim;			/* Time mail last checked */
pid_t	shpgrp;			/* Pgrp of shell */

/* If tpgrp is -1, leave tty alone! */
pid_t	tpgrp;			/* Terminal process group */

pid_t	opgrp;			/* Initial pgrp and tty pgrp */
int     oldisc;                 /* Initial line discipline or -1 */
struct tms shtimes;		/* shell and child times for process timing */

#ifdef CMDEDIT
uchar_t    *ShellTypeAheadToTenex; /* Read-ahead buffer for cmu_tenex () */
#endif

/*
 * These are declared here because they want to be
 * initialized in init.c (to allow them to be made readonly)
 */

extern struct mesg {
        uchar_t    *iname;         /* name from /usr/include */
	int        mesgno;
        uchar_t    *pname;         /* print name */
} mesg[];

struct biltins {
	uchar_t	*bname;
	void	(*bfunct)();
	int	minargs, maxargs;
};
extern struct biltins bfunc[];

struct srch {
	uchar_t	*s_name;
	int	s_value;
};
extern struct srch srchn[];

/*
 * To be able to redirect i/o for builtins easily, the shell moves the i/o
 * descriptors it uses away from 0,1,2.
 * Ideally these should be in units which are closed across exec's
 * (this saves work) but for version 6, this is not usually possible.
 * The desired initial values for these descriptors are defined in
 * local.h.
 */
int	SHIN;			/* Current shell input (script) */
int	SHOUT;			/* Shell output */
int	SHDIAG;			/* Diagnostic output... shell errs go here */
int	OLDSTD;			/* Old standard input (def for cmds) */

/*
 * Error control.
 *
 * Errors in scanning and parsing set up an error message to be printed
 * at the end and complete.  Other errors always cause a reset.
 * Because of source commands and .cshrc we need nested error catches.
 */

jmp_buf	reslab;

#define	setexit()	sigsetjmp(reslab, 0)
#define	reset()		siglongjmp(reslab, 0)

/* Should use structure assignment here */
#define	getexit(a)	copy((uchar_t *)(a), (uchar_t *)reslab, sizeof(reslab))
#define	resexit(a)	 copy((uchar_t *)reslab, ((uchar_t *)(a)), sizeof(reslab))	

uchar_t	*gointr;			/* Label for an onintr transfer */
void	(*parintr)(int);		/* Parents interrupt catch */
void	(*parterm)(int);		/* Parents terminate catch */

/*
 * Lexical definitions.
 *
 * All lexical space is allocated dynamically.
 * The eighth bit of characters is used to prevent recognition,
 * and eventually stripped.
 */
#define	QUOTE 	0200		/* Eighth char bit used internally for 'ing */
#define	TRIM	0177		/* Mask to strip quote bit */

/*
 * Each level of input has a buffered input structure.
 * There are one or more blocks of buffered input for each level,
 * exactly one if the input is seekable and tell is available.
 * In other cases, the shell buffers enough blocks to keep all loops
 * in the buffer.
 */
struct Bin {
	off_t	Bfseekp;		/* Seek pointer */
	off_t	Bfbobp;			/* Seekp of beginning of buffers */
	off_t	Bfeobp;			/* Seekp of end of buffers */
	int	Bfblocks;		/* Number of buffer blocks */
	uchar_t	**Bfbuf;		/* The array of buffer blocks */
} B;

#define	fseekp	B.Bfseekp
#define	fbobp	B.Bfbobp
#define	feobp	B.Bfeobp
#define	fblocks	B.Bfblocks
#define	fbuf	B.Bfbuf

off_t	btell();

/*
 * The shell finds commands in loops by reseeking the input
 * For whiles, in particular, it reseeks to the beginning of the
 * line the while was on; hence the while placement restrictions.
 */
off_t	lineloc;

/*
 * Input lines are parsed into doubly linked circular
 * lists of words of the following form.
 */
struct wordent {
	uchar_t	*word;
	struct wordent *prev;
	struct wordent *next;
};

/*
 * During word building, both in the initial lexical phase and
 * when expanding $ variable substitutions, expansion by `!' and `$'
 * must be inhibited when reading ahead in routines which are themselves
 * processing `!' and `$' expansion or after characters such as `\' or in
 * quotations.  The following flags are passed to the getC routines
 * telling them which of these substitutions are appropriate for the
 * next character to be returned.
 */
#define	DODOL	1
#define	DOEXCL	2
#define	DOALL	(DODOL|DOEXCL)

/*
 * Labuf implements a general buffer for lookahead during lexical operations.
 * Text which is to be placed in the input stream can be stuck here.
 * We stick parsed ahead $ constructs during initial input,
 * process id's from `$$', and modified variable values (from qualifiers
 * during expansion in dol.c) here.
 */
uchar_t	labuf[BUFR_SIZ];

uchar_t	*lap;

/*
 * Parser structure
 *
 * Each command is parsed to a tree of command structures and
 * flags are set bottom up during this process, to be propagated down
 * as needed during the semantics/exeuction pass (sem.c).
 */
struct command {
	int	t_dtyp;				/* Type of node */
	int	t_dflg;				/* Flags, e.g. FAND|... */
	union {
		uchar_t	*T_dlef;		/* Input redirect word */
		struct	command *T_dcar;	/* Left part of list/pipe */
	} L;
	union {
		uchar_t	*T_drit;		/* Output redirect word */
		struct	command *T_dcdr;	/* Right part of list/pipe */
	} R;
	uchar_t	**t_dcom;			/* Command/argument vector */
	struct	command *t_dspr;		/* Pointer to ()'d subtree */
	int	t_nice;
};
#define	t_dlef	L.T_dlef
#define	t_dcar	L.T_dcar
#define	t_drit	R.T_drit
#define	t_dcdr	R.T_dcdr

#define	TCOM	1		/* t_dcom <t_dlef >t_drit	*/
#define	TPAR	2		/* ( t_dspr ) <t_dlef >t_drit	*/
#define	TFIL	3		/* t_dlef | t_drit		*/
#define	TLST	4		/* t_dlef ; t_drit		*/
#define	TOR	5		/* t_dlef || t_drit		*/
#define	TAND	6		/* t_dlef && t_drit		*/

#define	FSAVE	(FNICE|FTIME|FNOHUP)	/* save these when re-doing */

#define	FAND	(1<<0)		/* executes in background	*/
#define	FCAT	(1<<1)		/* output is redirected >>	*/
#define	FPIN	(1<<2)		/* input is a pipe		*/
#define	FPOU	(1<<3)		/* output is a pipe		*/
#define	FPAR	(1<<4)		/* don't fork, last ()ized cmd	*/
#define	FINT	(1<<5)		/* should be immune from intr's */
/* spare */
#define	FDIAG	(1<<7)		/* redirect unit 2 with unit 1	*/
#define	FANY	(1<<8)		/* output was !			*/
#define	FHERE	(1<<9)		/* input redirection is <<	*/
#define	FREDO	(1<<10)		/* reexec aft if, repeat,...	*/
#define	FNICE	(1<<11)		/* t_nice is meaningful */
#define	FNOHUP	(1<<12)		/* nohup this command */
#define	FTIME	(1<<13)		/* time this command */

/*
 * Structure defining the existing while/foreach loops at this
 * source level.  Loops are implemented by seeking back in the
 * input.  For foreach (fe), the word list is attached here.
 */
struct whyle {
	off_t	w_start;		/* Point to restart loop */
	off_t	w_end;			/* End of loop (0 if unknown) */
	uchar_t	**w_fe, **w_fe0;	/* Current/initial wordlist for fe */
	uchar_t	*w_fename;		/* Name for fe */
	struct	whyle *w_next;		/* Next (more outer) loop */
} *whyles;

/*
 * Variable structure
 *
 * Lists of aliases and variables are sorted alphabetically by name
 */
struct varent {
	uchar_t	**vec;		/* Array of words which is the value */
	uchar_t	*name;		/* Name of variable/alias */
	struct  varent *link;
} shvhed, aliases;

/*
 * The following are for interfacing redo substitution in
 * aliases to the lexical routines.
 */
struct	wordent *alhistp;		/* Argument list (first) */
struct	wordent *alhistt;		/* Node after last in arg list */
uchar_t	**alvec;			/* The (remnants of) alias vector */

/* Filename/command name expansion variables */
int	gflag;				/* After tglob -> is globbing needed? */

/*
 * A reasonable limit on number of arguments would seem to be
 * the maximum number of characters in an arg list / 6.
 */
#define	GAVSIZ	NCARGS / 6

/* Variables for filename expansion */
uchar_t	**gargv;			/* Pointer to the (stack) arglist */
int	gargc;				/* Number args in gargv */
int	gnleft;

/* Variables for command expansion.  */
uchar_t	**pargv;			/* Pointer to the argv list space */
uchar_t	*pargs;				/* Pointer to start current word */
int	pargc;				/* Count of arguments in pargv */
int	pnleft;				/* Number of chars left in pargs */
uchar_t	*pargcp;			/* Current index into pargs */

/* Max size for env variable */
#define MAX_ENV_NAMLEN	(40*MB_LEN_MAX)

/* Max login name for ~foo use */
#define MAX_LOG_NAMLEN	MAX_ENV_NAMLEN

/*
 * History list
 *
 * Each history list entry contains an embedded wordlist
 * from the scanner, a number for the event, and a reference count
 * to aid in discarding old entries.
 *
 * Essentially "invisible" entries are put on the history list
 * when history substitution includes modifiers, and thrown away
 * at the next discarding since their event numbers are very negative.
 */
struct Hist {
	struct	wordent Hlex;
	int	Hnum;
	int	Href;
	struct	Hist *Hnext;
} Histlist;

struct	wordent	paraml;			/* Current lexical word list */
int	eventno;			/* Next events number */
int	lastev;				/* Last event reference (default) */

extern  wchar_t    HIST;                   /* history invocation character */
extern  wchar_t    HISTSUB;                /* auto-substitute character */
extern	int	   FSHMSG;

#define	NOSTR	((uchar_t *) 0)

/* setname is a macro to save space (see err.c) */
uchar_t	*bname;
#define	setname(a)	(bname = (a))

uchar_t	**evalvec;
uchar_t	*evalp;

/* NLQUOTE is an arbitrary escape byte, but NLQUOTE & QUOTE must be zero 
 * The current design of the csh will fail if NLQUOTE is entered as a command.
 * Thus if NLQUOTE is entered as input, it is quoted with NLQUOTE.
 * ALIASCHR and ALIASSTR are arbitrary (0200 can't be used for NLS since its
 * a valid character
 */
#define NLQUOTE '\001'
#define ALIASCHR '\002'
#define ALIASSTR "\002"

#define PUTCH(p,c)					\
	{						\
	int fclen;					\
	fclen = wctomb((char *)p,(wchar_t)c);		\
	if (fclen < 0)					\
		*p++ = c;				\
	else						\
		p += fclen;				\
	}

#define PUTSTR(a,b)					\
	{						\
	int fclen;					\
	fclen = mblen(((char *)b),(mb_cur_max));	\
	do						\
		*a++ = *b++;				\
	while (--fclen > 0);				\
	}



/* commands for file searching (see file.c) */
typedef enum {LIST, RECOGNIZE} COMMAND;


/* Macro to check the return value on the ioctl. */
#ifdef _IOCTL_DEBUG
#define	_IOCTL_PRINTF(number)	printf("csh:ioctl number %s\n",number);
#else
#define _IOCTL_PRINTF(number)
#endif

#define IOCTL(descriptor, request, request_ptr, number)			\
{									\
	int ioerror;							\
									\
	ioerror = ioctl(((int)descriptor), ((unsigned long)request), 	\
			((char *)request_ptr));				\
	if(ioerror < 0) {						\
		perror("csh:ioctl error:");				\
		_IOCTL_PRINTF(number)					\
	}								\
}

/* Marco to put a character to the output buffer.  */
#define PUT(p)								\
{									\
	int num[5], j, k, n, number;					\
									\
	number = p;							\
	for(k=0; number!=0; k++) {					\
		num[k] = (number % 10);					\
		number /= 10;						\
	}								\
	for(j=k-1, k=0; k<j ;k++,j--) {					\
		n = num[k];						\
		num[k] = num[j];					\
		num[j] = n;						\
	}								\
	if (k == 1) {							\
		display_char(num[0]);					\
	}								\
	else {								\
		for(n=0 ;n<k ;n++)					\
			display_char(num[n]);				\
	}								\
}

char	buffer[128];	/* This buffer used for line input to count
			   total bytes from the command line. */
/* Nice value for shell.  */
#define NICE	4

/*
 * This is the structure for each process the shell knows about:
 *     -allocated and filled in by proc:palloc.
 *     -flushed by proc:pflush. Freeing always happens at top level
 *      so the interrupt level has less to worry about.
 *     -processes are related to "friends" when in a pipeline.
 *      p_friends links makes a circular list of such jobs.
 */

struct process	{
	struct	process *p_next;	/* next in global "proclist" */
	struct	process	*p_friends;	/* next in job list (or self) */
	struct  process *p_pipeAnchor;  /* pipeline anchor (1st job, or self) */
	struct	directory *p_cwd;	/* cwd of the job (only in head) */
	unsigned short p_flags;		/* various job status flags */
	char	p_reason;		/* reason for entering this state */
	char	p_index;		/* shorthand job index */
	pid_t	p_pid;
	pid_t	p_jobid;		/* pid of job leader */
	/* if a job is stopped/background p_jobid gives its pgrp */
	struct timeval	p_btime;	/* begin time */
	struct timeval	p_etime;	/* end time */
	struct rusage p_rusage;
	uchar_t	*p_command;		/* first PMAXLEN uchar_ts of command */
};


/* Prototypes for csh. (dir.c) */
extern void			dinit(uchar_t *);
extern void			dodirs(uchar_t **);
extern void			dtildepr(uchar_t *, uchar_t *);
extern void			dochngd(uchar_t **);
extern uchar_t *		dfollow(uchar_t *);
extern void			dopushd(uchar_t **);
extern struct directory *	dfind(uchar_t *);
extern void			dopopd(uchar_t **);
extern void			dfree(struct directory *);
extern uchar_t *		dcanon(uchar_t *, uchar_t *);
extern void			dnewcwd(struct directory *);

/* dol.c */
extern void			Dfix(struct command *);
extern uchar_t *		Dfix1(uchar_t *);
extern void			Dfix2(uchar_t **);
extern int			Dword(void);
extern int			DgetC(int);
extern void			Dgetdol(void);
extern void			setDolp(uchar_t *);
extern void			unDredc(int);
extern int			Dredc(void);
extern int			Dtest(uchar_t);
extern int			Dtestq(uchar_t);
extern void			heredoc(uchar_t *);

/* err.c */
extern void			error(char *);
extern void			Perror(char *);
extern void			Perror_free(char *);
extern void			itoa (uchar_t *, int);
extern void			bferr(char *);
extern void			seterr(char *);
extern void			seterr2(char *, uchar_t *);
extern void			seterrc(char *, uchar_t);

/* exec.c */
extern void			doexec(struct command *);
extern void			execash(int, struct command *);
extern void			xechoit(uchar_t **);
extern void			dohash(void);
extern void			dounhash(void);
extern void			hashstat(void);

/* exp.c */
extern int			exp(uchar_t ***);
extern int			exp0(uchar_t ***, bool);
extern int			exp1(uchar_t ***, bool);
extern int			exp2(uchar_t ***, bool);
extern int			exp2a(uchar_t ***, bool);
extern int			exp2b(uchar_t ***, bool);
extern int			exp2c(uchar_t ***, bool);
extern uchar_t *		exp3(uchar_t ***, bool);
extern uchar_t *		exp3a(uchar_t ***, bool);
extern uchar_t *		exp4(uchar_t ***, bool);
extern uchar_t *		exp5(uchar_t ***, bool);
extern uchar_t *		exp6(uchar_t ***i, bool);
extern void			evalav(uchar_t **);
extern int			isa(uchar_t *, int);
extern int			egetn(uchar_t *);

/* file.c */
extern int			sortscmp(const uchar_t **, const uchar_t **);
extern int			tenex_search(uchar_t *, COMMAND, int);
extern int			tenex(uchar_t *, int);

/* func.c */
extern struct biltins *		isbfunc(struct command *);
extern void			func(struct command *, struct biltins *);
extern void			doonintr( uchar_t **);
extern void			donohup(void);
extern void			dozip(void);
extern void			prvars(void);
extern void			doalias(uchar_t **);
extern void			unalias(uchar_t **);
extern void			dologout(void);
extern void			dologin(uchar_t **);
extern void			donewgrp(uchar_t **);
extern void			doinlib(uchar_t **);
extern void			dormlib(uchar_t **);
extern void			islogin(void);
extern void			doif(uchar_t **, struct command *);
extern void			reexecute(struct command *);
extern void			doelse(void);
extern void			dogoto(uchar_t **);
extern void			doswitch(uchar_t **);
extern void			dobreak(void);
extern void			doexit(uchar_t **);
extern void			doforeach(uchar_t **);
extern void			dowhile(uchar_t **);
extern void			preread(void);
extern void			doend(void);
extern void			docontin(void);
extern void			doagain(void);
extern void			dorepeat(uchar_t **, struct command *);
extern void			doswbrk(void);
extern int			srchx(uchar_t *);
extern void			search(int , int, uchar_t *);
extern int			getword(uchar_t *);
extern void			toend(void);
extern void			wfree(void);
extern void			doecho(uchar_t **);
extern void			doglob(uchar_t **);
extern void			echo(uchar_t, uchar_t **);
extern void			dosetenv(uchar_t **);
extern void			dounsetenv(uchar_t **);
extern void			setcenv(char *, uchar_t *);
extern int			unsetcenv(uchar_t *);
extern void			doumask(uchar_t **);
extern struct limits *		findlim(uchar_t *);
extern void			dolimit(uchar_t **);
extern int			getval(struct limits *, uchar_t **);
extern void			limtail(uchar_t *, char *);
extern void			plim(struct limits *, uchar_t);
extern void			dounlimit(register uchar_t **);
extern int			setlim(struct limits *, uchar_t, int);
extern void			dosuspend(void);
extern void			doeval(uchar_t **);

/* glob.c */
extern uchar_t **		glob(uchar_t **);
extern void			ginit(uchar_t **);
extern void			collect(uchar_t *);
extern void			acollect(uchar_t *);
extern void			sort(void);
extern void			expand(uchar_t *);
extern void			matchdir(uchar_t *);
extern int			execbrc(uchar_t *, uchar_t *);
extern int			match(uchar_t *, uchar_t *);
extern int			amatch(uchar_t *, uchar_t *);
extern int			Gmatch(uchar_t *, uchar_t *);
extern void			Gcat(uchar_t *, uchar_t *);
extern void			addpath(uchar_t);
extern void			rscan(uchar_t **, int (*)(uchar_t));
extern void			scan(uchar_t **, void (*)());
extern int			tglob(uchar_t);
extern void			trim(void);
extern void			tback(uchar_t);
extern uchar_t *		globone(uchar_t *);
extern uchar_t **		dobackp(uchar_t *, bool);
extern void			backeval(uchar_t *, bool);
extern void			psave(uchar_t);
extern void			pword(void);

/* hist.c */
extern void			savehist(struct wordent *);
extern struct Hist *		enthist(int, struct wordent *, bool);
extern void			hfree(struct Hist *);
extern void			dohist(uchar_t **);
extern void			dohist1(struct Hist *, int *, int, int);
extern void			phist(struct Hist *, int);

#ifdef  OLDHIST
extern void			dohist(uchar_t **);
extern void			dohist1(struct Hist *, int *, int);
extern void			phist(struct Hist *);
#endif

/* init.c */
extern void 			doendif();
extern void 			doendsw();

/* lex.c */
extern int			lex(struct wordent *);
extern void			prlex(struct wordent *);

#ifdef CMDEDIT
extern void			prpushlex(struct wordent *);
#endif

extern void			copylex(struct wordent *, struct wordent *);
extern void			freelex(struct wordent *);
extern uchar_t *		word(void);
extern int			getC(int);
extern void			getdol(void);
extern void			addla(uchar_t *);
extern void			getexcl(int);
extern struct wordent *		getsub(struct wordent *);
extern struct wordent *		dosub(int, struct wordent *, bool);
extern uchar_t *		subword(uchar_t *, int, bool *);
extern uchar_t *		domod(uchar_t *, int);
extern int			matchs(uchar_t *, uchar_t *);
extern int			getsel(int *, int *, int);
extern struct wordent *		gethent(int);
extern struct Hist *		findev(uchar_t *, bool);
extern void			noev(uchar_t *);
extern int			matchev(struct Hist *, uchar_t *, bool);
extern void			setexclp(uchar_t *);
extern void			unreadc(wchar_t);
extern int			readc(bool);
extern int			read_one_c(bool);
extern int			bgetc(void);
extern void			bfree(void);
extern void			bseek(long);
extern long			btell(void);
extern void			btoeof(void);

/* misc.c */
#define letter(c) (iswalpha((wint_t)c) || (wint_t)c == '_')
#define digit(c)  iswdigit((wint_t)c)
extern int	  alnum(wint_t);
extern int	  any_noquote(wint_t, uchar_t *);
#define alnum(c) (letter(c) || digit(c))
extern int			any(wint_t, uchar_t*);
extern uchar_t **		blkend(uchar_t **);
extern void			blkpr(uchar_t **);
extern int			blklen(uchar_t **);
extern uchar_t **		blkcpy(uchar_t **,  uchar_t **);
extern uchar_t **		blkcat(uchar_t **, uchar_t **);
extern int			blkfree(uchar_t **);
extern uchar_t **		saveblk(uchar_t **);
extern uchar_t *		strspl(uchar_t *, uchar_t *);
extern uchar_t **		blkspl(uchar_t **, uchar_t **);
extern int			lastchr(uchar_t *);
extern void			closem(void);
extern void			closech(void);
extern void			donefds(void);
extern int			dmove(int, int);
extern int			dcopy(int, int);
extern int			renum(int, int);
extern void			copy(uchar_t *, uchar_t *, int);
extern void			lshift(uchar_t **, int);
extern int			number(uchar_t *);
extern uchar_t **		copyblk(uchar_t **);
extern uchar_t *		strend(uchar_t *);
extern uchar_t *		strip(uchar_t *);
extern void			udvar(uchar_t *);
extern int			prefix(uchar_t *, uchar_t *);
extern int			onlyread(uchar_t *);

/* parse.c */
extern void			alias(struct wordent *);
extern void			asyntax(struct wordent *, struct wordent *);
extern void			asyn0(struct wordent *, struct wordent *);
extern void			asyn3(struct wordent *, struct wordent *);
extern struct wordent *		freenod(struct wordent *, struct wordent *);
extern struct command *		syntax(struct wordent *, struct wordent *, int);
extern struct command *		syn0(struct wordent *, struct wordent *, int);
extern struct command *		syn1(struct wordent *, struct wordent *, int);
extern struct command *		syn1a(struct wordent *, struct wordent *, int);
extern struct command *		syn1b(struct wordent *, struct wordent *, int);
extern struct command *		syn2(struct wordent *, struct wordent *, int);
extern struct command *		syn3(struct wordent *, struct wordent *, int);
extern void			freesyn(struct command *);

/* print.c */
extern void			p60ths(long);
extern void			psecs(long);
extern void			p2dig(int);
extern void			display_char(int);
extern void			draino(void);
extern void			flush(void);
extern void			flush_now(void);
extern void			put_one_char(int);
extern void			draino(void);
extern void			plist(struct varent *);

/* proc.c */
extern void			pchild(int);
extern void			pnote(void);
extern void			pwait(void);
extern void			pjwait(struct process *);
extern void			dowait(void);
extern void			pflushall(void);
extern void			pflush(struct process  *);
extern void			pclrcurr(struct process *);
extern void			palloc(pid_t, pid_t, struct command *);
extern void			padd(struct command *);
extern void			pads(char *);
extern void			psavejob(void);
extern void			prestjob(void);
extern void			pendjob(void);
extern int			pprint(struct process *, int);
extern void			ptprint( struct process *);
extern void			dojobs(uchar_t **);
extern void			dofg(uchar_t **);
extern void			dobg(uchar_t **);
extern void			dostop(uchar_t **);
extern void			dokill(uchar_t **);
extern void			pkill(uchar_t **, int);
extern void			pstart(struct process *, int);
extern void			panystop(int);
extern struct process *		pfind(uchar_t *);
extern struct process *		pgetcurr(struct process *);
extern void			donotify(uchar_t **);
extern pid_t			pfork(struct command *, int);
extern void			okpcntl(void);
extern int			siggetmask(void);
extern void			donice(uchar_t **);

/* sem.c */
extern void			execute(struct command *, int, int *, int *);
extern void			doio(struct command *, int *, int *);
extern void			mypipe(int *);
extern void			chkclob(uchar_t *);

/* set.c */
extern void			doset(uchar_t **);
extern uchar_t *		getinx(uchar_t *, int *);
extern void			asx(uchar_t *, int, uchar_t *);
extern struct varent *		getvx(int, int);
extern void			dolet(uchar_t **);
extern uchar_t *		xset(uchar_t *, uchar_t ***);
extern uchar_t *		operate(uchar_t, uchar_t *, uchar_t *);
extern void			xfree(uchar_t *);
extern uchar_t *		savestr(uchar_t *);
extern uchar_t *		putn(int);
extern void			putn1(int);
extern int			getn(uchar_t *);
extern uchar_t *		value(char *);
extern uchar_t *		value1(char *, struct varent *);
extern struct varent *		adrof(char *);
extern struct varent *		madrof(uchar_t *, struct varent *);
extern struct varent *		adrof1(char *, struct varent *);
extern void			set(char *, uchar_t *);
extern void			set1(char *, uchar_t **, struct varent *);
extern void			setq(char *, uchar_t **, struct varent *);
extern void			unset(uchar_t *[]);
extern void			unset1(uchar_t *[], struct varent *);
extern void			unsetv(char *);
extern void			unsetv1(char *, struct varent *);
extern void			shift(uchar_t **);
extern void			exportpath(uchar_t **);
extern void			xfree(uchar_t *);

/* sh.c */
extern void			untty(void);
extern void			importpath(uchar_t *);
extern void			srccat(uchar_t *, uchar_t *);
extern void			srcunit(int, bool, bool);
extern void			rechist(void);
extern void			goodbye(void);
extern void			exitstat(void);
extern void			phup(void);
extern void			pintr(void);
extern void			pintr1(bool);
extern void			process(bool);
extern void			dosource(uchar_t **);
extern void			mailchk(void);
extern int			gethdir(uchar_t *);
extern void			initdesc(void);
extern void			exitcsh(int);
extern void			printprompt(void);

/* tenex.c */
#ifdef CMDEDIT
extern int			tenex_initialized(void);
extern void			dosetupterm (void);
extern int			ed_tenex (char *, int);
extern void			SaveMacros (void);
extern int			dobindings (char *);
extern void			dobind (char **);
extern char *			folddown (char *, char *);
extern int			stablk (char *, char **);
#endif

/* time.c */
extern void			settimes(void);
extern void			dotime(void);
extern void			ruadd(struct rusage *, struct rusage *);
extern void			prusage(struct rusage *, struct rusage *,
					struct timeval *, struct timeval *);
extern void			pdeltat(struct timeval *, struct timeval *);
extern void			tvadd(struct timeval *, struct timeval *);
extern void			tvsub(struct timeval *, struct timeval *,
					struct timeval *);
