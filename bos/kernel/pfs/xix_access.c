static char sccsid[] = "@(#)04	1.21.1.7  src/bos/kernel/pfs/xix_access.c, syspfs, bos411, 9428A410j 7/7/94 16:53:06";
/*
 * COMPONENT_NAME:  (SYSPFS) Physical File System
 *
 * FUNCTIONS:  jfs_access(), iaccess()
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
#include "jfs/inode.h"
#include "sys/errno.h"

#define	DPRINTF(args)

void	self_access();

/*
 * if this is changed (esp., to re-interpret specificity
 * of mode bits if acl is enabled but not present), some
 * of the routines below must change (esp., to check for
 * NULL acl)
 */
#define	USE_MODE(ip,mode) \
	( \
		(((mode) & S_IXACL) == 0) || \
		(((ip)->i_acl & ~ACL_INCORE) == 0) || \
		acl_mapin(ip, &acl, &saddr) \
	)

/*
 * make an access control list entry.
 * this is used to build pseudo entries for the file owner and group.
 */

static
make_ace (ace, acc_type, acc, id_type, id_data)
struct	acl_entry	*ace;
int	acc_type;
int	acc;
int	id_type;
int	id_data;
{
	ace->ace_len = sizeof(struct acl_entry);
	ace->ace_type = acc_type;
	ace->ace_access = acc;
	ace->ace_id[0].id_type = id_type;
	/* this ASSUMES the data is an integer */
	ace->ace_id[0].id_len = sizeof(struct ace_id);
	ace->ace_id[0].id_data[0] = id_data;
}


/*
 * NAME:	jfs_access (vp, mode, who, crp)
 *
 * FUNCTION:	Check rwx permissions:  If file has an ACL and I_SACL
 *		is on then use the ACL, otherwise use the mode bits.
 *
 * PARAMETERS:	vp	- vnode to check permissions on
 *		mode	- mode to check for ie rwx
 *		who	- one of:	ACC_SELF 
 *				 	ACC_OTHERS 
 *				 	ACC_ANY 
 *				 	ACC_ALL
 *		crp	- credential
 *
 * RETURNS:	0      - success
 *		EINVAL - invalid mode or who argument
 */

jfs_access(vp, mode, who, crp)
struct vnode	*vp;		/* Source vnode	*/
int		mode;		/* Mode(s) to check */
int		who;
struct ucred	*crp;		/* pointer to credential structure */
{
	int rc;				/* Return code	*/
	struct inode	  *ip;		/* Inode for vp	*/
	struct acl_entry  *acep;	/* roving pointer to ACL entries */
	struct acl_entry  *acl_end;	/* end of extended ACL */

	/*
	 * check the validity of "mode".
	 */
	if (mode & ~0x7)
		return(EINVAL);

	ip = VTOIP(vp);
	IREAD_LOCK(ip);

	switch (who)
	{
	    case ACC_SELF:
		DPRINTF (("jfs_access(SELF,%d)\n",mode));
		rc = iaccess(ip, mode<<6, crp);
		break;

	    case ACC_OTHERS:
		DPRINTF (("jfs_access(OTHERS,%d)\n",mode));
		rc = check_any(ip, mode, ACC_OTHERS);
		break;

	    case ACC_ANY:
		DPRINTF (("jfs_access(ANY,%d)\n",mode));
		rc = check_any(ip, mode, ACC_ANY);
		break;

	    case ACC_ALL:
		DPRINTF (("jfs_access(ALL)\n",mode));
		rc = check_all(ip, mode);
		break;

	    default:
		rc = EINVAL;
	}

	IREAD_UNLOCK(ip);
	return(rc);
}
  

/*
 * NAME:	iaccess(ip, mode, crp)
 *
 * FUNCTION:	Check mode permission on inode. Mode is READ, WRITE or EXEC.
 * 		The mode is shifted to select the owner/group/other fields.
 *		
 *		DAC permissions are checked first.  Permissions not granted
 *		are then checked for their corresponding  BYPASS_DAC_* 
 *		privilege.
 *
 *		If an ACL is present and the file's mode word has S_IXACL 
 *		on then the ACL is used for permission checking, otherwise
 *		the mode bits are used.  
 *
 * PARAMETERS:	ip	- inode to check
 *		mode	- mode to check(rwx)
 *
 * RETURN :	EACCES	- if permission requested is denied
 *
 * SERIALIZATION: read/write inode lock held on entry/exit.
 *			
 */

iaccess(ip, m, crp)
struct inode	*ip;
int		m;
struct ucred	*crp;
{
	int rc;
	struct acl	*acl;	/* a pointer to the (possibly mapped) acl */
	char		*saddr;	/* start of mapped inode */
	int	perm = 0;	/* sum of permissive modes */
	int	restr = 0;	/* sum of resticted modes */
	int	found = 0;	/* did any entry match? */
	int	mng;	 	/* modes not granted by DAC */
	uid_t	uid;

	/* right justify! */
	m >>= 6;

	/* existence check */
	if (m == 0)
		return(0);

	if (m & W_ACC)
	{	
		if (ip->i_ipmnt->i_iplog == NULL)
			return EROFS;

		if (ITOGP(ip)->gn_excnt)
			return ETXTBSY;
	}

	/*
	 * "optimize" for mode-bits only case.
	 * pretty much no matter what policy is implemented,
	 * we MUST preserve UNIX semantics when there is no ACL.
	 * and we expect this to be the most common case,
	 * so it's handled before the ACL traversal.
	 */
	uid = crp->cr_uid;
	if (USE_MODE(ip, ip->i_mode))
	{
		DPRINTF (("iaccess(): mode bits only\n"));
		if (uid == ip->i_uid)
		{
			perm = (ip->i_mode >> 6) & 07;
			/* this is also a restriction! */
			restr = ~perm & 07;
			goto chkaccess;
		}
		if (groupmember_cr(ip->i_gid, crp))
		{
			perm = (ip->i_mode >> 3) & 07;
			goto chkaccess;
		}

		/* others */
		perm = ip->i_mode & 07;
		restr = ~perm & 07;
		goto chkaccess;
	}

	DPRINTF (("iaccess(): trying acl\n"));
	/*
	 * the ACL is present, enabled and mapped.
	 * check for any applicable extended entries
	 *
	 *	owner entry is always SPECIFIC
	 *	group entry is always PERMISSIVE
	 *	default entry is applied only if nothing
	 *		else is applicable
	 * this allows emulation of ternary access checks
	 * (which are very much in vogue)
	 */

	if (uid == ip->i_uid)
	{
		perm = ((ip->i_mode >> 6) & 07);
		/* this is also a restriction! */
		restr = ~perm & 07;
		found = 1;
	}
	else 
		if (groupmember_cr(ip->i_gid, crp))
		{
			perm = ((ip->i_mode >> 3) & 07);
			found = 1;
		}

	self_access(acl->acl_ext, acl_last(acl), m, &perm, &restr, &found, crp);

	if (!found)
	{
		DPRINTF (("iaccess(): self_access() no entries\n"));
		perm = ip->i_mode & 07;
		restr = ~perm & 07;
	}
	
	acl_mapout(ip, saddr);

chkaccess:
	mng = (m & ~(perm & ~restr));

	if (mng == 0)
		return(0);
	
	if (privcheck_cr(BYPASS_DAC, crp) == 0)
		return(0);

	if (mng & R_ACC)
		if (privcheck_cr(BYPASS_DAC_READ, crp))
			return(EACCES);
		
	if (mng & W_ACC)
		if (privcheck_cr(BYPASS_DAC_WRITE, crp))
			return(EACCES);
		
	if (mng & X_ACC)
		if (privcheck_cr(BYPASS_DAC_EXEC, crp))
			return(EACCES);
	
	return(0);
}


/*
 * NAME:	self_access(first, last, m, p, d, crp)
 *
 * FUNCTION:	Run through ACL entries checking for requested access.
 *		Generally, in traversing an ACL, we need to sum the
 *		permissions and find any restriction.
 *
 * PARAMETERS:
 *		first	- the first acl entry
 *		last	- the last acl entry
 *		m	- the access mode requested
 *		p	- permitted access
 *		d	- boolean indicating applicable entry found
 *		crp	- credential
 *
 * RETURN :	0     - access allowed
 *		EACCES - access denied
 */
static void
self_access(first, last, m, p, r, f, crp)
struct	acl_entry *first;
struct	acl_entry *last;
int	m;
int	*p;	/* permissives */
int	*r;	/* restrictions */
int	*f;	/* found flag */
struct ucred	*crp;
{
	/* a pointer to entries in the acl */
	struct acl_entry	*ace;

	/* process the acl entries */
	for (ace = first; ace < last; ace = acl_nxt(ace))
	{
		int	access;

		/*
		 * returns non-zero if the current entry is applicable.
		 */
		if (!self_match(ace, crp))
			continue;
		*f = 1;
		access = ace->ace_access & 07;
		switch (ace->ace_type)
		{
			case ACC_PERMIT:
				*p |= access;
				break;
			case ACC_DENY:
				*r |= access;
				break;
			case ACC_SPECIFY:
				*p |= access;
				*r |= ~access & 07;
				continue;
		}
	}
	return;
}


static
self_match(acep, crp)
struct acl_entry	*acep;
struct ucred	*crp;
{
	struct ace_id	*idp;
	struct ace_id	*id_end;
	
	id_end = id_last(acep);
	for (idp = acep->ace_id; idp < id_end; idp = id_nxt(idp))
	{
		switch (idp->id_type)
		{
		    case ACEID_USER:
			if (*(uid_t *)idp->id_data != crp->cr_uid)
				return(0);
			break;

		    case ACEID_GROUP:
			if (!groupmember_cr(*(gid_t *)idp->id_data, crp))
				return(0);
			break;

		    default:
			return(0);
		}
	}
	return(1);
}


static
check_all(ip, m)
struct inode	*ip;
int		m;
{
	struct acl	*acl;		/* the mapped acl */
	char		*saddr;		/* the start of the mapped inode */
	struct acl_entry *acl_end;
	struct	acl_entry *ace;
	struct	acl_entry fowner;	/* pseudo-entries for owner & group */
	struct	acl_entry fgroup;
	int	perm;			/* permissions granted */

	DPRINTF (("check_all()\n"));

	/* the access to be checked must be exactly one of R/W/X */
	switch (m)
	{
		case R_ACC:
		case W_ACC:
		case X_ACC:
			break;
		default:
			return(EINVAL);
	}

	/*
	 * "optimize" for mode-bits only case.
	 * pretty much no matter what policy is implemented,
	 * we MUST preserve UNIX semantics when there is no ACL.
	 * and we expect this to be the most common case,
	 * so it's handled before the ACL traversal.
	 */
	if (USE_MODE(ip, ip->i_mode))
	{
		if (
			/* the access is denied the owner */
			(!((ip->i_mode >> 6) & m)) ||
			/* the access is denied the group */
			(!((ip->i_mode >> 3) & m)) ||
			/* the access is not granted by default */
			(!(ip->i_mode & m))
		   )
		{
			DPRINTF (("check_all(): access denied (mode)\n"));
			return(EACCES);
		}
		DPRINTF (("check_all(): access granted (mode)\n"));
		return(0);
	}

	/*
	 * in order to emulate ternary access checks,
	 * the default entry can be applied only if
	 * no other entries are applicable.
	 * this means, e.g., permissive group entries can be
	 * restrictions (if (access(group) < access(default))).
	 *
	 * annoying cases are:
	 * 1)	if there is any PERMISSIVE entry that does
	 *	not grant the requested access, we must
	 *	look for an equivalent (superset) ACL entry
	 *	that grants the access.  the file owner and
	 *	file group must also be considered.  only then
	 *	have we have computed all the access permitted
	 *	a process affected by the "restrictive" PERMISSIVE
	 *	entry.
	 * 2)	whenever we find a PERMISSIVE entry which grants
	 *	the access, we need to look for a RESTRICTIVE (or
	 *	SPECIFIC) entry that denies the requested access.
	 */

	/* fail if the access is denied the owner */
	if (!((ip->i_mode >> 6) & m))
	{
		DPRINTF (("check_all(): access denied to owner\n"));
		goto	noaccess;
	}
	/* fail if the access is not granted by default */
	if (!(ip->i_mode & m))
	{
		DPRINTF (("check_all(): access denied by default\n"));
		goto	noaccess;
	}

	/*
	 * first pass through the ACL.
	 * look for any SPECIFIC or RESTRICTIVE entry
	 * which denies the requested access.
	 * if one is found, the check fails.
	 * 
	 * while we're at it, look for a permissive entry
	 * which does NOT grant the access (including the group bits).
	 * if none is found (and we don't find a restriction),
	 * the total check succeeds.
	 */
	perm = ((ip->i_mode >> 3) & m);
	acl_end = acl_last(acl);
	for (ace = acl->acl_ext; ace < acl_end; ace = acl_nxt(ace))
	{
		switch (ace->ace_type)
		{
			case ACC_PERMIT:
				if (!(ace->ace_access & m))
					perm = 0;
				continue;
			case ACC_SPECIFY:
				if (!(ace->ace_access & m))
					goto	noaccess;
				continue;
			case ACC_DENY:
				if (ace->ace_access & m)
					goto	noaccess;
				continue;
		}
	}

	/*
	 * no restriction found.
	 * if all permissive entries grant the access,
	 * we are done!
	 */
	if (perm)
		goto	access;
	
	/*
	 * the relatively hard (pathological) case ...
	 * introduced by emulating ternary access checks.
	 * there are no restrictive entries which deny the
	 * access, but a permissive entry does not grant it.
	 * for each such permissive entry, we need to assure
	 * another (redundant) permissive entry grants the access.
	 */

	make_ace (&fowner, ACC_SPECIFY, perm, ACEID_USER, ip->i_uid);
	make_ace (&fgroup, ACC_PERMIT, perm, ACEID_GROUP, ip->i_gid);
	if (!((ip->i_mode >> 3) & m))
	{
		if (!id_perm(&fgroup, NULL, NULL, acl->acl_ext, 
							acl_last(acl), m))
			goto	noaccess;
	}

	acl_end = acl_last(acl);
	for (ace = acl->acl_ext; ace < acl_end; ace = acl_nxt(ace))
	{
		if (ace->ace_access & m)
			continue;
		if (ace->ace_type != ACC_PERMIT)
			continue;
		/*
		 * PERMISSIVE entry which does not grant the access.
		 * see if there is an entry out there that
		 * grants the missing access
		 */
		if (!id_perm(ace, &fowner, &fgroup, acl->acl_ext, 
							acl_last(acl), m))
			goto	noaccess;
	}

access:
	DPRINTF (("check_all(): permitted\n"));
	acl_mapout(ip, saddr);
	return(0);

noaccess:
	DPRINTF (("check_all(): denied\n"));
	acl_mapout(ip, saddr);
	return(EACCES);
}


/*
 * NAME:	id_perm(id, fowner, fgroup, first, last, m, p)
 *
 * FUNCTION:	Run through ACL entries checking for an entry that
 *		grants the requested access for all processes that
 *		satisfy the identification in <id>.
 *
 * PARAMETERS:
 *		id	- the interesting entry
 *		fowner	- the owner "acl entry"
 *		fgroup	- the group "acl entry"
 *		first	- the first acl entry to check
 *		last	- the last acl entry to check
 *		m	- the access mode requested
 *
 * RETURN :	1	- access allowed
 *		0	- access denied
 */

static
id_perm (id, fowner, fgroup, first, last, m)
struct	acl_entry	*id;
struct	acl_entry	*fowner;
struct	acl_entry	*fgroup;
struct	acl_entry	*first;
struct	acl_entry	*last;
int	m;
{
/* a pointer to entries in the acl */
struct acl_entry	*ace;

	/*
	 * search the acl entries for a specific or permissive
	 * entry that grants the access to all processes which
	 * satisfy <id>.
	 */
	if (fowner && id_match (id, fowner))
	{
		/* actually, this check is redundant */
		if (fowner->ace_access & m)
			return(1);
	}
	if (fgroup && id_match (id, fgroup))
	{
		if (fgroup->ace_access & m)
			return(1);
	}
	
	for (ace = first; ace < last; ace = acl_nxt(ace))
	{
		if (ace->ace_type == ACC_DENY)
			continue;
		if (!(ace->ace_access & m))
			continue;
		/*
		 * this is a SPECIFIC/PERMISSIVE entry which
		 * grants the access.
		 * check whether it is applicable ...
		 */
		if (!id_match (id, ace))
			continue;
		return(1);
	}
	return(0);
}


static
check_any(ip, m, who)
struct inode	*ip;
int		m;
int		who;
{
	struct	acl	*acl;		/* the ".inodex" segment */
	char		*saddr;		/* start of the mapped inode */
	struct	acl_entry *acl_end;
	struct acl_entry	*ace;
	struct acl_entry	fowner;	/* psuedo acl entry for owner */
	struct acl_entry	fgroup; /* psuedo acl entry for group */
	int	perm;
	int	rc;

	DPRINTF (("check_any()\n"));

	/* the access to be checked must be exactly one of R/W/X */
	switch (m)
	{
		case R_ACC:
		case W_ACC:
		case X_ACC:
			break;
		default:
			return(EINVAL);
	}

	/* instant success if mode bits for "others" allows access */
	if (ip->i_mode & m)
		return(0);
	
	/* handle normal case first */
	if (USE_MODE (ip, ip->i_mode))
	{
		if (who == ACC_ANY)
		{
			if (((ip->i_mode >> 6) & m) == m)
				return(0);
		}
		if (((ip->i_mode >> 3) & m) == m)
			return(0);
		return(EACCES);
	}

	/*
	 * for the "ANY" check:
	 * if owner mode grants the requested accesses,
	 *	check for ACL entries that deny the requested access.
	 *	if none are found, the check for ANY succeeds
	 */
	if (who == ACC_ANY)
	{
		perm = (ip->i_mode >> 6) & 07;
		make_ace (&fowner, ACC_SPECIFY, perm, ACEID_USER, ip->i_uid);
		if (perm & m)
		{
			/* find any restrictions against this access */
			if (!id_restr(&fowner, NULL, NULL, acl->acl_ext, 
							acl_last (acl), m))
				/* no restriction found */
				goto	granted;
		}
	}
	/*
	 * for the "OTHERS" check:
	 * act as if the owner is denied all access
	 */
	else
		make_ace (&fowner, ACC_SPECIFY, 0, ACEID_USER, ip->i_uid);

	/*
	 * If group mode grants some access,
	 * check for ACL entries that deny the access
	 */
	make_ace (&fgroup, ACC_PERMIT, perm, ACEID_GROUP, ip->i_gid);
	perm = (ip->i_mode >> 3) & 07;
	if (perm & m)
	{
		if (!id_restr(&fgroup, NULL, NULL, acl->acl_ext, 
							acl_last(acl), m))
			/* no restriction found */
			goto	granted;
	}
	
	/*
	 * A run through the ACL is required.
	 * For each PERMISSIVE or SPECIFIC entry which grants
	 * the access, we need to look for an over-riding restriction.
	 */
	acl_end = acl_last (acl);
	for (ace = acl->acl_ext; ace < acl_end; ace = acl_nxt (ace))
	{
		/* restrictive entry which denies the access */
		if (ace->ace_type == ACC_DENY)
			continue;
		/* irrelevant permissions */
		if (!(ace->ace_access & m))
			continue;
		/*
		 * found an entry which grants the requested access.
		 * check if it is overridden by a restrictive entry
		 */
		if (!id_restr(ace, &fowner, &fgroup, acl->acl_ext, 
							acl_last(acl), m))
			/* no restriction found */
			goto	granted;
	}

	/*
	 * If we get to here then we didn't find any permissives without
	 * restrictives...
	 */
denied:
	acl_mapout(ip, saddr);
	return(EACCES);
granted:
	acl_mapout(ip, saddr);
	return(0);
}


/*
 * NAME:	id_restr(id, fowner, fgroup, first, last, m)
 *
 * FUNCTION:	Run through ACL entries checking for an entry
 *		which will deny the permissions granted to
 *		to processes which satisfy <id>.
 *
 * PARAMETERS:
 *		id	- the interesting entry
 *		fowner	- the owner "acl entry"
 *		fgroup	- the group "acl entry"
 *		first	- the first acl entry to check
 *		last	- the last acl entry to check
 *		m	- the access mode requested
 *
 * RETURN :	0	- restriction not found
 *		1	- restriction found
 */

static
id_restr (id, fowner, fgroup, first, last, m)
struct	acl_entry	*id;
struct	acl_entry	*fowner;
struct	acl_entry	*fgroup;
struct	acl_entry	*first;
struct	acl_entry	*last;
int	m;
{
/* a pointer to entries in the acl */
struct acl_entry	*ace;

	/*
	 * for accessx(OTHERS), the owner entry is used to preclude
	 * uid-based access by the file owner.
	 */
	if (fowner && id_match (id, fowner))
	{
		if (!(fowner->ace_access & m))
			return(1);
	}
	/*
	 * the group entry is strictly permissive, and
	 * need not be checked.
	 * check other entries
	 */
	for (ace = first; ace < last; ace = acl_nxt(ace))
	{
		if (ace->ace_type == ACC_PERMIT)
			continue;
		if (ace->ace_type == ACC_DENY)
		{
			if (!(ace->ace_access & m))
				continue;
		}
		else /* ace->ace_type == ACC_SPECIFY */
		{
			if (ace->ace_access & m)
				continue;
		}
		/*
		 * this entry prohibits the access.
		 * if it is applicable, we are done
		 */
		if (id_match (id, ace))
			return(1);
	}
	return(0);
}


/*
 * test whether  the processes specified by "A" are
 * a subset of those specified by the acl entry "B";
 * i.e., each entry in "B" must appear in "A"
 *
 * return 1 if true; 0 if false
 */
static
id_match (A, B)
struct acl_entry	*A;
struct acl_entry	*B;
{
struct	ace_id	*A_end;
struct	ace_id	*a;
struct ace_id	*B_end;
struct ace_id	*b;

	DPRINTF (("id_match()\n"));

	/* check each ID in "B" for inclusion in "A" */
	A_end = id_last (A);
	B_end = id_last (B);
	for (b = B->ace_id; b < B_end; b = id_nxt(b))
	{
		/* check that this entry from "B" appears in "A" */
		for (a = A->ace_id; 1; a = id_nxt (a))
		{
			if (a >= A_end)
				/* this ID in "B" was not found in "A" */
				return(0);
			if (a->id_type != b->id_type)
				continue;
			if (a->id_len != b->id_len)
				continue;
			if (memcmp (b->id_data, a->id_data, b->id_len) == 0)
				/* found the ID from "B" in "A" */
				break;
		}
	}
	/* found all entries from "B" in "A" */
	return(1);
}
