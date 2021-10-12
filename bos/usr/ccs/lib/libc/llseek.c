static char sccsid[] = "@(#)58  1.2  src/bos/usr/ccs/lib/libc/llseek.c, libcfs, bos411, 9428A410j 12/14/93 13:22:24";
/*
 * COMPONENT_NAME: LIBCFS - File System interfaces in the C library
 *
 * FUNCTIONS: llseek
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>

/*
 *  syscall handler doesn't return long long's, so...
 *  _lseek() will pass back the return value (newoff)
 *      through an additional parameter
 */
offset_t
llseek(int	fd,		/* file descriptor		*/
       offset_t	offset,		/* offset in fs			*/
       int	whence)		/* SEEK_SET, SEEK_CUR, SEEK_END	*/
{
	offset_t	newoff;
	int		rc;

	rc = _lseek(fd, offset, whence, &newoff);
	return (rc < 0) ? (offset_t)rc : newoff;
}
