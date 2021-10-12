static char sccsid[] = "@(#)68	1.6  src/bos/usr/ccs/lib/libIN/PFuidusr.c, libIN, bos411, 9428A410j 6/10/91 10:22:28";
/*
 * LIBIN: PFuidusr
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
 * FUNCTION: Convert uid into username.
 *	Note: there may be more than one username with the same uid.
 *	This routine returns the first such username in the password file.
 *
 * RETURN VALUE DESCRIPTION: Returns a pointer to converted username.
 */

#include <stdio.h>
#include <IN/standard.h>
#include <IN/PFdefs.h>

extern  struct pwinfo _uinfo;
extern  char *_uargs[];

char *
PFuidusr(uid)
register int uid;
{       extern struct pwinfo *PFid();

	if (_uargs[0] == 0 || atoi(_uargs[2]) != uid)
	    PFid(&_uinfo,uid);
	return(_uargs[0]);
}

