static char sccsid[] = "@(#)01  1.2  src/bos/usr/ccs/lib/libtli/tblock.c, libtli, bos411, 9428A410j 11/9/93 18:45:41";
/*
 *   COMPONENT_NAME: LIBTLI
 *
 *   FUNCTIONS: t_blocking, t_is_nonblocking
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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */

#include <fcntl.h>
#include <sys/stropts.h>

int
t_blocking (fd)
	int	fd;
{
	register int status;

	/*
	 * Get descriptor's current flag settings
	 */
	if ((status = fcntl( fd, F_GETFL, 0 )) < 0)
		return status;

	/*
	 * Now ensure that the NO DELAY bit is cleared, ie. make
	 * descriptor do blocking I/O
	 */
	return fcntl( fd, F_SETFL, (status & ~O_NDELAY) );
}

/** Copyright (c) 1990  Mentat Inc.
 ** tcblock.c 2.1, last change 11/14/90
 **/

int
t_is_nonblocking (fd)
	int	fd;
{
	int	flags;

	flags = fcntl(fd, F_GETFL, 0);
	if ( flags == -1 )
		return -1;

	return ((flags & O_NDELAY) || (flags & O_NONBLOCK)) ? 1 : 0;

}
