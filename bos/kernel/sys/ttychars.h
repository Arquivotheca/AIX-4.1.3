/* @(#)37	1.8  src/bos/kernel/sys/ttychars.h, cmdtty, bos411, 9428A410j 6/16/90 00:39:04 */

/*
 * COMPONENT_NAME: (sysxtty) System Extension for tty support
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 3, 9, 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *
 * <bsd/sys/ttychars.h - a la 4.xBSD for BSD to AIX porting tools
 *	derived from BSD <sys/ttychars.h>
 * COPYRIGHT 1987 IBM CORP.
 */
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	(#)ttychars.h	7.1 (Berkeley) 6/4/86
 */

/*
 * User visible structures and constants
 * related to terminal handling.
 */
#ifndef _H_TTYCHARS_
#define	_H_TTYCHARS_

struct ttychars {
	char	tc_erase;	/* erase last character */
	char	tc_kill;	/* erase entire line */
	char	tc_intrc;	/* interrupt */
	char	tc_quitc;	/* quit */
	char	tc_startc;	/* start output */
	char	tc_stopc;	/* stop output */
	char	tc_eofc;	/* end-of-file */
	char	tc_brkc;	/* input delimiter (like nl) */
	char	tc_suspc;	/* stop process signal */
	char	tc_dsuspc;	/* delayed stop process signal */
	char	tc_rprntc;	/* reprint line */
	char	tc_flushc;	/* flush output (toggles) */
	char	tc_werasc;	/* word erase */
	char	tc_lnextc;	/* literal next character */
};

#define	CTRL(c)	('c'&037)

/* default special characters */
#define	CBRK	0377
#define	CDSUSP	CTRL(y)
#define CEOF	'\004'			/* ^D */
#define	CEOT	CEOF
#define	CERASE	'\010'			/* ^H */
#define	CFLUSH	CTRL(o)
#define	CINTR	'\003'			/* ^C */
#define CKILL   '\025'			/* ^U */
#define	CLNEXT	CTRL(v)
#define	CQUIT	'\034'			/* ^\ */
#define	CRPRNT	CTRL(r)
#define CSTART	'\021'			/* ^Q */
#define CSTOP	'\023'			/* ^S */
#define	CSUSP	CTRL(z)
#define	CWERASE	CTRL(w)
#endif /* _H_TTYCHARS_ */
