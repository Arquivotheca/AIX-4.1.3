static char sccsid[] = "@(#)87	1.27.1.12  src/bos/kernel/pfs/xix_acl.c, syspfs, bos411, 9439C411e 9/30/94 12:59:40";
/*
 * COMPONENT_NAME:  (SYSPFS) Physical File System
 *
 * FUNCTIONS: 	jfs_getacl(), jfs_setacl()
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
#include "sys/errno.h"
#include "sys/syspest.h"
#include "sys/malloc.h"
#include "sys/user.h"

#define DPRINTF(args)

int acl_check(struct acl *acl, int len);
int uiosiz(struct uio *uiop);
static int i_getacl(struct inode *ip, struct uio *uiop);
static int i_setacl(struct inode *ip, struct uio *uiop, struct ucred *crp);

/*
 * NAME:	jfs_getacl(vp, uiop, crp)
 *
 * FUNCTION:	Return ACL of file
 *
 * PARAMETERS:	vp 	_ is the pointer to the vnode that represents the file
 *			  to get the ACL from
 *		uiop	- address of a uio structure in which to put the ACL
 *		crp	- credential
 *
 * RETURN :	0 if successful, errno otherwise
 */

int
jfs_getacl(vp, uiop, crp)
struct vnode	*vp;
struct uio	*uiop;
struct ucred	*crp;		/* pointer to credential structure */
{
/* the inode of interest */
struct inode *ip;
int	rc;

	DPRINTF(("jfs_getacl():\n"));
	ip = VTOIP(vp);
	IWRITE_LOCK(ip);

	if (ip->i_acl & ACL_INCORE)
		/* get acl of incore object: lfs/vfs_access.c */
		rc = ic_getacl(ip->i_acl, ip->i_mode, uiop);
	else
		rc = i_getacl(ip, uiop);

	IWRITE_UNLOCK(ip);
	return(rc);
}


/*
 * NAME:	jfs_setacl(vp, uiop, crp)
 *
 * FUNCTION:	Store ACL of file
 *
 * PARAMETERS:	vp 	_ is the pointer to the vnode that represents the file
 *			  the ACL is for
 *		uiop	- address of a uio structure containing the ACL
 *		crp	- credential
 *
 * RETURN :	0 if successful, errno otherwise
 */

int
jfs_setacl(vp, uiop, crp)
struct vnode	*vp;
struct uio	*uiop;
struct ucred	*crp;		/* pointer to credential structure */
{
	struct inode *ip;
	int	rc;

	DPRINTF(("jfs_setacl():\n"));
	ip = VTOIP(vp);
	IWRITE_LOCK(ip);
	

	/*
	 * Is the user authorized to modify the ACL?  If they
	 * aren't, do they have the required kernel privilege?
	 */
	if ((crp->cr_uid != ip->i_uid) && privcheck_cr(SET_OBJ_DAC, crp))
	{
		rc = EPERM;
		goto out;
	}
		
	/*
	 * Should the ACL be incore or persistent ?
	 * incore objects (unlinked open files and MPX files)
	 * should have their ACLs incore.
	 */
	if ((ip->i_nlink == 0) ||
	    (vp->v_vntype == VMPC) ||
	    (ip->i_acl & ACL_INCORE))
	{
		/* set acl of incore object: lfs/vfs_access.c */
		if ((rc = ic_setacl(&ip->i_acl, &ip->i_mode, ip->i_gid, 
				   uiop, crp)) == 0)
			imark(ip, ICHG);
	}
	else
		rc = i_setacl(ip, uiop, crp);	/* persistent */

out:
	IWRITE_UNLOCK(ip);
	return(rc);
}


/*
 * NAME:	i_getacl(vp, uiop)
 *
 * FUNCTION:	Return ACL of an inode which gets committed;
 *		i.e., the ACL resides in the ".inodex" segment.
 *
 * PARAMETERS:	ip 	- is the pointer to the inode that represents the file
 *			  to get the ACL from
 *		uiop	- address of a uio structure in which to put the ACL
 *
 * RETURN :	0 if successful, errno otherwise
 *	
 * SERIALIZATION: called only by jfs_getacl() 
 *		  with inode locked on entry/exit
 */

i_getacl(struct inode	*ip,
	 struct uio	*uiop)
{
	int	rc;		/* normal junk */
	label_t	jb;		/* jump buf pointer */
	struct	inode	*ixip = NULL;	/* the .inodex segment */
	caddr_t	saddr;
	struct acl	aclhead; 	/* head of acl */
	struct acl	*aclptr;	/* the real acl */

	DPRINTF(("i_getacl():\n"));

	/* build the acl header from the file mode */
	DPRINTF(("jfs_getacl(): fixing mode\n"));
	aclhead.acl_mode = ip->i_mode & ACL_MODE;
	aclhead.u_access = (ip->i_mode >> 6) & 0x7;
	aclhead.g_access = (ip->i_mode >> 3) & 0x7;
	aclhead.o_access = ip->i_mode & 0x7;

	/* if an ACL doesn't exist for this file then just return the header */
	if (ip->i_acl == 0)
	{
		aclptr = NULL;
		aclhead.acl_len = ACL_SIZ;
	}
	else
	{
		ixip = ip->i_ipmnt->i_ipinodex;

		/* get addressibility to the acl */
		if (rc = iptovaddr(ixip, 1, &saddr))
			return rc;
		aclptr = (struct acl *)((int)saddr + (int)(ip->i_acl));

		IWRITE_LOCK(ixip);

		/* set up error handling */
		if (rc = setjmpx(&jb))
			switch (rc)
			{
			    case ENOSPC:
				rc = ENOMEM;
			    case ENOMEM:
			    case EIO:
				goto out;
			    default:
				assert(0);
			}
		aclhead.acl_len = aclptr->acl_len;
	}

	/* if the user's recieve buffer is smaller than the ACL, try
	 * to inform the user of the necessary size and return ENOSPC
	 */
	DPRINTF(("jfs_getacl(): checking size\n"));
	if (uiosiz(uiop) < aclhead.acl_len)
	{
		int	len;

		DPRINTF(("jfs_getacl():  uiosiz=%d, len=%d\n", uiosiz(uiop), 
							     aclhead.acl_len));
		len = aclhead.acl_len;
		uiop->uio_offset = 0;
		uiop->uio_iov->iov_len = uiop->uio_resid = sizeof(len);
		rc = uiomove((caddr_t)&len, (int)sizeof(len), 
				(enum uio_rw)UIO_READ, (struct uio *)uiop);
		if (rc == 0)
			rc = ENOSPC;
		goto out;
	}

	DPRINTF(("jfs_getacl(): copy out head\n"));
	if (rc = uiomove((caddr_t)&aclhead, (int)ACL_SIZ, 
				(enum uio_rw)UIO_READ, (struct uio *)uiop))
		goto out;

	DPRINTF(("jfs_getacl(): copy out extended entries\n"));
	if (aclptr)
		rc = uiomove((caddr_t)(((char *) aclptr) + ACL_SIZ), 
				(int)(aclhead.acl_len - ACL_SIZ), 
				(enum uio_rw)UIO_READ, (struct uio *)uiop);
	DPRINTF (("jfs_getacl(): successful\n"));

out:
	if (ixip)
	{
		IWRITE_UNLOCK(ixip);
		DPRINTF(("jfs_getacl(): unbinding \".inodex\"\n"));
		clrjmpx(&jb);
		ipundo(saddr);
	}

	return(rc);
}


/*
 * NAME:	i_setacl(ip, uiop, crp)
 *
 * FUNCTION:	Store ACL of file which is not to be committed;
 *		i.e., one for which the ACL is not stored in the
 *		".inodex" segment.
 *
 * PARAMETERS:	vp 	_ is the pointer to the vnode that represents the file
 *			  the ACL is for
 *		uiop	- address of a uio structure containing the ACL
 *		crp	- credential
 *
 * RETURN :	any errors that occur  while updating the inode
 *	
 * SERIALIZATION: called only by jfs_setacl() 
 *		  with inode locked on entry/exit
 */

i_setacl(struct inode	*ip,
	 struct uio	*uiop,
	 struct ucred   *crp)
{
	int	rc=0;
	label_t	jb;			/* jump buf pointer */
	caddr_t	saddr;
	struct inode	*ixip = NULL;	/* the inode extension segment */
	struct acl	*newacl;
	ulong	newmode;
	int	len;

	DPRINTF (("i_setacl(): \n"));

	/* get the new ACL length */
	len = uiosiz(uiop);
	if (len < ACL_SIZ)
	{
		DPRINTF(("i_setacl(): len=%d  sizeof acl=%d\n", len, ACL_SIZ));
		DPRINTF(("jfs_setacl(): bad length\n"));
		return(EINVAL);
	}

	/* allocate space for the new acl */
	newacl = (struct acl *)malloc((uint)len);
	if (newacl == NULL)
	{
		DPRINTF(("i_setacl(): malloc failed!!! (len=%d)\n", len));
		return(ENOMEM);
	}

	/* copy the new acl and check it out ... */
	DPRINTF (("jfs_setacl(): copying down and checking it out\n"));
	if ((rc = uiomove((caddr_t)newacl, (int)len, 
					(enum uio_rw)UIO_WRITE, 
					(struct uio *)uiop))  ||
	    (rc = acl_check(newacl, len)))
	{
		DPRINTF(("i_setacl(): copy or check failed!!! (rc=%d)\n", rc));
		free((void *)newacl);
		return(rc);
	}
	
	/* create new mode */
	DPRINTF(("jfs_setacl(): creating mode\n"));
	newmode = ip->i_mode & ~(ACL_MODE | 0777);
	newmode |= newacl->acl_mode;
	newmode |= (newacl->u_access << 6) | 
		   (newacl->g_access << 3) | 
		   (newacl->o_access);

       /*
	* If we don't have SET_OBJ_DAC then 
	* we can only add the sticky bit on directories 
	* (BSD sticky (8)) and
 	* we can only add (or keep) the ISGID bit
	* if we are in the owning group. (This means
	* If we own the file but are not in the owning
	* Group we can't keep the bit)
	*/

	if (privcheck_cr(SET_OBJ_DAC, crp))
	{

		if (!S_ISDIR (ip -> i_mode))
			newmode &= ~ISVTX;

		if (!groupmember_cr(ip->i_gid, crp))
			newmode &= ~ISGID;
	}

	ip->i_mode = newmode;
	imark(ip, ICHG);

	/* free old acl if present
	 */
	if (ip->i_acl)
	{
		ixip = ip->i_ipmnt->i_ipinodex;
		/* map .inodex into vm */
		iptovaddr(ixip, 1, &saddr);

                /* Since we will need a transaction block, gain it before
                 * locking .inodex.  Otherwise we could deadlock due to
                 * blocking on syncwait and having remove/rename block on
                 * .inodex lock.
                 */
		if (u.u_fstid == 0)
			vcs_gettblk(&rc);

		IWRITE_LOCK(ixip);

		if ((rc = setjmpx(&jb)) == 0)
		{
			smap_free(ixip, saddr, ip->i_acl) ;
			clrjmpx(&jb);
		}
		else
			switch (rc)
			{
			    case ENOSPC:
			    case ENOMEM:
			    case EIO:
				break;
			    default:
				assert(0);
			}
		ip->i_acl = 0;
	}
			
	/* allocate and initialize new acl
	 */
	if (newacl->acl_len > ACL_SIZ)
	{
		DPRINTF(("jfs_setacl():  extended acl\n"));
		if (ixip == NULL)
		{
			ixip = ip->i_ipmnt->i_ipinodex;
			/* map .inodex into vm */
			iptovaddr(ixip, 1, &saddr);

			if (u.u_fstid == 0)
				vcs_gettblk(&rc);

			IWRITE_LOCK(ixip);
		}
		if ((rc = setjmpx(&jb)) == 0)
		{
			smap_alloc(ixip, saddr, len, &(ip->i_acl));
			/* get transaction lock for acl */
			vm_gettlock(saddr + ip->i_acl, newacl->acl_len);
			bcopy(newacl, saddr + ip->i_acl, newacl->acl_len);
			clrjmpx(&jb);
		}
		else
			switch (rc)
			{
			    case ENOSPC:
			    case ENOMEM:
			    case EIO:
				break;
			    default:
				assert(0);
			}
	}

	free((void *)newacl);

	if (ixip)
	{
		ipundo(saddr);
		DPRINTF(("jfs_setacl(): committing ixip and ip\n"));
		imark(ixip, IFSYNC);
		commit(2, ixip, ip);
		IWRITE_UNLOCK(ixip);
	}
	else
	{
		DPRINTF(("jfs_setacl(): committing ip\n"));
		commit(1, ip);
	}

	return(0);
}


/*
 * NAME:	acl_check(acl, len)
 *
 * FUNCTION:	perform minimal correctness checking on an ACL:
 *			1) acl->acl_len == len
 *			2) use "valid_acl()" to check sums
 *			3) force off "irrelevant" mode bits
 *
 *  		only called by i_setacl().
 *
 * PARAMETERS:	acl	- a pointer to the access control list to
 *			  be checked
 *		len	- the advertised length of this list
 *
 * RETURN :	0 if successful, otherwise EINVAL or ENOSPC
 */

acl_check(struct acl	*acl,
	  int		len)
{
	DPRINTF(("acl_check():\n"));
	if (smap_alloc_check(len))
	{
		DPRINTF(("acl_check(): len=%d smap_alloc_check failed\n",len));
		return(ENOSPC);
	}

	if (acl->acl_len != len)
	{
		DPRINTF(("acl_check(): lengths don't match\n"));
		return(EINVAL);
	}
	
	if (!valid_acl(acl))
	{
		DPRINTF(("acl_check(): sum of lengths bad\n"));
		return(EINVAL);
	}

	/* strip the mode bits */
	DPRINTF(("acl_check(): successful\n"));
	acl->acl_mode &= ACL_MODE;
	acl->u_access &= 07;
	acl->g_access &= 07;
	acl->o_access &= 07;

	return (0);
}


/*
 * NAME:	uiosiz (uiop)
 *
 * FUNCTION:	Return the total length of the buffers in a uio
 *
 * PARAMETERS:	uiop	- the address of the uio structure
 *
 * RETURN :	the length of the uio structure
 */
uiosiz(struct uio	*uiop)
{
	struct iovec	*iov;
	int	i;
	int	len=0;

	iov = uiop->uio_iov;
	for (i=0; i<uiop->uio_iovcnt; i++)
		len += iov[i].iov_len;

	return(len);
}
