/* @(#)53 1.2 src/bos/kernel/sys/ttydefaults.h, sysxtty, bos411, 9428A410j 7/12/94 13:58:32 */
/*
 * COMPONENT_NAME: SYSXTTY
 *
 * FUNCTIONS :
 *
 * ORIGINS: 26, 71, 83
 *
 */
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)ttydefaults.h	7.2 (Berkeley) 11/20/89
 */
/*
 * OSF/1 1.2
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#ifndef _H_TTYDEFAULTS
#define	_H_TTYDEFAULTS

/*
 * System wide defaults for terminal state.
 */

/*
 * Defaults on "first" open.
 */
#ifdef _KERNEL
#define	TTYDEF_IFLAG	(BRKINT | ICRNL | IXON)
#else
#define	TTYDEF_IFLAG	(BRKINT | ICRNL | IMAXBEL | IXON | IXANY)
#endif
#define TTYDEF_OFLAG	(OPOST | ONLCR | TAB3)
#ifdef _KERNEL
#define TTYDEF_LFLAG	(ECHO | ICANON | ISIG)
#else
#define TTYDEF_LFLAG	(ECHO | ICANON | ISIG | IEXTEN | ECHOE|ECHOKE|ECHOCTL)
#endif
#define TTYDEF_CFLAG	(CREAD | CS8 )
#define TTYDEF_SPEED	(B9600)

/*
 * #define TTYDEFCHAR(index) to retrieve default control characters.
 * Be careful: The ttydefchars array is initialized with the
 * ==========  control character positions which are defined in
 *             the termios.h file.
 */
static cc_t ttydefchars[NCCS] = {
    /* VINTR */    CINTR,
    /* VQUIT */    CQUIT,
    /* VERASE */   CERASE,
    /* VKILL */    CKILL,
    /* VEOF */     CEOF,
    /* VEOL */     _POSIX_VDISABLE,
    /* VEOL2 */    _POSIX_VDISABLE,
    /* VSTART */   CSTART,
    /* VSTOP */    CSTOP,
    /* VSUSP */    CSUSP,
    /* VDSUSP */   CDSUSP,
    /* VREPRINT */ CRPRNT,
    /* VDISCRD */  CFLUSH,
    /* VWERSE */   CWERASE,
    /* VLNEXT */   CLNEXT
};

#define TTYDEFCHAR(index) ttydefchars[(index)] 

#endif /* _H_TTYDEFAULTS */
