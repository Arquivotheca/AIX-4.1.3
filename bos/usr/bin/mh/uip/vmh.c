static char sccsid[] = "@(#)94  1.16  src/bos/usr/bin/mh/uip/vmh.c, cmdmh, bos411, 9428A410j 11/9/93 09:45:26";
/* 
 * COMPONENT_NAME: CMDMH vmh.c
 * 
 * FUNCTIONS: ALRMser, MSGSTR, Mvmh, PEERinit, PIPEser, SIGser, 
 *            TSTPser, TTYinit, TTYoff, TTYon, WINgetstr, WINinit, 
 *            WINless, WINputc, WINwritev, abs, adorn, advertise, 
 *            done, foreground, ladvance, lgo, linsert, lreset, 
 *            lretreat, myputchar, pFIN, pINI, pLOOP, pTTY, pWIN, 
 *            pWINaux, sideground, sigmask, vmh, writev 
 *
 * ORIGINS: 10  26  27  28  35 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* static char sccsid[] = "vmh.c	8.1 88/04/15 15:50:59"; */

/* vmh.c - visual front-end to mh */

/* TODO:
	Pass signals to client during execution

	Get stand-alone SO/SE/CE to work under #ifdef SYS5

	Figure out a way for the user to say how big the Scan/Display
	windows should be.

	If curses ever gets fixed, then XYZ code can be removed
 */

#ifdef _AIX
#define SYS5
#endif

/* #include <cur00.h> */
#include <curses.h>
#undef	OK			/* tricky */
#include "mh.h"
#include "vmhsbr.h"
#include <ctype.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>

#ifdef _AIX
#undef  TIOCGPGRP
#undef  TIOCGLTC
#undef  SIGTSTP
#endif

#include "mh_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 

#ifndef	sigmask
#define	sigmask(s)	(1 << ((s) - 1))
#endif	not sigmask
#ifndef	BSD42
struct iovec {
    char   *iov_base;
    int     iov_len;
};
#else	BSD42
#include <sys/types.h>
#include <sys/uio.h>
#endif	BSD42

#define	ALARM	((unsigned int) 10)
#define	PAUSE	((unsigned int) 2)

#define	abs(a)		((a) > 0 ? (a) : -(a))
#define	SMALLMOVE	1
#define	LARGEMOVE	10


#define	XYZ			/* XXX */

/*  */

static struct swit switches[] = {
#define	PRMPTSW	0
    "prompt string", 6,

#define	PROGSW	1
    "vmhproc program", 7,
#define	NPROGSW	2
    "novmhproc", 9,

#define	HELPSW	3
    "help", 4,

    NULL, (int)NULL
};

/*  */
					/* PEERS */
static int  PEERpid = NOTOK;

static  jmp_buf PEERctx;


					/* WINDOWS */
static char *myprompt = "(%s) ";

static  WINDOW *Scan;
static  WINDOW *Status;
static  WINDOW *Display;
static  WINDOW *Command;

#define	NWIN	3
static	int numwins;
WINDOW *windows[NWIN + 1];


					/* LINES */

struct line {
    int     l_no;
    char   *l_buf;
    struct line *l_prev;
    struct line *l_next;
};

static struct line *lhead = NULL;
static struct line *ltop = NULL;
static struct line *ltail = NULL;

static int did_less = 0;
static int smallmove = SMALLMOVE;
static int largemove = LARGEMOVE;


					/* TTYS */

static int  tty_ready = NOTOK;

static int  intrc;
#ifndef	SYS5
#define	ERASE	sg.sg_erase
#define	KILL	sg.sg_kill
static struct sgttyb    sg;

#define	EOFC	tc.t_eofc
#define	INTR	tc.t_intrc
static struct tchars    tc;
#else	SYS5
#define	ERASE	sg.c_cc[VERASE]
#define	KILL	sg.c_cc[VKILL]
#define	EOFC	sg.c_cc[VEOF]
#define	INTR	sg.c_cc[VINTR]
static struct termio    sg;
#endif	SYS5

#ifndef	TIOCGLTC
#define	WERASC	('W' & 037)
#else	TIOCGLTC
#define	WERASC	ltc.t_werasc
static struct ltchars ltc;
#endif	TIOCGLTC


static int myputchar();

char *strchr();

#define _putchar myputchar

#ifndef	SYS5
int	_putchar ();
#endif	not SYS5
char   *tgoto ();


					/* SIGNALS */
static void     ALRMser (int), PIPEser (int), SIGser (int);
#ifdef	SIGTSTP
int	TSTPser ();
#endif	SIGTSTP


					/* MISCELLANY */
extern int  errno;
extern int  sys_nerr;

static void adorn();

static vmh(), lreset(), linsert(), TTYon(), TTYoff(), 
       foreground();

static int PEERinit(), pINI(), pLOOP(), pTTY(), WINgetstr(),
           pWIN(), pWINaux(), WINinit(), WINputc(), WINless(),
	   ladvance(), lretreat(), lgo(), TTYinit();

/*  */

/* ARGSUSED */

main (argc, argv)
int     argc;
char   *argv[];
{
    int     vecp = 1,
	    nprog = 0;
    char   *cp,
            buffer[BUFSIZ],
          **ap,
          **argp,
           *arguments[MAXARGS],
           *vec[MAXARGS];

    setlocale(LC_ALL,"");
    catd = catopen(MF_MH, NL_CAT_LOCALE);

    invo_name = r1bindex (argv[0], '/');
    if ((cp = m_find (invo_name)) != NULL) {
	ap = brkstring (cp = getcpy (cp), " ", "\n");
	ap = copyip (ap, arguments);
    }
    else
	ap = arguments;
    (void) copyip (argv + 1, ap);
    argp = arguments;

/*  */

    while (cp = *argp++)
	if (*cp == '-')
	    switch (smatch (++cp, switches)) {
		case AMBIGSW: 
		    ambigsw (cp, switches);
		    done (1);
		case UNKWNSW: 
		    vec[vecp++] = --cp;
		    continue;
		case HELPSW: 
		    (void) sprintf (buffer, MSGSTR(HELPSW5, "%s [switches for vmhproc]"), invo_name); /*MSG*/
		    help (buffer, switches);
		    done (1);

		case PRMPTSW:
		    if (!(myprompt = *argp++) || *myprompt == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    continue;

		case PROGSW: 
		    if (!(vmhproc = *argp++) || *vmhproc == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    continue;
		case NPROGSW:
		    nprog++;
		    continue;
	    }
	else
	    vec[vecp++] = cp;

/*  */

    if (TTYinit (nprog) == NOTOK || WINinit (nprog) == NOTOK) {
	vec[vecp] = NULL;

	vec[0] = r1bindex (vmhproc, '/');
	execvp (vmhproc, vec);
	adios (vmhproc, MSGSTR(UNEXEC, "unable to exec %s"), vmhproc); /*MSG*/
    }
    TTYoff ();
    (void) PEERinit (vecp, vec);
    TTYon ();

    vmh ();

    done (0);
}

/*  */

static  vmh () {
    char    buffer[BUFSIZ];

    for (;;) {
	(void) pLOOP (RC_QRY, NULLCP);

	wmove (Command, 0, 0);
	wprintw (Command, myprompt, invo_name);
	wclrtoeol (Command);
	wrefresh (Command);

	switch (WINgetstr (Command, buffer)) {
	    case NOTOK: 
		break;

	    case OK:
		done (0);	/* NOTREACHED */

	    default: 
		if (*buffer)
		    (void) pLOOP (RC_CMD, buffer);
		break;
	}
    }
}

/*    PEERS */

static int  PEERinit (vecp, vec)
int	vecp;
char   *vec[];
{
    int	    pfd0[2],
            pfd1[2];
    char    buf1[BUFSIZ],
            buf2[BUFSIZ];

    if (pipe (pfd0) == NOTOK || pipe (pfd1) == NOTOK)
	adios ("pipe", MSGSTR(NOPIPE, "unable to pipe")); /*MSG*/
    switch (PEERpid = vfork ()) {
	case NOTOK: 
	    adios ("vfork", MSGSTR(NOVFORK, "unable to vfork"));/* NOTREACHED */ /*MSG*/

	case OK: 
	    (void) close (pfd0[0]);
	    (void) close (pfd1[1]);

	    vec[vecp++] = "-vmhread";
	    (void) sprintf (buf1, "%d", pfd1[0]);
	    vec[vecp++] = buf1;
	    vec[vecp++] = "-vmhwrite";
	    (void) sprintf (buf2, "%d", pfd0[1]);
	    vec[vecp++] = buf2;
	    vec[vecp] = NULL;

	    (void) signal (SIGINT, SIG_DFL);
	    (void) signal (SIGQUIT, SIG_DFL);

	    vec[0] = r1bindex (vmhproc, '/');
	    execvp (vmhproc, vec);
	    perror (vmhproc);
	    _exit (-1);		/* NOTREACHED */

	default: 
	    (void) close (pfd0[1]);
	    (void) close (pfd1[0]);

	    (void) rcinit (pfd0[0], pfd1[1]);
	    return pINI ();
    }
}

/*  */

static int  pINI () {
    register char  *bp;
    char    buffer[BUFSIZ];
    struct record   rcs;
    register struct record *rc = &rcs;
    register    WINDOW **w;

    initrc (rc);

    bp = buffer;
    (void) sprintf (bp, "%d %d", RC_VRSN, numwins);
    bp += strlen (bp);
    for (w = windows; *w; w++) {
	(void) sprintf (bp, " %d", (*w) -> _maxy);
	bp += strlen (bp);
    }

    switch (str2rc (RC_INI, buffer, rc)) {
	case RC_ACK: 
	    return OK;

	case RC_ERR: 
	    if (rc -> rc_len)
		adios (NULLCP, "%s", rc -> rc_data);
	    else
		adios (NULLCP, MSGSTR(PERROR, "pINI peer error")); /*MSG*/

	case RC_XXX: 
	    adios (NULLCP, "%s", rc -> rc_data);

	default:
	    adios (NULLCP, MSGSTR(PROTOSU4, "pINI protocol screw-up")); /*MSG*/
    }
/* NOTREACHED */
}

/*  */

static int  pLOOP (code, str)
char	code,
       *str;
{
    int	    i;
    struct record   rcs;
    register struct record *rc = &rcs;

    initrc (rc);

    (void) str2peer (code, str);
    for (;;)
	switch (peer2rc (rc)) {
	    case RC_TTY:
		if (pTTY (rc) == NOTOK)
		    return NOTOK;
		break;

	    case RC_WIN:
		if (sscanf (rc -> rc_data, "%d", &i) != 1
			|| i <= 0
			|| i > numwins) {
		    (void) fmt2peer (RC_ERR, MSGSTR(NOWINDOW, "no such window \"%s\""), rc -> rc_data); /*MSG*/
		    return NOTOK;
		}
		if (pWIN (windows[i - 1]) == NOTOK)
		    return NOTOK;
		break;

	    case RC_EOF:
		return OK;

	    case RC_ERR:
		if (rc -> rc_len)
		    adorn(NULLCP, "%s", rc -> rc_data);
		else {
		    if (code == RC_QRY)
		        adorn (NULLCP, MSGSTR(QRYPERR, "pLOOP(QRY) peer error")); /*MSG*/
		    else
		        adorn (NULLCP, MSGSTR(CMDPERR, "pLOOP(CMD) peer error")); /*MSG*/
		}
		return NOTOK;

	    case RC_FIN:
		if (rc -> rc_len)
		    adorn (NULLCP, "%s", rc -> rc_data);
		(void) rcdone ();
		i = pidwait (PEERpid, OK);
		PEERpid = NOTOK;
		done (i);

	    case RC_XXX: 
		adios (NULLCP, "%s", rc -> rc_data);

	    default:
		if (code == RC_QRY)
		    adios (NULLCP, MSGSTR(QRYPROTOSU, "pLOOP(QRY) protocol screw-up")); /*MSG*/
		else
		    adios (NULLCP, MSGSTR(CMDPROTOSU, "pLOOP(CMD) protocol screw-up")); /*MSG*/
	}
}

/*  */

static int  pTTY (r)
register struct record *r;
{
    void     (*hstat) (int), (*istat) (int), (*qstat) (int), (*tstat) (int);
    struct record   rcs;
    register struct record *rc = &rcs;

    initrc (rc);

    TTYoff ();

    hstat = signal (SIGHUP, SIG_IGN);
    istat = signal (SIGINT, SIG_IGN);
    qstat = signal (SIGQUIT, SIG_IGN);
    tstat = signal (SIGTERM, SIG_IGN);

    (void) rc2rc (RC_ACK, 0, NULLCP, rc);

    (void) signal (SIGHUP, (void(*)(int))hstat);
    (void) signal (SIGINT, (void(*)(int))istat);
    (void) signal (SIGQUIT, (void(*)(int))qstat);
    (void) signal (SIGTERM, (void(*)(int))tstat);

    TTYon ();

    if (r -> rc_len && strcmp (r -> rc_data, "FAST") == 0)
	goto no_refresh;

#ifdef	SIGTSTP
    (void) signal (SIGTSTP, SIG_IGN);
#endif	SIGTSTP
#ifndef	SYS5
    if (SO)
	tputs (SO, 0, _putchar);
#endif	not SYS5
    fprintf (stdout, MSGSTR(ANYKEY, "Type any key to continue... ")); /*MSG*/
    (void) fflush (stdout);
#ifndef	SYS5
    if (SE)
	tputs (SE, 0, _putchar);
#endif	not SYS5
    (void) getc (stdin);
#ifdef	SIGTSTP
    (void) signal (SIGTSTP, TSTPser);
#endif	SIGTSTP

    wrefresh (curscr);

no_refresh: ;
    switch (rc -> rc_type) {
	case RC_EOF: 
	    (void) rc2peer (RC_ACK, 0, NULLCP);
	    return OK;

	case RC_ERR: 
	    if (rc -> rc_len)
		adorn (NULLCP, "%s", rc -> rc_data);
	    else
		adorn (NULLCP, MSGSTR(PEERERR, "pTTY peer error")); /*MSG*/
	    return NOTOK;

	case RC_XXX: 
	    adios (NULLCP, "%s", rc -> rc_data);

	default:
	    adios (NULLCP, MSGSTR(PROTOSU2, "pTTY protocol screw-up")); /*MSG*/
    }
/* NOTREACHED */
}

/*  */

static int  pWIN (w)
register WINDOW *w;
{
    int     i;

    did_less = 0;
    if ((i = pWINaux (w)) == OK && did_less)
	(void) WINless (w, 1);

    lreset ();

    return i;
}

/*  */

static int  pWINaux (w)
register WINDOW *w;
{
    register int    n;
    int	    eol;
    register char   c,
	           *ck,
                   *bp;
    struct record   rcs;
    register struct record *rc = &rcs;

    initrc (rc);

    werase (w);
    wmove (w, 0, 0);
#ifdef	XYZ
    if (w == Status)
	wstandout (w);
#endif	XYZ

    for (eol = 0;;)
	switch (rc2rc (RC_ACK, 0, NULLCP, rc)) {
	    case RC_DATA: 
/* These mods with ck are to replace a curses call which is failing
*  in linsert.
*/
		ck = rc -> rc_data;
		if (eol && WINputc (w, '\n') == ERR && WINless (w, 0))
		    goto flush;
		for (bp = rc -> rc_data, n = rc -> rc_len; n-- > 0; ) {
		    if ((c = *bp++) == '\n') {
			char *ptr;
			ptr = strchr(ck,'\n');
			*ptr = '\0';
		        linsert (w,ck);
			ck = bp;
			}
		    if (WINputc (w, c) == ERR)
			if (n == 0 && c == '\n')
			    eol++;
			else
			    if (WINless (w, 0)) {
flush: ;
				(void) fmt2peer (RC_ERR, MSGSTR(FLUSH, "flush window")); /*MSG*/
#ifdef	XYZ			/* should NEVER happen... */
				if (w == Status)
				    wstandend (w);
#endif	XYZ
				wrefresh (w);
				return NOTOK;
			    }
		}
		break;

	    case RC_EOF: 
		(void) rc2peer (RC_ACK, 0, NULLCP);
#ifdef	XYZ
		if (w == Status)
		    wstandend (w);
#endif	XYZ
		wrefresh (w);
		return OK;

	    case RC_ERR: 
		if (rc -> rc_len)
		    adorn (NULLCP, "%s", rc -> rc_data);
		else
		    adorn (NULLCP, MSGSTR(PEERERR2, "pWIN peer error")); /*MSG*/
		return NOTOK;

	    case RC_XXX: 
		adios (NULLCP, "%s", rc -> rc_data);

	    default:
		adios (NULLCP, MSGSTR(PROTOSU3, "pWIN protocol screw-up")); /*MSG*/
	}
/* NOTREACHED */
}

/*  */

static int  pFIN () {
    int     status;

    if (PEERpid <= OK)
	return OK;

    (void) rc2peer (RC_FIN, 0, NULLCP);
    (void) rcdone ();

    switch (setjmp (PEERctx)) {
	case OK: 
	    (void) signal (SIGALRM, (void(*)(int))ALRMser);
	    (void) alarm (ALARM);

	    status = pidwait (PEERpid, OK);

	    (void) alarm (0);
	    break;

	default: 
	    (void) kill (PEERpid, SIGKILL);
	    status = NOTOK;
	    break;
    }
    PEERpid = NOTOK;

    return status;
}

/*    WINDOWS */

static int  WINinit (nprog) {
    register int    lines,
                    top,
                    bottom;

    foreground ();
/* Replace following lines in favor of the simple curses call. 
*    if (initscr () == ERR)
*	if (nprog)
*	    return NOTOK;
*	else
*	    adios (NULLCP, "could not initialize terminal");
*/
	initscr();
#ifdef	SIGTSTP
    (void) signal (SIGTSTP, SIG_DFL);
#endif	SIGTSTP
    sideground ();

/* Replace following lines in favor of the simple curses call. 
*    if (CM == NULL)
*	if (nprog)
*	    return NOTOK;
*	else
*	    adios (NULLCP,
*		    "sorry, your terminal isn't powerful enough to run %s",
*		    invo_name);
*/
#ifndef	SYS5
    if (tgetflag ("xt") || tgetnum ("sg") > 0)
	SO = SE = US = UE = NULL;
#endif	not SYS5

    if ((lines = LINES - 1) < 11)
	adios (NULLCP, MSGSTR(SMSCREEN, "screen too small")); /*MSG*/
    if ((top = lines / 3 + 1) > LINES / 4 + 2)
	top--;
    bottom = lines - top - 2;

    numwins = 0;
    Scan = windows[numwins++] = newwin (top, COLS, 0, 0);
    Status = windows[numwins++] = newwin (1, COLS, top, 0);
#ifndef	XYZ
    wstandout (Status);
#endif	XYZ
    Display = windows[numwins++] = newwin (bottom, COLS, top + 1, 0);
    Command = newwin (1, COLS - 1, top + 1 + bottom, 0);
    windows[numwins] = NULL;

    largemove = Display -> _maxy / 2 + 2;
    return OK;
}

/*  */

static int WINgetstr (w, buffer)
register WINDOW *w;
char   *buffer;
{
    register int    c;
    register char  *bp;

    bp = buffer;
    *bp = (char)NULL;

    for (;;) {
	switch (c = toascii (wgetch (w))) {
	    case ERR: 
		adios (NULLCP, MSGSTR(LOST2, "wgetch lost")); /*MSG*/

	    case '\f':
		wrefresh (curscr);
		break;

	    case '\r': 
	    case '\n': 
		*bp = (char)NULL;
		if (bp > buffer) {
		    leaveok (curscr, FALSE);
		    wmove (w, 0, w -> _curx - (bp - buffer));
		    wrefresh (w);
		    leaveok (curscr, TRUE);
		}
		return DONE;

	    default: 
		if (c == intrc) {
		    wprintw (w, " ");
		    wstandout (w);
		    wprintw (w, "Interrupt");
		    wstandend (w);
		    wrefresh (w);
		    *buffer = (char)NULL;
		    return NOTOK;
		}
		if (c == EOFC) {
		    if (bp <= buffer)
			return OK;
		    break;
		}
		if (c == ERASE) {
		    if (bp <= buffer)
			continue;
		    bp--, w -> _curx--;
		    wclrtoeol (w);
		    break;
		}
		if (c == KILL) {
		    if (bp <= buffer)
			continue;
		    w -> _curx -= bp - buffer;
		    bp = buffer;
		    wclrtoeol (w);
		    break;
		}
		if (c == WERASC) {
		    if (bp <= buffer)
			continue;
		    do {
			bp--, w -> _curx--;
		    } while (isspace (*bp) && bp > buffer);

		    if (bp > buffer) {
			do {
			    bp--, w -> _curx--;
			} while (!isspace (*bp) && bp > buffer);
			if (isspace (*bp))
			    bp++, w -> _curx++;
		    }
		    wclrtoeol (w);
		    break;
		}
		
		if (c >= ' ')
		    (void) waddch (w, *bp++ = c);
		break;
	}

	wrefresh (w);
    }
}

/*  */

static int  WINwritev (w, iov, n)
register WINDOW *w;
register struct iovec   *iov;
register int     n;
{
    register int    i;

    werase (w);
    wmove (w, 0, 0);
    for (i = 0; i < n; i++, iov++)
	wprintw (w, "%*.*s", iov -> iov_len, iov -> iov_len, iov -> iov_base);
    wrefresh (w);

    sleep (PAUSE);

    return OK;
}

/*  */

static struct {
    char   *h_msg;
    int    *h_val;
}               hlpmsg[] = {
                    "		forward		backwards", NULL,
                    "		-------		---------", NULL,
                    "next screen	SPACE", NULL,
                    "next %d line%s	RETURN		y", &smallmove,
                    "next %d line%s	EOT		u", &largemove,
                    "go		g		G", NULL,
                    "", NULL,
                    "refresh		CTRL-L", NULL,
                    "quit		q", NULL,

                    NULL, NULL
};

/*  */

static int  WINless (w, fin)
register WINDOW *w;
int	fin;
{
    register int    c,
                    i,
                    n;
    int     nfresh,
#ifdef	notdef
	    nlatch,
#endif	notdef
            nwait;
    char   *cp;
    register struct line   *lbottom;

    did_less++;

    cp = NULL;
#ifdef	notdef
    if (fin)
	ltop = NULL;
#endif	notdef
    lbottom = NULL;
    nfresh = 1;
    nwait = 0;
    wrefresh (w);

    for (;;) {
	if (nfresh || nwait) {
	    nfresh = 0;
#ifdef	notdef
	    nlatch = 1;

once_only: ;
#endif	notdef
	    werase (w);
	    wmove (w, 0, 0);

	    if (ltop == NULL)
		if (fin) {
		    (void) lgo (ltail -> l_no - w -> _maxy + 1);
		    if (ltop == NULL)
			ltop = lhead;
		}
		else
		    ltop = lbottom && lbottom -> l_prev ? lbottom -> l_prev
			    : lbottom;

	    for (lbottom = ltop; lbottom; lbottom = lbottom -> l_next)
		if (waddstr (w, lbottom -> l_buf) == ERR
			|| waddch (w, '\n') == ERR)
		    break;
	    if (lbottom == NULL)
		if (fin) {
#ifdef	notdef
		    if (nlatch && (ltail -> l_no >= w -> _maxy)) {
			(void) lgo (ltail -> l_no - w -> _maxy + 1);
			nlatch = 0;
			goto once_only;
		    }
#endif	notdef
		    lbottom = ltail;
		    while (waddstr (w, "~\n") != ERR)
			continue;
		}
		else {
		    wrefresh (w);
		    return 0;
		}

	    if (!nwait)
		wrefresh (w);
	}

	wmove (Command, 0, 0);
	if (cp) {
	    wstandout (Command);
	    wprintw (Command, "%s", cp);
	    wstandend (Command);
	    cp = NULL;
	}
	else
	    wprintw (Command, fin ? "top:%d bot:%d end:%d" : "top:%d bot:%d",
		    ltop -> l_no, lbottom -> l_no, ltail -> l_no);
	wprintw (Command, ">> ");
	wclrtoeol (Command);
	wrefresh (Command);

	c = toascii (wgetch (Command));

	werase (Command);
	wrefresh (Command);

	if (nwait) {
	    nwait = 0;
	    wrefresh (w);
	}

	n = 0;
again: 	;
	switch (c) {
	    case ' ': 
		ltop = lbottom -> l_next;
		nfresh++;
		break;

	    case '\r': 
	    case '\n': 
	    case 'e': 
	    case 'j': 
		if (n)
		    smallmove = n;
		if (ladvance (smallmove))
		    nfresh++;
		break;

	    case 'y': 
	    case 'k': 
		if (n)
		    smallmove = n;
		if (lretreat (smallmove))
		    nfresh++;
		break;

	    case 'd': 
	eof: 	;
		if (n)
		    largemove = n;
		if (ladvance (largemove))
		    nfresh++;
		break;

	    case 'u': 
		if (n)
		    largemove = n;
		if (lretreat (largemove))
		    nfresh++;
		break;

	    case 'g': 
		if (lgo (n ? n : 1))
		    nfresh++;
		break;

	    case 'G': 
		if (lgo (n ? n : ltail -> l_no - w -> _maxy + 1))
		    nfresh++;
		break;

	    case '\f': 
	    case 'r': 
		wrefresh (curscr);
		break;

	    case 'h': 
	    case '?': 
		werase (w);
		wmove (w, 0, 0);
		for (i = 0; hlpmsg[i].h_msg; i++) {
		    if (hlpmsg[i].h_val)
			if (*hlpmsg[i].h_val > 1)
				wprintw (w,MSGSTR(PLHELPMSG+i,hlpmsg[i].h_msg),
				         *hlpmsg[i].h_val);
			else
				wprintw (w,MSGSTR(HELPMSG+i,hlpmsg[i].h_msg),
					 *hlpmsg[i].h_val);
		    else
			(void) waddstr (w, MSGSTR(HELPMSG+i,hlpmsg[i].h_msg));
		    (void) waddch (w, '\n');
		}
		wrefresh (w);
		nwait++;
		break;

	    case 'q': 
		return 1;

	    default: 
		if (c == EOFC)
		    goto eof;

		if (isdigit (c)) {
		    wmove (Command, 0, 0);
		    i = 0;
		    while (isdigit (c)) {
			wprintw (Command, "%c", c);
			wrefresh (Command);
			i = i * 10 + c - '0';
			c = toascii (wgetch (Command));
		    }
		    werase (Command);
		    wrefresh (Command);

		    if (i > 0) {
			n = i;
			goto again;
		    }
		    cp = MSGSTR(BADNUM1, "bad number"); /*MSG*/
		}
		else
		    cp = MSGSTR(NOGROK, "not understood"); /*MSG*/
		break;
	}
    }
}

/*  */

static int  WINputc (w, c)
register WINDOW *w;
register char c;
{
    register int    x,
                    y;

    if (w != Scan)
	return waddch (w, c);

    if ((x = w -> _curx) < 0 || x >= w -> _maxx
	    || (y = w -> _cury) < 0 || y >= w -> _maxy)
	return DONE;

    switch (c) {
	case '\t': 
	    for (x = 8 - (x & 0x07); x > 0; x--)
		if (WINputc (w, ' ') == ERR)
		    return ERR;
	    break;

	case '\n': 
	    if (++y < w -> _maxy) 
		(void) waddch (w, c);
	    else
		wclrtoeol (w);
	    break;

	default: 
	    if (++x < w -> _maxx) 
		(void) waddch (w, c);
	    break;
    }
    return DONE;
}

/*    LINES */

static  lreset () {
    register struct line   *lp,
                           *mp;

    for (lp = lhead; lp; lp = mp) {
	mp = lp -> l_next;
	free (lp -> l_buf);
	free ((char *) lp);
    }
    lhead = ltop = ltail = NULL;
}


static	linsert (w,ck)
WINDOW *w;
register char *ck;
{
    register char  *cp;
    register struct line   *lp;

    if ((lp = (struct line  *) calloc ((unsigned) 1, sizeof *lp)) == NULL)
	adios (NULLCP, MSGSTR(NOALNSTOR, "unable to allocate line storage")); /*MSG*/

    lp -> l_no = (ltail ? ltail -> l_no : 0) + 1;
/* The following line failed on AIX. why????
*    lp -> l_buf = getcpy (w -> _y[w -> _cury]);
*/
    lp -> l_buf = getcpy (ck);
    for (cp = lp -> l_buf + strlen (lp -> l_buf) - 1; cp >= lp -> l_buf; cp--)
	if (isspace (*cp))
	    *cp = (char)NULL;
	else
	    break;

    if (lhead == NULL)
	lhead = lp;
    if (ltop == NULL)
	ltop = lp;
    if (ltail)
	ltail -> l_next = lp;
    lp -> l_prev = ltail;
    ltail = lp;
}

/*  */

static int  ladvance (n)
int	n;
{
    register int    i;
    register struct line   *lp;

    for (i = 0, lp = ltop; i < n && lp; i++, lp = lp -> l_next)
	continue;

    if (ltop == lp)
	return 0;

    ltop = lp;
    return 1;
}


static int  lretreat (n)
int	n;
{
    register int    i;
    register struct line   *lp;

    for (i = 0, lp = ltop; i < n && lp; i++, lp = lp -> l_prev)
	if (!lp -> l_prev)
	    break;

    if (ltop == lp)
	return 0;

    ltop = lp;
    return 1;
}

/*  */

static int  lgo (n)
int	n;
{
    register int    i,
                    j;
    register struct line   *lp;

    if ((i = n - (lp = lhead) -> l_no) > (j = abs (n - ltop -> l_no)))
	i = j, lp = ltop;
    if (i > (j = abs (ltail -> l_no - n)))
	i = j, lp = ltail;

    if (n >= lp -> l_no) {
	for (; lp; lp = lp -> l_next)
	    if (lp -> l_no == n)
		break;
    }
    else {
	for (; lp; lp = lp -> l_prev)
	    if (lp -> l_no == n)
		break;
	if (!lp)
	    lp = lhead;
    }

    if (ltop == lp)
	return 0;

    ltop = lp;
    return 1;
}

/*    TTYS */

static int  TTYinit (nprog) {
    if (!isatty (fileno (stdin)) || !isatty (fileno (stdout)))
	if (nprog)
	    return NOTOK;
	else
	    adios (NULLCP, MSGSTR(NOTTTY, "not a tty")); /*MSG*/

    foreground ();
#ifndef	SYS5
    if (ioctl (fileno (stdin), TIOCGETP, (char *) &sg) == NOTOK)
	adios ("failed", "ioctl TIOCGETP failed");
    if (ioctl (fileno (stdin), TIOCGETC, (char *) &tc) == NOTOK)
	adios ("failed", "ioctl TIOCGETC failed");
#else	SYS5
    if (ioctl (fileno (stdin), TCGETA, &sg) == NOTOK)
	adios ("failed", "ioctl TCGETA failed");
#endif	SYS5
#ifdef	TIOCGLTC
    if (ioctl (fileno (stdin), TIOCGLTC, (char *) &ltc) == NOTOK)
	adios ("failed", "ioctl TIOCGLTC failed");
#endif	TIOCGLTC
    intrc = INTR;
    sideground ();

    tty_ready = OK;

    (void) signal (SIGPIPE, (void(*)(int))PIPEser);

    return OK;
}

/*  */

static	TTYon () {
    if (tty_ready == DONE)
	return;

    INTR = NOTOK;
#ifndef	SYS5
    (void) ioctl (fileno (stdin), TIOCSETC, (char *) &tc);
#else	SYS5
    (void) ioctl (fileno (stdin), TCSETA, &sg);
#endif	SYS5

    (void) crmode ();
    (void) noecho ();
    (void) nonl ();
    scrollok (curscr, FALSE);

    discard (stdin);

    tty_ready = DONE;

    (void) signal (SIGHUP, (void(*)(int))SIGser);
    (void) signal (SIGINT, (void(*)(int))SIGser);
    (void) signal (SIGQUIT, (void(*)(int))SIGser);
#ifdef	SIGTSTP
    (void) signal (SIGTSTP, (void(*)(int))TSTPser);
#endif	SIGTSTP
}

/*  */

static	TTYoff () {
    if (tty_ready == NOTOK)
	return;

    INTR = intrc;
#ifndef	SYS5
    (void) ioctl (fileno (stdin), TIOCSETC, (char *) &tc);
#else	SYS5
    (void) ioctl (fileno (stdin), TCSETA, &sg);
#endif	SYS5

    leaveok (curscr, TRUE);
    mvcur (0, COLS - 1, LINES - 1, 0);
    endwin ();
    if (tty_ready == DONE) {
#ifndef	SYS5
	if (CE)
	    tputs (CE, 0, _putchar);
	else
#endif	SYS5
	    fprintf (stdout, "\r\n");
    }
    (void) fflush (stdout);

    tty_ready = NOTOK;

    (void) signal (SIGHUP, SIG_DFL);
    (void) signal (SIGINT, SIG_DFL);
    (void) signal (SIGQUIT, SIG_DFL);
#ifdef	SIGTSTP
    (void) signal (SIGTSTP, SIG_DFL);
#endif	SIGTSTP
}

/*  */

static  foreground () {
#ifdef	TIOCGPGRP
    int     pgrp,
            tpgrp;
    int     (*tstat) ();

    if ((pgrp = getpgrp (0)) == NOTOK)
	adios ("process group", "unable to determine");
    for (;;) {
	if (ioctl (fileno (stdin), TIOCGPGRP, (char *) &tpgrp) == NOTOK)
	    adios ("tty's process group", MSGSTR(NOPGRP, 
		   "unable to determine tty's process group"));
	if (pgrp == tpgrp)
	    break;

	tstat = signal (SIGTTIN, SIG_DFL);
	(void) kill (0, SIGTTIN);
	(void) signal (SIGTTIN, tstat);
    }
    
    (void) signal (SIGTTIN, SIG_IGN);
    (void) signal (SIGTTOU, SIG_IGN);
    (void) signal (SIGTSTP, SIG_IGN);
#endif	TIOCGPGRP
}


sideground () {
#ifdef	TIOCGPGRP
    (void) signal (SIGTTIN, SIG_DFL);
    (void) signal (SIGTTOU, SIG_DFL);
    (void) signal (SIGTSTP, SIG_DFL);
#endif	TIOCGPGRP
}

/*    SIGNALS */

/* ARGSUSED */

static void  ALRMser (int     sig)
{
     longjmp (PEERctx, DONE);
}


#ifdef	BSD42
/* ARGSUSED */
#endif	BSD42

static void  PIPEser (int	sig)
{
#ifndef	BSD42
    (void) signal (sig, SIG_IGN);
#endif	BSD42

    adios (NULLCP, MSGSTR(LOSTPEER, "lost peer"));
}


#ifdef	BSD42
/* ARGSUSED */
#endif	BSD42

static void  SIGser (int     sig)
{
#ifndef	BSD42
    (void) signal (sig, SIG_IGN);
#endif	BSD42

    done (1);
}


#ifdef	SIGTSTP
static int  TSTPser (sig)
int     sig;
{
    tputs (tgoto (CM, 0, LINES - 1), 0, _putchar);
    (void) fflush (stdout);

    TTYoff ();
#ifdef	BSD42
    (void) sigsetmask (sigblock (0) & ~sigmask (SIGTSTP));
#endif	BSD42

    (void) kill (getpid (), sig);

#ifdef	BSD42
    (void) sigblock (sigmask (SIGTSTP));
#endif	BSD42
    TTYon ();

    wrefresh (curscr);
}
#endif	SIGTSTP

/*    MISCELLANY */

void	done (status)
int	status;
{
    TTYoff ();
    (void) pFIN ();

    exit (status);
}

/*  */

/* VARARGS2 */

static void  adorn (what, fmt, a, b, c, d, e, f)
char   *what,
       *fmt,
       *a,
       *b,
       *c,
       *d,
       *e,
       *f;
{
    char   *cp = invo_name;

    invo_name = NULL;
    advise (what, fmt, a, b, c, d, e, f);
    invo_name = cp;
}

/*  */

/* VARARGS3 */

void advertise (what, tail, fmt, a, b, c, d, e, f)
char   *what,
       *tail,
       *fmt,
       *a,
       *b,
       *c,
       *d,
       *e,
       *f;
{
    int	    eindex = errno;
    char    buffer[BUFSIZ],
            err[BUFSIZ];
    struct iovec    iob[20];
    register struct iovec  *iov = iob;

    (void) fflush (stdout);

    (void) fflush (stderr);

    if (invo_name) {
	iov -> iov_len = strlen (iov -> iov_base = invo_name);
	iov++;
	iov -> iov_len = strlen (iov -> iov_base = ": ");
	iov++;
    }
    
    (void) sprintf (buffer, fmt, a, b, c, d, e, f);
    iov -> iov_len = strlen (iov -> iov_base = buffer);
    iov++;
    if (what) {
	if (*what) {
/*
**          Taken care of before calling advertise
**
**	    iov -> iov_len = strlen (iov -> iov_base = " ");
**	    iov++;
**	    iov -> iov_len = strlen (iov -> iov_base = what);
**	    iov++;
*/
	    iov -> iov_len = strlen (iov -> iov_base = ": ");
	    iov++;
	}
	if (eindex > 0 && eindex < sys_nerr)
	    iov -> iov_len = strlen (iov -> iov_base = strerror(eindex));
	else {
	    (void) sprintf (err, MSGSTR(ERROR, "Error %d"), eindex); /*MSG*/
	    iov -> iov_len = strlen (iov -> iov_base = err);
	}
	iov++;
    }
    if (tail && *tail) {
	iov -> iov_len = strlen (iov -> iov_base = ", ");
	iov++;
	iov -> iov_len = strlen (iov -> iov_base = tail);
	iov++;
    }
    iov -> iov_len = strlen (iov -> iov_base = "\n");
    iov++;

    if (tty_ready == DONE)
	(void) WINwritev (Display, iob, iov - iob);
    else
	(void) writev (fileno (stderr), iob, iov - iob);
}

/*  */

#ifndef	BSD42
static int     writev (fd, iov, n)
register int     fd;
register struct iovec   *iov;
register int     n;
{
    register int    i,
                    j;

    for (i = j = 0; i < n; i++, iov++)
	if (write (fd, iov -> iov_base, iov -> iov_len) != iov -> iov_len)
	    break;
	else
	    j += iov -> iov_len;

    return j;
}
#endif	BSD42

/* 
 * myputchar - procedure for calling putchar.  putchar is a macro,
 *             and passing it's address above doesn't work.  Using
 *             it with a procedure makes it addressable.
 */
static int	myputchar(c)
int c;
{
  putchar (c);
}
