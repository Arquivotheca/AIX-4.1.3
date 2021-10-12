static char sccsid[] = "@(#)46	1.6  src/bos/usr/bin/uucp/xqt.c, cmduucp, bos41J, 9515A_all 4/11/95 14:09:22";
/* 
 * COMPONENT_NAME: CMDUUCP xqt.c
 * 
 * FUNCTIONS: euucico, xuucico, xuuxqt 
 *
 * ORIGINS: 10  27  3 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1995
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

/*	/sccs/src/cmd/uucp/s.xqt.c
	xqt.c	1.1	7/29/85 16:34:28
*/
#include "uucp.h"
/* VERSION( xqt.c	5.2 -  -  ); */
extern void close_log();

static euucico();

/*
 * start up uucico for rmtname
 * return:
 *	none
 */
void
xuucico(rmtname)
char *rmtname;
{
	/*
	 * start uucico for rmtname system
	 */
	if (fork() == 0) {	/* can't vfork() */
		/*
		 * hide the uid of the initiator of this job so that he
		 * doesn't get notified about things that don't concern him.
		 */
		/* not anymore we don't: the process must be owned by
		   the user for quota purposes
		(void) setuid(geteuid());
		*/
		euucico(rmtname);
	}
	return;
}

static
euucico(rmtname)
char	*rmtname;
{
	char opt[100];

	(void) close(0);
	(void) close(1);
	(void) close(2);
	(void) open("/dev/null", 0);
	(void) open("/dev/null", 1);
	(void) open("/dev/null", 1);
	(void) signal(SIGINT, SIG_IGN);
	(void) signal(SIGHUP, SIG_IGN);
	(void) signal(SIGQUIT, SIG_IGN);
	close_log();
	if (rmtname[0] != '\0')
		(void) sprintf(opt, "-s%s", rmtname);
	else
		opt[0] = '\0';
	(void) execle(UUCICO, "UUCICO", "-r1", opt, 0, Env);
	exit(100);
}


/*
 * start up uuxqt
 * return:
 *	none
 */
void
xuuxqt(rmtname, old_behavior)
char	*rmtname;
int      old_behavior;
{
	char	opt[100];

	if (rmtname && rmtname[0] != '\0')
		(void) sprintf(opt, "-s%s", rmtname);
	else
		opt[0] = '\0';

	/*
	 * start uuxqt
	 */
	if (vfork() == 0) {
		(void) close(0);
		(void) close(1);
		(void) close(2);
		(void) open("/dev/null", 2);
		(void) open("/dev/null", 2);
		(void) open("/dev/null", 2);
		(void) signal(SIGINT, SIG_IGN);
		(void) signal(SIGHUP, SIG_IGN);
		(void) signal(SIGQUIT, SIG_IGN);
		closelog();
		/*
		 * hide the uid of the initiator of this job so that he
		 * doesn't get notified about things that don't concern him.
		 */
		(void) setuid(geteuid());
		if (rmtname && rmtname[0] != '\0') {
			(void) sprintf(opt, "-s%s", rmtname);
			if (old_behavior)
				(void) execle(UUXQT, "UUXQT", opt, "-e", (char *) 0, Env);
			else
				(void) execle(UUXQT, "UUXQT", opt, (char *) 0, Env);
		}
		else {
			if (old_behavior)
				(void) execle(UUXQT, "UUXQT", "-e", (char *) 0, Env);
			else
				(void) execle(UUXQT, "UUXQT", (char *) 0, Env);
		}
		(void) _exit(100);
	}
	return;
}
