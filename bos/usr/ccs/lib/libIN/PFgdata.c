static char sccsid[] = "@(#)85	1.6  src/bos/usr/ccs/lib/libIN/PFgdata.c, libIN, bos411, 9428A410j 6/10/91 10:21:44";
/*
 * LIBIN: PFgdata
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
 * FUNCTION: Table of group information.
 *
 */

#include <stdio.h>
#include <IN/standard.h>
#include <IN/PFdefs.h>

char *_gargs[4] = { 0, 0, 0, 0 };
char _gbuf[128];

struct  pwinfo _ginfo =
{       NULL,
	"/etc/group",
	4,
	_gargs,
	128,
	_gbuf
};

