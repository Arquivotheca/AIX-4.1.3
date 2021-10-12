static char sccsid[] = "@(#)96	1.6  src/bos/usr/ccs/lib/libIN/PFgidgrp.c, libIN, bos411, 9428A410j 6/10/91 10:21:50";
/*
 * LIBIN: PFgidgrp
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
 * FUNCTION: Convert gid into groupname.
 *	Note: There may be more than one groupname with the same gid.
 *	This routine returns the first such groupname in the group file.
 *
 * RETURN VALUE DESCRIPTION: Returns a pointer to converted groupname.
 */

#include <stdio.h>
#include <IN/standard.h>
#include <IN/PFdefs.h>

extern  struct pwinfo _ginfo;
extern  char *_gargs[];

char *
PFgidgrp(gid)
register int gid;
{       extern struct pwinfo *PFid();

	if (_gargs[0] == 0 || atoi(_gargs[2]) != gid)
	    PFid(&_ginfo,gid);
	return(_gargs[0]);
}

