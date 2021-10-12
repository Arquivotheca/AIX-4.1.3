static char sccsid[] = "@(#)57	1.14  src/bos/usr/bin/mh/uip/post.c, cmdmh, bos411, 9428A410j 11/9/93 09:43:16";
/* 
 * COMPONENT_NAME: CMDMH post.c
 * 
 * FUNCTIONS: MSGSTR, Mpost, anno, annoaux, chkadr, copyfile, die, 
 *            do_a_cipher, do_addresses, do_an_address, do_text, 
 *            err_abrt, fcc, finish_headers, get_header, insert, 
 *            insert_fcc, localmail, make_bcc_file, make_uucp_file, 
 *            netmail, pl, post, postcipher, postplain, putadr, 
 *            putfmt, putgrp, refile_local, sigoff, sigon, sigser, 
 *            start_headers, strcmpX, uptolow, usr_hook, uucpmail, 
 *            verify_all_addresses 
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
/* static char sccsid[] = "post.c	8.1 88/06/10 11:35:19"; */

/* post.c - enter messages into the transport system */

#include "mh.h"
#include "addrsbr.h"
#include "aliasbr.h"
#include "dropsbr.h"
#include "tws.h"
#ifndef	MMDFMTS
#include <ctype.h>
#include <errno.h>
#include <setjmp.h>
#include <stdio.h>
#include <pwd.h>
#include <sys/types.h>
#else	MMDFMTS
#include "mmdf/util.h"
#include "mmdf/mmdf.h"
#endif	MMDFMTS
#include "mts.h"
#ifdef	MHMTS
#ifndef	V7
#include <sys/ioctl.h>
#endif	not V7
#include <sys/stat.h>
#endif	MHMTS
#ifdef	SENDMTS
#include "smail.h"
#undef	MF
#endif	SENDMTS
#include <signal.h>

#include "mh_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 

#ifndef	MMDFMTS
#define	uptolow(c)	(isupper (c) ? tolower (c) : (c))
#endif	not MMDFMTS

#define FCCS		10	/* max number of fccs allowed */

/*  */

static struct swit switches[] = {
#define	ALIASW	0
    "alias aliasfile", 0,

#define	CHKSW	1
    "check", -5,		/* interface from whom */
#define	NCHKSW	2
    "nocheck", -7,		/* interface from whom */

#define	DEBUGSW	3
    "debug", -5,

#define	DISTSW	4
    "dist", -4,			/* interface from dist */

#define	ENCRSW	5
    "encrypt",
#ifndef	TMA
	-7,
#else	TMA
	0,
#endif	TMA
#define	NENCRSW	6
    "noencrypt",
#ifndef	TMA
	-9,
#else	TMA
	0,
#endif	TMA

#define	FILTSW	7
    "filter filterfile", 0,
#define	NFILTSW	8
    "nofilter", 0,

#define	FRMTSW	9
    "format", 0,
#define	NFRMTSW	10
    "noformat", 0,

#define	LIBSW	11		/* interface from send, whom */
    "library directory", -7,

#define	MSGDSW	12
    "msgid", 0,
#define	NMSGDSW	13
    "nomsgid", 0,

#define	VERBSW	14
    "verbose", 0,
#define	NVERBSW	15
    "noverbose", 0,

#define	WATCSW	16
    "watch", 0,
#define	NWATCSW	17
    "nowatch", 0,

#define	WHOMSW	18	/* interface from whom */
    "whom", -4,		

#define	WIDTHSW	19
    "width columns", 0,

#define	HELPSW	20
    "help", 4,

#define	MAILSW	21
    "mail", -4,
#define	SAMLSW	22
    "saml", -4,
#define	SENDSW	23
    "send", -4,
#define	SOMLSW	24
    "soml", -4,

#define	ANNOSW	25		/* interface from send */
    "idanno number", -6,

#define	DLVRSW	26
    "deliver address-list", -7,

#define	CLIESW	27
    "client host", -6,
#define	SERVSW	28
    "server host", -6,
#define	SNOOPSW	29
    "snoop", -5,

#if defined(X400)
#define XPUSHSW 30
    "xpush", -5,
#endif

    NULL, (int)NULL
};

/*  */

struct headers {
    char   *value;

    unsigned int    flags;
#define	HNOP	0x0000		/* just used to keep .set around */
#define	HBAD	0x0001		/* bad header - don't let it through */
#define	HADR	0x0002		/* header has an address field */
#define	HSUB	0x0004		/* Subject: header */
#define	HTRY	0x0008		/* try to send to addrs on header */
#define	HBCC	0x0010		/* don't output this header */
#define	HMNG	0x0020		/* munge this header */
#define	HNGR	0x0040		/* no groups allowed in this header */
#define	HFCC	0x0080		/* FCC: type header */
#define	HNIL	0x0100		/* okay for this header not to have addrs */
#define	HIGN	0x0200		/* ignore this header */
#if defined(X400)
#define HXOR	0x0400		/* this is an x400 IPM originator */
#endif

    unsigned int    set;
#define	MFRM	0x0001		/* we've seen a From: */
#define	MDAT	0x0002		/* we've seen a Date: */
#define	MRFM	0x0004		/* we've seen a Resent-From: */
#define	MVIS	0x0008		/* we've seen sighted addrs */
#define	MINV	0x0010		/* we've seen blind addrs */
};

/*  */

static struct headers  NHeaders[] = {
    "Return-Path", HBAD, (int)NULL,
    "Received", HBAD, (int)NULL,
    "Reply-To", HADR | HNGR, (int)NULL,
    "From", HADR | HNGR | HTRY, MFRM,
    "Sender", HADR | HBAD, (int)NULL,
    "Date", HBAD, (int)NULL,
    "Subject", HSUB, (int)NULL,
    "To", HADR | HTRY, MVIS,
    "cc", HADR | HTRY, MVIS,
    "Bcc", HADR | HTRY | HBCC | HNIL, MINV,
    "Message-ID", HBAD, (int)NULL,
    "Fcc", HFCC, (int)NULL,

    NULL
};

static struct headers  RHeaders[] = {
    "Resent-Reply-To", HADR | HNGR, (int)NULL,
    "Resent-From", HADR | HNGR, MRFM,
    "Resent-Sender", HADR | HBAD, (int)NULL,
    "Resent-Date", HBAD, (int)NULL,
    "Resent-Subject", HSUB, (int)NULL,
    "Resent-To", HADR | HTRY, MVIS,
    "Resent-cc", HADR | HTRY, MVIS,
    "Resent-Bcc", HADR | HTRY | HBCC, MINV,
    "Resent-Message-ID", HBAD, (int)NULL,
    "Resent-Fcc", HFCC, (int)NULL,
    "Reply-To", HADR, (int)NULL,
    "From", HADR | HNGR, MFRM,
#ifdef	MMDFI
    "Sender", HADR | HMNG | HNGR, (int)NULL,
#else	not MMFDI
    "Sender", HADR | HNGR, (int)NULL,
#endif	not MMDFI
    "Date", HNOP, MDAT,
    "To", HADR | HNIL, (int)NULL,
    "cc", HADR | HNIL, (int)NULL,
    "Bcc", HADR | HTRY | HBCC | HNIL, (int)NULL,
    "Fcc", HIGN, (int)NULL,

    NULL
};

/*  */


static short    fccind = 0;	/* index into fccfold[] */
static short    outputlinelen = OUTPUTLINELEN;

static int  pfd = NOTOK;	/* fd to write annotation list to */
static int  myuid;		/* my user id */
static int  mygid;		/* my group id */
static int  recipients = 0;	/* how many people will get a copy */
static int  unkadr = 0;		/* how many of those were unknown */
static int  badadr = 0;		/* number of bad addrs */
static int  badmsg = 0;		/* message has bad semantics */
static int  verbose = 0;	/* spell it out */
static int  format = 1;		/* format addresses */
static int  msgid = 0;		/* add msgid */
static int  debug = 0;		/* debugging post */
static int  watch = 0;		/* watch the delivery process */
static int  whomsw = 0;		/* we are whom not post */
static int  checksw = 0;	/* whom -check */
static int  linepos;		/* putadr()'s position on the line */
static int  nameoutput;		/* putadr() has output header name */

static unsigned msgflags = 0;	/* what we've seen */
#if defined(X400)
static int x400status = 0;	/* if status != 0, don't send mail */
static int xpushflg = 0;	/* true if parent is called with -push */
char *x400fldchk();
#endif

#define	NORMAL 0
#define	RESENT 1
static int msgstate = NORMAL;

static long clock = 0L;		/* the time we started (more or less) */

static void (*hstat) (int), (*istat) (int), (*qstat) (int), (*tstat) (int);

static char tmpfil[BUFSIZ];
static char bccfil[BUFSIZ];

static char from[BUFSIZ];	/* my network address */
#if defined(X400)
static char signature[BUFSIZ];  /* my signature */
static char *orname=(char *)NULL;               /* my x400 O/R name */
static struct passwd *pw;       /* my passwd stuff */
#endif
static char signature[BUFSIZ];	/* my signature */
static char *filter = NULL;	/* the filter for BCC'ing */
static char *subject = NULL;	/* the subject field for BCC'ing */
static char *fccfold[FCCS];	/* foldernames for FCC'ing */

static struct headers  *hdrtab;	/* table for the message we're doing */

static struct mailname localaddrs;	/* local addrs */
static struct mailname netaddrs;	/* network addrs */
static struct mailname uuaddrs;		/* uucp addrs */
static struct mailname tmpaddrs;	/* temporary queue */

/*  */

#ifdef	MMDFMTS
static char *submitmode = "m";	/* deliver to mailbox only */
#ifndef	RP_DOK
static char submitopts[6] = "vl";/* initial options for submit */
#else	RP_DOK
static char submitopts[7] = "vlk";/* initial options for submit */
#endif	RP_DOK
#endif	MMDFMTS

#ifdef	MHMTS
static char *deliver = NULL;

extern char **environ;

void	sigser (int);
#endif	MHMTS

#ifdef	SENDMTS
static int smtpmode = S_MAIL;
static int snoop = 0;
static char *clientsw = NULL;
static char *serversw = NULL;

extern struct smtp  sm_reply;
#endif	SENDMTS

#ifdef	TMA
#define	post(a,b,c) \
    if (encryptsw) postcipher ((a), (b), (c)); else postplain ((a), (b), (c))
#endif	TMA

static int  encryptsw = 0;	/* encrypt it */
char	hlds[NL_TEXTMAX];

static putfmt (register char *, register char *, register FILE *);
static chkadr(), make_bcc_file(), anno(),
       sigoff(), sigon(), pl(), start_headers();
static finish_headers (register FILE *);
static refile_local (register char *);
static int     get_header (register char *, register struct headers *);
static int     putadr (register char *, register char *,
                     register struct mailname *, register FILE *,
                     unsigned int);
static putgrp (register char *, register char *,
             register FILE *, unsigned int);
static int     insert (register struct mailname *);
static int  annoaux (register struct mailname *);
static  insert_fcc (register struct headers *, register char *);
static  verify_all_addresses (int);
static do_an_address (register struct mailname *, int , int);

#ifdef        TMA
static postplain (register char *, int, int);
#else TMA
static post (register char *, int , int );
#endif        TMA

#ifndef       MHMTS
static do_addresses (int bccque, int talk);
static  do_text (register char *, int);
#else MHMTS
static do_addresses (register char *file, int fd, int ud,
                   int bccque, int talk);
#endif        MHMTS
static fcc (register char *, register char *);

long	lseek ();

#if defined(X400)
static char save_from_line[BUFSIZ];	/* text from the From: line */
#define EQUAL		1
#define NOTEQUAL	0
#endif

/*    MAIN */

/* ARGSUSED */

main (argc, argv)
int     argc;
char   *argv[];
{
    int     state,
            compnum;
    char   *cp,
           *msg = NULL,
          **argp = argv + 1,
            buf[BUFSIZ],
            name[NAMESZ];
    FILE   *in,
	   *out;

	setlocale(LC_ALL,"");
	catd = catopen(MF_MH, NL_CAT_LOCALE);

    invo_name = r1bindex (argv[0], '/');
    m_foil (NULLCP);
    mts_init (invo_name);
#ifdef	MMDFMTS
#ifdef	MMDFII
    mmdf_init (invo_name);
#endif	MMDFII
#endif	MMDFMTS

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

		case LIBSW:
		    if (!(cp = *argp++) || *cp == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    m_foil (cp);
		    continue;

		case ALIASW: 
		    if (!(cp = *argp++) || *cp == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
#ifdef	MHMTS
		    if (access (libpath (cp), 04) == NOTOK)
			adios (cp, MSGSTR(NOREAD, "unable to read %s"), cp); /*MSG*/
#endif	MHMTS
		    if ((state = alias (cp)) != AK_OK) {
			strcpy(hlds, MSGSTR(AERROR2, "aliasing error in %s - %s")); /*MSG*/
			adios (NULLCP, hlds, cp, akerror (state));
		    }
		    continue;

		case CHKSW: 
		    checksw++;
		    continue;
		case NCHKSW: 
		    checksw = 0;
		    continue;

		case DEBUGSW: 
		    debug++;
		    continue;

		case DISTSW:
		    msgstate = RESENT;
		    continue;

		case FILTSW:
		    if (!(filter = *argp++) || *filter == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    continue;
		case NFILTSW:
		    filter = NULL;
		    continue;
		
		case FRMTSW: 
		    format++;
		    continue;
		case NFRMTSW: 
		    format = 0;
		    continue;

		case MSGDSW: 
		    msgid++;
		    continue;
		case NMSGDSW: 
		    msgid = 0;
		    continue;

		case VERBSW: 
		    verbose++;
		    continue;
		case NVERBSW: 
		    verbose = 0;
		    continue;

		case WATCSW: 
		    watch++;
		    continue;
		case NWATCSW: 
		    watch = 0;
		    continue;

		case WHOMSW: 
		    whomsw++;
		    continue;

		case WIDTHSW: 
		    if (!(cp = *argp++) || *cp == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    if ((outputlinelen = atoi (cp)) < 10)
			adios (NULLCP, MSGSTR(BADWIDTH, "impossible width %d"), outputlinelen); /*MSG*/
		    continue;

		case ENCRSW:
		    encryptsw++;
		    continue;
		case NENCRSW:
		    encryptsw = 0;
		    continue;

		case ANNOSW: 
		    if (!(cp = *argp++) || *cp == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    if ((pfd = atoi (cp)) <= 2)
			adios (NULLCP, MSGSTR(BADARG, "bad argument %s %s"), argp[-2], cp); /*MSG*/
		    continue;

#ifdef	MMDFMTS
		case MAILSW:
		    submitmode = "m";
		    continue;
		case SOMLSW:	/* for right now, sigh... */
		case SAMLSW:
		    submitmode = "b";
		    continue;
		case SENDSW:
		    submitmode = "y";
		    continue;
#endif	MMDFMTS

#ifndef	MHMTS
		case DLVRSW:
		    if (!(cp = *argp++) || *cp == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    continue;
#else	MHMTS
		case MAILSW:
		case SAMLSW:
		case SOMLSW:
		case SENDSW:
		    continue;
		case DLVRSW: 
		    if (!(deliver = *argp++) || *deliver == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    continue;
#endif	MHMTS

#ifndef	SENDMTS
		case CLIESW:
		case SERVSW:
		    if (!(cp = *argp++) || *cp == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    continue;

		case SNOOPSW:
		    continue;
#else	SENDMTS
		case MAILSW:
		    smtpmode = S_MAIL;
		    continue;
		case SAMLSW:
		    smtpmode = S_SAML;
		    continue;
		case SOMLSW:
		    smtpmode = S_SOML;
		    continue;
		case SENDSW:
		    smtpmode = S_SEND;
		    continue;
		case CLIESW:
		    if (!(clientsw = *argp++) || *clientsw == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    continue;
		case SERVSW:
		    if (!(serversw = *argp++) || *serversw == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    continue;
		case SNOOPSW:
		    snoop++;
		    continue;
#if defined(X400)
		case XPUSHSW:
		    xpushflg++;
		    continue;
#endif
#endif	SENDMTS
	    }
	if (msg)
	    adios (NULLCP, MSGSTR(ONEMSG, "only one message at a time!")); /*MSG*/
	else
	    msg = cp;
    }

    (void) alias (AliasFile);

/*  */

    if (!msg)
	adios (NULLCP, MSGSTR(USAGE3, "usage: %s [switches] file"), invo_name); /*MSG*/

    if (outputlinelen < 10)
	adios (NULLCP, MSGSTR(BADWIDTH, "impossible width %d"), outputlinelen); /*MSG*/

#ifdef	MHMTS
    if (access (msg, 04) == NOTOK)
	adios (msg, MSGSTR(NOREAD, "unable to read %s"), msg); /*MSG*/
#endif	MHMTS
    if ((in = fopen (msg, "r")) == NULL)
	adios (msg, MSGSTR(NOOPEN, "unable to open %s"), msg); /*MSG*/

    start_headers ();
    if (debug) {
	verbose++;
	out = stdout;
#ifdef	MHMTS
	if (deliver) {
	    (void) strcpy (tmpfil, msg);
	    putfmt ("To", deliver, out);
	    goto daemon;
	}
#endif	MHMTS
    }
    else
#ifdef	MHMTS
    if (deliver) {
	if ((out = fopen ("/dev/null", "r")) == NULL)
	    adios ("/dev/null", MSGSTR(NOWRTDN, "unable to write /dev/null")); /*MSG*/
	(void) strcpy (tmpfil, msg);
	putfmt ("To", deliver, out);
	goto daemon;
    }
    else
#endif	MHMTS
	if (whomsw) {
	    if ((out = fopen ("/dev/null", "w")) == NULL)
		adios ("/dev/null", MSGSTR(NOOPNDN, "unable to open /dev/null")); /*MSG*/
	}
	else {
	    (void) strcpy (tmpfil, m_tmpfil (invo_name));
	    if ((out = fopen (tmpfil, "w")) == NULL)
		adios (tmpfil, MSGSTR(NOCREATE, "unable to create %s"), tmpfil); /*MSG*/
#ifdef	MHMTS
	    (void) chown (tmpfil, myuid, mygid);
#endif	MHMTS
	    (void) chmod (tmpfil, 0600);
	}

/*  */

    hdrtab = msgstate == NORMAL ? NHeaders : RHeaders;

    for (compnum = 1, state = FLD;;) {
	switch (state = m_getfld (state, name, buf, sizeof buf, in)) {
	    case FLD: 
	    case FLDEOF: 
	    case FLDPLUS: 
		compnum++;
		cp = add (buf, NULLCP);
		while (state == FLDPLUS) {
		    state = m_getfld (state, name, buf, sizeof buf, in);
		    cp = add (buf, cp);
		}
#if defined(X400)
		if ((cp = x400fldchk(name, cp, xpushflg, &x400status)) == NULLCP) {
		    break;
		}
		cp = getcpy(cp);
#endif
		putfmt (name, cp, out);
		free (cp);
		if (state != FLDEOF)
		    continue;
		finish_headers (out);
		break;

	    case BODY: 
	    case BODYEOF: 
		finish_headers (out);
		if (whomsw)
		    break;
		fprintf (out, "\n%s", buf);
		while (state == BODY) {
		    state = m_getfld (state, name, buf, sizeof buf, in);
		    fputs (buf, out);
		}
		break;

	    case FILEEOF: 
		finish_headers (out);
		break;

	    case LENERR: 
	    case FMTERR: 
		adios (NULLCP, MSGSTR(MFRMTERR, "message format error in component #%d"), compnum); /*MSG*/

	    default: 
		adios (NULLCP, MSGSTR(RET, "getfld() returned %d"), state); /*MSG*/
	}
	break;
    }

/*  */

#ifdef	MHMTS
daemon: ;
#endif	MHMTS
    if (pfd != NOTOK)
	anno ();
    (void) fclose (in);
    if (debug) {
	pl ();
	done (0);
    }
    else
	(void) fclose (out);

#ifdef	TMA
    if (encryptsw)
	tmastart ();
#endif	TMA
    if (whomsw) {
	verify_all_addresses (1);
	done (0);
    }

#ifdef	MMDFMTS
    (void) strcat (submitopts, submitmode);
    if (watch)
	(void) strcat (submitopts, "nw");
#endif	MMDFMTS
#ifdef	MHMTS
    verify_all_addresses (0);
#endif	MHMTS
    if (encryptsw)
	verify_all_addresses (verbose);
    if (msgflags & MINV) {
	make_bcc_file ();
	if (msgflags & MVIS) {
#ifndef	MHMTS
	    if (!encryptsw)
		verify_all_addresses (verbose);
#endif	not MHMTS
	    post (tmpfil, 0, verbose);
	}
	post (bccfil, 1, verbose);
	(void) unlink (bccfil);
    }
    else
	post (tmpfil, 0, isatty (1));
#ifdef	TMA
    if (encryptsw)
	tmastop ();
#endif	TMA

    refile_local (tmpfil);

#ifdef	MHMTS
    if (!deliver)
#endif	MHMTS
	(void) unlink (tmpfil);

    if (verbose)
	printf (MSGSTR(PROCESSD, "Message Processed\n")); /*MSG*/

    done (0);
}

/*    DRAFT GENERATION */
static putfmt (register char *name,
             register char *str,
             register FILE *out)
{
    int     count,
            grp,
            i,
            keep;
    register char  *cp,
                   *pp,
                   *hold,
                   *qp;
    char    namep[BUFSIZ];
    register struct mailname   *mp,
                               *np;
    register struct headers *hdr;


    while (*str == ' ' || *str == '\t')
	str++;

    if (msgstate == NORMAL && uprf (name, "resent")) {
	advise (NULLCP, MSGSTR(ILLLINE, "illegal header line -- %s:"), name); /*MSG*/
	badmsg++;
	return;
    }

    if ((i = get_header (name, hdrtab)) == NOTOK) {
	fprintf (out, "%s: %s", name, str);
	return;
    }

    hdr = &hdrtab[i];
    if (hdr -> flags & HIGN)
	return;
    if (hdr -> flags & HBAD) {
	advise (NULLCP, MSGSTR(ILLLINE, "illegal header line -- %s:"), name); /*MSG*/
	badmsg++;
	return;
    }
    msgflags |= (hdr -> set & ~(MVIS | MINV));

#if defined(X400)
    if (msgflags & MFRM)
        strcpy(save_from_line,str);/* used to compare to x400 O/R name later */
#endif

    if (hdr -> flags & HSUB)
	subject = subject ? add (str, add ("\t", subject)) : getcpy (str);
    if (hdr -> flags & HFCC) {
	if (cp = (char *)rindex (str, '\n'))
	    *cp = (char)NULL;
	for (cp = pp = str; cp = (char *)index (pp, ','); pp = cp) {
	    *cp++ = (char)NULL;
	    insert_fcc (hdr, pp);
	}
	insert_fcc (hdr, pp);
	return;
    }

/*  */

    if (!(hdr -> flags & HADR)) {
	fprintf (out, "%s: %s", name, str);
	return;
    }

    tmpaddrs.m_next = NULL;
    for (count = 0; cp = getname (str); count++)
	if (mp = getm (cp, NULLCP, 0, AD_HOST, NULLCP)) {
	    if (tmpaddrs.m_next)
		np -> m_next = mp;
	    else
		tmpaddrs.m_next = mp;
	    np = mp;
	}
	else
	    if (hdr -> flags & HTRY)
		badadr++;
	    else
		badmsg++;

    if (count < 1) {
	if (hdr -> flags & HNIL)
	    fprintf (out, "%s: %s", name, str);
	else {
#ifdef	notdef
	    advise (NULLCP, MSGSTR(NEEDADD, "%s: field requires at least one address"), name); /*MSG*/
	    badmsg++;
#endif	notdef
	}
	return;
    }

/*  */

    nameoutput = linepos = 0;
    (void) sprintf (namep, "%s%s",
	    (hdr -> flags & HMNG) ? MSGSTR(ORIG, "Original-") : "", name); /*MSG*/

    for (grp = 0, mp = tmpaddrs.m_next; mp; mp = np)
	if (mp -> m_nohost) {	/* also used to test (hdr -> flags & HTRY) */
/*  When BERK is in effect, the only way nohost is set to true is for 
*   addresses. We have added an @ LocalName to these. We will strip it
*   for alias check.
*/
	    hold = getcpy(mp -> m_mbox);
	    pp = (char *)strrchr(hold,'@'); 
	    if (pp) *pp = '\0';	/* if no @, ignore - P0007179 */
	    pp = akvalue (hold);
	    qp = akvisible () ? mp -> m_mbox : "";
	    np = mp;
	    if (np -> m_gname)
		putgrp (namep, np -> m_gname, out, hdr -> flags);
	    while (cp = getname (pp)) {
		if (!(mp = getm (cp, NULLCP, 0, AD_HOST, NULLCP))) {
		    badadr++;
		    continue;
		}
		if (hdr -> flags & HBCC)
		    mp -> m_bcc++;
		if (mp -> m_ingrp = np -> m_ingrp)
		    grp++;
#ifdef	MHMTS
		mp -> m_aka = getcpy (np -> m_mbox);
#endif	MHMTS
		if (putadr (namep, qp, mp, out, hdr -> flags))
		    msgflags |= (hdr -> set & (MVIS | MINV));
		else
		    mnfree (mp);
	    }
	    mp = np;
	    np = np -> m_next;
	    mnfree (mp);
	}
	else {
	    if (hdr -> flags & HBCC)
		mp -> m_bcc++;
	    if (mp -> m_gname)
		putgrp (namep, mp -> m_gname, out, hdr -> flags);
	    if (mp -> m_ingrp)
		grp++;
	    keep = putadr (namep, "", mp, out, hdr -> flags);
	    np = mp -> m_next;
	    if (keep) {
		mp -> m_next = NULL;
		msgflags |= (hdr -> set & (MVIS | MINV));
	    }
	    else
		mnfree (mp);
	}

    if (grp > 0 && (hdr -> flags & HNGR)) {
	advise (NULLCP, MSGSTR(NOGRPS, "%s: field does not allow groups"), name); /*MSG*/
	badmsg++;
    }
    if (linepos)
	(void) putc ('\n', out);
}

/*  */

static  start_headers () {
    register char  *cp;
#if defined(X400)
    char    *save_defpath,
            gecosbuf[BUFSIZ]={' ','(','\0'};
#endif
    char    myhost[BUFSIZ],
            sigbuf[BUFSIZ];
    register struct mailname   *mp;

    myuid = getuid ();
    mygid = getgid ();
    (void) time (&clock);

#if defined(X400)
    save_defpath=defpath; /*search for .mh_profile has been foiled */
    defpath=(char *)NULL; /* allow search for .mh_profile */
    /* find user's X400 O/R name in .mh_profile if it exists */
    if (orname=m_find("orname")) {
        msgflags |= HXOR;
        if ((pw = getpwuid(getuid())) && *(pw->pw_gecos)) {
            strcat(gecosbuf,pw->pw_gecos);
            strcat(gecosbuf,")");
            strcat(orname,gecosbuf);
        }
    }
    defpath=save_defpath; /* foil search for .mh_profile again */
#endif

    (void) strcpy (from, adrsprintf (NULLCP, NULLCP));

    (void) strcpy (myhost, LocalName ());
#if  !defined(X400)	/* Don't convert address to lower case */
    for (cp = myhost; *cp; cp++)
	*cp = uptolow (*cp);
#endif

#ifdef	MHMTS
    if (deliver) {
	if (geteuid () == 0 && myuid != 0 && myuid != 1 && mygid != 1)
	    adios (NULLCP, MSGSTR(DUNKWN, "-deliver unknown")); /*MSG*/
#if defined(X400)
        (void) strcpy (signature, (msgflags & HXOR) ? orname : from);
#else
        (void) strcpy (signature, from);
#endif
    }
#endif	MHMTS

    if ((cp = getfullname ()) && *cp) {
	(void) strcpy (sigbuf, cp);
	(void) sprintf (signature, "%s <%s>", sigbuf,
#if defined(X400)
            (msgflags & HXOR) ? orname : adrsprintf (NULLCP, NULLCP));
#else
            adrsprintf (NULLCP, NULLCP));
#endif

#if defined(X400)
        if (!(*orname)) {
                if ((cp = getname (signature)) == NULL)
                    adios (NULLCP, MSGSTR(LOSE, "getname () failed -- you lose extraordinarily big")); /*MSG*/
                if ((mp = getm (cp, NULLCP, 0, AD_HOST, NULLCP)) == NULL)
                    adios (NULLCP, MSGSTR(BADSIGN, "bad signature '%s'"), sigbuf); /*MSG*/
                mnfree (mp);
                while (getname (""))
                    continue;
        }
#else
        if ((cp = getname (signature)) == NULL)
            adios (NULLCP, MSGSTR(LOSE, "getname () failed -- you lose extraordinarily big")); /*MSG*/
        if ((mp = getm (cp, NULLCP, 0, AD_HOST, NULLCP)) == NULL)
            adios (NULLCP, MSGSTR(BADSIGN, "bad signature '%s'"), sigbuf); /*MSG*/
        mnfree (mp);
        while (getname (""))
            continue;
#endif

    }
    else
#if defined(X400)
        (void) strcpy (signature,(msgflags & HXOR) ? orname : adrsprintf (NULLCP, NULLCP));
#else
        (void) strcpy (signature, adrsprintf (NULLCP, NULLCP));
#endif
}

/*  */

static finish_headers (register FILE *out)
{

    switch (msgstate) {
	case NORMAL: 
	    if (whomsw)
		break;

	    fprintf (out, "Date: %s\n", dtime (&clock));
	    if (msgid)
		fprintf (out, "Message-ID: <%d.%ld@%s>\n", getpid (), clock, LocalName ());
            if (msgflags & MFRM) {
#if defined(X400)
                if (!(msgflags & HXOR))
                    fprintf (out, "Sender: %s\n", from);
                else
                    if (strcmpX(save_from_line , orname) == NOTEQUAL)
                        fprintf(out, "Sender: %s\n", orname);
#else
                fprintf (out, "Sender: %s\n", from);
#endif
            }
            else
                fprintf (out, "From: %s\n", signature);
	    if (!(msgflags & MVIS))
		fprintf (out, "Bcc: Blind Distribution List: ;\n");
	    break;

	case RESENT: 
	    if (!(msgflags & MDAT)) {
		advise (NULLCP, MSGSTR(NODATE, "message has no Date: header")); /*MSG*/
		badmsg++;
	    }
	    if (!(msgflags & MFRM)) {
		advise (NULLCP, MSGSTR(NOFROM, "message has no From: header")); /*MSG*/
		badmsg++;
	    }
	    if (whomsw)
		break;

#ifdef	MMDFI			/* sigh */
	    fprintf (out, "Sender: %s\n", from);
#endif	MMDFI

	    fprintf (out, "Resent-Date: %s\n", dtime (&clock));
	    if (msgid)
		fprintf (out, "Resent-Message-ID: <%d.%ld@%s>\n", getpid (), clock, LocalName ());
            if (msgflags & MRFM) {
#if defined(X400)
                if (!(msgflags & HXOR))
                    fprintf (out, "Resent-Sender: %s\n", from);
                else
                    if (strcmpX(save_from_line , orname) == NOTEQUAL)
                        fprintf(out, "Resent-Sender: %s\n", orname);
#else
                fprintf (out, "Resent-Sender: %s\n", from);
#endif
            }
            else
                fprintf (out, "Resent-From: %s\n", signature);
	    if (!(msgflags & MVIS))
		fprintf (out, "Resent-Bcc: Blind Re-Distribution List: ;\n");
	    break;
    }

    if (badmsg)
	adios (NULLCP, MSGSTR(REFORMAT, "re-format message and try again")); /*MSG*/
    if (!recipients)
	adios (NULLCP, MSGSTR(NOADDRS, "no addressees")); /*MSG*/
}

/*  */

static int     get_header (register char   *header,
			register struct headers *table)
{
    register struct headers *h;

    for (h = table; h -> value; h++)
	if (uleq (header, h -> value))
	    return (h - table);

    return NOTOK;
}

/*  */
static int     putadr (register char *name,
                     register char *aka,
                     register struct mailname *mp,
                     register FILE *out,
                     unsigned int flags)
{
    int     len;
    register char   *cp;
    char    buffer[BUFSIZ];

    if (mp -> m_mbox == NULL || ((flags & HTRY) && !insert (mp)))
	return 0;
    if ((flags & HBCC) || mp -> m_ingrp)
	return 1;

    if (!nameoutput) {
	fprintf (out, "%s: ", name);
	linepos += (nameoutput = strlen (name) + 2);
    }

    if (*aka && mp -> m_type != UUCPHOST && !mp -> m_pers)
	mp -> m_pers = getcpy (aka);
    if (format) {
	if (mp -> m_gname)
	    (void) sprintf (cp = buffer, "%s;", mp -> m_gname);
	else
	    cp = adrformat (mp);
    }
    else
	cp = mp -> m_text;
    len = strlen (cp);

    if (linepos != nameoutput)
	if (len + linepos + 2 > outputlinelen)
	    fprintf (out, ",\n%*s", linepos = nameoutput, "");
	else {
	    fputs (", ", out);
	    linepos += 2;
	}

    fputs (cp, out);
    linepos += len;

    return (flags & HTRY);
}

/*  */

static putgrp (register char *name,
             register char *group,
             register FILE *out,
             unsigned int flags)
{
    int     len;
    char   *cp;

    if (flags & HBCC)
	return;

    if (!nameoutput) {
	fprintf (out, "%s: ", name);
	linepos += (nameoutput = strlen (name) + 2);
    }

    cp = concat (group, ";", NULLCP);
    len = strlen (cp);

    if (linepos != nameoutput)
	if (len + linepos + 2 > outputlinelen) {
	    fprintf (out, ",\n%*s", nameoutput, "");
	    linepos = nameoutput;
	}
	else {
	    fputs (", ", out);
	    linepos += 2;
	}

    fputs (cp, out);
    linepos += len;
}

/*  */

static int     insert (register struct mailname *np)
{
    register struct mailname   *mp;

    if (np -> m_mbox == NULL)
	return 0;

    for (mp = np -> m_type == LOCALHOST ? &localaddrs
	    : np -> m_type == UUCPHOST ? &uuaddrs
	    : &netaddrs;
	    mp -> m_next;
	    mp = mp -> m_next)
	if (uleq (np -> m_host, mp -> m_next -> m_host)
		&& uleq (np -> m_mbox, mp -> m_next -> m_mbox)
		&& np -> m_bcc == mp -> m_next -> m_bcc)
	    return 0;

    mp -> m_next = np;
    recipients++;
    return 1;
}


static  pl () {
    register int     i;
    register struct mailname *mp;

    printf (MSGSTR(ADDRSFORM, "-------\n\t-- Addresses --\nlocal:\t")); /*MSG*/
    for (mp = localaddrs.m_next; mp; mp = mp -> m_next)
	printf ("%s%s%s", mp -> m_mbox,
		mp -> m_bcc ? "[BCC]" : "",
		mp -> m_next ? ",\n\t" : "");

    printf ("\nnet:\t");
    for (mp = netaddrs.m_next; mp; mp = mp -> m_next)
	printf ("%s%s@%s%s%s", mp -> m_path ? mp -> m_path : "",
		mp -> m_mbox, mp -> m_host,
		mp -> m_bcc ? "[BCC]" : "",
		mp -> m_next ? ",\n\t" : "");

    printf ("\nuucp:\t");
    for (mp = uuaddrs.m_next; mp; mp = mp -> m_next)
	printf ("%s!%s%s", mp -> m_host, mp -> m_mbox,
		mp -> m_bcc ? "[BCC]" : "",
		mp -> m_next ? ",\n\t" : "");

    printf (MSGSTR(COPFORM, "\n\t-- Folder Copies --\nfcc:\t")); /*MSG*/
    for (i = 0; i < fccind; i++)
	printf ("%s%s", fccfold[i], i + 1 < fccind ? ",\n\t" : "");
    printf ("\n");
}

/*  */

static  anno () {
    register struct mailname *mp;

    for (mp = localaddrs.m_next; mp; mp = mp -> m_next)
	if (annoaux (mp) == NOTOK)
	    goto oops;

    for (mp = netaddrs.m_next; mp; mp = mp -> m_next)
	if (annoaux (mp) == NOTOK)
	    goto oops;

    for (mp = uuaddrs.m_next; mp; mp = mp -> m_next)
	if (annoaux (mp) == NOTOK)
	    break;

oops: ;
    (void) close (pfd);
    pfd = NOTOK;
}


static int  annoaux (register struct mailname *mp)
{
    int     i;
    char    buffer[BUFSIZ];

    (void) sprintf (buffer, "%s\n", adrformat (mp));
    i = strlen (buffer);

    return (write (pfd, buffer, i) == i ? OK : NOTOK);
}

/*  */

static  insert_fcc (register struct headers *hdr,
                  register char *pp)
{
    register char   *cp;

    for (cp = pp; isspace (*cp); cp++)
	continue;
    for (pp += strlen (pp) - 1; pp > cp && isspace (*pp); pp--)
	continue;
    if (pp >= cp)
	*++pp = (char)NULL;
    if (*cp == (char)NULL)
	return;

    if (fccind >= FCCS)
	adios (NULLCP, MSGSTR(TOOMANY, "too many %ss"), hdr -> value); /*MSG*/
    fccfold[fccind++] = getcpy (cp);
}

/*    BCC GENERATION */

static  make_bcc_file () {
    int     fd,
	    i,
            child_id;
    char   *vec[6];
    register FILE   *out;

    (void) strcpy (bccfil, m_tmpfil ("bccs"));
    if ((out = fopen (bccfil, "w")) == NULL)
	adios (bccfil, MSGSTR(NOCREATE, "unable to create %s"), bccfil); /*MSG*/
    (void) chmod (bccfil, 0600);

    fprintf (out, "Date: %s\n", dtime (&clock));
    if (msgid)
	fprintf (out, "Message-ID: <%d.%ld@%s>\n", 
		getpid (), clock, LocalName ());
    fprintf (out, "From: %s\n", signature);
    if (subject)
	fprintf (out, "Subject: %s", subject);
    fprintf (out, "BCC:\n\n------- Blind-Carbon-Copy\n\n");
    (void) fflush (out);

    if (filter == NULL) {
	if ((fd = open (tmpfil, 0)) == NOTOK)
	    adios (NULLCP, MSGSTR(NOREOPEN2, "unable to re-open")); /*MSG*/
	cpydgst (fd, fileno (out), tmpfil, bccfil);
	(void) close (fd);
    }
    else {
	vec[0] = r1bindex (mhlproc, '/');

	for (i = 0; (child_id = fork ()) == NOTOK && i < 5; i++)
	    sleep (5);
	switch (child_id) {
	    case NOTOK: 
		adios ("fork", MSGSTR(NOFORK, "unable to fork")); /*MSG*/

	    case OK: 
		(void) dup2 (fileno (out), 1);

		i = 1;
		vec[i++] = "-forward";
		vec[i++] = "-form";
		vec[i++] = filter;
		vec[i++] = tmpfil;
		vec[i] = NULL;

		execvp (mhlproc, vec);
		fprintf (stderr, MSGSTR(NOEXEC, "unable to exec ")); /*MSG*/
		perror (mhlproc);
		_exit (-1);

	    default: 
		(void) pidXwait (child_id, mhlproc);
		break;
	}
    }

    (void) fseek (out, 0L, 2);
    fprintf (out, "\n------- End of Blind-Carbon-Copy\n");
    (void) fclose (out);
}

/*    ADDRESS VERIFICATION */

static  verify_all_addresses (int talk)
{
#ifndef	MHMTS
    int     retval;
#endif	not MHMTS
#ifdef	MMDFMTS
#ifdef	RP_NS
    int	    len;
    struct rp_bufstruct reply;
#endif	RP_NS
#endif	MMDFMTS
    register struct mailname *lp;

#ifndef	MHMTS
    sigon ();
#endif	not MHMTS

#ifdef	MMDFMTS
    if (!whomsw || checksw) {
	if (rp_isbad (retval = mm_init ())
		|| rp_isbad (retval = mm_sbinit ())
		|| rp_isbad (retval = mm_winit (NULLCP, submitopts, from)))
	    die (NULLCP, MSGSTR(MMDFPROB, "problem initializing MMDF system [%s]"), rp_valstr (retval)); /*MSG*/
#ifdef	RP_NS
	if (rp_isbad (retval = mm_rrply (&reply, &len)))
	    die (NULLCP, MSGSTR(SADDRPROB, "problem with sender address [%s]"), rp_valstr (retval)); /*MSG*/
#endif	RP_NS
    }
#endif	MMDFMTS
#ifdef	SENDMTS
    if (!whomsw || checksw)
	if (rp_isbad (retval = sm_init (clientsw, serversw, 0, 0, snoop))
		|| rp_isbad (retval = sm_winit (smtpmode, from)))
	    die (NULLCP, MSGSTR(SERVPROB, "problem initializing server; %s"), rp_string (retval)); /*MSG*/
#endif	SENDMTS

    if (talk && !whomsw)
	printf (MSGSTR(ADDRVER, " -- Address Verification --\n")); /*MSG*/
#ifndef	BERK
    if (talk && localaddrs.m_next)
	printf (MSGSTR(LRECIPS, "  -- Local Recipients --\n")); /*MSG*/
#endif	BERK
    for (lp = localaddrs.m_next; lp; lp = lp -> m_next)
	do_an_address (lp, talk, encryptsw);

#ifndef	BERK
    if (talk && uuaddrs.m_next)
	printf (MSGSTR(URECIPS, "  -- UUCP Recipients --\n")); /*MSG*/
#endif	BERK
    for (lp = uuaddrs.m_next; lp; lp = lp -> m_next)
	do_an_address (lp, talk, encryptsw);

#ifndef	BERK
    if (talk && netaddrs.m_next)
	printf (MSGSTR(NRECIPS, "  -- Network Recipients --\n")); /*MSG*/
#endif	BERK
    for (lp = netaddrs.m_next; lp; lp = lp -> m_next)
	do_an_address (lp, talk, encryptsw);

    chkadr ();
    if (talk && !whomsw)
	printf (MSGSTR(ADDRVEROK, " -- Address Verification Successful --\n")); /*MSG*/

#ifdef	MMDFMTS
    if (!whomsw || checksw)
	(void) mm_end (NOTOK);
#endif	MMDFMTS
#ifdef	SENDMTS
    if (!whomsw || checksw)
	(void) sm_end (DONE);
#endif	SENDMTS
    (void) fflush (stdout);

#ifndef	MHMTS
    sigoff ();
#endif	not MHMTS
}

/*  */

static  chkadr () {
    if (badadr && unkadr)
	if (badadr == 1)
	    (unkadr == 1) ?
		die (NULLCP, MSGSTR(POST_MSG1,
			"1 address unparsable, 1 addressee undeliverable")) :
		die (NULLCP, MSGSTR(POST_MSG2,
			"1 address unparsable, %d addressees undeliverable"),
			unkadr);
	else
	    (unkadr == 1) ?
		die (NULLCP, MSGSTR(POST_MSG3,
			"%d addresses unparsable, 1 addressee undeliverable"),
			badadr) :
		die (NULLCP, MSGSTR(POST_MSG4,
			"%d addresses unparsable, %d addressees undeliverable"),
			badadr, unkadr);

    if (badadr)
	(badadr == 1) ?
	    die (NULLCP, MSGSTR(POST_MSG5,"1 address unparsable")) :
	    die (NULLCP, MSGSTR(POST_MSG6,"%d addresses unparsable"), badadr);

    if (unkadr)
	(unkadr == 1) ?
	    die (NULLCP, MSGSTR(POST_MSG7,"1 addressee undeliverable")) :
	    die (NULLCP, MSGSTR(POST_MSG8,"%d addressees undeliverable"),
		unkadr);
}

/*    MTS INTERACTION */

#ifdef	TMA
static postplain (register char *file, int bccque, int tal)
#else	TMA
static post (register char *file, int bccque, int talk)
#endif	TMA
{
    int     fd;
#ifndef	MHMTS
    int	    retval;
#ifdef	MMDFMTS
#ifdef	RP_NS
    int	    len;
    struct rp_bufstruct reply;
#endif	RP_NS
#endif	MMDFMTS
#else	MHMTS
    int	    ud;
#endif	MHMTS

#if defined(X400)
    if (x400status != 0) {
	sleep(5);
	done(67);
    }
#endif
    if (verbose)
	if (msgflags & MINV) {
	    strcpy (hlds, MSGSTR(POST, " -- Posting for %s Recipients --\n")); /*MSG*/
	    printf (hlds, bccque ? MSGSTR(BLIND, "Blind") : MSGSTR(SIGHTED, "Sighted")); /*MSG*/
	}
	else
	    printf (MSGSTR(POSTALL, " -- Posting for All Recipients --\n")); /*MSG*/

    sigon ();

#ifdef	MMDFMTS
    if (rp_isbad (retval = mm_init ())
	    || rp_isbad (retval = mm_sbinit ())
	    || rp_isbad (retval = mm_winit (NULLCP, submitopts, from)))
	die (NULLCP, MSGSTR(MMDFPROB, "problem initializing MMDF system [%s]"), rp_valstr (retval)); /*MSG*/
#ifdef	RP_NS
	if (rp_isbad (retval = mm_rrply (&reply, &len)))
	    die (NULLCP, MSGSTR(SADDRPROB, "problem with sender address [%s]"), rp_valstr (retval)); /*MSG*/
#endif	RP_NS
#endif	MMDFMTS
#ifdef	SENDMTS
    if (rp_isbad (retval = sm_init (clientsw, serversw, watch, verbose, snoop))
	    || rp_isbad (retval = sm_winit (smtpmode, from)))
	die (NULLCP, MSGSTR(SERVPROB, "problem initializing server; %s"), rp_string (retval)); /*MSG*/
#endif	SENDMTS

#ifndef	MHMTS
    do_addresses (bccque, talk && verbose);
    if ((fd = open (file, 0)) == NOTOK)
	die (file, MSGSTR(NOROPEN, "unable to re-open %s"), file); /*MSG*/
    do_text (file, fd);
#else	MHMTS
    if ((fd = open (file, 0)) == NULL)
	adios (file, MSGSTR(NOROPEN, "unable to re-open %s"), file); /*MSG*/
#ifdef	MF
    ud = UucpChan () && uuaddrs.m_next ? make_uucp_file (fd) : NOTOK;
#else	not MF
    ud = NOTOK;
#endif	not MF
    do_addresses (file, fd, ud, bccque, talk && verbose);
    if (ud != NOTOK)
	(void) close (ud);
#endif	MHMTS
    (void) close (fd);
    (void) fflush (stdout);

#ifdef	MMDFMTS
    (void) mm_sbend ();
    (void) mm_end (OK);
#endif	MMDFMTS
#ifdef	SENDMTS
    (void) sm_end (!(msgflags & MINV) || bccque ? OK : DONE);
#endif	SENDMTS

    sigoff ();

    if (verbose)
	if (msgflags & MINV) {
	    strcpy (hlds, MSGSTR(COPIES, " -- %s Recipient Copies Posted --\n")); /*MSG*/
	    printf (hlds, bccque ? MSGSTR(BLIND, "Blind") : MSGSTR(SIGHTED, "Sighted")); /*MSG*/
	}
	else
	    printf (MSGSTR(COPIES2, " -- Recipient Copies Posted --\n")); /*MSG*/
    (void) fflush (stdout);
}

/*  */

#ifdef	TMA
static postcipher (file, bccque, talk)
register char   *file;
int     bccque,
        talk;
{
    int     fdP,
            state;
    char    reason[BUFSIZ];
    struct mailname *lp;

    if (verbose)
	if (msgflags & MINV) {
	    strcpy (hlds, MSGSTR(POST, " -- Posting for %s Recipients --\n")); /*MSG*/
	    printf (hlds, bccque ? MSGSTR(BLIND, "Blind") : MSGSTR(SIGHTED, "Sighted"));  /*MSG*/
	}
	else
	    printf (MSGSTR(POSTALL, " -- Posting for All Recipients --\n")); /*MSG*/

    if ((fdP = open (file, 0)) == NOTOK)
	adios (file, MSGSTR(NOROPEN, "unable to re-open %s"), file); /*MSG*/
    if (ciphinit (fdP, reason) == NOTOK)
	adios (NULLCP, "%s", reason);
    (void) close (fdP);

    for (state = 0, lp = localaddrs.m_next; lp; lp = lp -> m_next)
	if (lp -> m_bcc ? bccque : !bccque) {
#ifndef	BERK
	    if (talk && !state)
		printf (MSGSTR(LRECIPS, "  -- Local Recipients --\n")); /*MSG*/
#endif	BERK
	    do_a_cipher (lp, talk);
#ifndef	BERK
	    state++;
#endif	BERK
	}

    for (state = 0, lp = uuaddrs.m_next; lp; lp = lp -> m_next)
	if (lp -> m_bcc ? bccque : !bccque) {
#ifndef	BERK
	    if (talk && !state)
		printf (MSGSTR(URECIPS, "  -- UUCP Recipients --\n")); /*MSG*/
#endif	BERK
	    do_a_cipher (lp, talk);
#ifndef	BERK
	    state++;
#endif	BERK
	}

    for (state = 0, lp = netaddrs.m_next; lp; lp = lp -> m_next)
	if (lp -> m_bcc ? bccque : !bccque) {
#ifndef	BERK
	    if (talk && !state)
		printf (MSGSTR(NRECIPS, "  -- Network Recipients --\n")); /*MSG*/
#endif	BERK
	    do_a_cipher (lp, talk);
#ifndef	BERK
	    state++;
#endif	BERK
	}

    if (ciphdone (reason) == NOTOK)
	admonish (NULLCP, "%s", reason);
#ifdef	SENDMTS
    if (!(msgflags & MINV) || bccque)
	(void) sm_end (OK);
#endif	SENDMTS

    if (verbose)
	if (msgflags & MINV) {
	    strcpy (hlds, MSGSTR(COPIES, " -- %s Recipient Copies Posted --\n")); /*MSG*/
	    printf (hlds, bccque ? MSGSTR(BLIND, "Blind") : MSGSTR(SIGHTED, "Sighted")); /*MSG*/
	}
	else
	    printf (MSGSTR(COPIES2, " -- Recipient Copies Posted --\n")); /*MSG*/
    (void) fflush (stdout);
}

/*  */

static do_a_cipher (register struct mailname *lp,
	    	    int	talk)
{
    int     fd,
            retval;
    register char  *mbox,
                   *host;
    char    addr[BUFSIZ],
            reason[BUFSIZ];
#ifdef	MMDFMTS
#ifdef	RP_NS
    int	    len;
    struct rp_bufstruct reply;
#endif	RP_NS
#endif	MMDFMTS

    sigon ();

#ifdef	MMDFMTS
    if (rp_isbad (retval = mm_init ())
	    || rp_isbad (retval = mm_sbinit ())
	    || rp_isbad (retval = mm_winit (NULL, submitopts, from)))
	die (NULLCP, MSGSTR(MMDFPROB, "problem initializing MMDF system [%s]"), rp_valstr (retval)); /*MSG*/
#ifdef	RP_NS
	if (rp_isbad (retval = mm_rrply (&reply, &len)))
	    die (NULLCP, MSGSTR(SADDRPROB, "problem with sender address [%s]"), rp_valstr (retval)); /*MSG*/
#endif	RP_NS
#endif	MMDFMTS
#ifdef	SENDMTS
    if (rp_isbad (retval = sm_init (clientsw, serversw, watch, verbose, snoop))
	    || rp_isbad (retval = sm_winit (smtpmode, from)))
	die (NULLCP, MSGSTR(SERVPROB, "problem initializing server; %s"), rp_string (retval)); /*MSG*/
#endif	SENDMTS

    do_an_address (lp, talk, 0);

    switch (lp -> m_type) {
	case LOCALHOST: 
	    mbox = lp -> m_mbox;
	    host = LocalName ();
	    (void) strcpy (addr, mbox);
	    break;

	case UUCPHOST: 
#ifdef	MMDFMTS
	    mbox = concat (lp -> m_host, "!", lp -> m_mbox, NULLCP);
	    host = UucpChan ();
#endif	MMDFMTS
#ifdef	SENDMTS
	    mbox = auxformat (lp, 0);
	    host = NULL;
#endif	SENDMTS
	    (void) sprintf (addr, "%s!%s", lp -> m_host, lp -> m_mbox);
	    break;

	default: 
	    mbox = lp -> m_mbox;
	    host = lp -> m_host;
	    (void) sprintf (addr, "%s at %s", lp -> m_mbox, lp -> m_host);
	    break;
    }
    chkadr ();			/* XXX */

#ifdef	MMDFMTS
    if (rp_isbad (retval = mm_waend ()))
	die (NULLCP, MSGSTR(PROB, "problem ending addresses [%s]\n"), rp_valstr (retval)); /*MSG*/
#endif	MMDFMTS
#ifdef	SENDMTS
    if (rp_isbad (retval = sm_waend ()))
	die (NULLCP, MSGSTR(PROB2, "problem ending addresses; %s"), rp_string (retval)); /*MSG*/
#endif	SENDMTS

    if ((fd = encipher (mbox, host, reason)) == NOTOK)
	die (NULLCP, "%s: %s", addr, reason);
    do_text ("temporary file", fd);
    (void) close (fd);
    (void) fflush (stdout);

#ifdef	MMDFMTS
    (void) mm_sbend ();
    (void) mm_end (OK);
#endif	MMDFMTS
#ifdef	SENDMTS
    (void) sm_end (DONE);
#endif	SENDMTS

    sigoff ();
}
#endif	TMA

/*  */

#ifndef	MHMTS
static do_addresses (int bccque, int talk)
#else	MHMTS
static do_addresses (register char *file, int fd, int ud, int ccque, int alk)
#endif	MHMTS
{
    int     retval;
#ifndef	BERK
    int	    state;
#endif	not BERK
    register struct mailname *lp;

#ifndef	BERK
    state = 0;
#endif	not BERK
    for (lp = localaddrs.m_next; lp; lp = lp -> m_next)
	if (lp -> m_bcc ? bccque : !bccque) {
#ifndef	BERK
	    if (talk && !state)
		printf (MSGSTR(LRECIPS, "  -- Local Recipients --\n")); /*MSG*/
#endif	not BERK
#ifndef	MHMTS
	    do_an_address (lp, talk, 0);
#else	MHMTS
	    localmail (lp, talk, fd);
#endif	MHMTS
#ifndef	BERK
	    state++;
#endif	not BERK
	}

#ifndef	BERK
    state = 0;
#endif	not BERK
    for (lp = uuaddrs.m_next; lp; lp = lp -> m_next)
	if (lp -> m_bcc ? bccque : !bccque) {
#ifndef	BERK
	    if (talk && !state)
		printf (MSGSTR(URECIPS, "  -- UUCP Recipients --\n")); /*MSG*/
#endif	not BERK
#ifndef	MHMTS
	    do_an_address (lp, talk, 0);
#else	MHMTS
	    uucpmail (lp, talk, ud != NOTOK ? ud : fd, ud == NOTOK);
#endif	MHMTS
#ifndef	BERK
	    state++;
#endif	not BERK
	}

#ifndef	BERK
    state = 0;
#endif	not BERK
    for (lp = netaddrs.m_next; lp; lp = lp -> m_next)
	if (lp -> m_bcc ? bccque : !bccque) {
#ifndef	BERK
	    if (talk && !state)
		printf (MSGSTR(NRECIPS, "  -- Network Recipients --\n")); /*MSG*/
#endif	not BERK
#ifndef	MHMTS
	    do_an_address (lp, talk, 0);
#else	MHMTS
	    netmail (talk, fd, bccque);
#endif	MHMTS
#ifndef	BERK
	    state++;
#endif	not BERK
	}

/*  */

    chkadr ();

#ifdef	MMDFMTS
    if (rp_isbad (retval = mm_waend ()))
	die (NULLCP, MSGSTR(PROB, "problem ending addresses [%s]\n"), rp_valstr (retval)); /*MSG*/
#endif	MMDFMTS
#ifdef	SENDMTS
    if (rp_isbad (retval = sm_waend ()))
	die (NULLCP, MSGSTR(PROB2, "problem ending addresses; %s"), rp_string (retval)); /*MSG*/
#endif	SENDMTS
}

/*  */

#ifndef	MHMTS
static  do_text (register char *file, int fd)
{
    int     retval,
            state;
    char    buf[BUFSIZ];
#ifdef	MMDFMTS
    struct rp_bufstruct reply;
#endif	MMDFMTS

    (void) lseek (fd, 0L, 0);
    while ((state = read (fd, buf, sizeof buf)) > 0)
#ifdef	MMDFMTS
	if (rp_isbad (mm_wtxt (buf, state)))
	    die (NULLCP, MSGSTR(PROB3, "problem writing text [%s]\n"), rp_valstr (retval)); /*MSG*/
#endif	MMDFMTS
#ifdef	SENDMTS
	if (rp_isbad (retval = sm_wtxt (buf, state)))
	    die (NULLCP, MSGSTR(PROB4, "problem writing text; %s\n"), rp_string (retval)); /*MSG*/
#endif	SENDMTS

    if (state == NOTOK)
	die (file, MSGSTR(PROB5, "problem reading from %s"), file); /*MSG*/

#ifdef	MMDFMTS
    if (rp_isbad (retval = mm_wtend ()))
	die (NULLCP, MSGSTR(PROB6, "problem ending text [%s]\n"), rp_valstr (retval)); /*MSG*/

    if (rp_isbad (retval = mm_rrply (&reply, &state)))
	die (NULLCP, MSGSTR(PROB7, "problem getting submission status [%s]\n"), rp_valstr (retval)); /*MSG*/

    switch (rp_gval (reply.rp_val)) {
	case RP_OK: 
	case RP_MOK: 
	    break;

	case RP_NO: 
	    die (NULLCP, MSGSTR(YOULOSE2, "you lose; %s"), reply.rp_line); /*MSG*/

	case RP_NDEL: 
	    die (NULLCP, MSGSTR(NODELIV, "no delivery occurred; %s"), reply.rp_line); /*MSG*/

	case RP_AGN: 
	    die (NULLCP, MSGSTR(TRYAGAIN, "try again later; %s"), reply.rp_line); /*MSG*/

	case RP_NOOP: 
	    die (NULLCP, MSGSTR(NOTHING, "nothing done; %s"), reply.rp_line); /*MSG*/

	default: 
	    die (NULLCP, MSGSTR(UNXPRESP, "unexpected response;\n\t[%s] -- %s"), rp_valstr (reply.rp_val), reply.rp_line); /*MSG*/
    }
#endif	MMDFMTS
#ifdef	SENDMTS
    switch (retval = sm_wtend ()) {
	case RP_OK: 
	    break;

	case RP_NO: 
	case RP_NDEL: 
	    die (NULLCP, MSGSTR(FAILED, "posting failed; %s"), rp_string (retval)); /*MSG*/

	default: 
	    die (NULLCP, MSGSTR(UNEXPRESP, "unexpected response; %s"), rp_string (retval)); /*MSG*/
    }
#endif	SENDMTS
}
#endif	not MHMTS

/*    MTS-SPECIFIC INTERACTION */

#ifdef	MMDFMTS

#ifndef	TMA
/* ARGSUSED */
#endif	TMA

static do_an_address (register struct mailname *lp,
                    int talk, int tma)
{
    int     len,
            retval;
    register char  *mbox,
                   *host,
                   *text,
                   *path;
    char    addr[BUFSIZ];
#ifdef	TMA
    char    reason[BUFSIZ];
#endif	TMA
    struct rp_bufstruct reply;

    switch (lp -> m_type) {
	case LOCALHOST: 
	    mbox = lp -> m_mbox;
	    host = LocalName ();
	    (void) strcpy (addr, mbox);
	    break;

	case UUCPHOST: 
#ifdef	MF
	    mbox = concat (lp -> m_host, "!", lp -> m_mbox, NULLCP);
	    host = UucpChan ();
	    (void) strcpy (addr, mbox);
	    break;
#else	MF
	    fprintf (talk ? stdout : stderr, "  %s!%s: %s\n",
		lp -> m_host, lp -> m_mbox, MSGSTR(NOTSUPP, "not supported; UUCP address")); /*MSG*/
	    unkadr++;
	    (void) fflush (stdout);
	    return;
#endif	MF

	default: 		/* let MMDF decide if the host is bad */
	    mbox = lp -> m_mbox;
	    host = lp -> m_host;
	    (void) sprintf (addr, MSGSTR(AT, "%s at %s"), mbox, host); /*MSG*/
	    break;
    }
#ifdef	TMA
    if ((!whomsw || checksw)
	    && tma
	    && seekaddr (mbox, host, reason) == NOTOK) {
	fprintf (talk ? stdout : stderr, "  %s%s: %s\n",
		addr, "[TMA]", reason);
	unkadr++;
    }
#endif	TMA

    if (talk)
	printf ("  %s%s", addr, whomsw && lp -> m_bcc ? "[BCC]" : "");

    if (whomsw && !checksw) {
	(void) putchar ('\n');
	return;
    }
    if (talk)
	printf (": ");
    (void) fflush (stdout);

/*  */

#ifdef	MMDFII
    if (lp -> m_path)
/*
 *    If host is not null, add @host to recipient.  Else leave it
 *    alone and let local sendmail take care of it. - P0007179
 */
      if (strlen(host) == 0) 
	path = concat (lp -> m_path, mbox, "", host, NULLCP);  
      else
	path = concat (lp -> m_path, mbox, "@", host, NULLCP);
    else
#endif	MMDFII
	path = NULLCP;
    if (rp_isbad (retval = mm_wadr (path ? NULLCP : host, path ? path : mbox))
	    || rp_isbad (retval = mm_rrply (&reply, &len)))
	die (NULLCP, MSGSTR(PROB8, "problem submitting address [%s]"), rp_valstr (retval)); /*MSG*/

    switch (rp_gval (reply.rp_val)) {
	case RP_AOK: 
	    if (talk)
		printf (MSGSTR(ADDOK, "address ok\n")); /*MSG*/
	    (void) fflush (stdout);
	    return;

#ifdef	RP_DOK
	case RP_DOK: 
	    if (talk)
		printf (MSGSTR(QUEUED, "nameserver timeout - queued for checking\n")); /*MSG*/
	    (void) fflush (stdout);
	    return;
#endif	RP_DOK

	case RP_NO: 
	    text = MSGSTR(YOULOSE, "you lose"); /*MSG*/
	    break;

#ifdef	RP_NS
	case RP_NS: 
	    text = MSGSTR(TMPNFAIL, "temporary nameserver failure"); /*MSG*/
	    break;

#endif	RP_NS

	case RP_USER: 
	case RP_NDEL: 
	    text = MSGSTR(NOTDEL, "not deliverable"); /*MSG*/
	    break;

	case RP_AGN: 
	    text = MSGSTR(LATER, "try again later"); /*MSG*/
	    break;

	case RP_NOOP: 
	    text = MSGSTR(NOTHING2, "nothing done"); /*MSG*/
	    break;

	default: 
	    if (!talk)
		fprintf (stderr, "  %s: ", addr);
	    text = MSGSTR(UNXPRESP2, "unexpected response"); /*MSG*/
	    die (NULLCP, "%s;\n    [%s] -- %s", text,
		    rp_valstr (reply.rp_val), reply.rp_line);
    }

    if (!talk)
	fprintf (stderr, "  %s: ", addr);
    fprintf (talk ? stdout : stderr, "%s;\n    %s\n", text, reply.rp_line);
    unkadr++;

    (void) fflush (stdout);
}
#endif	MMDFMTS

/*  */

#ifdef	MHMTS
/* ARGSUSED */

static do_an_address (lp, talk, tma)
register struct mailname *lp;
int     talk,
	tma;
{
    register char  *mbox;
    char    addr[BUFSIZ];

    switch (lp -> m_type) {
	case LOCALHOST: 
	    (void) strcpy (addr, lp -> m_mbox);
	    break;

	case UUCPHOST: 
	    (void) sprintf (addr, "%s!%s", lp -> m_host, lp -> m_mbox);
	    break;

	default: 
	    (void) sprintf (addr, MSGSTR(AT, "%s at %s"), lp -> m_mbox, lp -> m_host); /*MSG*/
	    break;
    }
    if (talk)
	printf ("  %s%s", addr, whomsw && lp -> m_bcc ? "[BCC]" : "");

    if (whomsw && !checksw) {
	(void) putchar ('\n');
	return;
    }
    if (talk)
	printf (": ");
    (void) fflush (stdout);

/*  */

    switch (lp -> m_type) {
	case LOCALHOST: 
	    mbox = lp -> m_mbox;
	    if (*mbox == '~')
		mbox++;
	    if (seek_home (mbox)) {
		lp -> m_mbox = mbox;
		if (talk)
		    printf (MSGSTR(ADDOK, "address ok\n")); /*MSG*/
	    }
	    else {
		if (!talk)
		    fprintf (stderr, "  %s: ", addr);
		fprintf (talk ? stdout : stderr,
			MSGSTR(NOTDEL2, "not deliverable; unknown user\n")); /*MSG*/
		unkadr++;
	    }
	    break;

	case UUCPHOST: 
	    if (uucpsite (lp -> m_host) == OK) {
		if (talk)
		    printf (MSGSTR(ADDOK, "address ok\n")); /*MSG*/
	    }
	    else {
		if (!talk)
		    fprintf (stderr, "  %s: ", addr);
		fprintf (talk ? stdout : stderr,
			MSGSTR(NOTDEL3, "not deliverable; unknown system\n")); /*MSG*/
		unkadr++;
	    }
	    break;

	case NETHOST: 
	    if (talk)
		printf (MSGSTR(ADDOK, "address ok\n")); /*MSG*/
	    break;

	default: 
	    if (!talk)
		fprintf (stderr, "  %s: ", addr);
	    fprintf (talk ? stdout : stderr,
		    MSGSTR(NOTDEL4, "not deliverable; unknown host\n")); /*MSG*/
	    unkadr++;
	    break;
    }

    (void) fflush (stdout);
}
#endif	MHMTS

/*  */

#ifdef	SENDMTS

#ifndef	TMA
/* ARGSUSED */
#endif	TMA

static do_an_address (register struct mailname *lp,
		      int     talk,
		      int     tma)
{
    int     retval;
    register char  *mbox,
                   *host;
    char    addr[BUFSIZ];
#ifdef	TMA
    char    reason[BUFSIZ];
#endif	TMA

    switch (lp -> m_type) {
	case LOCALHOST: 
	    mbox = lp -> m_mbox;
	    host = lp -> m_host;
	    (void) strcpy (addr, mbox);
	    break;

	case UUCPHOST: 
	    mbox = auxformat (lp, 0);
	    host = NULL;
	    (void) sprintf (addr, "%s!%s", lp -> m_host, lp -> m_mbox);
	    break;

	default: 		/* let SendMail decide if the host is bad  */
	    mbox = lp -> m_mbox;
	    host = lp -> m_host;
	    (void) sprintf (addr, MSGSTR(AT, "%s at %s"), mbox, host); /*MSG*/
	    break;
    }

#ifdef	TMA
    if ((!whomsw || checksw)
	    && tma
	    && seekaddr (mbox, host, reason) == NOTOK) {
	fprintf (talk ? stdout : stderr, "  %s%s: %s\n",
		addr, "[TMA]", reason);
	unkadr++;
    }
#endif	TMA

    if (talk)
	printf ("  %s%s", addr, whomsw && lp -> m_bcc ? "[BCC]" : "");

    if (whomsw && !checksw) {
	(void) putchar ('\n');
	return;
    }
    if (talk)
	printf (": ");
    (void) fflush (stdout);

/*  */

    switch (retval = sm_wadr (mbox, host,
			 lp -> m_type != UUCPHOST ? lp -> m_path : NULLCP)) {
	case RP_OK: 
	    if (talk)
		printf (MSGSTR(ADDOK, "address ok\n")); /*MSG*/
	    break;

	case RP_NO: 
	case RP_USER: 
	    if (!talk)
		fprintf (stderr, "  %s: ", addr);
	    fprintf (talk ? stdout : stderr, MSGSTR(LOSES, "loses; %s\n"), rp_string (retval)); /*MSG*/
	    unkadr++;
	    break;

	default: 
	    if (!talk)
		fprintf (stderr, "  %s: ", addr);
	    die (NULLCP, MSGSTR(UNEXPRESP, "unexpected response; %s"), rp_string (retval)); /*MSG*/
    }

    (void) fflush (stdout);
}
#endif	SENDMTS

/*    SIGNAL HANDLING */

#ifndef	MHMTS

/* ARGSUSED */

static	void sigser (int i)
{
#ifndef	BSD42
    (void) signal (i, SIG_IGN);
#endif	not BSD42
    (void) unlink (tmpfil);
    if (msgflags & MINV)
	(void) unlink (bccfil);
#ifdef	MMDFMTS
    if (!whomsw || checksw)
	(void) mm_end (NOTOK);
#endif	MMDFMTS
#ifdef	SENDMTS
    if (!whomsw || checksw)
	(void) sm_end (NOTOK);
#endif	SENDMTS
    done (1);
}
#endif	not MHMTS


static  sigon () {
    if (debug)
	return;

#ifndef	MHMTS
    setsigx (hstat, SIGHUP, (void(*)(int))sigser);
    setsigx (istat, SIGINT, (void(*)(int))sigser);
    setsigx (qstat, SIGQUIT, (void(*)(int))sigser);
    setsigx (tstat, SIGTERM, (void(*)(int))sigser);
#else	MHMTS
    setsigx (hstat, SIGHUP, SIG_IGN);
    setsigx (istat, SIGINT, SIG_IGN);
    setsigx (qstat, SIGQUIT, SIG_IGN);
    setsigx (tstat, SIGTERM, SIG_IGN);
#endif	MHMTS
}


static sigoff () {
    if (debug)
	return;

    (void) signal (SIGHUP, (void(*)(int))hstat);
    (void) signal (SIGINT, (void(*)(int))istat);
    (void) signal (SIGQUIT, (void(*)(int))qstat);
    (void) signal (SIGTERM, (void(*)(int))tstat);
}

/*    FCC INTERACTION */

static  refile_local (register char   *file)
{
    register int     i;

    if (fccind == 0)
	return;

#ifdef	MHMTS
    (void) setuid (myuid);
#endif	MHMTS
    if (verbose)
	printf (MSGSTR(FORMAT1, " -- Filing Folder Copies --\n")); /*MSG*/
    for (i = 0; i < fccind; i++)
	fcc (file, fccfold[i]);
    if (verbose)
	printf (MSGSTR(FORMAT2, " -- Folder Copies Filed --\n")); /*MSG*/
}


static fcc (register char *file, register char *folder)
{
    int     i,
            child_id,
	    status;
    char    fold[BUFSIZ];

    if (verbose)
	printf ("  %sFcc %s: ", msgstate == RESENT ? "Resent-" : "", folder);
    (void) fflush (stdout);

    for (i = 0; (child_id = fork ()) == NOTOK && i < 5; i++)
	sleep (5);
    switch (child_id) {
	case NOTOK: 
	    if (!verbose)
		fprintf (stderr, "  %sFcc %s: ",
			msgstate == RESENT ? "Resent-" : "", folder);
	    fprintf (verbose ? stdout : stderr, MSGSTR(NOFORKS, "no forks, so not ok\n")); /*MSG*/
	    break;

	case OK: 
	    (void) sprintf (fold, "%s%s",
		    *folder == '+' || *folder == '@' ? "" : "+", folder);
	    execlp (fileproc, r1bindex (fileproc, '/'),
		    "-link", "-file", file, fold, NULLCP);
	    _exit (-1);

	default: 
	    if (status = pidwait (child_id, OK)) {
		if (!verbose)
		    fprintf (stderr, "  %sFcc %s: ",
			    msgstate == RESENT ? "Resent-" : "", folder);
		(void) pidstatus (status, verbose ? stdout : stderr, NULLCP);
	    }
	    else
		if (verbose)
		    printf (MSGSTR(FOLDOK, "folder ok\n")); /*MSG*/
    }

    (void) fflush (stdout);
}

/*    TERMINATION */

/* VARARGS2 */

die (what, fmt, a, b, c, d)
char   *what,
       *fmt,
       *a,
       *b,
       *c,
       *d;
{
#ifndef	MHMTS
    (void) unlink (tmpfil);
    if (msgflags & MINV)
	(void) unlink (bccfil);
#endif	MHMTS
#ifdef	MMDFMTS
    if (!whomsw || checksw)
	(void) mm_end (NOTOK);
#endif	MMDFMTS
#ifdef	SENDMTS
    if (!whomsw || checksw)
	(void) sm_end (NOTOK);
#endif	SENDMTS

    adios (what, fmt, a, b, c, d);
}


#ifdef	MMDFMTS
/* 
 *    err_abrt() is used by the mm_ routines
 *    		 do not, under *ANY* circumstances, remove it from post,
 *		 or you will lose *BIG*
 */

err_abrt (code, fmt, a, b, c)
int     code;
char   *fmt,
       *a,
       *b,
       *c;
{
    char    buffer[BUFSIZ];

    (void) sprintf (buffer, "[%s]", rp_valstr (code));

    adios (buffer, fmt, a, b, c);
}
#endif	MMDFMTS

/*    STAND-ALONE DELIVERY */

#ifdef	MHMTS

/* BUG: MHMTS ignores 822-style route addresses... */

static  localmail (lp, talk, fd)
register struct mailname *lp;
int     talk,
        fd;
{
    int     md;
    char    mailbox[BUFSIZ],
	    ddate[BUFSIZ];
    register struct home *hp;

    if (talk)
	printf ("  %s: ", lp -> m_mbox);
    (void) fflush (stdout);

    if ((hp = seek_home (lp -> m_mbox)) == NULL) {
	if (!talk)
	    fprintf (stderr, "  %s: ", lp -> m_mbox);
	fprintf (talk ? stdout : stderr,
		MSGSTR(NOTDEL5, "not deliverable; unknown address\n")); /*MSG*/
	unkadr++;
	return;
    }

    (void) sprintf (mailbox, "%s/%s",
	    mmdfldir[0] ? mmdfldir : hp -> h_home,
	    mmdflfil[0] ? mmdflfil : hp -> h_name);

/*  */

    switch (access (slocalproc, 01)) {
	default: 
	    if (talk)
		printf (MSGSTR(HOOK2, "(invoking hook)\n\t")); /*MSG*/
	    (void) fflush (stdout);

	    if (usr_hook (lp, talk, fd, hp, mailbox) != NOTOK)
		return;
	    if (talk)
		printf ("  %s: ", lp -> m_mbox);
	    (void) fflush (stdout);

	case NOTOK: 
	    (void) lseek (fd, 0L, 0);
	    if ((md = mbx_open (mailbox, hp -> h_uid, hp -> h_gid, m_gmprot ()))
		    == NOTOK) {
		if (!talk)
		    fprintf (stderr, "  %s: ", lp -> m_mbox);
		fprintf (talk ? stdout : stderr,
			MSGSTR(TRANSERR, "error in transmission; unable to open maildrop\n")); /*MSG*/
		unkadr++;
		return;
	    }
	    (void) sprintf (ddate, "Delivery-Date: %s\n", dtimenow ());
	    if (mbx_copy (mailbox, md, fd, 0, ddate, 0) == NOTOK) {
		if (!talk)
		    fprintf (stderr, "  %s: ", lp -> m_mbox);
		fprintf (talk ? stdout : stderr,
			MSGSTR(TRANSERR2, "error in transmission; write to maildrop failed\n")); /*MSG*/
		unkadr++;
		(void) close (md);
		return;
	    }
	    mbx_close (mailbox, md);

	    if (talk)
		printf (MSGSTR(SENT, "sent\n")); /*MSG*/
	    break;
    }

    (void) fflush (stdout);
}

/*  */

static int  usr_hook (lp, talk, fd, hp, mailbox)
register struct mailname *lp;
int     talk,
        fd;
register struct home *hp;
register char   *mailbox;
{
    int     i,
            child_id,
            status;
    char    tmpfil[BUFSIZ];

    if ((fd = copyfile (fd, tmpfil)) == NOTOK) {
	if (!talk)
	    fprintf (stderr, "  %s: ", lp -> m_mbox);
	fprintf (talk ? stdout : stderr,
		MSGSTR(NOCOPY, "unable to copy message; skipping hook\n")); /*MSG*/
	return NOTOK;
    }
    (void) chown (tmpfil, hp -> h_uid, hp -> h_gid);

    (void) fflush (stdout);

    for (i = 0; (child_id = fork ()) == NOTOK && i < 5; i++)
	sleep (5);
    switch (child_id) {
	case NOTOK: 
	    if (!talk)
		fprintf (stderr, "  %s: ", lp -> m_mbox);
	    fprintf (talk ? stdout : stderr,
		    MSGSTR(NOHOOK, "unable to invoke hook; fork() failed\n")); /*MSG*/
	    return NOTOK;

	case OK: 
	    if (fd != 0)
		(void) dup2 (fd, 0);
	    (void) freopen ("/dev/null", "w", stdout);
	    (void) freopen ("/dev/null", "w", stderr);
	    if (fd != 3)	/* backwards compatible... */
		(void) dup2 (fd, 3);
	    closefds (4);
#ifdef	TIOCNOTTY
	    if ((fd = open ("/dev/tty", 2)) != NOTOK) {
		(void) ioctl (fd, TIOCNOTTY, NULLCP);
		(void) close (fd);
	    }
#endif	TIOCNOTTY
#ifdef	BSD42
	    (void) setpgrp (0, getpid ());
#endif	BSD42

	    *environ = NULL;
	    (void) putenv ("USER", hp -> h_name);
	    (void) putenv ("HOME", hp -> h_home);
	    (void) putenv ("SHELL", hp -> h_shell);
	    if (chdir (hp -> h_home) == NOTOK)
		(void) chdir ("/");
	    (void) umask (0077);
#ifdef	BSD41A
	    (void) inigrp (hp -> h_name, hp -> h_gid);
#endif	BSD41A
	    (void) setgid (hp -> h_gid);
#ifdef	BSD42
	    (void) initgroups (hp -> h_name, hp -> h_gid);
#endif	BSD42
	    (void) setuid (hp -> h_uid);

	    execlp (slocalproc, r1bindex (slocalproc, '/'),
		    "-file", tmpfil, "-mailbox", mailbox,
		    "-home", hp -> h_home, "-addr", lp -> m_aka,
		    "-user", hp -> h_name, "-sender", from,
		    talk ? "-verbose" : NULLCP, NULLCP);
	    _exit (-1);

/*  */

	default: 
	    (void) close (fd);

	    status = pidwait (child_id, OK);

	    (void) unlink (tmpfil);
	    if (status == 0) {
		if (talk)
		    printf (MSGSTR(ACCEPTED, "accepted\n")); /*MSG*/
		return OK;
	    }
	    if (!talk)
		fprintf (stderr, "  %s: ", lp -> m_mbox);
	    strcpy (hlds, MSGSTR(HOOKERR, "%s error on hook; status=0%o\n")); /*MSG*/
	    fprintf (talk ? stdout : stderr, hlds,
		    status & 0x00ff ? MSGSTR(SYSTEM, "system") : MSGSTR(USER, "user"), status & 0x00ff ? status & 0xff : (status & 0xff00) >> 8); /*MSG*/
	    return NOTOK;
    }
}

/*  */

static int  copyfile (qd, tmpfil)
int     qd;
register char   *tmpfil;
{
    int     i,
            fd;
    char    buffer[BUFSIZ];

    (void) strcpy (tmpfil, m_tmpfil ("hook"));
    if ((fd = creat (tmpfil, 0600)) == NOTOK)
	return NOTOK;
    (void) close (fd);
    if ((fd = open (tmpfil, 2)) == NOTOK)
	return NOTOK;

    (void) lseek (qd, 0L, 0);
    while ((i = read (qd, buffer, sizeof buffer)) > 0)
	if (write (fd, buffer, i) != i) {
	    (void) close (fd);
	    return NOTOK;
	}
    if (i == NOTOK) {
	(void) close (fd);
	return NOTOK;
    }

    (void) lseek (fd, 0L, 0);

    return fd;
}

/*  */

static  uucpmail (lp, talk, fd, from)
register struct mailname *lp;
int     talk,
        fd,
	from;
{
    int     i;
    int     (*pstat) ();
    char    addr[BUFSIZ],
            buffer[BUFSIZ];
    register FILE *fp;

    (void) sprintf (addr, "%s!%s", lp -> m_host, lp -> m_mbox);
    if (talk)
	printf ("  %s: ", addr);
    (void) fflush (stdout);

#ifndef	UCI
    (void) sprintf (buffer, "uux -r -p %s!rmail \\(%s\\)",
		lp -> m_host, lp -> m_mbox);
#else	UCI
    (void) sprintf (buffer, "uux -p %s!rmail \\(%s\\)", lp -> m_host,
	    lp -> m_mbox);
#endif	UCI
    if ((fp = popen (buffer, "w")) == NULL) {
	if (!talk)
	    fprintf (stderr, "  %s: ", addr);
	fprintf (talk ? stdout : stderr,
		MSGSTR(POPENFAIL, "unable to start uux; popen() failed\n")); /*MSG*/
	unkadr++;
	return;
    }

    pstat = signal (SIGPIPE, SIG_IGN);
    if (from) {			/* no mail filtering, so... */
	(void) sprintf (buffer, "From %s %.24s remote from %s\n", getusr (), ctime (&clock), SystemName ());
	i = strlen (buffer);
	if (fwrite (buffer, sizeof *buffer, i, fp) != i)
	    goto oops;
    }

    (void) lseek (fd, 0L, 0);
    while ((i = read (fd, buffer, sizeof buffer)) > 0)
	if (fwrite (buffer, sizeof *buffer, i, fp) != i) {
    oops:   ;
	    if (!talk)
		fprintf (stderr, "  %s: ", addr);
	    fprintf (talk ? stdout : stderr,
		    MSGSTR(TRANSERR3, "error in transmission; write to uux failed\n")); /*MSG*/
	    unkadr++;
	    (void) pclose (fp);
	    return;
	}
    if (pclose (fp))
	goto oops;
    (void) signal (SIGPIPE, pstat);

    if (i < 0) {
	if (!talk)
	    fprintf (stderr, "  %s: ", addr);
	fprintf (talk ? stdout : stderr,
		MSGSTR(TRANSERR4, "error in transmission; read failed\n")); /*MSG*/
	unkadr++;
	return;
    }

    if (talk)
	printf (MSGSTR(QUEUED2, "queued (via uux)\n")); /*MSG*/
    (void) fflush (stdout);
}

/*  */

#ifdef	MF
static int  make_uucp_file (td)
int     td;
{
    int     i,
            qd,
            fd;
    char    tmpfil[BUFSIZ];

    (void) lseek (td, 0L, 0);
    if ((qd = dup (td)) == NOTOK)
	adios ("fd", MSGSTR(NODUPFD, "unable to dup fd")); /*MSG*/

    (void) strcpy (tmpfil, m_tmpfil ("uumf"));
    if ((fd = creat (tmpfil, 0600)) == NOTOK)
	adios (tmpfil, MSGSTR(NOCREATE, "unable to create %s"), tmpfil); /*MSG*/
    (void) close (fd);
    if ((fd = open (tmpfil, 2)) == NOTOK)
	adios (tmpfil, MSGSTR(NOROPEN, "unable to re-open %s"), tmpfil); /*MSG*/

    switch (i = mmdf2uucp (qd, fd, 1)) {
	case OK: 
	    if (!debug)
		(void) unlink (tmpfil);
	    break;

	default: 
	    adios (NULLCP, MSGSTR(NOFILTER, "unable to filter mail(%d), examine %s"), i, tmpfil); /*MSG*/
    }
    (void) close (qd);

    return fd;
}
#endif	MF

/*  */

static  netmail (talk, fd, bccque)
int     talk,
        fd,
        bccque;
{
    int     i,
            naddrs;
    char    buffer[BUFSIZ];
    register struct mailname *lp;

    naddrs = 0;
    if (nm_init (getusr (), &clock) == NOTOK) {
	for (lp = netaddrs.m_next; lp; lp = lp -> m_next)
	    if (lp -> m_bcc ? bccque : !bccque)
		fprintf (stderr, MSGSTR(NOQUEUE, "  %s at %s: unable to get queue file\n"), lp -> m_mbox, lp -> m_host); /*MSG*/
	return;
    }

    for (lp = netaddrs.m_next; lp; lp = lp -> m_next)
	if (lp -> m_bcc ? bccque : !bccque) {
	    (void) nm_wadr (lp -> m_mbox, lp -> m_host);
	    naddrs++;
	    if (talk)
		printf (MSGSTR(QUEUED3, "  %s at %s: queued\n"), lp -> m_mbox, lp -> m_host); /*MSG*/
	    (void) fflush (stdout);
	}
    nm_waend ();

    (void) lseek (fd, 0L, 0);
    while ((i = read (fd, buffer, sizeof buffer)) > 0)
	if (nm_wtxt (buffer, i) == NOTOK) {
	    fprintf (stderr,
		    MSGSTR(TRANSERR5, "error in transmission; write to temporary failed")); /*MSG*/
	    unkadr += naddrs;
	    return;
	}

    if (i < 0) {
	fprintf (stderr, MSGSTR(TRANSERR4, "error in transmission; read failed\n")); /*MSG*/
	unkadr += naddrs;
	return;
    }

    if (nm_wtend () == NOTOK) {
	fprintf (stderr, MSGSTR(TRANSERR6, "error in transmission; unable to queue message\n")); /*MSG*/
	unkadr += naddrs;
	return;
    }
}
#endif	MHMTS

#if defined(X400)
static
strcmpX( str1 , str2 )
char    *str1,
        *str2;
{
int     lstr;

    for(lstr=strlen(str1);lstr;lstr--)
      if ((tolower(*str1) != tolower(*str2)) | (*str2 == (char) NULL))
          return (NOTEQUAL);

    return ((*(++str2)) ? NOTEQUAL : EQUAL);
}
#endif
