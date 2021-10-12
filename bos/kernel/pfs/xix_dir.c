static char sccsid[] = "@(#)08	1.68.1.23  src/bos/kernel/pfs/xix_dir.c, syspfs, bos41J, 9507C 2/14/95 13:09:26";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: dir_create, dir_delete, dir_find, dir_del_access
 *            dir_fixparent, dir_ialloc, dir_link, dir_lookup
 *            dir_makentry, dir_mkdir, dir_mknod,
 *            dir_valid, dir_rmentry, dir_search,
 *            dirmangled
 *
 * ORIGINS: 3, 24, 26, 27
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
 *
 * Copyright (c) 1986 by Sun Microsystems, Inc.
 */

#include	"jfs/fsdefs.h"
#include	"jfs/inode.h"
#include	"jfs/jfslock.h"
#include	"jfs/commit.h"
#include	"sys/vfs.h"
#include	"sys/errno.h"
#include	"sys/sysinfo.h"
#include	"sys/syspest.h"
#include	"sys/dir.h"
#include	"vmm/vmsys.h"

/*
 * The slot structure holds state for the directory manager operations 
 * that create and remove directory entries.
 * After dirfind() the values are:
 *	status	offset		size
 *	------	------		----
 *	NONE	end of dir	needed
 *	COMPACT	start of area	of area
 *	FOUND	start of entry	of ent
 *	EXIST	start if entry	of prev ent
 */

enum slotstat	{NONE, COMPACT, FOUND, EXIST};
struct slot {
	enum	slotstat sl_status;	/* status			*/
	direct_t * sl_addr;	/* addr of area with free space */
	int	sl_size;	/* size of area from sl_addr */
	int	sl_free; 	/* size of freespace of the area */
};

dname_t dot = {".", 1 };
dname_t dotdot = {"..", 2 };

/* Prototype empty directory
 */
static struct edir {
	/* Dot */
	ulong	d_ino;		/* XXX */
	ushort	d_reclen;
	ushort	d_namlen;
	char	d_name[DIROUND];
	/* Dot Dot */
	ulong	dd_ino;		/* XXX */
	ushort	dd_reclen;
	ushort	dd_namlen;
	char	dd_name[DIROUND];
} edir = {0, LDIRSIZE(1), 1, ".", 0, DIRBLKSIZ - LDIRSIZE(1), 2, ".."};

/* Static subroutines within xix_dir.c
 */
static void dirmangled(direct_t *);
static dir_makentry(), dir_rmentry(), dir_search(), dir_find();

#define NOSTALE			((ino_t)-1)
#define ONBOUNDARY(addr)	((((uint)(addr)) & (DIRBLKSIZ - 1)) == 0)
#define	ISDOTDOT(name)		((name)[0] == '.' && (name)[1] == '.' \
				&& (name)[2] == '\0')


/*
 * NAME:	dir_search(dp, nmp, slotp, ino)
 *
 * FUNCTION:	This function parses a directory to search for file
 *		named "nmp.nm".  If nm is not found ENOENT is returned 
 *		else slotp.sl_addr is set to point to the vaddr for the 
 *		corresponding directory entry.
 *
 * PARAMETERS:	dp	- directory inode
 *		nmp	- name struct, ie string and length
 *		slotp	- bsd style slot info.  Used as input and output.
 *			  Give characteristics of found directory structure.
 *			  See structure declaration above.
 *		ino	- compare to inode for ESTALE detection 
 *
 *
 * RETURN : 	if found,
 *			return addr in slotp->sl_addr
 *		else
 *			return ENOENT
 *
 * NOTE:	Caller is responsible for restoring virtual address space,
 *		via ipundo().  (this is not unlike the bsd'ism of 
 *		making directory update callers do brele()'s.)
 *
 *		Caller must verify search permission if required.
 */

static
dir_search (dp, nmp, slotp, ino)
struct inode	*dp;			/* Directory inode		*/
dname_t		*nmp;			/* Directory name struct	*/
struct slot	*slotp;			/* Return info			*/
ino_t		ino;			/* compare to ino 		*/
{
	int rc;				/* return code			*/
	direct_t *dirp;			/* Directory vaddr		*/
	direct_t *dlast;		/* Directory bounds		*/
	direct_t *ldirp;		/* Last entry processed		*/
	label_t   jb;      		/* setjmpx/longjmpx buffer 	*/

	/* dp must be a directory */
	if ((dp->i_mode & IFMT) != IFDIR || dp->i_nlink < 1)
		return ENOTDIR;

	/* set up to search a directory */

	if (rc = iptovaddr(dp, 1, &dirp))
		return rc;

	dlast = (direct_t *)((caddr_t)dirp + dp->i_size);
	ldirp = dirp;

	/* establish exception return point for 
	 * permanent io errors. errors may occur as result of loads
	 * or stores into memory mapped directory.
	 */
	if (rc = setjmpx(&jb))
	{	
		ipundo(dirp);
		rc |= PFS_EXCEPTION;
		return rc;
	}

	while (dirp < dlast)
	{
		/* tick sysinfo counter when crossing into a new block */
		if (((int) dirp & (DIRBLKSIZ-1)) == 0)
		{
			sysinfo.dirblk++;	/* unserialized, may produce */
			cpuinfo[CPUID].dirblk++;/* fuzzy statistics          */
		}

		/* String compare the directory entry and the 
		 * current component. If they do not match, try next entry.
		 */
		if (dirp->d_ino && *dirp->d_name == *nmp->nm &&
		    dirp->d_name[LDIRNMLEN(dirp)] == nmp->nm[nmp->nmlen] &&
		    LDIRNMLEN (dirp) == nmp->nmlen &&
		    strncmp(nmp->nm, dirp->d_name, LDIRNMLEN (dirp)) == 0)
		{	
			if (ino == NOSTALE || ino == dirp->d_ino)
			{	
				slotp->sl_status = EXIST;
				slotp->sl_addr = dirp;
				slotp->sl_size = ldirp->d_reclen; 
				goto srch_out;
			}
			rc = ESTALE;
			break;		/* while */

		}

		ldirp = dirp;
		dirp = (direct_t *)((caddr_t)dirp + dirp->d_reclen);
	}

	slotp->sl_status = NONE;
	slotp->sl_addr = NULL;
	ipundo (dirp);
	rc = (rc) ? rc : ENOENT;

srch_out:
	clrjmpx(&jb);		/* pop exception return */
	return rc;
}


/*
 * NAME:	dir_find(dp, nmp, slotp, inop, crp)
 *
 * FUNCTION:	This function parses a directory to search for file
 *		named "nmp.nm".  
 *		If nm is not found, either a directory entry
 *		or a directory region of consecutive directory
 *		entries which when compacted can accomodate the name is 
 *		recorded in the slot parameter and 0 is returned.  
 *		If the name is found, the vaddr for the corresponding 
 *		directory entry is recorded in the slot parameter and 
 *		EEXIST is returned.
 *
 * PARAMETERS:	dp	- directory inode
 *		nmp	- name struct, ie string and length
 *		slotp	- bsd style slot info.  Used as input and output.
 *			  Give characteristics of found directory structure.
 *			  See structure declaration above.
 *		inop	- pointer to ino # to return on EEXIST for create()
 *		crp	- credential
 *
 * RETURN : 	if found,
 *			return EEXIST
 * 		else
 *			return 0
 *
 * NOTE: the caller is responsible for locking, unlocking, mapping and 
 * unmapping the directory.
 */

static
dir_find (dp, nmp, slotp, inop, crp)
struct inode	*dp;		/* Directory inode		*/
dname_t		*nmp;		/* Directory name struct	*/
struct slot	*slotp;		/* Return info			*/
ino_t		*inop;		/* !null if create, return ino	*/
struct ucred	*crp;		/* pointer to credential structure */
{
	int rc;				/* return code			*/
	direct_t *dirp;			/* Mapped directory address	*/
	direct_t *dlast;		/* Directory bounds		*/
	direct_t *ldirp;		/* Last entry processed		*/
	int slotfreespace;		/* free space in page 		*/
	int needed;			/* size for new entry		*/
	int wrtacc;			/* rc - access check for write	*/
	label_t   jb;      		/* setjmpx/longjmpx buffer 	*/

	/* dp must be a directory */
	if ((dp->i_mode & IFMT) != IFDIR || dp->i_nlink < 1)
		return ENOTDIR;

	/* Don't search in the directory unless our caller 
	 * has permission.
	 */
	if (rc = iaccess(dp, IEXEC, crp))
		return rc;

	/* Delay actual checking for write permission until we
	 * know that we must actually modify the dir.
	 */
	wrtacc = iaccess(dp, IWRITE, crp);

	if (rc = iptovaddr(dp, 1, &dirp))
		return rc;

	/* establish exception return point for 
	 * permanent io errors. io errors may occur as result of loads
	 * or stores into memory mapped directory.
	 */
	if (rc = setjmpx(&jb))
	{	
		ipundo(dirp);
		rc |= PFS_EXCEPTION;
		return rc;
	}

	/*
	 * scan directory
	 */
	slotp->sl_status = NONE;
	needed = LDIRSIZE (nmp->nmlen);
	dlast = (direct_t *)((caddr_t)dirp + dp->i_size);
	ldirp = dirp;

	while (dirp < dlast)
	{	
		if (slotp->sl_status != FOUND)
		{
			int size = dirp->d_reclen;

			/* if on DIRBLKSIZ boundary and we are still in the
			 * hunt, reset.  if we have already found a dir block
			 * to use then don't destroy the saved address.
			 * 		jar
			 */
			if (ONBOUNDARY(dirp))
			{	
				if (slotp->sl_status == NONE)
					slotp->sl_addr = (direct_t *) -1;
				slotfreespace = 0;
			}

			/*
			 * If an appropriate sized slot has not yet been found,
			 * check to see if one is available. Accumulate space
			 * in the current DIRBLK so that we can determine if
			 * compaction is viable.
			 */

			/* This subtraction takes care of entries whose 
			 * record length is holding the "slop" at the
			 * tail of a directory page.  IF there is enough
			 * then claim it.  d_reclen > LDIRSIZE() is ok
			 * but not vise-versa.
			 *
			 * d_ino == 0 happens only on DIRBLKSIZE boundary.
			 */
			if (dirp->d_ino != 0)
				size -= LDIRSIZE(dirp->d_namlen);

			if (size > 0)
			{	
				if (size >= needed)
				{	
					/* current entry has freespace
					 * for new entry
					 */
					slotp->sl_status = FOUND;
					slotp->sl_addr = dirp;
					slotp->sl_size = dirp->d_reclen;
					slotp->sl_free = size;
				}
				/* Someday we could consider keeping
				 * track of all slotfreespace and compacting
				 * the whole directory every so often.
				 */
				else if (slotp->sl_status == NONE)
				{
					/* current entry has freespace
					 * less than for new entry.
					 * accumulate for compaction.
					 */
					slotfreespace += size;

					/* if we have crossed into a new
					 * block at some point, reset compaction
					 * begining.
					 */
					if ((int) slotp->sl_addr == -1)
						slotp->sl_addr = dirp;

					/* if compaction accumulated freespace
					 * for new entry, record COMPACT.
					 * if a single entry with freespace
					 * for new entry is found later,
					 * it will be reset for FOUND.
					 */
					if (slotfreespace >= needed)
					{	
						slotp->sl_status = COMPACT;
						slotp->sl_size
						  = (caddr_t) dirp 
						  + dirp->d_reclen
						  - (caddr_t) slotp->sl_addr;
						slotp->sl_free = slotfreespace;
					}
				}
			} /* end (size > 0) */
		} /* end (sl_status != FOUND) */

		/* String compare the directory entry and the 
		 * current component. If they do not match, try next entry.
		 */
		if (dirp->d_ino && *dirp->d_name == *nmp->nm &&
		    dirp->d_name[LDIRNMLEN(dirp)] == nmp->nm[nmp->nmlen] &&
		    LDIRNMLEN (dirp) == nmp->nmlen &&
		    strncmp(nmp->nm, dirp->d_name, LDIRNMLEN (dirp)) == 0)
		{	
			slotp->sl_status = EXIST;
			slotp->sl_addr = dirp;
			slotp->sl_size = ldirp->d_reclen;

			/* inop is used instead of create flag or otherwise
			 * so i don't need execption handler in more
			 * places than absolutely neccessary
			 */
			if (inop)
				*inop = dirp->d_ino;	

			ipundo(dirp);
			rc = EEXIST;
			goto find_out;
		}

		ldirp = dirp;
		dirp = (direct_t *)((caddr_t)dirp + dirp->d_reclen);
	} /* end while */

	/* finalize the slot structure to return
	 */
	if (slotp->sl_status == NONE)
	{	
		assert(((uint)dlast & (DIRBLKSIZ-1)) == 0);
		slotp->sl_addr = dlast;
		slotp->sl_size = needed;
		slotp->sl_free = needed;
	}
	else if (slotp->sl_status == FOUND)
	{
		/* Consistency check entry about to be returned. */
		dirmangled(slotp->sl_addr);
	}
	else /* (slotp->sl_status == COMPACT) */
	{
		int	dsize = 0;

		/* Consistency check entry about to be returned. */
		dirp = slotp->sl_addr;
		while (dsize < slotp->sl_size) 
		{
			dirmangled(dirp);
			dsize += dirp->d_reclen;
			dirp = (direct_t *)((caddr_t)dirp + dirp->d_reclen);
		}
		
	}

	/* Now check write access to the directory since
	 * it's going to have to be modified.
	 */
	if (rc == 0)
		if (rc = wrtacc)
			ipundo(dirp);

find_out:
	clrjmpx(&jb);			/* pop exception return */
	return rc;
}


/*
 * NAME:	dir_lookup (dp, nmp, inop, crp)
 *
 * FUNCTION:	This function is an external interface to directory
 *		management to return a pointer to a dirent sturct.
 *		This avoids externalizing the slot structure.
 *
 * PARAMETERS:	Dp	- directory inode
 *		nmp	- name struct, ie string and length
 *		inop	- pointer to pointer for found directory entry
 *			  inode number
 *		crp	- credential
 *
 * NOTE: the caller is responsible for locking, unlocking the directory.
 */

dir_lookup (dp, nmp, inop, crp)
struct inode	*dp;		/* Directory inode		*/
dname_t		*nmp;		/* Directory name struct	*/
ino_t		*inop;		/* returned ino 		*/
struct ucred	*crp;		/* pointer to credential structure */
{
	int rc;
	struct slot slot;

	*inop = 0;

	/* check for search permission in dp first */
	if (rc = iaccess(dp, IEXEC, crp))
		return rc;

	if (rc = dir_search(dp, nmp, &slot, NOSTALE))
		return rc;

	/* Calculated risk.  by all rights there should be an exception
	 * handler here. dir_search() just touched *sl_addr so there is
	 * next to no chance of a pagefault resulting in i/o error.
	 */
	*inop = slot.sl_addr->d_ino;

	ipundo(slot.sl_addr);

	return rc;
}


/*
 * NAME:	dir_link (dp, nmp, ip, crp)
 *
 * FUNCTION:	create a directory entry in <dp> which refers to <ip>;
 *		write the directory entry in <dp>
 *		bump the link count on <ip>
 *
 * PARAMETERS:	Dp	- directory inode
 *		nmp	- name struct, ie string and length
 *		ip	- inode to link to
 *		crp	- credential
 *
 * RETURN :	errors from subroutines
 *
 * SERIALIZATION: parent directory inode locked on entry/exit.
 */

dir_link(dp, nmp, ip, crp)
struct inode	*dp;		/* Directory inode		*/
dname_t		*nmp;		/* Directory name struct	*/
struct inode	*ip;		/* Link to inode		*/
struct ucred	*crp;		/* pointer to credential structure */
{
	int rc;
	struct	slot slot, *slp; 	/* Directory slot info		*/
	struct  ucred *ucp = NULL;

	/* check for link count overflow on parent dir */
	if (ip->i_nlink >= LINK_MAX)
		return EMLINK;

	slp = &slot;		
	if (rc = dir_find(dp, nmp, slp, NULL, crp))
		return rc;

	/* Page fault quota allocation check looks at uid but NFS
	 * server which is mono threaded always runs as uid zero.
	 */
	if (U.U_procp->p_threadcount == 1 && crp->cr_uid != U.U_uid)
	{
		ucp = U.U_cred;
		U.U_cred = crp;
	}

	/* Make directory entry for nmp.
	 */
	if ((rc = dir_makentry(dp, nmp, ip->i_number, slp)) == 0)
	{	
		ip->i_nlink++;
		imark(ip, ICHG);
	}

	if (ucp)
		U.U_cred = ucp;

	/* restore virtual address space */
	ipundo(slp->sl_addr);

	return rc;
}


/*
 * NAME:	dir_create(dp, nmp, mode, ipp, vfsp, crp)
 *
 * FUNCTION:	create an object in <dp> with specified name and mode/type.
 * 		specifically:
 *			find entry if it exists
 *			if not, allocate an inode and write entry
 *
 * PARAMETERS:	Dp	- directory inode
 *		nmp	- name struct, ie string and length
 *		mode	- rwx mode and IFMT
 *		ipp	- Returned inode
 *		vfsp	- vfs pointer
 *		crp	- credential
 *
 * RETURN:	if new object created successfully
 *			rc = 0 with newly created locked inode
 *		if already exists
 *			rc = EEXIST with referenced inode
 *		otherwise rc = errno from subroutines
 *
 * SERIALIZATION: parent directory inode locked on entry/exit.
 */

dir_create(dp, nmp, mode, ipp, vfsp, crp)
struct inode	*dp;		/* Directory inode		*/
dname_t		*nmp;		/* Directory name struct	*/
int		mode;		/* mode of new file		*/
struct inode	**ipp; 		/* ptr to inode to return	*/
struct vfs	*vfsp;		/* vfs pointer			*/
struct ucred	*crp;		/* pointer to credential structure */
{
	int rc, rc1;			/* Return code			*/
	struct	slot slot, *slp; 	/* Directory slot info		*/
	struct inode	*ip;		/* Varying inode		*/
	ino_t ino;			/* inode # if EEXIST 		*/
	struct hinode *hip;		/* hash list inode resides on	*/
	struct 	ucred *ucp = NULL;
	
	*ipp = NULL;

	slp = &slot;		
	rc = dir_find(dp, nmp, slp, &ino, crp);
	switch (rc)
	{
		case 0:
	 		/* dir_ialloc() returns new, locked inode */
			if ((rc = dir_ialloc(dp, mode, &ip, vfsp, crp)) == 0)
			{	
				/* Page fault quota allocation check looks at 
				 * uid but NFS server which is mono threaded 
				 * always runs as uid zero.
	 			 */
				if (U.U_procp->p_threadcount == 1 && 
				    crp->cr_uid != U.U_uid)
				{
					ucp = U.U_cred;
					U.U_cred = crp;
				}

				if ((rc = dir_makentry
					(dp, nmp, ip->i_number, slp)) == 0)
						*ipp = ip;
				else
				{
					/* discard the new inode
					 */
					ip->i_nlink = 0;
					IWRITE_UNLOCK(ip);
					ICACHE_LOCK();
					iput(ip, vfsp);
					ICACHE_UNLOCK();
				}
				if (ucp)
					U.U_cred = ucp;
			}

			/* restore virtual address space */
			ipundo(slp->sl_addr);
			break;

		case EEXIST:
			IHASH(dp->i_dev, ino, hip);
			ICACHE_LOCK();
			if (rc1 = _iget(dp->i_dev, ino, hip, ipp, 1, vfsp))
				rc = rc1;
			ICACHE_UNLOCK();
			sysinfo.iget++;
			cpuinfo[CPUID].iget++;
			break;
	}

	return rc;
}


/*
 * NAME:	dir_mknod(dp, nmp, mode, ipp, vfsp, crp);
 *
 * FUNCTION:	create an object in <dp> with specified name and mode/type.
 * 		specifically:
 *			find entry. IF it exists return error.
 *			if not, allocate an inode and write entry
 *
 * PARAMETERS:	Dp	- directory inode
 *		nmp	- name struct, ie string and length
 *		mode	- rwx mode and IFMT
 *		ipp	- Returned inode
 *		vfsp	- vfs pointer
 *		crp	- credential
 *
 * RETURN:	Zero is returned if completes sucessfully.
 *		An error code (errno) is returned to indicate
 *		failure.
 *
 * SERIALIZATION: parent directory inode locked on entry/exit.
 */

dir_mknod(dp, nmp, mode, ipp, vfsp, crp)
struct inode	*dp;		/* Directory inode		*/
dname_t		*nmp;		/* Directory name struct	*/
int		mode;		/* mode of new file		*/
struct inode	**ipp;		/* return inode			*/
struct vfs	*vfsp;		/* vfs pointer			*/
struct ucred	*crp;		/* pointer to credential structure */
{
	int rc = 0;			/* Return code			*/
	struct	slot slot, *slp; 	/* Directory slot info		*/
	struct inode *ip;
	struct ucred *ucp = NULL;

	*ipp = NULL;

	slp = &slot;		
	if (rc = dir_find(dp, nmp, slp, NULL, crp))
		return rc;

	/* dir_ialloc() returns new, locked inode */
	if ((rc = dir_ialloc(dp, mode, &ip, vfsp, crp)) == 0)
	{
		/* Page fault quota allocation check looks at uid but NFS
	 	 * server which is mono threaded always runs as uid zero.
	 	 */
		if (U.U_procp->p_threadcount == 1 && crp->cr_uid != U.U_uid)
		{
			ucp = U.U_cred;
			U.U_cred = crp;
		}

		if ((rc = dir_makentry(dp, nmp, ip->i_number, slp)) == 0)
			*ipp = ip;
		else
		{
			/* discard the new inode
			 */
			ip->i_nlink = 0;
			IWRITE_UNLOCK(ip);
			ICACHE_LOCK();
			iput(ip, vfsp);
			ICACHE_UNLOCK();
		}
		if (ucp)
			U.U_cred = ucp;
	}

	/* restore virtual address space */
	ipundo(slp->sl_addr);

	return rc;
}

/*
 * NAME:	dir_valid(dp, parentino, check4empty, *dots)
 *
 * FUNCTION:	Validate a directory for remove and rename operations. 
 *		The status of both dot and dot dot is returned in *dots 
 *		parameter.
 *
 * PARAMETERS:	
 *	struct inode	*dp		- Directory inode
 *	ino_t 		parentino 	- Real parent's ino
 *	int		check4empty	- If non zero then verify that the
 *					directory is empty.
 *	int		*dots		- return the status of the '.'
 *					and '..' entries; caller is required
 *					to initialize to zero.
 *
 * RETURN VALUE:	
 *	return 0 if dp is a directory and all other check pass.
 *	return EEXIST if check4empty and dp is not empty. 
 *
 * NOTE: the caller is responsible for locking, unlocking the directory.
 */

dir_valid (struct inode	*dp,
	   ino_t	*parentino,
	   int		check4empty,
	   int		*dots)
{
	int 	rc = 0;		/* Return code			*/
	direct_t *dirp;		/* Mapped directory address	*/
	direct_t *dlast;	/* Directory bounds		*/
	label_t   jb;     	/* setjmpx/longjmpx buffer 	*/

	/* dp must be a directory */
	if ((dp->i_mode & IFMT) != IFDIR || dp->i_nlink < 1)
		return ENOTDIR;

	/* Any directories?	*/
	if (check4empty && dp->i_nlink > 2)
		return  EEXIST;

	/* set up to search a directory */

	if (rc = iptovaddr(dp, 0, &dirp))
		return rc;

	/* establish exception return point for 
	 * permanent io errors. errors may occur as result of loads
	 * or stores into memory mapped directory.
	 */
	if (rc = setjmpx(&jb))
	{	
		ipundo(dirp);
		rc |= PFS_EXCEPTION;
		return rc;
	}

	/* Search for any name other than "." or ".." */

	for (dlast = (direct_t *)((caddr_t)dirp + dp->i_size);
	     dirp < dlast;
	     dirp = (direct_t *) ((caddr_t) dirp + dirp->d_reclen))
	{
		if (dirp->d_ino != 0)
		{
			/* This check is not for dup . or .. entries;
			 * its purpose is to record the status of . and ..;
			 * the caller decides how to handle . and .. anomalies.
			 */
			if (ISDOTS(dirp->d_name))
			{
				if (dirp->d_namlen == 2)
				{
					*dots |= DDOT_EXIST | 
				((dirp->d_ino == parentino) ? DDOT_VALID : 0);  
					continue;
				}
			 	if (dirp->d_namlen == 1) 
				{
					*dots |= DOT_EXIST | 
				((dirp->d_ino == dp->i_number) ? DOT_VALID : 0);
					continue;
				}
	
				rc = EINVAL;
				goto out;
			
			}
			if (check4empty)
			{	
				rc = EEXIST;
				goto out;
			}
		}
	}

out:
	clrjmpx(&jb);			/* pop exception return */
	ipundo(dirp);			/* restore virtual address space */

	return rc;
}

/*
 * NAME:	dir_mkdir(dp, nmp, mode, vfsp, crp)
 *
 * FUNCTION:	create a directory in <dp> with specified name and mode/type.
 * 		specifically:
 *			find entry if it exists
 *			if not, allocate an inode and write entry
 *
 * PARAMETERS:	Dp	- directory inode
 *		nmp	- name struct, ie string and length
 *		mode	- rwx mode
 *		vfsp	- vfs pointer
 *		crp	- credential
 *
 * RETURN:	Zero is returned if completes sucessfully.
 *		An error code (errno) is returned to indicate
 *		failure.
 *
 * EXCEPTION:	exception on new directory is cleaned up by discarding
 *		the new directory inode.
 *		exception on parent directory cleans up transaction resources on 
 *		parent directory and new directory (in dir_makentry()). 
 *
 * SERIALIZATION: parent directory inode locked on entry/exit.
 */

dir_mkdir(dp, nmp, mode, ipp, vfsp, crp)
struct inode	*dp;		/* Directory inode		*/
dname_t		*nmp;		/* Directory name struct	*/
int		mode;		/* mode of new directory	*/
struct inode	**ipp;		/* Directory inode created	*/
struct vfs	*vfsp;		/* directory vfs pointer	*/
struct ucred	*crp;		/* pointer to credential structure */
{
	int rc = 0;			/* Return code			*/
	direct_t *dirp;			/* Mapped directory address	*/
	struct inode *ip = NULL;	/* temp inode 			*/
	dname_t	dnmp;			/* Name space			*/
	struct	slot slot, *slp; 	/* Directory slot info		*/
	struct edir *edp = NULL;	/* Empty directory pointer	*/
	int	type = 0;		/* allocation type */
	label_t   jb;      		/* setjmpx/longjmpx buffer 	*/
	struct ucred *ucp = NULL;

	*ipp = NULL;

	/* check for link count overflow on parent dir */
	if (dp->i_nlink >= LINK_MAX)
		return EMLINK;

	slp = &slot;		
	if (rc = dir_find(dp, nmp, slp, NULL, crp))
		return rc;

	dirp = slp->sl_addr;

	/* allocate inode for new/child directory:
	 * dir_ialloc() returns new, locked inode
	 */
	if (rc = dir_ialloc(dp, mode, &ip, vfsp, crp))
		goto out;
	
	/* Bind new directory to a virtual memory address
	 */
	if ((rc = iptovaddr(ip, 1, &edp)) == 0)
	{	
	        /* Page fault quota allocation check looks at uid but NFS
         	 * server which is mono threaded always runs as uid zero.
         	 */
        	if (U.U_procp->p_threadcount == 1 && crp->cr_uid != U.U_uid)
        	{
                	ucp = U.U_cred;
                	U.U_cred = crp;
        	}

		/* establish exception return point for i/o errors 
		 * on memory mapped new directory.
 		 * all resources associated with new directory is 
		 * cleaned up when the new directory inode is discarded
		 * on failure.
		 */
		if (rc = setjmpx(&jb))
		{	
			rc |= PFS_EXCEPTION;
			transactionfail(dp->i_ipmnt, 0);
			goto out;
		}

		/* allocate fragments for the new directory if
		 * needed.
		 */
		if (rc = vm_dirfrag(ip,DIRBLKSIZ,&type))
		{
			clrjmpx(&jb);
			transactionfail(dp->i_ipmnt, 0);
			goto out;
		}

		/* Fixup prototype directory and copy 
		 * Structure assign. all "stores" should happen before 
		 * serious changes are made to the incore inode so
		 * error paths work properly.	jar
		 */

		/* get transaction lock on the new directory
		 */
		vm_gettlock(edp, sizeof(struct edir));

		*edp = edir;
		edp->d_ino = ip->i_number;
		edp->dd_ino = dp->i_number;

		clrjmpx(&jb);			/* pop exception return */

		/* make entry for new directory in parent directory
		 * (exception on parent directory has cleaned up
		 * transaction resources)
		 */
		if (rc = dir_makentry(dp, nmp, ip->i_number, slp))
			goto out;

		ip->i_size = DIRBLKSIZ;
		ip->i_nlink++;
		dp->i_nlink++;

		/* dp imark()'d in dir_makentry() */
		imark(ip, ICHG|IFSYNC);	

		if (ucp)
			U.U_cred = ucp;

		ipundo(dirp);
		ipundo(edp);
		*ipp = ip;
		return 0;
	}

out:
	if (ucp)
		U.U_cred = ucp;

	/* map out parent directory */
	ipundo(dirp);

	/* discard new directory */
	if (ip)
	{	
		/* map out new directory */
		if (edp)
			ipundo(edp);

		/* discard new directory inode */
		ip->i_nlink = 0;
		IWRITE_UNLOCK(ip);
		ICACHE_LOCK();
		iput(ip, vfsp);
		ICACHE_UNLOCK();
	}

	return rc;
}


/*
 * NAME:	dir_del_access(dp, ip, crp)
 *
 * FUNCTION:    Check that invoker can delete <ip> from <dp>.
 *              Specifically, check for <dp> being a sticky directory
 *              in which case, to be able to delete <ip> one must
 *              own either <dp> or <ip> or have authority to bypass
 *              write checking.
 *
 * PARAMETERS:	dp	- directory inode
 *		ip	- inode to unlink
 *		crp	- credential
 *
 * NOTES:	This test is in a function by itself rather than
 *		simply being coded into dir_delete because jfs_rename
 *		avoids complicated error recovery by checking
 *		permissions before beginning to move links around.
 *		This function encapsulates the test so jfs_rename can
 *		check without knowing the details.
 *
 * RETURNS:	0	if deleting is ok
 *              EPERM	otherwise
 */

dir_del_access(dp, ip, crp)
struct inode *dp;		/* parent directory inode */
struct inode *ip;		/* inode we'd like to unlink */
struct ucred *crp;		/* cred pointer */
{
	int rc;

	/* Let them delete it if they have BYPASS_DAC_WRITE privilege.
	 * (However, note that if they have SET_OBJ_DAC privilege
	 * they could just clear the sticky bit, delete ip,
	 * and restore the sticky bit.)
	 * Recall privcheck returns a (non-zero) error code if one
	 * _lacks_ the privilege in question.
	 */
	if ((dp->i_mode & S_ISVTX) && (crp->cr_uid != ip->i_uid) && 
	     (crp->cr_uid != dp->i_uid) && privcheck_cr(BYPASS_DAC_WRITE, crp))
		return EPERM;
	return 0;
}


/*
 * NAME:	dir_delete(dp, nmp, ip, crp)
 *
 * FUNCTION:	remove the entry in <dp> for <name>.
 * 		this name must refer to <ip>.
 * 		specifically:
 *			overwrite the directory entry in <dp>
 *			decrement the link count on <ip>
 *
 *
 * PARAMETERS:	Dp	- directory inode
 *		nmp	- name struct, ie string and length
 *		ip	- inode to link delete
 *		crp	- credential
 *
 * RETURN VALUE:	If the routine fails an error number(errno) is 
 *			returned indicating failure.
 *
 * SERIALIZATION: parent directory inode and child inode locked on entry/exit.
 */

dir_delete(dp, nmp, ip, crp)
struct inode	*dp;		/* Directory inode		*/
dname_t		*nmp;		/* Directory name struct	*/
struct inode	*ip;		/* inode for name		*/
struct ucred	*crp;		/* pointer to credential structure */
{
	int rc = 0;			/* Return code			*/
	struct	slot slot, *slp;	/* Returned slot info		*/
	
	if (rc = iaccess(dp, IWRITE|IEXEC, crp))
		return rc;

	if (rc = dir_del_access(dp, ip, crp))
		return rc;

#ifdef SVVS
	/* SVVS requires that we fail to remove the last link
	 * to a file open for execute
	 */
	if (ip->i_nlink == 1 && iaccess(ip, IWRITE, crp) == ETXTBSY)
		return ETXTBSY;
#endif /* SVVS */
	
	slp = &slot;
	if (rc = dir_search(dp, nmp, slp, ip->i_number))
		return rc;

	/* update the directory */
	if ((rc = dir_rmentry(dp, ip, nmp, slp)) == 0)
	{
		ip->i_nlink--;

		/* imark with ICHG if it was a directory with no links.
		 */
		if (ip->i_nlink <= 2 && (ip->i_mode & IFMT) == IFDIR)
			imark(ip, IFSYNC|ICHG);		
		else
			imark(ip, ICHG);
	}

	/* restore virtual address space */
	ipundo(slp->sl_addr);	

	return rc;
}


/*
 * NAME:	dir_fixparent (dp, oi, ni)
 *
 * FUNCTION:	Fix dotdot entries for jfs_rename()
 *
 * PARAMETERS:	Dp	- directory inode
 *		oi	- old inode for .. entry in dp
 *		ni	- new inode for .. entry in dp
 *
 * RETURN:	If the routine fails an error number(errno) is 
 *		returned indicating failure.
 *
 * EXCEPTION:	caller (jfs_rename()) panics on failure !
 */

dir_fixparent(dp, oi, ni)
struct inode	*dp;			/* Directory inode		*/
struct inode	*oi;			/* old parent inode 		*/
struct inode	*ni;			/* new parent inode		*/
{
	int rc = 0;			/* Return code			*/
	direct_t *targ;			/* Returned directory struct	*/
	ino_t tino;			/* target inode # 		*/
	struct	slot slot, *slp;	/* Returned slot info		*/
	label_t   jb;      		/* setjmpx/longjmpx buffer 	*/
	
	slp = &slot;
	
	/* Access perms already checked by caller
	 */
	if (rc = dir_search(dp, &dotdot, slp, oi->i_number))
		return rc;

	targ = slp->sl_addr;

	/* establish exception return point for 
	* permanent io errors. errors may occur as result of loads
	* or stores into memory mapped directory.
	*/
	if (rc = setjmpx(&jb))
	{	
		rc |= PFS_EXCEPTION;
		goto out;
	}

	/* get transaction lock on i_no field of the entry */
	vm_gettlock(&targ->d_ino, sizeof(targ->d_ino));

	targ->d_ino = ni->i_number;

	clrjmpx(&jb);	/* pop exception return */

	oi->i_nlink--;
	imark(oi, ICHG);

	ni->i_nlink++;
	imark(ni, ICHG);

	imark(dp, IUPD|IFSYNC);

out:
	/* restore virtual address space */
	ipundo(slp->sl_addr);

	return rc;
}


/*
 * NAME:	dir_ialloc (pip, mode, ipp, vfsp, crp)
 *
 * FUNCTION:	Allocate new inode for file in directory
 *
 * PARAMETERS:	pip	- parent inode pointer
 *		mode	- rwx and IFMT mode
 *		ipp	- Returned inode
 *		vfsp	- vfs pointer
 *		crp	- credential
 *
 * RETURN:	0	- new, locked inode
 *		EIO	- I/O error on dquot (from subroutine)
 *		EDQUOT	- over disk inode quota (from subroutine)
 */

dir_ialloc(pip, mode, ipp, vfsp, crp)
struct inode	*pip;
mode_t		mode;
struct inode	**ipp;
struct vfs	*vfsp;
struct ucred	*crp;		/* pointer to credential structure */
{
	int rc;
	uid_t uid;
	gid_t gid;

	uid = crp->cr_uid;
	gid = crp->cr_gid;

	/* allocate new inode:
	 * dev_ialloc() returns new, locked inode
	 */
	if ((mode & IFMT) == IFDIR)
	{
		/* try to allocate from new allocation group
		 * (near i_number = 0)
	 	 */
		if (rc = dev_ialloc(pip, 0, mode, vfsp, ipp))
			return rc;

		if (pip->i_mode & ISGID)
			/* inherit ISGID from parent directory */
			(*ipp)->i_mode |= ISGID;
	}
	else
		/* try to allocate from parent's allocation group
		 * (near i_number = parent i_number)
		 */
		if (rc = dev_ialloc(pip, pip->i_number, mode, vfsp, ipp))
			return rc;

	(*ipp)->i_uid = uid;
	(*ipp)->i_gid = (pip->i_mode & ISGID) ? pip->i_gid : gid;

	/* check the disk inode quota for the inode.
	 */
	ICACHE_LOCK();
	rc = getinoquota(*ipp);
	if (rc == 0)
	{
		ICACHE_UNLOCK();
		if ((rc = allociq(*ipp, 1, crp)) == 0)
			return 0;
		ICACHE_LOCK();
	}

	/* put the dquots.
	 */
	putinoquota(*ipp);

	/* discard the new inode
	 */
	(*ipp)->i_nlink = 0;
	IWRITE_UNLOCK(*ipp);
	iput(*ipp, vfsp);
	ICACHE_UNLOCK();
	*ipp = NULL;

	return rc;
}


/*
 *	dir_makentry(dp, nmp, ino, slp)
 *
 * make a directory entry.
 *
 * EXCEPTION: exception on mapped directory cleans up transaction
 *		resources.
 */
static
dir_makentry(dp, nmp, ino, slp)
struct inode *dp;			/* Directory inode		*/
dname_t *nmp;				/* Name info			*/
ino_t	ino;				/* Inode #			*/
struct	slot *slp;			/* Slot info			*/
{
	int		rc;
	direct_t	*dirp, *ndirp;	
	caddr_t		dirbuf;
	int		newsize, dsize, loc;
	volatile	update = 0,	/* update state */
			type = 0,	/* allocation type */
			oldnblocks,
			oldsize;
	int		oldfrag = 0, 
			pno;	
	struct inode	tip;
	label_t		jb;		/* setjmpx/longjmpx buffer */

	/* establish exception return point for io errors
	 * on memory mapped directory.
	 */
	if (rc = setjmpx(&jb))
	{	
		rc |= PFS_EXCEPTION;

		/* release new allocations
		 */
		if (type != 0 && dp->i_nblocks != oldnblocks)
		{
			/* save oldfrag */
			if (dp->i_movedfrag)
			{
				pno = BTOPN(oldsize);
				oldfrag = dp->i_movedfrag->olddisk[pno & 0x03];
			}

			/* free new allocation */
			tip.i_nblocks = oldnblocks;
			if (setjmpx(&jb) == 0)
			{
				ifreenew(dp, &tip);
				clrjmpx(&jb);
			}

			/* restore oldfrag */
			if (oldfrag)
				dp->i_rdaddr[pno] = oldfrag;

			dp->i_size = oldsize;
		}

		/* for EIO on old page from extension memory, 
		 * mark file system dirty
		 */
		if (update == 1)
			transactionfail(dp->i_ipmnt, 1);
		else
			transactionfail(dp->i_ipmnt, 0);

		return rc;
	}

	/* new entry is to be put at, in the freespace of existing entry 
	 * at, or in the compacted free space of region at slp->sl_addr.
	 */
	dirp = slp->sl_addr;

	switch (slp->sl_status)
	{
	case NONE:
		/* No space in the directory. 
		 * slp->sl_addr will be on a directory block boundary and 
		 * we will write the new entry into a fresh directory block.  
		 * if a fragment allocation is required, vm_dirfrag() will 
		 * perform the allocation. otherwise, a full block will be 
		 * allocated under the page fault handler.
		 *
		 * NOTE: when mapping into a new block(page) the inode size
		 * is incremented to the end of the new disk allocation by
		 * the fragment allocator or the page fault handler.  we
		 * must remember our new inode size and adjust the inode
		 * with this value after the disk allocation has occurred.
		 */

		/* allocate fragments for the new directory block
		 * if needed.
		 * case of new entry allocation type for exception handling:
		 * 0. in-place allocation in old page (type = 0),
		 * 1. new fragment allocation in old page (type == 1)
		 *    by vm_dirfrag() (with dp->i_movedfrag != 0),
		 * 2. new page allocation (type = 2) by vm_gettlock() or 
		 *    new fragment allocation in new page by vm_dirfrag().
		 */ 
		oldsize = dp->i_size;
		oldnblocks = dp->i_nblocks;
		newsize = dp->i_size + DIRBLKSIZ;
		if (rc = vm_dirfrag(dp,newsize,&type))
		{
			clrjmpx(&jb);			
			transactionfail(dp->i_ipmnt, 0);
			return rc;
		}

		/* vm_dirfrag() marks the old page as not home ok
		 * when fragment allocated in old page, i.e.,
		 * vm_gettlock() may pagein EIO from extension memory
		 */ 
		if (type == 1)
			update = 1;

		/* get transaction lock on the new directory entry
		 */	
		dsize = LDIRSIZE(nmp->nmlen);
		vm_gettlock(dirp, dsize);

		/* update exception: pagein EIO from extension memory 
		 * for old page will mark file system dirty as logage 
		 * of possibly hot spot page is lost in log synchronization 
		 * when current transaction is aborted.
		 */
		if (type == 0)
			update = 1;

		/* update directory */
		dirp->d_reclen = DIRBLKSIZ;
		dp->i_size = newsize;
		break;

	case FOUND:
		/* Found space for the new entry in a single existing
		 * entry in the range sl_addr to sl_addr + sl_size
		 * with free space of sl_free.
		 * (if d_ino = 0, sl_free = d_reclen)
		 */

	case COMPACT:
		/* Found space for the new entry from multiple consecutive 
		 * existing entries in the range sl_addr to sl_addr + sl_size
		 * with free space of sl_free. 
		 * To use this space, compact the entries by moving them 
		 * together towards the beginning of the block, 
		 * leaving the free space in one usable chunk at the end.
		 */
		dirbuf = (caddr_t)dirp;
		dsize = LDIRSIZE (dirp->d_namlen);
		loc = dirp->d_reclen;

		/* get transaction lock on the directory region updated */
		vm_gettlock(dirp, slp->sl_size - slp->sl_free + LDIRSIZE(nmp->nmlen));

		/* in-place allocation of entry: vm_gettlock()
		 * has marked the page not homeok on success, 
		 * i.e., further exceptions are from pagein EIO
		 * from extension memory.
		 */
		/* update exception: pagein EIO from extension memory 
		 * for old page will mark file system dirty as logage 
		 * of possibly hot spot page is lost in log synchronization 
		 * when current transaction is aborted.
		 */
		update = 1;

		/* compact the directory entries */
		while (loc < slp->sl_size) {
			ndirp = (struct direct *)(dirbuf + loc); /* next entry */
			if (dirp->d_ino) {
				/* trim the current entry and 
				 * determine new location of next entry
				 */
				dirp->d_reclen = dsize;
				dirp = (struct direct *)((char *)dirp + dsize);
			}

			dsize = LDIRSIZE(ndirp->d_namlen);
			loc += ndirp->d_reclen;
			bcopy(ndirp, dirp, dsize);
		}

		/* At this point, dirp is the last entry in the range
		 * sl_addr to sl_addr + sl_size.
		 * freespace is the now unallocated space after the
		 * dirp entry that resulted from moving entries above.
		 */
		/* Update the pointer fields in the previous entry (if any). */
		if (dirp->d_ino) {	
			dirp->d_reclen = dsize;
			dirp = (struct direct *)((char *)dirp + dsize);
		}

		/* initialize new entry */
		dirp->d_reclen = slp->sl_free;
		dsize = LDIRSIZE(nmp->nmlen);
		break;

	default:
		panic("dir_makentry: invalid slot status");
	}

	/* finalize the new entry */
	dirp->d_ino = ino;
	dirp->d_namlen = nmp->nmlen;
	bcopy (nmp->nm, dirp->d_name, nmp->nmlen);
	for (dirbuf = dirp->d_name + nmp->nmlen; dirbuf < (caddr_t)dirp + dsize; dirbuf++) 
		*dirbuf = '\0';

	/* pop exception return */
	clrjmpx(&jb);			

	imark(dp, ICHG|IUPD|IFSYNC);	/* ichg=>posix */

	/* Add entry to directory cache. */
	dnlc_enter(dp->i_dev, dp->i_number, nmp, ino);

	return 0;
}


/*
 *	dir_rmentry (dp, ip, nmp, slp)
 *
 * remove a directory entry
 *
 * EXCEPTION: exception on mapped directory cleans up transaction
 *		resources.
 */
static
dir_rmentry (dp, ip, nmp, slp)
struct inode *dp;
struct inode *ip;
dname_t *nmp;
struct	slot *slp;			/* slot info			*/
{
	int rc;				/* return code 			*/
	direct_t *dirp;			/* mapped directory address	*/
	volatile	update = 0;
	label_t		jb;		/* setjmpx/longjmpx buffer */

	/* Purge entry from directory cache.  If inode for name was
	 * a directory bash . and ..
	 * purge cache entries before possible page faults on
	 * directory data
	 */
	dnlc_delete(dp->i_dev, dp->i_number, nmp);

	if ((ip->i_mode & IFMT) == IFDIR)
	{	
		/* purge . from directory cache */
		dnlc_delete(ip->i_dev, ip->i_number, &dot);

		/* purge .. from directory cache */
		dnlc_delete(ip->i_dev, ip->i_number, &dotdot);
	}

	/* establish exception return point for io errors
	 * on memory mapped directory.
	 */
	if (rc = setjmpx(&jb))
	{	
		rc |= PFS_EXCEPTION;

		/* for EIO on old page from extension memory, 
		 * mark file system dirty
		 */
		if (update == 1)
			transactionfail(dp->i_ipmnt, 1);
		else
			transactionfail(dp->i_ipmnt, 0);

		return rc;
	}

	/* remove entry from directory
	 */
	/* in-place removal of entry: vm_gettlock()
	 * has marked the page not homeok on success, 
	 * i.e., further exceptions are from pagein EIO
	 * from extension memory.
	 */
	/* update exception: pagein EIO from extension memory 
	 * for old page will mark file system dirty as logage 
	 * of possibly hot spot page is lost in log synchronization 
	 * when current transaction is aborted.
	 */
	if (ONBOUNDARY(slp->sl_addr)) {
		/*
		 * First entry in block: set d_ino to zero.
	 	 */
		/* get transaction lock on d_ino field of the entry */
		vm_gettlock(&(slp->sl_addr->d_ino), sizeof(slp->sl_addr->d_ino));

		/* update directory */
		update = 1;
		slp->sl_addr->d_ino = 0;
	} else {	
		/*
		 * Collapse new free space into previous entry.
	 	 */
		dirp = (struct direct *) ((caddr_t)slp->sl_addr - slp->sl_size);

		/* get transaction lock on d_reclen field of the previous entry */
		vm_gettlock(&dirp->d_reclen, sizeof(dirp->d_reclen));

		/* update directory */
		update = 1;
		dirp->d_reclen += slp->sl_addr->d_reclen;
	}

	clrjmpx (&jb);			/* pop exception return */

	imark(dp, ICHG|IUPD|IFSYNC);	/* ichg=>posix */

	return 0;
}


/*
 * NAME: dirmangled(dirp)
 *
 * FUNCTION: Directory consistency checking:  Checks include
 *		record length must be multiple of 4
 *		record length must not be zero
 *		entry must fit in rest of this DIRBLKSIZ block
 *		record must be large enough to contain name.
 *		name is not longer than MAXNAMLEN
 *
 * RETURNS: void
 */
static void 
dirmangled(direct_t *dirp)
{
	register int i;
	char s[100];
	
        i = DIRBLKSIZ - ((int)dirp & (DIRBLKSIZ - 1));
        if ((dirp->d_reclen & (DIROUND-1)) || (dirp->d_reclen == 0) ||
	    (dirp->d_reclen > i) || (LDIRSIZE(dirp->d_namlen) > dirp->d_reclen)
	    || (dirp->d_namlen > MAXNAMLEN))
	{
                sprintf(s,"dir_find: reclen is %d space is %d namelen is %d",
			dirp->d_reclen,i,dirp->d_namlen);
                panic(s);
	}
}
