#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)14  1.1  src/bos/usr/ccs/lib/libcurses/uparm.h, libcurses, bos411, 9428A410j 9/3/93 15:14:49";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS:
 *		
 *
 *   ORIGINS: 27, 4
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#endif /* _POWER_PROLOG_ */


/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*#ident	"@(#)curses:screen/uparm.h	1.2"		*/


/*
 * Local configuration of various files.  Used if you can't put these
 * things in the standard places or aren't the super user, so you
 * don't have to modify the source files.  Thus, you can install updates
 * without having to re-localize your sources.
 *
 * This file used to go in /usr/include/local/uparm.h.  Every version of
 * UNIX has undone this, so now it wants to be installed in each source
 * directory that needs it.  This means you now include "uparm.h" instead
 * of "local/uparm.h".
 */

/* Path to library files */
#define libpath(file) "/usr/lib/file"

/* Path to local library files */
#define loclibpath(file) "/usr/local/lib/file"

/* Path to binaries */
#define binpath(file) "/usr/bin/file"

/* Path to things under /usr (e.g. /usr/preserve) */
#define usrpath(file) "/usr/file"

/* Location of termcap file */
#define E_TERMCAP	"/etc/termcap"

/* Location of terminfo source file */
#define E_TERMINFO	"/usr/share/lib/terminfo/terminfo.src"

/* Location of terminfo binary directory tree */
#define _TERMPATH(file)	"/usr/share/lib/terminfo/file"

/* Location of the C shell */
#define B_CSH		"/usr/bin/csh"
