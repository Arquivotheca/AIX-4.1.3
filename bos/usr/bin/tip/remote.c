static char sccsid[] = "@(#)41	1.5  src/bos/usr/bin/tip/remote.c, cmdtip, bos411, 9428A410j 4/10/91 09:07:14";
/* 
 * COMPONENT_NAME: UUCP remote.c
 * 
 * FUNCTIONS: MSGSTR, getremcap, getremote 
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

/* static char sccsid[] = "remote.c	5.3 (Berkeley) 4/30/86"; */

# include "tip.h"
#include <nl_types.h>  
#include "tip_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TIP,n,s) 

/*
 * Attributes to be gleened from remote host description
 *   data base.
 */
static char **caps[] = {
	&AT, &DV, &CM, &CU, &EL, &IE, &OE, &PN, &PR, &DI,
	&ES, &EX, &FO, &RC, &RE, &PA
};

static char *capstrings[] = {
	"at", "dv", "cm", "cu", "el", "ie", "oe", "pn", "pr",
	"di", "es", "ex", "fo", "rc", "re", "pa", 0
};

char *rgetstr();

static
getremcap(host)
	register char *host;
{
	int stat;
	char tbuf[BUFSIZ];
	static char buf[BUFSIZ/2];
	char *bp = buf;
	register char **p, ***q;

	if ((stat = rgetent(tbuf, host)) <= 0) {
		if (DV ||
		    host[0] == '/' && access(DV = host, R_OK | W_OK) == 0) {
			CU = DV;
			HO = host;
			HW = 1;
			DU = 0;
			if (!BR)
				BR = DEFBR;
			FS = DEFFS;
			return;
		}
		fprintf(stderr, stat == 0 ?
			MSGSTR(UNKNOWNHOST, "tip: unknown host %s\n") : /*MSG*/
			MSGSTR(CANTOPEN3, "tip: can't open host description file\n"), host); /*MSG*/
		exit(3);
	}

	for (p = capstrings, q = caps; *p != NULL; p++, q++)
		if (**q == NULL)
			**q = rgetstr(*p, &bp);
	if (!BR && (BR = rgetnum("br")) < 0)
		BR = DEFBR;
	if ((FS = rgetnum("fs")) < 0)
		FS = DEFFS;
	DU = rgetflag("du");
	if (DV == NOSTR) {
		fprintf(stderr, MSGSTR(NODEVSPEC, "%s: missing device spec\n"), host); /*MSG*/
		exit(3);
	}
	if (DU && CU == NOSTR)
		CU = DV;
	if (DU && PN == NOSTR) {
		fprintf(stderr, MSGSTR(NOPHONE, "%s: missing phone number\n"), host); /*MSG*/
		exit(3);
	}

	HD = rgetflag("hd");

	/*
	 * This effectively eliminates the "hw" attribute
	 *   from the description file
	 */
	if (!HW)
		HW = (CU == NOSTR) || (DU && equal(DV, CU));
	HO = host;
	/*
	 * see if uppercase mode should be turned on initially
	 */
	if (rgetflag("ra"))
		boolean(value(RAISE)) = 1;
	if (rgetflag("ec"))
		boolean(value(ECHOCHECK)) = 1;
	if (rgetflag("be"))
		boolean(value(BEAUTIFY)) = 1;
	if (rgetflag("nb"))
		boolean(value(BEAUTIFY)) = 0;
	if (rgetflag("sc"))
		boolean(value(SCRIPT)) = 1;
	if (rgetflag("tb"))
		boolean(value(TABEXPAND)) = 1;
	if (rgetflag("vb"))
		boolean(value(VERBOSE)) = 1;
	if (rgetflag("nv"))
		boolean(value(VERBOSE)) = 0;
	if (rgetflag("ta"))
		boolean(value(TAND)) = 1;
	if (rgetflag("nt"))
		boolean(value(TAND)) = 0;
	if (rgetflag("rw"))
		boolean(value(RAWFTP)) = 1;
	if (rgetflag("hd"))
		boolean(value(HALFDUPLEX)) = 1;
	if (RE == NOSTR)
		RE = (char *)"tip.record";
	if (EX == NOSTR)
		EX = (char *)"\t\n\b\f";
	if (ES != NOSTR)
		vstring("es", ES);
	if (FO != NOSTR)
		vstring("fo", FO);
	if (PR != NOSTR)
		vstring("pr", PR);
	if (RC != NOSTR)
		vstring("rc", RC);
	if ((DL = rgetnum("dl")) < 0)
		DL = 0;
	if ((CL = rgetnum("cl")) < 0)
		CL = 0;
	if ((ET = rgetnum("et")) < 0)
		ET = 10;
}

char *
getremote(host)
	char *host;
{
	register char *cp;
	static char *next;
	static int lookedup = 0;

	if (!lookedup) {
		if (host == NOSTR && (host = getenv("HOST")) == NOSTR) {
			fprintf(stderr, MSGSTR(NOHOST, "tip: no host specified\n")); /*MSG*/
			exit(3);
		}
		getremcap(host);
		next = DV;
		lookedup++;
	}
	/*
	 * We return a new device each time we're called (to allow
	 *   a rotary action to be simulated)
	 */
	if (next == NOSTR)
		return (NOSTR);
	if ((cp = index(next, ',')) == NULL) {
		DV = next;
		next = NOSTR;
	} else {
		*cp++ = '\0';
		DV = next;
		next = cp;
	}
	return (DV);
}
