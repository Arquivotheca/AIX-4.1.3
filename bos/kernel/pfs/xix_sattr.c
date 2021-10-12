static char sccsid[] = "@(#)32	1.37.1.5  src/bos/kernel/pfs/xix_sattr.c, syspfs, bos411, 9428A410j 7/7/94 16:54:51";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: jfs_setattr
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

#include "sys/errno.h"
#include "jfs/jfslock.h"
#include "jfs/inode.h"
#include "sys/vattr.h"

extern time_t time;

/*
 * NAME:	jfs_setattr(vp, cmd, arg1, arg2, arg3, crp)
 *
 * FUNCTION:	This is a common routine to update the owner,
 *		group, or time in the file's inode based on the
 *		information in the svattr structure
 *
 * PARAMETERS:	Vp	- vnode for the inode to change.
 *		Cmd	- an integer that indicates what portion of 
 *			  the attributes we wish to set. It also indicates
 *			  what the arguments will mean.
 *		Arg1	-  various arguments, cmd determines their meaning.
 *		arg2
 *		arg3
 *		crp	- credential
 *
 * RETURNS:	The owner, group, or time is updated in the inode and
 *		any error codes that occur are returned.
 */

jfs_setattr(vp, cmd, arg1, arg2, arg3, crp)
struct vnode	*vp;
int		cmd;
int		arg1, arg2, arg3;
struct ucred	*crp;		/* pointer to credential structure */
{
	int 	rc = 0;			/* Return code			*/
	struct	inode *ip;		/* Inode for vp			*/
	int	newuid;			/* new uid for file chown	*/
	int	newgid;			/* new gid for file chown	*/
	int	newmode;		/* new mode for file chown	*/
	int	flag = 0;		/* flags for inode update	*/
	
	ip = VTOIP(vp);
	IWRITE_LOCK(ip);

	switch (cmd) {
		case V_MODE:
			rc = dochmod(ip->i_uid,
				     ip->i_gid,
				     arg1,		/* new mode */
				     &ip->i_mode,
				     crp);
			if (rc == 0)
				imark(ip, ICHG);
			break;

		case V_OWN:
			newuid = ip->i_uid;
			newgid = ip->i_gid;
			newmode = ip->i_mode;
			rc = dochown(arg1,		/* flags	*/
				     arg2,		/* new uid	*/
				     arg3,		/* new gid	*/
				     &newuid,
				     &newgid,
				     &newmode,
				     crp);

			/* check for quota violation */
			if (rc == 0 && ((rc = chowndq(ip,newuid,newgid,crp)) == 0)) {
				ip->i_uid = newuid;
				ip->i_gid = newgid;
				ip->i_mode = newmode;
				imark(ip, ICHG);
			}
			break;

		case V_UTIME:
			/* do T_SETTIME checking here */
			if (arg1 & T_SETTIME &&
					(crp->cr_uid != ip->i_uid) &&
					privcheck_cr(SET_OBJ_STAT,crp) &&
					iaccess(ip, IWRITE, crp))
			{
				rc = EACCES;
				break;
			}

			rc = doutime(ip->i_uid,
				     arg1,		/* flags	*/
				     arg2,		/* new atime	*/
				     arg3,		/* new mtime	*/
				     &ip->i_atime_ts,
				     &ip->i_mtime_ts,
				     crp);			     
			if (rc == 0)
				imark(ip, ICHG);
			break;

		case V_STIME:
			/* We don't need to check privilege for setting time for
			 * the specfs.
			 */
			if (arg1 > 0)
                                flag |= IACC;
                        if (arg2 > 0)
                                flag |= IUPD;
                        if (arg3 > 0)
                                flag |= ICHG;

                        /* mark inode times as changed, don't commit */
                        imark(ip, flag);

			if (arg1 > 0)
				ip->i_atime_ts = *(struct timestruc_t *)arg1;
			if (arg2 > 0)
				ip->i_mtime_ts = *(struct timestruc_t *)arg2;
			if (arg3 > 0)
				ip->i_ctime_ts = *(struct timestruc_t *)arg3;
			rc = 0;
			break;

		default:
			rc = EINVAL;
			break;
	}

	/* commit changes of inode */
	if (rc == 0 && cmd != V_STIME && (ip->i_flag & (IUPD|ICHG|IACC)))
		commit(1, ip);

	IWRITE_UNLOCK(ip);
	return rc;
}
