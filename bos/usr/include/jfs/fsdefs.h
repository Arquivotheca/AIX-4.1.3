/* @(#)41	1.2  src/bos/usr/include/jfs/fsdefs.h, syspfs, bos411, 9428A410j 12/9/92 08:13:15 */
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System 
 *
 * FUNCTIONS: fsdefs.h
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_JFS_FSDEFS
#define _H_JFS_FSDEFS

/* Scratch segment registers addresses
 */
#define  SR12ADDR  0xC0000000
#define  SR13ADDR  0xD0000000

/* True if name is "." or ".."
 */
#define	ISDOTS(name)	((name)[0] == '.' && (((name)[1] == '\0') || \
			((name)[1] == '.' && (name)[2] == '\0')))

/*
 * fundamental variables
 * don't change too often
 */

#ifdef PAGESIZE
#undef PAGESIZE
#endif
#define	PAGESIZE	4096		/* page size */

#ifdef SEGSIZE
#undef SEGSIZE
#endif
#define SEGSIZE		(1<<28)		/* segment size */

#define NDADDR		8		/* Direct blocks in inode */
#define NFADDR		8		/* Direct blocks in fifo */
#define DILENGTH	128		/* Disk inode length */
#define LOGMINOR	8		/* reserved minor number for log */

#define L_BCHUNK	32		/* # blocks allocated together  */
#define MIN_AIX3SIZE	L_BCHUNK    	/* minimum aix3 filesystem size */

#endif /* _H_JFS_FSDEFS */
