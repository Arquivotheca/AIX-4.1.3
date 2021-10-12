static char sccsid[] = "@(#)05	1.7  src/bos/usr/bin/uucp/logent.c, cmduucp, bos411, 9428A410j 6/17/93 14:21:44";
/* 
 * COMPONENT_NAME: CMDUUCP logent.c
 * 
 * FUNCTIONS: close_log, logent, millitick, uusyslog 
 *
 * ORIGINS: 10  27  3 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	/sccs/src/cmd/uucp/s.logent.c
	logent.c	1.2	7/29/85 16:33:13
*/
#include "uucp.h"
/* VERSION( logent.c	5.2 -  -  ); */

extern nl_catd catd;
static FILE	*_Lf = NULL;
static int	_Sf = -1;

/*
 * Make log entry
 *	text	-> ptr to text string
 *	status	-> ptr to status string
 * Returns:
 *	none
 */
void
logent(text, status)
register char	*text, *status;
{
	static	char	logfile[MAXFULLNAME];

	if (*Rmtname == NULLCHAR) /* ignore logging if Rmtname is not yet set */
		return;
	if (Nstat.t_pid == 0)
		Nstat.t_pid = getpid();

	if (_Lf != NULL
	   && strncmp(Rmtname, BASENAME(logfile, '/'), SYSNSIZE) != 0) {
		fclose(_Lf);
		_Lf = NULL;
	}

	if (_Lf == NULL) {
		sprintf(logfile, "%s/%s", Logfile, Rmtname);
		_Lf = fopen(logfile, "a");
		(void) chmod(logfile, 0644);
		if (_Lf == NULL)
			return;
		setbuf(_Lf, CNULL);
	}
	(void) fseek(_Lf, 0L, 2);
	(void) fprintf(_Lf, "%s %s %s ", User, Rmtname, Jobid);
	(void) fprintf(_Lf, "(%s,%d,%d) ", timeStamp(), Nstat.t_pid, Seqn);
	(void) fprintf(_Lf, "%s (%s)\n", status, text);
	return;
}


/*
 * Make entry for a conversation (uucico only)
 *	text	-> pointer to message string
 * Returns:
 *	none
 */
void
uusyslog(text)
register char	*text;
{
	int	sbuflen;
	char	sysbuf[BUFSIZ];

	(void) sprintf(sysbuf, "%s!%s %s (%s) (%c,%d,%d) [%s] %s\n",
		Rmtname, User, Role == SLAVE ? "S" : "M", timeStamp(),
		Pchar, getpid(), Seqn, Dc, text);
	sbuflen = strlen(sysbuf);
	if (_Sf < 0) {
		errno = 0;
		_Sf = open(SYSLOG, 1);
		if (errno == ENOENT) {
			_Sf = creat(SYSLOG, 0644);
			(void) chmod(SYSLOG, 0644);
		}
		if (_Sf < 0)
			return;
	}
	(void) lseek(_Sf, 0L, 2);
	(void) write(_Sf, sysbuf, sbuflen);
	return;
}


/*
 * Close log files before a fork
 */
void
close_log()
{
	if (_Sf >= 0) {
		(void) close(_Sf);
		_Sf = -1;
	}
	if (_Lf) {
		(void) fclose(_Lf);
		_Lf = NULL;
	}
}

/*
 *	millitick()
 *
 *	return msec since last time called
 */
#ifdef ATTSV
time_t
millitick()
{
	struct tms	tbuf;
	time_t	now, rval;
	static time_t	past;	/* guaranteed 0 first time called */

	if (past == 0) {
		past = times(&tbuf);
		rval = 0;
	} else {
		rval = ((now = times(&tbuf)) - past) * 1000 / HZ;
		past = now;
	}
	return(rval);
}

#else !ATTSV
time_t
millitick()
{
	struct timeb	tbuf;
	static struct timeb	tbuf1;
	static past;		/* guaranteed 0 first time called */
	time_t	rval;

	if (past == 0) {
		past++;
		rval = 0;
		ftime(&tbuf1);
	} else {
		ftime(&tbuf);
		rval = (tbuf.time - tbuf1.time) * 1000
			+ tbuf.millitm - tbuf1.millitm;
		tbuf1 = tbuf;
	}
	return(rval);
}
#endif ATTSV
