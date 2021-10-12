/* @(#)05	1.23  src/bos/kernel/sys/buf.h, sysios, bos411, 9428A410j 5/19/94 09:04:54 */
#ifndef _H_BUF
#define _H_BUF
/*
 * COMPONENT_NAME: (SYSIOS) Buffer Header structure
 *
 * ORIGINS: 3, 27, 83
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *   LEVEL 1,  5 Years Bull Confidential Information
 */

/*
 *  Buffer cache buffer header
 *  A buffer header contains all the information required to perform block
 *  I/O.  It is the primary interface to the bottom half of block device
 *  drivers.  These drivers are needed for all file system and paging
 *  devices.  In AIX version 3, the traditional strategy() interface is
 *  extended as follows:
 *
 *    1.   The device driver strategy() routine is called with a
 *	   list of buf structures, chained using the av_forw pointer.
 *	   The last entry in this list has a NULL av_forw pointer.
 *
 *    2.   When the operation is completed, and the driver calls
 *	   iodone(), the b_iodone function is scheduled to run as a
 *	   INTIODONE software interrupt handler.  This function is
 *	   passed the buf struct address as its argument.
 *
 *  Buf structures are allocated by I/O requesters, and contain fields
 *  representing the state of some associated data page.  The device driver
 *  must leave most of the fields in this structure intact.  It is allowed
 *  to use av_forw and av_back to queue active requests, and it must set
 *  b_resid on return.	If there was an error, it sets B_ERROR in b_flags,
 *  and returns an errno value in b_error.
 *
 *  The buf struct and its associated data page must be pinned before
 *  calling the strategy() routine.  Block driver bottom halves run
 *  without access to user process context, and are not allowed to page
 *  fault.
 *
 *		    Kernel buffer cache management
 *
 * The block I/O routines use a pool of buf structures to manage
 * the kernel buffer cache.  Each buffer in the pool is usually
 * doubly linked into 2 lists:
 *
 *   1)  a hash list if it has useful contents, and
 *   2)  a list of blocks available for allocation.
 *
 * A buffer is on the available list, and can be reassigned to another
 * disk block, if and only if it is not marked BUSY.  When a buffer
 * is busy, the available-list pointers can be used for other purposes.
 * Most drivers use the forward ptr as a link in their I/O active queue.
 * A buffer header contains all the information required to perform I/O.
 * Most of the routines which manipulate these things are in bio.c.
 */

#include <sys/types.h>
#include <sys/xmem.h>
#include <sys/time.h>

struct buf {				/* buffer header		 */
	int	b_flags;		/* flag word (see defines below) */

	struct	buf *b_forw;		/* hash list forward link	 */
	struct	buf *b_back;		/* hash list backward link	 */
	struct	buf *av_forw;		/* free list forward link	 */
	struct	buf *av_back;		/* free list backward link	 */

	void	(*b_iodone)();		/* ptr to iodone routine	 */
	struct	vnode *b_vp;		/* vnode associated with block	 */
	dev_t	b_dev;			/* major+minor device name	 */
	daddr_t b_blkno;		/* block # on device or in file  */

	union {
	    caddr_t  b_addr;		/* buffer address		 */
	} b_un;

	unsigned     b_bcount;		/* transfer count, OR		  */
					/* #blks in list (bfreelist only) */
	char	     b_error;		/* returned after I/O		  */
	unsigned int b_resid;		/* words not xferred after error  */
	int	     b_work;		/* work area for device drivers   */
	int	     b_options; 	/* readx/writex extension options */
	int	     b_event;		/* anchor for event list	  */
	struct timestruc_t b_start;	/* request start time		  */
	struct	     xmem b_xmemd;	/* xmem descriptor		  */
};

#define	b_baddr		b_un.b_addr	/* address of data		  */

/*
 * These flags are kept in b_flags.
 */
#define B_WRITE 	0x0000	/* non-read pseudo-flag */
#define B_READ		0x0001	/* read when I/O occurs */
#define B_DONE		0x0002	/* I/O complete */
#define B_ERROR 	0x0004	/* error detected */
#define B_BUSY		0x0008	/* in use or I/O in progress */
#define B_INFLIGHT	0x0020	/* this request is in-flight */
#define B_AGE		0x0080	/* put at head of freelist when released */
#define B_ASYNC 	0x0100	/* don't wait for I/O completion */
#define B_DELWRI	0x0200	/* don't write till block is reassigned */
#define B_NOHIDE	0x0400	/* don't hide data pages during dma xfer */
#define B_STALE 	0x0800	/* data in buffer is no longer valid */
#define B_MORE_DONE	0x1000	/* more buffers to be processed */

#define B_PFSTORE 	0x2000	/* store operation */
#define B_PFPROT 	0x4000	/* protection violation */
#define B_SPLIT 	0x8000	/* ok to enable split read/write */
#define B_PFEOF 	0x10000	/* check for reference beyond end-of-file */
#define B_MPSAFE	0x40000 /* Invoker of strategy() is MP safe */
#ifdef _KERNSYS
#define B_MPSAFE_INITIAL 0x80000 /* devstrat() converts B_MPSAFE into */
				/*	this flag */
#define B_COMPACTED	0x100000 /* comapcted coalesce list */
#define	B_DONTUNPIN	0x200000 /* pin() failed - don't unpin buf */
#endif
/*
 * The following services provide the interface to the buffer cache
 * and block I/O.
 */

#ifdef _KERNEL
#ifndef _NO_PROTO

struct buf * getblk(		/* allocate uninitialized buffer to block */
	dev_t dev,		/* the device containg block */
	daddr_t blkno);		/* the block to be allocated */

struct buf *geteblk();		/* allocate uninitialized buffer	*/

struct buf *bread( 		/* allocate buffer to block and read it */
	dev_t dev,		/* the device containg block */
	daddr_t blkno);		/* the block to be allocated */

struct buf *breada( 		/* allocate buffer to block and read it */
	dev_t dev,		/* the device containg block */
	daddr_t blkno,		/* the block to be allocated */
	daddr_t rablkno);	/* read ahead block */

void brelse(	 		/* free buffer; no I/O implied */
	struct buf *bp); 	/* buffer to be released */

int bwrite(	 		/* write buffer; then free it */
	struct buf *bp); 	/* buffer to be written */

void bdwrite(	 		/* mark buffer for delayed write and free it */
	struct buf *bp); 	/* buffer to be written */

int bawrite(	 		/* async write buffer; then free it */
	struct buf *bp); 	/* buffer to be written */

void bflush(	 		/* flush all delayed write blocks */
	dev_t dev);		/* the device containg blocks */

int blkflush(			/* flush the delayed write block */
	dev_t dev,		/* the device containg block */
	daddr_t blkno);		/* the block to be flushed */

void binval(	 		/* invalidate all blocks */
	dev_t dev);		/* the device containg blocks */

int iowait(			/* wait for I/O completion */
	struct buf *bp); 	/* buffer to wait for completion of */

void iodone(	 		/* call the requester's I/O done routine */
	struct buf *bp); 	/* buffer with completed operation */

#else

struct buf *getblk();
struct buf *geteblk();
struct buf *bread();
struct buf *breada();
void brelse();
int bwrite();
void bdwrite();
int bawrite();
void bflush();
int blkflush();
void binval();
int iowait();
void iodone();

#endif /* not _NO_PROTO */
#endif /* _KERNEL */

#endif /* _H_BUF */
