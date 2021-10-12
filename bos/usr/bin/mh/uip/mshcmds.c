static char sccsid[] = "@(#)67	1.10  src/bos/usr/bin/mh/uip/mshcmds.c, cmdmh, bos411, 9428A410j 2/2/93 11:22:56";
/* 
 * COMPONENT_NAME: CMDMH mshcmds.c
 * 
 * FUNCTIONS: MSGSTR, ask, burst, copy_digest, copy_message, distcmd, 
 *            eom_action, explcmd, filecmd, filehak, foldcmd, forkcmd, 
 *            forw, forwcmd, gettws, helpcmd, markcmd, mhl_action, 
 *            msgsort, pack, packcmd, packhak, pickcmd, process, 
 *            replcmd, rmm, rmmcmd, scancmd, show, showcmd, sortcmd 
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
/* static char sccsid[] = "mshcmds.c	7.1 87/10/13 17:30:58"; */

/* mshcmds.c - command handlers in msh */

#include "mh.h"
#include "dropsbr.h"
#include "formatsbr.h"
#include "scansbr.h"
#include "tws.h"
#include <stdio.h>
#include "mts.h"
#include <ctype.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include "mshsbr.h"
#include <sys/types.h>
#include <sys/stat.h>

#include "mh_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 

/*  */

char *mktemp();

extern int errno;
static  burst (struct Msg *,int,int,int,int);
static	forw (char *, char *, int, char **);
static  rmm();
static  show (int);
static	int eom_action (int);
static	FP mhl_action (char *);
static  ask (int);
static struct tws  *gettws (char  *, int);
static int  msgsort (struct Msg *, struct Msg *);
static int  process (int, char *, int, char **);
static  copy_message (int ,FILE * );
static  copy_digest (int , FILE *);
				/* BURST */
static char delim3[] = "-------";/* from burst.c */


				/* SHOW */
static int  mhlnum;
static FILE *mhlfp;

void clear_screen ();


				/* SORTM */
/*  */

forkcmd (args, pgm)
char  **args,
       *pgm;
{
    int     child_id;
    char   *vec[MAXARGS];

    vec[0] = r1bindex (pgm, '/');
    (void) copyip (args, vec + 1);

    if (fmsh) {
	(void) m_delete (pfolder);
	m_replace (pfolder, fmsh);
	m_sync (mp);
	m_update ();
    }
    (void) fflush (stdout);
    switch (child_id = fork ()) {
	case NOTOK: 
	    advise ("fork", MSGSTR(NOFORK, "unable to fork")); /*MSG*/
	    return;

	case OK: 
	    closefds (3);

	    (void) signal (SIGINT, (void(*)(int))istat);
	    (void) signal (SIGQUIT, (void(*)(int))qstat);

	    execvp (pgm, vec);
	    fprintf (stderr, MSGSTR(NOEXEC, "unable to exec ")); /*MSG*/
	    perror (cmd_name);
	    _exit (1);

	default: 
	    (void) pidXwait (child_id, NULLCP);
	    break;
    }
    if (fmsh) {			/* assume the worst case */
	mp -> msgflags |= MODIFIED;
	modified++;
    }
}

/*  */

static struct swit distswit[] = {
#define	DIANSW	0
    "annotate", 0,
#define	DINANSW	1
    "noannotate", 0,
#define	DIDFSW	2
    "draftfolder +folder", 0,
#define	DIDMSW	3
    "draftmessage msg", 0,
#define	DINDFSW	4
    "nodraftfolder", 0,
#define	DIEDTSW	5
    "editor editor", 0,
#define	DINEDSW	6
    "noedit", 0,
#define	DIFRMSW	7
    "form formfile", 0,
#define	DIINSW	8
    "inplace", 0,
#define	DININSW	9
    "noinplace", 0,
#define	DIWHTSW	10
    "whatnowproc program", 0,
#define	DINWTSW	11
    "nowhatnowproc", 0,
#define	DIHELP	12
    "help", 4,

    NULL, (int)NULL
};

/*  */

distcmd (args)
char  **args;
{
    int     vecp = 1;
    char   *cp,
           *msg = NULL,
            buf[BUFSIZ],
           *vec[MAXARGS];

    if (fmsh) {
	forkcmd (args, cmd_name);
	return;
    }

    while (cp = *args++) {
	if (*cp == '-')
	    switch (smatch (++cp, distswit)) {
		case AMBIGSW: 
		    ambigsw (cp, distswit);
		    return;
		case UNKWNSW: 
		    fprintf (stderr, MSGSTR(NOTKNOWN, "-%s unknown\n"), cp); /*MSG*/
		    return;
		case DIHELP: 
		    (void) sprintf (buf, MSGSTR(DIHELP1, "%s [msgs] [switches]"), cmd_name); /*MSG*/
		    help (buf, distswit);
		    return;

		case DIANSW:	/* not implemented */
		case DINANSW: 
		case DIINSW: 
		case DININSW: 
		    continue;

		case DINDFSW:
		case DINEDSW:
		case DINWTSW:
		    vec[vecp++] = --cp;
		    continue;

		case DIEDTSW: 
		case DIFRMSW: 
		case DIDFSW:
		case DIDMSW:
		case DIWHTSW:
		    vec[vecp++] = --cp;
		    if (!(cp = *args++) || *cp == '-') {
			advise (NULLCP, MSGSTR(NOARG, "missing argument to %s"), args[-2]); /*MSG*/
			return;
		    }
		    vec[vecp++] = cp;
		    continue;
	    }
	if (*cp == '+' || *cp == '@') {
	    advise (NULLCP, MSGSTR(SORRY, "sorry, no folders allowed!")); /*MSG*/
	    return;
	}
	else
	    if (msg) {
		advise (NULLCP, MSGSTR(ONEMSG, "only one message at a time!")); /*MSG*/
		return;
	    }
	    else
		msg = cp;
    }

    vec[0] = cmd_name;
    vec[vecp++] = "-file";
    vec[vecp] = NULL;
    if (!msg)
	msg = "cur";
    if (!m_convert (mp, msg))
	return;
    m_setseq (mp);

    if (mp -> numsel > 1) {
	advise (NULLCP, MSGSTR(ONEMSG, "only one message at a time!")); /*MSG*/
	return;
    }
    (void) process (mp -> hghsel, cmd_name, vecp, vec);
    m_setcur (mp, mp -> hghsel);
}

/*  */

static struct swit explswit[] = {
#define	EXINSW	0
    "inplace", 0,
#define	EXNINSW	1
    "noinplace", 0,
#define	EXQISW	2
    "quiet", 0,
#define	EXNQISW	3
    "noquiet", 0,
#define	EXVBSW	4
    "verbose", 0,
#define	EXNVBSW	5
    "noverbose", 0,
#define	EXHELP	6
    "help", 4,

    NULL, (int)NULL
};

/*  */

explcmd (args)
char  **args;
{
    int     inplace = 0,
            quietsw = 0,
            verbosw = 0,
            msgp = 0,
            hi,
            msgnum;
    char   *cp,
            buf[BUFSIZ],
           *msgs[MAXARGS];
    struct Msg *smsgs;

    if (fmsh) {
	forkcmd (args, cmd_name);
	return;
    }

    while (cp = *args++) {
	if (*cp == '-')
	    switch (smatch (++cp, explswit)) {
		case AMBIGSW: 
		    ambigsw (cp, explswit);
		    return;
		case UNKWNSW: 
		    fprintf (stderr, MSGSTR(NOTKNOWN, "-%s unknown\n"), cp); /*MSG*/
		    return;
		case EXHELP: 
		    (void) sprintf (buf, MSGSTR(DIHELP1, "%s [msgs] [switches]"), cmd_name); /*MSG*/
		    help (buf, explswit);
		    return;

		case EXINSW: 
		    inplace++;
		    continue;
		case EXNINSW: 
		    inplace = 0;
		    continue;
		case EXQISW: 
		    quietsw++;
		    continue;
		case EXNQISW: 
		    quietsw = 0;
		    continue;
		case EXVBSW: 
		    verbosw++;
		    continue;
		case EXNVBSW: 
		    verbosw = 0;
		    continue;
	    }
	if (*cp == '+' || *cp == '@') {
	    advise (NULLCP, MSGSTR(SORRY, "sorry, no folders allowed!")); /*MSG*/
	    return;
	}
	else
	    msgs[msgp++] = cp;
    }

    if (!msgp)
	msgs[msgp++] = "cur";
    for (msgnum = 0; msgnum < msgp; msgnum++)
	if (!m_convert (mp, msgs[msgnum]))
	    return;
    m_setseq (mp);

    smsgs = (struct Msg *)
		calloc ((unsigned) (MAXFOLDER + 2), sizeof *smsgs);
    if (smsgs == NULL)
	adios (NULLCP, MSGSTR(NOAFSTOR, "unable to allocate folder storage")); /*MSG*/

    hi = mp -> hghmsg + 1;
    interrupted = 0;
    for (msgnum = mp -> lowsel;
	    msgnum <= mp -> hghsel && !interrupted;
	    msgnum++)
	if (mp -> msgstats[msgnum] & SELECTED)
	    if (burst (smsgs, msgnum, inplace, quietsw, verbosw) != OK)
		break;

    free ((char *) smsgs);

    if (inplace)
	m_setcur (mp, mp -> lowsel);
    else
	if (hi <= mp -> hghmsg)
	    m_setcur (mp, hi);

    mp -> msgflags |= MODIFIED;
    modified++;
}

/*  */

static  burst (struct Msg *smsgs,
	       int     msgnum,
	       int     inplace,
	       int     quietsw,
	       int     verbosw)
{
    int     i,
            j,
            ld3,
	    wasdlm,
            msgp;
    long    pos;
    char    c,
            buffer[BUFSIZ];
    register FILE *zp;

    ld3 = strlen (delim3);

    if (Msgs[msgnum].m_scanl) {
	free (Msgs[msgnum].m_scanl);
	Msgs[msgnum].m_scanl = NULL;
    }

    pos = ftell (zp = msh_ready (msgnum, 1));
    for (msgp = 1; msgp <= MAXFOLDER;) {
	while (fgets (buffer, sizeof buffer, zp) != NULL
		&& buffer[0] == '\n'
		&& pos < Msgs[msgnum].m_stop)
	    pos += (long) strlen (buffer);
	if (feof (zp) || pos >= Msgs[msgnum].m_stop)
	    break;
	(void) fseek (zp, pos, 0);
	smsgs[msgp].m_start = pos;

	for (c = (char)NULL;
		fgets (buffer, sizeof buffer, zp) != NULL
		&& pos < Msgs[msgnum].m_stop;
		c = buffer[0])
	    if (strncmp (buffer, delim3, ld3) == 0
		    && peekc (zp) == '\n'
		    && (msgp == 1 || c == '\n'))
		break;
	    else
		pos += (long) strlen (buffer);

	wasdlm = strncmp (buffer, delim3, ld3) == 0;
	if (smsgs[msgp].m_start != pos)
	    smsgs[msgp++].m_stop = c == '\n' && wasdlm ? pos - 1 : pos;
	if (feof (zp) || pos >= Msgs[msgnum].m_stop) {
	    if (wasdlm) {
		smsgs[msgp - 1].m_stop -= ((long) strlen (buffer) + 1);
		msgp++;		/* fake "End of XXX Digest" */
	    }
	    break;
	}
	pos += (long) strlen (buffer);
    }

    switch (--msgp) {		/* toss "End of XXX Digest" */
	case 0: 
	    adios (NULLCP, MSGSTR(BIGLOSE3, "burst() botch -- you lose big")); /*MSG*/

	case 1: 
	    if (!quietsw)
		printf (MSGSTR(NODFORM, "message %d not in digest format\n"), msgnum); /*MSG*/
	    return OK;

	default: 
	    if (verbosw) {
		if (msgp - 1 != 1)	
		    printf (MSGSTR(EXP, "%d messages exploded from digest %d\n"), msgp - 1, msgnum); /*MSG*/
		else
		    printf (MSGSTR(EXP2, "1 message exploded from digest %d\n"), msgnum); /*MSG*/
		}
	    if (msgp == 2)
		msgp++;
	    break;
    }

    msgp--;
    if ((i = msgp + mp -> hghmsg) > MAXFOLDER) {
	advise (NULLCP, MSGSTR(MANYMSGS, "more than %d messages"), MAXFOLDER); /*MSG*/
	return NOTOK;
    }
    if ((mp = m_remsg (mp, 0, i)) == NULL)
	adios (NULLCP, MSGSTR(NOAFSTOR, "unable to allocate folder storage")); /*MSG*/

    j = mp -> hghmsg;
    mp -> hghmsg += msgp - 1;
    mp -> nummsg += msgp - 1;
    if (mp -> hghsel > msgnum)
	mp -> hghsel += msgp - 1;

    if (inplace && msgp > 1)
	for (i = mp -> hghmsg; j > msgnum; i--, j--) {
	    if (verbosw)
		printf (MSGSTR(MTM, "message %d becomes message %d\n"), j, i); /*MSG*/

	    Msgs[i].m_bboard_id = Msgs[j].m_bboard_id;
	    Msgs[i].m_top = Msgs[j].m_top;
	    Msgs[i].m_start = Msgs[j].m_start;
	    Msgs[i].m_stop = Msgs[j].m_stop;
	    Msgs[i].m_scanl = NULL;
	    if (Msgs[j].m_scanl) {
		free (Msgs[j].m_scanl);
		Msgs[j].m_scanl = NULL;
	    }
	    mp -> msgstats[i] = mp -> msgstats[j];
	}

    if (Msgs[msgnum].m_bboard_id == 0)
	(void) readid (msgnum);

    mp -> msgstats[msgnum] &= ~SELECTED;
    i = inplace ? msgnum + msgp - 1 : mp -> hghmsg;
    for (j = msgp; j >= (inplace ? 1 : 2); i--, j--) {
	if (verbosw && i != msgnum)
	    printf (MSGSTR(TRANS, "message %d of digest %d becomes message %d\n"), j, msgnum, i); /*MSG*/

	Msgs[i].m_bboard_id = Msgs[msgnum].m_bboard_id;
	Msgs[i].m_top = Msgs[j].m_top;
	Msgs[i].m_start = smsgs[j].m_start;
	Msgs[i].m_stop = smsgs[j].m_stop;
	Msgs[i].m_scanl = NULL;
	mp -> msgstats[i] = mp -> msgstats[msgnum];
    }

    return OK;
}

/*  */

static struct swit fileswit[] = {
#define	FIDRFT	0
    "draft", 0,
#define	FILINK	1
    "link", 0,
#define	FINLINK	2
    "nolink", 0,
#define	FIPRES	3
    "preserve", 0,
#define FINPRES	4
    "nopreserve", 0,
#define	FISRC	5
    "src +folder", 0,
#define	FIFILE	6
    "file file", 0,
#define	FIHELP	7
    "help", 4,

    NULL, (int)NULL
};

/*  */

filecmd (args)
char  **args;
{
    int	    linksw = 0,
	    msgp = 0,
            vecp = 1,
	    i,
            msgnum;
    char   *cp,
            buf[BUFSIZ],
           *msgs[MAXARGS],
           *vec[MAXARGS];

    if (fmsh) {
	forkcmd (args, cmd_name);
	return;
    }

    while (cp = *args++) {
	if (*cp == '-')
	    switch (i = smatch (++cp, fileswit)) {
		case AMBIGSW: 
		    ambigsw (cp, fileswit);
		    return;
		case UNKWNSW: 
		    fprintf (stderr, MSGSTR(NOTKNOWN, "-%s unknown\n"), cp); /*MSG*/
		    return;
		case FIHELP: 
		    (void) sprintf (buf, MSGSTR(FIHELP1, "%s +folder... [msgs] [switches]"), cmd_name); /*MSG*/
		    help (buf, fileswit);
		    return;

		case FILINK:
		    linksw++;
		    continue;
		case FINLINK: 
		    linksw = 0;
		    continue;

		case FIPRES: 
		case FINPRES: 
		    continue;

		case FISRC: 
		case FIDRFT:
		case FIFILE: 
		    advise (NULLCP, MSGSTR(SORRY1, "sorry, -%s not allowed!"), fileswit[i].sw); /*MSG*/
		    return;
	    }
	if (*cp == '+' || *cp == '@')
	    vec[vecp++] = cp;
	else
	    msgs[msgp++] = cp;
    }

    vec[0] = cmd_name;
    vec[vecp++] = "-file";
    vec[vecp] = NULL;
    if (!msgp)
	msgs[msgp++] = "cur";
    for (msgnum = 0; msgnum < msgp; msgnum++)
	if (!m_convert (mp, msgs[msgnum]))
	    return;
    m_setseq (mp);

    interrupted = 0;
    for (msgnum = mp -> lowsel;
	    msgnum <= mp -> hghsel && !interrupted;
	    msgnum++)
	if (mp -> msgstats[msgnum] & SELECTED)
	    if (process (msgnum, fileproc, vecp, vec)) {
		mp -> msgstats[msgnum] &= ~SELECTED;
		mp -> numsel--;
	    }

    if (mp -> numsel != mp -> nummsg || linksw)
	m_setcur (mp, mp -> hghsel);
    if (!linksw)
	rmm ();
}

/*  */

int	filehak (args)
char  **args;
{
    int	    result,
	    vecp = 0;
    char   *cp,
	   *cwd,
           *vec[MAXARGS];

    while (cp = *args++) {
	if (*cp == '-')
	    switch (smatch (++cp, fileswit)) {
		case AMBIGSW: 
		case UNKWNSW: 
		case FIHELP: 
		    return NOTOK;

		case FILINK:
		case FINLINK: 
		case FIPRES: 
		case FINPRES: 
		    continue;

		case FISRC: 
		case FIDRFT:
		case FIFILE: 
		    return NOTOK;
	    }
	if (*cp == '+' || *cp == '@')
	    vec[vecp++] = cp;
    }
    vec[vecp] = NULL;

    result = NOTOK;
    cwd = NULL;
    for (vecp = 0; (cp = vec[vecp]) && result == NOTOK; vecp++) {
	if (cwd == NULL)
	    cwd = getcpy (pwd ());
	(void) chdir (m_maildir (""));
	cp = path (cp + 1, *cp == '+' ? TFOLDER : TSUBCWF);
	if (access (m_maildir (cp), 0) == NOTOK)
	    result = OK;
	free (cp);
    }
    if (cwd)
	(void) chdir (cwd);

    return result;
}

/*  */

static struct swit foldswit[] = {
#define	FLALSW	0
    "all", 0,
#define	FLFASW	1
    "fast", 0,
#define	FLNFASW	2
    "nofast", 0,
#define	FLHDSW	3
    "header", 0,
#define	FLNHDSW	4
    "noheader", 0,
#define	FLPKSW	5
    "pack", 0,
#define	FLNPKSW	6
    "nopack", 0,
#define	FLRCSW	7
    "recurse", 0,
#define	FLNRCSW	8
    "norecurse", 0,
#define	FLTLSW	9
    "total", 0,
#define	FLNTLSW	10
    "nototal", 0,
#define	FLPRSW	11
    "print", 0,
#define	FLPUSW	12
    "push", 0,
#define	FLPOSW	13
    "pop", 0,
#define	FLLISW	14
    "list", 0,
#define	FLHELP	15
    "help", 4,

    NULL, (int)NULL
};

/*  */

foldcmd (args)
char  **args;
{
    int     fastsw = 0,
            headersw = 0,
	    packsw = 0,
	    hole,
	    msgnum;
    char   *cp,
           *folder = NULL,
           *msg = NULL,
            buf[BUFSIZ],
	  **vec = args;

    if (args == NULL)
	goto fast;

    while (cp = *args++) {
	if (*cp == '-')
	    switch (smatch (++cp, foldswit)) {
		case AMBIGSW: 
		    ambigsw (cp, foldswit);
		    return;
		case UNKWNSW: 
		    fprintf (stderr, MSGSTR(NOTKNOWN, "-%s unknown\n"), cp); /*MSG*/
		    return;
		case FLHELP: 
		    (void) sprintf (buf, MSGSTR(UMSG, "%s [+folder] [msg] [switches]"), cmd_name); /*MSG*/
		    help (buf, foldswit);
		    return;

		case FLALSW:	/* not implemented */
		case FLRCSW: 
		case FLNRCSW: 
		case FLTLSW: 
		case FLNTLSW: 
		case FLPRSW:
		case FLPUSW:
		case FLPOSW:
		case FLLISW:
		    continue;

		case FLFASW: 
		    fastsw++;
		    continue;
		case FLNFASW: 
		    fastsw = 0;
		    continue;
		case FLHDSW: 
		    headersw++;
		    continue;
		case FLNHDSW: 
		    headersw = 0;
		    continue;
		case FLPKSW: 
		    packsw++;
		    continue;
		case FLNPKSW: 
		    packsw = 0;
		    continue;
	    }
	if (*cp == '+' || *cp == '@')
	    if (folder) {
		advise (NULLCP, MSGSTR(ONLYONE, "only one folder at a time!\n")); /*MSG*/
		return;
	    }
	    else
		folder = fmsh ? path (cp + 1, *cp == '+' ? TFOLDER : TSUBCWF)
			    : cp + 1;
	else
	    if (msg) {
		advise (NULLCP, MSGSTR(ONLYONE1, "only one message at a time!\n")); /*MSG*/
		return;
	    }
	    else
		msg = cp;
    }

    if (folder) {
	if (*folder == (char)NULL) {
	    advise (NULLCP, MSGSTR(NONULL, "null folder names are not permitted")); /*MSG*/
	    return;
	}
	if (fmsh) {
	    if (access (m_maildir (folder), 04) == NOTOK) {
		advise (folder, MSGSTR(NOREAD, "unable to read %s"), folder); /*MSG*/
		return;
	    }
	}
	else {
	    (void) strcpy (buf, folder);
	    if (expand (buf) == NOTOK)
		return;
	    folder = buf;
	    if (access (folder, 04) == NOTOK) {
		advise (folder, MSGSTR(NOREAD, "unable to read %s"), folder); /*MSG*/
		return;
	    }
	}
	m_reset ();

	if (fmsh)
	    fsetup (folder);
	else
	    setup (folder);
	readids (0);
	display_info (0);
    }

    if (msg) {
	if (!m_convert (mp, msg))
	    return;
	m_setseq (mp);

	if (mp -> numsel > 1) {
	    advise (NULLCP, MSGSTR(ONEMSG, "only one message at a time!")); /*MSG*/
	    return;
	}
	m_setcur (mp, mp -> hghsel);
    }

    if (packsw) {
	if (fmsh) {
	    forkcmd (vec, cmd_name);
	    return;
	}

	if (mp -> lowmsg > 1 && (mp = m_remsg (mp, 1, mp -> hghmsg)) == NULL)
	    adios (NULLCP, MSGSTR(NOAFSTOR, "unable to allocate folder storage")); /*MSG*/
	for (msgnum = mp -> lowmsg, hole = 1; msgnum <= mp -> hghmsg; msgnum++)
	    if (mp -> msgstats[msgnum] & EXISTS) {
		if (msgnum != hole) {
		    Msgs[hole].m_bboard_id = Msgs[msgnum].m_bboard_id;
		    Msgs[hole].m_top = Msgs[msgnum].m_top;
		    Msgs[hole].m_start = Msgs[msgnum].m_start;
		    Msgs[hole].m_stop = Msgs[msgnum].m_stop;
		    Msgs[hole].m_scanl = NULL;
		    if (Msgs[msgnum].m_scanl) {
			free (Msgs[msgnum].m_scanl);
			Msgs[msgnum].m_scanl = NULL;
		    }
		    mp -> msgstats[hole] = mp -> msgstats[msgnum];
		    if (mp -> curmsg == msgnum)
			m_setcur (mp, hole);
		}
		hole++;
	    }
	if (mp -> nummsg > 0) {
	    mp -> lowmsg = 1;
	    mp -> hghmsg = hole - 1;
	}
	mp -> msgflags |= MODIFIED;
	modified++;
    }

fast: ;
    if (fastsw)
	printf ("%s\n", fmsh ? fmsh : mp -> foldpath);
    else {
	if (headersw)
	    printf (MSGSTR(HEADER1, "\t\tFolder  %*s# of messages (%*srange%*s); cur%*smsg\n"), 
		DMAXFOLDER, "", DMAXFOLDER - 2, "", DMAXFOLDER - 2, "",
		DMAXFOLDER - 2, ""); /*MSG*/
	printf (args ? "%22s  " : "%s ", fmsh ? fmsh : mp -> foldpath);
	if (mp -> hghmsg == 0)
	    printf (MSGSTR(NOTANY, "has   no messages%*s"), 
		    mp -> msgflags & OTHERS ? DMAXFOLDER * 2 + 4 : 0, ""); /*MSG*/
	else {
	    if (mp -> nummsg != 1)	
	        printf (MSGSTR(MULTIPLE, "has %*d messages (%*d-%*d)"),
		    DMAXFOLDER, mp -> nummsg, 
		    DMAXFOLDER, mp -> lowmsg, DMAXFOLDER, mp -> hghmsg); /*MSG*/
	    else	
	        printf (MSGSTR(ONEMSG2, "has %*d message (%*d-%*d)"),
		    DMAXFOLDER, mp -> nummsg, 
		    DMAXFOLDER, mp -> lowmsg, DMAXFOLDER, mp -> hghmsg); /*MSG*/
	    if (mp -> curmsg >= mp -> lowmsg
		    && mp -> curmsg <= mp -> hghmsg)
		printf ("; cur=%*d", DMAXFOLDER, mp -> curmsg);
	}
	printf (".\n");
    }
}

/*  */

static struct swit forwswit[] = {
#define	FOANSW	0
    "annotate", 0,
#define	FONANSW	1
    "noannotate", 0,
#define	FODFSW	2
    "draftfolder +folder", 0,
#define	FODMSW	3
    "draftmessage msg", 0,
#define	FONDFSW	4
    "nodraftfolder", 0,
#define	FOEDTSW	5
    "editor editor", 0,
#define	FONEDSW	6
    "noedit", 0,
#define	FOFTRSW	7
    "filter filterfile", 0,
#define	FOFRMSW	8
    "form formfile", 0,
#define	FOFTSW	9
    "format", 5,
#define	FONFTSW	10
    "noformat", 7,
#define	FOINSW	11
    "inplace", 0,
#define	FONINSW	12
    "noinplace", 0,
#define	FOWHTSW	13
    "whatnowproc program", 0,
#define	FONWTSW	14
    "nowhatnow", 0,
#define	FOHELP	15
    "help", 4,

    NULL, (int)NULL
};

/*  */

forwcmd (args)
char  **args;
{
    int	    msgp = 0,
            vecp = 1,
            msgnum;
    char   *cp,
           *filter = NULL,
            buf[BUFSIZ],
           *msgs[MAXARGS],
           *vec[MAXARGS];

    if (fmsh) {
	forkcmd (args, cmd_name);
	return;
    }

    while (cp = *args++) {
	if (*cp == '-')
	    switch (smatch (++cp, forwswit)) {
		case AMBIGSW: 
		    ambigsw (cp, forwswit);
		    return;
		case UNKWNSW: 
		    fprintf (stderr, MSGSTR(NOTKNOWN, "-%s unknown\n"), cp); /*MSG*/
		    return;
		case FOHELP: 
		    (void) sprintf (buf, MSGSTR(DIHELP1, "%s [msgs] [switches]"), cmd_name); /*MSG*/
		    help (buf, forwswit);
		    return;

		case FOANSW:	/* not implemented */
		case FONANSW: 
		case FOINSW: 
		case FONINSW: 
		    continue;

		case FONDFSW:
		case FONEDSW:
		case FONWTSW:
		    vec[vecp++] = --cp;
		    continue;

		case FOEDTSW: 
		case FOFRMSW: 
		case FODFSW:
		case FODMSW:
		case FOWHTSW:
		    vec[vecp++] = --cp;
		    if (!(cp = *args++) || *cp == '-') {
			advise (NULLCP, MSGSTR(NOARG, "missing argument to %s"), args[-2]); /*MSG*/
			return;
		    }
		    vec[vecp++] = cp;
		    continue;
		case FOFTRSW: 
		    if (!(filter = *args++) || *filter == '-') {
			advise (NULLCP, MSGSTR(NOARG, "missing argument to %s"), args[-2]); /*MSG*/
			return;
		    }
		    continue;
		case FOFTSW: 
		    if (access (filter = myfilter, 04) == NOTOK) {
			advise (filter, MSGSTR(NOREADFF, "unable to read default filter file %s"), filter); /*MSG*/
			return;
		    }
		    continue;
		case FONFTSW: 
		    filter = NULL;
		    continue;
	    }
	if (*cp == '+' || *cp == '@') {
	    advise (NULLCP, MSGSTR(SORRY, "sorry, no folders allowed!")); /*MSG*/
	    return;
	}
	else
	    msgs[msgp++] = cp;
    }

					/* foil search of .mh_profile */
    (void) sprintf (buf, "%sXXXXXX", invo_name);
    vec[0] = mktemp (buf);
    vec[vecp++] = "-file";
    vec[vecp] = NULL;
    if (!msgp)
	msgs[msgp++] = "cur";
    for (msgnum = 0; msgnum < msgp; msgnum++)
	if (!m_convert (mp, msgs[msgnum]))
	    return;
    m_setseq (mp);

    if (filter) {
	(void) strcpy (buf, filter);
	if (expand (buf) == NOTOK)
	    return;
	if (access (filter = getcpy (libpath (buf)), 04) == NOTOK) {
	    advise (filter, MSGSTR(NOREAD, "unable to read %s"), filter); /*MSG*/
	    free (filter);
	    return;
	}
    }
    forw (cmd_name, filter, vecp, vec);
    m_setcur (mp, mp -> hghsel);
    if (filter)
	free (filter);
}

/*  */

static	forw (char *proc, char *filter, 
	      int  vecp,  char **vec   )
{
    int     i,
            child_id,
            msgnum,
            msgcnt;
    char    tmpfil[80],
           *args[MAXARGS];
    FILE   *out;

    (void) strcpy (tmpfil, m_tmpfil (invo_name));
    interrupted = 0;
    if (filter)
	switch (child_id = fork ()) {
	    case NOTOK: 
		advise ("fork", MSGSTR(NOFORK, "unable to fork")); /*MSG*/
		return;

	    case OK: 		/* "trust me" */
		if (freopen (tmpfil, "w", stdout) == NULL) {
		    fprintf (stderr, MSGSTR(NOCREATE1, "unable to create ")); /*MSG*/
		    perror (tmpfil);
		    _exit (1);
		}
		args[0] = r1bindex (mhlproc, '/');
		i = 1;
		args[i++] = "-forwall";
		args[i++] = "-form";
		args[i++] = filter;
		for (msgnum = mp -> lowsel; msgnum <= mp -> hghsel; msgnum++)
		    if (mp -> msgstats[msgnum] & SELECTED)
			args[i++] = getcpy (m_name (msgnum));
		args[i] = NULL;
		(void) mhlsbr (i, args, mhl_action);
		m_eomsbr ((int (*) ()) 0);
		(void) fclose (stdout);
		_exit (0);

	    default: 
		if (pidXwait (child_id, NULLCP))
		    interrupted++;
		break;
	}
    else {
	if ((out = fopen (tmpfil, "w")) == NULL) {
	    advise (tmpfil, MSGSTR(NOTMPF2, "unable to create temporary file %s"), tmpfil); /*MSG*/
	    return;
	}

	msgcnt = 1;
	for (msgnum = mp -> lowsel;
		msgnum <= mp -> hghsel && !interrupted;
		msgnum++)
	    if (mp -> msgstats[msgnum] & SELECTED) {
		fprintf (out, "\n\n-------");
		if (msgnum == mp -> lowsel) {
		    if (mp -> numsel > 1)
		        fprintf (out, MSGSTR(FORMSGS, " Forwarded Messages")); /*MSG*/
		    else
		        fprintf (out, MSGSTR(FORWMSG, " Forwarded Message")); /*MSG*/
		}
		else
		    fprintf (out, MSGSTR(MESG, " Message %d"), msgcnt); /*MSG*/
		fprintf (out, "\n\n");
		copy_digest (msgnum, out);
		msgcnt++;
	    }

	if (mp -> numsel > 1)
	    fprintf (out, MSGSTR(FORMEND, "\n\n------- End of Forwarded Messages\n")); /*MSG*/
	else
	    fprintf (out, MSGSTR(FORMEND2, "\n\n------- End of Forwarded Message\n")); /*MSG*/
	(void) fclose (out);
    }

    (void) fflush (stdout);
    if (!interrupted)
	switch (child_id = fork ()) {
	    case NOTOK: 
		advise ("fork", MSGSTR(NOFORK, "unable to fork")); /*MSG*/
		break;

	    case OK: 
		closefds (3);

		(void) signal (SIGINT,  (void(*)(int))istat);
		(void) signal (SIGQUIT,  (void(*)(int))qstat);

		vec[vecp++] = tmpfil;
		vec[vecp] = NULL;

		execvp (proc, vec);
		fprintf (stderr, MSGSTR(NOEXEC, "unable to exec ")); /*MSG*/
		perror (proc);
		_exit (1);

	    default: 
		(void) pidXwait (child_id, NULLCP);
		break;
	}

    (void) unlink (tmpfil);
}

/*  */

static char hlpmsg[] =
    "The %s program emulates many of the commands found in the Rand MH\n\
system.  Instead of operating on MH folders, commands to %s concern\n\
a single file.\n\n\
To see the list of commands available, just type a ``?'' followed by\n\
the RETURN key.  To find out what switches each command takes, type\n\
the name of the command followed by ``-help''.  To leave %s, use the\n\
``quit'' command.\n\n\
Although a lot of MH commands are found in %s, not all are fully\n\
implemented.  %s will always recognize all legal switches for a\n\
given command though, and will let you know when you ask for an\n\
option that it is unable to perform.\n\n\
Running %s is fun, but using MH from your shell is far superior.\n\
After you have familiarized yourself with the MH style by using %s,\n\
you should try using MH from the shell.  You can still use %s for\n\
message files that aren't in MH format, such as BBoard files.\n " ;


/* ARGSUSED */

helpcmd (args)
char  **args;
{
    printf (MSGSTR(HLPMSG, hlpmsg), invo_name, invo_name, invo_name, invo_name, invo_name, invo_name, invo_name, invo_name); /*MSG*/
}

/*  */

static struct swit markswit[] = {
#define	MADDSW	0
    "add", 0,
#define	MDELSW	1
    "delete", 0,
#define	MLSTSW	2
    "list", 0,
#define	MSEQSW	3
    "sequence name", 0,
#define	MPUBSW	4
    "public", 0,
#define	MNPUBSW	5
    "nopublic", 0,
#define	MZERSW	6
    "zero", 0,
#define	MNZERSW	7
    "nozero", 0,
#define	MHELP	8
    "help", 4,
#define	MDBUGSW	9
    "debug", -5,

    NULL, (int)NULL
};

/*  */

markcmd (args)
char  **args;
{
    int     addsw = 0,
            deletesw = 0,
            debugsw = 0,
            listsw = 0,
            zerosw = 0,
            seqp = 0,
            msgp = 0,
            i,
            msgnum;
    char   *cp,
            buf[BUFSIZ],
           *seqs[NATTRS + 1],
           *msgs[MAXARGS];

    while (cp = *args++) {
	if (*cp == '-')
	    switch (smatch (++cp, markswit)) {
		case AMBIGSW: 
		    ambigsw (cp, markswit);
		    return;
		case UNKWNSW: 
		    fprintf (stderr, MSGSTR(NOTKNOWN, "-%s unknown\n"), cp); /*MSG*/
		    return;
		case MHELP: 
		    (void) sprintf (buf, MSGSTR(DIHELP1, "%s [msgs] [switches]"), cmd_name); /*MSG*/
		    help (buf, markswit);
		    return;

		case MADDSW: 
		    addsw++;
		    deletesw = listsw = 0;
		    continue;
		case MDELSW: 
		    deletesw++;
		    addsw = listsw = 0;
		    continue;
		case MLSTSW: 
		    listsw++;
		    addsw = deletesw = 0;
		    continue;

		case MSEQSW: 
		    if (!(cp = *args++) || *cp == '-') {
			advise (NULLCP, MSGSTR(NOARG, "missing argument to %s"), args[-2]); /*MSG*/
			return;
		    }
		    if (seqp < NATTRS)
			seqs[seqp++] = cp;
		    else {
			advise (NULLCP, MSGSTR(ONLYSEQS, "only %d sequences allowed!"), NATTRS); /*MSG*/
			return;
		    }
		    continue;

		case MPUBSW: 	/* not implemented */
		case MNPUBSW: 
		    continue;

		case MDBUGSW: 
		    debugsw++;
		    continue;

		case MZERSW: 
		    zerosw++;
		    continue;
		case MNZERSW: 
		    zerosw = 0;
		    continue;
	    }
	if (*cp == '+' || *cp == '@') {
	    advise (NULLCP, MSGSTR(SORRY, "sorry, no folders allowed!")); /*MSG*/
	    return;
	}
	else
	    msgs[msgp++] = cp;
    }

    if (!addsw && !deletesw && !listsw)
	if (seqp)
	    addsw++;
	else
	    if (debugsw)
		listsw++;
	    else {
		seqs[seqp++] = "unseen";
		deletesw++;
		zerosw = 0;
		if (!msgp)
		    msgs[msgp++] = "all";
	    }

    if (!msgp)
	msgs[msgp++] = listsw ? "all" :"cur";
    for (msgnum = 0; msgnum < msgp; msgnum++)
	if (!m_convert (mp, msgs[msgnum]))
	    return;

    if (debugsw) {
	printf ("invo_name=%s mypath=%s defpath=%s\n",
		invo_name, mypath, defpath);
	printf ("ctxpath=%s context flags=%s\n",
		ctxpath, sprintb (buf, (unsigned) ctxflags, DBITS));
	printf ("foldpath=%s flags=%s\n",
		mp -> foldpath,
		sprintb (buf, (unsigned) mp -> msgflags, FBITS));
	printf ("hghmsg=%d lowmsg=%d nummsg=%d curmsg=%d\n",
		mp -> hghmsg, mp -> lowmsg, mp -> nummsg, mp -> curmsg);
	printf ("lowsel=%d hghsel=%d numsel=%d\n",
		mp -> lowsel, mp -> hghsel, mp -> numsel);
#ifndef	MTR
	printf ("lowoff=%d hghoff=%d\n",
		mp -> lowoff, mp -> hghoff);
#else	MTR
	printf ("lowoff=%d hghoff=%d msgbase=0x%x msgstats=0x%x\n",
		mp -> lowoff, mp -> hghoff, mp -> msgbase, mp -> msgstats);
#endif	MTR
    }

    if (seqp == 0 && (addsw || deletesw)) {
	advise (NULLCP, MSGSTR(NEEDONE, "-%s requires at least one -sequence argument"), addsw ? "add" : "delete"); /*MSG*/
	return;
    }
    seqs[seqp] = NULL;

    if (addsw)
	for (seqp = 0; seqs[seqp]; seqp++) {
	    if (zerosw && !m_seqnew (mp, seqs[seqp], 0))
		return;
	    for (msgnum = mp -> lowsel; msgnum <= mp -> hghsel; msgnum++)
		if (mp -> msgstats[msgnum] & SELECTED)
		    if (!m_seqadd (mp, seqs[seqp], msgnum, 0))
			return;
	}

    if (deletesw)
	for (seqp = 0; seqs[seqp]; seqp++) {
	    if (zerosw)
		for (msgnum = mp -> lowmsg; msgnum <= mp -> hghmsg; msgnum++)
		    if (mp -> msgstats[msgnum] & EXISTS)
			if (!m_seqadd (mp, seqs[seqp], msgnum, 0))
			    return;
	    for (msgnum = mp -> lowsel; msgnum <= mp -> hghsel; msgnum++)
		if (mp -> msgstats[msgnum] & SELECTED)
		    if (!m_seqdel (mp, seqs[seqp], msgnum))
			return;
	}

    if (listsw) {
	int     bits = FFATTRSLOT;

	if (seqp == 0)
	    for (i = 0; mp -> msgattrs[i]; i++)
		printf ("%s%s: %s\n", mp -> msgattrs[i],
			mp -> attrstats & (1 << (bits + i))
			? " (private)" : "",
			m_seq (mp, mp -> msgattrs[i]));
	else
	    for (seqp = 0; seqs[seqp]; seqp++)
		printf ("%s%s: %s\n", seqs[seqp], m_seq (mp, seqs[seqp]));

	interrupted = 0;
	if (debugsw)
	    for (msgnum = mp -> lowsel;
		    msgnum <= mp -> hghsel && !interrupted;
		    msgnum++)
		if (mp -> msgstats[msgnum] & SELECTED) {
		    printf ("%*d: id=%d top=%d start=%ld stop=%ld %s\n",
			    DMAXFOLDER, msgnum,
			    Msgs[msgnum].m_bboard_id, Msgs[msgnum].m_top,
			    Msgs[msgnum].m_start, Msgs[msgnum].m_stop,
			    sprintb (buf, (unsigned) mp -> msgstats[msgnum],
				m_seqbits (mp)));
		    if (Msgs[msgnum].m_scanl)
			printf ("%s", Msgs[msgnum].m_scanl);
		}			    
    }
}

/*  */

static struct swit packswit[] = {
#define	PAFISW	0
    "file name", 0,

#define	PAHELP	1
    "help", 4,

    NULL, (int)NULL
};

/*  */

packcmd (args)
char  **args;
{
    int     msgp = 0,
            md,
            msgnum;
    char   *cp,
           *file = NULL,
            buf[BUFSIZ],
           *msgs[MAXARGS];
    char hlds[NL_TEXTMAX];
    struct stat st;

    if (fmsh) {
	forkcmd (args, cmd_name);
	return;
    }

    while (cp = *args++) {
	if (*cp == '-')
	    switch (smatch (++cp, packswit)) {
		case AMBIGSW: 
		    ambigsw (cp, packswit);
		    return;
		case UNKWNSW: 
		    fprintf (stderr, MSGSTR(NOTKNOWN, "-%s unknown\n"), cp); /*MSG*/
		    return;
		case PAHELP: 
		    (void) sprintf (buf, MSGSTR(DIHELP1, "%s [msgs] [switches]"), cmd_name); /*MSG*/
		    help (buf, packswit);
		    return;

		case PAFISW: 
		    if (!(file = *args++) || *file == '-') {
			advise (NULLCP, MSGSTR(NOARG, "missing argument to %s"), args[-2]); /*MSG*/
			return;
		    }
		    continue;
	    }
	if (*cp == '+' || *cp == '@') {
	    advise (NULLCP, MSGSTR(SORRY, "sorry, no folders allowed!")); /*MSG*/
	    return;
	}
	else
	    msgs[msgp++] = cp;
    }

    if (!file)
	file = "./msgbox";
    file = path (file, TFILE);
    if (stat (file, &st) == NOTOK) {
	if (errno != ENOENT) {
	    advise (file, MSGSTR(FILEERR, "error on file %s"), file); /*MSG*/
	    goto done_pack;
	}
	sprintf(hlds, MSGSTR(CREFIL, "Create file \"%s\" <yes or no>? "), file); /*MSG*/
	md = getanswer (hlds);
	if (!md)
	    goto done_pack;
    }

    if (!msgp)
	msgs[msgp++] = "all";
    for (msgnum = 0; msgnum < msgp; msgnum++)
	if (!m_convert (mp, msgs[msgnum]))
	    goto done_pack;
    m_setseq (mp);

    if ((md = mbx_open (file, getuid (), getgid (), m_gmprot ())) == NOTOK) {
	advise (file, MSGSTR(NOOPEN, "unable to open %s"), file); /*MSG*/
	goto done_pack;
    }
    for (msgnum = mp -> lowsel; msgnum <= mp -> hghsel; msgnum++)
	if (mp -> msgstats[msgnum] & SELECTED)
	    if (pack (file, md, msgnum) == NOTOK)
		break;
    (void) mbx_close (file, md);

    if (mp -> hghsel != mp -> curmsg)
	m_setcur (mp, mp -> lowsel);

done_pack: ;
    free (file);
}

/*  */

int	pack (mailbox, md, msgnum)
char   *mailbox;
int     md,
        msgnum;
{
    register FILE *zp;

    if (Msgs[msgnum].m_bboard_id == 0)
	(void) readid (msgnum);

    zp = msh_ready (msgnum, 1);
    return mbx_write (mailbox, md, zp, Msgs[msgnum].m_bboard_id,
	    ftell (zp), Msgs[msgnum].m_stop, 1, 1);
}

/*  */

int	packhak (args)
char  **args;
{
    int	    result;
    char   *cp,
	   *file = NULL;

    while (cp = *args++) {
	if (*cp == '-')
	    switch (smatch (++cp, packswit)) {
		case AMBIGSW: 
		case UNKWNSW: 
		case PAHELP: 
		    return NOTOK;

		case PAFISW: 
		    if (!(file = *args++) || *file == '-') 
			return NOTOK;
		    continue;
	    }
	if (*cp == '+' || *cp == '@')
	    return NOTOK;
    }

    file = path (file ? file : "./msgbox", TFILE);
    result = access (file, 0) == NOTOK ? OK : NOTOK;
    free (file);

    return result;
}

/*  */

static struct swit pickswit[] = {
#define	PIANSW	0
    "and", 0,
#define	PIORSW	1
    "or", 0,
#define	PINTSW	2
    "not", 0,
#define	PILBSW	3
    "lbrace", 0,
#define	PIRBSW	4
    "rbrace", 0,

#define	PICCSW	5
    "cc  pattern", 0,
#define	PIDASW	6
    "date  pattern", 0,
#define	PIFRSW	7
    "from  pattern", 0,
#define	PISESW	8
    "search  pattern", 0,
#define	PISUSW	9
    "subject  pattern", 0,
#define	PITOSW	10
    "to  pattern", 0,
#define	PIOTSW	11
    "-othercomponent  pattern", 15,
#define	PIAFSW	12
    "after date", 0,
#define	PIBFSW	13
    "before date", 0,
#define	PIDFSW	14
    "datefield field", 5,
#define	PISQSW	15
    "sequence name", 0,
#define	PIPUSW	16
    "public", 0,
#define	PINPUSW	17
    "nopublic", 0,
#define	PIZRSW	18
    "zero", 0,
#define	PINZRSW	19
    "nozero", 0,
#define	PILISW	20
    "list", 0,
#define	PINLISW	21
    "nolist", 0,
#define	PIHELP	22
    "help", 4,

    NULL, (int)NULL
};

/*  */

pickcmd (args)
char  **args;
{
    int     zerosw = 1,
            msgp = 0,
            seqp = 0,
            vecp = 0,
            hi,
            lo,
            msgnum;
    char   *cp,
            buf[BUFSIZ],
           *msgs[MAXARGS],
           *seqs[NATTRS],
           *vec[MAXARGS];
    register FILE *zp;

    while (cp = *args++) {
	if (*cp == '-') {
	    if (*++cp == '-') {
		vec[vecp++] = --cp;
		goto pattern;
	    }
	    switch (smatch (cp, pickswit)) {
		case AMBIGSW: 
		    ambigsw (cp, pickswit);
		    return;
		case UNKWNSW: 
		    fprintf (stderr, MSGSTR(NOTKNOWN, "-%s unknown\n"), cp); /*MSG*/
		    return;
		case PIHELP: 
		    (void) sprintf (buf, MSGSTR(DIHELP1, "%s [msgs] [switches]"), cmd_name); /*MSG*/
		    help (buf, pickswit);
		    return;

		case PICCSW: 
		case PIDASW: 
		case PIFRSW: 
		case PISUSW: 
		case PITOSW: 
		case PIDFSW: 
		case PIAFSW: 
		case PIBFSW: 
		case PISESW: 
		    vec[vecp++] = --cp;
pattern: ;
		    if (!(cp = *args++)) {/* allow -xyz arguments */
			advise (NULLCP, MSGSTR(NOARG, "missing argument to %s"), args[-2]); /*MSG*/
			return;
		    }
		    vec[vecp++] = cp;
		    continue;
		case PIOTSW: 
		    advise (NULLCP, MSGSTR(INTERR3, "internal error!")); /*MSG*/
		    return;
		case PIANSW: 
		case PIORSW: 
		case PINTSW: 
		case PILBSW: 
		case PIRBSW: 
		    vec[vecp++] = --cp;
		    continue;

		case PISQSW: 
		    if (!(cp = *args++) || *cp == '-') {
			advise (NULLCP, MSGSTR(NOARG, "missing argument to %s"), args[-2]); /*MSG*/
			return;
		    }
		    if (seqp < NATTRS)
			seqs[seqp++] = cp;
		    else {
			advise (NULLCP, MSGSTR(ONLYSEQS, "only %d sequences allowed!"), NATTRS); /*MSG*/
			return;
		    }
		    continue;
		case PIZRSW: 
		    zerosw++;
		    continue;
		case PINZRSW: 
		    zerosw = 0;
		    continue;

		case PIPUSW: 	/* not implemented */
		case PINPUSW: 
		case PILISW: 
		case PINLISW: 
		    continue;
	    }
	}
	if (*cp == '+' || *cp == '@') {
	    advise (NULLCP, MSGSTR(SORRY, "sorry, no folders allowed!")); /*MSG*/
	    return;
	}
	else
	    msgs[msgp++] = cp;
    }
    vec[vecp] = NULL;

    if (!msgp)
	msgs[msgp++] = "all";
    for (msgnum = 0; msgnum < msgp; msgnum++)
	if (!m_convert (mp, msgs[msgnum]))
	    return;
    m_setseq (mp);

    interrupted = 0;
    if (!pcompile (vec, NULLCP))
	return;

    lo = mp -> lowsel;
    hi = mp -> hghsel;

    for (msgnum = mp -> lowsel;
	    msgnum <= mp -> hghsel && !interrupted;
	    msgnum++)
	if (mp -> msgstats[msgnum] & SELECTED) {
	    zp = msh_ready (msgnum, 1);
	    if (pmatches (zp, msgnum, fmsh ? 0L : Msgs[msgnum].m_start,
			fmsh ? 0L : Msgs[msgnum].m_stop)) {
		if (msgnum < lo)
		    lo = msgnum;
		if (msgnum > hi)
		    hi = msgnum;
	    }
	    else {
		mp -> msgstats[msgnum] &= ~SELECTED;
		mp -> numsel--;
	    }
	}

    if (interrupted)
	return;

    mp -> lowsel = lo;
    mp -> hghsel = hi;

    if (mp -> numsel <= 0) {
	advise (NULLCP, MSGSTR(NOMAT, "no messages match specification")); /*MSG*/
	return;
    }

    seqs[seqp] = NULL;
    for (seqp = 0; seqs[seqp]; seqp++) {
	if (zerosw && !m_seqnew (mp, seqs[seqp], 0))
	    return;
	for (msgnum = mp -> lowsel; msgnum <= mp -> hghsel; msgnum++)
	    if (mp -> msgstats[msgnum] & SELECTED)
		if (!m_seqadd (mp, seqs[seqp], msgnum, 0))
		    return;
    }

    if (mp -> numsel == 1)
        printf (MSGSTR(ONEHIT, "1 hit\n")); /*MSG*/
    else
        printf (MSGSTR(HITS, "%d hits\n"), mp -> numsel); /*MSG*/
}

/*  */

static struct swit replswit[] = {
#define	REANSW	0
    "annotate", 0,
#define	RENANSW	1
    "noannotate", 0,
#define	RECCSW	2
    "cc type", 0,
#define	RENCCSW	3
    "nocc type", 0,
#define	REDFSW	4
    "draftfolder +folder", 0,
#define	REDMSW	5
    "draftmessage msg", 0,
#define	RENDFSW	6
    "nodraftfolder", 0,
#define	REEDTSW	7
    "editor editor", 0,
#define	RENEDSW	8
    "noedit", 0,
#define	REFCCSW	9
    "fcc +folder", 0,
#define	REFLTSW	10
    "filter filterfile", 0,
#define	REFRMSW	11
    "form formfile", 0,
#define	REFRSW	12
    "format", 5,
#define	RENFRSW	13
    "noformat", 7,
#define	REINSW	14
    "inplace", 0,
#define	RENINSW	15
    "noinplace", 0,
#define	REQUSW	16
    "query", 0,
#define	RENQUSW	17
    "noquery", 0,
#define	REWHTSW	18
    "whatnowproc program", 0,
#define	RENWTSW	19
    "nowhatnow", 0,
#define	REWIDSW	20
    "width columns", 0,
#define	REHELP	21
    "help", 4,

    NULL, (int)NULL
};

/*  */

replcmd (args)
char  **args;
{
    int     vecp = 1;
    char   *cp,
           *msg = NULL,
            buf[BUFSIZ],
           *vec[MAXARGS];

    if (fmsh) {
	forkcmd (args, cmd_name);
	return;
    }

    while (cp = *args++) {
	if (*cp == '-')
	    switch (smatch (++cp, replswit)) {
		case AMBIGSW: 
		    ambigsw (cp, replswit);
		    return;
		case UNKWNSW: 
		    fprintf (stderr, MSGSTR(NOTKNOWN, "-%s unknown\n"), cp); /*MSG*/
		    return;
		case REHELP: 
		    (void) sprintf (buf, MSGSTR(DIHELP1, "%s [msgs] [switches]"), cmd_name); /*MSG*/
		    help (buf, replswit);
		    return;

		case REANSW:	/* not implemented */
		case RENANSW: 
		case REINSW: 
		case RENINSW: 
		    continue;

		case REFRSW: 
		case RENFRSW: 
		case REQUSW:
		case RENQUSW:
		case RENDFSW:
		case RENEDSW:
		case RENWTSW:
		    vec[vecp++] = --cp;
		    continue;

		case RECCSW: 
		case RENCCSW: 
		case REEDTSW: 
		case REFCCSW: 
		case REFLTSW:
		case REFRMSW: 
		case REWIDSW: 
		case REDFSW:
		case REDMSW:
		case REWHTSW:
		    vec[vecp++] = --cp;
		    if (!(cp = *args++) || *cp == '-') {
			advise (NULLCP, MSGSTR(NOARG, "missing argument to %s"), args[-2]); /*MSG*/
			return;
		    }
		    vec[vecp++] = cp;
		    continue;
	    }
	if (*cp == '+' || *cp == '@') {
	    advise (NULLCP, MSGSTR(SORRY, "sorry, no folders allowed!")); /*MSG*/
	    return;
	}
	else
	    if (msg) {
		advise (NULLCP, MSGSTR(ONEMSG, "only one message at a time!")); /*MSG*/
		return;
	    }
	    else
		msg = cp;
    }

    vec[0] = cmd_name;
    vec[vecp++] = "-file";
    vec[vecp] = NULL;
    if (!msg)
	msg = "cur";
    if (!m_convert (mp, msg))
	return;
    m_setseq (mp);

    if (mp -> numsel > 1) {
	advise (NULLCP, MSGSTR(ONEMSG, "only one message at a time!")); /*MSG*/
	return;
    }
    (void) process (mp -> hghsel, cmd_name, vecp, vec);
    m_setcur (mp, mp -> hghsel);
}

/*  */

static struct swit rmmswit[] = {
#define	RMHELP	0
    "help", 4,

    NULL, (int)NULL
};

/*  */

rmmcmd (args)
char  **args;
{
    int	    msgp = 0,
            msgnum;
    char   *cp,
            buf[BUFSIZ],
           *msgs[MAXARGS];

    while (cp = *args++) {
	if (*cp == '-')
	    switch (smatch (++cp, rmmswit)) {
		case AMBIGSW: 
		    ambigsw (cp, rmmswit);
		    return;
		case UNKWNSW: 
		    fprintf (stderr, MSGSTR(NOTKNOWN, "-%s unknown\n"), cp); /*MSG*/
		    return;
		case RMHELP: 
		    (void) sprintf (buf, MSGSTR(DIHELP1, "%s [msgs] [switches]"), cmd_name); /*MSG*/
		    help (buf, rmmswit);
		    return;
	    }
	if (*cp == '+' || *cp == '@') {
	    advise (NULLCP, MSGSTR(SORRY, "sorry, no folders allowed!")); /*MSG*/
	    return;
	}
	else
	    msgs[msgp++] = cp;
    }

    if (!msgp)
	msgs[msgp++] = "cur";
    for (msgnum = 0; msgnum < msgp; msgnum++)
	if (!m_convert (mp, msgs[msgnum]))
	    return;
    m_setseq (mp);

    rmm ();
}

/*  */

static  rmm () {
    register int    msgnum,
                    vecp;
    register char  *cp;
    char    buffer[BUFSIZ],
	   *vec[MAXARGS];

    if (fmsh) {
	if (rmmproc) {
	    if (mp -> numsel > MAXARGS - 1) {
		advise (NULLCP, MSGSTR(MANYEMSGS, "more than %d messages for %s exec"), MAXARGS - 1, rmmproc); /*MSG*/
		return;
	    }
	    vecp = 0;
	    for (msgnum = mp -> lowsel; msgnum <= mp -> hghsel; msgnum++)
		if (mp -> msgstats[msgnum] & SELECTED)
		    vec[vecp++] = getcpy (m_name (msgnum));
	    vec[vecp] = NULL;
	    forkcmd (vec, rmmproc);
	    for (vecp = 0; vec[vecp]; vecp++)
		free (vec[vecp]);
	}
	else
	    for (msgnum = mp -> lowsel; msgnum <= mp -> hghsel; msgnum++)
		if (mp -> msgstats[msgnum] & SELECTED) {
		    (void) strcpy (buffer, m_backup (cp = m_name (msgnum)));
		    if (rename (cp, buffer) == NOTOK)
			admonish (buffer, MSGSTR(NORENAME, "unable to rename %s to %s"), cp, buffer); /*MSG*/
		}
    }

    for (msgnum = mp -> lowsel; msgnum <= mp -> hghsel; msgnum++)
	if (mp -> msgstats[msgnum] & SELECTED) {
	    mp -> msgstats[msgnum] |= DELETED;
	    mp -> msgstats[msgnum] &= ~EXISTS;
	}

    if ((mp -> nummsg -= mp -> numsel) <= 0) {
	if (fmsh)
	    admonish (NULLCP, MSGSTR(NONELEFT, "no messages remaining in +%s"), fmsh); /*MSG*/
	else
	    admonish (NULLCP, MSGSTR(NONELEFT2, "no messages remaining in %s"), mp -> foldpath); /*MSG*/
	mp -> lowmsg = mp -> hghmsg = mp -> nummsg = 0;
    }
    if (mp -> lowsel == mp -> lowmsg) {
	for (msgnum = mp -> lowmsg + 1; msgnum <= mp -> hghmsg; msgnum++)
	    if (mp -> msgstats[msgnum] & EXISTS)
		break;
	mp -> lowmsg = msgnum;
    }
    if (mp -> hghsel == mp -> hghmsg) {
	for (msgnum = mp -> hghmsg - 1; msgnum >= mp -> lowmsg; msgnum--)
	    if (mp -> msgstats[msgnum] & EXISTS)
		break;
	mp -> hghmsg = msgnum;
    }

    mp -> msgflags |= MODIFIED;
    modified++;
}

/*  */

static struct swit scanswit[] = {
#define	SCCLR	0
    "clear", 0,
#define	SCNCLR	1
    "noclear", 0,
#define	SCFORM	2
    "form formatfile", 0,
#define	SCFMT	3
    "format string", 5,
#define	SCHEAD	4
    "header", 0,
#define SCNHEAD	5
    "noheader", 0,
#define	SCWID	6
    "width columns", 0,
#define	SCHELP	7
    "help", 4,

    NULL, (int)NULL
};

/*  */

scancmd (args)
char  **args;
{
#define	equiv(a,b)	(a ? b && !strcmp (a, b) : !b)

    int     clearsw = 0,
            headersw = 0,
	    width = 0,
            msgp = 0,
            msgnum,
	    optim,
	    state;
    char   *cp,
	   *form = NULL,
	   *format = NULL,
            buf[BUFSIZ],
	   *nfs,
           *msgs[MAXARGS];
    register FILE *zp;
    static int s_optim = 0;
    static char *s_form = NULL,
		*s_format = NULL;

    while (cp = *args++) {
	if (*cp == '-')
	    switch (smatch (++cp, scanswit)) {
		case AMBIGSW: 
		    ambigsw (cp, scanswit);
		    return;
		case UNKWNSW: 
		    fprintf (stderr, MSGSTR(NOTKNOWN, "-%s unknown\n"), cp); /*MSG*/
		    return;
		case SCHELP: 
		    (void) sprintf (buf, MSGSTR(DIHELP1, "%s [msgs] [switches]"), cmd_name); /*MSG*/
		    help (buf, scanswit);
		    return;

		case SCCLR: 
		    clearsw++;
		    continue;
		case SCNCLR: 
		    clearsw = 0;
		    continue;
		case SCHEAD: 
		    headersw++;
		    continue;
		case SCNHEAD: 
		    headersw = 0;
		    continue;
		case SCFORM: 
		    if (!(form = *args++) || *form == '-') {
			advise (NULLCP, MSGSTR(NOARG, "missing argument to %s"), args[-2]); /*MSG*/
			return;
		    }
		    format = NULL;
		    continue;
		case SCFMT: 
		    if (!(format = *args++) || *format == '-') {
			advise (NULLCP, MSGSTR(NOARG, "missing argument to %s"), args[-2]); /*MSG*/
			return;
		    }
		    form = NULL;
		    continue;
		case SCWID: 
		    if (!(cp = *args++) || *cp == '-') {
			advise (NULLCP, MSGSTR(NOARG, "missing argument to %s"), args[-2]); /*MSG*/
			return;
		    }
		    width = atoi (cp);
		    continue;
	    }
	if (*cp == '+' || *cp == '@') {
	    advise (NULLCP, MSGSTR(SORRY, "sorry, no folders allowed!")); /*MSG*/
	    return;
	}
	else
	    msgs[msgp++] = cp;
    }

    if (!msgp)
	msgs[msgp++] = "all";
    for (msgnum = 0; msgnum < msgp; msgnum++)
	if (!m_convert (mp, msgs[msgnum]))
	    return;
    m_setseq (mp);

    nfs = new_fs (form, format, FORMAT);
    if (scanl) {		/* force scansbr to (re)compile format */
	(void) free (scanl);
	scanl = NULL;
    }

    if (s_optim == 0) {
	s_optim = optim = 1;
	s_form = form ? getcpy (form) : '\0';
	s_format = format ? getcpy (format) : '\0';
    }
    else
	optim = equiv (s_form, form) && equiv (s_format, format);

    interrupted = 0;
    for (msgnum = mp -> lowsel;
	    msgnum <= mp -> hghsel && !interrupted;
	    msgnum++)
	if (mp -> msgstats[msgnum] & SELECTED) {
	    if (optim && Msgs[msgnum].m_scanl)
		printf ("%s", Msgs[msgnum].m_scanl);
	    else {
		zp = msh_ready (msgnum, 0);
		switch (state = scan (zp, msgnum, 0, nfs, width,
			msgnum == mp -> curmsg, headersw,
			fmsh ? 0L : (long) (Msgs[msgnum].m_stop - Msgs[msgnum].m_start),
			1)) {
		    case SCNMSG:
		    case SCNERR:
			if (optim)
			    Msgs[msgnum].m_scanl = getcpy (scanl);
			break;

		    default:
			advise (NULLCP, MSGSTR(SCAN1, "scan() botch (%d)"), state); /*MSG*/
			return;

		    case SCNEOF:
			printf (MSGSTR(EMPTY4, "%*d  empty\n"), DMAXFOLDER, msgnum); /*MSG*/
			break;
		    }
	    }
	    headersw = 0;
	}

    if (clearsw)
	clear_screen ();
}

/*  */

static struct swit showswit[] = {
#define	SHDRAFT	0
    "draft", 5,
#define	SHFORM	1
    "form formfile", 4,
#define	SHPROG	2
    "moreproc program", 4,
#define	SHNPROG	3
    "nomoreproc", 3,
#define	SHLEN	4
    "length lines", 4,
#define	SHWID	5
    "width columns", 4,
#define	SHSHOW	6
    "showproc program", 4,
#define	SHNSHOW	7
    "noshowproc", 3,
#define	SHHEAD	8
    "header", 4,
#define SHNHEAD	9
    "noheader", 3,
#define	SHHELP	10
    "help", 4,

    NULL, (int)NULL
};

/*  */

showcmd (args)
char  **args;
{
    int	    headersw = 1,
            nshow = 0,
            msgp = 0,
            vecp = 1,
            mhl = 0,
            seen = 0,
            mode = 0,
	    i,
            msgnum;
    char   *cp,
           *proc = showproc,
            buf[BUFSIZ],
           *msgs[MAXARGS],
           *vec[MAXARGS];

    if (uleq (cmd_name, "next"))
	mode = 1;
    else
	if (uleq (cmd_name, "prev"))
	    mode = -1;
    while (cp = *args++) {
	if (*cp == '-')
	    switch (i = smatch (++cp, showswit)) {
		case AMBIGSW: 
		    ambigsw (cp, showswit);
		    return;
		case UNKWNSW: 
		case SHNPROG:
		    vec[vecp++] = --cp;
		    continue;
		case SHHELP: 
		    (void) sprintf (buf,
			    MSGSTR(SHHELP1, "%s %s[switches] [switches for showproc]"), cmd_name, mode ? NULL : "[msgs] "); /*MSG*/
		    help (buf, showswit);
		    return;

		case SHFORM: 
		case SHPROG:
		case SHLEN:
		case SHWID:
		    vec[vecp++] = --cp;
		    if (!(cp = *args++) || *cp == '-') {
			advise (NULLCP, MSGSTR(NOARG, "missing argument to %s"), args[-2]); /*MSG*/
			return;
		    }
		    vec[vecp++] = cp;
		    continue;
		case SHHEAD: 
		    headersw++;
		    continue;
		case SHNHEAD: 
		    headersw = 0;
		    continue;
		case SHSHOW: 
		    if (!(proc = *args++) || *proc == '-') {
			advise (NULLCP, MSGSTR(NOARG, "missing argument to %s"), args[-2]); /*MSG*/
			return;
		    }
		    nshow = 0;
		    continue;
		case SHNSHOW: 
		    nshow++;
		    continue;

		case SHDRAFT: 
		    advise (NULLCP, MSGSTR(SORRY1, "sorry, -%s not allowed!"), showswit[i].sw); /*MSG*/
		    return;
	    }
	if (*cp == '+' || *cp == '@') {
	    advise (NULLCP, MSGSTR(SORRY, "sorry, no folders allowed!")); /*MSG*/
	    return;
	}
	else
	    if (mode) {
		fprintf (stderr,
			MSGSTR(USAGE6, "usage: %s [switches] [switches for showproc]\n"), cmd_name); /*MSG*/
		return;
	    }
	    else
		msgs[msgp++] = cp;
    }
    vec[vecp] = NULL;

    if (!msgp)
	msgs[msgp++] = mode > 0 ? "next" : mode < 0 ? "prev" : "cur";
    for (msgnum = 0; msgnum < msgp; msgnum++)
	if (!m_convert (mp, msgs[msgnum]))
	    return;
    m_setseq (mp);

    if (nshow)
	proc = "cat";
    else
	if (strcmp (showproc, "mhl") == 0) {
	    proc = mhlproc;
	    mhl++;
	}

    seen = m_seqflag (mp, "unseen");
    vec[0] = r1bindex (proc, '/');
    if (mhl) {
	msgp = vecp;
	for (msgnum = mp -> lowsel; msgnum <= mp -> hghsel; msgnum++)
	    if (mp -> msgstats[msgnum] & SELECTED) {
		vec[vecp++] = getcpy (m_name (msgnum));
		if (seen)
		    (void) m_seqdel (mp, "unseen", msgnum);
	    }
	vec[vecp] = NULL;
	if (mp -> numsel == 1 && headersw)
	    show (mp -> lowsel);
	(void) mhlsbr (vecp, vec, mhl_action);
	m_eomsbr ((int (*)()) 0);
	while (msgp < vecp)
	    free (vec[msgp++]);
    }
    else {
	interrupted = 0;
	for (msgnum = mp -> lowsel;
		msgnum <= mp -> hghsel && !interrupted;
		msgnum++)
	    if (mp -> msgstats[msgnum] & SELECTED) {
		switch (ask (msgnum)) {
		    case NOTOK: /* QUIT */
			break;

		    case OK: 	/* INTR */
			continue;

		    default:
			if (mp -> numsel == 1 && headersw)
			    show (msgnum);
			if (nshow)
			    copy_message (msgnum, stdout);
			else
			    (void) process (msgnum, proc, vecp, vec);

			if (seen)
			    (void) m_seqdel (mp, "unseen", msgnum);
			continue;
		}
		break;
	    }
    }

    m_setcur (mp, mp -> hghsel);
}

/*  */

static  show (int msgnum)
{
    if (Msgs[msgnum].m_bboard_id == 0)
	(void) readid (msgnum);

    printf (MSGSTR(MESG2, "(Message %d"), msgnum); /*MSG*/
    if (Msgs[msgnum].m_bboard_id > 0)
	printf (", %s: %d", BBoard_ID, Msgs[msgnum].m_bboard_id);
    printf (")\n");
}


/* ARGSUSED */

static	int eom_action (int c)
{
    return (ftell (mhlfp) >= Msgs[mhlnum].m_stop);
}


static	FP mhl_action (char   *name)
{
    int     msgnum;

    if ((msgnum = m_atoi (name)) < mp -> lowmsg
	    || msgnum > mp -> hghmsg
	    || !(mp -> msgstats[msgnum] & EXISTS))
	return NULL;
    mhlnum = msgnum;

    mhlfp = msh_ready (msgnum, 1);
    if (!fmsh)
	m_eomsbr (eom_action);

    return mhlfp;
}


/*  */

static  ask (int msgnum)
{
    char    buf[BUFSIZ];

    if (mp -> numsel == 1 || !interactive || redirected)
	return DONE;

    if (SOprintf (MSGSTR(LIST1, "Press <return> to list \"%d\"..."), msgnum)) { /*MSG*/
	if (mp -> lowsel != msgnum)
	    printf ("\n\n\n");
	printf (MSGSTR(LIST1, "Press <return> to list \"%d\"..."), msgnum); /*MSG*/
    }
    (void) fflush (stdout);
    buf[0] = (char)NULL;
#ifndef	BSD42
    (void) read (fileno (stdout), buf, sizeof buf);
#else	BSD42
    switch (setjmp (sigenv)) {
	case OK: 
	    should_intr = 1;
	    (void) read (fileno (stdout), buf, sizeof buf);/* fall... */

	default: 
	    should_intr = 0;
	    break;
    }
#endif	BSD42
    if (index (buf, '\n') == (char)NULL)
	(void) putchar ('\n');

    if (told_to_quit) {
	told_to_quit = interrupted = 0;
	return NOTOK;
    }
    if (interrupted) {
	interrupted = 0;
	return OK;
    }

    return DONE;
}

/*  */

static struct swit sortswit[] = {
#define	SODATE	0
    "datefield field", 0,
#define	SOVERB	1
    "verbose", 0,
#define	SONVERB	2
    "noverbose", 0,
#define	SOHELP	3
    "help", 4,

    NULL, (int)NULL
};

/*  */

sortcmd (args)
char  **args;
{
    int     msgp = 0,
            msgnum;
    char   *cp,
           *datesw = NULL,
            buf[BUFSIZ],
           *msgs[MAXARGS];
    struct tws  tb,
               *tw;

    if (fmsh) {
	forkcmd (args, cmd_name);
	return;
    }

    while (cp = *args++) {
	if (*cp == '-')
	    switch (smatch (++cp, sortswit)) {
		case AMBIGSW: 
		    ambigsw (cp, sortswit);
		    return;
		case UNKWNSW: 
		    fprintf (stderr, MSGSTR(NOTKNOWN, "-%s unknown\n"), cp); /*MSG*/
		    return;
		case SOHELP: 
		    (void) sprintf (buf, MSGSTR(DIHELP1, "%s [msgs] [switches]"), cmd_name); /*MSG*/
		    help (buf, sortswit);
		    return;

		case SODATE: 
		    if (datesw) {
			advise (NULLCP, MSGSTR(ONEDFLD, "only one date field at a time!")); /*MSG*/
			return;
		    }
		    if (!(datesw = *args++) || *datesw == '-') {
			advise (NULLCP, MSGSTR(NOARG, "missing argument to %s"), args[-2]); /*MSG*/
			return;
		    }
		    continue;

		case SOVERB: 		/* not implemented */
		case SONVERB: 
		    continue;
	    }
	if (*cp == '+' || *cp == '@') {
	    advise (NULLCP, MSGSTR(SORRY, "sorry, no folders allowed!")); /*MSG*/
	    return;
	}
	else
	    msgs[msgp++] = cp;
    }

    if (!msgp)
	msgs[msgp++] = "all";
    if (!datesw)
	datesw = "Date";
    for (msgnum = 0; msgnum < msgp; msgnum++)
	if (!m_convert (mp, msgs[msgnum]))
	    return;
    m_setseq (mp);

    twscopy (&tb, dtwstime ());

    for (msgnum = mp -> lowsel; msgnum <= mp -> hghsel; msgnum++) {
	if (Msgs[msgnum].m_scanl) {
	    free (Msgs[msgnum].m_scanl);
	    Msgs[msgnum].m_scanl = NULL;
	}
	if (mp -> msgstats[msgnum] & SELECTED) {
	    if ((tw = gettws (datesw, msgnum)) == NULL)
		tw = msgnum != mp -> lowsel ? &Msgs[msgnum - 1].m_tb : &tb;
	}
	else
	    tw = &tb;
	twscopy (&Msgs[msgnum].m_tb, tw);
	Msgs[msgnum].m_stats = mp -> msgstats[msgnum];
	if (mp -> curmsg == msgnum)
	    Msgs[msgnum].m_stats |= CUR;
    }

    qsort ((char *) &Msgs[mp -> lowsel], mp -> hghsel - mp -> lowsel + 1,
	    sizeof (struct Msg), msgsort);

    for (msgnum = mp -> lowsel; msgnum <= mp -> hghsel; msgnum++) {
	mp -> msgstats[msgnum] = Msgs[msgnum].m_stats & ~CUR;
	if (Msgs[msgnum].m_stats & CUR)
	    m_setcur (mp, msgnum);
    }
	    
    mp -> msgflags |= MODIFIED;
    modified++;
}

/*  */

static struct tws  *gettws (char  *datesw,
			    int   msgnum)
{
    int	    state;
    char   *bp,
            buf[BUFSIZ],
            name[NAMESZ];
    struct tws *tw;
    register FILE *zp;

    zp = msh_ready (msgnum, 0);
    for (state = FLD;;)
	switch (state = m_getfld (state, name, buf, sizeof buf, zp)) {
	    case FLD: 
	    case FLDEOF: 
	    case FLDPLUS: 
		if (uleq (name, datesw)) {
		    bp = getcpy (buf);
		    while (state == FLDPLUS) {
			state = m_getfld (state, name, buf, sizeof buf, zp);
			bp = add (buf, bp);
		    }
		    if ((tw = dparsetime (bp)) == NULL)
			admonish (NULLCP,
				MSGSTR(NOPARSE, "unable to parse %s field in message %d"), datesw, msgnum); /*MSG*/
		    free (bp);
		    return tw;
		}
		while (state == FLDPLUS)
		    state = m_getfld (state, name, buf, sizeof buf, zp);
		if (state != FLDEOF)
		    continue;

	    case BODY: 
	    case BODYEOF: 
	    case FILEEOF: 
		admonish (NULLCP, MSGSTR(NOFIELD, "no %s field in message %d"), datesw, msgnum); /*MSG*/
		return NULL;

	    case LENERR: 
	    case FMTERR: 
		admonish (NULLCP, MSGSTR(FORMERR, "format error in message %d"), msgnum); /*MSG*/
		return NULL;

	    default: 
		adios (NULLCP, MSGSTR(INTERR2, "internal error -- you lose")); /*MSG*/
	}
}


static int  msgsort (struct Msg *a,
		     struct Msg *b)
{
    return twsort (&a -> m_tb, &b -> m_tb);
}

/*  */

static int  process (int msgnum, char *proc, int vecp, char **vec)
{
    int	    child_id,
	    status;
    char    tmpfil[80];
    FILE   *out;

    if (fmsh) {
	(void) strcpy (tmpfil, m_name (msgnum));
	(void) m_delete (pfolder);
	m_replace (pfolder, fmsh);
	m_sync (mp);
	m_update ();
	goto ready;
    }

    (void) strcpy (tmpfil, m_scratch ("", invo_name));
    if ((out = fopen (tmpfil, "w")) == NULL) {
	int     olderr;
	extern int  errno;
	char    newfil[80];

	olderr = errno;
	(void) strcpy (newfil, m_tmpfil (invo_name));
	if ((out = fopen (newfil, "w")) == NULL) {
	    errno = olderr;
	    advise (tmpfil, MSGSTR(NOTMPF2, "unable to create temporary file %s"), tmpfil); /*MSG*/
	    return NOTOK;
	}
	else
	    (void) strcpy (tmpfil, newfil);
    }
    copy_message (msgnum, out);
    (void) fclose (out);

ready: ;
    (void) fflush (stdout);
    switch (child_id = fork ()) {
	case NOTOK: 
	    advise ("fork", MSGSTR(NOFORK, "unable to fork")); /*MSG*/
	    status = NOTOK;
	    break;
	    
	case OK: 
	    closefds (3);

	    (void) signal (SIGINT,  (void(*)(int))istat);
	    (void) signal (SIGQUIT,  (void(*)(int))qstat);

	    vec[vecp++] = tmpfil;
	    vec[vecp] = NULL;

	    execvp (proc, vec);
	    fprintf (stderr, MSGSTR(NOEXEC, "unable to exec ")); /*MSG*/
	    perror (proc);
	    _exit (1);

	default: 
	    status = pidXwait (child_id, NULLCP);
	    break;
    }

    if (!fmsh)
	(void) unlink (tmpfil);
    return status;
}

/*  */

static  copy_message (int  msgnum,
		      FILE * out)
{
    long    pos;
    static char buffer[BUFSIZ];
    register    FILE * zp;

    zp = msh_ready (msgnum, 1);
    if (fmsh) {
	while (fgets (buffer, sizeof buffer, zp) != NULL) {
	    fputs (buffer, out);
	    if (interrupted && out == stdout)
		break;
	}
    }
    else {
	pos = ftell (zp);
	while (fgets (buffer, sizeof buffer, zp) != NULL
		&& pos < Msgs[msgnum].m_stop) {
	    fputs (buffer, out);
	    pos += (long) strlen (buffer);
	    if (interrupted && out == stdout)
		break;
	}
    }
}


static  copy_digest (int msgnum,
		     FILE * out)
{
    char    c;
    long    pos;
    static char buffer[BUFSIZ];
    register FILE *zp;

    c = '\n';
    zp = msh_ready (msgnum, 1);
    if (!fmsh)
	pos = ftell (zp);
    while (fgets (buffer, sizeof buffer, zp) != NULL
	    && !fmsh && pos < Msgs[msgnum].m_stop) {
	if (c == '\n' && *buffer == '-')
	    (void) fputc (' ', out);
	fputs (buffer, out);
	c = buffer[strlen (buffer) - 1];
	if (!fmsh)
	    pos += (long) strlen (buffer);
	if (interrupted && out == stdout)
	    break;
    }
}
