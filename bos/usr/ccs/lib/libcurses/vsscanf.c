#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)28  1.6  src/bos/usr/ccs/lib/libcurses/vsscanf.c, libcurses, bos411, 9428A410j 9/3/93 14:49:43";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: vsscanf
 *		
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

/* #ident	"@(#)curses:screen/vsscanf.c	1.5"		*/



#include "curses_inc.h"
#include <varargs.h>

/*
 *	This routine implements vsscanf (nonportably) until such time
 *	as one is available in the system (if ever).
 */

/*VARARGS2*/
vsscanf(buf, fmt, ap)
char	*buf;
char	*fmt;
va_list	ap;
{
	FILE	junk;
	extern int _doscan();

#ifdef SYSV
	junk._flag = _IOREAD;
	junk._file = -1;
	junk._base = junk._ptr = (unsigned char *) buf;
#else
	junk._flag = _IOREAD|_IOSTRG;
	junk._base = junk._ptr = buf;
#endif
	junk._cnt = strlen(buf);
	return _doscan(&junk, fmt, ap);
}
