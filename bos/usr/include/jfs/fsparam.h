/* @(#)17	1.3  src/bos/usr/include/jfs/fsparam.h, syspfs, bos411, 9428A410j 12/9/92 08:22:45 */

#ifndef _H_JFS_FSPARAM
#define _H_JFS_FSPARAM

/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27, 24, 3
 *
 * Copyright International Business Machines Corp. 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 *
 */

/*
* This file contains all of the file system specific parameters
* for the physical file system.  Param.h includes this file.
*/

#define	BSIZE		4096	/* size of file system block (bytes)	*/
#define SBUFSIZE	(BSIZE)	/* system buffer size			*/
#define BSHIFT		12	 /* LOG2(BSIZE)				*/
#define FsBSIZE(dev)	(BSIZE)
#define FsBSHIFT(dev)	(BSHIFT)
#define FsBOFF(dev, x)	((x)&07777)
#define FsBNO(dev, x)	((x)>>BSHIFT)
#define FsINOPB(dev)	32
#define	FsLTOP(dev, b)	((b)<<3)
#define	FsPTOL(dev, b)	((b)>>3)

#define	ROOTINO	((ino_t)2)	/* i number of all roots		*/
#define SUPERB	((daddr_t)1)	/* physical block number of the super block */
#define SUPERBOFF(dev)	(BSIZE) /* byte offset of the super block */

#endif /* _H_JFS_FSPARAM */




