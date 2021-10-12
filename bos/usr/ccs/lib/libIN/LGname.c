static char sccsid[] = "@(#)98	1.6  src/bos/usr/ccs/lib/libIN/LGname.c, libIN, bos411, 9428A410j 6/10/91 10:19:40";
/*
 * LIBIN: LGname
 *
 * ORIGIN: 9
 *
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1985, 1989
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 *
 * FUNCTION: Return the user's login name.
 *
 *      If UINFO is defined in uinfo.h (i.e. if this system has the
 *      usrinfo system call), then this version of LGname calls
 *      getuinfo() to get the user information.  It returns
 *      the address of the character following the leading "NAME=" in
 *      the user information string.  If the string returned by getuinfo()
 *      does not begin with "NAME=", "UNKNOWN" is returned.
 *
 *
 *      If this system has no usrinfo system call, then this version of
 *      LGname searches /etc/utmp for an entry matching getenv("LOGNAME"),
 *      with a terminal name matching the terminal open on fd0, fd2, or fd1.
 *      It also sets _LGdbuf to the terminal name for use by LGdev.
 *
 *      If no such entry is found, the user name of the real user ID is
 *      returned.  If the real user ID cannot be located in the password file,
 *      "UNKNOWN" is returned.  _LGdbuf is set to "/dev/null".
 *
 *      Note that the results of this routine are only approximate,
 *      and may be subverted by conscious action on the user's part.
 *
 */

#include <stdio.h>
#include <uinfo.h>

#ifndef UINFO

#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <utmp.h>
#include <IN/standard.h>
#include <IN/CSdefs.h>

#define DEVLEN  (sizeof ((struct utmp *)0)->ut_line)
#define NAMLEN  (sizeof ((struct utmp *)0)->ut_name)

#define INIT    0
#define CHECK   1
#define FOUND   2
#define NONE    3

static char tfds[] = {0, 2, 1};

#define NTTYS   ARRAYLEN(tfds)

typedef struct {
	char    tflag;
	char    tdev[DEVLEN];
	char    tnam[NAMLEN];
	ino_t   tino;
} Tty;

#define DV  5        /* sizeof "/dev/" */

char    _LGdbuf[DV+DEVLEN+1] = "/dev/null"; /* save for LGdev */

#endif

char *
LGname ()
{
#ifdef UINFO

extern   char *getuinfo();
register char *cp = getuinfo("NAME");

	return ( (cp && *cp) ? cp : "UNKNOWN" );

#else
	static char nambuf[NAMLEN+1];
	struct utmp ubuf;
	struct stat devstat, fdstat;
	Tty ttys[NTTYS];
	register int tn;
	register Tty *tp;
	register int fd;
	register char *np;
	register highest = -1;
	register struct passwd *pw;
	extern struct passwd *getpwuid();
	extern char *getenv();

	if( *nambuf )
	    return nambuf;

	for( tn = 0; tn < NTTYS; tn++ )
	    ttys[tn].tflag = INIT;

	if( (np = getenv("LOGNAME")) && (fd = open("/etc/utmp", 0)) >= 0 )
next:   {   while( read(fd, (char *)&ubuf, sizeof ubuf) == sizeof ubuf )
	    {   if( *ubuf.ut_name == NUL )
		    continue;
		CAcpy(nambuf, ubuf.ut_name, NAMLEN);
		if( CScmp(np, nambuf) != 0 )
		    continue;

		CAcpy(_LGdbuf+DV, ubuf.ut_line, DEVLEN);
		if( stat(_LGdbuf, &devstat) < 0 )
		    continue;
		for( tn = 0; tn < NTTYS; tn++ )
		    switch( (tp = &ttys[tn])->tflag )
		    {   case INIT:
			    if( fstat(tfds[tn], &fdstat) < 0
				    || (fdstat.st_mode&S_IFMT) != S_IFCHR
				    || fdstat.st_dev != devstat.st_dev )
			    {   tp->tflag = NONE;
				continue;
			    }
			    tp->tino = fdstat.st_ino;
			    tp->tflag = CHECK;
			    if( highest < 0 ) highest = tn;
			    /* fall through */
			case CHECK:
			    if( devstat.st_ino == tp->tino )
			    {   if( tn == highest )
				{   close(fd);
				    return nambuf;
				}
				CAcpy(tp->tdev, ubuf.ut_line, DEVLEN);
				CAcpy(tp->tnam, ubuf.ut_name, NAMLEN);
				tp->tflag = FOUND;
				goto next;
			    }
		    }
	    }
	    close(fd);
	}
	for( tn = 0; tn < NTTYS; tn++ )
	    if( (tp = &ttys[tn])->tflag == FOUND )
	    {   CAcpy(_LGdbuf+DV, tp->tdev, DEVLEN);
		CAcpy(nambuf, tp->tnam, NAMLEN);
		return nambuf;
	    }

	CScpy(nambuf,
		(pw = getpwuid(getuid()))? pw->pw_name : "UNKNOWN");
	return nambuf;
#endif
}
