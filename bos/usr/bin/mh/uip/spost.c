static char sccsid[] = "@(#)58	1.17  src/bos/usr/bin/mh/uip/spost.c, cmdmh, bos411, 9428A410j 11/9/93 09:45:20";
/* 
 * COMPONENT_NAME: CMDMH spost.c
 * 
 * FUNCTIONS: MSGSTR, Mspost, die, fcc, file, finish_headers, 
 *            get_header, insert_fcc, make_bcc_file, putadr, putfmt, 
 *            putone, start_headers, uptolow 
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
/* static char sccsid[] = "spost.c	8.1 88/04/15 15:50:54"; */

/*
 * #ifndef lint
 * static char sccsid[] = "spost.c	1.6 (Berkeley) 11/2/85";
 * #endif
 */

/* spost.c - feed messages to sendmail */
/*
 * (This is a simpler, faster, replacement for "post" for use when "sendmail"
 * is the transport system) 
 */

#include <ctype.h>
#include <stdio.h>
#include <signal.h>
#include <pwd.h>
#include <string.h>
#include "mh.h"
#include "addrsbr.h"
#include "aliasbr.h"
#include "dropsbr.h"
#include "tws.h"

#include "mh_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 

extern char *getfullname (), *getusr ();
extern char *sendmail;


#define	uptolow(c)	(isupper (c) ? tolower (c) : c)

#define	SENDMAIL	"/usr/lib/sendmail"
#define FCCS		10	/* max number of fccs allowed */

/*  */

struct swit switches[] = {
#define	FILTSW	0
    "filter filterfile", 0,
#define	NFILTSW	1
    "nofilter", 0,

#define	FRMTSW	2
    "format", 0,
#define	NFRMTSW	3
    "noformat", 0,

#define	REMVSW	4
    "remove", 0,
#define	NREMVSW	5
    "noremove", 0,

#define	VERBSW	6
    "verbose", 0,
#define	NVERBSW	7
    "noverbose", 0,

#define	WATCSW	8
    "watch", 0,
#define	NWATCSW	9
    "nowatch", 0,

#define	HELPSW	10
    "help", 4,

#define	DEBUGSW	11
    "debug", -5,

#define	DISTSW	12
    "dist", -4,			/* interface from dist */

#define BACKSW 13
    "backup", 0,
#define NBACKSW 14
    "nobackup", 0,

#define CHKSW 15
    "check", -5,		/* interface from whom */
#define NCHKSW 16
    "nocheck", -7,		/* interface from whom */
#define WHOMSW 17
    "whom", -4,			/* interface from whom */

#define PUSHSW 18		/* fork to sendmail then exit */
    "push", -4,
#define NPUSHSW 19		/* exec sendmail */
    "nopush", -6,

#define ALIASW 20
    "alias aliasfile", 0,
#define NALIASW 21
    "noalias", 0,

#define WIDTHSW 22
    "width columns", 0,

#define LIBSW 23
    "library directory", -7,

#define	ANNOSW	24
    "idanno number", -6,

#if defined(X400)
#define	XPUSHSW	25
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
#define	HMNG	0x0020		/* mung this header */
#define	HNGR	0x0040		/* no groups allowed in this header */
#define	HFCC	0x0080		/* FCC: type header */
#define	HNIL	0x0100		/* okay for this header not to have addrs */
#define	HIGN	0x0200		/* ignore this header */
#if defined(X400)
#define HXOR    0x0400          /* this is an x400 IPM originator */
#endif

    unsigned int    set;
#define	MFRM	0x0001		/* we've seen a From: */
#define	MDAT	0x0002		/* we've seen a Date: */
#define	MRFM	0x0004		/* we've seen a Resent-From: */
#define	MVIS	0x0008		/* we've seen sighted addrs */
#define	MINV	0x0010		/* we've seen blind addrs */
#define	MRDT	0x0020		/* we've seen a Resent-Date: */
};

/*  */

static struct headers  NHeaders[] = {
    "Return-Path", HBAD, (int)NULL,
    "Received", HBAD, (int)NULL,
    "Reply-To", HADR | HNGR, (int)NULL,
    "From", HADR | HNGR | HTRY, MFRM,
    "Sender", HADR | HBAD, (int)NULL,
    "Date", HNOP, MDAT,
    "Subject", HSUB, (int)NULL,
    "To", HADR | HTRY, MVIS,
    "cc", HADR | HTRY, MVIS,
    "Bcc", HADR | HTRY | HBCC | HNIL, MINV,
    "Message-Id", HBAD, (int)NULL,
    "Fcc", HFCC, (int)NULL,

    NULL
};

static struct headers  RHeaders[] = {
    "Resent-Reply-To", HADR | HNGR, (int)NULL,
    "Resent-From", HADR | HNGR, MRFM,
    "Resent-Sender", HADR | HBAD, (int)NULL,
    "Resent-Date", HNOP, MRDT,
    "Resent-Subject", HSUB, (int)NULL,
    "Resent-To", HADR | HTRY, MVIS,
    "Resent-cc", HADR | HTRY, MVIS,
    "Resent-Bcc", HADR | HTRY | HBCC, MINV,
    "Resent-Message-Id", HBAD, (int)NULL,
    "Resent-Fcc", HFCC, (int)NULL,
    "Reply-To", HADR, (int)NULL,
    "Fcc", HIGN, (int)NULL,

    NULL
};

/*  */


static short    fccind = 0;	/* index into fccfold[] */

static int  badmsg = 0;		/* message has bad semantics */
static int  verbose = 0;	/* spell it out */
static int  debug = 0;		/* debugging post */
static int  rmflg = 1;		/* remove temporary file when done */
static int  watch = 0;		/* watch the delivery process */
static int  backflg = 0;	/* rename input file as *.bak when done */
static int  whomflg = 0;	/* if just checking addresses */
static int  pushflg = 0;	/* if going to fork to sendmail */
#if defined(X400)
static int  xpushflg = 0;	/* if parent is called with '-push' */
#endif
static int  aliasflg = -1;	/* if going to process aliases */
static int  outputlinelen=72;

static unsigned msgflags = 0;	/* what we've seen */

static enum {
    normal, resent
} msgstate = normal;

static char tmpfil[] = "/tmp/pstXXXXXX";

static char from[BUFSIZ];	/* my network address */
#if defined(X400)
static char *orname=(char *)NULL;               /* my x400 O/R name */
static char *orname_base = NULL;/* base x400 O/R name from profile */
static struct passwd *pw;       /* my passwd stuff */
static char signature[BUFSIZ];	/* my signature */
#endif
static char *filter = NULL;	/* the filter for BCC'ing */
static char *subject = NULL;	/* the subject field for BCC'ing */
static char *fccfold[FCCS];	/* foldernames for FCC'ing */

static struct headers  *hdrtab;	/* table for the message we're doing */
static FILE *out;		/* output (temp) file */
static putfmt (char *, char *, FILE *);
static start_headers(), finish_headers(FILE *);
static int get_header (char *, struct headers *);
static putadr (char *, struct mailname *);
static int putone (register char *, register int, int);
static  insert_fcc (struct headers *, char *);
static  file (char *), fcc (char *, char *);

#if defined(X400)
static char save_from_line[BUFSIZ];     /* text from the From: line */
#define EQUAL           1
#define NOTEQUAL        0
#endif

/*    MAIN */

/* ARGSUSED */

main (argc, argv)
int     argc;
char   *argv[];
{
    int     state,
	    i,
	    pid,
            compnum;
#if defined(X400)
    int x400status = 0;		/* If status != 0, do not send mail out */
    char *x400fldchk();
#endif
    char   *cp,
           *msg = NULL,
          **argp = argv + 1,
	    *sargv[16],
            buf[BUFSIZ],
            name[NAMESZ],
	    *arguments[MAXARGS];
    char    hlds[NL_TEXTMAX];
    FILE * in;

	setlocale(LC_ALL,"");
	catd = catopen(MF_MH, NL_CAT_LOCALE);

    invo_name = r1bindex (argv[0], '/');
    mts_init (invo_name);
    if ((cp = m_find (invo_name)) != NULL) {
	argp = copyip (brkstring (cp, " ", "\n"), arguments);
	(void) copyip (argv+1, argp);
	argp = arguments;
    }

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
		    (void)sprintf (buf, MSGSTR(USEFILE, "%s [switches] file"), invo_name); /*MSG*/
		    help (buf, switches);
		    done (1);

		case DEBUGSW: 
		    debug++;
		    continue;

		case DISTSW:
		    msgstate = resent;
		    continue;

		case WHOMSW:
		    whomflg++;
		    continue;

		case FILTSW:
		    if (!(filter = *argp++) || *filter == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    continue;
		case NFILTSW:
		    filter = NULL;
		    continue;
		
		case REMVSW: 
		    rmflg++;
		    continue;
		case NREMVSW: 
		    rmflg = 0;
		    continue;

		case BACKSW: 
		    backflg++;
		    continue;
		case NBACKSW: 
		    backflg = 0;
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
		
		case PUSHSW:
		    pushflg++;
		    continue;
		case NPUSHSW:
		    pushflg = 0;
		    continue;

		case ALIASW:
		    if (!(cp = *argp++) || *cp == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    if (aliasflg < 0)
			(void)alias (AliasFile);/* load default aka's */
		    aliasflg = 1;
		    if ((state = alias(cp)) != AK_OK)  {
			strcpy(hlds, MSGSTR(ALIASERR, "aliasing error in file %s - %s")); /*MSG*/
			adios (NULLCP, hlds, cp, akerror(state) );
		    }
		    continue;
		case NALIASW:
		    aliasflg = 0;
		    continue;

		case WIDTHSW:
		    if (!(cp = *argp++) || *cp == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    outputlinelen = atoi (cp);
		    if (outputlinelen <= 10)
			outputlinelen = 72;
		    continue;

		case CHKSW:
		case NCHKSW:
		    /* -check & -nocheck switches ignored */
		    continue;

		case LIBSW:
		case ANNOSW:
		    /* -library & -idanno switch ignored */
		    if (!(cp = *argp++) || *cp == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    continue;
#if defined(X400)
		case XPUSHSW:
		    xpushflg++;
		    continue;
#endif
	    }
	if (msg)
	    adios (NULLCP, MSGSTR(ONEMSG, "only one message at a time!")); /*MSG*/
	else
	    msg = cp;
    }

/*  */

    if (aliasflg < 0)
	alias (AliasFile);	/* load default aka's */

    if (!msg)
	adios (NULLCP, MSGSTR(USAGE3, "usage: %s [switches] file"), invo_name); /*MSG*/

    if ((in = fopen (msg, "r")) == NULL)
	adios (msg, MSGSTR(NOOPEN, "unable to open %s"), msg); /*MSG*/

    start_headers ();
    if (debug) {
	verbose++;
	out = stdout;
    }
    else {
	    (void)mktemp (tmpfil);
	    if ((out = fopen (tmpfil, "w")) == NULL)
		adios (tmpfil, MSGSTR(NOCREATE, "unable to create %s"), tmpfil); /*MSG*/
	    (void)chmod (tmpfil, 0600);
	}

    hdrtab = (msgstate == normal) ? NHeaders : RHeaders;

    for (compnum = 1, state = FLD;;) {
	switch (state = m_getfld (state, name, buf, sizeof buf, in)) {
	    case FLD: 
		compnum++;
#if defined(X400)
		if ((cp = x400fldchk(name, buf, xpushflg, &x400status)) == 
			NULLCP) {
		    continue;
		}
		putfmt (name, cp, out);
#else
		putfmt (name, buf, out);
#endif
		continue;

	    case FLDPLUS: 
		compnum++;
		cp = add (buf, cp);
		while (state == FLDPLUS) {
		    state = m_getfld (state, name, buf, sizeof buf, in);
		    cp = add (buf, cp);
		}
#if defined(X400)
		if ((cp = x400fldchk(name, cp, xpushflg, &x400status)) == 
			NULLCP) {
		    continue;
		}
		cp = getcpy(cp);
#endif
		putfmt (name, cp, out);
		free (cp);
		continue;

	    case BODY: 
		finish_headers (out);
		fprintf (out, "\n%s", buf);
		if(whomflg == 0)
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

    (void)fclose (in);
    if (backflg && !whomflg) {
	(void) strcpy (buf, m_backup (msg));
	if (rename (msg, buf) == NOTOK)
	    advise (buf, MSGSTR(NORENAME, "unable to rename %s to %s"), msg, buf); /*MSG*/
    }

    if (debug) {
	done (0);
    }
    else
	(void)fclose (out);

    file (tmpfil);

    /*
     * re-open the temp file, unlink it and exec sendmail, giving it
     * the msg temp file as std in.
     */
    if ( freopen( tmpfil, "r", stdin) == NULL)
	adios (tmpfil, MSGSTR(NOREOPEN, "can't reopen for sendmail %s"), tmpfil); /*MSG*/
    if (rmflg)
	(void)unlink (tmpfil);

    argp = sargv;
    *argp++ = "send-mail";
    *argp++ = "-m";	/* send to me too */
    *argp++ = "-t";	/* read msg for recipients */
    *argp++ = "-i";	/* don't stop on "." */
    *argp++ = "-x";	/* indicate possible extended characters */
    if (whomflg)
	*argp++ = "-bv";
    if (watch || verbose)
	*argp++ = "-v";
    *argp = NULL;

    if (pushflg && !(watch || verbose)) {
	/* fork to a child to run sendmail */
	for (i=0; (pid = vfork()) == NOTOK && i < 5; i++)
	    sleep(5);
	switch (pid) {
	    case NOTOK:
		fprintf (verbose ? stdout : stderr, MSGSTR(NOFORK2, "%s: can't fork to %s\n"), invo_name, SENDMAIL); /*MSG*/
		exit(-1);
	    case OK:
		/* we're the child .. */
		break;
	    default:
		exit(0);
	}
    }
#if defined(X400)
    if( x400status != 0 ){
	sleep(5);
	exit(67); /* Something is wrong in the X400 extended headers, let 
		   * 'sendsbr' return the message to the sender  
		   */
    }
#endif
    if (sendmail == NULL)
	sendmail = SENDMAIL;
    execv ( sendmail, sargv);

    adios ( SENDMAIL, MSGSTR(NOEXEC2, "can't exec %s"), SENDMAIL); /*MSG*/
}

/*    DRAFT GENERATION */

static putfmt (char *name, char *str, FILE *out)
{
    int     count,
            grp,
            i,
            keep;
    char   *cp,
           *pp,
           *qp,
            namep[BUFSIZ];
    struct mailname *mp,
                   *np;
    struct headers *hdr;


    while (*str == ' ' || *str == '\t')
	str++;

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
    msgflags |= hdr -> set;

#if defined(X400)
    if (hdr->set & MFRM) {
        strcpy(save_from_line,str);/* used to compare to x400 O/R name later */
	if (save_from_line[count = strlen(save_from_line) - 1] == '\n')
	    save_from_line[count] = '\0';  /* strip newline */
    }
#endif

    if (hdr -> flags & HSUB)
	subject = subject ? add (str, add ("\t", subject)) : getcpy (str);

    if (hdr -> flags & HFCC) {
	if (cp = (char*)rindex (str, '\n'))
	    *cp = (char)NULL;
	for (cp = pp = str; cp = (char*)index (pp, ','); pp = cp) {
	    *cp++ = (char)NULL;
	    insert_fcc (hdr, pp);
	}
	insert_fcc (hdr, pp);
	return;
    }

#ifdef notdef
    if (hdr -> flags & HBCC) {
	insert_bcc(str);
	return;
    }
#endif notdef

    if (*str != '\n' && *str != '\0')
	if (aliasflg && hdr->flags & HTRY) {
	    /* this header contains address(es) that we have to do
	     * alias expansion on.  Because of the saved state in
	     * getname we have to put all the addresses into a list.
	     * We then let putadr munch on that list, possibly
	     * expanding aliases.
	     */
	    register struct mailname *f = 0;
	    register struct mailname *mp = 0;

	    while (cp = getname( str ) ) {
		mp = getm( cp, NULLCP, 0, AD_HOST, NULLCP);
		if (f == 0) {
		    f = mp;
		    mp->m_next = mp;
		} else {
		    mp->m_next = f->m_next;
		    f->m_next = mp;
		    f = mp;
		}
	    }
	    f = mp->m_next; mp->m_next = 0;
	    putadr( name, f );
	} else {
	    fprintf (out, "%s: %s", name, str );
	}
}

/*  */

static
start_headers ()
{
    char   *cp;
    char    sigbuf[BUFSIZ];

    (void)strcpy( from, getusr() );
#if defined(X400)
    if (orname = orname_base = m_find("orname")) {
        msgflags |= HXOR;
        if ((pw = getpwuid(getuid())) && *(pw->pw_gecos)) {
	    if (cp = malloc(strlen(orname) + strlen(pw->pw_gecos) + 4)) {
		sprintf(cp, "%s (%s)", orname, pw->pw_gecos);
		orname = cp;
	    }
        }
    }
#endif

    if ((cp = getfullname ()) && *cp) {
	(void)strcpy (sigbuf, cp);
        (void)sprintf (signature, "%s <%s>", sigbuf,
#if defined(X400)
                      (msgflags & HXOR) ? orname : from);
#else
		      from);
#endif
    }
    else
        (void)sprintf (signature, "%s",
#if defined(X400)
                      (msgflags & HXOR) ? orname : from);
#else
		      from);
#endif
}

/*  */

static
finish_headers (FILE *out)
{
    switch (msgstate) {
	case normal: 
	    if (!(msgflags & MDAT))
		fprintf (out, "Date: %s\n", dtimenow ());
            if (msgflags & MFRM) {
#if defined(X400)
                if (!(msgflags & HXOR))
#endif
                    fprintf (out, "Sender: %s\n", from);
#if defined(X400)
                else
                    if (strcmpX(save_from_line, orname_base) == NOTEQUAL)
                        fprintf(out, "Sender: %s\n", orname);
#endif
            }
	    else
		fprintf (out, "From: %s\n", signature);
#ifdef notdef
	    if (!(msgflags & MVIS))
		fprintf (out, "Bcc: Blind Distribution List: ;\n");
#endif notdef
	    break;

	case resent: 
	    if (!(msgflags & MRDT))
		fprintf (out, "Resent-Date: %s\n", dtimenow());
            if (msgflags & MFRM) {
#if defined(X400)
                if (!(msgflags & HXOR))
#endif
                    fprintf (out, "Resent-Sender: %s\n", from);
#if defined(X400)
                else
                    if (strcmpX(save_from_line, orname_base) == NOTEQUAL)
                        fprintf(out, "Resent-Sender: %s\n", orname);
#endif
            }
	    else
		fprintf (out, "Resent-From: %s\n", signature);
#ifdef notdef
	    if (!(msgflags & MVIS))
		fprintf (out, "Resent-Bcc: Blind Re-Distribution List: ;\n");
#endif notdef
	    break;
    }

    if (badmsg)
	adios (NULLCP, MSGSTR(REFORMAT, "re-format message and try again")); /*MSG*/
}

/*  */

static int
get_header (char *header, struct headers *table)
{
    struct headers *h;

    for (h = table; h -> value; h++)
	if (uleq (header, h -> value))
	    return (h - table);

    return NOTOK;
}

/*  */

/* output the address list for header "name".  The address list
 * is a linked list of mailname structs.  "nl" points to the head
 * of the list.  Alias substitution should be done on nl.
 */
static putadr (char *name, struct mailname *nl)
{
    register struct mailname *mp, *mp2;
    register int linepos;
    register char *cp;
    register char *hold;
    int namelen;

    fprintf (out, "%s: ", name);
    namelen = strlen(name) + 2;
    linepos = namelen;

    for (mp = nl; mp; ) {
	if (mp->m_nohost) {
	    /* When BERK is in effect, the only way nohost is set
	     * to true is for local addresses. We have added a @
	     * LocalName to these. We will strip it for alias check.
	     * a local name - see if it's an alias */
	    hold = getcpy(mp->m_mbox);
	    cp = (char*)strrchr(hold,'@');
	    if (cp) *cp = '\0';
	    cp = akvalue(hold);
	    if (cp == hold)
		/* wasn't an alias - use what the user typed */
		linepos = putone( mp->m_text, linepos, namelen );
	    else
		/* an alias - expand it */
		while (cp = getname(cp) ) {
		    mp2 = getm( cp, NULLCP, 0, AD_HOST, NULLCP);
		    if (akvisible()) {
			mp2->m_pers = getcpy(mp->m_mbox);
			linepos = putone( adrformat(mp2), linepos, namelen );
		    } else {
			linepos = putone( mp2->m_text, linepos, namelen );
		    }
		    mnfree( mp2 );
		}
	} else {
	    /* not a local name - use what the user typed */
	    linepos = putone( mp->m_text, linepos, namelen );
	}
	mp2 = mp;
	mp = mp->m_next;
	mnfree( mp2 );
    }
    putc( '\n', out );
}

static int putone (register char *adr, register int pos, int indent )
{
    register int len;

    len = strlen( adr );
    if ( pos+len > outputlinelen ) {
	fprintf ( out, ",\n%*s", indent, "");
	pos = indent;
    } else if ( pos > indent ) {
	fputs( ", ", out );
	pos += 2;
    }
    fputs( adr, out );

    return (pos+len);
}

/*  */

static  insert_fcc (struct headers *hdr, char *pp)
{
    char   *cp;

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

#ifdef notdef
/*    BCC GENERATION */

static  make_bcc_file () {
    int     fd,
	    i,
            child_id,
	    status;
    char   *vec[6];
    FILE * in, *out;

    (void)mktemp (bccfil);
    if ((out = fopen (bccfil, "w")) == NULL)
	adios (bccfil, MSGSTR(NOCREATE, "unable to create %s"), bccfil); /*MSG*/
    (void)chmod (bccfil, 0600);

    fprintf (out, "Date: %s\n", dtimenow ());
    fprintf (out, "From: %s\n", signature);
    if (subject)
	fprintf (out, "Subject: %s", subject);
    fprintf (out, "BCC:\n\n------- Blind-Carbon-Copy\n\n");
    (void)fflush (out);

    if (filter == NULL) {
	if ((fd = open (tmpfil, 0)) == NOTOK)
	    adios (NULLCP, MSGSTR(NOREOPEN2, "unable to re-open")); /*MSG*/
	cpydgst (fd, fileno (out), tmpfil, bccfil);
	close (fd);
    }
    else {
	vec[0] = r1bindex (mhlproc, '/');

	for (i = 0; (child_id = vfork ()) == NOTOK && i < 5; i++)
	    sleep (5);
	switch (child_id) {
	    case NOTOK: 
		adios ("vfork", MSGSTR(NOVFORK, "unable to vfork")); /*MSG*/

	    case OK: 
		dup2 (fileno (out), 1);

		i = 1;
		vec[i++] = "-forward";
		vec[i++] = "-form";
		vec[i++] = filter;
		vec[i++] = tmpfil;
		vec[i] = NULL;

		execvp (mhlproc, vec);
		adios (mhlproc, MSGSTR(UNEXEC, "unable to exec %s"), mhlproc); /*MSG*/

	    default: 
		if (status = pidwait (child_id, OK))
		    admonish (NULL, MSGSTR(LOST, "%s lost (status=0%o)"), vec[0], status); /*MSG*/
		break;
	}
    }

    fseek (out, 0L, 2);
    fprintf (out, "\n------- End of Blind-Carbon-Copy\n");
    (void)fclose (out);
}
#endif notdef

/*    FCC INTERACTION */

static  file (char *path)
{
    int     i;

    if (fccind == 0)
	return;

    for (i = 0; i < fccind; i++)
	if (whomflg)
	    printf ("Fcc: %s\n", fccfold[i]);
	else
	    fcc (path, fccfold[i]);
}


static fcc (char *file, char *folder)
{
    int     i,
            child_id,
            status;
    char    fold[BUFSIZ];

    if (verbose) {
	if (msgstate == resent)
	    printf ("Resent-Fcc: %s\n", folder);
	else
	    printf ("Fcc: %s\n", folder);
    }
    (void)fflush (stdout);

    for (i = 0; (child_id = vfork ()) == NOTOK && i < 5; i++)
	sleep (5);
    switch (child_id) {
	case NOTOK: 
	    if (!verbose) {
		if (msgstate == resent)
		    fprintf (stderr, "  Resent-Fcc %s: ", folder);
		else
		    fprintf (stderr, "  Fcc %s: ", folder);
	    }
	    fprintf (verbose ? stdout : stderr, MSGSTR(NOFORKS, "no forks, so not ok\n")); /*MSG*/
	    break;

	case OK: 
	    (void)sprintf (fold, "%s%s",
		    *folder == '+' || *folder == '@' ? "" : "+", folder);
	    execlp (fileproc, r1bindex (fileproc, '/'),
		    "-link", "-file", file, fold, NULL);
	    _exit (-1);

	default: 
	    if (status = pidwait (child_id)) {
		if (!verbose) {
		    if (msgstate == resent)
		        fprintf (stderr, "  Resent-Fcc %s: ", folder);
		    else
		        fprintf (stderr, "  Fcc %s: ", folder);
	        }
		fprintf (verbose ? stdout : stderr,
			MSGSTR(ERRORED, " errored (0%o)\n"), status); /*MSG*/
	    }
    }

    (void)fflush (stdout);
}

/*    TERMINATION */

/* VARARGS2 */

static die (what, fmt, a, b, c, d)
char   *what,
       *fmt,
       *a,
       *b,
       *c,
       *d;
{
    adios (what, fmt, a, b, c, d);
}

#if defined(X400)
static
strcmpX( str1 , str2 )
char    *str1,
        *str2;
{
    while (tolower(*str1) == tolower(*str2++))
	if (*str1++ == '\0')
	    return(EQUAL);
    return(NOTEQUAL);
}
#endif
