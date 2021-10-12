static char sccsid[] = "@(#)36	1.7  src/bos/usr/bin/tip/cu.c, cmdtip, bos411, 9428A410j 4/10/91 09:06:17";
/* 
 * COMPONENT_NAME: UUCP cu.c
 * 
 * FUNCTIONS: MSGSTR, cumain 
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

/* static char sccsid[] = "cu.c	5.2 (Berkeley) 1/13/86"; */

#include "tip.h"
#include <nl_types.h>
#include "tip_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TIP,n,s) 

void	cleanup(int);
int	timeout();

/*
 * Botch the interface to look like cu's
 */
cumain(argc, argv)
	char *argv[];
{
	register int i;
	static char sbuf[12];

	if (argc < 2) {
		printf(MSGSTR(USAGE3, "usage: cu telno [-t] [-s speed] [-a acu] [-l line] [-#]\n")); /*MSG*/
		exit(8);
	}
	CU = DV = NOSTR;
	for (; argc > 1; argv++, argc--) {
		if (argv[1][0] != '-')
			PN = argv[1];
		else switch (argv[1][1]) {

		case 't':
			HW = 1, DU = -1;
			--argc;
			continue;

		case 'a':
			CU = argv[2]; ++argv; --argc;
			break;

		case 's':
			if (speed(atoi(argv[2])) == 0) {
				fprintf(stderr, MSGSTR(NOSUPPORT, "cu: unsupported speed %s\n"), /*MSG*/
					argv[2]);
				exit(3);
			}
			BR = atoi(argv[2]); ++argv; --argc;
			break;

		case 'l':
			DV = argv[2]; ++argv; --argc;
			break;

		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			if (CU)
				CU[strlen(CU)-1] = argv[1][1];
			if (DV)
				DV[strlen(DV)-1] = argv[1][1];
			break;

		default:
			printf(MSGSTR(BADFLAG, "Bad flag %s"), argv[1]); /*MSG*/
			break;
		}
	}
	signal(SIGINT, (void(*)(int)) cleanup);
	signal(SIGQUIT, (void(*)(int)) cleanup);
	signal(SIGHUP, (void(*)(int)) cleanup);
	signal(SIGTERM, (void(*)(int)) cleanup);

	/*
	 * The "cu" host name is used to define the
	 * attributes of the generic dialer.
	 */
	if ((i = hunt(sprintf(sbuf, "cu%d", BR))) == 0) {
		printf(MSGSTR(ALLBUSY, "all ports busy\n")); /*MSG*/
		exit(3);
	}
	if (i == -1) {
		printf(MSGSTR(LINKDOWN, "link down\n")); /*MSG*/
		ttyunlock(uucplock);
		exit(3);
	}
	setbuf(stdout, NULL);
	loginit();
	gid = getgid();
	egid = getegid();
	uid = getuid();
	euid = geteuid();
	setregid(egid, gid);
	setreuid(euid, uid);
	vinit();
	setparity("none");
	boolean(value(VERBOSE)) = 0;
	if (HW)
		ttysetup(speed(BR));
	if (connect()) {
		printf(MSGSTR(CONNECTFAIL, "Connect failed\n")); /*MSG*/
		setreuid(uid, euid);
		setregid(gid, egid);
		ttyunlock(uucplock);
		exit(1);
	}
	if (!HW)
		ttysetup(speed(BR));
}
