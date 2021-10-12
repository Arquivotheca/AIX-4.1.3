static char sccsid[] = "@(#)35	1.4.1.4  src/bos/kernel/lfs/vfs_access.c, syslfs, bos411, 9428A410j 4/19/94 16:15:45";
/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: dochown, dochmod, doutime,
 *	      ic_getacl, ic_setacl
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "sys/user.h"
#include "sys/errno.h"
#include "sys/var.h"
#include "sys/vnode.h"
#include "sys/syspest.h"
#include "sys/vattr.h"
#include "sys/chownx.h"
#include "sys/flock.h"
#include "sys/priv.h"
#include "sys/id.h"
#include "sys/uio.h"
#include "sys/acl.h"
#include "sys/malloc.h"

#define DPRINTF(x)

/* 
 * The serialization model for these routines imply that the caller
 * is holding a lock on the object which is being modified. (i.e.
 * holding the specnode_lock if modifying a special files attributes)
 */

int
dochmod (
	uid_t		uid,		/* uid of object to chmod	*/
	gid_t		gid,		/* gid of object to chmod	*/
	int		newmode,	/* new mode for object		*/
	mode_t *	mode,		/* ptr to actual mode of object	*/
	struct ucred *	crp)		/* cred pointer 		*/ 
{
	int		mine;		/* user is owner of object	*/ 
	int		privileged;	/* user has appropriate privs	*/

	mine = (crp->cr_uid == uid);
	privileged = privcheck_cr(SET_OBJ_DAC, crp) ? FALSE : TRUE;

	if (!mine && !privileged)
		return EPERM;

	/* We disallow certain changes for non-privileged users.	*/
	if (!privileged)
	{
		/* We need SET_OBJ_DAC privilege to make sticky text
		 * unless it is a directory (BSD sticky (8)).
		 */
		if (!S_ISDIR (*mode)) 
			newmode &= ~ISVTX;

		/* We need to check the gid because of BSD gid inheritance.
		 * The gid under BSD is inherited from the directory.
		 */
		if (!groupmember_cr(gid, crp))
			newmode &= ~ISGID;
	}

	/* Preserve the high order mode bits (except for S_IXACL) and
	 * set low order mode bits.
	 */
	*mode &= ~(07777 | S_IXACL);
	*mode |= newmode & 07777;

	return 0;
}

int
dochown (
	int		tflag,		/* uid, gid translate flags	*/
	uid_t		newuid,		/* new uid			*/
	gid_t		newgid,		/* new gid			*/
	uid_t *		uid,		/* ptr to actual uid to change	*/
	gid_t *		gid,		/* ptr to actual gid to change	*/
	mode_t *	mode,		/* ptr to actual mode to change	*/
	struct ucred *	crp)		/* cred pointer 		*/ 
{
	int		mine;		/* user is owner of object	*/ 
	int		privileged;	/* user has appropriate privs	*/
	uid_t		nuid;		/* new user id to establish	*/
	gid_t		ngid;		/* new group id to establish	*/

	mine = (crp->cr_uid == *uid);
	privileged = !privcheck_cr(SET_OBJ_DAC, crp);

	if (!mine && !privileged)
		return EPERM;

	nuid = *uid;
	ngid = *gid;

	if (!(tflag & T_OWNER_AS_IS) && newuid != *uid)
	{
		if (!privileged)
			return EPERM;

		nuid = newuid;
	}

	if (!(tflag & T_GROUP_AS_IS) && newgid != *gid)
	{
		if (!privileged && !groupmember_cr(newgid, crp))
			return EPERM;

		ngid = newgid;
	}

	/*
	 * For POSIX test suite, if not privileged process, then
	 * clear the setuid and setgid bits.
	 */
	if (!privileged)
		*mode &= ~(ISUID|ISGID);

	/* Now that all checking is done, set values. */
	*uid = nuid;
	*gid = ngid;

	return 0;
}

int
doutime(
	uid_t		uid,		/* user id of object to change	*/
	int		timeflg,	/* type of time change desired	*/
	struct timestruc_t *newatime,	/* new access time		*/
	struct timestruc_t *newmtime,	/* new modification time	*/
	struct timestruc_t *atime,	/* ptr to actual atime of obj	*/
	struct timestruc_t *mtime,	/* ptr to actual mtime of obj	*/
	struct ucred *	crp)		/* cred pointer 		*/ 
{
	int		mine;		/* user is owner of object	*/ 
	int		privileged;	/* user has appropriate privs	*/
	struct timestruc_t t;

	mine = (crp->cr_uid == uid);
	privileged = !privcheck_cr(SET_OBJ_STAT, crp);

	if (timeflg & T_SETTIME)
	{
		/* The caller is responsible for all checking. The caller
		 * didn't supply the new times so we use the current time.
		 */
		curtime(&t);
		*atime = t;
		*mtime = t;
	}
	else
	{
		if (!mine && !privileged)
			return EPERM;

		if (newatime)
			*atime = *newatime;
		if (newmtime)
			*mtime = *newmtime;
	}

	return 0;
}

/*
 * NAME:	ic_getacl(acl, mode, uiop)
 *
 * FUNCTION:	Return ACL of in-core object.
 *		This should be called both for multiplexed device
 *		inodes and for unlinked open files.
 *		the caller is responsible for locking and unlocking
 *		any data structures containing the acl.
 *
 * PARAMETERS:	acl	- in-core ACL of object
 *		mode	- mode associated with object
 *		uiop	- address of a uio structure in which to put the ACL
 *
 * RETURN :	0 if successful, errno otherwise
 */

ic_getacl(struct acl	*acl,
	 mode_t		mode,
	 struct uio	*uiop)
{
	/* the kernel's copy of the acl */
	struct	acl	*aclptr;
	int	acllen;
	int	rc;

	DPRINTF(("ic_getacl():\n"));

	/*
	 * We must be holding a lock on the underlying object.
	 * i.e. the specnode node if we are getting the attributes
	 * of a specnode.
	 */

	/*
	 * if an ACL doesn't exist for this file allocate a base.  The
	 * base portion is filled in from information in the mode
	 * bits regardless...
	 *
	 * Note: The ACL_INCORE bit needs to be stripped off first...
	 */
	aclptr = (struct acl *)((long)acl & ~ACL_INCORE);
	if (!aclptr)
	{
		static	struct acl	aclbuf;

		DPRINTF(("ic_getacl():  aclptr == NULL\n"));
		aclptr = &aclbuf;
		aclptr->acl_len = ACL_SIZ;
	}
	else
	{
		DPRINTF(("ic_getacl():  aclptr == 0x%x\n", (long)acl));
	}

	/* patch it up, with respect to the file mode */
	DPRINTF(("ic_getacl(): fixing mode\n"));
	aclptr->acl_mode = mode;
	aclptr->u_access = (mode >> 6) & 0x7;
	aclptr->g_access = (mode >> 3) & 0x7;
	aclptr->o_access = mode & 0x7;
	acllen = aclptr->acl_len;

	/*
	 * if the user's recieve buffer is smaller than the ACL, try
	 * to inform the user of the necessary size and return ENOSPC
	 */
	if (uiosiz(uiop) < acllen)
	{
		DPRINTF(("ic_getacl():  uiosiz=%d, len=%d\n", uiosiz(uiop), 
							      acllen));
		uiop->uio_offset = 0;
		uiop->uio_iov->iov_len = uiop->uio_resid = sizeof(acllen);
		rc = uiomove((caddr_t)&acllen, (int)sizeof(acllen), 
					(enum uio_rw)UIO_READ, 
					(struct uio *)uiop);
		if (rc == 0)
			rc = ENOSPC;
		goto out;
	}

	DPRINTF(("ic_getacl(): copy out %d\n", acllen));
	uiop->uio_offset = 0;
	uiop->uio_resid = acllen;
	rc = uiomove((caddr_t)aclptr, (int)acllen, (enum uio_rw)UIO_READ, 
				(struct uio *)uiop);

	DPRINTF(("ic_getacl(): successful\n"));
out:
	return(rc);
}


/*
 * NAME:	ic_setacl(acl, mode, gid, uiop, crp)
 *
 * FUNCTION:	Set ACL of in-core object.
 *		This should be called both for multiplexed device
 *		inodes and for unlinked open files.
 *		The caller is responsible for locking and unlocking
 *		the any data structures containing the ACL.
 *
 * PARAMETERS:	acl	- pointer to in-core ACL to set
 *		mode	- pointer to mode of in-core object
 *		gid	- group id of object
 *		uiop	- address of a uio structure from which to get the ACL
 *		crp	- cred pointer
 *
 * RETURN :	0 if successful, errno otherwise
 */

ic_setacl(struct acl	**acl,
	  mode_t	*mode,
	  gid_t		gid,
	  struct uio	*uiop,
	  struct ucred  *crp)
{
	/* the new protection */
	struct	acl	*newacl = NULL;
	/* for error conditions: indicates newacl was allocated */
	int	newalloc = 0;
	ulong	newmode;
	int	len;
	int	rc=0;
	struct acl	aclbuf;

	DPRINTF(("ic_setacl():\n"));

	/* 
	 * We must be holding the lock on the object we are trying to 
	 * modify at this point
	 */

	/* get the new ACL length */
	len = uiosiz(uiop);
	if (len < ACL_SIZ)
	{
		DPRINTF(("ic_setacl(): bad length\n"));
		rc = EINVAL;
		goto fail;
	}

	/* allocate space for the new acl */
	if (len == ACL_SIZ)
	{
		DPRINTF(("ic_setacl(): mode bits only\n"));
		newacl = &aclbuf;
	}
	else
	{
		/* allocate the new acl */
		DPRINTF(("ic_setacl(): allocating new acl\n"));
		if ((newacl = (struct acl *)malloc((uint)len)) == NULL)
			goto fail;
		newalloc = 1;
	}

	/* copy the new acl down */
	DPRINTF(("ic_setacl(): copying down\n"));
	if (rc = uiomove((caddr_t)newacl, (int)len, 
				(enum uio_rw)UIO_WRITE, (struct uio *)uiop))
		goto fail;
	/* and check it out ... */
	if (rc = acl_check(newacl, len))
		goto fail;

	/* create new mode */
	DPRINTF(("ic_setacl(): creating mode\n"));
	newmode = *mode & ~(ACL_MODE | 0777);
	newmode |= newacl->acl_mode & ACL_MODE;
	newmode |= ((newacl->u_access & 07) << 6) | 
		   ((newacl->g_access & 07) << 3) | 
		   ((newacl->o_access & 07));

       /*
	* If we don't have SET_OBJ_DAC then we can only 	
	* add the sticky bit on directories
	* (BSD sticky (8))
 	* and we can only add (or keep) the ISGID bit
	* if we are in the owning group. (This means
	* If we own the file but are not in the owning
	* Group we can't keep the bit 
	*/

	if (privcheck_cr(SET_OBJ_DAC, crp))
	{
		if (!S_ISDIR (*mode))
			newmode &= ~ISVTX;


		if (!groupmember_cr(gid, crp))
			newmode &= ~ISGID;
	}
		

	/* if an ACL is already present, free it */
	{
		struct	acl	*oldacl;

		oldacl = (struct acl *)((long)*acl & ~ACL_INCORE);
		if (oldacl)
		{
			DPRINTF(("ic_setacl():  (free) extended acl\n"));
			free((void *)oldacl);
		}
	}

	/* establish the new protection */
	DPRINTF(("ic_setacl(): setting new protection\n"));
	*mode = newmode;
	if (len <= ACL_SIZ)
		*acl = (struct acl *)ACL_INCORE;
	else
		*acl = (struct acl *)(ACL_INCORE | ((int)newacl));
	return(rc);

fail:
	if (newalloc)
	{
		DPRINTF(("ic_setacl(): freeing newacl\n"));
		free((void *)newacl);
	}
	return(rc);
}
