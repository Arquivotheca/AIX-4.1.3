static char sccsid[] = "@(#)66	1.11  src/bos/usr/bin/mh/uip/msh.c, cmdmh, bos411, 9428A410j 1/3/94 08:06:17";
/* 
 * COMPONENT_NAME: CMDMH msh.c
 * 
 * FUNCTIONS: MSGSTR, Mmsh, alrmser, check_folder, display_info, 
 *            expand, fin_io, finaux_io, fsetup, getargs, getcmds, 
 *            init_io, initaux_io, intrser, m_gMsgs, m_init, m_reset, 
 *            m_setcur, msh, msh_ready, pCMD, pFIN, pINI, pQRY, 
 *            pQRY1, pQRY2, padios, padvise, parse, peerwait, 
 *            pipeser, pop_action, quit, quitser, read_file, read_map, 
 *            read_pop, readid, readids, scanrange, scanstring, setup, 
 *            ttyN, ttyNaux, ttyR, winN, winR, winX, write_ids 
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
/* static char sccsid[] = "msh.c	7.1 87/10/13 17:30:34"; */

/* msh.c - The MH shell (sigh) */

/* TODO:
	Keep more status information in maildrop map
 */

#include "mh.h"
#include "dropsbr.h"
#include "formatsbr.h"
#include "scansbr.h"
#include "tws.h"
#include <stdio.h>
#include "mts.h"
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef	SYS5
#include <sgtty.h>
#else	SYS5
#include <termio.h>
#include <sys/ioctl.h>
#endif	SYS5
#include <pwd.h>
#include <setjmp.h>
#include <signal.h>
#include "mshsbr.h"
#include "vmhsbr.h"

#include "mh_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 

#define	SIGEMT	SIGIOT		/* AIX hack	*/

#define	QUOTE	'\\'		/* sigh */


/*  */

static struct swit switches[] = {
#define	IDSW	0
    "idstart number", -7,	/* interface from bbc */
#define	FDSW	1
    "idstop number", -6,	/*  .. */
#define	QDSW	2
    "idquit number", -6,	/*  .. */
#define	NMSW	3
    "idname BBoard", -6,	/*  .. */

#define	PRMPTSW	4
    "prompt string", 0,

#define	SCANSW	5
    "scan", -5,
#define	NSCANSW	6
    "noscan", -6,

#define	READSW	7
    "vmhread fd", -7,
#define	WRITESW	8
    "vmhwrite fd", -8,	

#define	PREADSW	9
    "popread fd", -7,
#define	PWRITSW	10
    "popwrite fd", -8,

#define	TCURSW	11
    "topcur", 0,
#define	NTCURSW	12
    "notopcur", 0,

#define	HELPSW	13
    "help", 4,

    NULL, (int)NULL
};

/*  */
				/* FOLDER */
char  *fmsh = NULL;		/* folder instead of file */
int    modified;		/* command modified folder */
struct msgs *mp;		/* used a lot */
static int   nMsgs = 0;
struct Msg  *Msgs = NULL;	/* Msgs[0] not used */
static FILE *fp;		/* input file */
static FILE *yp = NULL;		/* temporary file */
static int  mode;		/* mode of file */
static int  numfds = 0;		/* number of files cached */
static int  maxfds = 0;		/* number of files cached to be cached */
static time_t mtime = (time_t) 0;/* mtime of file */
static  msh (int);
static int  read_map (char *, long);
static  int   read_file (register long , register int);
static m_gMsgs (int);
static int  check_folder (int);
static	scanrange (int, int);
static	scanstring (char *);
static  quit();
static int  getargs (char *, struct swit *, struct Cmd *);
static int  getcmds (struct swit *, struct Cmd *, int);
static int  parse (char *, struct Cmd *);
static int  init_io (register struct Cmd *, int);
static int  initaux_io (register struct Cmd *);
static  fin_io (register struct Cmd *, int);
static int  finaux_io (register struct Cmd *);
static m_init();
static int  pINI (), pFIN();
static int pQRY(char*, int);
static int pQRY1(int), pQRY2();
static int pCMD (char *, struct swit *, struct Cmd *);
static int peerwait();
static int  ttyNaux (register struct Cmd *,char *);
static int  ttyR (register struct Cmd *);
static int  winN (register struct Cmd *, int , int);
static int  winR (register struct Cmd *);
static int  winX (int);

				/* VMH */
#define	ALARM	((unsigned int) 10)
#define	ttyN(c)	ttyNaux ((c), NULLCP)

static int  vmh = 0;

static int  vmhpid = OK;
static int  vmhfd0;
static int  vmhfd1;
static int  vmhfd2;

static int  vmhtty = NOTOK;

#define	SCAN	1
#define	STATUS	2
#define	DISPLAY	3
#define	NWIN	DISPLAY

static int  topcur = 0;

static int  numwins = 0;
static int  windows[NWIN + 1];

static jmp_buf peerenv;

void	padios (), padvise ();
static void	alrmser (int);


#ifdef	BPOP
				/* POP */

static int pmsh = 0;		/* BPOP enabled */

extern char response[];
#endif	BPOP


				/* PARENT */
static int  pfd = NOTOK;	/* fd parent is reading from */
static int  ppid = 0;		/* pid of parent */


				/* COMMAND */
int     interactive;		/* running from a /dev/tty */
int     redirected;		/* re-directing output */
FILE  *sp = NULL;		/* original stdout */

char   *cmd_name;		/* command being run */

char    myfilter[BUFSIZ];	/* path to mhl.forward */

static char *myprompt = "(%s) ";/* prompting string */


				/* BBOARDS */
static int    gap;		/* gap in BBoard-ID:s */

static char *myname = NULL;	/* BBoard name */

char   *BBoard_ID = "BBoard-ID";/* BBoard-ID constant */

				/* SIGNALS */
void  (*istat) (int);
void  (*pstat) (int);	/* current SIGPIPE */
void  (*qstat) (int);
#ifdef	SIGTSTP
void  (*tstat) (int);	/* original SIGTSTP */
#endif	SIGTSTP
int     interrupted;		/* SIGINT detected */
int     broken_pipe;		/* SIGPIPE detected */
int     told_to_quit;		/* SIGQUIT detected */

#ifdef	BSD42
int     should_intr;		/* signal handler should interrupt call */
jmp_buf sigenv;			/* the environment pointer */
#endif	BSD42

static  void	intrser (int), pipeser (int), quitser (int);


#ifdef	SYS5
struct passwd  *getpwnam ();
#endif	SYS5

char *index();

/*  */

/* ARGSUSED */

main (argc, argv)
int     argc;
char  **argv;
{
    int	    id = 0,
            scansw = 0,
    	    vmh1 = 0,
            vmh2 = 0;
#ifdef	BPOP
    int	    pmsh1 = 0,
	    pmsh2 = 0;
#endif	BPOP
    char   *cp,
           *file = NULL,
           *folder = NULL,
          **ap,
          **argp,
            buf[80],
           *arguments[MAXARGS];

        setlocale(LC_ALL,"");
	catd = catopen(MF_MH, NL_CAT_LOCALE);

    invo_name = r1bindex (argv[0], '/');
    mts_init (invo_name);
    if ((cp = m_find (invo_name)) != NULL) {
	ap = brkstring (cp = getcpy (cp), " ", "\n");
	ap = copyip (ap, arguments);
    }
    else
	ap = arguments;
    (void) copyip (argv + 1, ap);
    argp = arguments;

/*  */

    while (cp = *argp++) {
	if (*cp == '-')
	    switch (smatch (++cp, switches)) {
		case AMBIGSW: 
		    ambigsw (cp, switches);
		    done (1);
		case UNKWNSW: 
		    adios (NULLCP, MSGSTR(UNKWNSW1, "-%s unknown"), cp); /*MSG*/
		case HELPSW: 
		    (void) sprintf (buf, MSGSTR(USEFILE, "%s [switches] file"), invo_name); /*MSG*/
		    help (buf, switches);
		    done (1);

		case IDSW: 
		    if (!(cp = *argp++) || *cp == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    if ((id = atoi (cp)) < 1)
			adios (NULLCP, MSGSTR(BADARG, "bad argument %s %s"), argp[-2], cp); /*MSG*/
		    continue;
		case FDSW: 
		    if (!(cp = *argp++) || *cp == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    if ((pfd = atoi (cp)) <= 1)
			adios (NULLCP, MSGSTR(BADARG, "bad argument %s %s"), argp[-2], cp); /*MSG*/
		    continue;
		case QDSW: 
		    if (!(cp = *argp++) || *cp == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    if ((ppid = atoi (cp)) <= 1)
			adios (NULLCP, MSGSTR(BADARG, "bad argument %s %s"), argp[-2], cp); /*MSG*/
		    continue;
		case NMSW:
		    if (!(myname = *argp++) || *myname == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    continue;

		case SCANSW: 
		    scansw++;
		    continue;
		case NSCANSW: 
		    scansw = 0;
		    continue;

		case PRMPTSW:
		    if (!(myprompt = *argp++) || *myprompt == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    continue;

		case READSW: 
		    if (!(cp = *argp++) || *cp == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    if ((vmh1 = atoi (cp)) < 1)
			adios (NULLCP, MSGSTR(BADARG, "bad argument %s %s"), argp[-2], cp); /*MSG*/
		    continue;
		case WRITESW: 
		    if (!(cp = *argp++) || *cp == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    if ((vmh2 = atoi (cp)) < 1)
			adios (NULLCP, MSGSTR(BADARG, "bad argument %s %s"), argp[-2], cp); /*MSG*/
		    continue;

		case PREADSW: 
		    if (!(cp = *argp++) || *cp == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
#ifdef	BPOP
		    if ((pmsh1 = atoi (cp)) < 1)
			adios (NULLCP, MSGSTR(BADARG, "bad argument %s %s"), argp[-2], cp); /*MSG*/
#endif	BPOP
		    continue;
		case PWRITSW: 
		    if (!(cp = *argp++) || *cp == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
#ifdef	BPOP
		    if ((pmsh2 = atoi (cp)) < 1)
			adios (NULLCP, MSGSTR(BADARG, "bad argument %s %s"), argp[-2], cp); /*MSG*/
#endif	BPOP
		    continue;

		case TCURSW:
		    topcur++;
		    continue;
		case NTCURSW:
		    topcur = 0;
		    continue;
	    }
	if (*cp == '+' || *cp == '@') {
	    if (folder)
		adios (NULLCP, MSGSTR(ONEFOLDER, "only one folder at a time!")); /*MSG*/
	    else
		folder = path (cp + 1, *cp == '+' ? TFOLDER : TSUBCWF);
	}
	else
	    if (file)
		adios (NULLCP, MSGSTR(ONEFILE, "only one file at a time!")); /*MSG*/
	    else
		file = cp;
    }

/*  */

    if (!file && !folder)
	file = "./msgbox";
    if (file && folder)
	adios (NULLCP, MSGSTR(NOTBOTH, "use a file or a folder, not both")); /*MSG*/
    (void) strcpy (myfilter, libpath (mhlforward));
#ifdef	FIOCLEX
    if (pfd > 1)
	(void) ioctl (pfd, FIOCLEX, NULLCP);
#endif	FIOCLEX

#ifdef	BSD42
    should_intr = 0;
#endif	BSD42
    setsigx (istat, SIGINT, (void(*)(int))intrser);
    setsigx (qstat, SIGQUIT, (void(*)(int))quitser);

    (void) sc_width ();		/* MAGIC... */

    if (vmh = vmh1 && vmh2) {
	(void) rcinit (vmh1, vmh2);
	(void) pINI ();
	(void) signal (SIGINT, SIG_IGN);
	(void) signal (SIGQUIT, SIG_IGN);
#ifdef	SIGTSTP
	tstat = signal (SIGTSTP, SIG_IGN);
#endif	SIGTSTP
    }

#ifdef	BPOP
    if (pmsh = pmsh1 && pmsh2) {
	cp = getenv ("MHPOPDEBUG");
	if (pop_set (pmsh1, pmsh2, cp && *cp) == NOTOK)
	    padios (NULLCP, "%s", response);
	if (folder)
	    file = folder, folder = NULL;
    }
#endif	BPOP

    if (folder)
	fsetup (folder);
    else
	setup (file);
    readids (id);
    display_info (id > 0 ? scansw : 0);

    msh (id > 0 ? scansw : 0);

    m_reset ();
    
    done (0);
}

/*  */

static struct swit mshcmds[] = {
#define	ALICMD	0
    "ali", 0,
#define	EXPLCMD	1
    "burst", 0,
#define	COMPCMD	2
    "comp", 0,
#define	DISTCMD	3
    "dist", 0,
#define	EXITCMD	4
    "exit", 0,
#define	FOLDCMD	5
    "folder", 0,
#define	FORWCMD	6
    "forw", 0,
#define	HELPCMD	7
    "help", 0,
#define	INCMD	8
    "inc", 0,
#define	MARKCMD	9
    "mark", 0,
#define	MAILCMD	10
    "mhmail", 0,
#define	MSGKCMD	11
    "msgchk", 0,
#define	NEXTCMD	12
    "next", 0,
#define	PACKCMD	13
    "packf", 0,
#define	PICKCMD	14
    "pick", 0,
#define	PREVCMD	15
    "prev", 0,
#define	QUITCMD	16
    "quit", 0,
#define	FILECMD	17
    "refile", 0,
#define	REPLCMD	18
    "repl", 0,
#define	RMMCMD	19
    "rmm", 0,
#define	SCANCMD	20
    "scan", 0,
#define	SENDCMD	21
    "send", 0,
#define	SHOWCMD	22
    "show", 0,
#define	SORTCMD	23
    "sortm", 0,
#define	WHATCMD	24
    "whatnow", 0,
#define	WHOMCMD	25
    "whom", 0,

    NULL, (int)NULL
};

/*  */

static  msh (int scansw)
{
    int     i;
    register char  *cp,
                  **ap;
    char    prompt[BUFSIZ],
           *vec[MAXARGS];
    struct Cmd  typein;
    register struct Cmd *cmdp;

    (void) sprintf (prompt, myprompt, invo_name);
    cmdp = &typein;

    for (;;) {
	if (yp) {
	    (void) fclose (yp);
	    yp = NULL;
	}
	if (vmh) {
	    if ((i = getcmds (mshcmds, cmdp, scansw)) == EOF) {
		(void) rcdone ();
		return;
	    }
	}
	else {
	    (void) check_folder (scansw);
	    if ((i = getargs (prompt, mshcmds, cmdp)) == EOF) {
		(void) putchar ('\n');
		return;
	    }
	}
	cmd_name = mshcmds[i].sw;

	switch (i) {
	    case QUITCMD: 
		quit ();
		return;

	    case EXITCMD:
	    case EXPLCMD: 
	    case FOLDCMD: 
	    case FORWCMD: 	/* sigh */
	    case MARKCMD: 
	    case NEXTCMD: 
	    case PACKCMD: 
	    case PICKCMD: 
	    case PREVCMD: 
	    case RMMCMD: 
	    case SHOWCMD: 
	    case SCANCMD: 
	    case SORTCMD: 
		if ((cp = m_find (cmd_name)) != NULL) {
		    ap = brkstring (cp = getcpy (cp), " ", "\n");
		    ap = copyip (ap, vec);
		}
		else
		    ap = vec;
		break;

	    default: 
		cp = NULL;
		ap = vec;
		break;
	}
	(void) copyip (cmdp -> args + 1, ap);

	m_init ();

	if (!vmh && init_io (cmdp, vmh) == NOTOK) {
	    if (cp != NULL)
		free (cp);
	    continue;
	}
	modified = 0;
	redirected = vmh || cmdp -> direction != STDIO;

	switch (i) {
	    case ALICMD: 
	    case COMPCMD: 
	    case INCMD: 
	    case MAILCMD: 
	    case MSGKCMD: 
	    case SENDCMD: 
	    case WHATCMD: 
	    case WHOMCMD: 
		if (!vmh || ttyN (cmdp) != NOTOK)
		    forkcmd (vec, cmd_name);
		break;

	    case DISTCMD: 
		if (!vmh || ttyN (cmdp) != NOTOK)
		    distcmd (vec);
		break;

	    case EXPLCMD: 
		if (!vmh || winN (cmdp, DISPLAY, 1) != NOTOK)
		    explcmd (vec);
		break;

	    case FILECMD: 
		if (!vmh 
			|| (filehak (vec) == OK ? ttyN (cmdp)
					: winN (cmdp, DISPLAY, 1)) != NOTOK)
		    filecmd (vec);
		break;

	    case FOLDCMD: 
		if (!vmh || winN (cmdp, DISPLAY, 1) != NOTOK)
		    foldcmd (vec);
		break;

	    case FORWCMD: 
		if (!vmh || ttyN (cmdp) != NOTOK)
		    forwcmd (vec);
		break;

	    case HELPCMD: 
		if (!vmh || winN (cmdp, DISPLAY, 1) != NOTOK)
		    helpcmd (vec);
		break;

	    case EXITCMD:
	    case MARKCMD: 
		if (!vmh || winN (cmdp, DISPLAY, 1) != NOTOK)
		    markcmd (vec);
		break;

	    case NEXTCMD: 
	    case PREVCMD: 
	    case SHOWCMD: 
		if (!vmh || winN (cmdp, DISPLAY, 1) != NOTOK)
		    showcmd (vec);
		break;

	    case PACKCMD: 
		if (!vmh 
			|| (packhak (vec) == OK ? ttyN (cmdp)
					: winN (cmdp, DISPLAY, 1)) != NOTOK)
		    packcmd (vec);
		break;

	    case PICKCMD: 
		if (!vmh || winN (cmdp, DISPLAY, 1) != NOTOK)
		    pickcmd (vec);
		break;

	    case REPLCMD: 
		if (!vmh || ttyN (cmdp) != NOTOK)
		    replcmd (vec);
		break;

	    case RMMCMD: 
		if (!vmh || winN (cmdp, DISPLAY, 1) != NOTOK)
		    rmmcmd (vec);
		break;

	    case SCANCMD: 
		if (!vmh || winN (cmdp, DISPLAY, 1) != NOTOK)
		    scancmd (vec);
		break;

	    case SORTCMD: 
		if (!vmh || winN (cmdp, DISPLAY, 1) != NOTOK)
		    sortcmd (vec);
		break;

	    default: 
		padios (NULLCP, MSGSTR(NODISP, "no dispatch for %s"), cmd_name); /*MSG*/
	}

	if (vmh) {
	    if (vmhtty != NOTOK)
		(void) ttyR (cmdp);
	    if (vmhpid > OK)
		(void) winR (cmdp);
	}
	else
	    fin_io (cmdp, vmh);
	if (cp != NULL)
	    free (cp);
	if (i == EXITCMD) {
	    quit ();
	    return;
	}
    }
}

/*  */

fsetup (folder)
char   *folder;
{
    register int msgnum;
    char   *maildir;
    struct stat st;

    maildir = m_maildir (folder);
    if (chdir (maildir) == NOTOK)
	padios (maildir, MSGSTR(NOCHANGE, "unable to change directory to %s"), maildir); /*MSG*/
    if (!(mp = m_gmsg (folder)))
	padios (NULLCP, MSGSTR(NOREADF, "unable to read folder %s"), folder); /*MSG*/
    if (mp -> hghmsg == 0)
	padios (NULLCP, MSGSTR(NOMESF, "no messages in %s"), folder); /*MSG*/

    mode = m_gmprot ();
    mtime = stat (mp -> foldpath, &st) != NOTOK ? st.st_mtime : 0;

    m_gMsgs (mp -> hghmsg);

    for (msgnum = mp -> lowmsg; msgnum <= mp -> hghmsg; msgnum++) {
	Msgs[msgnum].m_bboard_id = 0;
	Msgs[msgnum].m_top = NOTOK;
	Msgs[msgnum].m_start = Msgs[msgnum].m_stop = 0L;
	Msgs[msgnum].m_scanl = NULL;
    }

    m_init ();

    fmsh = getcpy (folder);

#ifndef	BSD42
    maxfds = _NFILE / 2;
#else	BSD42
    maxfds = getdtablesize () / 2;
#endif	BSD42
    if ((maxfds -= 2) < 1)
	maxfds = 1;
}

/*  */

setup (file)
char   *file;
{
    int     i,
            msgp;
#ifdef	BPOP
    char    tmpfil[BUFSIZ];
#endif	BPOP
    struct stat st;

#ifdef	BPOP
    if (pmsh) {
	(void) strcpy (tmpfil, m_tmpfil (invo_name));
	if ((fp = fopen (tmpfil, "w+")) == NULL)
	    padios (tmpfil, MSGSTR(NOCREATE, "unable to create %s"), tmpfil); /*MSG*/
	(void) unlink (tmpfil);
    }
    else
#endif	BPOP
    if ((fp = fopen (file, "r")) == NULL)
	padios (file, MSGSTR(NOREAD, "unable to read %s"), file); /*MSG*/
#ifdef	FIOCLEX
    (void) ioctl (fileno (fp), FIOCLEX, NULLCP);
#endif	FIOCLEX
    if (fstat (fileno (fp), &st) != NOTOK) {
	mode = (int) (st.st_mode & 0777), mtime = st.st_mtime;
	msgp = read_map (file, (long) st.st_size);
    }
    else {
	mode = m_gmprot (), mtime = 0;
	msgp = 0;
    }

    if ((msgp = read_file (msgp ? Msgs[msgp].m_stop : 0L, msgp + 1)) < 1)
	padios (NULLCP, MSGSTR(NOMESF, "no messages in %s"), myname ? myname : file); /*MSG*/

    mp = (struct msgs  *) calloc ((unsigned) 1, MSIZE (mp, 1, msgp + 1));
    if (mp == NULL)
	padios (NULLCP, MSGSTR(NOAFSTOR, "unable to allocate folder storage")); /*MSG*/

    mp -> hghmsg = msgp;
    mp -> nummsg = msgp;
    mp -> lowmsg = 1;
    mp -> curmsg = 0;

    mp -> foldpath = getcpy (myname ? myname : file);
    mp -> msgflags = (char)NULL;
#ifdef	BPOP
    if (pmsh)
	mp -> msgflags |= READONLY;
    else {
#endif	BPOP
	(void) stat (file, &st);
	if (st.st_uid != getuid () || access (file, 02) == NOTOK)
	    mp -> msgflags |= READONLY;
#ifdef	BPOP
    }
#endif	BPOP
    mp -> lowoff = 1;
    mp -> hghoff = mp -> hghmsg + 1;

#ifdef	MTR
    mp -> msgstats = (short *)
		calloc ((unsigned) 1, MSIZEX (mp, mp -> lowmsg, mp -> hghmsg));
    if (mp -> msgstats == NULL)
	padios (NULLCP, MSGSTR(NOAMSTOR, "unable to allocate messages storage")); /*MSG*/
    mp -> msgstats = (mp -> msgbase = mp -> msgstats) - mp -> lowoff;
    if (mp -> msgstats < 0)
	padios (NULLCP, MSGSTR(BOTCH3, "setup() botch -- you lose big")); /*MSG*/
#endif	MTR
#ifdef	BPOP
    if (pmsh) {
	for (i = mp -> lowmsg; i <= mp -> hghmsg; i++) {
	    Msgs[i].m_top = i;
	    mp -> msgstats[i] = EXISTS | VIRTUAL;
	}
    }
    else
#endif	BPOP
    for (i = mp -> lowmsg; i <= mp -> hghmsg; i++)
	mp -> msgstats[i] = EXISTS;
    m_init ();

    mp -> msgattrs[0] = getcpy ("unseen");
    mp -> msgattrs[1] = NULL;

    m_unknown (fp);		/* the MAGIC invocation */    
    if (fmsh) {
	free (fmsh);
	fmsh = NULL;
    }
}

/*  */

static int  read_map (char *file, long size)
{
    register int    i,
                    msgp;
    register struct drop   *dp,
                           *mp;
    struct drop *rp;

#ifdef	BPOP
    if (pmsh)
	return read_pop ();
#endif	BPOP

    if ((i = map_read (file, size, &rp, 1)) == 0)
	return 0;

    m_gMsgs (i);

    msgp = 1;
    for (dp = rp; i-- > 0; msgp++, dp++) {
	mp = &Msgs[msgp].m_drop;
	mp -> d_id = dp -> d_id;
	mp -> d_size = dp -> d_size;
	mp -> d_start = dp -> d_start;
	mp -> d_stop = dp -> d_stop;
	Msgs[msgp].m_scanl = NULL;
    }
    free ((char *) rp);

    return (msgp - 1);
}

/*  */

static  int   read_file (register long pos,
			 register int  msgp)
{
    register int    i;
    register struct drop   *dp,
                           *mp;
    struct drop *rp;

#ifdef	BPOP
    if (pmsh)
	return (msgp - 1);
#endif	BPOP

    if ((i = mbx_read (fp, pos, &rp, 1)) <= 0)
	return (msgp - 1);

    m_gMsgs ((msgp - 1) + i);

    for (dp = rp; i-- > 0; msgp++, dp++) {
	mp = &Msgs[msgp].m_drop;
	mp -> d_id = 0;
	mp -> d_size = dp -> d_size;
	mp -> d_start = dp -> d_start;
	mp -> d_stop = dp -> d_stop;
	Msgs[msgp].m_scanl = NULL;
    }
    free ((char *) rp);

    return (msgp - 1);
}

/*  */

#ifdef	BPOP
static int  read_pop () {
    int	    nmsgs,
            nbytes;

    if (pop_stat (&nmsgs, &nbytes) == NOTOK)
	padios (NULLCP, "%s", response);

    m_gMsgs (nmsgs);

    return nmsgs;
}


static int  pop_action (s)
register char  *s;
{
    fprintf (yp, "%s\n", s);
}
#endif	BPOP

/*  */

static m_gMsgs (int n)
{
    if (Msgs == NULL) {
	nMsgs = n + MAXFOLDER / 2;
	Msgs = (struct Msg *) calloc ((unsigned) (nMsgs + 2), sizeof *Msgs);
	if (Msgs == NULL)
	    padios (NULLCP, MSGSTR(NOAMSTRUCT, "unable to allocate Msgs structure")); /*MSG*/
	return;
    }

    if (nMsgs >= n)
	return;

    nMsgs = n + MAXFOLDER / 2;
    Msgs = (struct Msg *) realloc ((char *) Msgs,
	                    (unsigned) (nMsgs + 2) * sizeof *Msgs);
    if (Msgs == NULL)
	padios (NULLCP, MSGSTR(NORAMSTRUCT, "unable to reallocate Msgs structure")); /*MSG*/
}

/*  */

FILE   *msh_ready (msgnum, full)
register int msgnum;
int	full;
{
    register int    msgp;
    int     fd;
    long    pos1,
            pos2;
    char   *cp,
            tmpfil[BUFSIZ];

    if (yp) {
	(void) fclose (yp);
	yp = NULL;
    }

    if (fmsh) {
	if ((fd = Msgs[msgnum].m_top) == NOTOK) {
	    if (numfds >= maxfds)
		for (msgp = mp -> lowmsg; msgp <= mp -> hghmsg; msgp++)
		    if (Msgs[msgp].m_top != NOTOK) {
			(void) close (Msgs[msgp].m_top);
			Msgs[msgp].m_top = NOTOK;
			numfds--;
			break;
		    }

	    if ((fd = open (cp = m_name (msgnum), 0)) == NOTOK)
		padios (cp, MSGSTR(NOOPENM, "unable to open message %s"), cp); /*MSG*/
	    Msgs[msgnum].m_top = fd;
	    numfds++;
	}

	if ((fd = dup (fd)) == NOTOK)
	    padios ("cached message", MSGSTR(NODUPCMSG, "unable to dup cached message")); /*MSG*/
	if ((yp = fdopen (fd, "r")) == NULL)
	    padios (NULLCP, MSGSTR(NOFDOPENCM, "unable to fdopen cached message")); /*MSG*/
	(void) fseek (yp, 0L, 0);
	return yp;
    }

#ifdef	BPOP
    if (pmsh && (mp -> msgstats[msgnum] & VIRTUAL)) {
	if (Msgs[msgnum].m_top == 0)
	    padios (NULLCP, MSGSTR(BOTCH4, "msh_ready (%d, %d) botch"), msgnum, full); /*MSG*/
	if (!full) {
	    (void) strcpy (tmpfil, m_tmpfil (invo_name));
	    if ((yp = fopen (tmpfil, "w+")) == NULL)
		padios (tmpfil, MSGSTR(NOCREATE, "unable to create %s"), tmpfil); /*MSG*/
	    (void) unlink (tmpfil);

	    if (pop_top (Msgs[msgnum].m_top, 4, pop_action) == NOTOK)
		padios (NULLCP, "%s", response);

	    m_eomsbr ((int (*)()) 0);	/* XXX */
	    msg_style = MS_DEFAULT;	/*  .. */
	    (void) fseek (yp, 0L, 0);
	    return yp;
	}

	(void) fseek (fp, 0L, 2);
	(void) fwrite (mmdlm1, 1, strlen (mmdlm1), fp);
	if (fflush (fp))
	    padios ("temporary file", MSGSTR(WERRTF, "write error on temporary file")); /*MSG*/
	(void) fseek (fp, 0L, 2);
	pos1 = ftell (fp);

	yp = fp;
	if (pop_retr (Msgs[msgnum].m_top, pop_action) == NOTOK)
	    padios (NULLCP, "%s", response);
	yp = NULL;

	(void) fseek (fp, 0L, 2);
	pos2 = ftell (fp);
	(void) fwrite (mmdlm2, 1, strlen (mmdlm2), fp);
	if (fflush (fp))
	    padios ("temporary file", MSGSTR(WERRTF, "write error on temporary file")); /*MSG*/

	Msgs[msgnum].m_start = pos1;
	Msgs[msgnum].m_stop = pos2;

	mp -> msgstats[msgnum] &= ~VIRTUAL;
    }
#endif	BPOP

    m_eomsbr ((int (*)()) 0);	/* XXX */
    (void) fseek (fp, Msgs[msgnum].m_start, 0);
    return fp;
}

/*  */

static int  check_folder (int scansw)
{
    int     flags,
            i,
            low,
            hgh,
            msgp;
    struct stat st;

#ifdef	BPOP
    if (pmsh)
	return 0;
#endif	BPOP

    if (fmsh) {
	if (stat (mp -> foldpath, &st) == NOTOK)
	    padios (mp -> foldpath, MSGSTR(NOSTAT, "unable to stat %s"), mp -> foldpath); /*MSG*/
	if (mtime == st.st_mtime)
	    return 0;
	mtime = st.st_mtime;

	low = mp -> hghmsg + 1;
	m_fmsg (mp);

	if (!(mp = m_gmsg (fmsh)))
	    padios (NULLCP, MSGSTR(NORRFLDR, "unable to re-read folder %s"), fmsh); /*MSG*/

	hgh = mp -> hghmsg;

	for (msgp = mp -> lowmsg; msgp <= mp -> hghmsg; msgp++) {
	    if (Msgs[msgp].m_top != NOTOK) {
		(void) close (Msgs[msgp].m_top);
		Msgs[msgp].m_top = NOTOK;
		numfds--;
	    }
	    if (Msgs[msgp].m_scanl) {
		free (Msgs[msgp].m_scanl);
		Msgs[msgp].m_scanl = NULL;
	    }
	}

	m_init ();

	if (modified || low > hgh)
	    return 1;
	goto check_vmh;
    }
    if (fstat (fileno (fp), &st) == NOTOK)
	padios (mp -> foldpath, MSGSTR(NOFSTAT, "unable to fstat %s"), mp -> foldpath); /*MSG*/
    if (mtime == st.st_mtime)
	return 0;
    mode = (int) (st.st_mode & 0777);
    mtime = st.st_mtime;

    if ((msgp = read_file (Msgs[mp -> hghmsg].m_stop, mp -> hghmsg + 1)) < 1)
	padios (NULLCP, MSGSTR(NOMESF, "no messages in %s"), mp -> foldpath);	/* XXX */ /*MSG*/
    if (msgp >= MAXFOLDER)
	padios (NULLCP, MSGSTR(MANYMESS, "more than %d messages in %s"), MAXFOLDER, mp -> foldpath); /*MSG*/
    if (msgp <= mp -> hghmsg)
	return 0;		/* XXX */

    if ((mp = m_remsg (mp, 0, msgp)) == NULL)
	padios (NULLCP, MSGSTR(NOAFSTOR, "unable to allocate folder storage")); /*MSG*/

    low = mp -> hghmsg + 1, hgh = msgp;
    flags = scansw ? m_seqflag (mp, "unseen") : 0;
    for (i = mp -> hghmsg + 1; i <= msgp; i++) {
	mp -> msgstats[i] = EXISTS | flags;
	mp -> nummsg++;
    }
    mp -> hghmsg = msgp;
    m_init ();

check_vmh: ;
    if (vmh)
	return 1;

    advise (NULLCP, MSGSTR(NEWMAIL, "new messages have arrived!\007")); /*MSG*/
    if (scansw)
	scanrange (low, hgh);

    return 1;
}

/*  */

static	scanrange (int low, int hgh)
{
    char    buffer[BUFSIZ];

    (void) sprintf (buffer, "%d-%d", low, hgh);
    scanstring (buffer);
}


static	scanstring (char   *arg)
{
    char   *cp,
          **ap,
           *vec[MAXARGS];

    if ((cp = m_find (cmd_name = "scan")) != NULL) {
	ap = brkstring (cp = getcpy (cp), " ", "\n");
	ap = copyip (ap, vec);
    }
    else
	ap = vec;
    *ap++ = arg;
    *ap = NULL;
    m_init ();
    scancmd (vec);
    if (cp != NULL)
	free (cp);
}

/*  */

readids (id)
int     id;
{
    register int    cur,
                    flags,
                    i,
                    msgnum;

    if (mp -> curmsg == 0)
	m_setcur (mp, mp -> lowmsg);
    if (id <= 0 || (flags = m_seqflag (mp, "unseen")) == 0)
	return;

    for (msgnum = mp -> hghmsg; msgnum >= mp -> lowmsg; msgnum--)
	mp -> msgstats[msgnum] |= flags;

    if (id != 1) {
	cur = mp -> curmsg;

	for (msgnum = mp -> hghmsg; msgnum >= mp -> lowmsg; msgnum--)
	    if ((i = readid (msgnum)) > 0 && i < id) {
		cur = msgnum + 1;
		mp -> msgstats[msgnum] &= ~flags;
		break;
	    }
	for (i = mp -> lowmsg; i < msgnum; i++)
	    mp -> msgstats[i] &= ~flags;

	if (cur > mp -> hghmsg)
	    cur = mp -> hghmsg;

	m_setcur (mp, cur);
    }

    if ((gap = 1 < id && id < (i = readid (mp -> lowmsg)) ? id : 0) && !vmh)
	advise (NULLCP, MSGSTR(IDGAP, "gap in ID:s, last seen %d, lowest present %d\n"), id - 1, i); /*MSG*/
}

/*  */

int 	readid (msgnum)
int     msgnum;
{
    int     i,
            state;
#ifdef	BPOP
    int	    arg1,
	    arg2,
	    arg3;
#endif	BPOP
    char   *bp,
            buf[BUFSIZ],
            name[NAMESZ];
    register FILE *zp;

    if (Msgs[msgnum].m_bboard_id)
	return Msgs[msgnum].m_bboard_id;
#ifdef	BPOP
    if (pmsh) {
	if (Msgs[msgnum].m_top == 0)
	    padios (NULLCP, MSGSTR(REDO, "readid (%d) botch"), msgnum); /*MSG*/
	if (pop_list (Msgs[msgnum].m_top, (int *) 0, &arg1, &arg2, &arg3) == OK
		&& arg3 > 0)
	    return (Msgs[msgnum].m_bboard_id = arg3);
    }
#endif	BPOP

    zp = msh_ready (msgnum, 0);
    for (state = FLD;;)
	switch (state = m_getfld (state, name, buf, sizeof buf, zp)) {
	    case FLD: 
	    case FLDEOF: 
	    case FLDPLUS: 
		if (uleq (name, BBoard_ID)) {
		    bp = getcpy (buf);
		    while (state == FLDPLUS) {
			state = m_getfld (state, name, buf, sizeof buf, zp);
			bp = add (buf, bp);
		    }
		    i = atoi (bp);
		    free (bp);
		    if (i > 0)
			return (Msgs[msgnum].m_bboard_id = i);
		    else
			continue;
		}
		while (state == FLDPLUS)
		    state = m_getfld (state, name, buf, sizeof buf, zp);
		if (state != FLDEOF)
		    continue;

	    default: 
		return 0;
	}
}

/*  */

display_info (scansw)
int     scansw;
{
    int     flags,
            sd;

    interactive = isatty (fileno (stdout));
    if (sp == NULL) {
	if ((sd = dup (fileno (stdout))) == NOTOK)
	    padios ("standard output", MSGSTR(NODUPSO3, "unable to dup standard output")); /*MSG*/
#ifndef	BSD42			/* XXX */
#ifdef	FIOCLEX
	(void) ioctl (sd, FIOCLEX, NULL);
#endif	FIOCLEX
#endif	not BSD42
	if ((sp = fdopen (sd, "w")) == NULL)
	    padios ("standard output", MSGSTR(NOFDOPENSO, "unable to fdopen standard output")); /*MSG*/
    }

    (void) putenv ("mhfolder", mp -> foldpath);
    if (vmh)
	return;

    if (myname)
	if (SOprintf ("%s", myname))
	    printf (MSGSTR(MSH_READ1,
		"Reading %s, currently at message %d of %d\n"),
		myname,mp->curmsg,mp->hghmsg); /*MSG*/
	else
	    printf (MSGSTR(MSH_READ2,
		"Reading, currently at message %d of %d\n"),
			mp->curmsg,mp->hghmsg); /*MSG*/
    else if (fmsh)
	printf (MSGSTR(MSH_READ3,
	    "Reading +%s, currently at message %d of %d\n"),
			fmsh,mp->curmsg,mp->hghmsg); /*MSG*/
    else
	printf (MSGSTR(MSH_READ1,
	    "Reading %s, currently at message %d of %d\n"),
			mp->foldpath,mp->curmsg,mp->hghmsg); /*MSG*/

    if ((flags = m_seqflag (mp, "unseen"))
	    && scansw
	    && (mp -> msgstats[mp -> hghmsg] & flags))
	scanstring ("unseen");
}

/*  */

static	write_ids () {
    int     i = 0,
	    flags,
            msgnum;
    char    buffer[80];

    if (pfd <= 1)
	return;

    if (flags = m_seqflag (mp, "unseen"))
	for (msgnum = mp -> hghmsg; msgnum >= mp -> lowmsg; msgnum--)
	    if (!(mp -> msgstats[msgnum] & flags)) {
		if (Msgs[msgnum].m_bboard_id == 0)
		    (void) readid (msgnum);
		if ((i = Msgs[msgnum].m_bboard_id) > 0)
		    break;
	    }

    (void) sprintf (buffer, "%d %d\n", i, Msgs[mp -> hghmsg].m_bboard_id);
    (void) write (pfd, buffer, sizeof buffer);
    (void) close (pfd);
    pfd = NOTOK;
}

/*  */

static  quit () {
    int     i,
            md,
            msgnum;
    char   *cp,
            tmpfil[BUFSIZ],
            map1[BUFSIZ],
            map2[BUFSIZ];
    char hlds[NL_TEXTMAX];
    struct stat st;
    FILE   *dp;

    if (!(mp -> msgflags & MODIFIED) || mp -> msgflags & READONLY || fmsh) {
	    if (vmh)
		(void) rc2peer (RC_FIN, 0, NULLCP);
	return;
    }

    if (vmh) 
	(void) ttyNaux (NULLCMD, "FAST");
    cp = NULL;
    if ((dp = lkfopen (mp -> foldpath, "r")) == NULL) {
	advise (mp -> foldpath, MSGSTR(NOLOCK, "unable to lock %s"), mp -> foldpath); /*MSG*/
	if (vmh) {
	    (void) ttyR (NULLCMD);
	    (void) pFIN ();
	}	
	return;
    }
    if (fstat (fileno (dp), &st) == NOTOK) {
	advise (mp -> foldpath, MSGSTR(NOSTAT, "unable to stat %s"), mp -> foldpath); /*MSG*/
	goto release;
    }
    if (mtime != st.st_mtime) {
	advise (NULLCP, MSGSTR(NEWMSGNU, "new messages have arrived, no update")); /*MSG*/
	goto release;
    }
    mode = (int) (st.st_mode & 0777);

    if (mp -> nummsg == 0) {
	sprintf(hlds, MSGSTR(ZEROFL, "Zero file \"%s\" <yes or no>?"), mp -> foldpath); /*MSG*/
	if (getanswer (hlds)) {
	    if ((i = creat (mp -> foldpath, mode)) != NOTOK)
		(void) close (i);
	    else
		advise (mp -> foldpath, MSGSTR(ZERROR, "error zero'ing %s"), mp -> foldpath); /*MSG*/
	    (void) unlink (map_name (mp -> foldpath));/* XXX */
	}
	goto release;
    }

    sprintf(hlds, MSGSTR(UPDFIL, "Update file \"%s\" <yes or no>?"), mp -> foldpath); /*MSG*/
    if (!getanswer (hlds))
	goto release;
    (void) strcpy (tmpfil, m_backup (mp -> foldpath));
    if ((md = mbx_open (tmpfil, st.st_uid, st.st_gid, mode)) == NOTOK) {
	advise (tmpfil, MSGSTR(NOOPEN, "unable to open %s"), tmpfil); /*MSG*/
	goto release;
    }

    for (msgnum = mp -> lowmsg; msgnum <= mp -> hghmsg; msgnum++)
	if (mp -> msgstats[msgnum] & EXISTS
		&& pack (tmpfil, md, msgnum) == NOTOK) {
	    (void) mbx_close (tmpfil, md);
	    (void) unlink (tmpfil);
	    (void) unlink (map_name (tmpfil));
	    goto release;
	}
    (void) mbx_close (tmpfil, md);

    if (rename (tmpfil, mp -> foldpath) == NOTOK)
	admonish (mp -> foldpath, MSGSTR(NORENAME, "unable to rename %s to %s"), tmpfil, mp -> foldpath); /*MSG*/
    else {
	(void) strcpy (map1, map_name (tmpfil));
	(void) strcpy (map2, map_name (mp -> foldpath));

	if (rename (map1, map2) == NOTOK) {
	    admonish (map2, MSGSTR(NORENAME, "unable to rename %s to %s"), map1, map2); /*MSG*/
	    (void) unlink (map1);
	    (void) unlink (map2);
	}
    }

release: ;
    if (cp)
	free (cp);
    (void) lkfclose (dp, mp -> foldpath);
    if (vmh) {
	(void) ttyR (NULLCMD);
	(void) pFIN ();
    }
}

/*  */

static int  getargs (char   *prompt,
		     struct swit *sw,
		     struct Cmd *cmdp)
{
    int     i;
    char   *cp;
    static char buffer[BUFSIZ];

    told_to_quit = 0;
    for (;;) {
	interrupted = 0;
#ifdef	BSD42
	switch (setjmp (sigenv)) {
	    case OK:
		should_intr = 1;
		break;

	    default:
		should_intr = 0;
		if (interrupted && !told_to_quit) {
		    (void) putchar ('\n');
		    continue;
		}
		if (ppid > 0)
		    (void) kill (ppid, SIGEMT);
		return EOF;
	}
#endif	BSD42
	if (interactive) {
	    printf ("%s", prompt);
	    (void) fflush (stdout);
	}
	for (cp = buffer; (i = getchar ()) != '\n';) {
#ifndef	BSD42
	    if (interrupted && !told_to_quit) {
		buffer[0] = NULL;
		(void) putchar ('\n');
		break;
	    }
	    if (told_to_quit || i == EOF) {
		if (ppid > 0)
		    (void) kill (ppid, SIGEMT);
		return EOF;
	    }
#else	BSD42
	    if (i == EOF)
		longjmp (sigenv, DONE);
#endif	BSD42
	    if (cp < &buffer[sizeof buffer - 2])
		*cp++ = i;
	}
	*cp = (char)NULL;

	if (buffer[0] == (char)NULL)
	    continue;
	if (buffer[0] == '?') {
	    printf (MSGSTR(CMDS, "commands:\n")); /*MSG*/
	    printsw (ALL, sw, "");
	    printf (MSGSTR(QUIT, "type CTRL-D or use ``quit'' to leave %s\n"), invo_name); /*MSG*/
	    continue;
	}

	if (parse (buffer, cmdp) == NOTOK)
	    continue;

	switch (i = smatch (cmdp -> args[0], sw)) {
	    case AMBIGSW: 
		ambigsw (cmdp -> args[0], sw);
		continue;
	    case UNKWNSW: 
		printf (MSGSTR(SAYWHAT, "say what: ``%s'' -- type ? (or help) for help\n"), cmdp -> args[0]); /*MSG*/
		continue;
	    default: 
#ifdef	BSD42
		should_intr = 0;
#endif	BSD42
		return i;
	}
    }
}

/*  */

static int  getcmds (struct swit *sw,
		     struct Cmd *cmdp,
		     int	scansw )
{
    int     i;
    struct record   rcs,
                   *rc = &rcs;

    initrc (rc);

    for (;;)
	switch (peer2rc (rc)) {
	    case RC_QRY: 
		(void) pQRY (rc -> rc_data, scansw);
		break;

	    case RC_CMD: 
		if ((i = pCMD (rc -> rc_data, sw, cmdp)) != NOTOK)
		    return i;
		break;

	    case RC_FIN: 
		if (ppid > 0)
		    (void) kill (ppid, SIGEMT);
		return EOF;

	    case RC_XXX: 
		padios (NULLCP, "%s", rc -> rc_data);

	    default: 
		(void) fmt2peer (RC_ERR, MSGSTR(PROTOSU, "pLOOP protocol screw-up")); /*MSG*/
		done (1);
	}
}

/*  */

static int  parse (char   *buffer,
		   struct Cmd *cmdp)
{
    int     argp = 0;
    char    c,
           *cp,
           *pp;

    cmdp -> line[0] = (char)NULL;
    pp = cmdp -> args[argp++] = cmdp -> line;
    cmdp -> redirect = NULL;
    cmdp -> direction = STDIO;
    cmdp -> stream = NULL;

    for (cp = buffer; c = *cp; cp++)
	if (!isspace (c))
	    break;
    if (c == (char)NULL) {
	if (vmh)
	    (void) fmt2peer (RC_EOF, MSGSTR(NULLCMD1, "null command")); /*MSG*/
	return NOTOK;
    }

    while (c = *cp++) {
	if (isspace (c)) {
	    while (isspace (c))
		c = *cp++;
	    if (c == (char)NULL)
		break;
	    *pp++ = (char)NULL;
	    cmdp -> args[argp++] = pp;
	    *pp = (char)NULL;
	}

	switch (c) {
	    case '"': 
		for (;;) {
		    switch (c = *cp++) {
			case '\0': 
			    padvise (NULLCP, MSGSTR(NOMATCH, "unmatched \"")); /*MSG*/
			    return NOTOK;
			case '"': 
			    break;
			case QUOTE: 
			    if ((c = *cp++) == (char)NULL)
				goto no_quoting;
			default: 
			    *pp++ = c;
			    continue;
		    }
		    break;
		}
		continue;

	    case QUOTE: 
		if ((c = *cp++) == (char)NULL) {
	    no_quoting: ;
		    padvise (NULLCP, MSGSTR(NOQUOTE, "the newline character can not be quoted")); /*MSG*/
		    return NOTOK;
		}

	    default: ;
		*pp++ = c;
		continue;

	    case '>': 
	    case '|': 
		if (pp == cmdp -> line) {
		    padvise (NULLCP, MSGSTR(INVNULLCMD, "invalid null command")); /*MSG*/
		    return NOTOK;
		}
		if (*cmdp -> args[argp - 1] == (char)NULL)
		    argp--;
		cmdp -> direction = c == '>' ? CRTIO : PIPIO;
		if (cmdp -> direction == CRTIO && (c = *cp) == '>') {
		    cmdp -> direction = APPIO;
		    cp++;
		}
		cmdp -> redirect = pp + 1;/* sigh */
		for (; c = *cp; cp++)
		    if (!isspace (c))
			break;
		if (c == (char)NULL) {
		    padvise (NULLCP, cmdp -> direction != PIPIO
			    ? MSGSTR(NORDNAM, "missing name for redirect") 
			    : MSGSTR(INVNULLCMD, "invalid null command")); /*MSG*/
		    return NOTOK;
		}
		(void) strcpy (cmdp -> redirect, cp);
		if (cmdp -> direction != PIPIO) {
		    for (; *cp; cp++)
			if (isspace (*cp)) {
			    padvise (NULLCP, MSGSTR(BADRDNAM, "bad name for redirect")); /*MSG*/
			    return NOTOK;
			}
		    if (expand (cmdp -> redirect) == NOTOK)
			return NOTOK;
		}
		break;
	}
	break;
    }

    *pp++ = (char)NULL;
    cmdp -> args[argp] = NULL;

    return OK;
}

/*  */

int	expand (redirect)
char   *redirect;
{
    char   *cp,
           *pp;
    char    path[BUFSIZ];
    struct passwd  *pw;

    if (*redirect != '~')
	return OK;

    if (cp = index (pp = redirect + 1, '/'))
	*cp++ = (char)NULL;
    if (*pp == (char)NULL)
	pp = mypath;
    else
	if (pw = getpwnam (pp))
	    pp = pw -> pw_dir;
	else {
	    padvise (NULLCP, MSGSTR(UNKNUSER, "unknown user: %s"), pp); /*MSG*/
	    return NOTOK;
	}

    (void) sprintf (path, "%s/%s", pp, cp ? cp : "");
    (void) strcpy (redirect, path);
    return OK;
}

/*  */

static int  init_io (register struct Cmd *cmdp,
		     int	vio)
{
    int     io,
            result;

    io = vmh;

    vmh = vio;
    result = initaux_io (cmdp);
    vmh = io;

    return result;
}


static int  initaux_io (register struct Cmd *cmdp)
{
    char   *mode;

    switch (cmdp -> direction) {
	case STDIO: 
	    return OK;

	case CRTIO: 
	case APPIO: 
	    mode = cmdp -> direction == CRTIO ? MSGSTR(WRITE, "write") : MSGSTR(APPEND, "append");  /*MSG*/
	    if ((cmdp -> stream = fopen (cmdp -> redirect, mode)) == NULL) {
		padvise (cmdp -> redirect, MSGSTR(UNABLE3, "unable to %s %s"), mode, cmdp -> redirect); /*MSG*/
		cmdp -> direction = STDIO;
		return NOTOK;
	    }
	    break;

	case PIPIO: 
	    if ((cmdp -> stream = popen (cmdp -> redirect, "w")) == NULL) {
		padvise (cmdp -> redirect, MSGSTR(NOPIPE2, "unable to pipe %s"), cmdp -> redirect); /*MSG*/
		cmdp -> direction = STDIO;
		return NOTOK;
	    }
	    (void) signal (SIGPIPE, pipeser);
	    broken_pipe = 0;
	    break;

	default: 
	    padios (NULLCP, MSGSTR(NOCMDRD, "unknown redirection for command")); /*MSG*/
    }

    (void) fflush (stdout);
    if (dup2 (fileno (cmdp -> stream), fileno (stdout)) == NOTOK)
	padios ("standard output", MSGSTR(NODUPSO4, "unable to dup2 standard output")); /*MSG*/
    clearerr (stdout);

    return OK;
}

/*  */

static  fin_io (register struct Cmd *cmdp, int	vio)
{
    int     io;

    io = vmh;

    vmh = vio;
    finaux_io (cmdp);
    vmh = io;
}


static int  finaux_io (register struct Cmd *cmdp)
{
    switch (cmdp -> direction) {
	case STDIO: 
	    return;

	case CRTIO: 
	case APPIO: 
	    (void) fflush (stdout);
	    (void) close (fileno (stdout));
	    if (ferror (stdout))
		padvise (NULLCP, MSGSTR(WPROB, "problems writing %s"), cmdp -> redirect); /*MSG*/
	    (void) fclose (cmdp -> stream);
	    break;

	case PIPIO: 
	    (void) fflush (stdout);
	    (void) close (fileno (stdout));
	    (void) pclose (cmdp -> stream);
	    (void) signal (SIGPIPE, SIG_DFL);
	    break;

	default: 
	    padios (NULLCP, MSGSTR(NOCMDRD, "unknown redirection for command")); /*MSG*/
    }

    if (dup2 (fileno (sp), fileno (stdout)) == NOTOK)
	padios ("standard output", MSGSTR(NODUPSO4, "unable to dup2 standard output")); /*MSG*/
    clearerr (stdout);

    cmdp -> direction = STDIO;
}

/*  */

static  m_init () {
    int     msgnum;

    for (msgnum = mp -> lowmsg; msgnum <= mp -> hghmsg; msgnum++)
	mp -> msgstats[msgnum] &= ~SELECTED;
    mp -> lowsel = mp -> hghsel = mp -> numsel = 0;
}


m_reset () {
    write_ids ();
    m_fmsg (mp);
    myname = NULL;
#ifdef	BPOP
    if (pmsh) {
	(void) pop_done ();
	pmsh = 0;
    }
#endif	BPOP
}

/*  */

void	m_setcur (mp, msgnum)
register struct msgs *mp;
register int msgnum;
{
    if (mp -> curmsg == msgnum)
	return;

    if (mp -> curmsg && Msgs[mp -> curmsg].m_scanl) {
	free (Msgs[mp -> curmsg].m_scanl);
	Msgs[mp -> curmsg].m_scanl = NULL;
    }
    if (Msgs[msgnum].m_scanl) {
	free (Msgs[msgnum].m_scanl);
	Msgs[msgnum].m_scanl = NULL;
    }

    mp -> curmsg = msgnum;
}

/*  */

/* ARGSUSED */

static void  intrser (int i)
{
#ifndef	BSD42
    (void) signal (SIGINT, (void(*)(int))intrser);
#endif	not BSD42

    discard (stdout);

    interrupted++;
#ifdef	BSD42
    if (should_intr)
	longjmp (sigenv, NOTOK);
#endif	BSD42
}


/* ARGSUSED */

static void  pipeser (int i)
{
#ifndef	BSD42
    (void) signal (SIGPIPE, (void(*)(int))pipeser);
#endif	not BSD42

    if (broken_pipe++ == 0)
	fprintf (stderr, MSGSTR(BROKE, "broken pipe\n")); /*MSG*/
    told_to_quit++;
    interrupted++;
#ifdef	BSD42
    if (should_intr)
	longjmp (sigenv, NOTOK);
#endif	BSD42
}


/* ARGSUSED */

static void  quitser (int     i)
{
#ifndef	BSD42
    (void) signal (SIGQUIT, (void(*)(int))quitser);
#endif	BSD42

    told_to_quit++;
    interrupted++;
#ifdef	BSD42
    if (should_intr)
	longjmp (sigenv, NOTOK);
#endif	BSD42
}

/*  */

static int  pINI () {
    int     i,
            vrsn;
    char   *bp;
    struct record   rcs,
                   *rc = &rcs;

    initrc (rc);

    switch (peer2rc (rc)) {
	case RC_INI: 
	    bp = rc -> rc_data;
	    while (isspace (*bp))
		bp++;
	    if (sscanf (bp, "%d", &vrsn) != 1) {
	bad_init: ;
		(void) fmt2peer (RC_ERR, MSGSTR(BADINIT, "bad init \"%s\""), rc -> rc_data); /*MSG*/
		done (1);
	    }
	    if (vrsn != RC_VRSN) {
		(void) fmt2peer (RC_ERR, MSGSTR(UNSUPP, "version %d unsupported"), vrsn); /*MSG*/
		done (1);
	    }

	    while (*bp && !isspace (*bp))
		bp++;
	    while (isspace (*bp))
		bp++;
	    if (sscanf (bp, "%d", &numwins) != 1 || numwins <= 0)
		goto bad_init;
	    if (numwins > NWIN)
		numwins = NWIN;

	    for (i = 1; i <= numwins; i++) {
		while (*bp && !isspace (*bp))
		    bp++;
		while (isspace (*bp))
		    bp++;
		if (sscanf (bp, "%d", &windows[i]) != 1 || windows[i] <= 0)
		    goto bad_init;
	    }
	    (void) rc2peer (RC_ACK, 0, NULLCP);
	    return OK;

	case RC_XXX: 
	    padios (NULLCP, "%s", rc -> rc_data);

	default: 
	    (void) fmt2peer (RC_ERR, MSGSTR(PROTOSU4, "pINI protocol screw-up")); /*MSG*/
	    done (1);		/* NOTREACHED */
    }
}

/*  */

/* ARGSUSED */

static int  pQRY (char   *str, int	scansw)
{
    if (pQRY1 (scansw) == NOTOK || pQRY2 () == NOTOK)
	return NOTOK;

    (void) rc2peer (RC_EOF, 0, NULLCP);
    return OK;
}
	
/*  */

static int  pQRY1 (int scansw)
{
    int     oldhgh;
    static int  lastlow = 0,
                lastcur = 0,
                lasthgh = 0,
		lastnum = 0;

    oldhgh = mp -> hghmsg;
    if (check_folder (scansw) && oldhgh < mp -> hghmsg) {
	switch (winX (STATUS)) {
	    case NOTOK: 
		return NOTOK;

	    case OK: 
		printf (MSGSTR(NEWMSGS, "new messages have arrived!")); /*MSG*/
		(void) fflush (stdout);
		(void) fflush (stderr);
		_exit (0);	/* NOTREACHED */

	    default: 
		lastlow = lastcur = lasthgh = lastnum = 0;
		break;
	}

	switch (winX (DISPLAY)) {
	    case NOTOK: 
		return NOTOK;

	    case OK: 
		scanrange (oldhgh + 1, mp -> hghmsg);
		(void) fflush (stdout);
		(void) fflush (stderr);
		_exit (0);	/* NOTREACHED */

	    default: 
		break;
	}
	return OK;
    }

    if (gap)
	switch (winX (STATUS)) {
	    case NOTOK:
		return NOTOK;

	    case OK:
		printf (MSGSTR(IDGAP2, "%s: gap in ID:s, last seen %d, lowest present %d\n"), myname ? myname : fmsh ? fmsh : mp -> foldpath, gap - 1,
		    readid (mp -> lowmsg)); /*MSG*/
		(void) fflush (stdout);
		(void) fflush (stderr);
		_exit (0);	/* NOTREACHED */

	    default:
		gap = 0;
		return OK;
	}

    if (mp -> lowmsg != lastlow
	    || mp -> curmsg != lastcur
	    || mp -> hghmsg != lasthgh
	    || mp -> nummsg != lastnum)
	switch (winX (STATUS)) {
	    case NOTOK: 
		return NOTOK;

	    case OK: 
		foldcmd (NULLVP);
		(void) fflush (stdout);
		(void) fflush (stderr);
		_exit (0);	/* NOTREACHED */

	    default: 
		lastlow = mp -> lowmsg;
		lastcur = mp -> curmsg;
		lasthgh = mp -> hghmsg;
		lastnum = mp -> nummsg;
		return OK;
	}

    return OK;
}

/*  */

static int  pQRY2 () {
    int     i,
            j,
	    k,
            msgnum,
            n;
    static int  cur = 0,
                num = 0,
		lo = 0,
		hi = 0;

    if (mp -> nummsg == 0 && mp -> nummsg != num)
	switch (winX (SCAN)) {
	    case NOTOK: 
		return NOTOK;

	    case OK: 
		printf (MSGSTR(EMPTY3, "empty!")); /*MSG*/
		(void) fflush (stdout);
		(void) fflush (stderr);
		_exit (0);	/* NOTREACHED */

	    default: 
		num = mp -> nummsg;
		return OK;
	}
    num = mp -> nummsg;

    i = 0;
    j = (k = windows[SCAN]) / 2;
    for (msgnum = mp -> curmsg; msgnum <= mp -> hghmsg; msgnum++)
	if (mp -> msgstats[msgnum] & EXISTS)
	    i++;
    if (i-- > 0)
	if (topcur)
	    k = i >= k ? 1 : k - i;
	else
	    k -= i > j ? j : i;

    i = j = 0;
    n = 1;
    for (msgnum = mp -> curmsg; msgnum >= mp -> lowmsg; msgnum--)
	if (mp -> msgstats[msgnum] & EXISTS) {
	    i = msgnum;
	    if (j == 0)
		j = msgnum;
	    if (n++ >= k)
		break;
	}
    for (msgnum = mp -> curmsg + 1; msgnum <= mp -> hghmsg; msgnum++)
	if (mp -> msgstats[msgnum] & EXISTS) {
	    if (i == 0)
		i = msgnum;
	    j = msgnum;
	    if (n++ >= windows[SCAN])
		break;
	}
    if (!topcur
	    && lo > 0
	    && hi > 0
	    && mp -> msgstats[lo] & EXISTS
	    && mp -> msgstats[hi] & EXISTS
	    && (lo < mp -> curmsg
		    || (lo == mp -> curmsg && lo == mp -> lowmsg))
	    && (mp -> curmsg < hi
		    || (hi == mp -> curmsg && hi == mp -> hghmsg))
	    && hi - lo == j - i)
	i = lo, j = hi;

    if (mp -> curmsg != cur || modified)
	switch (winN (NULLCMD, SCAN, 0)) {
	    case NOTOK: 
		return NOTOK;

	    case OK:
		return OK;

	    default: 
		scanrange (lo = i, hi = j);
		cur = mp -> curmsg;
		(void) winR (NULLCMD);
		return OK;
	}

    return OK;
}

/*  */

static int pCMD (char *str,
		 struct swit *sw,
		 struct Cmd *cmdp)
{
    int     i;

    if (*str == '?')
	switch (winX (DISPLAY)) {
	    case NOTOK: 
		return NOTOK;

	    case OK: 
		printf (MSGSTR(CMDS, "commands:\n")); /*MSG*/
		printsw (ALL, sw, "");
		printf (MSGSTR(QUIT2, "type ``quit'' to leave %s\n"), invo_name); /*MSG*/
		(void) fflush (stdout);
		(void) fflush (stderr);
		_exit (0);	/* NOTREACHED */

	    default: 
		(void) rc2peer (RC_EOF, 0, NULLCP);
		return NOTOK;
	}

    if (parse (str, cmdp) == NOTOK)
	return NOTOK;

    switch (i = smatch (cmdp -> args[0], sw)) {
	case AMBIGSW: 
	    switch (winX (DISPLAY)) {
		case NOTOK: 
		    return NOTOK;

		case OK: 
		    ambigsw (cmdp -> args[0], sw);
		    (void) fflush (stdout);
		    (void) fflush (stderr);
		    _exit (0);	/* NOTREACHED */

		default: 
		    (void) rc2peer (RC_EOF, 0, NULLCP);
		    return NOTOK;
	    }

	case UNKWNSW: 
	    (void) fmt2peer (RC_ERR,
		    MSGSTR(HELP2, "say what: ``%s'' -- type ? (or help) for help"), cmdp -> args[0]); /*MSG*/
	    return NOTOK;

	default: 
	    return i;
    }
}

/*  */

static int  pFIN () {
    int     status;

    switch (setjmp (peerenv)) {
	case OK: 
	    (void) signal (SIGALRM, (void(*)(int))alrmser);
	    (void) alarm (ALARM);

	    status = peerwait ();

	    (void) alarm (0);
	    return status;

	default: 
	    return NOTOK;
    }
}


static int  peerwait () {
    struct record   rcs,
                   *rc = &rcs;

    initrc (rc);

    switch (peer2rc (rc)) {
	case RC_QRY: 
	case RC_CMD: 
	    (void) rc2peer (RC_FIN, 0, NULLCP);
	    return OK;

	case RC_XXX: 
	    advise (NULLCP, "%s", rc -> rc_data);
	    return NOTOK;

	default: 
	    (void) fmt2peer (RC_FIN, MSGSTR(PROTOSU, "pLOOP protocol screw-up")); /*MSG*/
	    return NOTOK;
    }
}


/* ARGSUSED */

static void alrmser (int i)
{
    longjmp (peerenv, DONE);
}

/*  */

static int  ttyNaux (register struct Cmd *cmdp,
		     char   *s)
{
    struct record   rcs,
                   *rc = &rcs;

    initrc (rc);

    if (cmdp && init_io (cmdp, vmh) == NOTOK)
	return NOTOK;

    if (!fmsh)
	(void) fseek (fp, 0L, 0);/* XXX: fseek() too tricky for our own good */

    vmhtty = NOTOK;
    switch (rc2rc (RC_TTY, s ? strlen (s) : 0, s, rc)) {
	case RC_ACK: 
	    vmhtty = OK;	/* fall */
	case RC_ERR: 
	    break;

	case RC_XXX: 
	    padios (NULLCP, "%s", rc -> rc_data);/* NOTREACHED */

	default: 
	    (void) fmt2peer (RC_ERR, MSGSTR(PROTOSU2, "pTTY protocol screw-up")); /*MSG*/
	    done (1);		/* NOTREACHED */
    }

#ifdef	SIGTSTP
    (void) signal (SIGTSTP, (void(*)(int))tstat);
#endif	SIGTSTP
    return vmhtty;
}

/*  */

static int  ttyR (register struct Cmd *cmdp)
{
    struct record   rcs,
                   *rc = &rcs;

#ifdef	SIGTSTP
    (void) signal (SIGTSTP, SIG_IGN);
#endif	SIGTSTP

    if (vmhtty != OK)
	return NOTOK;

    initrc (rc);

    if (cmdp)
	fin_io (cmdp, 0);

    vmhtty = NOTOK;
    switch (rc2rc (RC_EOF, 0, NULLCP, rc)) {
	case RC_ACK: 
	    (void) rc2peer (RC_EOF, 0, NULLCP);
	    return OK;

	case RC_XXX: 
	    padios (NULLCP, "%s", rc -> rc_data);/* NOTREACHED */

	default: 
	    (void) fmt2peer (RC_ERR, MSGSTR(PROTOSU2, "pTTY protocol screw-up")); /*MSG*/
	    done (1);		/* NOTREACHED */
    }
}

/*  */

static int  winN (register struct Cmd *cmdp,
		  int	n,   int eof)
{
    int     i,
            pd[2];
    char    buffer[BUFSIZ];
    struct record   rcs,
                   *rc = &rcs;

    if (vmhpid == NOTOK)
	return OK;

    initrc (rc);

    if (!fmsh)
	(void) fseek (fp, 0L, 0);/* XXX: fseek() too tricky for our own good */

    vmhpid = OK;

    (void) sprintf (buffer, "%d", n);
    switch (str2rc (RC_WIN, buffer, rc)) {
	case RC_ACK: 
	    break;

	case RC_ERR: 
	    return NOTOK;

	case RC_XXX: 
	    padios (NULLCP, "%s", rc -> rc_data);

	default: 
	    (void) fmt2peer (RC_ERR, MSGSTR(PROTOSU3, "pWIN protocol screw-up")); /*MSG*/
	    done (1);
    }

    if (pipe (pd) == NOTOK) {
	(void) err2peer (RC_ERR, "pipe", MSGSTR(NOPIPE, "unable to pipe")); /*MSG*/
	return NOTOK;
    }

    switch (vmhpid = fork ()) {
	case NOTOK: 
	    (void) err2peer (RC_ERR, "fork", MSGSTR(NOFORK, "unable to fork")); /*MSG*/
	    (void) close (pd[0]);
	    (void) close (pd[1]);
	    return NOTOK;

	case OK: 
	    (void) close (pd[1]);
	    (void) signal (SIGPIPE, SIG_IGN);
	    while ((i = read (pd[0], buffer, sizeof buffer)) > 0)
		switch (rc2rc (RC_DATA, i, buffer, rc)) {
		    case RC_ACK: 
			break;

		    case RC_ERR: 
			_exit (1);

		    case RC_XXX: 
			advise (NULLCP, "%s", rc -> rc_data);
			_exit (2);

		    default: 
			(void) fmt2peer (RC_ERR, MSGSTR(PROTOSU3, "pWIN protocol screw-up")); /*MSG*/
			_exit (2);
		}
	    if (i == OK)
		switch (rc2rc (RC_EOF, 0, NULLCP, rc)) {
		    case RC_ACK: 
			if (eof)
			    (void) rc2peer (RC_EOF, 0, NULLCP);
			i = 0;
			break;

		    case RC_XXX: 
			advise (NULLCP, "%s", rc -> rc_data);
			i = 2;
			break;

		    default: 
			(void) fmt2peer (RC_ERR, MSGSTR(PROTOSU3, "pWIN protocol screw-up")); /*MSG*/
			i = 2;
			break;
		}
	    if (i == NOTOK)
		(void) err2peer (RC_ERR, "pipe", MSGSTR(PRERR, "error reading from pipe")); /*MSG*/
	    (void) close (pd[0]);
	    _exit (i != NOTOK ? i : 1);

	default: 
	    if ((vmhfd0 = dup (fileno (stdin))) == NOTOK)
		padios ("standard input", MSGSTR(NODUPSI, "unable to dup standard input")); /*MSG*/
	    if ((vmhfd1 = dup (fileno (stdout))) == NOTOK)
		padios ("standard output", MSGSTR(NODUPSO3, "unable to dup standard output")); /*MSG*/
	    if ((vmhfd2 = dup (fileno (stderr))) == NOTOK)
		padios ("diagnostic output", MSGSTR(NODUPDO, "unable to dup diagnostic output")); /*MSG*/

	    (void) close (0);
	    if ((i = open ("/dev/null", 0)) != NOTOK && i != fileno (stdin)) {
		(void) dup2 (i, fileno (stdin));
		(void) close (i);
	    }

	    (void) fflush (stdout);
	    if (dup2 (pd[1], fileno (stdout)) == NOTOK)
		padios ("standard output", MSGSTR(NODUPSO4, "unable to dup2 standard output")); /*MSG*/
	    clearerr (stdout);

	    (void) fflush (stderr);
	    if (dup2 (pd[1], fileno (stderr)) == NOTOK)
		padios ("diagnostic output", MSGSTR(NODUPDO2, "unable to dup2 diagnostic output")); /*MSG*/
	    clearerr (stderr);

	    if (cmdp && init_io (cmdp, 0) == NOTOK)
		return NOTOK;
	    pstat = signal (SIGPIPE, pipeser);
	    broken_pipe = 1;

	    (void) close (pd[0]);
	    (void) close (pd[1]);

	    return vmhpid;
    }
}

/*  */

static int  winR (register struct Cmd *cmdp)
{
    int     status;

    if (vmhpid <= OK)
	return NOTOK;

    if (cmdp)
	fin_io (cmdp, 0);

    if (dup2 (vmhfd0, fileno (stdin)) == NOTOK)
	padios ("standard input", MSGSTR(NODUPSI2, "unable to dup2 standard input")); /*MSG*/
    clearerr (stdin);
    (void) close (vmhfd0);

    (void) fflush (stdout);
    if (dup2 (vmhfd1, fileno (stdout)) == NOTOK)
	padios ("standard output", MSGSTR(NODUPSO4, "unable to dup2 standard output")); /*MSG*/
    clearerr (stdout);
    (void) close (vmhfd1);

    (void) fflush (stderr);
    if (dup2 (vmhfd2, fileno (stderr)) == NOTOK)
	padios ("diagnostic output", MSGSTR(NODUPDO2, "unable to dup2 diagnostic output")); /*MSG*/
    clearerr (stderr);
    (void) close (vmhfd2);

    (void) signal (SIGPIPE, (void(*)(int))pstat);

    if ((status = pidwait (vmhpid, OK)) == 2)
	done (1);

    vmhpid = OK;
    return (status == 0 ? OK : NOTOK);
}

/*  */

static int  winX (int	n)
{
    int     i,
            pid,
            pd[2];
    char    buffer[BUFSIZ];
    struct record   rcs,
                   *rc = &rcs;

    initrc (rc);

    if (!fmsh)
	(void) fseek (fp, 0L, 0);/* XXX: fseek() too tricky for our own good */

    (void) sprintf (buffer, "%d", n);
    switch (str2rc (RC_WIN, buffer, rc)) {
	case RC_ACK: 
	    break;

	case RC_ERR: 
	    return NOTOK;

	case RC_XXX: 
	    padios (NULLCP, "%s", rc -> rc_data);

	default: 
	    (void) fmt2peer (RC_ERR, MSGSTR(PROTOSU3, "pWIN protocol screw-up")); /*MSG*/
	    done (1);
    }

    if (pipe (pd) == NOTOK) {
	(void) err2peer (RC_ERR, "pipe", MSGSTR(NOPIPE, "unable to pipe")); /*MSG*/
	return NOTOK;
    }

    switch (pid = fork ()) {
	case NOTOK: 
	    (void) err2peer (RC_ERR, "fork", MSGSTR(NOFORK, "unable to fork")); /*MSG*/
	    (void) close (pd[0]);
	    (void) close (pd[1]);
	    return NOTOK;

	case OK: 
	    (void) close (fileno (stdin));
	    if ((i = open ("/dev/null", 0)) != NOTOK && i != fileno (stdin)) {
		(void) dup2 (i, fileno (stdin));
		(void) close (i);
	    }
	    (void) dup2 (pd[1], fileno (stdout));
	    (void) dup2 (pd[1], fileno (stderr));
	    (void) close (pd[0]);
	    (void) close (pd[1]);
	    vmhpid = NOTOK;
	    return OK;

	default: 
	    (void) close (pd[1]);
	    while ((i = read (pd[0], buffer, sizeof buffer)) > 0)
		switch (rc2rc (RC_DATA, i, buffer, rc)) {
		    case RC_ACK: 
			break;

		    case RC_ERR: 
			(void) close (pd[0]);
			(void) pidwait (pid, OK);
			return NOTOK;

		    case RC_XXX: 
			padios (NULLCP, "%s", rc -> rc_data);

		    default: 
			(void) fmt2peer (RC_ERR, MSGSTR(PROTOSU3, "pWIN protocol screw-up")); /*MSG*/
			done (1);
		}
	    if (i == OK)
		switch (rc2rc (RC_EOF, 0, NULLCP, rc)) {
		    case RC_ACK: 
			break;

		    case RC_XXX: 
			padios (NULLCP, "%s", rc -> rc_data);

		    default: 
			(void) fmt2peer (RC_ERR, MSGSTR(PROTOSU3, "pWIN protocol screw-up")); /*MSG*/
			done (1);
		}
	    if (i == NOTOK)
		(void) err2peer (RC_ERR, "pipe", MSGSTR(PRERR, "error reading from pipe")); /*MSG*/

	    (void) close (pd[0]);
	    (void) pidwait (pid, OK);
	    return (i != NOTOK ? pid : NOTOK);
    }
}

/*  */

/* VARARGS2 */

void	padios (what, fmt, a, b, c, d, e, f)
char   *what,
       *fmt,
       *a,
       *b,
       *c,
       *d,
       *e,
       *f;
{
    if (vmh) {
	(void) err2peer (RC_FIN, what, fmt, a, b, c, d, e, f);
	(void) rcdone ();
    }
    else
	advise (what, fmt, a, b, c, d, e, f);

    done (1);
}


/* VARARGS2 */

void	padvise (what, fmt, a, b, c, d, e, f)
char   *what,
       *fmt,
       *a,
       *b,
       *c,
       *d,
       *e,
       *f;
{
    if (vmh)
	(void) err2peer (RC_ERR, what, fmt, a, b, c, d, e, f);
    else
	advise (what, fmt, a, b, c, d, e, f);
}
