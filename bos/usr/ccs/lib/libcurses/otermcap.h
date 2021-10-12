/* @(#)51	1.1  src/bos/usr/ccs/lib/libcurses/otermcap.h, libcurses, bos411, 9428A410j 9/3/93 15:10:34 */
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS:
 *
 *   ORIGINS: 4
 *
 *                    SOURCE MATERIALS
 */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/* #ident	"@(#)curses:screen/otermcap.h	1.2" */
#define TBUFSIZE	2048		/* double the norm */

/* externs from libtermcap.a */
extern int otgetflag (), otgetnum (), otgetent ();
extern char *otgetstr ();
extern char *tskip ();			/* non-standard addition */
extern int TLHtcfound;			/* non-standard addition */
extern char TLHtcname[];		/* non-standard addition */
