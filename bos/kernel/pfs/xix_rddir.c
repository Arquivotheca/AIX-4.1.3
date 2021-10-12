static char sccsid[] = "@(#)24	1.12.1.5  src/bos/kernel/pfs/xix_rddir.c, syspfs, bos411, 9428A410j 7/7/94 16:55:24";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: jfs_readdir
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "jfs/jfslock.h"
#include "jfs/inode.h"
#include "sys/dir.h"
#include "sys/errno.h"
#include "sys/syspest.h"
#include "sys/malloc.h"

#define DBUFSIZE	2*BSIZE
/*
 * NAME:	jfs_readdir(vp, uiop, crp)
 *
 * FUNCTION:	Read directory
 *
 * PARAMETERS:	vp 	- pointer to the vnode that represents the 
 *			  directory we want to read
 *		uiop	- How much to read and where it goes
 *		crp	- credential
 *
 * RETURN :	EINVAL	- if not a directory or too many iovecs
 *		errors from subroutines
 *			
 */

# define	next(d, type)	((struct type *) ((caddr_t)d + d->d_reclen))

jfs_readdir(vp, uiop, crp)
struct vnode	*vp;
struct uio	*uiop;
struct ucred	*crp;		/* pointer to credential structure */
{	
	int 	rc = 0;		/* return code */
	struct	inode	*dp;	/* directory inode ptr */
	struct	uio	uio;	/* uio for intermediate readi() */
	struct 	iovec	iovec;	/* iovec for intermediate readi() */
	direct_t *d;		/* direct buffer ptr */
	caddr_t	mbuf = NULL;	/* malloc'd storage for dirents and directs */
	caddr_t	readdirbuf;	/* stack or malloc'd buffer pointer */
	caddr_t	tbuf;		/* bufs for intermediate calcs */
	struct  dirent *tdirp;	/* temp space dirent */
	off_t	off;		/* real offset holder */
	int 	ubytes;		/* bytes in user space */
	int 	kbytes;		/* bytes in kernel space */
	int 	cnt;		/* sizeof (readdirbuf) */
	int 	nbytes;		/* uiomove count */
	char	dbuf[DBUFSIZE];	/* stack storage for dirents and directs */

	/* read struct directs off disk and translate into struct dirents.
	 * unused DIRBLKSIZ blocks are not returned to the caller
	 * therefore, we must account for the space in the user's buffer
	 * separately.  no guarantees can be made that the exact offset 
	 * requested can be found.  we do the best we can.
	 */

	if (uiop->uio_iovcnt != 1 || vp->v_vntype != VDIR)
		return EINVAL;
	if (uiop->uio_resid == 0)
		return 0;

	dp = VTOIP(vp);
	IREAD_LOCK(dp);

        /* Read in ubytes size chunks which will be FsBSIZE 99.9% of
         * the time. (see opendir(3).c).  If directory is small then use
         * i_size for kernel space needs.  Very large requests will cause
         * obscenely large mallocs.  I'll ignore this problem since 
	 * readdir(3) is our largest(only?) customer and we know what he 
	 * does.  The code to break large requests into managable pieces 
	 * adds too much complexity.
         */
	ubytes = uiop->uio_resid;
	kbytes = MIN(dp->i_size, ubytes);

	/* if kbytes == i_size, kbytes is usually a multiple of 
	 * DIRBLKSIZ.  only in the case of a "molested" directory(by 
	 * unlink) will this not be true.  better safe that sorry.
	 */
	kbytes = (kbytes + (DIRBLKSIZ-1)) & ~(DIRBLKSIZ-1);

	/* If the requested size is greater than our stack variable dbuf,
	 * then we will have to allocate the storage with malloc().  This
	 * should rarely happen.  The dirents buffer will be larger than 
	 * kbytes.  Maximum possible size can be calculated as, 
	 * kbytes + (sizeof (d_offset) * (kbytes/LDIRSIZE(1))), but its 
	 * not worth it.
	 */
	if (kbytes + ubytes > DBUFSIZE)
	{
		if ((mbuf = (caddr_t)malloc((uint)(kbytes + ubytes))) == NULL)
		{
			rc = ENOMEM;
			goto out;
		}
		readdirbuf = (caddr_t)mbuf;
	}
	else
		readdirbuf = (caddr_t)dbuf;
		
	tbuf = readdirbuf + kbytes;
	tdirp = (struct dirent *)tbuf;
	nbytes = 0;

	/* setup intermediate uio structure for readi */
	uio.uio_iov = &iovec;

	/* Need to force offset to DIRBLKSIZ boundary.  this must
	 * be done to ensure that results of readi() can be interpretted
	 */
	off = uio.uio_offset = uiop->uio_offset & ~(DIRBLKSIZ-1);
	uio.uio_segflg = UIO_SYSSPACE;
	uio.uio_fmode = uiop->uio_fmode;
	uio.uio_iovcnt = 1;

	/* loop until request satisfied or error */

	while (off < dp->i_size)
	{
		uio.uio_resid = kbytes;
		iovec.iov_len = kbytes;
		iovec.iov_base = readdirbuf;

		/* all or nothing */
		if (rc = readi(dp, 0, 0, &uio))
			goto out;

		cnt = kbytes - uio.uio_resid;
		d = (struct direct *)readdirbuf;

		/* eof should not happen if tracking offset is correct */
		ASSERT(cnt != 0);

		while (cnt > 0)
		{
			/* off may start somewhere before requested offset
			 * due to the rounding of the user offset
			 */
			if (d->d_ino != 0 && off >= uiop->uio_offset)
			{
				/* see if enough space left for an
				 * entry of this size.  do not update
				 * any fields in tdirp until we
				 * are sure we have space.  don't
				 * want to clobber malloc'd memory.
				 */
				if ((nbytes + DIRSIZ(d)) > ubytes)
					goto done;

				tdirp->d_offset = off + d->d_reclen;
				tdirp->d_ino = d->d_ino;
				tdirp->d_namlen = d->d_namlen;

				/* DIRSIZ() > d->d_reclen */
				tdirp->d_reclen = DIRSIZ (tdirp);
				bcopy(d->d_name, tdirp->d_name, d->d_namlen+1);

				nbytes += tdirp->d_reclen;
				tdirp = next(tdirp, dirent);
			}

			cnt -= d->d_reclen;
			off += d->d_reclen;
			d = next(d, direct);
		}
	}

done:
	rc = uiomove(tbuf, nbytes, UIO_READ, uiop);

	/* establish real offset */
	uiop->uio_offset = off;
out:		
	if (mbuf)
		free((void *)mbuf);

	IREAD_UNLOCK(dp);
	return rc;
}
