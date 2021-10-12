static char sccsid[] = "@(#)14  1.2  src/bos/usr/ccs/lib/libtli/tnblock.c, libtli, bos411, 9428A410j 11/9/93 18:46:16";
/*
 *   COMPONENT_NAME: LIBTLI
 *
 *   FUNCTIONS: t_nonblocking
 *
 *   ORIGINS: 18 27 63
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

#include <fcntl.h>
#include "common.h"
#include <sys/stropts.h>

int
t_nonblocking (fd)
	int	fd;
{
	register int status;

	/*
	 * Get descriptor's current flag settings
	 */
	if ((status = fcntl( fd, F_GETFL, 0 )) < 0)
		return status;

	/*
	 * Now ensure that the NO DELAY bit is set, ie. make
	 * descriptor do nonblocking I/O
	 */
	return fcntl( fd, F_SETFL, (status | FNDELAY) );
}
