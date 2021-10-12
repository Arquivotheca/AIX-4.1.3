/* @(#)82       1.5.1.3  src/bos/usr/include/ttyent.h, cmdtty, bos411, 9428A410j 3/31/93 09:28:54 */
#ifndef _H_TTYENT
#define _H_TTYENT
#ifdef _POWER_PROLOG_
/*
 *   COMPONENT_NAME: CMDTTY
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 26,27,71
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992, 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#endif /* _POWER_PROLOG_ */

/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

struct	ttyent {		/* see getttyent(3) */
    char *ty_name;		/* terminal device name */
    char *ty_getty;		/* command to execute, usually getty */
    char *ty_type;		/* terminal type for termcap (3X) */
    int	ty_status;		/* status flags (see below for defines) */
    char *ty_window;		/* command to start up window manager */
    char *ty_comment;		/* usually the location of the terminal */
};

#define TTY_ON		0x1	/* enable logins (startup getty) */
#define ENABLE_TRUE	TTY_ON
#define TTY_SECURE	0x2	/* allow root to login */
#define SUPER_TRUE	TTY_SECURE
#define ENABLE_SHARE	0x5	/* bi-directional use */
#define ENABLE_DELAY	0x9	/* bi-directional use but getty sets 
				 * the lock after the first character 
				 * has been received */

#ifdef _NO_PROTO
extern struct ttyent *getttyent();
extern struct ttyent *getttynam();
extern void setttyent();
extern void endttyent();
#else /* _NO_PROTO */
extern struct ttyent *getttyent(void);
extern struct ttyent *getttynam(char *);
extern void setttyent(void);
extern void endttyent(void);
#endif /* _NO_PROTO */

#ifdef _NO_PROTO

#ifdef _THREAD_SAFE
extern int getttyent_r();
extern void setttyent_r();
extern void endttyent_r();
extern int getttynam_r();
#endif /* _THREAD_SAFE */

#else /* _NO_PROTO */

#ifdef _THREAD_SAFE
extern int getttyent_r(char *line, struct ttyent **curinfo, 
		       struct ttyent *tty_info);
extern void setttyent_r(struct ttyent **curinfo);
extern void endttyent_r(struct ttyent **tty_info);
extern int getttynam_r(const char *tty, struct ttyent *tte, char *buf);
#endif /* _THREAD_SAFE */

#endif /* _NO_PROTO */

#endif /* _H_TTYENT */
