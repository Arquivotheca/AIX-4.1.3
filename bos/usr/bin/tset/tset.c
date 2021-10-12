#ifndef lint
static char sccsid[] = "@(#)97	1.5  src/bos/usr/bin/tset/tset.c, cmdtty, bos411, 9428A410j 12/20/93 06:49:48";
#endif

/*
 * COMPONENT_NAME: CMDTTY tty control commands
 *
 * FUNCTIONS: main (tset)
 *
 * ORIGINS: 26, 27, 83
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
char copyright[] =
"Copyright (c) 1980 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint

/*
**  TSET -- set terminal modes
**
**	This program does sophisticated terminal initialization.
**	I recommend that you include it in your .profile or .login
**	file to initialize whatever terminal you are on.
**
**	There are several features:
**
**	A special file or sequence (as controlled by the termcap file)
**	is sent to the terminal.
**
**	Mode bits are set on a per-terminal_type basis (much better
**	than UNIX itself).  This allows special delays, automatic
**	tabs, etc.
**
**	Erase and Kill characters can be set to whatever you want.
**	Default is to change erase to control-H on a terminal which
**	can overstrike, and leave it alone on anything else.  Kill
**	is always left alone unless specifically requested.  These
**	characters can be represented as "^X" meaning control-X;
**	X is any character.
**
**	Terminals which are dialups or plugboard types can be aliased
**	to whatever type you may have in your home or office.  Thus,
**	if you know that when you dial up you will always be on a
**	TI 733, you can specify that fact to tset.  You can represent
**	a type as "?type".  This will ask you what type you want it
**	to be -- if you reply with just a newline, it will default
**	to the type given.
**
**	The current terminal type can be queried.
**
**	Usage:
**		tset [-] [-EC] [-eC] [-kC] [-iC] [-s] [-h] [-u] [-r]
**			[-m [ident] [test baudrate] :type]
**			[-Q] [-I] [-S] [type]
**
**		In systems with environments, use:
**			eval `tset -s ...`
**		Actually, this doesn't work in old csh's.
**		Instead, use:
**			tset -s ... > tset.tmp
**			source tset.tmp
**			rm tset.tmp
**		or:
**			set noglob
**			set term=(`tset -S ....`)
**			setenv TERM $term[1]
**			setenv TERMCAP "$term[2]"
**			unset term
**			unset noglob
**
**	Positional Parameters:
**		type -- the terminal type to force.  If this is
**			specified, initialization is for this
**			terminal type.
**
**	Flags:
**		- -- report terminal type.  Whatever type is
**			decided on is reported.  If no other flags
**			are stated, the only affect is to write
**			the terminal type on the standard output.
**		-r -- report to user in addition to other flags.
**		-EC -- set the erase character to C on all terminals
**			except those which cannot backspace (e.g.,
**			a TTY 33).  C defaults to control-H.
**		-eC -- set the erase character to C on all terminals.
**			C defaults to control-H.  If not specified,
**			the erase character is untouched; however, if
**			not specified and the erase character is NULL
**			(zero byte), the erase character  is set to delete.
**		-kC -- set the kill character to C on all terminals.
**			Default for C is control-X.  If not specified,
**			the kill character is untouched; however, if
**			not specified and the kill character is NULL
**			(zero byte), the kill character is set to control-U.
**		-iC -- set the interrupt character to C on all terminals.
**			Default for C is control-C.  If not specified, the
**			interrupt character is untouched; however, if
**			not specified and the interrupt character is NULL
**			(zero byte), the interrupt character is set to
**			control-C.
**		-qC -- reserved for setable quit character.
**		-m -- map the system identified type to some user
**			specified type. The mapping can be baud rate
**			dependent. This replaces the old -d, -p flags.
**			(-d type  ->  -m dialup:type)
**			(-p type  ->  -m plug:type)
**			Syntax:	-m identifier [test baudrate] :type
**			where: ``identifier'' is terminal type found in
**			/etc/ttys for this port, (abscence of an identifier
**			matches any identifier); ``test'' may be any combination
**			of  >  =  <  !  @; ``baudrate'' is as with stty(1);
**			``type'' is the actual terminal type to use if the
**			mapping condition is met. Multiple maps are scanned
**			in order and the first match prevails.
**		-h -- don't read htmp file.  Normally the terminal type
**			is determined by reading the htmp file or the
**			environment (unless some mapping is specified).
**			This forces a read of the ttytype file -- useful
**			when htmp is somehow wrong. (V6 only)
**		-u -- don't update htmp.  It seemed like this should
**			be put in.  Note that htmp is never actually
**			written if there are no changes, so don't bother
**			bother using this for efficiency reasons alone.
**		-s -- output setenv commands for TERM.  This can be
**			used with
**				`tset -s ...`
**			and is to be prefered to:
**				setenv TERM `tset - ...`
**			because -s sets the TERMCAP variable also.
**		-S -- Similar to -s but outputs 2 strings suitable for
**			use in csh .login files as follows:
**				set noglob
**				set term=(`tset -S .....`)
**				setenv TERM $term[1]
**				setenv TERMCAP "$term[2]"
**				unset term
**				unset noglob
**		-Q -- be quiet.  don't output 'Erase set to' etc.
**		-I -- don't do terminal initialization (is & if
**			strings).
**		-v -- On virtual terminal systems, don't set up a
**			virtual terminal.  Otherwise tset will tell
**			the operating system what kind of terminal you
**			are on (if it is a known terminal) and fix up
**			the output of -s to use virtual terminal sequences.
**
**	Files:
**		/etc/ttys
**			contains a terminal id -> terminal type
**			mapping; used when any user mapping is specified,
**			or the environment doesn't have TERM set.
**		/etc/termcap
**			a terminal_type -> terminal_capabilities
**			mapping.
**
**	Return Codes:
**		-1 -- couldn't open ttycap.
**		1 -- bad terminal type, or standard output not tty.
**		0 -- ok.
**
**	Defined Constants:
**		DIALUP -- the type code for a dialup port.
**		PLUGBOARD -- the type code for a plugboard port.
**		ARPANET -- the type code for an arpanet port.
**		BACKSPACE -- control-H, the default for -e.
**		CNTL('X') -- control-X, the default for -k.
**		OLDERASE -- the system default erase character.
**		OLDKILL -- the system default kill character.
**		FILEDES -- the file descriptor to do the operation
**			on, nominally 1 or 2.
**		STDOUT -- the standard output file descriptor.
**		UIDMASK -- the bit pattern to mask with the getuid()
**			call to get just the user id.
**		GTTYN -- defines file containing generalized ttynames
**			and compiles code to look there.
**
**	Requires:
**		Routines to handle htmp, ttys, and ttycap.
**
**	Compilation Flags:
**		OLDFLAGS -- must be defined to compile code for any of
**			the -d, -p, or -a flags.
**		OLDDIALUP -- accept the -d flag.
**		OLDPLUGBOARD -- accept the -p flag.
**		OLDARPANET -- accept the -a flag.
**		V6 -- if clear, use environments, not htmp.
**			also use TIOCSETN rather than stty to avoid flushing
**		GTTYN -- if set, compiles code to look at /etc/ttys.
**		UCB_NTTY -- set to handle new tty driver modes.
**
**	Trace Flags:
**		none
**
**	Diagnostics:
**		Bad flag
**			An incorrect option was specified.
**		Too few args
**			more command line arguments are required.
**		Unexpected arg
**			wrong type of argument was encountered.
**		Cannot open ...
**			The specified file could not be openned.
**		Type ... unknown
**			An unknown terminal type was specified.
**		Cannot update htmp
**			Cannot update htmp file when the standard
**			output is not a terminal.
**		Erase set to ...
**			Telling that the erase character has been
**			set to the specified character.
**		Kill set to ...
**			Ditto for kill
**		Erase is ...    Kill is ...
**			Tells that the erase/kill characters were
**			wierd before, but they are being left as-is.
**		Not a terminal
**			Set if FILEDES is not a terminal.
**
**	Compilation Instructions:
**		cc -n -O tset.c -ltermlib
**		mv a.out tset
**		chown bin tset
**		chmod 4755 tset
**
**		where 'bin' should be whoever owns the 'htmp' file.
**		If 'htmp' is 666, then tset need not be setuid.
**
**		For version 6 the compile command should be:
**		cc -n -O -I/usr/include/retrofit tset.c -ltermlib -lretro -lS
**
**	Author:
**		Eric Allman
**		Electronics Research Labs
**		U.C. Berkeley
**
**	History:
**		1/81 -- Added alias checking for mapping identifiers.
**		9/80 -- Added UCB_NTTY mods to setup the new tty driver.
**			Added the 'reset ...' invocation.
**		7/80 -- '-S' added. '-m' mapping added. TERMCAP string
**			cleaned up.
**		3/80 -- Changed to use tputs.  Prc & flush added.
**		10/79 -- '-s' option extended to handle TERMCAP
**			variable, set noglob, quote the entry,
**			and know about the Bourne shell.  Terminal
**			initialization moved to before any information
**			output so screen clears would not screw you.
**			'-Q' option added.
**		8/79 -- '-' option alone changed to only output
**			type.  '-s' option added.  'VERSION7'
**			changed to 'V6' for compatibility.
**		12/78 -- modified for eventual migration to VAX/UNIX,
**			so the '-' option is changed to output only
**			the terminal type to STDOUT instead of
**			FILEDES.
**		9/78 -- '-' and '-p' options added (now fully
**			compatible with ttytype!), and spaces are
**			permitted between the -d and the type.
**		8/78 -- The sense of -h and -u were reversed, and the
**			-f flag is dropped -- same effect is available
**			by just stating the terminal type.
**		10/77 -- Written.
*/

#define _BSD
#define BSD

#  define index strchr
#  define rindex strrchr
#  define curerase mode.c_cc[VERASE]
#  define curkill mode.c_cc[VKILL]
#  define curintr mode.c_cc[VINTR]
#  define olderase oldmode.c_cc[VERASE]
#  define oldkill oldmode.c_cc[VKILL]
#  define oldintr oldmode.c_cc[VINTR]

# define	GTTYN
# include        <locale.h>
# include	<ttyent.h>

# include <termios.h>
# include <sys/ioctl.h>

# include	<stdio.h>
# include	<stdlib.h>
# include	<signal.h>

# define	YES		1
# define	NO		0
#undef CNTL
# define	CNTL(c)		((c)&037)
# define	BACKSPACE	(CNTL('H'))
# define	CHK(val, dft)	((val == 0 || val == '\377') ? dft : val)
# define	isdigit(c)	(c >= '0' && c <= '9')
# define	isalnum(c)	(c > ' ' && (index("<@=>!:|\177", (int)c) == NULL))
# define	OLDERASE	'#'
# define	OLDKILL		'@'
# define	OLDINTR		'\177'	/* del */

/* default special characters */
#ifndef CERASE
#define	CERASE	'\177'
#endif
#ifndef CKILL
#define	CKILL	CNTL('U')
#endif
#ifndef CINTR
#define	CINTR	CNTL('C')
#endif
#ifndef CDSUSP
#define	CQUIT	034		/* FS, ^\ */
#define	CSTART	CNTL('Q')
#define	CSTOP	CNTL('S')
#define	CEOF	CNTL('D')
#define	CEOT	CEOF
#define	CBRK	0377
#define	CSUSP	CNTL('Z')
#define	CDSUSP	CNTL('Y')
#define	CRPRNT	CNTL('R')
#define	CFLUSH	CNTL('O')
#define	CWERASE	CNTL('W')
#define	CLNEXT	CNTL('V')
#endif

int FILEDES  = 2;

# define	STDOUT		1	/* output of -s/-S to this descriptor */

# define	UIDMASK		0377

# define	USAGE	MSGSTR(USE, "usage: tset [-] [-rsIQS] [-eC] [-kC] [-iC] [-m [ident][test speed]:type] [type]\n")

# define	OLDFLAGS
# define	DIALUP		"dialup"
# define	OLDDIALUP	"sd"
# define	PLUGBOARD	"plugboard"
# define	OLDPLUGBOARD	"sp"
/***
# define	ARPANET		"arpanet"
# define	OLDARPANET	"sa"
***/

# define	DEFTYPE		"unknown"


# ifdef GTTYN
# define	NOTTY		0
# else
# define	NOTTY		'x'
# endif

/*
 * Baud Rate Conditionals
 */
# define	ANY		0
# define	GT		1
# define	EQ		2
# define	LT		4
# define	GE		(GT|EQ)
# define	LE		(LT|EQ)
# define	NE		(GT|LT)
# define	ALL		(GT|EQ|LT)



# define	NMAP		10

struct	map {
	char *Ident;
	char Test;
	char Speed;
	char *Type;
} map[NMAP];

struct map *Map = map;

/* This should be available in an include file */
struct {
	char	*string;
	int	speed;
	int	baudrate;
} speeds[] = {
	"0",	B0,	0,
	"50",	B50,	50,
	"75",	B75,	75,
	"110",	B110,	110,
	"134",	B134,	134,
	"134.5",B134,	134,
	"150",	B150,	150,
	"200",	B200,	200,
	"300",	B300,	300,
	"600",	B600,	600,
	"1200",	B1200,	1200,
	"1800",	B1800,	1800,
	"2400",	B2400,	2400,
	"4800",	B4800,	4800,
	"9600",	B9600,	9600,
	"19200",EXTA,	19200,
	"exta",	EXTA,	19200,
	"extb",	EXTB,	38400,
	0,
};

char	Erase_char;		/* new erase character */
char	Kill_char;		/* new kill character */
char	Intr_char;		/* new interrupt character */
char	Specialerase;		/* set => Erase_char only on terminals with backspace */

# ifdef	GTTYN
char	*Ttyid = NOTTY;		/* terminal identifier */
extern char *ttyname(int fd);
# else
char	Ttyid = NOTTY;		/* terminal identifier */
# endif
char	*TtyType;		/* type of terminal */
char	*DefType;		/* default type if none other computed */
char	*NewType;		/* mapping identifier based on old flags */
int	Mapped;			/* mapping has been specified */
int	Dash_u;			/* don't update htmp */
int	Dash_h;			/* don't read htmp */
int	DoSetenv;		/* output setenv commands */
int	BeQuiet;		/* be quiet */
int	NoInit;			/* don't output initialization string */
int	IsReset;		/* invoked as reset */
int	Report;			/* report current type */
int	Ureport;		/* report to user */
int	RepOnly;		/* report only */
int	CmndLine;		/* output full command lines (-s option) */
int	Ask;			/* ask user for termtype */
int	DoVirtTerm = YES;	/* Set up a virtual terminal */
int	PadBaud;		/* Min rate of padding needed */
int	lines, columns;

# define CAPBUFSIZ	1024
char	Capbuf[CAPBUFSIZ];	/* line from /etc/termcap for this TtyType */
char	*Ttycap;		/* termcap line from termcap or environ */

char	Aliasbuf[128];
char	*Alias[16];

extern char *tgetstr();

struct delay {
    int	d_delay;
    int	d_bits;
};

# include <string.h>
# include	"tset.h"

#include        <nl_types.h>
#include        "tset_msg.h"

nl_catd catd;
#define MSGSTR(num,str) catgets(catd,MS_TSET,num,str)  /*MSG*/

struct termios	mode;
struct termios	oldmode;

int Debugflg;
extern char PC;
extern short ospeed;

extern settabs();
extern setmode(int flag);
extern reportek(char *name, cc_t new, cc_t old, cc_t def);
extern setdelay(char *cap, struct delay dtab[], int bits, tcflag_t *flags);
extern prs(char *s);
extern prc(char c);
extern flush();
extern cat(char *file);
extern bmove(char *from, char *to, int length);
extern bequal(char *a, char *b, int len);
extern sequal(char *a, char *b);
extern makealias(char *buf);
extern isalias(char *ident);
extern char *stypeof(char *ttyid);
extern wrtermcap(char *bp);
extern cancelled(char *cap);
extern char *putbuf(char *ptr, char *str);
extern baudrate(char *p);
extern char *mapped(char *type);
extern prmap();
extern char *nextarg(int argc, char *argv[]);
extern fatal(char *mesg, char *obj);
extern debugchk();


main(int argc, char *argv[])
{
    char buf[CAPBUFSIZ];
    char termbuf[32];
    auto char *bufp;
    register char *p;
    char *command;
    register int i;
    int Break;
    int Not;
    struct winsize win;
    char bs_char;
    int csh;
    int settle;
    int setmode(int);
    char *gstr;

    setlocale(LC_ALL,"") ;
    catd = catopen(MF_TSET, NL_CAT_LOCALE);

    debugchk();

    if (tcgetattr(FILEDES, &mode)) {
	prs(MSGSTR(NOTTERM, "Not a terminal\n"));
	exit(1);
    }
    bmove((char *)&mode, (char *)&oldmode, (int)sizeof mode);
    ospeed = cfgetospeed(&mode);
    (void) signal(SIGINT, (void(*)(int)) setmode);
    (void) signal(SIGQUIT, (void(*)(int)) setmode);
    (void) signal(SIGTERM, (void(*)(int)) setmode);

    if (command = rindex(argv[0], (int)'/'))
	command++;
    else
	command = argv[0];
    if (sequal(command, "reset")) {
	/* 
	 * reset the teletype mode bits to a sensible state. Copied
	 * from the program by Kurt Shoens & Mark Horton. Very useful
	 * after crapping out in raw.
	 */
	(void) tcgetattr(0, &mode);
	curerase = CHK(curerase, OLDERASE);
	curkill = CHK(curkill, OLDKILL);
	curintr = CHK(curintr, OLDINTR);
	mode.c_cc[VSUSP] = CHK(mode.c_cc[VSUSP], CSUSP);
	mode.c_cc[VDSUSP] = CHK(mode.c_cc[VDSUSP], CDSUSP);
	mode.c_cc[VREPRINT] = CHK(mode.c_cc[VREPRINT], CRPRNT);
	mode.c_cc[VDISCRD] = CHK(mode.c_cc[VDISCRD], CFLUSH);
	mode.c_cc[VWERSE] = CHK(mode.c_cc[VWERSE], CWERASE);
	mode.c_cc[VLNEXT] = CHK(mode.c_cc[VLNEXT], CLNEXT);
	mode.c_cc[VINTR] = CHK(mode.c_cc[VINTR], CINTR);
	mode.c_cc[VQUIT] = CHK(mode.c_cc[VQUIT], CQUIT);
	mode.c_cc[VSTART] = CHK(mode.c_cc[VSTART], CSTART);
	mode.c_cc[VSTOP] = CHK(mode.c_cc[VSTOP], CSTOP);
	mode.c_cc[VEOF] = CHK(mode.c_cc[VEOF], CEOF);

	mode.c_iflag |= (BRKINT|ICRNL|IXON);
	mode.c_iflag &= ~(IGNBRK|PARMRK|INPCK|INLCR|IGNCR|IUCLC|IXOFF);
	mode.c_oflag |= (OPOST|ONLCR);
	mode.c_oflag &= ~(OLCUC|OCRNL|ONOCR|ONLRET|OFILL|OFDEL|
			  NLDLY|CRDLY|TABDLY|BSDLY|VTDLY|FFDLY);
	mode.c_cflag &= ~(CSIZE|PARENB|PARODD|CLOCAL);
	mode.c_cflag |= (CS8|CREAD);
	mode.c_lflag |= (ISIG|ICANON|ECHO|ECHOK);
	mode.c_lflag &= ~(XCASE|ECHONL|NOFLSH);
	(void) tcsetattr(FILEDES, TCSADRAIN, &mode);
	Dash_u = YES;
	BeQuiet = YES;
	IsReset = YES;
    } else if (argc == 2 && sequal(argv[1], "-")) {
	RepOnly = YES;
	Dash_u = YES;
    }
    argc--;

    /* scan argument list and collect flags */
    while (--argc >= 0) {
	p = *++argv;
	if (*p == '-') {
	    if (*++p == '\0')
		Report = YES;		/* report current terminal type */
	    else while (*p) switch (*p++) {

	    case 'r':			/* report to user */
		Ureport = YES;
		continue;

	    case 'E':			/* special erase */
		Specialerase = YES;
		/* explicit fall-through to -e case */

	    case 'e':			/* erase character */
		if (*p == '\0')
		    Erase_char = -1;
		else {
		    if (*p == '^' && p[1] != '\0')
			if (*++p == '?')
			    Erase_char = '\177';
			else
			    Erase_char = CNTL(*p);
		    else
			Erase_char = *p;
		    p++;
		}
		continue;

	    case 'i':			/* interrupt character */
		if (*p == '\0')
		    Intr_char = CNTL('C');
		else {
		    if (*p == '^' && p[1] != '\0')
			if (*++p == '?')
			    Intr_char = '\177';
			else
			    Intr_char = CNTL(*p);
		    else
			Intr_char = *p;
		    p++;
		}
		continue;

	    case 'k':			/* kill character */
		if (*p == '\0')
		    Kill_char = CNTL('X');
		else {
		    if (*p == '^' && p[1] != '\0')
			if (*++p == '?')
			    Kill_char = '\177';
			else
			    Kill_char = CNTL(*p);
		    else
			Kill_char = *p;
		    p++;
		}
		continue;

# ifdef OLDFLAGS
# ifdef OLDDIALUP
	    case 'd':			/* dialup type */
		NewType = DIALUP;
		goto mapold;
# endif

# ifdef OLDPLUGBOARD
	    case 'p':			/* plugboard type */
		NewType = PLUGBOARD;
		goto mapold;
# endif

# ifdef OLDARPANET
	    case 'a':			/* arpanet type */
		Newtype = ARPANET;
		goto mapold;
# endif

	    mapold: Map->Ident = NewType;
		Map->Test = ALL;
		if (*p == '\0') {
		    p = nextarg(argc--, argv++);
		}
		Map->Type = p;
		Map++;
		Mapped = YES;
		p = "";
		continue;
# endif

	    case 'm':			/* map identifier to type */
		/* This code is very loose. Almost no
		 ** syntax checking is done!! However,
		 ** illegal syntax will only produce
		 ** weird results.
		 */
		if (*p == '\0') {
		    p = nextarg(argc--, argv++);
		}
		if (isalnum(*p)) {
		    Map->Ident = p;	/* identifier */
		    while (isalnum(*p)) p++;
		} else
		    Map->Ident = "";
		Break = NO;
		Not = NO;
		while (!Break) switch (*p) {
		case '\0':
		    p = nextarg(argc--, argv++);
		    continue;

		case ':':		/* mapped type */
		    *p++ = '\0';
		    Break = YES;
		    continue;

		case '>':		/* conditional */
		    Map->Test |= GT;
		    *p++ = '\0';
		    continue;

		case '<':		/* conditional */
		    Map->Test |= LT;
		    *p++ = '\0';
		    continue;

		case '=':		/* conditional */
		case '@':
		    Map->Test |= EQ;
		    *p++ = '\0';
		    continue;
 
		case '!':		/* invert conditions */
		    Not = ~Not;
		    *p++ = '\0';
		    continue;

		case 'B':		/* Baud rate */
		    p++;
		    /* intentional fallthru */
		default:
		    if (isdigit(*p) || *p == 'e') {
			Map->Speed = baudrate(p);
			while (isalnum(*p) || *p == '.')
			    p++;
		    } else
			Break = YES;
		    continue;
		}
		if (Not) {		/* invert sense of test */
		    Map->Test = (~(Map->Test))&ALL;
		}
		if (*p == '\0') {
		    p = nextarg(argc--, argv++);
		}
		Map->Type = p;
		p = "";
		Map++;
		Mapped = YES;
		continue;

	    case 'h':			/* don't get type from htmp or env */
		Dash_h = YES;
		continue;

	    case 'u':			/* don't update htmp */
		Dash_u = YES;
		continue;

	    case 's':			/* output setenv commands */
		DoSetenv = YES;
		CmndLine = YES;
		continue;

	    case 'S':			/* output setenv strings */
		DoSetenv = YES;
		CmndLine = NO;
		continue;

	    case 'Q':			/* be quiet */
		BeQuiet = YES;
		continue;

	    case 'I':			/* no initialization */
		NoInit = YES;
		continue;

	    case 'A':			/* Ask user */
		Ask = YES;
		continue;
 
	    case 'v':			/* no virtual terminal */
		DoVirtTerm = NO;
		continue;

	    default:
		*p-- = '\0';
		fatal(MSGSTR(BADFLAG, "Bad flag -"), p);
	    }
	} else {
	    /* terminal type */
	    DefType = p;
	}
    }

    if (DefType) {
	if (Mapped) {
	    Map->Ident = "";		/* means "map any type" */
	    Map->Test = ALL;		/* at all baud rates */
	    Map->Type = DefType;	/* to the default type */
	}
	else
	    TtyType = DefType;
    }

    /*
     * Get rid of $TERMCAP, if it's there, so we get a real
     * entry from /etc/termcap. This prevents us from being
     * fooled by out of date stuff in the environment, and
     * makes tabs work right on CB/Unix.
     */
    bufp = getenv("TERMCAP");
    if (bufp && *bufp != '/')
	(void) strcpy(bufp-8, "NOTHING"); /* overwrite only "TERMCAP" */
    /* get current idea of terminal type from environment */
    if (!Dash_h && TtyType == 0)
	TtyType = getenv("TERM");

    /* determine terminal id if needed */
    if (!RepOnly && Ttyid == NOTTY && (TtyType == 0 || !Dash_h))
	Ttyid = ttyname(FILEDES);

# ifdef GTTYN
    /* If still undefined, look at /etc/ttytype */
    if (TtyType == 0) {
	TtyType = stypeof(Ttyid);
    }
# endif

    /* If still undefined, use DEFTYPE */
    if (TtyType == 0) {
	TtyType = DEFTYPE;
    }

    /* check for dialup or other mapping */
    if (Mapped) {
	if (!(Alias[0] && isalias(TtyType)))
	    if (tgetent(Capbuf, TtyType) > 0)
		makealias(Capbuf);
	TtyType = mapped(TtyType);
    }

    /* TtyType now contains a pointer to the type of the terminal */
    /* If the first character is '?', ask the user */
    if (TtyType[0] == '?') {
	Ask = YES;
	TtyType++;
	if (TtyType[0] == '\0')
	    TtyType = DEFTYPE;
    }
    if (Ask) {
    ask:
	prs("TERM = (");
	prs(TtyType);
	prs(") ");
	flush();

	/* read the terminal. If not empty, set type */
	i = read(2, termbuf, sizeof termbuf - 1);
	if (i > 0) {
	    if (termbuf[i - 1] == '\n')
		i--;
	    termbuf[i] = '\0';
	    if (termbuf[0] != '\0')
		TtyType = termbuf;
	}
    }

    /* get terminal capabilities */
    if (!(Alias[0] && isalias(TtyType))) {
	switch (tgetent(Capbuf, TtyType)) {
	case -1:
	    prs(MSGSTR(FIND, "Cannot find termcap\n"));
	    flush();
	    exit(-1);

	case 0:
	    prs(MSGSTR(TYPE, "Type "));
	    prs(TtyType);
	    prs(MSGSTR(UNKNOWN, " unknown\n"));
	    flush();
	    if (DoSetenv) {
		TtyType = DEFTYPE;
		Alias[0] = '\0';
		goto ask;
	    } else
		exit(1);
	}
    }
    Ttycap = Capbuf;

    if (!RepOnly) {
	/* determine erase and kill characters */
	if (Specialerase && !tgetflag("bs"))
	    Erase_char = 0;
	bufp = buf;
	p = tgetstr("kb", &bufp);
	if (p == NULL || p[1] != '\0')
	    p = tgetstr("bc", &bufp);
	if (p != NULL && p[1] == '\0')
	    bs_char = p[0];
	else if (tgetflag("bs"))
	    bs_char = BACKSPACE;
	else
	    bs_char = 0;
	if (Erase_char == 0 && !tgetflag("os") && curerase == OLDERASE) {
	    if (tgetflag("bs") || bs_char != 0)
		Erase_char = -1;
	}
	if (Erase_char == '\377')
	    Erase_char = (bs_char != 0) ? bs_char : BACKSPACE;

	if (curerase == 0)
	    curerase = CERASE;
	if (Erase_char != 0)
	    curerase = Erase_char;

	if (curintr == 0)
	    curintr = CINTR;
	if (Intr_char != 0)
	    curintr = Intr_char;

	if (curkill == 0)
	    curkill = CKILL;
	if (Kill_char != 0)
	    curkill = Kill_char;

	/* set modes */
	PadBaud = tgetnum("pb");	/* OK if fails */
	for (i=0; speeds[i].string; i++)
	    if (speeds[i].baudrate == PadBaud) {
		PadBaud = speeds[i].speed;
		break;
	    }
	setdelay("dC", CRdelay, CRbits, &mode.c_oflag);
	setdelay("dN", NLdelay, NLbits, &mode.c_oflag);
	setdelay("dB", BSdelay, BSbits, &mode.c_oflag);
	setdelay("dF", FFdelay, FFbits, &mode.c_oflag);
	setdelay("dT", TBdelay, TBbits, &mode.c_oflag);
	setdelay("dV", VTdelay, VTbits, &mode.c_oflag);

	if (tgetflag("UC") || (command[0] & 0140) == 0100) {
	    mode.c_iflag |= IUCLC;
	    mode.c_oflag |= OLCUC;
	} else if (tgetflag("LC")) {
	    mode.c_iflag &= ~IUCLC;
	    mode.c_oflag &= ~OLCUC;
	}
	mode.c_iflag &= ~(PARMRK|INPCK);
	mode.c_lflag |= ICANON;
	if (tgetflag("EP")) {
	    mode.c_cflag |= PARENB;
	    mode.c_cflag &= ~PARODD;
	}
	if (tgetflag("OP")) {
	    mode.c_cflag |= PARENB;
	    mode.c_cflag |= PARODD;
	}

	mode.c_oflag |= ONLCR;
	mode.c_iflag |= ICRNL;
	mode.c_lflag |= ECHO;
	mode.c_oflag |= TAB3;
	if (tgetflag("NL")) {		/* new line, not line feed */
	    mode.c_oflag &= ~ONLCR;
	    mode.c_iflag &= ~ICRNL;
	}
	if (tgetflag("HD"))		/* half duplex */
	    mode.c_lflag &= ~ECHO;
	if (tgetflag("pt"))		/* print tabs */
	    mode.c_oflag &= ~TAB3;
 
	mode.c_lflag |= (ECHOE|ECHOK);

	mode.c_lflag |= ECHOCTL;	/* display ctrl chars */
	if (tgetflag("hc")) {		/** set printer modes **/
	    mode.c_lflag &= ~(ECHOE|ECHOK);
	    mode.c_lflag |= ECHOPRT;
	} else {			/** set crt modes **/
	    if (!tgetflag("os")) {
		mode.c_lflag &= ~ECHOPRT;
		mode.c_lflag |= ECHOE;
		if (cfgetospeed(&mode) >= B1200)
		    mode.c_lflag |= ECHOK|ECHOKE;
	    }
	}

	/* get pad character */
	bufp = buf;
	if ((gstr=tgetstr("pc", &bufp)) != 0)
	    PC = *gstr;

	columns = tgetnum("co");
	lines = tgetnum("li");

	/* Set window size */
	(void) ioctl(FILEDES, TIOCGWINSZ, (char *)&win);
	if ((win.ws_row != lines || win.ws_col != columns) &&
	    lines > 0 && columns > 0) {
	    win.ws_row = lines;
	    win.ws_col = columns;
	    (void) ioctl(FILEDES, TIOCSWINSZ, (char *)&win);
	}
	/* output startup string */
	if (!NoInit) {
	    if (oldmode.c_oflag&(TAB3|ONLCR|OCRNL|ONLRET)) {
		oldmode.c_oflag &= (TAB3|ONLCR|OCRNL|ONLRET);
		setmode(-1);
	    }
	    bufp = buf;
	    if ( IsReset ) { /* RESET (reset) */
		if ( gstr=tgetstr("r1", &bufp) ) {
		    tputs(gstr, 0, prc);
		    bufp = buf;
		}
		if ( gstr=tgetstr("r2", &bufp) ) {
		    tputs(gstr, 0, prc);
		    bufp = buf;
		}
		if ( gstr=tgetstr("rf", &bufp)  ) {
		    cat(gstr);
		    bufp = buf;
		}
		if ( gstr=tgetstr("r3", &bufp) ) {
		    tputs(gstr, 0, prc);
		    bufp = buf;
		}
		settle = YES;
		flush();
	    } else { 	/* INITIALIZE (tset) */
		if ( (gstr=tgetstr("iP",&bufp)) && strlen(gstr) ) {
		    int cpid;
		    if ( cpid=fork() && cpid>0 )  /* parent - good fork */
			wait();
		    else if ( cpid == 0 ) { /* child - run iprog */
			execlp(gstr,(char *)0);
			prs(MSGSTR(BADEXE,
				"Could not find executable for iprog: "));
			prs(gstr);
			prs("\n");
			exit(1);
		    }
		    else
			prs(MSGSTR(BADFORK,"Error forking process\n"));
		    flush();
		}
		if ( gstr=tgetstr("i1",&bufp) ) {
		    tputs(gstr, 0, prc);
		    bufp = buf;
		    settle = YES;
		}
		if ( gstr=tgetstr("is",&bufp) ) { /* is=is2 in tgetstr() */
		    tputs(gstr, 0, prc);
		    bufp = buf;
		    settle = YES;
		}
		if ( settabs() ) {	/* set tabs - tbc(tc) and hts(st) */
		    settle = YES;
		    flush();
		}
		if ( gstr=tgetstr("if", &bufp) ) {
			cat(gstr);
		    	bufp = buf;
			settle = YES;
		}
		if ( gstr=tgetstr("i2",&bufp) ) { /* i2=is3 in tgetstr() */
		    tputs(gstr, 0, prc);
		    bufp = buf;
		    settle = YES;
		}
	    }

	    if (settle) {
		prc('\r');
		flush();
		sleep(1);		/* let terminal settle down */
	    }
	}

	setmode(0);			/* set new modes, if they've changed */

	/* set up environment for the shell we are using */
	/* (this code is rather heuristic, checking for $SHELL */
	/* ending in the 3 characters "csh") */
	csh = NO;
	if (DoSetenv) {
	    char *sh;

	    if ((sh = getenv("SHELL")) && (i = strlen(sh)) >= 3) {
		if ((csh = sequal(&sh[i-3], "csh")) && CmndLine)
		    (void) write(STDOUT, "set noglob;\n", 12);
	    }
#ifdef _AIX
	    if (!csh)
		/* running Bourne shell */
		(void) write(STDOUT, "export TERM;\n", 13);
#else
	    if (!csh)
		/* running Bourne shell */
		(void) write(STDOUT, "export TERMCAP TERM;\n", 21);
#endif
	}
    }

    /* report type if appropriate */
    if (DoSetenv || Report || Ureport) {
	/* if type is the short name, find first alias (if any) */
	makealias(Ttycap);
	if (sequal(TtyType, Alias[0]) && Alias[1]) {
	    TtyType = Alias[1];
	}

	if (DoSetenv) {
	    if (csh) {
		if (CmndLine)
		    (void) write(STDOUT, "setenv TERM ", 12);
		(void) write(STDOUT, TtyType, strlen(TtyType));
		(void) write(STDOUT, " ", 1);
		if (CmndLine)
		    (void) write(STDOUT, ";\n", 2);
	    } else {
		(void) write(STDOUT, "TERM=", 5);
		(void) write(STDOUT, TtyType, strlen(TtyType));
		(void) write(STDOUT, ";\n", 2);
	    }
	} else if (Report) {
	    (void) write(STDOUT, TtyType, strlen(TtyType));
	    (void) write(STDOUT, "\n", 1);
	}
	if (Ureport) {
	    prs(MSGSTR(TERMTYPE, "Terminal type is "));
	    prs(TtyType);
	    prs("\n");
	    flush();
	}

#ifdef _AIX
        if ( DoSetenv && csh && CmndLine )
            (void) write(STDOUT, "unset noglob;\n", 14);
#else
	if (DoSetenv) {
	    if (csh) {
		if (CmndLine)
		    (void) write(STDOUT, "setenv TERMCAP '", 16);
	    } else
		(void) write(STDOUT, "TERMCAP='", 9);
	    wrtermcap(Ttycap);
	    if (csh) {
		if (CmndLine) {
		    (void) write(STDOUT, "';\n", 3);
		    (void) write(STDOUT, "unset noglob;\n", 14);
		}
	    } else
		(void) write(STDOUT, "';\n", 3);
	}
#endif
    }

    if (RepOnly)
	exit(0);

    /* tell about changing erase, kill and interrupt characters */
    reportek("Erase", curerase, olderase, (cc_t)OLDERASE);
    reportek("Kill", curkill, oldkill, (cc_t)OLDKILL);
    reportek("Interrupt", curintr, oldintr, (cc_t)OLDINTR);

    exit(0);
}

/*
 * Set the hardware tabs on the terminal, using the ct (clear all tabs),
 * st (set one tab) and ch (horizontal cursor addressing) capabilities.
 * This is done before if and is, so they can patch in case we blow this.
 */
settabs()
{
    char caps[100];
    char *capsp = caps;
    char *clear_tabs, *set_tab, *set_column, *set_pos;
    char *tg_out, *tgoto();
    int c;

    clear_tabs = tgetstr("ct", &capsp);
    set_tab = tgetstr("st", &capsp);
    set_column = tgetstr("ch", &capsp);
    if (set_column == 0)
	set_pos = tgetstr("cm", &capsp);

    if (clear_tabs && set_tab) {
	prc('\r');			/* force to be at left margin */
	tputs(clear_tabs, 0, prc);
    }
    if (set_tab) {
	for (c=8; c<columns; c += 8) {
	    /* get to that column. */
	    tg_out = "OOPS";		/* also returned by tgoto */
	    if (set_column)
		tg_out = tgoto(set_column, 0, c);
	    if (*tg_out == 'O' && set_pos)
		tg_out = tgoto(set_pos, c, lines-1);
	    if (*tg_out != 'O')
		tputs(tg_out, 1, prc);
	    else {
		prc(' '); prc(' '); prc(' '); prc(' ');
		prc(' '); prc(' '); prc(' '); prc(' ');
	    }
	    /* set the tab */
	    tputs(set_tab, 0, prc);
	}
	prc('\r');
	return 1;
    }
    return 0;
}

/* 
 * flag serves several purposes: if called as the result of a signal,
 * flag will be > 0.  if called from terminal init, flag == -1 means
 * reset "oldmode".  called with flag == 0 at end of normal mode
 * processing.
 */
setmode(int flag)
{
    struct termios *ttymode;

    if (flag < 0)			/* unconditionally reset oldmode */
	ttymode = &oldmode;
    else if (!bequal((char *)&mode, (char *)&oldmode, (int)sizeof mode))
	ttymode = &mode;
    else				/* don't need it */
	ttymode = (struct termios *)0;
 
    if (ttymode)
	tcsetattr(FILEDES, TCSADRAIN, ttymode);


    if (flag > 0)			/* trapped signal */
	exit(1);
}

reportek(char *name, cc_t new, cc_t old, cc_t def)
{
    register char o;
    register char n;
    char p[2];
    char buf[32];
    char *bufp;

    if (BeQuiet)
	return;
    o = old;
    n = new;

    if (o == n && n == def)
	return;
    prs(name);
    if (o == n)
	prs(MSGSTR(IS, " is "));
    else
	prs(MSGSTR(SETTO, " set to "));
    bufp = buf;
    if (tgetstr("kb", &bufp) && n == buf[0] && buf[1] == '\0')
	prs("Backspace");
    else if (n == 0177)
	prs("Delete");
    else {
	if (n < 040) {
	    prs("Ctrl-");
	    n ^= 0100;
	}
	p[0] = n;
	p[1] = '\0';
	prs(p);
    }
    prs("\n");
    flush();
}

setdelay(char *cap, struct delay dtab[], int bits, tcflag_t *flags)
{
    register int i;
    register struct delay *p;

    /* see if this capability exists at all */
    i = tgetnum(cap);
    if (i < 0)
	i = 0;
    /* No padding at speeds below PadBaud */
    if (PadBaud > ospeed)
	i = 0;

    /* clear out the bits, replace with new ones */
    *flags &= ~bits;

    /* scan dtab for first entry with adequate delay */
    for (p = dtab; p->d_delay >= 0; p++) {
	if (p->d_delay >= i) {
	    p++;
	    break;
	}
    }

    /* use last entry if none will do */
    *flags |= (--p)->d_bits;
}

prs(char *s)
{
    while (*s != '\0')
	prc(*s++);
}

char OutBuf[256];
int OutPtr;

prc(char c)
{

    if(Debugflg) {
	putc(c,stdout);
    } else {
	putc(c,stderr);
	fflush(stderr);
    }
}

flush()
{
    if (OutPtr > 0)
	(void) write(2, OutBuf, OutPtr);
    OutPtr = 0;
}

cat(char *file)
{
    register int fd;
    register int i;
    char buf[BUFSIZ];

    fd = open(file, 0);
    if (fd < 0) {
	prs(MSGSTR(CANTOPEN, "Cannot open "));
	prs(file);
	prs("\n");
	flush();
	return;
    }

    while ((i = read(fd, buf, BUFSIZ)) > 0)
	(void) write(FILEDES, buf, i);

    (void) close(fd);
}

bmove(char *from, char *to, int length)
{
    register char *p, *q;
    register int i;

    i = length;
    p = from;
    q = to;

    while (i-- > 0)
	*q++ = *p++;
}



bequal(char *a, char *b, int len) /* must be same thru len chars */
{
    register char *p, *q;
    register int i;

    i = len;
    p = a;
    q = b;

    while ((*p == *q) && --i > 0) {
	p++; q++;
    }
    return ((*p == *q) && i >= 0);
}

sequal(char *a, char *b) /* must be same thru NULL */
{
    register char *p = a, *q = b;

    while (*p && *q && (*p == *q)) {
	p++; q++;
    }
    return (*p == *q);
}

makealias(char *buf)
{
    register int i;
    register char *a;
    register char *b;

    Alias[0] = a = Aliasbuf;
    b = buf;
    i = 1;
    while (*b && *b != ':') {
	if (*b == '|') {
	    *a++ = '\0';
	    Alias[i++] = a;
	    b++;
	}
	else
	    *a++ = *b++;
    }
    *a = '\0';
    Alias[i] = NULL;
# ifdef DEB
    for(i = 0; Alias[i]; printf("A:%s\n", Alias[i++]));
# endif
}

isalias(char *ident) /* is ident same as one of the aliases? */
{
    char **a = Alias;

    if (*a)
	while (*a)
	    if (sequal(ident, *a))
		return(YES);
	    else
		a++;
    return(NO);
}

# ifdef GTTYN
char *stypeof(char *ttyid)
{
    register char *PortType;
    register char *TtyId;
    struct ttyent *t;

    if (ttyid == NOTTY)
	return (DEFTYPE);

    /* split off end of name */
    TtyId = ttyid;
    while (*ttyid)
	if (*ttyid++ == '/')
	    TtyId = ttyid;

    /* scan the file */
    if ((t = getttynam(TtyId)) != NULL) {
	PortType = t->ty_type;
	/* get aliases from termcap entry */
	if (Mapped && tgetent(Capbuf, PortType) > 0) {
	    makealias(Capbuf);
	    if (sequal(Alias[0], PortType) && Alias[1])
		PortType = Alias[1];
	}
	return (PortType);
    }
    return (DEFTYPE);
}
# endif

/*
 * routine to output the string for the environment TERMCAP variable
 */
#define WHITE(c) (c == ' ' || c == '\t')
char delcap[128][2];
int ncap = 0;

wrtermcap(char *bp)
{
    char buf[CAPBUFSIZ];
    char *p = buf;
    char *tp;
    int space, empty;

    /* discard names with blanks */
    /** May not be desireable ? **/
    while (*bp && *bp != ':') {
	if (*bp == '|') {
	    tp = bp+1;
	    space = NO;
	    while (*tp && *tp != '|' && *tp != ':') {
		space = (space || WHITE(*tp) );
		tp++;
	    }
	    if (space) {
		bp = tp;
		continue;
	    }
	}
	*p++ = *bp++;
    }
    /**/

    while (*bp) {
	switch (*bp) {
	case ':':			/* discard empty, cancelled or dupl fields */
	    tp = bp+1;
	    empty = YES;
	    while (*tp && *tp != ':') {
		empty = (empty && WHITE(*tp) );
		tp++;
	    }
	    if (empty || cancelled(bp+1)) {
		bp = tp;
		continue;
	    }
	    break;

	case ' ':			/* no spaces in output */
	    p = putbuf(p, "\\040");
	    bp++;
	    continue;

	case '!':			/* the shell thinks this is history */
	    p = putbuf(p, "\\041");
	    bp++;
	    continue;

	case ',':			/* the shell thinks this is history */
	    p = putbuf(p, "\\054");
	    bp++;
	    continue;

	case '"':			/* no quotes in output */
	    p = putbuf(p, "\\042");
	    bp++;
	    continue;

	case '\'':			/* no quotes in output */
	    p = putbuf(p, "\\047");
	    bp++;
	    continue;

	case '`':			/* no back quotes in output */
	    p = putbuf(p, "\\140");
	    bp++;
	    continue;

	case '\\':
	case '^':			/* anything following is OK */
	    *p++ = *bp++;
	}
	*p++ = *bp++;
    }
    *p++ = ':';				/* we skipped the last : with the : lookahead hack */
    (void) write (STDOUT, buf, p-buf);
}

cancelled(char *cap)
{
    register int i;

    for (i = 0; i < ncap; i++) {
	if (cap[0] == delcap[i][0] && cap[1] == delcap[i][1])
	    return (YES);
    }
    /* delete a second occurrance of the same capability */
    delcap[ncap][0] = cap[0];
    delcap[ncap][1] = cap[1];
    ncap++;
    return (cap[2] == '@');
}

char *putbuf(char *ptr, char *str)
{
    char buf[20];

    while (*str) {
	switch (*str) {
	case '\033':
	    ptr = putbuf(ptr, "\\E");
	    str++;
	    break;
	default:
	    if (*str <= ' ') {
		(void) sprintf(buf, "\\%03o", *str);
		ptr = putbuf(ptr, buf);
		str++;
	    } else
		*ptr++ = *str++;
	}
    }
    return (ptr);
}

baudrate(char *p)
{
    char buf[8];
    int i = 0;

    while (i < 7 && (isalnum(*p) || *p == '.'))
	buf[i++] = *p++;
    buf[i] = '\0';
    for (i=0; speeds[i].string; i++)
	if (sequal(speeds[i].string, buf))
	    return (speeds[i].speed);
    return (-1);
}

char *mapped(char *type)
{
    int match;

# ifdef DEB
    printf ("spd:%d\n", ospeed);
    prmap();
# endif
    Map = map;
    while (Map->Ident) {
	if (*(Map->Ident) == '\0' || sequal(Map->Ident, type) || isalias(Map->Ident)) {
	    match = NO;
	    switch (Map->Test) {
	    case ANY:			/* no test specified */
	    case ALL:
		match = YES;
		break;
 
	    case GT:
		match = (ospeed > Map->Speed);
		break;

	    case GE:
		match = (ospeed >= Map->Speed);
		break;

	    case EQ:
		match = (ospeed == Map->Speed);
		break;

	    case LE:
		match = (ospeed <= Map->Speed);
		break;

	    case LT:
		match = (ospeed < Map->Speed);
		break;

	    case NE:
		match = (ospeed != Map->Speed);
		break;
	    }
	    if (match)
		return (Map->Type);
	}
	Map++;
    }
    /* no match found; return given type */
    return (type);
}

# ifdef DEB
prmap()
{
    Map = map;
    while (Map->Ident) {
	printf ("%s t:%d s:%d %s\n",
		Map->Ident, Map->Test, Map->Speed, Map->Type);
	Map++;
    }
}
# endif

char *nextarg(int argc, char *argv[])
{
    if (argc <= 0)
	fatal (MSGSTR(TOOFEW, "Too few args: "), *argv);
    if (*(*++argv) == '-')
	fatal (MSGSTR(UNEXPECT, "Unexpected arg: "), *argv);
    return (*argv);
}

fatal(char *mesg, char *obj)
{
    prs (mesg);
    prs (obj);
    prc ('\n');
    prs (USAGE);
    flush();
    exit(1);
}

debugchk()
{
    char *cp;

    if((cp = getenv("_TSET_DEBUG")) && strcmp(cp,"_TSET_DEBUG") == 0) {
	Debugflg++;
	FILEDES = 0;
    }
}

