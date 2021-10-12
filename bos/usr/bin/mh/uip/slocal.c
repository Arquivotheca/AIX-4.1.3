static char sccsid[] = "@(#)89	1.15  src/bos/usr/bin/mh/uip/slocal.c, cmdmh, bos411, 9428A410j 11/9/93 09:44:58";
/* 
 * COMPONENT_NAME: CMDMH slocal.c
 * 
 * FUNCTIONS: MSGSTR, Mslocal, adorn, alrmser, copyfile, copyinfo, 
 *            expand, glob, localmail, logged_in, lookup, matches, 
 *            parse, split, timely, usr_delivery, usr_file, usr_hook, 
 *            usr_pipe 
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
/* static char sccsid[] = "slocal.c	8.1 88/04/15 15:50:43"; */

/* slocal.c - MH style mailer to write to a local user's mailbox */

/* This program implements mail delivery in the MH/MMDF style.

   Under SendMail, users should add the line

	"| /usr/lib/mh/slocal"

   to their $HOME/.forward file.

   Under MMDF-I, users should (symbolically) link /usr/lib/mh/slocal
   to $HOME/bin/rcvmail.

   Under stand-alone MH, post will automatically run this during local
   delivery.

   This program should be used ONLY if you have "mts sendmail" or "mts mh"
   or "mts mmdf1" set in your MH configuration.
 */

/*  */

#include "mh.h"
#include "dropsbr.h"
#include "rcvmail.h"
#include "tws.h"
#include "mts.h"
#include <pwd.h>
#include <signal.h>
#ifndef	V7
#include <sys/ioctl.h>
#endif	not V7
#include <sys/stat.h>
#include <utmp.h>
#include <grp.h>
#include <sys/id.h>
#include <sys/priv.h>

#include "mh_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 


#define	NVEC	100

#define MBOXMODE 0660  /* really should be elsewhere, but where? */

/*  */

static struct swit switches[] = {
/*
*#define	ADDRSW	0
*    "addr address", 0,
*#define	USERSW	1
*    "user name", 0,
*#define	FILESW	2
*    "file file", 0,
*#define	SENDSW	3
*    "sender address", 0,
*#define	MBOXSW	4
*    "mailbox file", 0,
*#define	HOMESW	5
*    "home directory", 0,
*
*#define	MAILSW	6
*    "maildelivery file", 0,
*/
#define	VERBSW	0
    "verbose", 0,
#define	NVERBSW	1
    "noverbose", 0,

#define	DEBUGSW	2
    "debug", 0,

#define	HELPSW	3 
    "help", 4,

    NULL, (int)NULL
};

/*  */

static int  debug = 0;
static int  globbed = 0;
static int  parsed = 0;
static int  utmped = 0;
static int  verbose = 0;
#ifdef _AIX
static int  mailgid;
#endif _AIX

static char *addr = NULLCP;
static char *user = NULLCP;
static char *info = NULLCP;
static char *file = NULLCP;
static char *sender = NULLCP;
static char *mbox = NULLCP;
static char *home = NULLCP;


static struct passwd *pw;


static char ddate[BUFSIZ];

struct tws *now;


static jmp_buf myctx;

/*  */

static struct pair {
    char   *p_name;
    char   *p_value;

    char    p_flags;
#define	P_NIL	0x00
#define	P_ADR	0x01
#define	P_HID	0x02
#define	P_CHK	0x04
};



static struct pair  hdrs[NVEC + 1] = {
    "source", NULL, P_HID,
    "addr", NULL, P_HID,

    "Return-Path", NULL, P_ADR,
    "Reply-To", NULL, P_ADR,
    "From", NULL, P_ADR,
    "Sender", NULL, P_ADR,
    "To", NULL, P_ADR,
    "cc", NULL, P_ADR,
    "Resent-Reply-To", NULL, P_ADR,
    "Resent-From", NULL, P_ADR,
    "Resent-Sender", NULL, P_ADR,
    "Resent-To", NULL, P_ADR,
    "Resent-cc", NULL, P_ADR,

    NULL
};


static struct pair  vars[] = {
    "sender", NULL, P_NIL,
    "address", NULL, P_NIL,
    "size", NULL, P_NIL,
    "reply-to", NULL, P_CHK,
    "info", NULL, P_NIL,

    NULL
};

/*  */

extern char **environ;

static void	alrmser (int);
static int  localmail (int, char *, char *);
static int  usr_delivery (int, char *, int);
static int  split (char *, char **);
static int  parse (register int), logged_in();
static  expand (register char *, register char *, int);
static        glob (register int);
static struct pair *lookup (register struct pair *, register char *);
static int  timely (char *, char *);
static int  usr_file (int, char *, char *);
static int  usr_pipe (int, char *, char *, char **);
static        copyinfo (register FILE *, char *);
static int  copyfile (int , register char *, int);
static void  adorn (char *, char *, char *, char *,
                    char *, char *, char *, char *);


char *index();

long    lseek ();
#ifdef	SYS5
struct passwd *getpwnam ();
#endif	SYS5

/*  */

/* ARGSUSED */

main (argc, argv, envp)
int     argc;
char  **argv,
      **envp;
{
    int     fd;
    FILE   *fp    = stdin;
    struct group *gp;
    char   *cp,
	   *mdlvr = NULL,
            buf[100],
            from[BUFSIZ],
            mailbox[BUFSIZ],
            tmpfil[BUFSIZ],
          **argp = argv + 1;

    setlocale(LC_ALL,"");
    catd = catopen(MF_MH, NL_CAT_LOCALE);

    invo_name = r1bindex (*argv, '/');

#ifdef _AIX
    if (initprivs() < 0) {
	adios(invo_name, "initprivs()");
    }
#endif _AIX

    m_foil (NULLCP);
    mts_init (invo_name);

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
		    (void) sprintf (buf, MSGSTR(HELPSW4, "%s [switches] [address info sender]"), invo_name); /*MSG*/
		    help (buf, switches);
		    done (1);

/*
*		case ADDRSW: 
*		    if (!(addr = *argp++))
*		adios (NULLCP, "missing argument to %s", argp[-2]);
*		    continue;
*		case USERSW: 
*		    if (!(user = *argp++))
*		adios (NULLCP, "missing argument to %s", argp[-2]);
*		    continue;
*		case FILESW: 
*		    if (!(file = *argp++) || *file == '-')
*		adios (NULLCP, "missing argument to %s", argp[-2]);
*		    continue;
*		case SENDSW: 
*		    if (!(sender = *argp++))
*		adios (NULLCP, "missing argument to %s", argp[-2]);
*		    continue;
*		case MBOXSW: 
*		    if (!(mbox = *argp++) || *mbox == '-')
*		adios (NULLCP, "missing argument to %s", argp[-2]);
*		    continue;
*		case HOMESW: 
*		    if (!(home = *argp++) || *home == '-')
*		adios (NULLCP, "missing argument to %s", argp[-2]);
*		    continue;
*
*		case MAILSW: 
*		    if (!(cp = *argp++) || *cp == '-')
*		adios (NULLCP, "missing argument to %s", argp[-2]);
*		    if (mdlvr)
*		adios (NULLCP, "only one maildelivery file at a time!");
*		    mdlvr = cp;
*		    continue;
*/
		case VERBSW: 
		    verbose++;
		    continue;
		case NVERBSW: 
		    verbose = 0;
		    continue;

		case DEBUGSW: 
		    debug++;
		    continue;
	    }
/*
*	switch (argp - (argv + 1)) {
*	    case 1: 
*		addr = cp;
*		break;
*
*	    case 2: 
*		info = cp;
*		break;
*
*	    case 3: 
*		sender = cp;
*		break;
*	}
*/
    }

/*  */

#ifdef _AIX
    if ((gp = getgrnam("mail")) == NULL)
	adios(invo_name, "getgrnam(mail)");
    mailgid = gp->gr_gid;
#endif _AIX

    if (addr == NULL)
	addr = getusr ();
    if (user == NULL)
	user = (cp = index (addr, '.')) ? ++cp : addr;
    if ((pw = getpwnam (user)) == NULL)
	adios (NULLCP, MSGSTR(NOSUCHUSER, "no such local user as %s"), user); /*MSG*/

    if (chdir (pw -> pw_dir) == NOTOK)
	(void) chdir ("/");
    (void) umask (0077);

    if (geteuid () == 0) {
#ifdef	BSD41A
	(void) inigrp (pw -> pw_name, pw -> pw_gid);
#endif	BSD41A
	(void) setgid (pw -> pw_gid);
#ifdef	BSD42
	(void) initgroups (pw -> pw_name, pw -> pw_gid);
#endif	BSD42
	(void) setuid (pw -> pw_uid);
    }
#ifdef _AIX
    mts_init (invo_name);
#endif _AIX
    
    if (info == NULL)
	info = "";

    setbuf (stdin, NULLCP);

    if (file == NULL) {
	if ((fd = copyfile (fileno (stdin), file = tmpfil, 1)) == NOTOK)
	    adios (NULLCP, MSGSTR(NOTMPF, "unable to create temporary file")); /*MSG*/
	if (debug)
	    fprintf (stderr, "temporary file \"%s\" selected\n", tmpfil);
	else
	    (void) unlink (tmpfil);
   	if ((fp = fdopen (fd, "r+")) == NULL)
	    adios (NULLCP, MSGSTR(NOATMPFILE, "unable to access temporary file")); /*MSG*/
    }
    else
	fd = fileno (stdin);

    from[0] = (char)NULL;
    if (sender == NULL)
	copyinfo (fp, from);


    if (mbox == NULL) {
	(void) sprintf (mailbox, "%s/%s",
		mmdfldir[0] ? mmdfldir : pw -> pw_dir,
		mmdflfil[0] ? mmdflfil : pw -> pw_name);
	mbox = mailbox;
    }
    if (home == NULL)
	home = pw -> pw_dir;

    if ((now = dtwstime ()) == NULL)
	adios (NULLCP, MSGSTR(NOTIME, "unable to ascertain local time")); /*MSG*/
    (void) sprintf (ddate, "Delivery-Date: %s\n", dtimenow ());

    if (debug) {
	fprintf (stderr, "addr=\"%s\" user=\"%s\" info=\"%s\" file=\"%s\"\n",
		addr, user, info, file);
	fprintf (stderr, "sender=\"%s\" mbox=\"%s\" home=\"%s\" from=\"%s\"\n",
		sender, mbox, home, from);
	fprintf (stderr, "ddate=\"%s\" now=%02d:%02d\n",
		ddate, now -> tw_hour, now -> tw_min);
    }

    done (localmail (fd, from, mdlvr) != NOTOK ? RCV_MOK : RCV_MBX);
}

/*  */

#ifdef _AIX
#define ADD_PRIVILEGE(v,p)	(((p) <= 32) ? \
		(v.pv_priv[0] |= (1<<((p) - 1))):\
		(v.pv_priv[1] |= (1<<((p) - 33))))

/*
 * This was adapted from cmd/mailx/bellmail.c.
 * It sets the uids to the invoker and removes all privileges except for the
 * privilege to do a chown (SET_OBJ_DAC) and setuid/gid (SET_PROC_DAC).
 * These privileges are kept dormant until needed.
 */
static int
initprivs()
{
    priv_t priv = { 0, 0 };
    int rc;

    if (setuidx(ID_EFFECTIVE | ID_REAL | ID_SAVED, getuid()) < 0)
	return(-1);
    ADD_PRIVILEGE(priv, SET_OBJ_DAC);
    ADD_PRIVILEGE(priv, SET_PROC_DAC);
    rc = setpriv(PRIV_SET | PRIV_EFFECTIVE | PRIV_INHERITED | PRIV_BEQUEATH, 
	NULL, NULL);
    if (rc >= 0) {
	rc = setpriv(PRIV_SET | PRIV_MAXIMUM, &priv, sizeof(priv));
    }
    return(rc);
}

/*
 * If on is true, this sets our privs such that we can chown and
 * it sets the gid to the mail gid so that we can create the mailbox
 * with mail gid (note that this is used instead of root privs so
 * that it works over nfs).  If on is false, it resets the gid to
 * the original gid and resets the privs.
 */
static int
setprivs(on)
int on;
{
    priv_t priv = { 0, 0 };

    /* reset gid now before giving up privs */
    if (!on && setgid(pw->pw_gid) < 0)
	return(-1);
    if (on) {
	ADD_PRIVILEGE(priv, SET_OBJ_DAC);  /* need this for chown() */
	ADD_PRIVILEGE(priv, SET_PROC_DAC);  /* need this for setgid() */
    }
    if (setpriv(PRIV_SET | PRIV_EFFECTIVE, &priv, sizeof(priv)) < 0)
	return(-1);
    if (on)
	return(setgid(mailgid));
    return(0);
}
#endif _AIX

static int  localmail (int     fd,
                     char   *from,
                     char       *mdlvr)
{
    if (usr_delivery (fd, mdlvr ? mdlvr : ".maildelivery", 0) != NOTOK)
	return OK;

    if (usr_delivery (fd, maildelivery, 1) != NOTOK)
	return OK;

#ifdef	notdef
    if (verbose)
	printf (MSGSTR(HOOK, "(invoking hook)\n")); /*MSG*/
    if (usr_hook (fd, mbox) != NOTOK)
	return OK;
#endif	notdef

    if (verbose)
	printf (MSGSTR(NORMATPT, "(trying normal delivery)\n")); /*MSG*/
    return usr_file (fd, mbox, from);
}

/*  */

#define	matches(a,b)	(stringdex (b, a) >= 0)

static int  usr_delivery (int fd, char *delivery, int su)
{
    int     i,
	    accept,
            status,
            won,
	    vecp;
    register char  *cp,
                   *action,
                   *field,
                   *pattern,
		   *string;
    char    buffer[BUFSIZ],
	    tmpbuf[BUFSIZ],
           *vec[NVEC];
    struct stat st;
    register struct pair   *p;
    register FILE  *fp;

    if ((fp = fopen (delivery, "r")) == NULL)
	return NOTOK;
    if (fstat (fileno (fp), &st) == NOTOK
	    || (st.st_uid != 0 && (su || st.st_uid != pw -> pw_uid))
	    || st.st_mode & 0022) {
	if (verbose) {
	    printf (MSGSTR(BADSET, "%s: ownership/modes bad (%d, %d,%d,0%o)\n"), delivery, su, pw -> pw_uid, st.st_uid, st.st_mode); /*MSG*/
	    (void) fflush (stdout);
	}
	return NOTOK;
    }

    won = 0;
    while (fgets (buffer, sizeof buffer, fp) != NULL) {
	if (*buffer == '#')
	    continue;
	if (cp = index (buffer, '\n'))
	    *cp = (char)NULL;
	if ((vecp = split (buffer, vec)) < 5)
	    continue;
	if (debug)
	    for (i = 0; vec[i]; i++)
		fprintf (stderr, "vec[%d]: \"%s\"\n", i, vec[i]);

	field = vec[0];
	pattern = vec[1];
	action = vec[2];

	switch (vec[3][0]) {
	    case '?': 
		if (won)
		    continue;	/* else fall */
	    case 'A': 
	    case 'a': 
		accept = 1;
		break;

	    case 'R': 
	    case 'r': 
	    default: 
		accept = 0;
		break;
	}

	string = vec[4];

	if (vecp > 5) {
	    if (uleq (vec[5], "select")) {
		if (logged_in () != NOTOK)
		    continue;
		if (vecp > 7 && timely (vec[6], vec[7]) == NOTOK)
		    continue;
	    }
	}

	switch (*field) {
	    case '*': 
		break;

	    case 'd': 
		if (uleq (field, "default")) {
		    if (won)
			continue;
		    break;
		}		/* else fall */

	    default: 
		if (!parsed && parse (fd) == NOTOK) {
		    (void) fclose (fp);
		    return NOTOK;
		}
		if ((p = lookup (hdrs, field)) == NULL
			|| !matches (p -> p_value, pattern))
		    continue;
		break;
	}

	switch (*action) {
	    case 'q':
		if (!uleq (action, "qpipe"))
		    continue;	/* else fall */
	    case '^':
		expand (tmpbuf, string, fd);
		if (split (tmpbuf, vec) < 1)
		    continue;
		status = usr_pipe (fd, tmpbuf, vec[0], vec);
		break;

	    case 'p': 
		if (!uleq (action, "pipe"))
		    continue;	/* else fall */
	    case '|': 
		vec[2] = "sh";
		vec[3] = "-c";
		expand (tmpbuf, string, fd);
		vec[4] = tmpbuf;
		vec[5] = NULL;
		status = usr_pipe (fd, tmpbuf, "/bin/sh", vec + 2);
		break;

	    case 'f': 
		if (!uleq (action, "file"))
		    continue;	/* else fall */
	    case '>': 
		status = usr_file (fd, string, NULLCP);
		break;

	    case 'd': 
		if (!uleq (action, "destroy"))
		    continue;
		status = OK;
		break;
	}

	if (accept) {
	    if (status == NOTOK) {
		won = 0;
		break;
	    }
	    won++;
	}
    }

    (void) fclose (fp);
    return (won ? OK : NOTOK);
}

/*  */

#define	QUOTE	'\\'
static int  split (char *cp, char **vec)
{
    register int    i;
    register char  *s;

    for (i = 0, s = cp; i <= NVEC;) {
	vec[i] = NULL;
	while (isspace (*s) || *s == ',')
	    *s++ = (char)NULL;
	if (*s == (char)NULL)
	    break;

	if (*s == '"') {
	    for (vec[i++] = ++s; *s != (char)NULL && *s != '"'; s++)
		if (*s == QUOTE) {
		    if (*++s == '"')
			(void) strcpy (s - 1, s);
		    s--;
		}
	    if (*s == '"')
		*s++ = (char)NULL;
	    continue;
	}
	if (*s == QUOTE && *++s != '"')
	    s--;
	vec[i++] = s++;

	while (*s != (char)NULL && !isspace (*s) && *s != ',')
	    s++;
    }
    vec[i] = NULL;

    return i;
}

/*  */

static int  parse (register int fd)
{
    register int    i,
                    state;
    int     fd1;
    register char  *cp,
                   *dp,
                   *lp;
    char    name[NAMESZ],
            field[BUFSIZ];
    register struct pair   *p,
			   *q;
    register FILE  *in;

    if (parsed++)
	return OK;

    if ((fd1 = dup (fd)) == NOTOK)
	return NOTOK;
    if ((in = fdopen (fd1, "r")) == NULL) {
	(void) close (fd1);
	return NOTOK;
    }
    rewind (in);

    if (p = lookup (hdrs, "source"))
	p -> p_value = getcpy (sender);
    if (p = lookup (hdrs, "addr"))
	p -> p_value = getcpy (addr);

    for (i = 0, state = FLD;;) {
	switch (state = m_getfld (state, name, field, sizeof field, in)) {
	    case FLD: 
	    case FLDEOF: 
	    case FLDPLUS: 
		lp = add (field, NULLCP);
		while (state == FLDPLUS) {
		    state = m_getfld (state, name, field, sizeof field, in);
		    lp = add (field, lp);
		}
		for (p = hdrs; p -> p_name; p++)
		    if (uleq (p -> p_name, name)) {
			if (!(p -> p_flags & P_HID)) {
			    if (cp = p -> p_value)
				if (p -> p_flags & P_ADR) {
				    dp = cp + strlen (cp) - 1;
				    if (*dp == '\n')
					*dp = (char)NULL;
				    cp = add (",\n\t", cp);
				}
				else
				    cp = add ("\t", cp);
			    p -> p_value = add (lp, cp);
			}
			free (lp);
			break;
		    }
		if (p -> p_name == NULL && i < NVEC) {
		    p -> p_name = getcpy (name);
		    p -> p_value = lp;
		    p -> p_flags = P_NIL;
		    p++, i++;
		    p -> p_name = NULL;
		}
		if (state != FLDEOF)
		    continue;
		break;

	    case BODY: 
	    case BODYEOF: 
	    case FILEEOF: 
		break;

	    case LENERR: 
	    case FMTERR: 
		advise (NULLCP, MSGSTR(FRMTERR, "format error in message")); /*MSG*/
		break;

	    default: 
		advise (NULLCP, MSGSTR(INTERR, "internal error")); /*MSG*/
		(void) fclose (in);
		return NOTOK;
	}
	break;
    }
    (void) fclose (in);

    if (p = lookup (vars, "reply-to")) {
	if ((q = lookup (hdrs, "reply-to")) == NULL || q -> p_value == NULL)
	    q = lookup (hdrs, "from");
	p -> p_value = getcpy (q ? q -> p_value : "");
	p -> p_flags &= ~P_CHK;
	if (debug)
	    fprintf (stderr, "vars[%d]: name=\"%s\" value=\"%s\"\n",
		    p - vars, p -> p_name, p -> p_value);
    }
    if (debug)
	for (p = hdrs; p -> p_name; p++)
	    fprintf (stderr, "hdrs[%d]: name=\"%s\" value=\"%s\"\n",
		p - hdrs, p -> p_name, p -> p_value);

    return OK;
}

/*  */

#define	LPAREN	'('
#define	RPAREN	')'
static  expand (register char *s1, 
              register char *s2, 
              int fd)
{
    register char   c,
                   *cp;
    register struct pair   *p;

    if (!globbed)
	glob (fd);

    while (c = *s2++)
	if (c != '$' || *s2 != LPAREN)
	    *s1++ = c;
	else {
	    for (cp = ++s2; *s2 && *s2 != RPAREN; s2++)
		continue;
	    if (*s2 != RPAREN) {
		s2 = --cp;
		continue;
	    }
	    *s2++ = (char)NULL;
	    if (p = lookup (vars, cp)) {
		if (!parsed && (p -> p_flags & P_CHK))
		    (void) parse (fd);

		(void) strcpy (s1, p -> p_value);
		s1 += strlen (s1);
	    }
	}
    *s1 = (char)NULL;
}

/*  */

static	glob (register int fd)
{
    char buffer[BUFSIZ];
    struct stat st;
    register struct pair   *p;

    if (globbed++)
	return;

    if (p = lookup (vars, "sender"))
	p -> p_value = getcpy (sender);
    if (p = lookup (vars, "address"))
	p -> p_value = getcpy (addr);
    if (p = lookup (vars, "size")) {
	(void) sprintf (buffer, "%d",
		fstat (fd, &st) != NOTOK ? (int) st.st_size : 0);
	p -> p_value = getcpy (buffer);
    }
    if (p = lookup (vars, "info"))
	p -> p_value = getcpy (info);

    if (debug)
	for (p = vars; p -> p_name; p++)
	    fprintf (stderr, "vars[%d]: name=\"%s\" value=\"%s\"\n",
		    p - vars, p -> p_name, p -> p_value);
}

/*  */
static struct pair *lookup (register struct pair   *pairs,
                          register char  *key)
{
    register char  *cp;

    for (; cp = pairs -> p_name; pairs++)
	if (uleq (cp, key))
	    return pairs;

    return NULL;
}

/*  */

static int  logged_in () {
    struct utmp ut;
    register FILE  *uf;

    if (utmped)
	return utmped;

    if ((uf = fopen ("/etc/utmp", "r")) == NULL)
	return NOTOK;

    while (fread ((char *) &ut, sizeof ut, 1, uf) == 1)
	if (ut.ut_name[0] != (char)NULL
		&& strncmp (user, ut.ut_name, sizeof ut.ut_name) == 0) {
	    if (debug)
		continue;
	    (void) fclose (uf);
	    return (utmped = DONE);
	}

    (void) fclose (uf);
    return (utmped = NOTOK);
}


static int  timely (char *t1, char *t2)
{
#define	check(t,a,b)		if (t < a || t > b) return NOTOK
#define	cmpar(h1,m1,h2,m2)	if (h1 < h2 || (h1 == h2 && m1 < m2)) return OK

    int     t1hours,
            t1mins,
            t2hours,
            t2mins;

    if (sscanf (t1, "%d:%d", &t1hours, &t1mins) != 2)
	return NOTOK;
    check (t1hours, 0, 23);
    check (t1mins, 0, 59);

    if (sscanf (t2, "%d:%d", &t2hours, &t2mins) != 2)
	return NOTOK;
    check (t2hours, 0, 23);
    check (t2mins, 0, 59);

    cmpar (now -> tw_hour, now -> tw_min, t1hours, t1mins);
    cmpar (t2hours, t2mins, now -> tw_hour, now -> tw_min);

    return NOTOK;
}

/*  */

static int  usr_file (int fd, char *mailbox, char *from)
{
    int	    md,
	    mapping;
    register char  *bp;
    char    buffer[BUFSIZ];
    gid_t group;
    int mode;
#ifdef _AIX
    int privset = 0;
#endif _AIX

    if (verbose)
	printf (MSGSTR(DELIVERING, "\tdelivering to file \"%s\""), mailbox);
    if (from && *from) {
	(void) mbx_uucp ();
	if (verbose)
	    printf (MSGSTR(STYLE, " (uucp style)"));
	(void) sprintf (buffer, "%s%s", from, ddate);
	bp = buffer;
	mapping = 0;
    }
    else {
	bp = ddate;
	mapping = 1;
    }
    if (verbose)
	(void) fflush (stdout);

#ifdef _AIX
    /*
     * if this is the system mailbox we need to set group and mode
     * properly and acquire the privs to chown and create it.
     */
    if (*mmdfldir && (strncmp(mailbox, mmdfldir, strlen(mmdfldir)) == 0)) {
	group = mailgid;
	mode = MBOXMODE;
	if (setprivs(1) < 0) {
	    adorn("", "setprivs(1): ", "", "", "", "", "", "");
	    return(NOTOK);
	}
	privset = 1;
    } else
#endif _AIX
    {
	/*
	 * normal file: user's group, default mode, no privs.
	 */
	group = pw->pw_gid;
	mode = m_gmprot();
    }

    if ((md = mbx_open(mailbox, pw->pw_uid, group, mode)) == NOTOK) {
	adorn ("", MSGSTR(NOOPEN2, "unable to open:"),"","","","","","");
#ifdef _AIX
	/* reset privs */
	if (privset)
	    (void)setprivs(0);
#endif _AIX
	return NOTOK;
    }

#ifdef _AIX
    /* reset privs */
    if (privset && setprivs(0) < 0) {
	adorn("", "setprivs(0): ", "", "", "", "", "", "");
	return(NOTOK);
    }
#endif _AIX

    (void) lseek (fd, 0L, 0);
    if (mbx_copy (mailbox, md, fd, mapping, bp, verbose) == NOTOK) {
	adorn ("", MSGSTR(WERROR2, "error writing to:"),"","","","","","");
	return NOTOK;
    }

    (void) mbx_close (mailbox, md);
    if (verbose) {
	printf (", done.\n");
	(void) fflush (stdout);
    }
    return OK;
}

/*  */

#ifdef	notdef
static int  usr_hook (fd, mailbox)
int     fd;
char   *mailbox;
{
    int     i,
            vecp;
    char    receive[BUFSIZ],
            tmpfil[BUFSIZ],
           *vec[NVEC];

    if ((fd = copyfile (fd, tmpfil, 0)) == NOTOK) {
	if (verbose)
	    adorn (MSGSTR(NOCOPY, "unable to copy message; skipping hook\n"),"","","","","","",""); /*MSG*/
	return NOTOK;
    }
    (void) chown (tmpfil, pw -> pw_uid, pw -> pw_gid);

    vecp = 1;
    (void) sprintf (receive, "%s/.mh_receive", pw -> pw_dir);
    switch (access (receive, 01)) {
	case NOTOK: 
	    (void) sprintf (receive, "%s/bin/rcvmail", pw -> pw_dir);
	    if (access (receive, 01) == NOTOK) {
		(void) unlink (tmpfil);
		if (verbose) {
		    printf (MSGSTR(NOTTHERE, "\tnot present\n")); /*MSG*/
		    (void) fflush (stdout);
		}
		return NOTOK;
	    }
	    vec[vecp++] = addr;
	    vec[vecp++] = tmpfil;
	    vec[vecp++] = sender;
	    break;

	default: 
	    vec[vecp++] = tmpfil;
	    vec[vecp++] = mailbox;
	    vec[vecp++] = home;
	    vec[vecp++] = addr;
	    vec[vecp++] = sender;
	    break;
    }
    vec[0] = r1bindex (receive, '/');
    vec[vecp] = NULL;

    i = usr_pipe (fd, "rcvmail", receive, vec);
    (void) unlink (tmpfil);

    return i;
}
#endif	notdef

/*  */

static int  usr_pipe (int fd, char *cmd, char *pgm, char **vec)
{
    int     bytes,
	    i,
            child_id,
            status;
    struct stat st;

    if (verbose) {
	printf (MSGSTR(DELIVERING2, "\tdelivering to pipe \"%s\""), cmd); /*MSG*/
	(void) fflush (stdout);
    }
    (void) lseek (fd, 0L, 0);

    for (i = 0; (child_id = fork ()) == NOTOK && i < 5; i++)
	sleep (5);
    switch (child_id) {
	case NOTOK: 
	    adorn ("fork", MSGSTR(NOFORK, "unable to fork"),"","","","","",""); /*MSG*/
	    return NOTOK;

	case OK: 
	    if (fd != 0)
		(void) dup2 (fd, 0);
	    (void) freopen ("/dev/null", "w", stdout);
	    (void) freopen ("/dev/null", "w", stderr);
	    if (fd != 3)
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
	    (void) putenv ("USER", pw -> pw_name);
	    (void) putenv ("HOME", pw -> pw_dir);
	    (void) putenv ("SHELL", pw -> pw_shell);

	    execvp (pgm, vec);
	    _exit (-1);

	default: 
	    switch (setjmp (myctx)) {
		case OK: 
		    (void) signal (SIGALRM, (void(*)(int))alrmser);
		    bytes = fstat (fd, &st) != NOTOK ? (int) st.st_size : 100;
		    if (bytes <= 0)
			bytes = 100;
		    (void) alarm ((unsigned) (bytes * 60 + 300));

		    status = pidwait (child_id, OK);

		    (void) alarm (0);
#ifdef	MMDFI
		    if (status == RP_MOK || status == RP_OK)
			status = 0;
#endif	MMDFI
		    if (verbose) {
			if (status == 0)
			    printf (MSGSTR(WINS, ", wins.\n")); /*MSG*/
			else
			    if ((status & 0xff00) == 0xff00)
				printf (MSGSTR(SYSERR, ", system error\n")); /*MSG*/
			    else
				(void) pidstatus (status, stdout, MSGSTR(LOSES2, ", loses")); /*MSG*/
			(void) fflush (stdout);
		    }
		    return (status == 0 ? OK : NOTOK);

		default: 
#ifndef	BSD42
		    (void) kill (child_id, SIGKILL);
#else	BSD42
		    (void) killpg (child_id, SIGKILL);
#endif	BSD42
		    if (verbose) {
			printf (MSGSTR(TERM, ", timed-out; terminated\n")); /*MSG*/
			(void) fflush (stdout);
		    }
		    return NOTOK;
	    }
    }
}

/*  */

/* ARGSUSED */

static	void alrmser (int i)
{
    longjmp (myctx, DONE);
}

/*  */

static	copyinfo (register FILE *fp, char *from)
{
    int     i;
    register char  *cp;
    static char buffer[BUFSIZ];

    if (fgets (from, BUFSIZ, fp) == NULL)
	adios (NULLCP, MSGSTR(NOMSG, "no message")); /*MSG*/

    if (strncmp (from, "From ", i = strlen ("From "))) {
	rewind (fp);
	*from = (char)NULL;
	return;
    }

    (void) strcpy (buffer, from + i);
    if (cp = index (buffer, '\n')) {
	*cp = (char)NULL;
	cp -= 24;
	if (cp < buffer)
	    cp = buffer;
    }
    else
	cp = buffer;
    *cp = (char)NULL;

    for (cp = buffer + strlen (buffer) - 1; cp >= buffer; cp--)
	if (isspace (*cp))
	    *cp = (char)NULL;
	else
	    break;
    sender = buffer;
    rewind (fp);
}

/*  */

static int  copyfile (int qd, register char *tmpfil, int fold)
{
    register int    i,
                    fd1,
                    fd2;
    char    buffer[BUFSIZ];
    register FILE  *qfp,
		   *ffp;

    (void) strcpy (tmpfil, m_tmpfil (invo_name));
    if ((fd1 = creat (tmpfil, 0600)) == NOTOK)
	return NOTOK;
    (void) close (fd1);
    if ((fd1 = open (tmpfil, 2)) == NOTOK)
	return NOTOK;

    if (!fold) {
	while ((i = read (qd, buffer, sizeof buffer)) > 0)
	    if (write (fd1, buffer, i) != i) {
you_lose: ;
		(void) close (fd1);
		(void) unlink (tmpfil);
		return NOTOK;
	    }
	if (i == NOTOK)
	    goto you_lose;
	(void) lseek (fd1, 0L, 0);
	return fd1;
    }

    if ((fd2 = dup (qd)) == NOTOK) {
	(void) close (fd1);
	return NOTOK;
    }
    if ((qfp = fdopen (fd2, "r")) == NULL) {
	(void) close (fd1);
	(void) close (fd2);
	return NOTOK;
    }

    if ((fd2 = dup (fd1)) == NOTOK) {
	(void) close (fd1);
	(void) fclose (qfp);
	return NOTOK;
    }
    if ((ffp = fdopen (fd2, "r+")) == NULL) {
	(void) close (fd1);
	(void) close (fd2);
	(void) fclose (qfp);
	return NOTOK;
    }

    i = strlen ("From ");
    while (fgets (buffer, sizeof buffer, qfp)) {
	if (!strncmp (buffer, "From ", i))
	    continue;
	fputs (buffer, ffp);
	if (ferror (ffp)) {
	    (void) close (fd1);
	    (void) fclose (ffp);
	    (void) fclose (qfp);
	    return NOTOK;
	}
    }

    (void) fclose (ffp);
    if (ferror (qfp)) {
	(void) close (fd1);
	(void) fclose (qfp);
	return NOTOK;
    }
    (void) fclose (qfp);

    (void) lseek (fd1, 0L, 0);

    return fd1;
}

/*  */

/* VARARGS2 */

static void  adorn (char *what, char *fmt, char *a, char *b,
                    char *c, char *d, char *e, char *f)
{
    char   *cp = invo_name;

    if (!verbose)
	return;
    printf (", ");

    invo_name = NULL;
    advise (what, fmt, a, b, c, d, e, f);
    invo_name = cp;
}
