/* @(#)56 1.5 src/bos/kernext/tty/stream_tioc.h, sysxtty, bos411, 9428A410j 5/23/94 09:54:50 */
/*
 * COMPONENT_NAME:
 *
 * FUNCTIONS:
 *
 * ORIGINS: 40, 71, 83
 *
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 1.2
 */

/************************************************************************
 *                                                                      *
 *                      Copyright (c) 1990 by                           *
 *              Digital Equipment Corporation, Maynard, MA              *
 *                      All rights reserved.                            *
 *                                                                      *
 *   This software is furnished under a license and may be used and     *
 *   copied  only  in accordance with the terms of such license and     *
 *   with the  inclusion  of  the  above  copyright  notice.   This     *
 *   software  or  any  other copies thereof may not be provided or     *
 *   otherwise made available to any other person.  No title to and     *
 *   ownership of the software is hereby transferred.                   *
 *                                                                      *
 *   The information in this software is subject to change  without     *
 *   notice  and should not be construed as a commitment by Digital     *
 *   Equipment Corporation.                                             *
 *                                                                      *
 *   Digital assumes no responsibility for the use  or  reliability     *
 *   of its software on equipment which is not supplied by Digital.     *
 *                                                                      *
 ************************************************************************/

/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#ifndef	_H_STREAM_TIOC
#define	_H_STREAM_TIOC

#include <sys/str_tty.h>
#include <sys/eucioctl.h>
/*
 * dds type declaration for transparent ioctl's module. The dds_type definition
 * is issued from the general stream tty include "str_tty.h".
 */
struct	tioc_dds {
	enum	dds_type	which_dds;	/* dds identifier	*/
};

#ifdef	_KERNEL

/*
 *	 ldtty_tioc_reply is stacticly declared here. This avoids to use
 *	 the stream message protocol TIOC_REQUEST->TIOC_REPLY. So, this
 *	 implementation is also easier to use by the line printer module.
 */
static struct tioc_reply
ldtty_tioc_reply[] = {
	{ TIOCSETD, sizeof(int), TTYPE_COPYIN },
	{ TIOCGETD, sizeof(int), TTYPE_COPYOUT },
	{ TIOCFLUSH, sizeof(int), TTYPE_COPYIN },
	{ TIOCOUTQ, sizeof(int), TTYPE_COPYOUT },
	{ TIOCCONS, sizeof(int), TTYPE_COPYIN },
	{ TIOCSETA, sizeof(struct termios), TTYPE_COPYIN },
	{ TIOCSETAW, sizeof(struct termios), TTYPE_COPYIN },
	{ TIOCSETAF, sizeof(struct termios), TTYPE_COPYIN },
	{ TIOCGETA, sizeof(struct termios), TTYPE_COPYOUT },
	{ TIOCSTI, sizeof(char), TTYPE_COPYIN },
	{ TIOCSWINSZ, sizeof(struct winsize), TTYPE_COPYIN },
	{ TIOCGWINSZ, sizeof(struct winsize), TTYPE_COPYOUT },
/* COMPAT_BSD_4.3	*/
	{ TIOCSETP, sizeof(struct sgttyb), TTYPE_COPYIN },
	{ TIOCSETN, sizeof(struct sgttyb), TTYPE_COPYIN },
	{ TIOCGETP, sizeof(struct sgttyb), TTYPE_COPYOUT },
	{ TIOCSETC, sizeof(struct tchars), TTYPE_COPYIN },
	{ TIOCGETC, sizeof(struct tchars), TTYPE_COPYOUT },
	{ TIOCSLTC, sizeof(struct ltchars), TTYPE_COPYIN },
	{ TIOCGLTC, sizeof(struct ltchars), TTYPE_COPYOUT },
	{ TIOCLBIS, sizeof(int), TTYPE_COPYIN },
	{ TIOCLBIC, sizeof(int), TTYPE_COPYIN },
	{ TIOCLSET, sizeof(int), TTYPE_COPYIN },
	{ TIOCLGET, sizeof(int), TTYPE_COPYOUT },
	{ TIOCSDTR, 0, TTYPE_NOCOPY },
	{ TIOCCDTR, 0, TTYPE_NOCOPY },
	{ TIOCNXCL, 0, TTYPE_NOCOPY },
	{ TIOCEXCL, 0, TTYPE_NOCOPY },
/* COMPAT_BSD_4.3 */
/* SVID starts */
	{ TCSETA, sizeof(struct termio), TTYPE_COPYIN },
	{ TCSETAW, sizeof(struct termio), TTYPE_COPYIN },
	{ TCSETAF, sizeof(struct termio), TTYPE_COPYIN },
	{ TCGETA, sizeof(struct termio), TTYPE_COPYOUT },
	{ TCXONC, sizeof(int), TTYPE_IMMEDIATE },
	{ TCFLSH, sizeof(int), TTYPE_IMMEDIATE },
	{ TCSBRK, sizeof(int), TTYPE_IMMEDIATE },
/* SVID ends */
/* POSIX	*/
	{ TCSBREAK, sizeof(int), TTYPE_IMMEDIATE },
/* SVID.4 starts */
	{ TIOCMBIS, sizeof(int), TTYPE_COPYIN },
	{ TIOCMBIC, sizeof(int), TTYPE_COPYIN },
	{ TIOCMSET, sizeof(int), TTYPE_COPYIN },
	{ TIOCMGET, sizeof(int), TTYPE_COPYOUT },
/* SVID.4 ends */
	{ TCSAK, sizeof(int), TTYPE_COPYIN },
/* For pty : special AIX */
	{ TXTTYNAME, TTNAMEMAX, TTYPE_COPYOUT },
/* For pty : end	*/
/* AIX diagnostic	*/
	{ TCLOOP, sizeof(int), TTYPE_COPYIN },
/* AIX diagnostic	*/
/* Special for open and hard control disciplines	*/
	{ TCGETX, sizeof(struct termiox), TTYPE_COPYOUT },
	{ TCSETX, sizeof(struct	termiox), TTYPE_COPYIN  },
	{ TCSETXW, sizeof(struct termiox), TTYPE_COPYIN },
	{ TCSETXF, sizeof(struct termiox), TTYPE_COPYIN },
/* End specials				*/
	{ TCGSAK, sizeof(int), TTYPE_COPYOUT },
	{ TCGLEN, sizeof(int), TTYPE_COPYOUT },
	{ TCSLEN, sizeof(int), TTYPE_COPYIN },
	{ TXGETLD, TTNAMEMAX, TTYPE_COPYOUT },
	{ TXSETLD, sizeof(int), TTYPE_COPYIN },
	{ TXGETCD, TTNAMEMAX, TTYPE_COPYOUT },
	{ TXADDCD, sizeof(int), TTYPE_COPYIN },
	{ TXDELCD, sizeof(int), TTYPE_COPYINOUT },
	{ TXSETIHOG, sizeof(int), TTYPE_COPYIN },
	{ TXSETOHOG, sizeof(int), TTYPE_COPYIN },
	{ EUC_WGET, sizeof(eucioc_t), TTYPE_COPYOUT },
	{ EUC_WSET, sizeof(eucioc_t), TTYPE_COPYIN },
	{ IOCINFO, sizeof(struct devinfo), TTYPE_COPYOUT },
};
#endif	/* _KERNEL	*/

#endif	/* _H_STREAM_TIOC	*/
