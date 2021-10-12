static char sccsid[] = "@(#)30        1.11.1.8  src/bos/kernel/s/auth/gid.c, syssauth, bos411, 9428A410j 4/25/94 03:36:39";

/*
 *   COMPONENT_NAME: SYSSAUTH
 *
 *   FUNCTIONS: getgidx
 *              getgroups
 *              groupmember
 *		groupmember_cr
 *              setgid
 *              setgidx
 *              setgroups
 *
 *   ORIGINS: 27 83
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *   LEVEL 1,  5 Years Bull Confidential Information
 */

#include "sys/user.h"
#include "sys/priv.h"
#include "sys/id.h"
#include "sys/errno.h"
#include "sys/limits.h"
#include "sys/param.h"
#include "sys/ltypes.h"
#include "sys/audit.h"
#include "crlock.h"

extern  Simple_lock cred_lock;

int groupmember(gid_t gid);
int groupmember_cr(gid_t gid, struct ucred *crp);
int cgsmember(gid_t gid);

/*
 * NAME:	setgidx()
 *
 * PURPOSE:	Entry point for setting current group IDs.
 *
 * PARMS:	which  -- flag indicating single group ID.
 *
 * RETURNS:	requested group ID; -1 on error
 *
 * ALGORITHM:
 *		determine desired group ID and return it
 *		return EINVAL if flag is not effective, real
 *		    or saved ID.
 *
 * POLICY:
 *		The security policy for setting GIDs is:
 *		0). The GID -1 is not a valid GID.
 *		1). The effective GID may be set without having
 *		privilege to either the real, the effective,
 *              or any member of the concurrent groupset.  Otherwise
 *              a user must have the SET_PROC_DAC privilege.  Only
 *		the effective GID is changed.
 *		2). The real and effective GIDs may be set if the
 *		the SET_PROC_DAC privilege is present to any
 *		arbitrary valid value.  Only the real and effective
 *		GIDs are changed.
 *		3). The real, effective, and saved GIDs may be set
 *		if the SET_PROC_DAC privilege is present to any
 *		arbitrary valid value.
 */

int
setgidx(int	which, 
	gid_t	gid)
{
	gid_t rgid, sgid;
	static int svcnum = 0;
	unsigned long	gid_val;

	CRED_LOCK();
	gid_val =  U.U_cred->cr_rgid;
	CRED_UNLOCK();
	if(audit_flag && audit_svcstart("PROC_RealGID", &svcnum, 1, gid_val)){
		audit_svcfinis();
	}

	if (gid == -1)
	{
		u.u_error = EINVAL;
		return(-1);
	}

	/*
	 * SET_PROC_DAC privilege needed 
	 */
	if (which == (ID_SAVED|ID_REAL|ID_EFFECTIVE))
	{
		if (u.u_error = privcheck(SET_PROC_DAC))
		{
			u.u_error = EPERM;
			return(-1);
		}
		CRED_LOCK();
		U.U_cred = _crcopy(U.U_cred);
		U.U_cred->cr_sgid = gid;
		U.U_cred->cr_rgid = gid;
		U.U_cred->cr_gid  = gid;
		CRED_UNLOCK();
		return(0);
	}

	if (which == (ID_REAL|ID_EFFECTIVE))
	{
		if (u.u_error = privcheck(SET_PROC_DAC))
		{
			u.u_error = EPERM;
			return(-1);
		}
		CRED_LOCK();
		U.U_cred = _crcopy(U.U_cred);
		U.U_cred->cr_rgid = gid;
		U.U_cred->cr_gid  = gid;
		CRED_UNLOCK();
		return(0);
	}

	/*
	 * no privilege needed
	 */
	if (which == ID_EFFECTIVE)
	{
		CRED_LOCK();
		rgid = U.U_cred->cr_rgid;
		sgid = U.U_cred->cr_sgid;
		CRED_UNLOCK();
		/*
		 * can change effective only to real, saved
		 * a member of the concurrent group set, or a privileged
		 * user.
		 */
		if ((gid != rgid) && 
		    (gid != sgid) &&
		    (!groupmember(gid)) &&
		    privcheck(SET_PROC_DAC))
		{
			u.u_error = EPERM;
			return(-1);
		}
		CRED_LOCK();
		U.U_cred = _crcopy(U.U_cred);
		U.U_cred->cr_gid = gid;
		CRED_UNLOCK();
		return(0);
	}

	/* if we get this far then which is in error */
	u.u_error = EINVAL;
	return(-1);
}


/*
 * NAME:	setgid()
 *
 * PURPOSE:	Entry point for setting current effective, and
 *		possibly real and saved as well, group ID.
 *
 * PARMS:	gid  --  value of new group ID
 *
 * RETURNS:	zero on success; -1 on privilege failure
 *
 * ALGORITHM:
 *		if user lacks SET_PROC_DAC set only the effective
 *		    group ID
 *		otherwise set all three group IDs.
 *		call setgidx() to do the actual work.
 *		return result of operation from setgidx.
 */

int
setgid(gid_t	gid)
{
	/*
	 * if the invoker does not have SET_PROC_DAC privilege 
	 * then set only the effective gid
	 */
	if (privcheck(SET_PROC_DAC))
		return(setgidx(ID_EFFECTIVE, gid));

	/* 
	 * The invoker has SET_PROC_DAC privilege so the
	 * saved, real and effective gids will be set.
	 */
	return(setgidx(ID_SAVED|ID_REAL|ID_EFFECTIVE, gid));
}


/*
 * NAME:	getgidx()
 *
 * PURPOSE:	Entry point for getting current group IDs.
 *
 * PARMS:	which  -- flag indicating single group ID.
 *
 * RETURNS:	requested group ID; -1 on error
 *
 * ALGORITHM:
 *		determine desired group ID and return it
 *		return EINVAL if flag is not effective, real
 *		    or saved ID.
 */

gid_t
getgidx(int	which)
{
	gid_t 	rval;
	int tlock;

	if (tlock = (U.U_procp->p_active > 1)) CRED_LOCK();
	/*
	 * No privilege required.  Find single ID which user is
	 * requesting and return it.  Return EINVAL for illegal
	 * requests.
	 */

	switch (which)
	{
		case ID_EFFECTIVE:
			rval = U.U_cred->cr_gid;
			break;
		case ID_REAL:
			rval = U.U_cred->cr_rgid;
			break;
		case ID_SAVED:
			rval = U.U_cred->cr_sgid;
			break;
		default:
			if (tlock) CRED_UNLOCK();
			u.u_error = EINVAL;
			return(-1);
	}
	if (tlock) CRED_UNLOCK();
	return(rval);
}

/*
 * NAME:	getgroups()
 *
 * PURPOSE:	Entry point for getting the current list
 *		of group memberships
 *
 * PARMS:	ngroups  --  number of elements in user's buffer
 *		gidset  --  user's buffer
 *
 * RETURNS:	number of groups if no error; -1 on error
 *
 * ALGORITHM:
 *		validate size of user's buffer
 *		    zero group size returns size of list
 *		    too small a buffer returns error
 * 		copy groupset into user buffer
 *		return value to user is number of groups or -1 on error
 */

int
getgroups(int	ngroups, 
	  gid_t	*gidset)
{
	register gid_t	*current_gidset;
	register int	current_ngroups;
	int tlock;

	if (tlock = (U.U_procp->p_active > 1)) CRED_LOCK();

	current_gidset = U.U_cred->cr_groups;
	current_ngroups = U.U_cred->cr_ngrps;


	/*
	 * Validate the size of the user's buffer.  If the size is zero,
	 * report the number of groups.  If the buffer is too small,
	 * return an error.
	 */

	if (ngroups == 0)
	{
		if (tlock) CRED_UNLOCK();
		return(current_ngroups);
	}

	if (ngroups < current_ngroups)
	{
		if (tlock) CRED_UNLOCK();
		u.u_error = EINVAL;
		return(-1);
	}

	/*
	 * Copy groupset to user's buffer.
	 */

	if (copyout(current_gidset, gidset, current_ngroups * sizeof(gid_t)))
	{
		if (tlock) CRED_UNLOCK();
		u.u_error = EFAULT;
		return(-1);
	}

	if (tlock) CRED_UNLOCK();

	/*
	 * Return number of groups in groupset.
	 */

	return(current_ngroups);
}


/*
 * NAME:	setgroups()
 *
 * PURPOSE:	Entry point for setting the current list
 *		of group memberships
 *
 * PARMS:	ngroups  --  number of elements in users buffer
 *		gidset  -- user's buffer
 *
 * RETURNS:	0 if no error; -1 if error
 *
 * ALGORITHM:
 *		validate new groupset size
 *		get a duplicated cred structure
 *		copy groups into the new cred structure
 *		the process must have SET_PROC_DAC, or all of the elements
 *		    in new groupset must be in old groupset
 *		free the old cred structure replacing with validated structure
 *
 *		return value to the user is 0 or -1 on errors
 */

int
setgroups(int	ngroups, 
	  gid_t	*gidset)
{
	register gid_t	*gp, *new_gidset;
	struct	ucred	*newcr, *tmpcr;
	extern	struct	ucred	*_crdup();
	int	notsubset;

	/*
	 * Test that the new groupset is small enough to fit inside
	 * of the cred structure.
	 */

	if (ngroups < 0 || ngroups > NGROUPS_MAX)
	{
		u.u_error = EINVAL;
		return(-1);
	}

	CRED_LOCK();
	/*
	 * Allocate a new cred structure to copy the new groupset
	 * into.  Set the groupset size in the cred structure to be the
	 * size of the user's new groupset.
	 */

	newcr = _crdup(U.U_cred);
	CRED_UNLOCK();
	new_gidset = newcr->cr_groups;
	if (copyin(gidset, new_gidset, ngroups * sizeof(gid_t)))
	{
		u.u_error = EFAULT;
		goto out;
	}
	newcr->cr_ngrps = ngroups;

	/*
	 * Enforce the security policy for setgroups():
	 *
	 * The new groupset must be a subset of the old groupset
	 * or the process must have SET_PROC_DAC privilege.  
	 * GID -1 is not allowed and causes EINVAL
	 *
	 * Any or all groups may be omitted.  The current effective
	 * group ID is a member of the concurrent groupset.
	 */

	notsubset = 0;
	for (gp = new_gidset; gp < &new_gidset[ngroups]; gp++)
	{
		if (*gp == -1) 
		{
			u.u_error = EINVAL;
			goto out;
		}
		if (! groupmember (*gp))
			notsubset = 1;
	}

	if (notsubset && privcheck(SET_PROC_DAC))
	{
		u.u_error = EPERM;
		goto out;
	}

	/*
	 * Replace the old cred structure with the new structure and
	 * free the old one.
	 */

	CRED_LOCK();
	tmpcr = U.U_cred;
	U.U_cred = newcr;
	_crfree(tmpcr);
	CRED_UNLOCK();
	return(0);

	/*
	 * Handle all errors after the new cred structure was allocated.
	 * Free the cred structure and return -1.
	 */

out:	crfree(newcr);
	return(-1);
}


/*
 * Check if gid is a member of the group set or the current
 * effective GID.
 */

int
groupmember(gid_t	gid)
{
	register gid_t *gp;
	register int ngrps;
	int	rc;
	int tlock;

	if (tlock = (U.U_procp->p_active > 1)) CRED_LOCK();
	rc = groupmember_cr(gid, U.U_cred);
	if (tlock) CRED_UNLOCK();
	return rc;
}

/*
 * Common groupmember routine. This routine excepts a gid and a
 * pointer to a credential struct.
 */

int
groupmember_cr(gid_t gid, struct ucred *crp)
{
	register gid_t *gp;
	register int ngrps;

	if (crp->cr_gid == gid)
		return 1;

	ngrps = crp->cr_ngrps;
	gp = crp->cr_groups;
	
	/*
	 * Must still hold the CRED_LOCK as gp points at cr_groups array
	 */
	while (ngrps--)
		if (*gp++ == gid)
			return 1;

	return 0;
}
