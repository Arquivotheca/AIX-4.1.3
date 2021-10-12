/* @(#)92	1.37  src/bos/usr/bin/bsh/defs.h, cmdbsh, bos411, 9428A410j 5/6/94 16:43:50 */
/*
 * COMPONENT_NAME: (CMDBSH) Bourne shell and related commands
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
 * 1.25  com/cmd/sh/sh/defs.h, cmdsh, bos320, 9138320 9/9/91 15:16:14
 * 
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 *
 *
 * OSF/1 1.1
 */

#include <sys/types.h>

/* error exits from various parts of shell */
#define		ERROR		1
#define 	SYNBAD		2
#define 	SIGFAIL 	2000
#define 	SIGFLG		0200

/* command tree */
#define 	FPRS		0x0100
#define 	FINT		0x0200
#define 	FAMP		0x0400
#define 	FPIN		0x0800
#define 	FPOUT		0x1000
#define 	FPCL		0x2000
#define 	FCMD		0x4000
#define 	COMMSK		0x00F0
#define		CNTMSK		0x000F

#define 	TCOM		0x0000
#define 	TPAR		0x0010
#define 	TFIL		0x0020
#define 	TLST		0x0030
#define 	TIF		0x0040
#define 	TWH		0x0050
#define 	TUN		0x0060
#define 	TSW		0x0070
#define 	TAND		0x0080
#define 	TORF		0x0090
#define 	TFORK		0x00A0
#define 	TFOR		0x00B0
#define		TFND		0x00C0

/* execute table */
#define 	SYSSET		1
#define 	SYSCD		2
#define 	SYSEXEC		3
#define 	SYSNEWGRP	4
#define 	SYSTRAP		5
#define 	SYSEXIT		6
#define 	SYSSHFT 	7
#define 	SYSWAIT		8
#define 	SYSCONT 	9
#define 	SYSBREAK	10
#define 	SYSEVAL 	11
#define 	SYSDOT		12
#define 	SYSRDONLY	13
#define 	SYSTIMES 	14
#define 	SYSXPORT	15
#define 	SYSNULL 	16
#define 	SYSREAD 	17
#define		SYSTST		18
#define 	SYSLOGIN	19	
#define 	SYSUMASK 	20
#define 	SYSULIMIT	21
#define 	SYSECHO		22
#define		SYSHASH		23
#define		SYSPWD		24
#define 	SYSRETURN	25
#define		SYSUNS		26
#define		SYSMEM		27
#define		SYSTYPE  	28
#define		SYSGETOPT 	29
#ifdef _OSF
/* OSF was 29 & 30 for these defines, modifed for no conflict with SYSGETOPT */
#define		SYSINLIB 	30
#define		SYSRMLIB 	31
#endif
	

/*io nodes*/
#define 	USERIO		10
#define 	IOUFD		15
#define 	IODOC		16
#define 	IOPUT		32
#define 	IOAPP		64
#define 	IOMOV		128
#define 	IORDW		256
#define		IOSTRIP		512
#define 	INPIPE		0
#define 	OUTPIPE		1

/* arg list terminator */
#define 	ENDARGS		0

#include	"mac.h"
#include	"mode.h"
#include	"name.h"
#include	<signal.h>
#include	<stdio.h>


/* used for input and output of shell */
#define	INIO		(_open_max -1)


/*	error catching */
extern int		errno;

/* getopt */
extern int		optind;
extern char		*optarg;

#define		alloc 	malloc

#ifdef  _SHRLIB /* shared library */
#define		free 	alloc_free
#endif /* _SHRLIB */

/* result type declarations */
extern float		expr();
extern int		  exname();
extern int		  printexp();
extern int		  printnam();
extern int		  printro();
extern struct dolnod	 *useargs();
extern struct namnod	 *findnam();
extern struct namnod	 *lookup();
extern struct trenod	 *cmd();
extern struct trenod	 *makefork();
extern uchar_t		**scan();
extern uchar_t		**sh_setenv();
extern uchar_t		 *NLSndecode();
extern uchar_t		 *catpath();
extern uchar_t		 *getpath();
extern uchar_t		 *macro();
extern uchar_t		 *mactrim();
extern uchar_t		 *make();
extern uchar_t		 *movstr();
extern uchar_t		 *movstrn();
extern uchar_t		 *nextpath();
extern uchar_t		 *scanset();

#define		attrib(n,f)	(n->namflg |= f)
#define		round(a,b)	((((ulong)(a)+(b))-1)&~((b)-1))
#define		closepipe(x)	(close(x[INPIPE]), close(x[OUTPIPE]))
#define		eq(a,b)		(cf(a,b)==0)
#define		assert(x)	;
#define		NLSisencoded(s)	((s == NULL)? 0 : *(s)==FNLS)
#define		NLSskiphdr(s)	(NLSisencoded(s)?(s)++:(s))
#define		NLSneedesc(c)	((c)>127 || NLSfontshift(c) || (c)==FNLS)

#define		NLSenclen(s)	(((*(s) & STRIP) == FSH0) ? 2 \
				: ((*(s) & STRIP) == FSH21) ? \
				((*(s+1) & STRIP) * 2 + 2)    \
				: 1)

#include "bsh_msg.h"
extern nl_catd 	catd;
#define 	MSGSTR(num,str)	catgets(catd,MS_BSH,num,str)

/* temp files and io */
extern int		output;
extern int		ioset;
extern struct ionod	*iotemp;	/* files to be deleted sometime */
extern struct ionod	*fiotemp;	/* function files to be deleted */
extern struct ionod	*iopend;	/* documents waiting to be read at NL */
extern struct fdsave	*fdmap;


/* substitution */
extern int		dolc; /* number of command line arguments - $# */
extern uchar_t		**dolv;
extern struct dolnod	*argfor;
extern struct argnod	*gchain;

#include		"stak.h"

/* string constants */
extern uchar_t		atline[];
extern uchar_t		colon[];
extern uchar_t		endoffile[];
extern uchar_t		minus[];
extern uchar_t		nullstr[];
extern uchar_t		readmsg[];
extern uchar_t		sptbnl[];
extern uchar_t		synmsg[];
extern uchar_t		unexpected[];

/* name tree and words */
extern struct sysnod	reserved[];
extern int		no_reserved;
extern struct sysnod	commands[];
extern int		no_commands;

extern int		wdval;
extern int		wdnum;
extern int		fndef;
extern int		nohash;
extern struct argnod	*wdarg;
extern int		wdset;
extern BOOL		reserv;

/* prompting */
extern uchar_t		profile[];	/* user profile */
extern uchar_t		shstdprompt[];	/* shell standard prompt */
extern uchar_t		shsupprompt[];	/* shell superuser prompt */
extern uchar_t		sysprofile[];	/* system profile */

/* built in names */
extern struct namnod	acctnod;	/* SHACCT */
extern struct namnod	cdpnod;		/* CDPATH */
extern struct namnod	fngnod;
extern struct namnod	homenod;	/* HOME */
extern struct namnod	ifsnod;		/* IFS */
extern struct namnod	langnod;	/* LANG */
extern struct namnod	lcallnod;	/* LC_ALL */
extern struct namnod	locpathnod;	/* LOCPATH */
extern struct namnod	mailmnod;	/* MAILMSG */
extern struct namnod	mailnod;	/* MAIL */
extern struct namnod	mailpnod;	/* MAILPATH */
extern struct namnod	mchknod;	/* MAILCHECK */
extern struct namnod	nlspathnod;	/* NLSPATH */
extern struct namnod	pathnod;	/* PATH */
extern struct namnod	ps1nod;		/* PS1 */
extern struct namnod	ps2nod;		/* PS2 */
extern struct namnod	timenod;	/* TIME */

/* special names */
extern uchar_t		flagadr[];
extern uchar_t		*pcsadr;
extern uchar_t		*pidadr;
extern uchar_t		*cmdadr;

/* superuser pathnames */
extern uchar_t		shdefpath[];	/* shell default PATH */
extern uchar_t		sudefpath[];	/* shell supersuer PATH */

/* names always present */
extern uchar_t		acctname[];	/* SHACCT */
extern uchar_t		cdpname[];	/* CDPATH */
extern uchar_t		collate[];	/* LC_COLLATE */
extern uchar_t		ctype[];	/* LC_CTYPE */
extern uchar_t		homename[];	/* HOME */
extern uchar_t		ifsname[];	/* IFS */
extern uchar_t		lang[];		/* LANG */
extern uchar_t		lcall[];	/* LC_ALL */
extern uchar_t		lctime[];	/* LC_CTIME */
extern uchar_t		locpath[];	/* LOCPATH */
extern uchar_t		mailmname[];	/* MAILMSG */
extern uchar_t		mailname[];	/* MAIL */
extern uchar_t		mailpname[];	/* MAILPATH */
extern uchar_t		mchkname[];	/* MAILCHECK */
extern uchar_t		messages[];	/* LC_MESSAGES */
extern uchar_t		monetary[];	/* LC_MONETARY */
extern uchar_t		nlspath[];	/* NLSPATH */
extern uchar_t		numeric[];	/* LC_NUMERIC */
extern uchar_t		pathname[];	/* PATH */
extern uchar_t		ps1name[];	/* PS1 */
extern uchar_t		ps2name[];	/* PS2 */
extern uchar_t		shellname[];	/* SHELL */
extern uchar_t		timename[];	/* TIME */

/* transput */
extern uchar_t		tmpout[];	/* shell temporary files */
extern uchar_t		*tempname;
extern unsigned int	serial;		/* check digit for temp names */

#define		TMPNAM 		7	/* gets past /tmp/sh part of tmpout */

extern struct fileblk	*standin;	/* STDIN */

#define 	input		(standin->fdes) /* file descriptor of STDIN */
#define 	eof		(standin->sh_feof) /* EOF of STDIN */

extern unsigned int		peekc;
extern unsigned int		peekn;
extern uchar_t			*comdiv;
extern uchar_t			devnull[];
extern unsigned int		fshift;		/* readc() state */

/* flags */
#define		noexec		01	/* -n */
#define		sysflg		01
#define		intflg		02	/* -i */
#define		prompt		04
#define		setflg		010	/* -u */
#define		errflg		020	/* -e */
#define		ttyflg		040
#define		forked		0100
#define		oneflg		0200	/* -t */
#define		rshflg		0400	/* -r */
#define		waiting		01000
#define		stdflg		02000	/* -s */
#define		STDFLG		's'
#define		execpr		04000	/* -x */
#define		readpr		010000	/* -v */
#define		keyflg		020000	/* -k */
#define		hashflg		040000	/* -h */
#define		nofngflg	0200000	/* -f */
#define		exportflg	0400000	/* -a */
#ifdef NLSDEBUG
#define		debugflg	01000000 /* -D */
#endif

extern long	flags;
extern int	rwait;	/* flags read waiting */

/* error exits from various parts of shell */
#include		<setjmp.h>
extern jmp_buf		subshell;
extern jmp_buf		errshell;

/* fault handling */
#define		BRKINCR		050000  /* memory allocated, 5 pages */
#define		BRKMAX		BRKINCR*4 /* 20 pages */

extern unsigned	brkincr;
#define 	MINTRAP		0
#define 	TRAPSET		2
#define 	SIGSET		4
#define 	SIGMOD		8
#define 	SIGCAUGHT	16

extern void	fault(int, int, struct sigcontext *);
extern BOOL	trapnote;
extern BOOL	mailalarm;
extern uchar_t	*trapcom[];
extern BOOL	trapflg[];

/* name tree and words */
extern char		**environ;	/* global system environment */
extern uchar_t		numbuf[];
extern uchar_t		export[];	/* exported variables */
extern uchar_t		duperr[];
extern uchar_t		readonly[];	/* readonly variables */

/* execflgs */
extern int		exitval;	/* exit value */
extern int		retval;		/* return value */
extern BOOL		execbrk;	/* execute break */
extern int		loopcnt;	/* loop counter */
extern int		breakcnt;	/* break counter */
extern int		funcnt;		/* function counter */

/* messages */
extern uchar_t		arglist[];	/* arg list too long */
extern uchar_t		badcreate[];	/* cannot create */
extern uchar_t		baddir[];	/* bad directory */
extern uchar_t		badexec[];	/* cannot execute */
extern uchar_t		badexport[];	/* cannot export functions */
extern uchar_t		badfile[];	/* bad file number */
extern uchar_t		badhash[];	/* bad hash options(s) */
extern uchar_t		badnum[];	/* bad number */
extern uchar_t		badopen[];	/* cannot open */
extern uchar_t		badopt[];	/* bad option(s) */
extern uchar_t		badparam[];	/* parameter null or not set */
extern uchar_t		badperm[];	/* execute permission denied */
extern uchar_t		badreturn[];	/* can only return from a function */
extern uchar_t		badshift[];	/* cannot shift */
extern uchar_t		badsub[];	/* bad substitution */
extern uchar_t		badtrap[];	/* bad trap */
extern uchar_t		badunset[];	/* cannot unset */
extern uchar_t		cd_args[];	/* too many arguments */
extern uchar_t		coredump[];	/* - core dumped */
extern uchar_t		execpmsg[];	/* + (set -x mode) */
extern uchar_t		mailmsg[];	/* you have mail */
extern uchar_t		mssgargn[];	/* missing arguments */
extern uchar_t		nofork[];	/* fork failed - too many processes */
extern uchar_t		nohome[];	/* no home directory */
extern uchar_t		nospace[];	/* no space */
extern uchar_t		nostack[];	/* no stack space */
extern uchar_t		noswap[];	/* cannot fork: no swap space */
extern uchar_t		notfound[];	/* not found */
extern uchar_t		notid[];	/* is not an indentifier */
extern uchar_t		piperr[];	/* cannot make pipe */
extern uchar_t		restricted[];	/* restricted */
extern uchar_t		toobig[];	/* too big */
extern uchar_t		txtbsy[];	/* text busy */
extern uchar_t		ulimitbad[];	/* Specify a numeric ulimit value. */
extern uchar_t		ulimitexceed[];	/* new ulimit exceeds hard limit */
extern uchar_t		ulimithard[];	/* soft limit exceeds hard limit */
extern uchar_t		ulimitnotsu[];	/* only superuser can set hard limit */
extern uchar_t		ulimitsoft[];	/* new ulimit exceeds soft limit */
extern uchar_t		ulimitusage[];	/* ulimit usage */
extern uchar_t		unset[];	/* parameter not set */
extern uchar_t		wtfailed[];	/* is read only */
#ifdef _OSF
extern uchar_t		badinlib[];	/* loader install procedure failed */
extern uchar_t		badrmlib[];	/* loader removal procedure failed */
extern uchar_t		no_args[];	/* requires argument */
#endif

/* 'builtin' error messages */
extern uchar_t		badop[];
extern uchar_t		btest[];

/* Flag set for error condition for builtin echo */
int			echoerr;

/* fork constant */
#define 	FORKLIM 	32

#include	"shctype.h"

extern int		wasintr; /* used to tell if break or delete is hit
				 *  while executing a wait
				 */
extern int		eflag;


/*
 * Find out if it is time to go away.
 * `trapnote' is set to SIGSET when fault is seen and
 * no trap has been set.
 */

#define		sigchk()	if (trapnote & SIGSET)	\
					exitsh(exitval ? exitval : SIGFAIL)

#define		exitset()	retval = exitval
#define		MBMAX		4
#define		MBCDMAX		0xffffffff

#ifdef _AIX
#define		NAME_MAX	MAXNAMLEN
#endif
