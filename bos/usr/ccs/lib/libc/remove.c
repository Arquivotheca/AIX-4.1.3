static char sccsid[] = "@(#)23	1.11  src/bos/usr/ccs/lib/libc/remove.c, libcio, bos411, 9428A410j 5/6/91 11:42:47";
/*
 * COMPONENT_NAME: (LIBCIO) Standard C Library I/O Functions 
 *
 * FUNCTIONS: remove 
 *
 * ORIGINS: 3, 27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*                                                                    
 * FUNCTION: The remove() function causes the file, whose name is pointed
 *	     to by filename, to be removed
 *
 * PARAMETERS: char *filename - file to be removed
 *
 * RETURN VALUE DESCRIPTIONS:
 * 	      0 if successful
 *	      non-0 if not successful
 *
 */
/* The remove() function includes all the POSIX requirements */
#include <sys/stat.h>
#include <sys/limits.h>
int	
remove(const char *name)
{
	struct stat	stbuf;

	if (lstat((char *)name, &stbuf) < 0)
		return (-1);
	if ((stbuf.st_mode & S_IFMT) == S_IFDIR) {
		return(rmdir((char *)name));
	}
	else {
		return(unlink((char *)name));
	}
}
