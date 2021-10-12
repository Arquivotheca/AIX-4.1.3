/* @(#)77       1.7  src/bos/usr/bin/mail/v7.local.h, cmdmailx, bos41J, 9523C_all 6/8/95 13:36:45 */
/* 
 * COMPONENT_NAME: CMDMAILX v7.local.h
 * 
 * FUNCTIONS: 
 *
 * ORIGINS: 10  26  27  71
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *      v7.local.h  5.2 (Berkeley) 9/19/85
 */

/*
 * Declarations and constants specific to an installation.
 *
 * Vax/Unix version 7.
 */

#define	GETHOST				/* System has gethostname syscall */
#ifdef	GETHOST
#define	LOCAL		EMPTYID		/* Dynamically determined local host */
#else
#define	LOCAL		'Self'		/* Local host id */
#endif	GETHOST

#define MAILBOX		"/var/spool/mail"	/* system mailbox dir */
#define	MAIL		"/usr/sbin/sendmail"	/* Name of mail sender */
#define SENDMAIL	"/usr/sbin/sendmail" /* Name of classy mail deliverer */

#define	EDITOR		"/usr/bin/ed"	/* Name of text editor */
#define	VISUAL		"/usr/bin/vi"	/* Name of display editor */
#define	MORE		"/usr/bin/pg"	/* Standard output pager */
#define PAGE		"/usr/bin/pg"	/* Standard output pager */

#define	SHELL		"/usr/bin/sh"	/* Standard shell */
#define	POSTAGE		"/var/adm/maillog"
					/* Where to audit mail sending */
#define	UIDMASK		0177777		/* Significant uid bits */
#define	MASTER		"/usr/share/lib/Mail.rc"
#define	APPEND				/* New mail goes to end of mailbox */
#define CANLOCK				/* Locking protocol actually works */
#define	UTIME				/* System implements utime(2) */
