static char sccsid[] = "@(#)07	1.6  src/bos/usr/ccs/lib/libIN/PFgrpgid.c, libIN, bos411, 9428A410j 6/10/91 10:21:54";
/*
 * LIBIN: PFgrpgid
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
 * FUNCTION: Convert from groupname to gid.
 *
 * RETURN VALUE DESCRIPTION: Returns the groupid.
 */

#include <stdio.h>
#include <IN/standard.h>
#include <IN/PFdefs.h>

extern  struct pwinfo _ginfo;
extern  char *_gargs[];

PFgrpgid(s)
char *s;
{       extern struct pwinfo *PFname();

	if (_gargs[0] == 0 || CScmp(s,_gargs[0]) != 0)
	    PFname(&_ginfo,s);
	return(_gargs[0] ? atoi(_gargs[2]) : -1);
}

