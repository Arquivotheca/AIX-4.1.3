static char sccsid[] = "@(#)58	1.6  src/bos/usr/ccs/lib/libIN/PFudata.c, libIN, bos411, 9428A410j 6/10/91 10:22:24";
/*
 * LIBIN: PFudata
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
 * FUNCTION: Table of password information.
 *
 */

#include <stdio.h>
#include <IN/standard.h>
#include <IN/PFdefs.h>

char *_uargs[7] = { 0, 0, 0, 0, 0, 0, 0 };
char _ubuf[128];

struct  pwinfo _uinfo =
{       NULL,
	"/etc/passwd",
	7,
	_uargs,
	128,
	_ubuf,
};

