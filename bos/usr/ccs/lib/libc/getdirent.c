static char sccsid[] = "@(#)93	1.3  src/bos/usr/ccs/lib/libc/getdirent.c, libcio, bos411, 9428A410j 6/16/90 01:17:57";
/*
 * COMPONENT_NAME: (LIBCIO) Standard C Library I/O Functions 
 *
 * FUNCTIONS: getdirentries 
 *
 * ORIGINS: 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

/* compatibility interface for reading directory entries in
 * file system independent format.  the function getdirent()
 * replaces the function and s.b. used when writing new code or 
 * modifying existing source.
 */

int
getdirentries (fd, buf, bsize, offp)
register int fd;
register caddr_t buf;
register int bsize;
register off_t *offp;
{
	register 	int 	nbytes = 0;

	if ((nbytes = getdirent (fd, buf, bsize)) >= 0)
		*offp = lseek (fd, (off_t)SEEK_CUR, 0);
	return nbytes;	
}
