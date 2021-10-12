static char sccsid[] = "@(#)44	1.10  src/bos/usr/ccs/lib/libc/telldir.c, libcio, bos411, 9428A410j 5/30/92 10:46:36";
/*
 * COMPONENT_NAME: (LIBCIO) Standard C Library I/O Functions 
 *
 * FUNCTIONS: telldir 
 *
 * ORIGINS: 26, 27 
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
 *
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include <sys/param.h>
#include <sys/types.h>	/*AIX/BSD*/
#include <dirent.h>	/* posix */

/*                                                                    
 * FUNCTION: Return a pointer into a directory
 *
 * RETURN VALUE DESCRIPTION: Directory offset.
 */  

/*
 * return a pointer into a directory
 */
long
telldir(DIR *dirp)
{
	if (dirp == NULL)
		return (NULL);
	else    return dirp->dd_curoff;
}
