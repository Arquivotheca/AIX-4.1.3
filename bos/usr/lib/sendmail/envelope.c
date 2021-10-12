static char sccsid[] = "@(#)09	1.13.2.3  src/bos/usr/lib/sendmail/envelope.c, cmdsend, bos41J, 9510A_all 2/16/95 18:16:56";
/* 
 * COMPONENT_NAME: CMDSEND envelope.c
 * 
 * FUNCTIONS: MSGSTR, clearenvelope, closexscript, dropenvelope, 
 *            initsys, newenvelope, openxscript, setsender, settime, 
 *            trusteduser 
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
# include <time.h>
# include <stdio.h>
# include <ctype.h>
# include <memory.h>
# include "conf.h"
# include "useful.h"
# include <sys/types.h>
# include <netinet/in.h>
# include <sys/file.h>

# include "sendmail.h"
# include <sys/stat.h>
# include <string.h>
# include <sys/access.h>   /* security */

char  *getenv ();
int putheader(), putbody();
char *ttyname();
char *macvalue();
char *arpadate();
char **prescan();
char *username();
char **copyplist();
char  *queuename ();
char  *xalloc ();
ADDRESS  *parseaddr ();
#define  cur_time	time ((long *) 0)

ENVELOPE BlankEnvelope;
char *FullName;
extern int  Either;

/*
**  NEWENVELOPE -- allocate a new envelope
**
**	Supports inheritance.
**
**	Parameters:
**		e -- the new envelope to fill in.
**
**	Returns:
**		e.
**
**	Side Effects:
**		none.
*/

ENVELOPE *
newenvelope(e)
	register ENVELOPE *e;
{
	register ENVELOPE *parent;

	parent = CurEnv;
	if (e == CurEnv)
		parent = e->e_parent;
	clearenvelope(e, TRUE);
	if (e == CurEnv)
		(void) memcpy((char *) &e->e_from, (char *) &NullAddress, sizeof e->e_from);
	else
		(void) memcpy((char *) &e->e_from, (char *) &CurEnv->e_from, sizeof e->e_from);
	e->e_parent = parent;
	e->e_ctime = cur_time;
	e->e_msgpriority = parent->e_msgsize;
	e->e_puthdr = putheader;
	e->e_putbody = putbody;
	e->e_btype = Btype;
	if (CurEnv->e_xfp != NULL)
		(void) fflush(CurEnv->e_xfp);

	return (e);
}
/*
**  DROPENVELOPE -- deallocate an envelope.
**
**	Parameters:
**		e -- the envelope to deallocate.
**
**	Returns:
**		none.
**
**	Side Effects:
**		housekeeping necessary to dispose of an envelope.
**		Unlocks this queue file.
*/

dropenvelope(e)
	register ENVELOPE *e;
{
	int queueit = FALSE;
	register ADDRESS *q;

#ifdef DEBUG
	if (tTd(50, 1))
	{
		(void) printf("dropenvelope %x id=", e);
		xputs(e->e_id);
		(void) printf(" flags=%o\n", e->e_flags);
	}
#endif DEBUG
#ifdef LOG
	if (LogLevel > 10)
		syslog(LOG_DEBUG, MSGSTR(EN_DROP,
		    "dropenvelope, id=%s, flags=%o, pid=%d"),
		    e->e_id == NULL ? MSGSTR(EN_NONE, "(none)") : e->e_id,
		    e->e_flags, getpid());
#endif LOG

	/* we must have an id to remove disk files */
	if (e->e_id == NULL)
		return;

	/*
	**  Extract state information from dregs of send list.
	*/

	for (q = e->e_sendqueue; q != NULL; q = q->q_next)
	{
		if (bitset(QQUEUEUP, q->q_flags))
			queueit = TRUE;
	}

	/*
	**  Send back return receipts as requested.
	*/

	if (e->e_receiptto != NULL && bitset(EF_SENDRECEIPT, e->e_flags))
	{
		ADDRESS *rlist = NULL;

		sendtolist(CurEnv->e_receiptto, (ADDRESS *) NULL, &rlist);
		(void) returntosender("Return receipt", rlist, FALSE);
	}

	/*
	**  Arrange to send error messages if there are fatal errors.
	*/

	if (bitset(EF_FATALERRS|EF_TIMEOUT, e->e_flags) && ErrorMode != EM_QUIET)
		savemail(e);

	/*
	**  Instantiate or deinstantiate the queue.
	*/

	if ((!queueit && !bitset(EF_KEEPQUEUE, e->e_flags)) ||
	    bitset(EF_CLRQUEUE, e->e_flags))
	{
		if (e->e_df != NULL)
			xunlink(e->e_df);

		if (e->e_dfi != NULL && e->e_dfi != e->e_df)  /* ISO body */
			xunlink(e->e_dfi);	
		if (e->e_dfe != NULL && e->e_dfe != e->e_df)  /* esc body */
			xunlink(e->e_dfe);
		if (e->e_dff != NULL && e->e_dff != e->e_df)  /* flat body */
			xunlink(e->e_dff);
		if (e->e_dfn != NULL && e->e_dfn != e->e_df)  /* nls body */
			xunlink(e->e_dfn);

		if (e->e_dfs != NULL && e->e_dfs != e->e_df)  /* MailCode body*/
			xunlink(e->e_dfs);
		if (e->e_dfj != NULL && e->e_dfj != e->e_df)  /* NetCode body */
			xunlink(e->e_dfj);

		xunlink(queuename(e, 'q'));
	}
	else if (queueit || !bitset(EF_INQUEUE, e->e_flags))
	{
		queueup(e, FALSE, FALSE);
	}

	/* now unlock the job */
	closexscript(e);
	unlockqueue(e);

	/* make sure that this envelope is marked unused */
	e->e_id = e->e_df = NULL;
	if (e->e_dfp != NULL)
		(void) fclose(e->e_dfp);
	e->e_dfp = NULL;
}
/*
**  CLEARENVELOPE -- clear an envelope without unlocking
**
**	This is normally used by a child process to get a clean
**	envelope without disturbing the parent.
**
**	Parameters:
**		e -- the envelope to clear.
**		fullclear - if set, the current envelope is total
**			garbage and should be ignored; otherwise,
**			release any resources it may indicate.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Closes files associated with the envelope.
**		Marks the envelope as unallocated.
*/

clearenvelope(e, fullclear)
	register ENVELOPE *e;
	int fullclear;
{
	register HDR *bh;
	register HDR **nhp;

	if (!fullclear)
	{
		/* clear out any file information */
		if (e->e_xfp != NULL)
			(void) fclose(e->e_xfp);
		if (e->e_dfp != NULL)
			(void) fclose(e->e_dfp);
	}

	/* now clear out the data */
	*e = BlankEnvelope;
	bh = BlankEnvelope.e_header;
	nhp = &e->e_header;
	while (bh != NULL)
	{
		*nhp = (HDR *) xalloc(sizeof (*bh));
		(void) memcpy((char *) *nhp, (char *) bh, sizeof (*bh));
		bh = bh->h_link;
		nhp = &(*nhp)->h_link;
	}
}
/*
**  INITSYS -- initialize instantiation of system
**
**	In Daemon mode, this is done in the child.
**
**	Parameters:
**		none.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Initializes the system macros, some global variables,
**		etc.  In particular, the current time in various
**		forms is set.
*/

initsys()
{
	static char cbuf[5];			/* holds hop count */
	static char pbuf[10];			/* holds pid */
	static char ybuf[10];			/* holds tty id */
	register char *p;

	/*
	**  Give this envelope a reality.
	**	I.e., an id, a transcript, and a creation time.
	*/

	openxscript(CurEnv);
	CurEnv->e_ctime = cur_time;

	/*
	**  Set OutChannel to something useful if stdout isn't it.
	**	This arranges that any extra stuff the mailer produces
	**	gets sent back to the user on error (because it is
	**	tucked away in the transcript).
	*/

	if (OpMode == MD_DAEMON && QueueRun)
		OutChannel = CurEnv->e_xfp;

	/*
	**  Set up some basic system macros.
	*/

	/* process id */
	(void) sprintf(pbuf, "%d", getpid());
	define('p', pbuf, CurEnv);

	/* hop count */
	(void) sprintf(cbuf, "%d", CurEnv->e_hopcount);
	define('c', cbuf, CurEnv);

	/* time as integer, unix time, arpa time */
	settime();

	/* tty name */
	if (macvalue('y', CurEnv) == NULL)
	{
		p = ttyname(2);
		if (p != NULL)
		{
			if (strrchr(p, '/') != NULL)
				p = strrchr(p, '/') + 1;
			(void) strcpy(ybuf, p);
			define('y', ybuf, CurEnv);
		}
	}
}

/*
**  SETTIME -- set the current time.
**
**	Parameters:
**		none.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Sets the various time macros -- $a, $b, $d, $t.
*/

settime()
{
	register char *p;
	long  now;
	static char tbuf[20];			/* holds "current" time */
	static char dbuf[30];			/* holds ctime(tbuf) */
	register struct tm *tm;

	now = cur_time;
	tm = gmtime(&now);
	(void) sprintf(tbuf, "%02d%02d%02d%02d%02d", tm->tm_year, tm->tm_mon+1,
			tm->tm_mday, tm->tm_hour, tm->tm_min);
	define('t', tbuf, CurEnv);
	(void) strcpy(dbuf, ctime(&now));
	*strchr(dbuf, '\n') = '\0';
	if (macvalue('d', CurEnv) == NULL)
		define('d', dbuf, CurEnv);
	p = arpadate(dbuf);
	if (macvalue('a', CurEnv) == NULL)
		define('a', p, CurEnv);
	define('b', p, CurEnv);
}
/*
**  OPENXSCRIPT -- Open transcript file
**
**	Creates a transcript file for possible eventual mailing or
**	sending back.
**
**	Parameters:
**		e -- the envelope to create the transcript in/for.
**
**	Returns:
**		none
**
**	Side Effects:
**		Creates the transcript file.
*/

openxscript(e)
	register ENVELOPE *e;
{
	register char *p;
	int fd;

# ifdef LOG
	if (LogLevel > 19)
		syslog(LOG_DEBUG, "%s: openx%s", e->e_id, e->e_xfp == NULL ? "" : " (no)");
# endif LOG
	if (e->e_xfp != NULL)
		return;
	p = queuename(e, 'x');
	fd = open(p, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
	if (fd < 0)
		syserr(MSGSTR(EN_ECREATE, "Can't create %s"), p); /*MSG*/
	else
		e->e_xfp = fdopen(fd, "w");
}
/*
**  CLOSEXSCRIPT -- close the transcript file.
**
**	Parameters:
**		e -- the envelope containing the transcript to close.
**
**	Returns:
**		none.
**
**	Side Effects:
**		none.
*/

closexscript(e)
	register ENVELOPE *e;
{
	if (e->e_xfp == NULL)
		return;
	(void) fclose(e->e_xfp);
	e->e_xfp = NULL;
}
/*
**  SETSENDER -- set the person who this message is from
**
**	Under certain circumstances allow the user to say who
**	he is (using -f or -r).  These are:
**	1.  The user's uid is zero (root).
**	2.  The user's login name is in an approved list (typically
**	    from a network server).
**	3.  The address the user is trying to claim has a
**	    "!" character in it (since #2 doesn't do it for
**	    us if we are dialing out for UUCP).
**	A better check to replace #3 would be if the
**	effective uid is "UUCP" -- this would require me
**	to rewrite getpwent to "grab" uucp as it went by,
**	make getname more nasty, do another passwd file
**	scan, or compile the UID of "UUCP" into the code,
**	all of which are reprehensible.
**
**	Assuming all of these fail, we figure out something
**	ourselves.
**
**	Parameters:
**		from -- the person we would like to believe this message
**			is from, as specified on the command line.
**
**	Returns:
**		none.
**
**	Side Effects:
**		sets sendmail's notion of who the from person is.
*/

setsender(from)
	char *from;
{
	register char **pvp;
	char *realname = NULL;
	register struct passwd *pw;
	char buf[MAXNAME];
	char ubuf[MAXNAME];
	char pvpbuf[PSBUFSIZE];

# ifdef DEBUG
	if (tTd(45, 1))
		(void) printf("setsender(%s)\n", from == NULL ? "" : from);
# endif DEBUG

	/*
	**  Figure out the real user executing us.
	**	Username can return errno != 0 on non-errors.
	*/

	if (QueueRun || OpMode == MD_SMTP || OpMode == MD_ARPAFTP)
		realname = from;
	if (realname == NULL || realname[0] == '\0')
	{
		realname = username();
	}

	/*
	**  Determine if this real person is allowed to alias themselves.
	*/

	if (from != NULL)
	{
		if (!trusteduser (realname)    &&
/**//* ? */	    strchr (from, '!') == NULL && 
/**//* ? */	    !Either                       )
		{
		    /* network sends -r regardless (why why why?) */
		    /* syserr(MSGSTR(EN_EFFLAG, "%s, you cannot use the -f flag"), realname); */ /*MSG*/
		    from = NULL;
		}
	}

	/*
	 *  In the following code the "from" string is parsed.  If there
	 *  is no "from" string or if the existing string does not parse,
	 *  then "from" is reset to be "realname".  Then "from" ( == "realname")
	 *  is parsed again.  In case that fails, "postmaster" is used,
	 *  though "from" remains set to realname.  
	 *  I am not sure the processing is correct if this happens, since
	 *  CurEnv->e_from will have info from the parse of postmaster,
	 *  while "from" will still have the real name.
	 *  "from" is used to build the $f macro.
	 */
	SuprErrs = TRUE;
	if (from == NULL || parseaddr(from, &CurEnv->e_from, 1, '\0') == NULL)
	{
		/* log garbage addresses for traceback */
		if (from != NULL)
		{
# ifdef LOG
			if (LogLevel >= 1)
			    if (realname == from && RealHostName != NULL)
				syslog(LOG_NOTICE, MSGSTR(DL_EPARSE1, "from=%s unparseable, received from %s"),
				    from, RealHostName);
			    else
				syslog(LOG_NOTICE, MSGSTR(DL_EPARSE2, "Unparseable username %s wants from=%s"),
				    realname, from);
# endif LOG
		}
		from = newstr(realname);
		if (parseaddr(from, &CurEnv->e_from, 1, '\0') == NULL &&
		    parseaddr("postmaster", &CurEnv->e_from, 1, '\0') == NULL)
		{
			syserr(MSGSTR(DL_EPOSTMAST, "setsender: can't even parse postmaster!")); /*MSG*/
		}
	}
	else
		FromFlag = TRUE;	/* "from" parsed OK */
					/* else, is realname or "postmaster" */
	CurEnv->e_from.q_flags |= QDONTSEND;
	loweraddr(&CurEnv->e_from);
	SuprErrs = FALSE;

	(void) unquotestr (ubuf,
	    bitnset(M_FORWARD, CurEnv->e_from.q_mailer->m_flags) ?
	    CurEnv->e_from.q_paddr : CurEnv->e_from.q_user, MAXNAME);

	if ((CurEnv->e_from.q_mailer == LocalMailer ||
	    bitnset(M_FORWARD, CurEnv->e_from.q_mailer->m_flags)) &&
	    (pw = getpwnam(ubuf)) != NULL)
	{
		/*
		**  Process passwd file entry.
		*/

		/* extract home directory */
		CurEnv->e_from.q_home = newstr(pw->pw_dir);
		define('z', CurEnv->e_from.q_home, CurEnv);

		/* extract user and group id */
		CurEnv->e_from.q_uid = pw->pw_uid;
		CurEnv->e_from.q_gid = pw->pw_gid;
		CurEnv->e_from.q_flags |= QGOODUID;

		/* if the user has given fullname already, don't redefine */
		if (FullName == NULL)
			FullName = macvalue('x', CurEnv);
		if (FullName != NULL && FullName[0] == '\0')
			FullName = NULL;

		/* extract full name from passwd file */
		if (FullName == NULL && pw->pw_gecos != NULL &&
		    strcmp(pw->pw_name, ubuf) == 0)
		{
			buildfname(pw->pw_gecos, pw->pw_name, buf);
			if (buf[0] != '\0')
				FullName = newstr(buf);
		}
		if (FullName != NULL)
			define('x', FullName, CurEnv);
	}
	else	/* not local mailer, or no passwd file entry */
	{
		if (CurEnv->e_from.q_home == NULL)
			CurEnv->e_from.q_home = newstr(getenv("HOME"));
	}

/**/	if (bitset (QGOODUID, CurEnv->e_from.q_flags))
/**/	    message (0, "000 setsender: uid/gid = %d/%d",
/**/				CurEnv->e_from.q_uid, CurEnv->e_from.q_gid);
/**/	else
/**/	    message (0, MSGSTR(EN_EUID, "000 setsender: uid/gid not good")); /*MSG*/

	/*
	**  Rewrite the from person to dispose of possible implicit
	**	links in the net.
	*/

	/*
	 *  This defines the $f macro, and collects the "from" domain.
	 *  The rewrite sequence for this is rulesets 3,1,4 on the "from"
	 *  string.
	 */
	pvp = prescan(from, '\0', pvpbuf);
	if (pvp == NULL)
	{
# ifdef LOG
		if (LogLevel >= 1)
			syslog(LOG_NOTICE, MSGSTR(EN_PRESCAN, "cannot prescan from (%s)"), from);
# endif
		usrerr(MSGSTR(EN_PRESCAN, "cannot prescan from (%s)"), from);
		finis();
	}
	rewrite(pvp, 3);
	rewrite(pvp, 1);
	rewrite(pvp, 4);
	cataddr(pvp, buf, sizeof buf);
	define('f', newstr(buf), CurEnv);

	/* save the domain spec if this mailer wants it */
	if (CurEnv->e_from.q_mailer != NULL &&
	    bitnset(M_CANONICAL, CurEnv->e_from.q_mailer->m_flags))
	{
		while (*pvp != NULL && strcmp(*pvp, "@") != 0)
			pvp++;
		if (*pvp != NULL)
			CurEnv->e_fromdomain = copyplist(pvp, TRUE);
	}
}
/*
**  TRUSTEDUSER -- tell us if this user is to be trusted.
**
**	Parameters:
**		user -- the user to be checked.
**
**	Returns:
**		TRUE if the user is in an approved list.
**		FALSE otherwise.
**
**	Side Effects:
**		none.
*/

int
trusteduser(user)
	char *user;
{
	register char **ulist;

	for (ulist = TrustedUsers; *ulist != NULL; ulist++)
		if (strcmp(*ulist, user) == 0)
			return (TRUE);
	return (FALSE);
}
