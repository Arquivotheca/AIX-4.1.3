static char sccsid[] = "@(#)88	1.9  src/bos/usr/ccs/lib/libc/syslog.c, libcproc, bos411, 9428A410j 6/3/94 15:20:42";
#ifdef _POWER_PROLOG_
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: syslog, setlogmask, openlog, closelog
 *
 * ORIGINS: 26,27,71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#endif /* _POWER_PROLOG_ */

/*
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */


/*
 * FUNCTION: print message on log file
 */

/*
 * SYSLOG -- print message on log file
 *
 * This routine looks a lot like printf, except that it
 * outputs to the log file instead of the standard output.
 * Also:
 *	adds a timestamp,
 *	prints the module name in front of the message,
 *	has some other formatting types (or will sometime),
 *	adds a newline on the end of the message.
 *
 * The output of this routine is intended to be read by /etc/syslogd.
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/signal.h>
#include <sys/syslog.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>
#include <stdarg.h>
#include "libc_msg.h"
#include <time.h>
#include <sys/errno.h>

#define	MAXLINE	1024			/* max message size */

#define PRIMASK(p)	(1 << ((p) & LOG_PRIMASK))
#define PRIFAC(p)	(((p) & LOG_FACMASK) >> 3)
#define IMPORTANT 	LOG_ERR

static char	logname[] = "/dev/log";
static char	ctty[] = "/dev/console";

#ifdef _THREAD_SAFE

#define	OPENLOG(i,s,f)	openlog_r(i, s, f, syslog_data)

#define LogFile 	(syslog_data->log_file)
#define LogStat 	(syslog_data->log_stat)
#define LogTag 		(syslog_data->log_tag)
#define LogMask 	(syslog_data->log_mask)
#define LogFacility 	(syslog_data->log_facility)
#define SyslogAddr 	(syslog_data->syslog_addr)

#else

#define	OPENLOG	openlog

static int	LogFile = -1;		/* fd for log */
static int	LogStat	= 0;		/* status bits, set by openlog() */
static char	*LogTag = "syslog";	/* string to tag the entry with */
static int	LogMask = 0xff;		/* mask of priorities to be logged */
static int	LogFacility = LOG_USER;	/* default facility code */
static struct sockaddr SyslogAddr;	/* AF_UNIX address of local logger */

#endif /* _THREAD_SAFE */

/* Sockets are per-process */
static int	connected;		/* (sockets) have done connect */

#ifdef _THREAD_SAFE
int
syslog_r(int pri, struct syslog_data *syslog_data, const char *fmt, ...)

#else
int
syslog(int pri, const char *fmt, ...)
#endif	/* _THREAD_SAFE */
{
	char buf[MAXLINE + 1], outline[MAXLINE + 1];
	register char *b, *o;
	register const char *f;
	register int c;
	time_t now;
	pid_t pid;
	int olderrno = errno;
	va_list arg_list;

#ifdef _THREAD_SAFE
	char tmpstr[64];
#endif	/* _THREAD_SAFE */

	/* see if we should just throw out this message */
	if (pri <= 0 || PRIFAC(pri) >= LOG_NFACILITIES || (PRIMASK(pri) & LogMask) == 0)
		return(-1);
	if (LogFile < 0 || !connected)
		OPENLOG(LogTag, LogStat | LOG_NDELAY, 0);

	/* set default facility if none specified */
	if ((pri & LOG_FACMASK) == 0)
		pri |= LogFacility;

	/* build the message */
	o = outline;
	sprintf(o, "<%d>", pri);
	o += strlen(o);
	time(&now);
#ifdef _THREAD_SAFE
	sprintf(o, "%.15s ", ctime_r(&now, tmpstr) + 4);
#else
	sprintf(o, "%.15s ", ctime(&now) + 4);
#endif	/* _THREAD_SAFE */
	o += strlen(o);
	if (LogTag) {
		strcpy(o, LogTag);
		o += strlen(o);
	}
	if (LogStat & LOG_PID) {
		sprintf(o, "[%d]", getpid());
		o += strlen(o);
	}
	if (LogTag) {
		strcpy(o, ": ");
		o += 2;
	}

	/* f is the first element of the variable list
	   the rest of the variable list is stored in arg_list
	 */

	b = buf;

	f = fmt;

	while ((c = *f++) != '\0' && c != '\n' && b < &buf[MAXLINE]) {
		if (c != '%') {
			*b++ = c;
			continue;
		}
		if ((c = *f++) != 'm') {
			*b++ = '%';
			*b++ = c;
			continue;
		}
#ifdef _THREAD_SAFE
		strerror_r(olderrno, b, sizeof(buf) - (b - buf));
#else
		strcpy(b, strerror(olderrno));
#endif	/* _THREAD_SAFE */
		b += strlen(b);
	}
	*b++ = '\n';
	*b = '\0';
	va_start(arg_list, fmt);
	vsprintf(o, buf, arg_list);
	va_end(arg_list);
	c = strlen(outline);
	if (c > MAXLINE)
		c = MAXLINE;

	/* output the message to the local logger */
	if (send(LogFile, outline, c, 0) >= 0)
		return(0);
        else {
            /*
             * This was added because since we are now doing the
             * connect inside openlog, if syslogd dies, and someone
             * has already done a connect, they won't be able to log
             * to syslogd.  There are 2 different cases here,
             * ECONNRESET, which is the error you will get the first
             * time that you try to send something after syslogd died.
             *  After that, you will get EDESTADDRREQ, so we will need
             *  to check for both of them, and try to do an openlog
             *  and try to send again.  If we fail then, it's really
             *  dead, and we will follow through with the old logic.
             */
            if (errno == ECONNRESET || errno == EDESTADDRREQ) {
                LogFile = -1;
                connected = 0;
                openlog(LogTag, LogStat | LOG_NDELAY, 0);
                if (send(LogFile, outline, c, 0) >= 0) {
                    return(0);
                }
            }
        }
	if (!(LogStat & LOG_CONS))
		return(0);

	/* output the message to the console */
	pid = __fork();
	if (pid == -1)
		return(-1);
	if (pid == 0) {
		int fd;

		signal(SIGALRM, SIG_DFL);
		sigsetmask(sigblock(0) & ~sigmask(SIGALRM));
		alarm(5);
		fd = open(ctty, O_WRONLY);
		alarm(0);
		strcat(o, "\r");
		o = strchr(outline, '>') + 1;
		write(fd, o, c + 1 - (o - outline));
		close(fd);
		_exit(0);
	}
	if (!(LogStat & LOG_NOWAIT))
	        if (waitpid(pid, NULL, 0) == -1)
			return (-1);
	return (0);
}

/*
 * OPENLOG -- open system log
 */
#ifdef _THREAD_SAFE
int
openlog_r(const char *ident, int logstat, int logfac,
	  struct syslog_data *syslog_data)
#else
int
openlog(const char *ident, int logstat, int logfac)
#endif /* _THREAD_SAFE */
{
	if (ident != NULL)
		LogTag = (char *)ident;
	LogStat = logstat;
	if (logfac != 0)
		LogFacility = logfac & LOG_FACMASK;
	if (LogFile >= 0)
		return(0);
	SyslogAddr.sa_family = AF_UNIX;
	strncpy(SyslogAddr.sa_data, logname, (size_t)sizeof SyslogAddr.sa_data);
	if (LogStat & LOG_NDELAY) {
		LogFile = socket(AF_UNIX, SOCK_DGRAM, 0);
		if (LogFile < 0)
			return 0;
		fcntl(LogFile, F_SETFD, 1);
	}
	if (LogFile != -1 && !connected && connect(LogFile, &SyslogAddr, sizeof(SyslogAddr)) != -1) 
		connected = 1;
	return (0);
}

/*
 * CLOSELOG -- close the system log
 */
#ifdef _THREAD_SAFE
void closelog_r(struct syslog_data *syslog_data)
#else
void closelog(void)
#endif	/* _THREAD_SAFE */
{

	(void) close(LogFile);
	LogFile = -1;
	connected = 0;
}

/*
 * SETLOGMASK -- set the log mask level
 */
#ifdef _THREAD_SAFE
int
setlogmask_r(int pmask, struct syslog_data *syslog_data)
#else
int
setlogmask(int pmask)
#endif	/* _THREAD_SAFE */
{
	int omask;

	omask = LogMask;
	if (pmask != 0)
		LogMask = pmask;
	return (omask);
}
