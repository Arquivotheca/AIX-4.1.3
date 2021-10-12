static char sccsid[] = "@(#)38	1.5  src/bos/usr/bin/tip/log.c, cmdtip, bos411, 9428A410j 4/10/91 09:06:22";
/* 
 * COMPONENT_NAME: UUCP log.c
 * 
 * FUNCTIONS: MSGSTR, flock, logent, loginit 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/* static char sccsid[] = "log.c	5.1 (Berkeley) 4/30/85"; */

#include <fcntl.h>
#include "tip.h"
#include <nl_types.h>
#include "tip_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TIP,n,s) 

#ifdef _AIX

flock(dummy1, dummy2)
int 	dummy1, dummy2;
{
	return 1;
}
#endif
/********************************************************************/

static	FILE *flog = NULL;

/*
 * Log file maintenance routines
 */

logent(group, num, acu, message)
	char *group, *num, *acu, *message;
{
	char *user, *timestamp;
	struct passwd *pwd;
	long t;

	struct flock flock_arg;
	flock_arg.l_type = F_WRLCK;
	flock_arg.l_whence = flock_arg.l_start = flock_arg.l_len = 0;

	if (flog == NULL)
		return;
	if (fcntl(fileno(flog), F_SETLK, &flock_arg) < 0) {
		perror(MSGSTR(FLOCK, "tip: flock")); /*MSG*/
		return;
	}
	if ((user = getlogin()) == NOSTR)
		if ((pwd = getpwuid(getuid())) == NOPWD)
			user = "???";
		else
			user = pwd->pw_name;
	t = time(0);
	timestamp = ctime(&t);
	timestamp[24] = '\0';
	fprintf(flog, "%s (%s) <%s, %s, %s> %s\n",
		user, timestamp, group,
#ifdef PRISTINE
		"",
#else
		num,
#endif
		acu, message);
	fflush(flog);
	flock_arg.l_type = F_UNLCK;
	(void) fcntl(fileno(flog), F_SETLK, &flock_arg);
}

loginit()
{

#ifdef ACULOG
	flog = fopen(value(LOG), "a");
	if (flog == NULL)
		fprintf(stderr, MSGSTR(CANTOPEN2, "can't open log file\r\n")); /*MSG*/
#endif
}
