static char sccsid[] = "@(#)90  1.15  src/bos/usr/bin/logger/logger.c, cmdmisc, bos41B, 9504A 1/4/95 14:12:09";
/*
 * COMPONENT_NAME: (CMDMISC) miscellaneous commands
 *
 * FUNCTIONS: logger
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <sys/syslog.h>
#include <ctype.h>
#include <pwd.h>
#include <sys/id.h>

#include "logger_msg.h" 
static nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_LOGGER,n,s) 

struct code {
	char	*c_name;
	int	c_val;
};

static struct code	PriNames[] = {
	"panic",	LOG_EMERG,
	"emerg",	LOG_EMERG,
	"alert",	LOG_ALERT,
	"crit",		LOG_CRIT,
	"err",		LOG_ERR,
	"error",	LOG_ERR,
	"warn",		LOG_WARNING,
	"warning",	LOG_WARNING,
	"notice",	LOG_NOTICE,
	"info",		LOG_INFO,
	"debug",	LOG_DEBUG,
	NULL,		-1
};

static struct code	FacNames[] = {
	"kern",		LOG_KERN,
	"user",		LOG_USER,
	"mail",		LOG_MAIL,
	"daemon",	LOG_DAEMON,
	"auth",		LOG_AUTH,
	"security",	LOG_AUTH,
	"syslog",	LOG_SYSLOG,
	"lpr",		LOG_LPR,
	"news",		LOG_NEWS,
	"uucp",		LOG_UUCP,
	"local0",	LOG_LOCAL0,
	"local1",	LOG_LOCAL1,
	"local2",	LOG_LOCAL2,
	"local3",	LOG_LOCAL3,
	"local4",	LOG_LOCAL4,
	"local5",	LOG_LOCAL5,
	"local6",	LOG_LOCAL6,
	"local7",	LOG_LOCAL7,
	NULL,		-1
};

/*
**  LOGGER -- read and log utility
**
**	This routine reads from an input and arranges to write the
**	result on the system log, along with a useful tag.
*/

main(argc, argv)
	int argc;
	char **argv;
{
	char buf[LINE_MAX + 1];
	char *tag;
	int pri = LOG_NOTICE;
	int logflags = 0, c;
	struct passwd *tmp_pwent;

	/* initialize */

	(void ) setlocale(LC_ALL,"");
	catd = catopen(MF_LOGGER, NL_CAT_LOCALE);
 	if((tmp_pwent = getpwuid(getuidx(ID_LOGIN))) == NULL)
 		tag = NULL;
 	else
 		tag = tmp_pwent->pw_name;

	while ((c=getopt(argc,argv,"f:ip:t:")) != EOF)
		switch(c) {
			case 'f':
				if (freopen(optarg, "r", stdin) == NULL) {
					perror("logger");
					exit(1);
				}
				break;
			case 'i':
				logflags |= LOG_PID;
				break;
		  	case 'p':		/* priority */
				pri = pencode(optarg);
				break;
		  	case 't':		/* tag */
				tag = optarg;
				break;
			default:
				fprintf(stderr, MSGSTR(USAGE, "Usage: logger [-f File] [-i] [-p Priority] [-t Tag] [Message]\n"));
				exit(1);
		}

	/* setup for logging */
	openlog(tag, logflags, 0);
	(void) fclose(stdout);

	if (optind < argc) {
		while (optind < argc) {
			c = 0;
			bzero(buf, sizeof(buf));
			while (argv[optind] && (c += strlen(argv[optind]) + 1) <= 1024) {
				strcat(buf, argv[optind++]);
				strcat(buf, " ");
			}
			if (buf[0])
				syslog(pri, buf);
			else
				syslog(pri, argv[optind++]);
		}
		exit(0);
	}

	/* main loop */
	while (fgets(buf, LINE_MAX, stdin) != NULL)
		syslog(pri, buf);

	exit(0);
}

/*
 *  Decode a symbolic name to a numeric value
 */

static pencode(s)
	register char *s;
{
	register char *p;
	int lev;
	int fac;
	char buf[100];

	for (p = buf; *s && *s != '.'; )
		*p++ = *s++;
	*p = '\0';
	if (*s++) {
		fac = decode(buf, FacNames);
		if (fac < 0)
			bailout(MSGSTR(UNKFACILITY, "unknown facility name: "), buf);
		for (p = buf; *p++ = *s++; )
			continue;
	} else
		fac = 0;
	lev = decode(buf, PriNames);
	if (lev < 0)
		bailout(MSGSTR(UNKPRIORITY, "unknown priority name: "), buf);

	return ((lev & LOG_PRIMASK) | (fac & LOG_FACMASK));
}


/*
 * NAME: decode
 * FUNCTION: search code table for name, return its value
 */
static decode(name, codetab)
	char *name;
	struct code *codetab;
{
	register struct code *c;
	register char *p;
	char buf[40];

	if (isdigit(*name))
		return (atoi(name));

	(void) strcpy(buf, name);
	for (p = buf; *p; p++)
		if (isupper(*p))
			*p = tolower(*p);
	for (c = codetab; c->c_name; c++)
		if (!strcmp(buf, c->c_name))
			return (c->c_val);
	return (-1);
}

/*
 * NAME: bailout
 * FUNCTION: display error message and exit
 */
static bailout(a, b)
	char *a, *b;
{
	fprintf(stderr, "logger: %s%s\n", a, b);
	exit(1);
}
