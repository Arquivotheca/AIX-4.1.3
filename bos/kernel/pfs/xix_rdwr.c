static char sccsid[] = "@(#)26	1.41.1.14  src/bos/kernel/pfs/xix_rdwr.c, syspfs, bos41B, 9506A 1/25/95 13:31:44";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: readi, writei, jfs_rdwr
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "jfs/jfslock.h"
#include "sys/errno.h"
#include "sys/vmount.h"
#include "sys/user.h"
#include "sys/file.h"
#include "sys/syspest.h"
#include "sys/trchkid.h"
#include "sys/uio.h"

/* declare debug variables for read and write */
BUGVDEF(xrdbug, 0);
BUGVDEF(xwrbug, 0);

/*
 * NAME:	jfs_rdwr(vp, rw, flags, uiop, ext, vinfo, vattrp, crp)
 *
 * FUNCTION:	Common code for AIX filesystem read and write calls:
 * 		check locks; set base, count, and offset;
 * 		and switch out to readi or writei code.
 *
 * PARAMETERS:	vp 	- pointer to the vnode that represents the 
 *			  object we want to read
 *		rw	- read or write
 *		flags	- open flags
 *		uiop	- howmuch to read/write and where it goes
 *		ext	- unused
 *		vinfo	- unused
 *		vattrp	- attributes to be returned
 *		crp	- credential
 *
 * RETURN :	EAGAIN	- if enforced locks and we can't wait
 *		errors from subroutines
 *			
 */

jfs_rdwr(vp, rw, flags, uiop, ext, vinfo, vattrp, crp)
struct vnode 	*vp;		/* Vnode for open file		*/
enum uio_rw 	rw;		/* Read or write		*/
int 		flags;		/* Open flags			*/
struct uio 	*uiop;		/* Uio info			*/
int 		ext;		/* Extended open ?		*/
caddr_t 	vinfo;		/* if remote			*/
struct vattr	*vattrp;	/* attributes on return		*/
struct ucred	*crp;		/* pointer to credential	*/
{
	int	rc = 0;		/* Return code			*/
	struct inode *ip;	/* Inode for vp			*/
	int type;		/* File type			*/
	long ulimit, orig_len;	/* Length restrictions		*/
	int	resid;		/* adjust value for resid	*/
	extern int ireadlockx(), ireadunlockx();
	extern int iwritelockx(), iwriteunlockx();

	ip = VTOIP(vp);
	BUGLPR(xwrbug+xrdbug, BUGNTF, \
		("jfs_rdwr vp=%X rw=%X fl=%X up=%X ex=%X vi=%X\n",\
		vp, rw, flags, uiop, ext, vinfo));

	type = ip->i_mode & IFMT;

	TRCHKL2T(HKWD_KERN_PFS|hkwd_PFS_RDWR, vp, ip);

	if (uiop->uio_offset < 0 || uiop->uio_offset > OFF_MAX)
		return EINVAL;

	if (rw == UIO_READ)
	{
		IREAD_LOCK(ip);
	}
	else
	{
		IWRITE_LOCK(ip);

		if (flags & FAPPEND)
		{	
			uiop->uio_offset = ip->i_size;
			orig_len = uiop->uio_resid;
		}
	}

retry:
	/* Make sure that the user can write all the way up
	 * to the ulimit value.
	 */

	resid = 0;
	if (type == IFREG && rw == UIO_WRITE)
	{	
		if (uiop->uio_fmode & FKERNEL)
			u.u_flags |= UTNOULIMIT;
		else
		{
			ulimit = (uint)U.U_limit - uiop->uio_offset;
 			if (uiop->uio_resid > ulimit)
			{
				if (ulimit > 0)	
				{  
					/* allow partial write up to u_limit */
					resid = uiop->uio_resid - ulimit;
					uiop->uio_resid = ulimit;
				} 
				else	/* writing past u_limit - error */
				{
					rc = EFBIG;
					goto out;
				}
			}
		}
	} 
		
	if (type == IFREG && ENF_LOCK(ip->i_mode))
	{	
		/* enforce record locking protocol */
		int i;
		struct eflock bf;

		bf.l_type = (rw & UIO_WRITE) ? F_WRLCK : F_RDLCK;
		bf.l_whence = 0;
		bf.l_start = uiop->uio_offset;
		bf.l_len = uiop->uio_resid;
		bf.l_pid = U.U_procp->p_pid;
		bf.l_sysid = 0;
		bf.l_vfs = MNT_JFS;
		i = (flags & (FNDELAY|FNONBLOCK)) ? INOFLCK : SLPFLCK|INOFLCK;

		rc = common_reclock (ITOGP(ip),
				     ip->i_size,
				     (off_t) uiop->uio_offset,
				     &bf,
				     i,
				     0,
				     0,
				     (rw == UIO_READ) ? ireadlockx : iwritelockx,
				     (rw == UIO_READ) ? ireadunlockx : iwriteunlockx);
		if (rc || bf.l_type != F_UNLCK)
		{
			if (rc == 0)
				rc = EAGAIN;
			goto exit;
		}

		/* If writting in append mode, and the file size
		 * changed while sleeping on an enforced mode file
		 * lock, repeat offset calculations and check
		 * for blocking locks at the new offset.
		 */
		if ((flags & FAPPEND) && rw == UIO_WRITE &&
			uiop->uio_offset != ip->i_size)
		{	
			uiop->uio_resid = orig_len;
			uiop->uio_offset = ip->i_size;
			goto retry;
		}
	}

	if (rw == UIO_READ)
		rc = readi(ip, flags, ext, uiop);
	else
	{
		rc = writei(ip, flags, ext, uiop, crp);
		u.u_flags &= ~UTNOULIMIT;
	}

	if (vattrp != NULL && rc == 0)
		(void) get_vattr (ip, vattrp);

exit:
	uiop->uio_resid += resid;

out:
	if (rw == UIO_READ)
		IREAD_UNLOCK(ip);
	else
		IWRITE_UNLOCK(ip);
	BUGLPR(xrdbug+xwrbug, BUGNTF, ("jfs_rdwr: rc=%d\n", rc));
	return rc;
}


/*
 * NAME:	readi (ip, flags, ext, uiop)
 *
 * FUNCTION:	Common code for reading an inode
 *
 * PARAMETERS:	ip 	- pointer to the inode that represents the 
 *			  object we want to read
 *		flags	- open flags
 *		ext	- unused
 *		uiop	- howmuch to read/write and where it goes
 *
 * RETURN :	EINVAL	- for negative offsets
 *		errors from subroutines
 *			
 */

readi(ip, flags, ext, uiop)
struct inode	*ip;		/* Inode to read		*/
int	flags;			/* Open style flags		*/
caddr_t	ext;			/* Extended info		*/
struct uio	*uiop;		/* Uio info			*/
{
	int	rc = 0;		/* return value			*/
	uint	n;
	int 	type, rem;
	off_t	off;		/* Offset into file		*/
	caddr_t	vaddr;

	BUGLPR(xrdbug, BUGNTF, ("readi: ip=%X fl=%X ext=%X up=%X\n", \
		ip, flags, ext, uiop));

	/* Ignore attempts to read 0 bytes
	 */
	if (uiop->uio_resid <= 0)
		return 0;

	type = ip->i_mode & IFMT;
	assert(type == IFREG || type == IFDIR || type == IFLNK);

	/* map into virtual memory if necessary
	 */
	if (ip->i_seg == 0)
		if (rc = ibindseg(ip))
			return rc;

	/* using uio_resid allows a call with multiple iovecs to work.
	 * since file is contiguous in virtual address space uiomove can
	 * parse the iovecs.
	 */
	off = uiop->uio_offset;
	rem = ip->i_size - off;

	if (rem > 0)
	{	
		n = (rem < uiop->uio_resid) ? rem : uiop->uio_resid;

		TRCHKL4T(HKWD_KERN_PFS|hkwd_PFS_READI, ip, ip->i_seg, off, n);

		rc = vm_move(ip->i_seg, off, n, UIO_READ, uiop);
	}

	/* always update access time, even if we're at/past EOF */
	INODE_LOCK(ip);
	imark(ip, IACC);
	INODE_UNLOCK(ip);

	return rc;
}


/*
 * NAME:	writei (ip, flags, ext, uiop, crp)
 *
 * FUNCTION:	Common code for writing an inode
 *
 * PARAMETERS:	ip 	- pointer to the inode that represents the 
 *			  object we want to read
 *		flags	- open flags
 *		ext	- unused
 *		uiop	- howmuch to write and where it goes
 *		crp	- credential
 *
 * RETURN :	EINVAL	- for negative offsets
 *		errors from subroutines
 *			
 */

#define EXEC_GO(x)	(((x)->i_mode & ((IEXEC >> 3)|(IEXEC >> 6))) != 0)
#define EXEC_UO(x)	(((x)->i_mode & (IEXEC|(IEXEC >> 6))) != 0)

writei(ip, flags, ext, uiop, crp)
struct inode *ip;		/* File to write		*/
int flags;			/* Open style flags		*/
caddr_t ext;			/* Extended info 		*/
struct uio *uiop;		/* Where to info		*/
struct ucred *crp;		/* credential			*/
{
	int	rc;
	off_t	off;
	caddr_t	vaddr;
	int	type, maxisize,	oldsize, length;
	void	clearprivbits();
	struct	ucred *ucp = NULL;

	BUGLPR(xwrbug, BUGNTF, ("writei: ip=%X fl=%X ext=%X up=%X\n", \
		ip, flags, ext, uiop));

	type = ip->i_mode & IFMT;

	assert(type == IFREG || type == IFLNK);
	
	/* map into virtual memory if necessary
	 */
	if (ip->i_seg == 0)
		if (rc = ibindseg(ip))
			return rc;

	/* Page fault quota allocation check looks at uid but NFS 
	 * server which is mono threaded always runs as uid zero.
	 */
	if (U.U_procp->p_threadcount == 1 && crp->cr_uid != U.U_uid)
	{
		ucp = U.U_cred;
		U.U_cred = crp;
	}
		
	/* using uio_resid allows a call with multiple iovecs to work.
	 * since file is contiguous in virtual address space uiomove can
	 * parse the iovecs.
	 */
	off = uiop->uio_offset;
	oldsize = ip->i_size;
	length = uiop->uio_resid;

	TRCHKL4T(HKWD_KERN_PFS|hkwd_PFS_WRITEI, ip, ip->i_seg, off,
		length);


	rc = vm_move(ip->i_seg, off, length, UIO_WRITE, uiop);

	if (ucp)
		U.U_cred = ucp;

	/* check if data was moved or if the size has changed.  the size
	 * can change without data movement if the last logical block of
	 * the file was promoted from a partial fragment allocation to
	 * full block but the move operation failed.
	 */
	if (off != uiop->uio_offset || oldsize != ip->i_size) 
	{
		/* clear the privledge bit and mark the inode as changed
		 * and updated.
		 */
		clearprivbits(ip, crp);
		imark(ip, IUPD|ICHG);

		if (off != uiop->uio_offset)
		{
			/* i_size may have been updated by file system pager
			 * or the fragment allocator and both place i_size at
			 * the end of the last logical block's disk allocation.
			 * however, i_size must be kept exact in the case of
			 * write(2).
		 	 */
			maxisize = MAX(uiop->uio_offset, oldsize);
			isetsize(ip, maxisize);

			/* sync the data if FSYNC is set. */
			if (flags & FSYNC)
			{
				int syncrc;

				syncrc = isyncdata(ip, off, length, oldsize,
						flags & FSYNCALL);

				/* If there was an error returned from vm_move,
				 * then return that to the caller, otherwise
				 * return the result of the isyncdata call.
				 */
				if (rc == 0)
					rc = syncrc;
			}
		}
	}
	
	return rc;
}


/*
 * NAME: clearprivbits
 *
 * FUNCTION: called on modification of file to clear suid, sgid, TCB, and
 *	TB bits if file modified by non-privledged user
 *
 * EXECUTION ENVIRONMENT: called by jfs_rdwr, jfs_unmap, jfs_fclear jfs_trunc
 *	vnode ops
 *
 * RETURNS: NONE
 */
void
clearprivbits(ip, crp)
struct inode *ip;
struct ucred *crp;
{

	/* What we'd like to do is clear the suid and sgid
	 * bits anytime someone other than owner (resp.
	 * groupmember) writes to the file.  However, we use
	 * sgid to indicate enforced record locking if no one
	 * can execute the file, so we have to be more careful
	 * about clearing bits.  We could just check for
	 * any execute permissions, but we're going to allow
	 * the benign (if senseless) cases.  Mode 4666 (rwSrw-rw-)
	 * doesn't mean anything special now, but it may someday.
	 *
	 * when writing to setuid and setgid files:
	 * if writer has SET_OBJ_DAC privilege
	 *    don't clear anything
	 * setuid:
	 *    if writer != owner && executable by group or other (_GO)
	 *       clear the suid bit
	 * setgid:
	 *    if writer isn't groupmember
	 *       && executable by owner or other (_UO)
	 *       clear the sgid bit
	 */
	if (privcheck_cr(SET_OBJ_DAC, crp)) {
		if ((crp->cr_uid != ip->i_uid) && EXEC_GO(ip))
			ip->i_mode &= ~ISUID;
		if (!groupmember_cr(ip->i_gid, crp) && EXEC_UO(ip))
			ip->i_mode &= ~ISGID;
	}

	/*
	 * modification by non-privileged users turns off the
	 * TCB and TP bits.
 	 */
	if (privcheck_cr(SET_OBJ_PRIV, crp))
		ip->i_mode &= ~(S_ITP|S_ITCB);
}
