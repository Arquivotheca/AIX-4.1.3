static char sccsid[] = "@(#)29	1.27.1.11  src/bos/kernel/pfs/xix_rename.c, syspfs, bos41J, 9507C 2/14/95 13:45:04";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: 	incest, 
 *		level_set, 
 *		unlockinodes,
 *		jfs_rename
 *
 * ORIGINS: 3, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "jfs/jfslock.h"
#include "jfs/inode.h"
#include "jfs/commit.h"
#include "sys/errno.h"
#include "sys/vfs.h"
#include "sys/dir.h"
#include "sys/sysinfo.h"

#define		NOLINKS(x)	((x)->i_nlink < 1)
#define		ISDIR(x)	(((x)->i_mode & IFMT) == IFDIR)

static int level_set(struct inode *, struct inode *, struct inode *,
                struct inode **, struct ucred *);
static int incest(struct inode *, struct inode *, struct inode *,
                struct vfs *, struct ucred *);
static int unlockinodes(struct inode *, struct inode *, struct inode *, 
		struct inode *, struct inode *);

/*
 * NAME:	jfs_rename (svp, spvp, sname, dvp, dpvp, dname, crp)
 *
 * FUNCTION:	Atomic rename of svp(name "sname") to dvp(name "dname").
 *		This function could be much simpler if I could be assured
 * 		that the source and destination parents still contained the
 *		source and destination given.  However several processes
 *		can race through the LFS lookup.  This can give me conditions
 *		where both processes discover no destination and one blocks
 *		while the other creates it.  Once the blocked process runs
 *		he will discover EEXIST when he tries to make the entry. 
 *
 *		Basically, I can't trust the vnodes for the source and
 *	 	destination names and have to look them up again after I
 *		gain locks.
 *
 * PARAMETERS:	svp	- vnode for sname, the source
 *		spvp	- svp's parent vnode
 *		sname	- name refering to svp, the source
 *		dvp	- destination vnode
 *		dpvp	- parent of dvp
 *		dname	- destination name
 *		crp	- credential
 *
 * RETURN : 	EINVAL	- . or .. renames or incestuous relationship
 *			  between the source and destination.
 *		EINVAL  - If !sameparents and source .. is missing or invalid 
 *		EINVAL  - If destination . or .. are invlaid 
 *		errors from subroutines
 */

jfs_rename (svp, spvp, sname, dvp, dpvp, dname, crp)
struct vnode  	*svp;
struct vnode  	*spvp;
caddr_t 	sname;
struct vnode  	*dvp;
struct vnode  	*dpvp;
caddr_t 	dname;
struct ucred 	*crp;		/* pointer to credential structure */
{
	struct hinode *hip;	/* hash list where inode resides */
        struct inode 
		*sip, 		/* Source inode pointer on entry 	*/
		*spip,		/* Source parent inode on entry		*/
		*dip,		/* Destination inode pointer on entry   */
		*dpip,		/* Destination parent inode on entry	*/
		*xsip = NULL,	/* Source inode after dir_lookup        */
		*xdip = NULL;	/* Destination inode after dir_lookup   */
	ino_t	ino;		/* Inode from dir_lookup		*/
	int 	rc;		/* Return code				*/
	int	sameparents;	/* Parent directories are same		*/
	int	dir2dir = 0;	/* Directory to directory rename	*/
	int	s_dots = 0;  	/* Status of source dot and dot dot 	*/
	int	d_dots = 0;	/* Status of destination dot and dot dot*/
	dname_t	dnmp, snmp;	/* Directory name struct		*/
	struct	commit	clist;
	struct	inode	*ixip = NULL;
	struct vfs	*svfsp, *dvfsp;
	extern	struct	inode *sec_delete();


	dpip = VTOIP(dpvp);		/* Destination's parent inode	*/
	dvfsp = dpvp->v_vfsp;
	if (!(dvfsp->vfs_flag & VFS_DEVMOUNT))
		dvfsp = NULL;
	dip = (dvp)? VTOIP(dvp): NULL;  /* Destination inode            */

	spip = VTOIP(spvp);		/* Source's parent inode	*/
	svfsp = spvp->v_vfsp;
	if (!(svfsp->vfs_flag & VFS_DEVMOUNT))
		svfsp = NULL;
	sip = VTOIP(svp);		/* Source inode			*/

	/* rename of "." or ".." is not supported. */
	if (ISDOTS(dname) || ISDOTS(sname))
		return EINVAL;

	/*
	 * Loop for redriving this interface if I discover that my source
	 * and destinations are not the same as the ones that the LFS gave me
	 */
	while (1)
	{
		if (sip == dip)			/* If same inode return ok */
		{
			rc = 0;
			goto iput_out1;
		}
		
		if (spip->i_dev != dpip->i_dev)	/* Cross device link */
		{
			rc = EXDEV;
			goto iput_out1;
		}

		/* If destination parent and source inode are the same, 
		 * this is a 1st order incestuous situation and deemed 
		 * rotten from start.
	 	 */
		if (sip == dpip)
		{
			rc = EINVAL;
			goto iput_out1;
		}

		/* 
	 	 * If source parent is destination inode, then it is not 
		 * empty.  This would of course be caught below but locking
		 * the inodes deadlocks!
	  	 */
		if (spip == dip)
		{
			rc = EEXIST;
			goto iput_out1;
		}

		/* 
	 	 * If Source and destination parents are same, then lock 
		 * the one inode.
	 	 */
		if (sameparents = (spip == dpip))
			iwritelocklist(dip ? 3 : 2, spip, sip, dip);
		else
			iwritelocklist(dip ? 4 : 3, spip, dpip, sip, dip);

        	/* Fix up name args */
        	dnmp.nm = dname;
        	dnmp.nmlen = strlen(dname);
        	snmp.nm = sname;
        	snmp.nmlen = strlen(sname);

		/*
		 * We are going to be looking up the current inode in the
		 * directory, and comparing it to the one that we were 
		 * initially given by the LFS.  If the inodes are different 
		 * we are going to release the locks and start over at the
		 * top of this routine.  If the dir_lookup() gives us
		 * any error on the source then we exit.  
		 */
        	if (rc = dir_lookup(spip, &snmp, &ino, crp))
			goto unlock_out;

		if (ino != sip->i_number)	/* very rare */
		{
			unlockinodes(NULL, sip, NULL, dip, ixip);

			IHASH(spip->i_dev, ino, hip);
			sysinfo.iget++;
			cpuinfo[CPUID].iget++;
			ICACHE_LOCK();
			if (xsip)
				iput(xsip, svfsp);

			rc = _iget(spip->i_dev, ino, hip, &xsip, 1, svfsp);
			/* unlock the parent inode
			 */
			unlockinodes(spip, NULL, sameparents ? NULL : dpip,
					NULL, NULL);
			if (rc)
				goto iput_out2;
			ICACHE_UNLOCK();

			sip = xsip;
			continue;
        	}

		/*
		 * Lookup the destination inode to see if it is different.
		 * The destination doesn't have to exist but all other
		 * errors are fatal.  This mismatch can occur quite often.
		 */
		if (rc = dir_lookup(dpip, &dnmp, &ino, crp))
		{
			if (rc == ENOENT)
			{
				if (dip)	/* No longer have original */
				{		/* destination	  	   */	
					IWRITE_UNLOCK(dip);
					dip = NULL;
				}
			}
			else
				goto unlock_out;
		}

		/*
		 * If upon lookup I get an inode and didn't have one at
		 * the beginning, or if I had one at the beginnning and
		 * it now doesn't match then I have to release the inodes
		 * and start over.
		 */
		if ((ino && !dip) || (dip && (dip->i_number != ino)))
		{
			unlockinodes(NULL, sip, NULL, dip, ixip);

			IHASH(dpip->i_dev, ino, hip);
			sysinfo.iget++;
			cpuinfo[CPUID].iget++;
			ICACHE_LOCK();
			if (xdip)
				iput(xdip, dvfsp);
			rc = _iget(dpip->i_dev, ino, hip, &xdip, 1, dvfsp);
			unlockinodes(spip, NULL, sameparents ? NULL : dpip,
					NULL, NULL);
			if (rc)
				goto iput_out2;
			ICACHE_UNLOCK();

			dip = xdip;
			continue;
		}

		break;
	} /* end while (1) */
			
	/* Check access permissions and invalid parameters */
	if (rc = level_set(spip, sip, dpip, &dip, crp))   
		goto unlock_out;

	/* Look for incestuous relationship between the 
	 * source and destination paths
	 */
	if (ISDIR(sip))
	{
		if (! sameparents)
		{
			if (rc = dir_valid (sip, spip->i_number, 0, &s_dots))
				goto unlock_out;

			/*
			 * Source must have a '..' entry and it must be valid
			 */
			if (DDOT_STAT(s_dots) != (DDOT_EXIST | DDOT_VALID))
			{
				rc = EINVAL;
				goto unlock_out;
			}

			if (dpip->i_nlink >= LINK_MAX)
			{
				rc = EMLINK;
				goto unlock_out;
			}
			if (rc = incest(spip, sip, dpip, dvfsp, crp))
				goto unlock_out;
		}

		dir2dir = 1;
		if (dip)
		{
  			if (rc = dir_valid(dip, dpip->i_number, 1, &d_dots))
	  			goto unlock_out;
			/*
			 * If the destination has '.' or '..' entries they
			 * must be valid.
			 */

			if (((d_dots & DOT_EXIST) &&  !(d_dots & DOT_VALID)) ||
			    ((d_dots & DDOT_EXIST) && !(d_dots & DDOT_VALID)))
			{
				rc = EINVAL;
				goto unlock_out;
			}
		}
	}
	
	/* if destination exists remove it from its parent */
	if (dip)
	{
		/*
		 * Before we delete the target, try to ensure that the source
		 * will be able to be linked by checking LINK_MAX. Other
		 * errors such as I/O problems during the dir_link() will
		 * result in a loss of the original target.
	  	*/
		if (sip->i_nlink >= LINK_MAX)
		{
			rc = EMLINK;		
			goto unlock_out;
		}

		if (rc = dir_delete (dpip, &dnmp, dip, crp))
			goto unlock_out;
	}

	/* adjust link counts if target was an existing directory */
	if (dip && dir2dir)
	{
		dip->i_nlink = 0;
		if (DDOT_STAT(d_dots) & DDOT_EXIST)
			dpip->i_nlink--;
	}

	/* Link the destination name to the source inode */
	if (rc = dir_link (dpip, &dnmp, sip, crp))
		goto unlock_out;

	/* Remove source name from source directory */
	if (dir_delete (spip, &snmp, sip, crp))
	    panic("jfs_rename: unlink source name failed");

	/* 
	 * If this is a directory to directory rename and the source 
	 * and destination do not have same parents then fix the .. 
	 * reference so .. in source refers to destination parent and
	 * not old sources parent.
	 */
	if (dir2dir && !sameparents)
		if (dir_fixparent(sip, spip, dpip))
		    panic("jfs_rename: dir_fixparent failed");

	clist.iptr[0] = sip;
	clist.iptr[1] = spip;
	clist.number = 2;
	if (!sameparents)
		clist.iptr[clist.number++] = dpip;
	if (dip)
	{
		clist.iptr[clist.number++] = dip;
		ixip = sec_delete(dip);
		if (ixip)
			clist.iptr[clist.number++] = ixip;
	}

	rc = comlist(&clist);

unlock_out:
	unlockinodes(spip, sip, sameparents ? NULL : dpip, dip, ixip);

iput_out1:
	ICACHE_LOCK();

iput_out2:
	if (xsip)
		iput(xsip, svfsp);

	if (xdip)
		iput(xdip, dvfsp);

	ICACHE_UNLOCK();

	RETURNX (rc, reg_elist);
}

/*
 * NAME: 	unlockinodes()
 * 
 * FUNCTION: 	Helper function for jfs_rename() to unlock source, source
 *		parent, destination, destination parent, and inode extenstion
 *		pointers.
 *
 * RETURNS: 	nothing
 */
static int
unlockinodes(struct inode *spip, 
	     struct inode *sip,
	     struct inode *dpip,
	     struct inode *dip,
	     struct inode *ixip)
{
	if (spip)
		IWRITE_UNLOCK(spip);
	if (dpip)
		IWRITE_UNLOCK(dpip);
	if (sip)
		IWRITE_UNLOCK(sip);
	if(dip)
		IWRITE_UNLOCK(dip);
	if (ixip)
		IWRITE_UNLOCK(ixip);
}
	
/*
 * NAME:	incest
 *     
 * FUNCTION:	Ensure source is not an element of destination parent.
 *		All inodes should be locked on entry.
 *
 * RETURNS:	EINVAL	- source is an element of destination
 *		errors from dir_lookup()		
 */
static int
incest(struct inode *sdp,       /* Source parent        */
       struct inode *sp,        /* Source pointer       */
       struct inode *ddp,       /* Destination parent   */
       struct vfs   *dvfsp,     /* Destination vfs   */
       struct ucred *crp)       /* Credentials          */
{
	struct inode *ip;
	struct hinode *hip;
	direct_t *dirp;
	dname_t	dnm;
	ino_t ino;
	int rc = 0;

	if (ddp->i_number == ROOTINO)
		return 0;

	IWRITE_LOCK(ddp->i_ipmnt);

	dnm.nm = "..";
	dnm.nmlen = 2;
	ip = ddp;

	while(1)
	{
		if (rc = dir_lookup (ip, &dnm, &ino, crp))
			break;

		/* Stop at root or common ancestor
		 */
		if (ino == ROOTDIR_I || ino == sdp->i_number)
			break;

		if (ino == sp->i_number)
		{	rc = EINVAL;
			break;
		}


		if (ip != ddp)
		{
			IWRITE_UNLOCK(ip);
			ICACHE_LOCK();
			iput(ip, dvfsp);
			ICACHE_UNLOCK();
		}
	
		IHASH(ip->i_dev, ino, hip);	
		ICACHE_LOCK();
		rc = _iget(ip->i_dev, ino, hip, &ip, 1, dvfsp);
		ICACHE_UNLOCK();
		sysinfo.iget++;
		cpuinfo[CPUID].iget++;
		if (rc)
			break;

		IWRITE_LOCK(ip);
	}

	IWRITE_UNLOCK(ddp->i_ipmnt);

	if (ip && ip != ddp)
	{
		IWRITE_UNLOCK(ip);
		ICACHE_LOCK();
		iput(ip, dvfsp);
		ICACHE_UNLOCK();
	}

	return rc;
} 

/* 
 * NAME:	level_set()
 *
 * FUNCTION:	Checks for proper inodes, inode types, and permissions
 *		for a rename() to occur.
 *
 * RETURNS:	ENOTDIR	- source is directory, yet target is not.
 *		EISDIR  - source is not a directory, yet target is.
 *		EACCESS - access denied
 *		EINVAL	- bad parameters
 *		EPERM	- savetext bit prevents deletion
 *
 * NOTES:	Caller is expected to hold inode locks on all parameters.  
 */
static int
level_set(struct inode *spip,
          struct inode *sip,
          struct inode *dpip,
          struct inode **dip,
          struct ucred *crp)
{
	int rc = 0;			/* Return code			*/
	int sameparents = 0;		/* are dpip and spip the same?	*/
	struct inode *dp = NULL;	/* Temp variable for *dip	*/

	if (*dip)
		dp = *dip;
	
	if (spip == dpip)
		sameparents++;

	/*
	 * Check the existence of the children.  If they exist we will 
	 * know that the parent must exist, ie they were not empty and
	 * therefore not removeable.
	 */

	/* All but dp must have positive link count
	 */
	if (dp && NOLINKS(dp))
	{
		IWRITE_UNLOCK(dp);
		dp = NULL;
		*dip = NULL;
	}

	/*
	 * If src is a directory then target must be a directory.
	 * if src is not a directory, then target cannot be a directory.
	 */
	if (dp && ((ISDIR(sip) && !ISDIR(dp)) || (!ISDIR(sip) && ISDIR(dp))))
	{
		rc = ((sip->i_mode & IFMT) == IFDIR ) ? ENOTDIR : EISDIR;
		goto bad;
	}
		
	if (NOLINKS(sip))
	{
		rc = EINVAL;
		goto bad;
	}

	if (!dp && !sameparents && (!ISDIR(dpip))) {
	    rc = (ISDIR(sip)) ? ENOTDIR : EISDIR;
	    goto bad;
	}

	/* We need search and write permission
	 * in both parent directories
	 */
	if (rc = iaccess(spip, IWRITE|IEXEC, crp))
		goto bad;

	if (!sameparents && (rc = iaccess(dpip, IWRITE|IEXEC, crp)))
		goto bad;

	/* Even if we have write permission in the source parent
	 * we still might not be able to delete the old link if
	 * it's a sticky directory (or for some other reason)
	 */
	rc = dir_del_access(spip, sip, crp);

bad:
	return rc;
}
