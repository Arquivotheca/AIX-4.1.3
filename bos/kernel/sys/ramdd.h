/* @(#)26	1.2  src/bos/kernel/sys/ramdd.h, sysio, bos411, 9428A410j 8/26/93 15:24:51 */
/*
 * COMPONENT_NAME: SYSIO
 *
 * FUNCTIONS: Compressed ram image for ram disk device driver
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_RAMDD
#define _H_RAMDD

#ifdef _KERNEL
#define PSIZE PAGESIZE
#else
#define PSIZE 4096
#endif

/*
 * magic number for ram disk image
 */
#define RAM_MAGIC "\001\002\003\004"
#define LAST_BLOCK 0xffff		/* end of list marker		*/
#define RAM_ALLOC (16*PSIZE)		/* amount to bump high water	*/
					/* mark by			*/

#define RAM_WPACK 0			/* compress new blocks on write */
#define RAM_WCOPY 1			/* do not compress new blocks	*/

/*
 * one structure exists for each block in the ram image
 */
struct ramblock {
	uint rb_addr;			/* offset into segment		*/
	ushort rb_prev;			/* previously allocated block	*/
	ushort rb_next;			/* next allocated block		*/
	ushort rb_size;			/* size of compressed bock	*/
};

/*
 * ramimage strucure starts at beginning of segment.  A r_block entry
 * exists for each block in the ram disk.  r_fist contains the block
 * number of the lowest (data offset) block in image. r_last contains the
 * block number of the highest (data offset) block of image. r_alloc
 * contains the offset of the end of the segment.
 */
struct ramimage {
	char r_magic[4];		/* magic number			*/
	ushort r_blocks;		/* nuber of blocks in ram image */
	short r_flag;			/* ram disk flags		*/
	uint r_segsize;			/* size in bytes of segment	*/
	uint r_alloc;			/* next available offset in seg	*/
	uint r_high;			/* high water mark in segment	*/
	ushort r_first;			/* lowest address block		*/
	ushort r_last;			/* highest address block	*/
	struct ramblock r_block[1];	/* block data			*/
};

#define RAM_HEAD_SIZE ((int)&(((struct ramimage *)0)->r_block[0]))
#define DATA_START(rd)	\
 ((int)&(((struct ramimage *)0)->r_block[rd->r_blocks]))

			/* RAM disk device driver ioctl to  */
#define	RIOCSYSMEM 3	/*  return total system memory size */

#endif /* H_RAMDD */
