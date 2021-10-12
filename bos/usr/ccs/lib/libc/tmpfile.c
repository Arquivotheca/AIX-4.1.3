static char sccsid[] = "@(#)66	1.11  src/bos/usr/ccs/lib/libc/tmpfile.c, libcio, bos411, 9428A410j 6/16/90 01:20:19";
/*
 * COMPONENT_NAME: (LIBCIO) Standard C Library I/O Functions 
 *
 * FUNCTIONS: tmpfile  
 *
 * ORIGINS: 3, 27 
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
 */

#include <stdio.h>
#include <fcntl.h>

extern int unlink();

/*                                                                    
 * FUNCTION: tmpfile - return a pointer to an update file that can be
 *	     used for scratch. The file will automatically
 *	     go away if the program using it terminates.
 *
 * RETURN VALUE DESCRIPTIONS:
 *           returns a pointer to the stream of the file that it created
 *	     If the file cannot be created, the tmpfile function returns
 *	     a null pointer.
 */

FILE  *
tmpfile(void)
{
	char	tfname[L_tmpnam];
	FILE	*p = NULL;
	int fd;

	(void) tmpnam(tfname);
	if ((fd = open(tfname, O_RDWR|O_EXCL|O_CREAT, 0666)) != -1)
	{
		p = fdopen(fd, "wb+");
		(void) unlink(tfname);
	}
	return(p);
}
