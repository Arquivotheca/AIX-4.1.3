static char sccsid[] = "@(#)77	1.6  src/bos/usr/ccs/lib/libIN/LGdev.c, libIN, bos411, 9428A410j 6/10/91 10:19:32";
/*
 * LIBIN: LGdev
 *
 * ORIGIN: 9
 *
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1985, 1989
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 *
 * FUNCTION: 
 *      Return the full pathname of the user's login terminal
 *
 *      If UINFO is defined, this version simply calls getuinfo()
 *      and returns a pointer to the device name in the user information.
 *      If the tty number is not in the user information, it returns
 *      "/dev/null".
 *
 *      If UINFO is not defined (in uinfo.h), this version calls LGname,
 *      and counts upon it to set _LGdbuf to the device name.
 */

#include <stdio.h>
#include <uinfo.h>

char *
LGdev ()
{
#ifdef UINFO

extern   char *getuinfo();
register char *cp = getuinfo("TTY");

	return( (cp && *cp) ? cp : "/dev/null" );

#else
	extern char _LGdbuf[];

	(void) LGname();
	return _LGdbuf;
#endif
}
