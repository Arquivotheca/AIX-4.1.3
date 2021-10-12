static char sccsid[] = "@(#)14	1.9  src/bos/usr/bin/mh/zotnet/mts.c, cmdmh, bos411, 9428A410j 2/2/93 11:31:22";
/* 
 * COMPONENT_NAME: CMDMH mts.c
 * 
 * FUNCTIONS: LocalName, SystemName, UucpChan, dup2, dup2aux, 
 *            getfullname, gethostname, getusr, grot, mts_init, 
 *            tailor_value 
 *
 * ORIGINS: 26  27  28  35 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* mts.c - definitions for the mail transport system */

/* LINTLIBRARY */

#undef	NETWORK
#if	defined(BSD41A) || defined(BSD42)
#define	NETWORK
#endif	not (defined(BSD41A) || defined(BSD42))

#include "strings.h"
#include <ctype.h>
#include <stdio.h>
#include "mts.h"
#ifdef	NETWORK
#ifdef	BSD42
#include <netdb.h>
#endif	BSD42
#else	not NETWORK
#ifndef SYS5
#include <whoami.h>
#else SYS5
#include <sys/utsname.h>
#include <netdb.h>
#endif SYS5
#endif	not NETWORK
#include <pwd.h>


#define	NOTOK	(-1)
#define	OK	0



#define	NULLCP	((char *) 0)

extern int  errno;

static char   *tailor_value ();


#ifdef	SYS5
#define	index	strchr
#define	rindex	strrchr
#endif	SYS5

/*
char   *index (), *malloc (), *mktemp (), *rindex (), *strcpy ();
*/

/* The following declaration is already in pwd.h */
/* struct passwd  *getpwuid (); */

/*  */

/*
   *mmdfldir and *uucpldir are the maildrop directories.  If maildrops
   are kept in the user's home directory, then these should be empty
   strings.  In this case, the appropriate ...lfil array should contain
   the name of the file in the user's home directory.  Usually, this is
   something like ".mail".
 */

static char *mtstailor = "/usr/lib/mh/mtstailor";

static char    *localname = "";
static char    *systemname = "";
#ifdef	MF
static char    *UUCPchan = "";
#endif	MF
char    *mmdfldir = "/usr/spool/mail";
char    *mmdflfil = "";
char    *uucpldir = "/usr/spool/mail";
char    *uucplfil = "";


char    *mmdlm1 = "\001\001\001\001\n";
char    *mmdlm2 = "\001\001\001\001\n";


static int  MMailids = 0;
static char *mmailid = "0";


#ifdef	MF
char   *umincproc = "/usr/lib/mh/uminc";
#else	MF
char   *umincproc = NULL;
#endif	MF


int	lockstyle = LOK_UNIX;
static char *lkstyle = "0";
char   *lockldir = "/etc/locks";

/*  */

/* MTS specific variables */

#ifdef	MHMTS
char   *Mailqdir = "/usr/spool/netmail";
char   *TMailqdir = "/usr/tmp";
int     Syscpy = 1;
static char *syscpy = "1";
char   *Overseer = "root";
char   *Mailer = "root";
char   *Fromtmp = "/tmp/rml.f.XXXXXX";
char   *Msgtmp = "/tmp/rml.m.XXXXXX";
char   *Errtmp = "/tmp/rml.e.XXXXXX";
int     Tmpmode = 0600;
static char *tmpmode = "0600";
char   *Okhosts = "/usr/lib/mh/Rmail.OkHosts";
char   *Okdests = "/usr/lib/mh/Rmail.OkDests";
#endif	MHMTS

#ifdef	MMDFMTS
#endif	MMDFMTS

#ifdef	SENDMTS
char   *hostable = "/usr/lib/mh/hosts";
char   *sendmail = "/usr/lib/sendmail";
#endif	SENDMTS


/* SMTP/POP stuff */

char   *servers = "localhost \01localnet";
char   *pophost = "";


/* BBoards-specific variables */

char   *bb_domain = "";


/* POP BBoards-specific variables */

#ifdef	BPOP
char    *popbbhost = "";
char    *popbbuser = "";
char    *popbblist = "/usr/lib/mh/hosts.popbb";
#endif	BPOP


/* MailDelivery */

char   *maildelivery = "/usr/lib/mh/maildelivery";


/* Aliasing Facility (doesn't belong here) */

int	Everyone = NOTOK;
static char *everyone = "-1";
char   *NoShell = "";

/*  */

/* customize the MTS settings for MH by reading /usr/lib/mh/mtstailor */

static  struct bind {
    char   *keyword;
    char  **value;
}       binds[] = {
    "localname", &localname,
    "systemname", &systemname,
#ifdef	MF
    "uucpchan", &UUCPchan,
#endif	MF
    "mmdfldir", &mmdfldir,
    "mmdflfil", &mmdflfil,
    "uucpldir", &uucpldir,
    "uucplfil", &uucplfil,
    "mmdelim1", &mmdlm1,
    "mmdelim2", &mmdlm2,
    "mmailid", &mmailid,
    "umincproc", &umincproc,
    "lockstyle", &lkstyle,
    "lockldir", &lockldir,

#ifdef	MHMTS
    "mailqdir", &Mailqdir,
    "tmailqdir", &TMailqdir,
    "syscpy", &syscpy,
    "overseer", &Overseer,
    "mailer", &Mailer,
    "fromtmp", &Fromtmp,
    "msgtmp", &Msgtmp,
    "errtmp", &Errtmp,
    "tmpmode", &tmpmode,
    "okhosts", &Okhosts,
    "okdests", &Okdests,
#endif	MHMTS

#ifdef	MMDFMTS
#endif	MMDFMTS

#ifdef	SENDMTS
    "hostable", &hostable,
    "sendmail", &sendmail,
#endif	SENDMTS

    "servers", &servers,
    "pophost", &pophost,

    "bbdomain", &bb_domain,

#ifdef	BPOP
    "popbbhost", &popbbhost,
    "popbbuser", &popbbuser,
    "popbblist", &popbblist,
#endif	BPOP

    "maildelivery", &maildelivery,

    "everyone", &everyone,
    "noshell", &NoShell,

    NULL
};

/*  */


/* I'd like to use m_getfld() here, but not all programs loading mts.o may be
   MH-style programs... */

/* ARGSUSED */

mts_init (name)
char    *name;
{
    register char  *bp,
                   *cp;
    char    buffer[BUFSIZ];
    register struct bind   *b;
    register    FILE *fp;
    static int  inited = 0;

    if (inited++ || (fp = fopen (mtstailor, "r")) == NULL)
	return;

    while (fgets (buffer, sizeof buffer, fp)) {
	if ((cp = index (buffer, '\n')) == NULL)
	    break;
	*cp = (char)NULL;
	if (*buffer == '#' || *buffer == (char)NULL)
	    continue;
	if ((bp = index (buffer, ':')) == NULL)
	    break;
	*bp++ = (char)NULL;
	while (isspace (*bp))
	    *bp++ = (char)NULL;

	for (b = binds; b -> keyword; b++)
	    if (strcmp (buffer, b -> keyword) == 0)
		break;
	if (b -> keyword && (cp = tailor_value (bp)))
	    *b -> value = cp;
    }

    (void) fclose (fp);

    MMailids = atoi (mmailid);
    if ((lockstyle = atoi (lkstyle)) < LOK_UNIX || lockstyle > LOK_MMDF)
	lockstyle = LOK_UNIX;
#ifdef	MHMTS
    Syscpy = atoi (syscpy);
    (void) sscanf (tmpmode, "0%o", &Tmpmode);
#endif	MHMTS
    Everyone = atoi (everyone);
}

/*  */

#define	QUOTE	'\\'
/* macro does not expand properly. Until cpp works properly we'll work around it (P40235)
   Also see P11584 for a little history on this problem.
#define	grot(x)	case 'x': *bp = '\x'; break */

static char *tailor_value (s)
register char   *s;
{
    register int    i,
                    r;
    register char  *bp;
    char    buffer[BUFSIZ];

    for (bp = buffer; *s; bp++, s++)
	if (*s != QUOTE)
	    *bp = *s;
	else
	    switch (*++s) {
/* Old way ---- this should work but doesn't
		grot (b);
		grot (f);
		grot (n);
		grot (t);
   Let's do this for now */
		case 'b': *bp++ = '\b'; break;
		case 'f': *bp++ = '\f'; break;
		case 'n': *bp++ = '\n'; break;
		case 't': *bp++ = '\t'; break;

		case '\0': s--;
		case QUOTE: 
		    *bp = QUOTE;
		    break;

		default: 
		    if (!isdigit (*s)) {
			*bp++ = QUOTE;
			*bp = *s;
		    }
		    r = *s != '0' ? 10 : 8;
		    for (i = 0; isdigit (*s); s++)
			i = i * r + *s - '0';
		    s--;
		    *bp = toascii (i);
		    break;
	    }
    *bp = (char)NULL;

    bp = malloc ((unsigned) (strlen (buffer) + 1));
    if (bp != NULL)
	(void) strcpy (bp, buffer);

    return bp;
}

/*  */

char   *LocalName () {
#ifdef	BSD41A
    char  *myname;
#endif	BSD41A
#ifdef	BSD42
    register struct hostent *hp;
#endif	BSD42
#ifdef	SYS5
/* In release 2.2.1, we have both a inet name space (in array 
*  hostname) and a uucp node name (in uts.nodename). We will preserve
*  the SYS5 code for loading nodename as it will be needed for
*  uucp domains & do a gethostbyname for Localname.
*/
/*    struct utsname name; */
    register struct hostent *hp;
#endif	SYS5
    static char buffer[BUFSIZ] = "";

    if (buffer[0])
	return buffer;

    mts_init ("mts");
    if (*localname)
	return (char *)strcpy (buffer, localname);

#ifdef	locname
    (void) strcpy (buffer, locname);
#else	not locname
#ifdef	NETWORK
#ifdef	BSD41A
    myname = "myname";
    if (rhost (&myname) == -1)
	(void) gethostname (buffer, sizeof buffer);
    else {
	(void) strcpy (buffer, myname);
	free (myname);
    }
#endif	BSD41A
#ifdef	BSD42
    (void) gethostname (buffer, sizeof buffer);
    sethostent (1);
    if (hp = gethostbyname (buffer))
	(void) strcpy (buffer, hp -> h_name);
#endif	BSD42
#else	not NETWORK
#ifndef	SYS5
    (void) strcpy (buffer, SystemName ());
#else	SYS5
/*    (void) uname (&name);
*     (void) strcpy (buffer, name.nodename);
*/
    (void) gethostname (buffer, sizeof buffer);
    sethostent (1);
    if (hp = gethostbyname (buffer))
	(void) strcpy (buffer, hp -> h_name);
#endif	SYS5
#endif	not NETWORK
#endif	not locname

    return buffer;
}

/*  */

char *SystemName () {
#ifdef	SYS5
    struct utsname name;
#endif	SYS5
    static char buffer[BUFSIZ] = "";

    if (buffer[0])
	return buffer;

    mts_init ("mts");
    if (*systemname)
	return (char *)strcpy (buffer, systemname);

#ifdef	sysname
    (void) strcpy (buffer, sysname);
#else	sysname
#ifndef	SYS5
    (void) gethostname (buffer, sizeof buffer);
#else   SYS5
    (void) uname (&name);
    (void) strcpy (buffer, name.nodename);
#endif  SYS5
#endif	sysname

    return buffer;
}

/*  */

char   *UucpChan () {
#ifdef	MF
    static char buffer[BUFSIZ] = "";
#endif	MF

#ifndef	MF
    return NULL;
#else	MF
    if (buffer[0])
	return buffer;

    mts_init ("mts");
    if (*UUCPchan)
	return strcpy (buffer, UUCPchan);

#ifdef	uucpchan
    (void) strcpy (buffer, uucpchan);
#else	uucpchan
    (void) strcpy (buffer, "uucp");
#endif	uucpchan
    return buffer;
#endif	MF
}

/*  */

#ifdef	ALTOS
gethostname (name, len)
register char   *name;
register int     len;
{
    register char  *cp;
    register FILE  *fp;

    if (fp = fopen ("/etc/systemid", "r")) {
	if (fgets (name, len, fp)) {
	    if (cp = index (name, '\n'))
		*cp = NULL;
	    (void) fclose (fp);
	    return OK;
	}
	(void) fclose (fp);
    }
    (void) strncpy (name, "altos", len);

    return OK;
}
#endif	ALTOS

/*  */

static char username[BUFSIZ] = "";
static char fullname[BUFSIZ] = "";

char   *getusr () {
	register char  *cp,
		       *np;
	register struct passwd *pw;

	if (username[0])
		return username;

	if ((pw = getpwuid (getuid ())) == NULL
		|| pw -> pw_name == NULL
		|| *pw -> pw_name == NULL) {
		(void) strcpy (username, "unknown");
		(void) sprintf (fullname, "The Unknown User-ID (%d)", getuid ());
		return username;
	}

	np = pw -> pw_gecos;
	for (cp = fullname; *np && *np != (MMailids ? '<' : ','); *cp++ = *np++)
		continue;
	*cp = NULL;
	if (MMailids) {
		if (*np)
		np++;
		for (cp = username; *np && *np != '>'; *cp++ = *np++)
			continue;
		*cp = NULL;
	}
	if (MMailids == 0 || *np == NULL)
		(void) strcpy (username, pw -> pw_name);

	if ((cp = getenv ("SIGNATURE")) && *cp)
		(void) strcpy (fullname, cp);
	if (index(fullname, '.')) {         /*  quote any .'s */
		char tmp[BUFSIZ];
		sprintf (tmp, "\"%s\"", fullname);/* should quote "'s too */
		strcpy (fullname, tmp);
	}

	return username;
}



char   *getfullname () {
    if (username[0] == (char)NULL)
	(void) getusr ();

    return fullname;
}

/*  */

#ifdef	SYS5
#ifndef	notdef			/* Supposedly this works, I prefer the
				   recursive solution... */

#include <fcntl.h>

int     dup2 (d1, d2)
register int    d1,
                d2;
{
    int     d;

    if (d1 == d2)
	return OK;

    (void) close (d2);
    if ((d = fcntl (d1, F_DUPFD, d2)) == NOTOK)
	return NOTOK;
    if (d == d2)
	return OK;

    errno = 0;
    return NOTOK;
}

#else	notdef
int     dup2 (d1, d2)
register int    d1,
                d2;
{
    if (d1 == d2)
	return OK;

    (void) close (d2);
    return dup2aux (d1, d2);
}


static int  dup2aux (d1, d2)
register int    d1,
                d2;
{
    int     d,
            i,
            eindex;

    if ((d = dup (d1)) == NOTOK)
	return NOTOK;
    if (d == d2)
	return OK;

    i = dup2aux (d1, d2);
    eindex = errno;
    (void) close (d);
    errno = eindex;
    return i;
}
#endif	notdef
#endif	SYS5
