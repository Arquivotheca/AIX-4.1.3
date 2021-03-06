static char sccsid[] = "@(#)87	1.7  src/bos/usr/ccs/lib/libIN/LGdir.c, libIN, bos411, 9428A410j 6/10/91 10:19:36";
/*
 * LIBIN: LGdir
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
 * FUNCTION: Return the full pathname of the user's login directory.
 */

#include <pwd.h>
#include <IN/standard.h>

char *
LGdir()
{
	extern char *LGname();
	register struct passwd *pw;

	pw = getpwnam(LGname());
	if( pw == NULL )
	    return "/tmp";
	return pw->pw_dir;
}
