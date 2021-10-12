static char sccsid[] = "@(#)24	1.2  src/bos/usr/ccs/lib/libc/vlimit.c, libcsys, bos411, 9428A410j 6/16/90 01:33:37";
/*
 * COMPONENT_NAME: (LIBCSYS) Standard C Library System Functions 
 *
 * FUNCTIONS: vlimit 
 *
 * ORIGINS: 26, 27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
 * (Almost) backwards compatible vlimit (except for LIM_NORAISE).
 */
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/vlimit.h>
#include <errno.h>

vlimit(limit, value)
int limit, value;
{
	struct rlimit rlim;

	if (limit <= 0 || limit > NLIMITS)
		return (EINVAL);
	if (value == -1) {
		if (getrlimit(limit - 1, &rlim) < 0)
			return (-1);
		return (rlim.rlim_cur);
	}
	rlim.rlim_cur = value;
	rlim.rlim_max = RLIM_INFINITY;
	return (setrlimit(limit - 1, &rlim));
}
