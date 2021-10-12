static char sccsid[] = "@(#)23	1.22.1.7  src/bos/kernel/pfs/xix_open.c, syspfs, bos411, 9430C411a 7/20/94 10:39:05";
/*
 * COMPONENT_NAME: (SYSPFS) open vnop
 *
 * FUNCTIONS: execaccess ip_access ip_open jfs_open
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
 *
 */

#include "jfs/jfslock.h"
#include "jfs/inode.h"
#include "jfs/commit.h"
#include "sys/user.h"
#include "sys/errno.h" 
#include "sys/file.h" 
#include "sys/flock.h"
#include "sys/sleep.h"
#include "sys/syspest.h"
#include "sys/vfs.h"
#include "sys/trchkid.h"

BUGVDEF(xopendbg,0)
/*
 * NAME:	jfs_open(vp, flag, ext, vinfop, crp)
 *
 * FUNCTION:	calls a common function that opens a file.
 *
 * PARAMETERS:	Vp	- vnode to open
 *		Flag	- open flags from the file pointer.
 *		Ext	- external data used by the device driver.
 *		Vinfop	- pointer to the vinfo field for the open file
 *		crp	- credential
 *
 * RETURNS:	ENOENT	- non-positive link count
 *		errors from subroutines
 */

jfs_open(vp, flag, ext, vinfop, crp)
struct vnode	*vp;		/* Vnode of file to open	*/
int		flag;		/* Open(2) flags		*/
int		ext;		/* Extended info for devs	*/
caddr_t		*vinfop;	/* gfs specific pointer		*/
struct ucred	*crp;		/* pointer to credential structure */
{
	int	rc;		/* Return code			*/
	struct inode *ip;	/* Inode of vnode to open	*/
	int 	type;

	BUGLPR(xopendbg,BUGNTF,	\
		("jfs_open:vp=%X fl=%X ex=%X ex=%X\n", \
		vp, flag, ext));

	ip = VTOIP (vp);
	type = ip->i_mode & IFMT;

	TRCHKL1T(HKWD_KERN_VFSINO | (vp->v_vfsp->vfs_number & 0xffff),
		ip->i_number);

	/* Until specfs flag this condition
	 */
	if (! (type == IFREG || type == IFDIR))
		return EINVAL;

	/*
	 * only map defered update for regular files
	 */
	if ((flag & FDEFER) && (type != IFREG))
		return EINVAL;

	IWRITE_LOCK(ip);

	/* 
	 * if the inode's link count less than one then set an error, 
	 * release the inode, and return
	 */
	if (ip->i_nlink < 1)
		rc = ENOENT;
	else
	{
		rc = ip_access(ip, flag, crp);
		if (rc == 0)
			rc = ip_open(ip, flag, ext, crp);
	}

	IWRITE_UNLOCK(ip);
	return rc;
}


/*
 * NAME:	ip_open(ip, flag, ext, crp)
 *
 * FUNCTION:	common function that opens a file.
 *
 * PARAMETERS:	ip	- inode to open
 *		Flag	- open flags from the file pointer.
 *		Ext	- external data used by the device driver.
 *		crp	- credential
 *
 * RETURNS:	EBUSY	- if truncating a mapped file
 *		errors from subroutines		
 */

ip_open(ip, flag, ext, crp)
struct inode *ip;		/* Inode of file to open	*/
int	flag;			/* Open(2) flags		*/
int	ext;			/* Extended info for devs	*/
struct ucred *crp;		/* cred pointer			*/
{
	int rc = 0;			/* Return code		*/
	int iflag;			/* inode mode		*/

	BUGLPR(xopendbg,BUGNTF,	\
		("ip_open:ip=%X fl=%X ex=%X\n", ip, flag, ext));

	if ((flag & FTRUNC) && (flag & FDEFER))
		return EBUSY;

	if (flag & FDEFER) {
		ASSERT(ITOGP(ip)->gn_excnt == 0);

		if (!(ip->i_flag & IDEFER)) {
			if (ip->i_seg != 0) {
				ip->i_flag |= IFSYNC;
				if (rc = commit(1, ip))
					return rc;
				vcs_defer(ip->i_seg);
			}
			ip->i_flag |= IDEFER;
		}
	}

	/* Bind the newly opened file to vm segment.  If ok then check
	 * for truncate.
	 */
	if ((ip->i_seg || (rc = ibindseg(ip)) == 0))
		if (flag & FTRUNC) {
			clearprivbits(ip, crp);
			rc = itrunc(ip,0,crp);
		}

	if (rc == 0) {
		if (flag & FEXEC) {
			if ((ip->i_flag & IUPD) || (ismodified(ip))) {
				ip->i_flag |= IFSYNC;
				if (rc = commit(1,ip))
					return(rc);
			}
			ITOGP(ip)->gn_excnt++;
		}
		if (flag & FREAD)
			ITOGP(ip)->gn_rdcnt++;
		if (flag & FWRITE)
			ITOGP(ip)->gn_wrcnt++;
		if (flag & (FRSHARE|FEXEC))
			ITOGP(ip)->gn_rshcnt++;
		if (flag & FNSHARE)
			ITOGP(ip)->gn_flags |= GNF_NSHARE;
	}

	BUGLPR(xopendbg,BUGNTF,("ip_open: rc=%d\n",rc));
	return rc;
}


/*
 * NAME:	ip_access (ip, flag, crp)
 *
 * FUNCTION:	Common routine to check access permissions before an open
 *
 * PARAMETERS:	ip	- inode in question
 *		flag	- permissions to check
 *		crp	- credential
 *
 * RETURN :	EISDIR	- if request to modify directory
 *		EACCES	- if trucating an enforced record locked file
 *		errors from called subroutines
 *			
 */

ip_access (ip, flag, crp)
struct inode	*ip;		/* Inode to check access for	*/
int		flag;		/* Open style flags		*/
struct ucred	*crp;
{
	int rc = 0;			/* return code			*/
	int imode = 0;			/* imode			*/
	int type;			/* itype			*/
	struct gnode *gp;		/* gnode pointer from inode	*/
	struct eflock lckdat;
	extern int iwritelockx();
	extern int iwriteunlockx();

	BUGLPR(xopendbg, BUGNTF,("ip_access:ip=%X fl=%X\n", ip, flag));

	gp = ITOGP(ip);		

	type = ip->i_mode & IFMT;

	if (flag & FREAD)
		imode |= IREAD;

	/* only FWRITE and FTRUNC (not FAPPEND) imply writing */
	if (flag & (FWRITE|FTRUNC))
	{	
		if (type == IFDIR)
			return EISDIR;
		imode |= IWRITE;
	}

loop:
	if ((flag & FEXEC)) {
		if (flag & FTRUNC) {
			BUGPR(("ip_access: invalid open mode\n"));
			return(EINVAL);
		}
		if (rc = execaccess(ip, crp))
			return( rc );
	} else if (rc = iaccess(ip, imode, crp))
		return rc;

	/*
	 * for rshare and exec make sure there are no writers
	 */
	if ((flag & (FRSHARE|FEXEC|FNSHARE)) && (gp->gn_wrcnt)) {
		if (rc = open_sleep(ip, flag))
			return(rc);
		else
			goto loop;
	}

	/*
	 * for nshare, there can be no readers or writers
	 */
	if ((flag & FNSHARE) && (gp->gn_rdcnt)) {
		if (rc = open_sleep(ip, flag))
			return(rc);
		else
			goto loop;
	} 

	/*
	 * if we are opening for write rshare count must be zero
	 */
	if ((imode & IWRITE) && (gp->gn_rshcnt)) {
		if (rc = open_sleep(ip, flag))
			return(rc);
		else
			goto loop;
	}

	/*
	 * no open can occur when file is opened with NSHARE
	 */
	if (gp->gn_flags & GNF_NSHARE) {
		if (rc = open_sleep(ip, flag))
			return(rc);
		else
			goto loop;
	}

	if ((flag & FTRUNC) && ENF_LOCK(ip->i_mode))
	{
		lckdat.l_type = F_WRLCK;
		lckdat.l_whence = 0;
		lckdat.l_len = 0;
		lckdat.l_start = 0;
		lckdat.l_pid = U.U_procp->p_pid;
		lckdat.l_sysid = 0;
		lckdat.l_vfs = MNT_JFS;
		rc = common_reclock(gp, ip->i_size, 0, &lckdat, INOFLCK, 0, 0,
				iwritelockx, iwriteunlockx); 
		if (lckdat.l_type != F_UNLCK)
			rc = EAGAIN;
	}

	return rc;
}


/*
 * NAME:	execaccess(ip, crp)
 *
 * FUNCTION:	Check exec access for the inode passed.
 *		
 * PARAMETERS:	Ip	- inode to check permissions for.
 *		crp	- credential
 *
 * RETURNS:	EACCES	- If inode is not for regular file
 *		errors from subroutines
 */

int
execaccess(ip, crp)
struct inode	*ip;
struct ucred	*crp;
{
	int rc;

	if ((ip->i_mode & IFMT) != IFREG)
		return EACCES;

	rc = iaccess(ip, IEXEC, crp);

	return(rc);
}


/*
 * NAME: open_sleep
 *
 * FUNCTION:	check open mode flags to see if process should sleep.  
 *		If O_DELAY is set the open will sleep on count
 *		becomming zero.
 *
 * EXECUTION ENVIRONMENT:
 *	called from ip_access
 *
 * RETURNS:
 *	error code value
 *	0 if successful
 *
 */

int open_sleep(ip, flag)
struct inode *ip;		/* inode being opened		*/
int flag;			/* open flags			*/
{
	int rc;

	BUGLPR(xopendbg, BUGNTA, ("open_sleep:ip=%X fl=%X\n", ip, flag));

	/*
	 * check if we should sleep at all
	 */
	if (! (flag & FDELAY))
		return(ETXTBSY);

	/*
	 * release inode read/write lock and sleep
	 */
#ifdef _I_MULT_RDRS
	rc = e_sleep_thread(&ip->i_openevent, &ip->i_rdwrlock,
				LOCK_WRITE|INTERRUPTIBLE);
#else /* simple lock */
	rc = e_sleep_thread(&ip->i_openevent, &ip->i_rdwrlock,
				LOCK_SIMPLE|INTERRUPTIBLE);
#endif /* _I_MULT_RDRS */

	if (rc == THREAD_INTERRUPTED)
		return(EINTR);

	if (ip->i_nlink < 1)
		return(ENOENT);

	return(0);
}
