static char sccsid[] = "@(#)21	1.17.1.4  src/bos/usr/lib/sendmail/recipient.c, cmdsend, bos41J, 9510A_all 2/24/95 10:46:04";
/* 
 * COMPONENT_NAME: CMDSEND recipient.c
 * 
 * FUNCTIONS: MSGSTR, finduser, getctladdr, include, recipient, 
 *            sendtoargv, sendtolist 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
**  Sendmail
**  Copyright (c) 1983  Eric P. Allman
**  Berkeley, California
**
**  Copyright (c) 1983 Regents of the University of California.
**  All rights reserved.  The Berkeley software License Agreement
**  specifies the terms and conditions for redistribution.
**
*/


#include <nl_types.h>
#include "sendmail_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_SENDMAIL,n,s) 

# include <pwd.h>
# include <stdio.h>
# include <ctype.h>
# include <errno.h>
# include <string.h>

# include "conf.h"
# include "useful.h"
# include <sys/types.h>
# include <netinet/in.h>

# include "sysexits.h"

# include "sendmail.h"
# include <sys/stat.h>
# include <arpa/nameser.h>
# include <resolv.h>

void setpwent ();
ADDRESS *recipient();
ADDRESS *getctladdr();
struct passwd *finduser();
struct passwd *getpwent();
struct passwd *_getpwent();
ADDRESS  *parseaddr();
char  *xalloc ();
char  *safestr ();
char *DelimChar;         /* defined in prescan */
char **prescan();

/*
**  This is what determines whether we should query the nameserver for
**  this address.  We assume that if it speaks smtp, then it's a domain
**  address.  This was pulled from openmailer().
*/
#define smtp_mailer(m) (! strcmp((m)->m_mailer, "[IPC]"))

/*
**  SENDTOLIST -- Designate a send list.
**
**	The parameter is a comma-separated list of people to send to.
**	This routine arranges to send to all of them.
**
**	Parameters:
**		list -- the send list.
**		ctladdr -- the address template for the person to
**			send to -- effective uid/gid are important.
**			This is typically the alias that caused this
**			expansion.
**		sendq -- a pointer to the head of a queue to put
**			these people into.
**
**	Returns:
**		none
**
**	Side Effects:
**		none.
*/

# define MAXRCRSN	10

sendtolist(list, ctladdr, sendq)
	char *list;
	ADDRESS *ctladdr;
	ADDRESS **sendq;
{
	register char *p;
	register ADDRESS *al;	/* list of addresses to send to */
	int firstone;		/* set on first address sent */
	int selfref;		/* set if this list includes ctladdr */
	char delimiter;		/* the address delimiter */

# ifdef DEBUG
	if (tTd(25, 1))
	{
		(void) printf("sendto: %s\n   ctladdr=", list);
		printaddr(ctladdr, FALSE);
	}
# endif DEBUG

	/* heuristic to determine old versus new style addresses */
	if (ctladdr == NULL &&
	    (strchr(list, ',') != NULL || strchr(list, ';') != NULL ||
	     strchr(list, '<') != NULL || strchr(list, '(') != NULL))
		CurEnv->e_flags &= ~EF_OLDSTYLE;
	delimiter = ' ';
	if (!bitset(EF_OLDSTYLE, CurEnv->e_flags) || ctladdr != NULL)
		delimiter = ',';

	firstone = TRUE;
	selfref = FALSE;
	al = NULL;

	for (p = list; *p != '\0'; )
	{
		register ADDRESS *a;

		/* parse the address */
		while (isspace(*p) || *p == ',')
			p++;
		a = parseaddr(p, (ADDRESS *) NULL, 1, delimiter);
		p = DelimChar;
		if (a == NULL)
			continue;
		a->q_next = al;
		a->q_alias = ctladdr;

		/*
		 *  Prevent any body type conversions on mail passing through 
		 *  that has not been aliased.  Only do conversions on mail
		 *  that originates or is aliased on this system. 
		 */
		if (!*NetCode && !*MailCode) {
		    if (ctladdr == NULL && CurEnv->e_btype != BT_NLS)
			a->q_btype = BT_UNK;
		}

		/* see if this should be marked as a primary address */
		if (ctladdr == NULL ||
		    (firstone && *p == '\0' && bitset(QPRIMARY, ctladdr->q_flags)))
			a->q_flags |= QPRIMARY;

		/* put on send queue or suppress self-reference */
		if (ctladdr != NULL && sameaddr(ctladdr, a))
			selfref = TRUE;
		else
			al = a;
		firstone = FALSE;

#ifdef _SUN
		/* inherit the fact that we were a domain-wide alias. */
		if (ctladdr != NULL && bitset(QDOMAIN, ctladdr->q_flags))
		{
			char *p;
		
			p = strrchr(a->q_paddr, '@');
			if (p)
			{
		   		STAB *s;
		 		  s = stab(p+1, ST_CLASS, ST_FIND);
		  		 if ((s != NULL && bitnset('w', s->s_class)) ||
					sameword(macvalue('w', CurEnv),p+1) ||
					sameword(MyHostName, p+1)) {
					if (tTd(27, 1))
						printf("Found local alias %s\n", a->q_paddr);
					a->q_flags |= QWASLOCAL;
					if (selfref) ctladdr->q_flags |= QWASLOCAL;
		   		}
			}
			a->q_flags |= QDOMAIN; 
		}
#endif _SUN
	}

	/* if this alias doesn't include itself, delete ctladdr */
	if (!selfref && ctladdr != NULL)
		ctladdr->q_flags |= QDONTSEND;

	/* arrange to send to everyone on the local send list */
	while (al != NULL)
	{
		register ADDRESS *a = al;

		al = a->q_next;
		setctladdr(a);
		a = recipient(a, sendq);

		/* arrange to inherit full name */
		if (a->q_fullname == NULL && ctladdr != NULL)
			a->q_fullname = ctladdr->q_fullname;
	}

	CurEnv->e_to = NULL;
}
/*
**  MKLIST -- Construct a comma-separated recipient list (suitable for
**		sendtolist()) from an array of strings.
**
**	Allocates a string and cats the strings in the array into it.
**	If there's no '@' in the recipient name, we turn the first '.' into
**	a '@', since we assume it's a nameserver response that is in the
**	form of "user.domain".
**
**	NOTE that this can alter the original strings by substituting '@'s
**	for '.'s.
**
**	Parameters:
**		cpp -- ptr to the array of strings.
**
**	Returns:
**		a ptr to the constructed list.
**
*/

static char *mklist(cpp)
register char **cpp;
{
register int len = 0;
register char **cpp2, *cp, *cp2;

    /* calc size needed to store everyone */

    for (cpp2 = cpp; *cpp2; ++cpp2)
	len += strlen(*cpp2) + 1;  /* add space for ',' or terminating '\0' */

    /* malloc string and cat the list into it */

    *(cp = xalloc(len)) = '\0';
    while (*cpp) {
	if (*cp)  /* add delimiter */
	    strcat(cp, ",");

	/* change first '.' into '@' if there are no others in address */

	if (! (cp2 = strchr(*cpp, '@')) && (cp2 = strchr(*cpp, '.')))
	    *cp2 = '@';
	strcat(cp, *cpp++);
    }
    return(cp);
}
/*
**  RECIPIENT -- Designate a message recipient
**
**	Saves the named person for future mailing.
**
**	Parameters:
**		a -- the (preparsed) address header for the recipient.
**		sendq -- a pointer to the head of a queue to put the
**			recipient in.  Duplicate supression is done
**			in this queue.
**
**	Returns:
**		The actual address in the queue.  This will be "a" if
**		the address is not a duplicate, else the original address.
**
**	Side Effects:
**		none.
*/

ADDRESS *
recipient(a, sendq)
	register ADDRESS *a;
	register ADDRESS **sendq;
{
	register ADDRESS *q;
	ADDRESS **pq;
	register struct mailer *m;
	register char *p;
	char buf[MAXNAME];		/* unquoted image of the user name */
	int x400;
	static char re_rrmsg[] = "resolved via %s record to %s";
	extern ADDRESS *getctladdr();
	extern int safefile();

	CurEnv->e_to = a->q_paddr;
	m = a->q_mailer;
	errno = 0;
	x400 = 0;

# ifdef DEBUG
	if (tTd(26, 1))
	{
		(void) printf("\nrecipient: ");
		printaddr(a, FALSE);
	}
# endif DEBUG

	/* break aliasing loops */
	if (AliasLevel > MAXRCRSN)
	{
		usrerr(MSGSTR(RE_EALIAS, "aliasing/forwarding loop broken")); /*MSG*/
		return (a);
	}

	/*
	**  Finish setting up address structure.
	*/

	/* set the queue timeout */
	a->q_timeout = TimeOut;

	if (strstr(a->q_user,"/ADMD"))
		x400 = 1;
	/* map user & host to lower case if requested on non-aliases */
	/* NOTE: loweraddr doesn't map the host to lower */
	if (a->q_alias == NULL)
		loweraddr(a);

	/*
	 *  It used to be that a \-quoted char in the user name would
	 *  disable usage of a .forward file at the delivery mailbox.
	 *  This totally undocumented usage has been removed.  Now, .forward
	 *  files will always be honored.  Mail which loops in forwarding will
	 *  be dropped on max hops just like interhost aliasing loops.
	 *  The sender always receives notification of dropped mail.
	 *
	 *  NOTE:  sendmail seems smart enough to not cause a forwarding loop
	 *  if one of the forwarding targets is self or the sender of the mail.
	 *
	 *  RFC type documentation only speaks about using \-quoting to prevent
	 *  special characters (such as comma, @, etc.) from being recognized
	 *  as special.
	 */
	(void) strcpy(buf, a->q_user);
	stripquotes(buf, TRUE);			/* q_user less quotes */

	/* do sickly crude mapping for program mailing, etc. */
	if (m == LocalMailer && buf[0] == '|')
	{
		a->q_mailer = m = ProgMailer;
		a->q_user++;
		if (a->q_alias == NULL && !bitset(QGOODUID, a->q_flags) && !ForceMail)
		{
			usrerr(MSGSTR(RE_EPROG, "Cannot mail directly to programs")); /*MSG*/
			a->q_flags |= QDONTSEND|QBADADDR;
		}
	}

	/*
	**  Look up this person in the recipient list.
	**	If they are there already, return, otherwise continue.
	**	If the list is empty, just add it.  Notice the cute
	**	hack to make from addresses suppress things correctly:
	**	the QDONTSEND bit will be set in the send list.
	**	[Please note: the emphasis is on "hack."]
	*/

	for (pq = sendq; (q = *pq) != NULL; pq = &q->q_next)
	{
		if (!ForceMail && sameaddr(q, a))
		{
# ifdef DEBUG
			if (tTd(26, 1))
			{
				(void) printf("%s in sendq: ", a->q_paddr);
				printaddr(q, FALSE);
			}
# endif DEBUG
			if (!bitset(QDONTSEND, a->q_flags))
				message(Arpa_Info, MSGSTR(RE_DUP, "duplicate suppressed")); /*MSG*/
			if (!bitset(QPRIMARY, q->q_flags))
				q->q_flags |= a->q_flags;
			return (q);
		}
	}

	/* add address on list */
	*pq = a;
	a->q_next = NULL;
	CurEnv->e_nrcpts++;

	/*
	**  For local names, check for :include: specs, aliases,
	**  and forwarding.
	*/

	if (m == LocalMailer) {
	    if (!bitset(QDONTSEND, a->q_flags))
	    {
		if (strncmp(a->q_user, ":include:", 9) == 0)
		{
			a->q_flags |= QDONTSEND;

			if (a->q_alias == NULL && !bitset(QGOODUID, a->q_flags) && !ForceMail)
			{
				a->q_flags |= QBADADDR;
				usrerr(MSGSTR(RE_EINCL, "Cannot mail directly to :include:s")); /*MSG*/
			}
			else
			{
				message(Arpa_Info, MSGSTR(RE_INCLF, "including file %s"), &a->q_user[9]); /*MSG*/
				include(&a->q_user[9], MSGSTR(RE_SENDING, " sending"), a, sendq); /*MSG*/
			}
		}

		else
			alias(a, sendq);
	    }

	    /*
	    **  If the user is still being sent, verify that
	    **  the address is good.  If it is, try to forward.
	    **  If the address is already good, we have a forwarding
	    **  loop.  This can be broken by just sending directly to
	    **  the user (which is probably correct anyway).
	    */

	    if (!bitset(QDONTSEND, a->q_flags))
	    {
		struct stat stb;
		extern int writable();

		/* see if this is to a file */
		if (buf[0] == '/' && !x400)
		{
			p = rindex(buf, '/');
			/* check if writable or creatable */
			if (a->q_alias == NULL && !bitset(QGOODUID, a->q_flags) && !ForceMail)
			{
				usrerr(MSGSTR(DF_MAILF, "Cannot mail directly to files")); /*MSG*/
				a->q_flags |= QDONTSEND|QBADADDR;
			}
			else if ((stat(buf, &stb) >= 0) ? (!writable(&stb)) :
			    (*p = '\0', !safefile(buf, getruid(), S_IWRITE|S_IEXEC)))
			{
				a->q_flags |= QBADADDR;
				giveresponse(EX_CANTCREAT, m, CurEnv);
			}
		}
		else
		{
			register struct passwd *pw;

			/* warning -- finduser may trash buf */
			pw = finduser(buf);
			if (pw == NULL)
			{
				a->q_flags |= QBADADDR;
				giveresponse(EX_NOUSER, m, CurEnv);
			}
			else
			{
				char nbuf[MAXNAME];

				(void) quotestr (nbuf, pw->pw_name, MAXNAME);

				if (strcmp(a->q_user, nbuf) != 0)
				{
					a->q_user = newstr(nbuf);
					(void) strcpy(buf, pw->pw_name);
				}
				a->q_home = newstr(pw->pw_dir);
				a->q_uid = pw->pw_uid;
				a->q_gid = pw->pw_gid;
				a->q_flags |= QGOODUID;
				buildfname(pw->pw_gecos, pw->pw_name, nbuf);
				if (nbuf[0] != '\0')
					a->q_fullname = newstr(nbuf);
				/*
				 *  There used to be a test for \-quoted
				 *  string to prevent a .forward file from
				 *  being honored.  This was documented
				 *  nowhere (though people used it), and
				 *  has been removed.  Forwarding loops will
				 *  behave the same as interhost aliasing
				 *  loops:  The mail will be dropped on
				 *  max hops, and the sender will be notified.
				 */
				forward(a, sendq);
			}
		}
	    }

	/*
	**  For domain addresses, check for matching nameserver records
	**  of the types specified in the configuration file.
	**  We first check for mail groups, treating them like aliases
	**  in that they can expand to arbitrary nesting of groups;
	**  we then check for renames, and then mailbox records.
	*/

	} else if (! bitset(QDONTSEND, a->q_flags) && smtp_mailer(m) &&
	    (bitnset(T_MB, NameServOpt) || bitnset(T_MG, NameServOpt) ||
	    bitnset(T_MR, NameServOpt))) {

	    char pvpbuf[PSBUFSIZE], addrbuf[MAXNAME], **respv;
	    register char **pvp, *resp;
	    int err, nresp, free_resp;
	    long old_options;

	    /*
	    **  Rewrite user address with rule 6: it changes user@domain
	    **  into user.domain, and returns CANONNET as the first token
	    **  to indicate success.
	    */

	    if (! (pvp = prescan(a->q_user, '\0', pvpbuf)))
		return(a);  /* punt on parsing errors */
	    rewrite(pvp, 6);
	    if (**pvp != CANONNET)  /* not a valid user@domain address */
		return(a);
	    cataddr(++pvp, addrbuf, sizeof(addrbuf));  /* flatten it back out */
# ifdef DEBUG
	    if (tTd(26, 10))
		(void) printf("recipient: nameserver query name is '%s'\n",
		    addrbuf);
# endif DEBUG

	    /* tell res_search() not to search parent domains; just let the
	       name server do it all */
	    old_options = _res.options;  /* save resolver options */
	    _res.options &= ~(RES_DEFNAMES | RES_DNSRCH);

	    /* first check for MG records */

	    nresp = free_resp = 0;
	    respv = NULL;
	    if (bitnset(T_MG, NameServOpt) &&
		(nresp = getrr(T_MG, addrbuf, &respv, &err)) > 0) {

		    /* make a comma-separated list out of the responses, and
		       change them from "user.domain" to "user@domain" */

		    resp = mklist(respv);
		    ++free_resp;  /* yech */
		    message(Arpa_Info, MSGSTR(RE_RRMSG, re_rrmsg), "MG", resp);

	    /* if we didn't query for MGs or we didn't get any, check for
	       MR and MB records */

	    } else if (! nresp) {

		if (bitnset(T_MR, NameServOpt) &&
		    (nresp = getrr(T_MR, addrbuf, &respv, &err)) > 0) {

			/*
			**  According to rfc-1035 this should resolve to an MB
			**  record, so we set up for an MB query by replacing
			**  the domain with the first MR response.  However,
			**  if it doesn't resolve, we accept the MR as a valid
			**  recipient.
			*/

			resp = (char *)strcpy(addrbuf, *respv);
			message(Arpa_Info, MSGSTR(RE_RRMSG, re_rrmsg),
			    "MR", addrbuf);
		}

		/* if no errors so far, check for MBs */

		if (nresp >= 0 && bitnset(T_MB, NameServOpt) &&
		    (nresp = getrr(T_MB, addrbuf, &respv, &err)) > 0) {

			/* response is a hostname; append it to the
			   username from the query address */

			register char *cp;

			*(cp = strchr(addrbuf, '.')) = '@';
			strcpy(++cp, *respv);
			resp = addrbuf;
			message(Arpa_Info, MSGSTR(RE_RRMSG, re_rrmsg),
			    "MB", addrbuf);
		}
	    }
	    _res.options = old_options;  /* restore resolver options */

	    if (nresp < 0) {
		/* if this is a temp failure queue it up, else mark it bad */
		a->q_flags |= (err == EX_TEMPFAIL ? QQUEUEUP : QBADADDR);
		giveresponse(err, m, CurEnv);

	    } else if (nresp) {  /* got valid response(s): put on the sendq */
		AliasLevel++;  /* break loops just like alias/forwards */
		sendtolist(resp, a, sendq);
		AliasLevel--;
	    }

	    /* free list and ptr array */
	    if (respv) {
		for (pvp = respv; *pvp; ++pvp)
		    free(*pvp);  /* free each string */
		free(respv);
	    }
	    if (free_resp)
		free(resp);
	}
	return (a);
}
/*
**  FINDUSER -- find the password entry for a user.
**
**	This looks a lot like getpwnam, except that it may want to
**	do some fancier pattern matching in /etc/passwd.
**
**	This routine contains most of the time of many sendmail runs.
**	It deserves to be optimized.
**
**	Parameters:
**		name -- the name to match against.
**
**	Returns:
**		A pointer to a pw struct.
**		NULL if name is unknown or ambiguous.
**
**	Side Effects:
**		None.
*/

struct passwd *
finduser(name)
	char *name;
{
	register struct passwd *pw;
	register char *p;

	/* look up this login name using fast path */
	if ((pw = _getpwnam_shadow(name,0)) != NULL)
		return (pw);

	/* search for a matching full name instead */
	for (p = name; *p != '\0'; p++)
	{
		if (*p == (SpaceSub & 0177) || *p == '_')
			*p = ' ';
	}
	setpwent();

	/* scan the /etc/passwd file from start to finish, ignoring the
	   /etc/security/passwd file. */

	while ((pw = _getpwent()) != NULL)
	{
		char buf[MAXNAME];

		buildfname(pw->pw_gecos, pw->pw_name, buf);

		if (strchr(buf, ' ') != NULL && sameword(buf, name))
		{
			message(Arpa_Info, MSGSTR(RE_LOGNAME, "sending to login name %s"), pw->pw_name); /*MSG*/
			return (pw);
		}
	}
	return (NULL);
}
/*
**  WRITABLE -- predicate returning if the file is writable.
**
**	This routine must duplicate the algorithm in sys/fio.c.
**	Unfortunately, we cannot use the access call since we
**	won't necessarily be the real uid when we try to
**	actually open the file.
**
**	Notice that ANY file with ANY execute bit is automatically
**	not writable.  This is also enforced by mailfile.
**
**	Parameters:
**		s -- pointer to a stat struct for the file.
**
**	Returns:
**		TRUE -- if we will be able to write this file.
**		FALSE -- if we cannot write this file.
**
**	Side Effects:
**		none.
*/

int
writable(s)
	register struct stat *s;
{
	int euid, egid;
	int bits;

	if (bitset(0111, s->st_mode))
		return (FALSE);
	euid = getruid();
	egid = getrgid();
	if (geteuid() == 0)
	{
		if (bitset(S_ISUID, s->st_mode))
			euid = s->st_uid;
		if (bitset(S_ISGID, s->st_mode))
			egid = s->st_gid;
	}

	if (euid == 0)
		return (TRUE);
	bits = S_IWRITE;
	if (euid != s->st_uid)
	{
		bits >>= 3;
		if (egid != s->st_gid)
			bits >>= 3;
	}
	return ((s->st_mode & bits) != 0);
}
/*
**  INCLUDE -- handle :include: specification.
**
**	Parameters:
**		fname -- filename to include.
**		msg -- message to print in verbose mode.
**		ctladdr -- address template to use to fill in these
**			addresses -- effective user/group id are
**			the important things.
**		sendq -- a pointer to the head of the send queue
**			to put these addresses in.
**
**	Returns:
**		none.
**
**	Side Effects:
**		reads the :include: file and sends to everyone
**		listed in that file.
*/

include(fname, msg, ctladdr, sendq)
	char *fname;
	char *msg;
	ADDRESS *ctladdr;
	ADDRESS **sendq;
{
	char buf[MAXLINE];
	register FILE *fp;
	char *oldto = CurEnv->e_to;
	char *oldfilename = FileName;
	int oldlinenumber = LineNumber;
	char hlds[NL_TEXTMAX];
	ADDRESS *ca;
	extern int safepath();

	strcpy(hlds, msg);

	if ((ca = getctladdr(ctladdr)) == NULL)
	{
		struct stat st;

		if (stat(fname, &st) < 0)
			syserr(MSGSTR(RE_EFSTAT, "Cannot fstat %s!"), fname); /*MSG*/
		ctladdr->q_uid = st.st_uid;
		ctladdr->q_gid = st.st_gid;
		ctladdr->q_flags |= QGOODUID;
	}
	else
	{
	    /*
	     * Enforce BSD-compatibility here.  Our "safepath" works like
	     * BSD's "safefile", so call it instead.  Basically, the GOODUID
	     * has to be able to read this file, but does not necessarily
	     * have to be the owner.
	     */
	    if (!safepath(fname, ca->q_uid, ca->q_gid, S_IREAD)) {
		    syserr(MSGSTR(DL_EOPEN, "Cannot open %s for %s from %s"),
			fname, CurEnv->e_to, ca->q_user); /*MSG*/
		    return; 
	    }
	}

	fp = fopen(fname, "r");
	if (fp == NULL)
	{
		usrerr(MSGSTR(DL_EOPEN, "Cannot open %s for %s from %s"), fname, CurEnv->e_to, ca->q_user); /*MSG*/
		return;
	}

#ifdef _SUN
	/* Names from include files are not domain-wide */
	ctladdr->q_flags &= ~QDOMAIN;
#endif _SUN

	/* read the file -- each line is a comma-separated list. */
	FileName = fname;
	LineNumber = 0;
	while (fgets(buf, sizeof buf, fp) != NULL)
	{
		register char *p = strchr(buf, '\n');

		LineNumber++;
		if (p != NULL)
			*p = '\0';
		if (buf[0] == '\0')
			continue;
		CurEnv->e_to = oldto;
		message(Arpa_Info, MSGSTR(RE_TO, "%s to %s"), hlds, buf); /*MSG*/
		AliasLevel++;
		sendtolist(buf, ctladdr, sendq);
		AliasLevel--;
	}

	(void) fclose(fp);
	FileName = oldfilename;
	LineNumber = oldlinenumber;
}
/*
**  SENDTOARGV -- send to an argument vector.
**
**	Parameters:
**		argv -- argument vector to send to.
**
**	Returns:
**		none.
**
**	Side Effects:
**		puts all addresses on the argument vector onto the
**			send queue.
*/

sendtoargv(argv)
	register char **argv;
{
	register char *p;

	while ((p = *argv++) != NULL)
	{
		if (argv[0] != NULL && argv[1] != NULL && sameword(argv[0], "at"))
		{
			char nbuf[MAXNAME];

			if (strlen(p) + strlen(argv[1]) + 2 > sizeof nbuf)
				usrerr(MSGSTR(RE_OFLOW, "address overflow")); /*MSG*/
			else
			{
				(void) strcpy(nbuf, p);
				(void) strcat(nbuf, "@");
				(void) strcat(nbuf, argv[1]);
				p = nbuf;
				argv += 2;
			}
		}
		p = safestr(p);
		p = newstr(p);
		sendtolist(p, (ADDRESS *) NULL, &CurEnv->e_sendqueue);
	}
}
/*
**  GETCTLADDR -- get controlling address from an address header.
**		This is only called from the "include" processor and
**		when setting up for the program mailer in deliver ().
**
**	If none, get one corresponding to the effective userid.
**
**	Parameters:
**		a -- the address to find the controller of.
**
**	Returns:
**		the controlling address.
**
**	Side Effects:
**		none.
*/

ADDRESS *
getctladdr(a)
	register ADDRESS *a;
{
	while (a != NULL && !bitset(QGOODUID, a->q_flags))
		a = a->q_alias;
	return (a);
}
