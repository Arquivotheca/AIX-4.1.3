static char sccsid[] = "@(#)67	1.13  src/bos/usr/ccs/lib/libc/seekdir.c, libcio, bos411, 9428A410j 5/30/92 10:46:33";
/*
 * COMPONENT_NAME: (LIBCIO) Standard C Library I/O Functions 
 *
 * FUNCTIONS: seekdir 
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

#include <sys/types.h>
#include <sys/param.h>
#include <dirent.h>

/*                                                                    
 * FUNCTION: sets position of next readdir() operation on directory stream.
 *
 * RETURN VALUE DESCRIPTION: None.
 */  

/*
 *  seekdir - seek to an entry in a directory
 */

void seekdir(DIR *dirp, long loc)
{
	/* if not open directory stream or not moving */
	if ( dirp == NULL || (dirp->dd_curoff == loc && loc != 0))
		return;

	/* wondering why we do this rounding?  seekdir() is mostly 
	 * guess work.  there is no way to guarantee the directory
	 * structure has not changed.  therefore we try to get as
	 * close as possible to the byte offset requested.  this 
	 * code as written assumes that things have not changed
	 * more than could be contained in one DIRBLKSIZ chunk.
	 * a reasonable assumption but not guaranteed.
	 *			jar
	 */
	dirp->dd_curoff = lseek (dirp->dd_fd, loc & ~(DIRBLKSIZ - 1), 0);

	/* force readdir() into doing a getdirent() and try to get back
	 * to the requested location
	 */
	dirp->dd_loc = dirp->dd_size = 0;
	while (dirp->dd_curoff < loc)
		if (readdir(dirp) == (struct dirent *)NULL)
			return;
}
