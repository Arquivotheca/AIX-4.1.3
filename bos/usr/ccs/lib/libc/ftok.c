static char sccsid[] = "@(#)35	1.8  src/bos/usr/ccs/lib/libc/ftok.c, libcgen, bos411, 9428A410j 11/17/93 17:13:25";
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: ftok
 *
 * ORIGINS: 3 27
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
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>

/*
 * NAME:	ftok
 *                                                                    
 * FUNCTION:	generate a standard ipc key
 *                                                                    
 * NOTES:	Ftok generates a key based on 'path' and 'id'.  'Path'
 *		is the pathname of a currently existing file that is
 *		accessible to the process.  'Id' is a character that
 *		can cause different keys to be generated for the same
 *		'path'.
 *
 * RETURN VALUE DESCRIPTION:	-1 if the file does not exist or is
 *		not accessible to the process, -1 if 'id' is '\0',
 *		else the key
 */  

key_t
ftok(const char *path, int id)
{
	struct stat st;

	/* Leave keys 0 to (2^24-1) for SNA, other use */
	if(id == '\0')
		return((key_t)-1);

	/*
	 * if the file is not accessible or doesn't exist, return -1.
	 * else use a combo of 'id', the minor device number of the
	 * device the file lives on, and the file's inode number to
	 * compute the key...
	 *
	 * Zero out the top byte of the minor number so that the id does 
         * not get over written by it.
	 */
	return(stat(path, &st) < 0 ? (key_t)-1 :
               (key_t)((key_t)id << 24 | 
               ( 0x00ffffff & ((long)(unsigned)minor(st.st_dev)) << 16 |
		(unsigned)st.st_ino)) );
}
