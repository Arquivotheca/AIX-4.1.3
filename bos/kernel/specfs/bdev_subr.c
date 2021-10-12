static char sccsid[] = "@(#)93	1.22.1.4  src/bos/kernel/specfs/bdev_subr.c, sysspecfs, bos411, 9428A410j 4/19/94 16:16:34";
/*
 * COMPONENT_NAME: (SYSSPECFS) Special File Filesystem
 *
 * FUNCTIONS: bdev_bmap, bdev_close, bdev_open,
 *            bdev_rdwr,  bdev_read, bdev_write
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/user.h"
#include "sys/vfs.h"
#include "sys/uio.h"
#include "sys/buf.h"
#include "sys/vnode.h"
#include "sys/vattr.h"
#include "sys/time.h"
#include "sys/errno.h"
#include "sys/conf.h"
#include "sys/syspest.h"
#include "sys/device.h"
#include "sys/specnode.h"
#include "sys/fs_locks.h"
#include "sys/limits.h"

/* Definitions */

/* Declarations */

static readblk(), writeblk();

static bdev_bmap( struct gnode *,
		  offset_t,
		  unsigned,
		  daddr_t *,
		  struct blockio *);

static struct blockio {
	ushort pbsize;
	ushort pboff;
	dev_t pbdev;
	daddr_t rablock;
};

/*
 * NAME:	bdev_open(dgp, flag, ext)
 *
 * FUNCTION:	This function opens the block device referenced by the
 *		specified block device special file.
 *
 * PARAMETERS:	dgp	- device gnode of device to open
 *		flag	- file open flags
 *		ext	- extended open info
 *
 * RETURN :	returns from subroutines
 *		ENXIO	- device number invalid
 *		EBUSY	- if current open is for mount and there are writers
 *		EINTR	- interupted in device open routine
 */

bdev_open (
	struct gnode *	dgp,		/* dev gnode of device to open	*/
	int		flag,		/* file open flags		*/
	int		ext)		/* extended open information	*/
{
	int		rc = 0;		/* return code			*/
	dev_t		dev;		/* device number		*/

	dev = dgp->gn_rdev;

	/* register device open */
	rc = rdevopen(dev, flag, NULL, ext, &dgp);

	return rc;
}

/*
 * NAME:	bdev_rdwr(sgp, pvp, rwmode, flags, uiop, ext, crp)
 *
 * FUNCTION:	Read or write block device
 *
 * PARAMETERS:	sgp	- device gnode to r or w
 *		pvp	- vnode of PFS block device special file
 *		rwmode	- read or write type (UIO_READ or UIO_WRITE)
 *		flag	- file open flags
 *		uiop	- read or write location information
 *		ext	- extended read or write info
 *				(ignored for block devices)
 *
 * RETURN :	returns from subroutines
 *		returns from bio interface
 *		EINVAL	- negative r/w offset
 */

int
bdev_rdwr (
	struct gnode *	sgp,		/* dev gnode of device		*/
	struct vnode *	pvp,		/* PFS vnode of device		*/
	enum uio_rw 	rwmode,		/* UIO_READ or UIO_WRITE	*/
	int 		flags,		/* file open flags		*/
	struct uio *	uiop,		/* read or write location info	*/
	int 		ext,		/* extended open flag		*/
	struct ucred *  crp)		/* credentials			*/
{
	struct gnode *	dgp;		/* device gnode of device	*/
	int		rc;		/* return code			*/

	/* 
	 * negative offsets and offsets that cannot fit in a buffer header 
	 * b_blkno are not allowed.
	 */
	if (uiop->uio_offset < 0 || uiop->uio_offset > DEV_OFF_MAX)
		return EINVAL;

	dgp = SGTODGP(sgp);

	if (rwmode == UIO_READ)
		rc = readblk(dgp, pvp, uiop, crp);
	else	/* UIO_WRITE */
		rc = writeblk(dgp, pvp, flags, uiop, crp);

	return rc;
}

static
int
readblk (
	struct gnode *	dgp,		/* dev gnode of device to read	*/
	struct vnode *	pvp,		/* PFS vnode accessed		*/
	struct uio *	uiop,		/* read location information	*/
	struct ucred *  crp)		/* credentials			*/
{
	struct buf *	bp;		/* buf struct for read data	*/
	dev_t		dev;		/* device number		*/
	daddr_t		blockno;	/* block number on device	*/
	struct iovec *	iov;		/* read destination location	*/
	unsigned	pboff;		/* offset within physical block	*/
	unsigned	cnt;		/* bytes to read in phys block	*/
	int		rc = 0;		/* return code			*/
	struct blockio 	blkio;		/* description of block read	*/
	int		didread = 0;	/* read successfully flag	*/
	struct timestruc_t t;

	iov = uiop->uio_iov;

	/* Keep reading blocks and moving data until the read request
	 * is satisfied or an error occurs
	 */
	while (rc == 0 && iov->iov_len != 0)
	{
		/* get block numbers, offsets, and sizes */
		rc = bdev_bmap(dgp,
			       uiop->uio_offset,
			       iov->iov_len,
			       &blockno,
			       &blkio);

		/* check for error or zero length read */
		if (rc || (cnt = blkio.pbsize) == 0)
			break;

		/* We want to do an asynchronous read (read ahead) of
		 * the block indicated by bdev_bmap(), if one was
		 * specified.  This is done in addition to the
		 * synchronous read of the current block.
		 */
		pboff = blkio.pboff;
		dev = blkio.pbdev;
		bp = (blkio.rablock)?
			breada(dev, blockno, blkio.rablock)
			: bread(dev, blockno);

		/* We halt the read if we get an error or if the amount
		 * of the block that was successfully read does not
		 * entirely include the portion we need.
		 */
		if (((rc = bp->b_error) != 0) ||
				(pboff + cnt > bp->b_bcount - bp->b_resid))
		{
			brelse(bp);
			break;
		}

		/* copy the data from the buffer to the destination */
		rc = uiomove(bp->b_baddr + pboff, cnt, UIO_READ, uiop);
		brelse(bp);

		/* remember that we successfully read for time update */
		didread = 1;

	}

	/* Update the access time on the PFS file if we read
	 * successfully.
	 */
	if (didread && pvp) {
		curtime(&t);
		(void)VNOP_SETATTR(pvp, V_STIME, &t, 0, 0, crp);
	}

	return rc;
}

static
int
writeblk (
	struct gnode *	dgp,		/* dev gnode of device to write	*/
	struct vnode *	pvp,		/* PFS vnode accessed		*/
	int		flags,		/* file open flags		*/
	struct uio *	uiop,		/* write location information	*/
	struct ucred *  crp)		/* credentials			*/
{
	struct buf *	bp;		/* buf struct for write data	*/
	dev_t		dev;		/* device number		*/
	daddr_t		blockno;	/* block number on device	*/
	struct iovec *	iov;		/* write destination location	*/
	unsigned	pboff;		/* offset within physical block	*/
	unsigned	cnt;		/* bytes to write in phys block	*/
	int		rc = 0;		/* return code			*/
	struct blockio 	blkio;		/* description of block write	*/
	int		didwrite = 0;	/* written successfully flag	*/
	struct timestruc_t t;

	iov = uiop->uio_iov;

	/* Keep reading blocks, moving data, and writing blocks until
	 * there is no more data to write or an error occurs.
	 */
	while (rc == 0 && iov->iov_len != 0)
	{
		/* get block numbers, offsets, and sizes */
		rc = bdev_bmap(dgp,
			       uiop->uio_offset,
			       iov->iov_len,
			       &blockno,
			       &blkio);

		/* check for error or zero length write */
		if (rc || (cnt = blkio.pbsize) == 0)
			break;

		/* We only want to read the block if the write will not
		 * cover the entire block.  In this case, we read the
		 * block to get data that will be unchanged by the write.
		 */
		pboff = blkio.pboff;
		dev = blkio.pbdev;
		bp = (cnt == FsBSIZE(dev))?
			getblk(dev, blockno)
			: bread(dev, blockno);
		if ((rc = bp->b_error) != 0)
		{
			brelse(bp);
			break;
		}

		/* copy the data from the source location to the buffer */
		rc = uiomove(bp->b_baddr + pboff, cnt, UIO_WRITE, uiop);
		if (rc)
		{
			brelse (bp);
			break;
		}

		/* If the device was opened for synchronous writes, do
		 * the write synchronously.  Otherwise, schedule the
		 * write.
		 */
		if (flags & FSYNC)
			rc = bwrite(bp);
		else
		{

			/* This is not a write for a filesystem, so there
			 * is no real reason to cache the buffered data.
			 */
			bp->b_flags |= B_AGE;
			rc = bawrite(bp);
		}

		/* Remember that we have successfully written for
		 * time update.
		 */
		didwrite = 1;
	}

	/* Update the update and change times on the PFS file if we
	 * have written successfully.
	 */
	if (didwrite && pvp) {
		curtime(&t);
		(void)VNOP_SETATTR(pvp, V_STIME, 0, &t, &t, crp);
	}

	return rc;
}

static
int
bdev_bmap (
	struct gnode *	dgp,		/* dev gnode for block device	*/
	offset_t	offset,		/* offset in device		*/
	unsigned	count,		/* length of I/O operation	*/
	daddr_t *	bnp,		/* block number return		*/
	struct blockio *biop)		/* block I/O description return	*/
{
	struct devnode *dp;		/* devnode for block device	*/
	struct buf *	bp;		/* buffer for reading data	*/
	daddr_t		blockno;	/* logical block num on device	*/
	dev_t		dev;		/* device number		*/
	ulong		iosz;		/* size of I/O data in block	*/
	int		raflag;		/* block read ahead flag	*/

	dp = DGTODP(dgp);

	/* We don't know whether we are going to use read ahead. */
	biop->rablock = 0;
	raflag = 0;

	/* get device number */
	dev = dgp->gn_rdev;
	biop->pbdev = dev;

	/* get the logical block number for this offset */
	blockno = (daddr_t) FsBNO(dev, offset);

	if (blockno < 0)
		return EFBIG;
	
	/* Take the devnode lock when changing the read ahead fields */
	DEVNODE_LOCK(dp);

	/* If we last read the previous block, we want to read the
	 * current block and schedule the next block (read ahead).
	 */
	if ((dp->dv_lastr + 1) == blockno)
		raflag = 1;

	/* calculate the physical block offset */
	biop->pboff = FsBOFF(dev, offset);

	/* Calculate the amount of data remaining to be read in the
	 * current physical block.
	 */
	iosz = FsBSIZE(dev) - biop->pboff;

	/* If the read will not extend beyond this block, turn off
	 * read ahead.
	 */
	if (count < iosz)
	{
		iosz = count;
		raflag = 0;
	}
	else
		dp->dv_lastr = blockno;

	/* release the devnode lock */
	DEVNODE_UNLOCK(dp);

	/* save the actual amount of data to move */
	biop->pbsize = (ushort)iosz;

	/* If we are going to do read ahead, indicate which block to
	 * read asynchronously.
	 */
	if (raflag)
		biop->rablock = blockno + 1;

	/* return the logical block number to read (synchronously) */
	*bnp = blockno;
	return 0;
}
