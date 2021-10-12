static char sccsid[] = "@(#)88	1.3  src/bos/usr/ccs/lib/libc/isastream.c, libcpse, bos411, 9428A410j 8/1/91 08:53:07";
/*
 *   COMPONENT_NAME: LIBCPSE
 *
 *   ORIGINS: 27 63
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/** Copyright (c) 1990  Mentat Inc.
 ** unixio.c 1.2
 **/

#include <stropts.h>
#include <errno.h>

int
isastream(fd)
	int fd;
{
	int rc;

	rc = ioctl(fd, I_NREAD, &rc);
	if (rc == -1 && errno == EBADF)
		return -1;
	return rc == -1 ? 0 : 1;
}
