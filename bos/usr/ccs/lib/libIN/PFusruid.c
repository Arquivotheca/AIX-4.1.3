static char sccsid[] = "@(#)00	1.6  src/bos/usr/ccs/lib/libIN/PFusruid.c, libIN, bos411, 9428A410j 6/10/91 10:22:40";
/*
 * LIBIN: PFusruid
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
 * FUNCTION: Convert from username to uid via password file.
 *
 * RETURN VALUE DESCRIPTION: Returns uid value.
 */

#include <stdio.h>
#include <IN/standard.h>
#include <IN/PFdefs.h>

extern  struct pwinfo _uinfo;
extern  char *_uargs[];

PFusruid(s)
char *s;
{       extern struct pwinfo *PFname();

	if (_uargs[0] == 0 || CScmp(s,_uargs[0]) != 0)
	    PFname(&_uinfo,s);
	return(_uargs[0] ? atoi(_uargs[2]) : -1);
}

