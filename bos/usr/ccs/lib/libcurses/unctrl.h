#ifdef _POWER_PROLOG_
/* @(#)11    1.1  src/bos/usr/ccs/lib/libcurses/unctrl.h, libcurses, bos411, 9428A410j 9/3/93 15:14:36 */
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS:
 *
 *   ORIGINS: 4
 *
 *                    SOURCE MATERIALS
 */
#endif /* _POWER_PROLOG_ */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*#ident	"@(#)curses:screen/unctrl.h	1.3"		*/
/*
 * unctrl.h
 *
 */

#ifndef UNCTRL_H
#define	UNCTRL_H

extern char	*_unctrl[];

#if	!defined(NOMACROS) && !defined(lint)

#define	unctrl(ch)	(_unctrl[(unsigned) ch])

#endif	/* NOMACROS && lint */

#endif	/* UNCTRL_H */
