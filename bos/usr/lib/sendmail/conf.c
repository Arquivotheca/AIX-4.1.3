static char sccsid[] = "@(#)96	1.11.1.1  src/bos/usr/lib/sendmail/conf.c, cmdsend, bos41B, 9506A 1/25/95 16:50:15";
/* 
 * COMPONENT_NAME: CMDSEND conf.c
 * 
 * FUNCTIONS: MSGSTR, checkcompat, getla, setproctitle, shouldqueue, 
 *            ttypath, username, setdefuser 
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

# include <errno.h>
# include <pwd.h>
# include <stdio.h>
# include <ctype.h>
# include <string.h>
# include <sys/types.h>
# include <netinet/in.h>
# include <sys/access.h>

# include "conf.h"
# include "useful.h"
# include "sendmail.h"

char *getlogin();
char *ttyname();

extern int  CurUid;
char **Argv;
char *LastArgv;

/*
**  CONF.C -- Sendmail Configuration Tables.
**
**	Defines the configuration of this installation.
**
**	Compilation Flags:
**
**	Configuration Variables:
**		HdrInfo -- a table describing well-known header fields.
**			Each entry has the field name and some flags,
**			which are described in sendmail.h.
**
**	Notes:
**		I have tried to put almost all the reasonable
**		configuration information into the configuration
**		file read at runtime.  My intent is that anything
**		here is a function of the version of UNIX you
**		are running, or is really static -- for example
**		the headers are a superset of widely used
**		protocols.  If you find yourself playing with
**		this file too much, you may be making a mistake!
*/




/*
**  Header info table
**	Final (null) entry contains the flags used for any other field.
**
**	Not all of these are actually handled specially by sendmail
**	at this time.  They are included as placeholders, to let
**	you know that "someday" I intend to have sendmail do
**	something with them.
*/

struct hdrinfo	HdrInfo[] =
{
		/* originator fields, most to least significant  */
	"resent-sender",	H_FROM|H_RESENT,
	"resent-from",		H_FROM|H_RESENT,
	"resent-reply-to",	H_FROM|H_RESENT,
	"sender",		H_FROM,
	"from",			H_FROM,
	"reply-to",		H_FROM,
	"full-name",		H_ACHECK,
	"return-receipt-to",	H_FROM,
	"errors-to",		H_FROM,
		/* destination fields */
	"to",			H_RCPT,
	"resent-to",		H_RCPT|H_RESENT,
	"cc",			H_RCPT,
	"resent-cc",		H_RCPT|H_RESENT,
	"bcc",			H_RCPT|H_ACHECK,
	"resent-bcc",		H_RCPT|H_ACHECK|H_RESENT,
		/* message identification and control */
	"message-id",		H_SPACESUB,
	"resent-message-id",	H_RESENT,
	"message",		H_EOH,
	"text",			H_EOH,
		/* date fields */
	"date",			0,
	"resent-date",		H_RESENT,
		/* trace fields */
	"received",		H_TRACE|H_FORCE,
	"via",			H_TRACE|H_FORCE,
	"mail-from",		H_TRACE|H_FORCE,
	"x-nlsesc",		H_ESC,
		/* for x.400 space-substitution */
	"p1-message-id",	H_SPACESUB,
	"p1-recipient",		H_SPACESUB,
	"x400-trace",		H_SPACESUB,

	NULL,			0,
};


/*
**  ARPANET error message numbers.
*/

char	Arpa_Info[] =		"050";	/* arbitrary info */
char	Arpa_TSyserr[] =	"451";	/* some (transient) system error */
char	Arpa_PSyserr[] =	"554";	/* some (permanent) system error */
char	Arpa_Usrerr[] =		"554";	/* some (fatal) user error */


/*
**  SETDEFUSER -- set/reset DefUser using DefUid (for initgroups())
*/

setdefuser()
{
	struct passwd *defpwent;

	if (DefUser != NULL)
		free(DefUser);
	if ((defpwent = getpwuid(DefUid)) != NULL)
		DefUser = newstr(defpwent->pw_name);
	else
		DefUser = newstr("nobody");
}


/*
**  GETRUID -- get real user id (V7)
*/

getruid()
{
	if (OpMode == MD_DAEMON)
		return (RealUid);
	else
		return (getuid());
}


/*
**  GETRGID -- get real group id (V7).
*/

getrgid()
{
	if (OpMode == MD_DAEMON)
		return (RealGid);
	else
		return (getgid());
}


/*
**  USERNAME -- return the user id of the logged in user.
**
**	Parameters:
**		none.
**
**	Returns:
**		The login name of the logged in user.
**
**	Side Effects:
**		none.
**
**	Notes:
**		The return value is statically allocated.
*/

char *
username()
{
	static char *myname = NULL;
	register struct passwd *pw;

	/* cache the result */
	if (myname == NULL)
	{
		myname = getlogin();	/* get login name	*/

		/*
		 *  If login name can't be got, then try using uid
		 */
		if (myname == NULL || myname[0] == '\0')
		{
			pw = getpwuid (CurUid);
			if (pw != NULL)
				myname = newstr(pw->pw_name);
		}
		else	/* if login name exists, compare to pw file */
		{
			myname = newstr(myname);
			if ((pw = _getpwnam_shadow(myname,0)) == NULL ||
				CurUid != pw->pw_uid)
			{

			/*
			 *  If uid doesn't match or exist, then use uid instead
			 */
				pw = getpwuid (CurUid);
				if (pw != NULL)
					myname = newstr(pw->pw_name);
			}
		}

		/*
		 *  If nothing works...
		 */
		if (myname == NULL || myname[0] == '\0')
		{
			syserr(MSGSTR(CF_EUSER, "USER NAME CANNOT BE DETERMINED!")); /*MSG*/
			myname = "UNKNOWN";
		}
	}

	return (myname);
}


/*
**  TTYPATH -- Get the path of the user's tty
**
**	Returns the pathname of the user's tty.  Returns NULL if
**	the user is not logged in or if s/he has write permission
**	denied.
**
**	Parameters:
**		none
**
**	Returns:
**		pathname of the user's tty.
**		NULL if not logged in or write permission denied.
**
**	Side Effects:
**		none.
**
**	WARNING:
**		Return value is in a local buffer.
**
**	Called By:
**		savemail
*/

char *
ttypath()
{
	register char *pathn;

	/* compute the pathname of the controlling tty */
	if ((pathn = ttyname(2)) == NULL && (pathn = ttyname(1)) == NULL &&
	    (pathn = ttyname(0)) == NULL)
	{
		errno = 0;
		return (NULL);
	}

	/* see if we have write permission */
	if (accessx(pathn, W_ACC, ACC_OTHERS))
	{
		errno = 0;
		return (NULL);
	}

	/* see if the user is logged in */
	if (getlogin() == NULL)
		return (NULL);

	/* looks good */
	return (pathn);
}


/*
**  CHECKCOMPAT -- check for From and To person compatible.
**
**	This routine can be supplied on a per-installation basis
**	to determine whether a person is allowed to send a message.
**	This allows restriction of certain types of internet
**	forwarding or registration of users.
**
**	If the hosts are found to be incompatible, an error
**	message should be given using "usrerr" and FALSE should
**	be returned.
**
**	'NoReturn' can be set to suppress the return-to-sender
**	function; this should be done on huge messages.
**
**	Parameters:
**		to -- the person being sent to.
**
**	Returns:
**		TRUE -- ok to send.
**		FALSE -- not ok.
**
**	Side Effects:
**		none (unless you include the usrerr stuff)
*/

int
checkcompat(to)
	register ADDRESS *to;
{
# ifdef EXAMPLE_CODE
	/* this code is intended as an example only */
	register STAB *s;

	s = stab("arpa", ST_MAILER, ST_FIND);
	if (s != NULL && CurEnv->e_from.q_mailer != LocalMailer &&
	    to->q_mailer == s->s_mailer)
	{
		usrerr(MSGSTR(NOMAIL, "No ARPA mail through this machine: see your system administration")); /*MSG*/
		/* NoReturn = TRUE; to supress return copy */
		return (FALSE);
	}
# endif EXAMPLE_CODE
	return (TRUE);
}


/*
**  GETLA -- get the current load average
**
**	This code stolen from la.c.
**
**	Parameters:
**		none.
**
**	Returns:
**		The current load average as an integer.
**
**	Side Effects:
**		none.
*/

getla()
{
	/* can something useful be put here? */
	/* it HAS to be efficient! */

	return (0);
}



/*
**  SHOULDQUEUE -- should this message be queued or sent?
**
**	Compares the message cost to the load average to decide.
**
**	Parameters:
**		pri -- the priority of the message in question.
**
**	Returns:
**		TRUE -- if this message should be queued up for the
**			time being.
**		FALSE -- if the load is low enough to send this message.
**
**	Side Effects:
**		none.
*/

int
shouldqueue(pri)
	long pri;
{
	int la;

	la = getla();
	if (la < QueueLA)
		return (FALSE);

	/*
	 *  Smaller values of "pri" must mean higher priorities.
	 */
	return (pri > (QueueFactor / (la - QueueLA + 1)));
}


/*
**  SETPROCTITLE -- set process title for ps
**
**	Parameters:
**		fmt -- a printf style format string.
**		a, b, c -- possible parameters to fmt.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Clobbers argv of our main procedure so ps(1) will
**		display the title.
*/


setproctitle (fmt, a, b, c)
char    *fmt;
{
        static  int     once;
        static  char    buffer[MAXLINE];

        if (! once) {
                Argv[0] = buffer;
                Argv[1] = (char *) 0;
                once++;
        }
        (void) sprintf (buffer, fmt, a, b, c);
}

