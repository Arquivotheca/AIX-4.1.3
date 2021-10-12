static char sccsid[] = "@(#)31	1.9  src/bos/usr/bin/tip/acu.c, cmdtip, bos411, 9428A410j 3/11/94 10:16:25";
/* 
 * COMPONENT_NAME: UUCP acu.c
 * 
 * FUNCTIONS: MSGSTR, acuabort, acutype, connect, disconnect 
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

/* Static char sccsid[] = "acu.c	5.3 (Berkeley) 4/3/86"; */

#include "tip.h"

static acu_t *acu = NOACU;
static int conflag;
static void acuabort(int);
static acu_t *acutype();
static jmp_buf jmpbuf;

#include <nl_types.h>
#include "tip_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TIP,n,s) 

/*
 * Establish connection for tip
 *
 * If DU is true, we should dial an ACU whose type is AT.
 * The phone numbers are in PN, and the call unit is in CU.
 *
 * If the PN is an '@', then we consult the PHONES file for
 *   the phone numbers.  This file is /etc/phones, unless overriden
 *   by an exported shell variable.
 *
 * The data base files must be in the format:
 *	host-name[ \t]*phone-number
 *   with the possibility of multiple phone numbers
 *   for a single host acting as a rotary (in the order
 *   found in the file).
 */
char *
connect()
{
	register char *cp = PN;
	char *phnum, string[256];
	FILE *fd;
	int tried = 0;

	if (!DU) {		/* regular connect message */
		if (CM != NOSTR)
			pwrite(FD, CM, size(CM));
		return (NOSTR);
	}
	/*
	 * @ =>'s use data base in PHONES environment variable
	 *        otherwise, use /etc/phones
	 */
	signal(SIGINT, (void(*)(int)) acuabort);
	signal(SIGQUIT, (void(*)(int)) acuabort);
	if (setjmp(jmpbuf)) {
		signal(SIGINT, SIG_IGN);
		signal(SIGQUIT, SIG_IGN);
		printf(MSGSTR(CALLABORT, "\ncall aborted\n")); /*MSG*/
		logent(value(HOST), "", "", MSGSTR(CALLABORT2, "call aborted")); /*MSG*/
		if (acu != NOACU) {
			boolean(value(VERBOSE)) = FALSE;
			if (conflag)
				disconnect(NOSTR);
			else
				(*acu->acu_abort)();
		}
		ttyunlock(uucplock);
		exit(1);
	}
	if ((acu = acutype(AT)) == NOACU)
		return (MSGSTR(UNKNOWNACU, "unknown ACU type")); /*MSG*/
	if (*cp != '@') {
		while (*cp) {
			for (phnum = cp; *cp && *cp != ','; cp++)
				;
			if (*cp)
				*cp++ = '\0';
			
			if (conflag = (*acu->acu_dialer)(phnum, CU)) {
				logent(value(HOST), phnum, acu->acu_name,
					MSGSTR(CALLCOMPLETE, "call completed")); /*MSG*/
				return (NOSTR);
			} else
				logent(value(HOST), phnum, acu->acu_name,
					MSGSTR(CALLFAIL, "call failed")); /*MSG*/
			tried++;
		}
	} else {
		if ((fd = fopen(PH, "r")) == (FILE *)TIP_NOFILE) {
			printf("%s: ", PH);
			return (MSGSTR(NOPNFILE, "can't open phone number file")); /*MSG*/
		}
		while (fgets(string, sizeof(string), fd) != NOSTR) {
			for (cp = string; !any(*cp, " \t\n"); cp++)
				;
			if (*cp == '\n') {
				fclose(fd);
				fprintf(stderr, (MSGSTR(PLSREMOV, "Remove any blank lines or lines with only one non-blank\nfield from the phones file.\n"))); 
				return (MSGSTR(UNRECHOST, "unrecognizable host name")); /*MSG*/
			}
			*cp++ = '\0';
			if (strcmp(string, value(HOST)))
				continue;
			while (any(*cp, " \t"))
				cp++;
			if (*cp == '\n') {
				fclose(fd);
				return (MSGSTR(NOPN, "missing phone number")); /*MSG*/
			}
			for (phnum = cp; *cp && *cp != ',' && *cp != '\n'; cp++)
				;
			if (*cp)
				*cp++ = '\0';
			
			if (conflag = (*acu->acu_dialer)(phnum, CU)) {
				fclose(fd);
				logent(value(HOST), phnum, acu->acu_name,
					MSGSTR(CALLCOMPLETE, "call completed")); /*MSG*/
				return (NOSTR);
			} else
				logent(value(HOST), phnum, acu->acu_name,
					MSGSTR(CALLFAIL, "call failed")); /*MSG*/
			tried++;
		}
		fclose(fd);
	}
	if (!tried)
		logent(value(HOST), "", acu->acu_name, MSGSTR(NOPN, "missing phone number")); /*MSG*/
	else
		(*acu->acu_abort)();
	return (tried ? MSGSTR(CALLFAIL, "call failed") : MSGSTR(NOPN, "missing phone number")); /*MSG*/ /*MSG*/
}

disconnect(reason)
	char *reason;
{
	if (!conflag)
		return;
	if (reason == NOSTR) {
		logent(value(HOST), "", acu->acu_name, MSGSTR(CALLTERM, "call terminated")); /*MSG*/
		if (boolean(value(VERBOSE)))
			printf(MSGSTR(DISCONNECTING, "\r\ndisconnecting...")); /*MSG*/
	} else 
		logent(value(HOST), "", acu->acu_name, reason);
	(*acu->acu_disconnect)();
}

static void
acuabort(int s)
{
	signal(s, SIG_IGN);
	longjmp(jmpbuf, 1);
}

static acu_t *
acutype(s)
	register char *s;
{
	register acu_t *p;
	extern acu_t acutable[];

	for (p = acutable; p->acu_name != '\0'; p++)
		if (!strcmp(s, p->acu_name))
			return (p);
	return (NOACU);
}
