/* @(#)02       1.5  src/bos/kernel/sys/fscntl.h, syspfs, bos411, 9428A410j 5/20/94 03:53:33 */
/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: fscntl header file
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_FSCNTL
#define _H_FSCNTL

/*
 * 'cmd' args for fscntl system call
 */

#define	FS_EXTENDFS	  0x1		/* extend the filesystem	*/
#define	FS_MOVEBLKS	  0x2		/* move disk blocks		*/
#define FS_EXTENDFS_EXTRA 0x3		/* extend filesystem and over-  */
					/* allocate the maps by 1 block */


#endif /* _H_FSCNTL */
