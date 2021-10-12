static char sccsid[] = "@(#)22	1.15  src/bos/kernext/cfs/cdr_subs.c, sysxcfs, bos411, 9428A410j 5/6/94 12:14:01";
/*
 * COMPONENT_NAME: (SYSXCFS) CDROM File System
 *
 * FUNCTIONS: CDRFS cdrnode magagement, etc. 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/param.h>
#include <sys/errno.h>
#include <unistd.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <sys/inode.h>			/* should be <jfs/inode.h> */
#include <sys/user.h>
#include <sys/id.h>
#include  "vmm/vmsys.h"	/* should be in <sys/vmsys.h> */
#include <sys/sleep.h>
#include <sys/pri.h>
#include <sys/sysinfo.h>
#include <sys/vfs.h>
#include <sys/vmount.h>
#include <sys/malloc.h>
#include <sys/syspest.h>
#include <sys/trchkid.h>
#include <sys/lockl.h>
#include <sys/limits.h>

/* CD-ROM file system includes */
#include "cdr_xcdr.h"
#include "cdr_rrg.h"
#include "cdr_cdrfs.h"
#include "cdr_cdrnode.h"

int	maxcdrcachesize;	/* max # of cdrnodes allowed in cache	*/
int	mincdrcachesize;	/* min desired # of cdrnodes in cache	*/
int	cdrcachesize;		/* number cdrnodes currently in cache	*/

/* cdrnode cache list - This is a circular doubly-linked list of
 *	cdrnodes.  insque2() and remque2() are used to add cdrnodes to
 *	and delete cdrnodes from this list.  The offsets of ic_next and
 *	ic_prev in this structure must be same as those in the cdrnode.
 *	The cdrnodes at the front of this list (referenced via ic_next)
 *	are the least recently used cdrnodes, while those at the tail
 *	of this list (referenced via ic_prev) are the most recently
 *	used.  Invalid cdrnodes are placed at the front of this list
 * 	so they will be re-used before any valid (reclaimable
 *	cdrnodes.
 *	insque2(cdrnode1, cdrnode2) - This adds cdrnode1 to
 *		doubly-linked list in the second pair of pointers in
 *		structure.  This inserts cdrnode1 after cdrnode2 in the
 *		list.  insque2(cdrp, cdrcachelist) inserts cdrp at the
 *		head of the cache list, while
 *		insque2(cdrp, cdrcachelist.ic_prev) inserts cdrp at the
 *		tail of the cache list.
 *	remque2(cdrnode) - This removes cdrnode from doubly-linked list
 *		in second pair of pointers in structure.
 */
struct
{
	struct cdrnode *ic_forw;	/* hash list offsets, not used	*/
	struct cdrnode *ic_back;
	struct cdrnode *ic_next;	/* head of cache list		*/
	struct cdrnode *ic_prev;	/* tail of cache list		*/
} cdrcachelist;

/* cdrnode hash list - This is a circular doubly-linked list of
 *	cdrnodes.  insque() and remque() are used to add cdrnodes to
 *	and delete cdrnodes from this list.  The offsets of hcdr_forw
 *	and hcdr_back in this structure must be same as those in the
 *	cdrnode.  When a cdrnode is not on a hash list (an invalid
 *	cdrnode), these pointers in the structure are set to NULL.
 *	insque(cdrnode1, cdrnode2) - This adds cdrnode1 to the
 *		doubly-linked list in the first pair of pointers in
 *		the structure.  This inserts cdrnode1 after cdrnode2 in
 *		the list.  insque2(cdrp, cdrcachelist) inserts cdrp at
 *		the head of the hash list, while
 *		insque2(cdrp, cdrcachelist.ic_prev) inserts cdrp at the
 *		tail of the hash list.
 *	remque(cdrnode) - This removes cdrnode from the doubly-linked
 *		list in the first pair of pointers in the structure.
 */
struct hcdrnode {	
	struct cdrnode *	hcdr_forw;	/* head of hash list	*/
	struct cdrnode *	hcdr_back;	/* tail of hash list	*/
};

/* The number of hash buckets must be a power of two as the hash
 * function subtracts one from it to generate a mask.  We allocate
 * enough space for one page of hash list headers (hash buckets).  This
 * is currently 512, and maxcdrcachesize is 256, so we will normally
 * have very few cdrnodes per hash list.
 */
#define NHCDRNO			PAGESIZE/sizeof(struct hcdrnode)
struct hcdrnode hcdrnode[NHCDRNO];	/* hash table */

/* CDRHASH() is the hash function to generate a hash bucket pointer
 * from a dirent number.  This should be a fast function.  NULL_CACHE
 * is used to set the cache list pointers when a cdrnode is not on the
 * cache list.
 */
#define	CDRHASH(dirent)		(&hcdrnode[(int)((dirent & 0x1ff)?	\
				dirent : dirent >> 9) & (NHCDRNO-1)])
#define	NULL_CACHE(cdrp)	((cdrp)->cn_next = (cdrp)->cn_prev = (cdrp))

/* This is the stale cdrnode that vnodes from forced unmounted filesystems
 * will reference.
 */
struct cdrnode stale_cdrnode;	/* stale cdrnode for unmounted vnodes	*/

extern time_t		time;	/* current time for marking cdrnode */

/* declarations for internally used functions */
int
cdrput (	struct cdrnode *	cdrp);
void
cdrunhash (	struct cdrnode *	cdrp);
void
cdrclose (	struct cdrnode *	cdrp);
void
cdrclear(	struct cdrnode *	cdrp);
struct cdrnode *
cdrfind (dev_t	dev, daddr_t	dirent);

/* debugger level variable declaration to control BUGLPR() macro */
BUGXDEF(buglevel)

/*
 * NAME:	cdrnoinit()
 *
 * FUNCTION:	This function initializes the cdrnode hash table and
 *		cache list.
 *
 * PARAMETERS:	none
 *
 * RETURN :	0	- success
 *
 * SERIALIZATION: The cdrfs lock is held when this function is called
 *		  from the cdr_init() routine.
 */
int
cdrnoinit (void)
{
	struct hcdrnode *	hcdrp;		/* hash list header	*/
	extern struct vnodeops	stale_cdr_vops;	/* stale vnode ops	*/

	/* Limit maximum cdrnode cache size to initial allocation */
	maxcdrcachesize = 256;
	mincdrcachesize = maxcdrcachesize / 2;

	/* initialize the hash table */
	for (hcdrp = hcdrnode; hcdrp < &hcdrnode[NHCDRNO]; hcdrp++)
	{
		hcdrp->hcdr_forw = (struct cdrnode *)hcdrp;
		hcdrp->hcdr_back = (struct cdrnode *)hcdrp;
	}

	/* null cache cdrcachelist */
	cdrcachesize = 0;
	cdrcachelist.ic_next = (struct cdrnode *) &cdrcachelist;
	cdrcachelist.ic_prev = (struct cdrnode *) &cdrcachelist;

	/* Initialize the vnode operations of the stale cdrnode's gnode
	 * so that stale vnodes will get the correct vnode operations.
	 * Clear the stale cdrnode's gnode's vnode list so that vn_free()
	 * can have a vnode list on the gnode to remove the vnode from.
	 * Make cdrnode have a count of zero so we can tell it from
	 * valid cdrnodes which need to be put in cdr_rele().
	 */
	stale_cdrnode.cn_count = 0;
	stale_cdrnode.cn_gnode.gn_ops = &stale_cdr_vops;
	stale_cdrnode.cn_gnode.gn_vnode = NULL;
	stale_cdrnode.cn_gnode.gn_data = (caddr_t)&stale_cdrnode;

	return 0;
}

/*
 * NAME:	cdrnoterm()
 *
 * FUNCTION:	This function is called by cdr_config()
 *		to cleanup the cdrnode hash table and cache list.
 *
 * PARAMETERS:	none
 *
 * RETURN :	none
 *
 */
void
cdrnoterm (void)
{
	struct hcdrnode *	hcdrp;	/* head of hash list	*/
	struct cdrnode *	cdrp;	/* cdrnode in hash list	*/
	struct cdrnode *	next;	/* next cdrnode in list	*/

	/* remove all cdrnode from hash list */
	for (hcdrp = hcdrnode; hcdrp < &hcdrnode[NHCDRNO]; hcdrp++)
	{
		cdrp = hcdrp->hcdr_forw;
		while (cdrp != (struct cdrnode *)hcdrp)
		{
			ASSERT(cdrp->cn_count == 0);
			ASSERT(cdrp->cn_xdlist == NULL);
			next = cdrp->cn_forw;
			remque(cdrp);
			free(cdrp);
			cdrp = next;
		}
		ASSERT(hcdrp->hcdr_forw == (struct cdrnode *)hcdrp);
		ASSERT(hcdrp->hcdr_back == (struct cdrnode *)hcdrp);
	}

	/* null cache cdrcachelist */
	cdrcachesize = 0;
	cdrcachelist.ic_next = (struct cdrnode *) &cdrcachelist;
	cdrcachelist.ic_prev = (struct cdrnode *) &cdrcachelist;
}

/*
 * NAME:	cdrget(dev, dirent, pdirent, cdrpp)
 *
 * FUNCTION:	This function looks up a cdrnode by device, directory
 *		entry location, and parent cdrnode's directory entry
 *		location.  This function sets *cdrpp to the address of
 *		the cdrnode found or created.  If the cdrnode is not
 *		found in the cdrnode table, it is created using data read
 *		from the specified device.  The cdrnode count is
 *		incremented, but the cdrnode is not locked when returned.
 *		If an error occurs, *cdrpp is left unchanged.
 *
 * PARAMETERS:	dev	- device of desired cdrnode
 *		dirent	- location of file's (first) directory entry
 *		pdirent	- location of parent directory's directory entry
 *		cdrpp	- address for return of cdrnode pointer
 *
 * RETURN :	0	- success
 *		ENFILE	- if out of cdrnodes
 *		errors from subroutines
 *
 * SERIALIZATION: The cdrfs lock is held across the entire call of this
 *		  function.
 */
int
cdrget (
	dev_t			dev,		/* dev num of cdrnode	*/
	daddr_t			dirent,		/* cdrnode's dirent num	*/
	daddr_t			pdirent,	/* parent's dirent num	*/
	struct cdrnode **	cdrpp)		/* ptr for cdrnode	*/
{ 
	int	rc = 0;		/* return code */
	struct hcdrnode *	hcdrp;		/* hash table header	*/
	struct cdrnode *	cdrp;		/* cdrnode gotten	*/
	struct cdrnode	*mntcdrp;	/* mount cdrnode */
	struct cdrfsmount	*cdrfsp; /* mounted file system data */
	struct gnode *		gp;		/* gnode for cdrnode	*/
	extern struct vnodeops	cdr_vops;	/* cdrnode vnode ops	*/

	ASSERT(lock_mine(&cdrfs_lock));

	BUGLPR(buglevel, 9, ("cdrget:  dirent = 0x%x\n", dirent));
	/* keep statistics */
	sysinfo.iget++;
loop:
	/* get hash list to search */
	hcdrp = CDRHASH(dirent);

	/* search the hash list for this cdrnode */
	for (cdrp = hcdrp->hcdr_forw; cdrp != (struct cdrnode *) hcdrp; cdrp = cdrp->cn_forw)
		if (dirent == cdrp->cn_dirent && dev == cdrp->cn_dev) 
		{
			BUGLPR(buglevel, 9, ("found cdrnode in hash:  dirent = 0x%x\n", cdrp->cn_dirent));
			/* We found the cdrnode.  If IGETLCK is set,
			 * some other process has the cdrnode and it is
			 * not on the cache list, so we increment the
			 * cdrnode count before sleeping to prevent
			 * them from returning the cdrnode to the cache
			 * list.  If IGETLCK is not set and the count
			 * was zero, then we need to remove the cdrnode
			 * from the cache list.  The IGETLCK is held
			 * while a cdrnode is being established (the
			 * cdrread() needed might sleep) and when the
			 * cdrnode is being removed from the cache.
			 */
			cdrp->cn_count++;
			if (cdrp->cn_flag & IGETLCK)
			{
				/* Wakeups could occur for processes
				 * waiting for ILOCK bit on cdrnode,
				 * so we wait for IGETLCK to be
				 * cleared
				 */
				do
				{
					cdrp->cn_flag |= IWANT;
					(void) e_sleep_thread(&cdrp->cn_event,
						              &cdrfs_lock,
						              LOCK_SIMPLE);
					if (cdrp->cn_mode == 0)
					{
						cdrput(cdrp);
						goto loop;
					}
				} while (cdrp->cn_flag & IGETLCK);

				/* We waited for some other process
				 * which had the getlock on the cdrnode
				 * (and thus had the cdrnode off the
				 * cache list already), so just return
				 * it.
				 */
				*cdrpp = cdrp;
				return 0;
			}

			/* If the cdrnode count was zero, it's on the
			 * cached list and must be removed.
			 */
			if (cdrp->cn_count == 1)
			{
				remque2(cdrp);
				NULL_CACHE(cdrp);
				cdrcachesize--;
			}

			*cdrpp = cdrp;
			return 0;
		}
	BUGLPR(buglevel, 9, ("did not find cdrnode in hash:  dirent = 0x%x\n", dirent));

	/* The cdrnode wasn't in the hash list, so we need to get a new
	 * cdrnode, read from the disk, add the data to the cdrnode,
	 * and add the cdrnode to the hash list.
	 */

	/* We call getcdrp() to get a new cdrnode for us.  If getcdrp()
	 * might have slept, it returns 1 and we have to try again
	 * because the cdrnode might now be in the hash.  getcdrp()
	 * returns 0 if it couldn't have slept, and we can proceed to
	 * fill in the cdrnode.  getcdrp() returns ENFILE if no more
	 * cdrnodes are available and none could be created.
	 */
	switch (rc = getcdrp(&cdrp)) 
	{
		case 0:
			BUGLPR(buglevel, 9, ("getcdrp returned 0\n"));
			break;
		case 1:
			/* getcdrp() could have slept releasing the
			 * cdrfs lock.  This would allow another
			 * process to create a valid cdrnode for the
			 * cdrnode we are trying to get,  thus we need
			 * to search the hash again before creating
			 * another cdrnode.  getcdrp() created a
			 * cdrnode on which it won't sleep the next
			 * time, so we should only loop here once.
			 */
			BUGLPR(buglevel, 9, ("getcdrp returned 1\n"));
			goto loop;
		case ENFILE:
			/* This is really a lack of memory error,
			 * rather than a file table overflow error.
			 * Maybe this should return ENOMEM.
			 */
			BUGLPR(buglevel, 9, ("getcdrp returned %d\n", rc));
			return rc;
	}

	BUGLPR(buglevel, 9, ("dirent = 0x%x\n", dirent));
	/* clear out cdrnode */
	bzero(cdrp, sizeof *cdrp);

	/* Connect the cdrnode to the hash list so that it can be found
	 * by cdrget()'s while we are sleeping in cdrread() below.
	 */
	insque(cdrp, hcdrp);
	NULL_CACHE(cdrp);

	/* Fill in enough of the cdrnode to cause cdrget()'s to sleep
	 * waiting for us to release it after filling it in.  These
	 * cdrget()'s can occur while we are sleeping in cdrread()
	 * below.
	 */
	cdrp->cn_dev =		dev;
	cdrp->cn_dirent =	dirent;
	cdrp->cn_pdirent =	pdirent;
	cdrp->cn_count =	1;
	cdrp->cn_flag =		IGETLCK;	/* lock cdrnode */
	cdrp->cn_event =	EVENT_NULL;	/* initialize the event list */

	/* Fill in enough gnode data so that strategy routine can find
	 * cdrnode's directory entry.  cdrptovaddr() can be called
	 * before cdrread() has completely filled in the cdrnode.
	 */
	gp =			CDRTOGP(cdrp);
	gp->gn_ops =		&cdr_vops;
	gp->gn_data =		(caddr_t)cdrp;
	gp->gn_rdev =		cdrp->cn_dev;

	/* read disk cdrnode */
	if (dirent != 0) {
		mntcdrp = (struct cdrnode *) cdrfind(dev, 0);
		cdrfsp = mntcdrp->cn_cdrfs;
		cdrp->cn_cdrfs = cdrfsp;
		if (cdrfsp->fs_format == CDR_ROCKRIDGE)
			rc = cdrread_rrg(cdrp);
		else
			rc = cdrread_iso(cdrp);
	} /* else -  mount cdrnode does not have disk cdrnode */

	/* unlock cdrnode and wakeup sleepers */
	cdrp->cn_flag &= ~IGETLCK;
	if (cdrp->cn_flag & IWANT)
	{
		cdrp->cn_flag &= ~IWANT;
		e_wakeup(&cdrp->cn_event);
	}

	/* check cdrread return code */
	if (rc)
	{
		/* remove the cdrnode from the hash list */
		cdrp->cn_mode = 0;
		remque(cdrp);

		cdrput(cdrp);
		RETURNX(rc, cdr_elist);
	}

	/* copy type from cdrnode to gnode */
	gp->gn_type = IFTOVT(cdrp->cn_mode);
	if (gp->gn_type == VBLK || gp->gn_type == VCHR)
		gp->gn_rdev = cdrp->cn_rdev;
	else
		gp->gn_rdev = cdrp->cn_dev;

	*cdrpp = cdrp;
	return 0;
}

/*
 * NAME:	cdrput(cdrp)
 *
 * FUNCTION:	This function decrements the reference count of
 *		a cdrnode structure.  If the cdrnode was locked
 *		on entry, it is not unlocked on exit.
 *
 * PARAMETERS:	cdrp	- unwanted cdrnode
 *
 * RETURN :	errors from subroutines
 * 
 * SERIALIZATION: This function can only be called with the cdrfs lock held.
 *			
 */
int
cdrput (struct cdrnode *	cdrp)
{
	BUGLPR(buglevel, 9, ("cdrput:  dirent = 0x%x, count = %d\n", cdrp->cn_dirent, cdrp->cn_count));
	assert(cdrp->cn_count > 0);

	/* decrement hold count and return if still held */
	if (--cdrp->cn_count > 0)
		return 0;

	/* fix cdrnode flags and place at end of cachelist */
	cdrp->cn_flag = 0;
	cdrcachesize++;
	insque2(cdrp, cdrcachelist.ic_prev);
	return 0;
}

/*
 * NAME:	cdrfind(dev, dirent)
 *
 * FUNCTION:	This function finds the entry in cdrnode table corresponding
 *		to a device number and directory entry location.  This
 *		procedure should not be called unless it is certain that
 *		the cdrnode table entry exists.  Its intended use is for
 *		finding things like the mount cdrnodes.
 *
 * PARAMETERS:	dev	- device number of desired cdrnode
 *		dirent	- directory entry location of cdrnode
 *
 * RETURN :	none
 *		panics if an appropriate cdrnode could not be found
 *			
 * 
 * SERIALIZATION: This function can only be called with the cdrfs lock held.
 *			
 */
struct cdrnode *
cdrfind (
	dev_t		dev,	/* device number of cdrnode to find	*/
	daddr_t		dirent)	/* dirent number of cdrnode to find	*/
{
	struct cdrnode	*cdrp;	/* pointer to cdrnode in hash table	*/
	struct hcdrnode	*hcdrp;	/* hash table bucket header		*/

	/* get hash list (hash bucket) */
	hcdrp = CDRHASH(dirent);

	/* search the hash list for this cdrnode */
	for (cdrp = hcdrp->hcdr_forw; cdrp != (struct cdrnode *)hcdrp; cdrp = cdrp->cn_forw)
		if (cdrp->cn_dirent == dirent && cdrp->cn_dev == dev) 
			return cdrp;

	/* cdrfind() should only be called for a cdrnode which is known
	 * to be in the hash list.
	 */
	panic("cdrfind: failure");
}

/*
 * NAME:	getcdrp(cdrpp)
 *
 * FUNCTION:	This function allocates a free cdrnode table entry and
 *		sets *cdrpp to its address.  If it can do so without
 *		sleeping, it returns zero.  If getcdrp might have slept
 *		in its execution, *cdrpp is left unchanged and the value 1
 *		is returned (but the next call to getcdrp will succeed).
 *		If no cdrnodes can be allocated, ENFILE is returned.
 *
 * PARAMETERS:	cdrpp	- address for pointer to cdrnode to be returned
 *
 * RETURN :	0	- success
 *		1	- getcdrp() may have slept
 *		ENFILE	- cdrnode table overflow
 * 
 * SERIALIZATION: This function should only be called with the cdrfs lock held.
 *			
 */
int
getcdrp (struct cdrnode **	cdrpp)
{
	struct cdrnode *	cdrp = NULL;	/* cdrnode to return	*/

	/* tentatively choose first cdrnode on cache list, if it exists */
	if (cdrcachesize > 0)
		cdrp = (struct cdrnode *)cdrcachelist.ic_next;

	/* If there are no unhashed cdrnodes on cache list and the
	 * cache list is less than desired size, use malloc to get
	 * new cdrnodes.  This method reclaims worthless cdrnodes from
	 * the cache list and only uses valid cdrnodes when we have
	 * reached mincdrcachesize valid cdrnodes.
	 */
	if (cdrcachesize == 0 ||
			(cdrp->cn_mode != 0 && cdrcachesize < mincdrcachesize))
	{
		BUGLPR(buglevel, 9, ("malloc'ed cdrnode:  "));
		cdrp = (struct cdrnode *)malloc(sizeof *cdrp);
		if (!cdrp)
			return ENFILE;
		*cdrpp = cdrp;
		return 0;
	}

	BUGLPR(buglevel, 9, ("cdrcachesize = %d, getting cdrnode from cache\n", cdrcachesize));
	/* Use the cdrnode cache if we have more than the desired
	 * number of valid cdrnodes in the cache.
	 */

	/* remove the cdrnode from the cdrcache */
	if (cdruncache(cdrp))
	{
		BUGLPR(buglevel, 9, ("got cdrnode from cache, blocked\n"));
		/* cdruncache() could have slept, releasing the cdrfs
		 * lock.  This would allow another process to create a
		 * valid cdrnode for the cdrnode we are trying to get.
		 * Thus we need to search the hash again before
		 * creating another cdrnode.  So if the cdrnode still
		 * has zero count, put the cdrnode back on the front of
		 * the cache list and return error.
		 */
		if (cdrp->cn_count == 0)
		{
			cdrcachesize++;
			insque2(cdrp, &cdrcachelist);
		}
		return 1;	/* could have blocked */
	}

	BUGLPR(buglevel, 9, ("got cdrnode from cache:  "));
	/* remove from previous hash chain */
	BUGLPR(buglevel, 9, ("cdrp = 0x%x, cn_forw = 0x%x, cn_back = 0x%x\n", cdrp, cdrp->cn_forw, cdrp->cn_back));
	if (cdrp->cn_mode != 0)
		remque(cdrp);
	*cdrpp = cdrp;
	BUGLPR(buglevel, 9, ("getcdrp returning 0\n"));
	return 0;
}

/*
 * NAME:	cdrptovaddr(cdrp, vaddr)
 *
 * FUNCTION:	This is a attempt to give some of the file system 
 *		code a flavor of portability.  This subroutine 
 *		could in theory be replaced with whatever code is required
 *		to map a cdrnode to a virtual address.
 *
 * PARAMETERS:	cdrp	- cdrnode to be mapped into virtual memory
 *		vaddr	- returned virtual address
 *
 * RETURN :	0	- success
 *		errors from cdrbindseg()
 */
int
cdrptovaddr (
	struct cdrnode *	cdrp,	/* cdrnode to map in to memory	*/
	caddr_t *		vaddr)	/* returned virtual address	*/
{
	int			rc = 0;	/* return code			*/
	
	/* Bind this cdrnode to a virtual memory address and load a
	 * free segment register with segment id from the cdrnode.
	 */
	if (cdrp->cn_seg || (rc = cdrbindseg(cdrp)) == 0)
	{
		/* This may not be the best thing to use here. vm_att()
		 * may fail (panic).  In general, there should be
		 * enough scratch segment registers to go around, but
		 * given the level of nesting allowed in version 3, we
		 * may run out.  If so, this code could use the more
		 * fundamental vm_geth() and vm_seth() to save and
		 * restore a particular segment register.
		 */
		*vaddr = vm_att(SRVAL(cdrp->cn_seg, 0, FALSE), 0);
	}
	return rc;
}

/*
 * NAME:	cdrpundo(vaddr)
 *
 * FUNCTION:	This is a attempt to give some of the file system 
 *		code a flavor of portability.  This subroutine 
 *		could in theory be replaced with whatever code is required
 *		to unmap a cdrnode from a virtual address.
 *
 * PARAMETERS:	vaddr	- virtual address of segment to free
 *
 * RETURN :	0	- success
 */
void
cdrpundo (caddr_t	vaddr)
{
	(void)vm_det(vaddr);
}

/*
 * NAME:	cdractivity(vfsp, dev, root, forced, crp)
 *
 * FUNCTION:	This function checks for activity on a device.  It invalidates
 *		all cdrnodes with a count of zero for regular unmounts and all
 *		cdrnodes irrespective of count for forced unmounts.  This
 *		function is used by the umount vfs operation to determine
 *		whether it is safe to unmount.
 *
 * PARAMETERS:	vfsp	- vfs of the device mount to check activity for
 *		dev	- device to check for activity
 *		root	- cdrnode of root of device filesystem
 *		forced	- forced unmount flag
 *
 * RETURN :	0	- success
 *		EBUSY	- not forced unmount and cdrnodes from dev in use
 * 
 * SERIALIZATION: This function should only be called with the cdrfs lock held.
 *			
 */
int
cdractivity (
	struct vfs *		vfsp,		/* vfs of device mount	*/
	dev_t			dev,		/* device to check	*/
	struct cdrnode *	root,		/* root cdrnode of vfs	*/
	int			forced,		/* forced unmount flag	*/
	struct ucred *		crp)		/* credentials		*/
{
	struct vfs *		tvfsp;		/* vfs ptr in vfs list	*/
	struct hcdrnode *	hcdrp;		/* hash list (bucket)	*/
	struct cdrnode *	cdrp;		/* cdrnode in hash list	*/
	struct cdrnode *	next;		/* next cdrnode in list	*/
	int			rc = 0;		/* func return value	*/
	struct vnode *		vp;		/* vnode in vfs search	*/
	extern struct vnodeops	stale_cdr_vops;	/* stale vnode ops	*/

loop:
	BUGLPR(buglevel, 9, ("checking cdrnode activity, dev = 0x%x, root = 0x%x, forced = %d\n", dev, root, forced));
	BUGLPR(buglevel, 9, ("&cdrcachelist = 0x%x, next = 0x%x, prev = 0x%x\n", &cdrcachelist, cdrcachelist.ic_next, cdrcachelist.ic_prev));
	for (cdrp = cdrcachelist.ic_next; cdrp != (struct cdrnode *)&cdrcachelist; cdrp = cdrp->cn_next)
		BUGLPR(buglevel, 9, ("cdrnode on cache:  0x%x, dev = 0x%x, dirent = 0x%x, next = 0x%x\n", cdrp, cdrp->cn_dev, cdrp->cn_dirent, cdrp->cn_next));

	/* if not forced unmount, return EBUSY if root cdrnode is held
	 * more than once.
	 */
	if (!forced && (root->cn_count > 1 || vfsp->vfs_mntd->v_count > 1))
		return EBUSY;

	for (hcdrp = hcdrnode; hcdrp < &hcdrnode[NHCDRNO]; hcdrp++)
	{
		next = hcdrp->hcdr_forw;
		while (next != (struct cdrnode *)hcdrp)
		{
			cdrp = next;
			next = cdrp->cn_forw;	/* hang onto cn_forw */
			BUGLPR(buglevel, 9, ("found cdrnode 0x%x, dev = 0x%x, dirent = 0x%x\n", cdrp, cdrp->cn_dev, cdrp->cn_dirent));
			if (cdrp->cn_dev == dev)
			{
				BUGLPR(buglevel, 9, ("found cdrnode in hash:  0x%x, count = %d\n", cdrp, cdrp->cn_count));

				/* handle the mount cdrnode later */
				if (cdrp->cn_dirent == 0)
					continue;

				/* if not forced unmount, check the reference
				 * count.
				 */
				if (!forced && cdrp != root && cdrp->cn_count > 0)
					return EBUSY;
				
				BUGLPR(buglevel, 9, ("removing cdrnode from cache\n"));
				/* Call cdruncache() to free vmm stuff.
				 * If cdruncache() sleeps we need to
				 * start over.  If we can find a sure
				 * way to quiet the file system during
				 * unmount, this would be unneccessary,
				 * but until then....
				 */
				if (cdruncache(cdrp))
				{
					/* put back on front of cache list */
					if (cdrp->cn_count == 0)
					{
						cdrcachesize++;
						insque2(cdrp, &cdrcachelist);
					}
					goto loop;
				}

				/* Remove the cdrnode from the hash
				 * lists and put it back on the cache list.
				 */
				BUGLPR(buglevel, 9, ("removing cdrnode from hash list\n"));
				if (cdrp->cn_count > 0)
					remque(cdrp);
				else
					cdrunhash(cdrp);
			}
		}
	}
	
	/* If this is not a forced unmount, we're done. */
	if (!forced)
		return 0;

	/* Search the vfs list to find vfs's which are from the same device
	 * as our device mount, as these should be unmounted with the device
	 * unmount.  All vfs's for "directory over directory" mounts and
	 * "file over file" mounts will be after our vfs on the vfs list.
	 */
	BUGLPR(buglevel, 9, ("searching vnode list of vfs\n"));
	for (tvfsp = vfsp; tvfsp != NULL; tvfsp = tvfsp->vfs_next)
	{
		/* We don't want to look at unmounting filesystems other
		 * than our own.  Make sure that the filesystem is from
		 * the same device as the device mount.
		 */
		if (tvfsp->vfs_type != MNT_CDROM ||
		    tvfsp->vfs_fsid.fsid_dev != dev ||
		    (tvfsp->vfs_flag & VFS_UNMOUNTING && tvfsp != vfsp))
			continue;

		for (vp = tvfsp->vfs_vnodes; vp != NULL; vp = vp->v_vfsnext)
		{
			cdrp = VTOCDRP(vp);
			BUGLPR(buglevel, 9, ("found vnode 0x%x, cdrnode 0x%x in vfs list\n", vp, cdrp));
			BUGLPR(buglevel, 9, ("cdrnode 0x%x:  count = %d, mapcount = %d\n", cdrp, cdrp->cn_count, cdrp->cn_mapcnt));

			ASSERT(cdrp->cn_count > 0);
			ASSERT(cdrp->cn_mapcnt >= 0);
			if (cdrp->cn_mapcnt == 0)
			{
				if (--cdrp->cn_count <= 0 &&
						cdrp != &stale_cdrnode)
				{
					/* Free the cdrnode's segments and
					 * clear the gnode's vnode list.
					 */
					cdrclear(cdrp);
					cdrp->cn_gnode.gn_vnode = NULL;
				}

				/* Make the vnode use the stale cdrnode and
				 * add the vnode to the stale cdrnode's gnode's
				 * vnode list.
				 */
				vp->v_gnode = &stale_cdrnode.cn_gnode;
				vp->v_next = stale_cdrnode.cn_gnode.gn_vnode;
				stale_cdrnode.cn_gnode.gn_vnode = vp;
			}
			else
				/* make vnode ops use stale vnode ops */
				cdrp->cn_gnode.gn_ops = &stale_cdr_vops;
		}

		/* Unmount the filesystem, if it is not ours. */
		if (tvfsp != vfsp)
		{
			tvfsp->vfs_mntdover->v_mvfsp = NULL;
			tvfsp->vfs_flag |= VFS_UNMOUNTING;
			rc = VFS_UNMOUNT(tvfsp, UVMNT_FORCE, crp);
			ASSERT(rc == 0);
			if (rc)
			{
				tvfsp->vfs_mntdover->v_mvfsp = tvfsp;
				tvfsp->vfs_flag &= ~VFS_UNMOUNTING;
			}
			if (tvfsp->vfs_vnodes == NULL)
				vfsrele(vfsp);
		}
	}
	BUGLPR(buglevel, 9, ("done searching vnode list of vfs\n"));

	BUGLPR(buglevel, 9, ("finished checking activity, cdrcachesize = %d\n", cdrcachesize));
	BUGLPR(buglevel, 9, ("&cdrcachelist = 0x%x, next = 0x%x, prev = 0x%x\n", &cdrcachelist, cdrcachelist.ic_next, cdrcachelist.ic_prev));
	for (cdrp = cdrcachelist.ic_next; cdrp != (struct cdrnode *)&cdrcachelist; cdrp = cdrp->cn_next)
		BUGLPR(buglevel, 9, ("cdrnode on cache:  0x%x, dev = 0x%x, dirent = 0x%x, next = 0x%x\n", cdrp, cdrp->cn_dev, cdrp->cn_dirent, cdrp->cn_next));

	return 0;
}

/*
 * NAME:	cdrunhash(cdrp)
 *
 * FUNCTION:	This function removes a cdrnode from the hash list.
 *
 * PARAMETERS:	cdrp	- cdrnode to remove from hash list
 *
 * RETURN :	none
 */
void
cdrunhash(struct cdrnode *	cdrp)
{
	BUGLPR(buglevel, 9, ("cdrunhash:  cdrp = 0x%x\n", cdrp));
	/* remove cdrnode from hash list */
	ASSERT(cdrp->cn_forw != NULL);
	ASSERT(cdrp->cn_back != NULL);
	remque(cdrp);
	cdrclear(cdrp);
}

/*
 * NAME:	cdrclear(cdrp)
 *
 * FUNCTION:	This function clears many of the fields in the cdrnode
 *		and adds the cdrnode to the cache list (or frees it).
 *
 * PARAMETERS:	cdrp	- cdrnode to clear
 *
 * RETURN :	none
 */
void
cdrclear(struct cdrnode *	cdrp)
{
	/* make cdrnode free */
	cdrp->cn_forw	= cdrp->cn_back = NULL;
	cdrp->cn_flag	= 0;
	cdrp->cn_dirent	= 0;
	cdrp->cn_dev	= 0xdeadbeef;
	cdrp->cn_seg	= 0;
	cdrp->cn_mode	= 0;
	cdrp->cn_count	= 0;

	if (cdrcachesize < maxcdrcachesize)
	{
		/* This is a completely useless cdrnode, so put it at the head
		 * of cache list so that it will be used as soon as possible.
		 */
		BUGLPR(buglevel, 9, ("cdrclear:  adding cdrnode 0x%x to cache\n", cdrp));
		cdrp->cn_flag = 0;
		cdrcachesize++;
		insque2(cdrp, &cdrcachelist);
		for (cdrp = cdrcachelist.ic_next; cdrp != (struct cdrnode *)&cdrcachelist; cdrp = cdrp->cn_next)
			BUGLPR(buglevel, 9, ("cdrnode on cache:  0x%x, dev = 0x%x, dirent = 0x%x, next = 0x%x\n", cdrp, cdrp->cn_dev, cdrp->cn_dirent, cdrp->cn_next));
	}
	else
	{
		BUGLPR(buglevel, 9, ("cdrclear:  freeing cdrnode 0x%x\n", cdrp));
		free(cdrp);
	}
}

/*
 * NAME:	cdruncache(cdrp)
 *
 * FUNCTION:	This function removes a cdrnode from the cache list and cleans
 *		up any virtual memory data left over.  If this function may
 *		have slept waiting for the virtual memory data to be cleared,
 *		the value 1 is returned.
 *
 * PARAMETERS:	cdrp	- cdrnode to remove from the cache list
 *
 * RETURN :	0 - success
 *		1 - if cdruncahce() may have slept
 */
int
cdruncache (struct cdrnode *	cdrp)
{
	int			sid;	/* segment id of cdrnode */

	BUGLPR(buglevel, 9, ("cdruncache:  cdrp = 0x%x, count = %d\n", cdrp, cdrp->cn_count));
	/* If the cdrnode was on the cache list, remove the cdrnode
	 * from the cache list.  Otherwise, just release the resources.
	 */
	if (cdrp->cn_count == 0)
	{
		ASSERT(cdrp != cdrp->cn_next);	/* shouldn't be null cached */
		ASSERT(cdrp->cn_mapcnt == 0);	/* shouldn't be mapped */
		cdrcachesize--;
		remque2(cdrp);
		NULL_CACHE(cdrp);
	}
	else
	{
		BUGLPR(buglevel, 9, ("uncacheing uncached cdrnode:  cdrp = 0x%x, prev = 0x%x, next = 0x%x\n", cdrp, cdrp->cn_prev, cdrp->cn_next));
		ASSERT(cdrp->cn_prev == cdrp);
		ASSERT(cdrp->cn_next == cdrp);
	}

	/* Check for a vmm segment to free.
	 */
	if ((sid = cdrp->cn_seg) == 0)
		return 0;

	/* Make vmm segment of the cdrnode inactive.
	 */
	vms_inactive(sid);
	vm_releasep(sid, 0, MAXFSIZE/PSIZE);

	/* If this cdrnode is mapped, we don't want to
	 * clean up right now.
	 */
	if (cdrp->cn_mapcnt > 0)
		return 0;

	BUGLPR(buglevel, 9, ("freeing cdrnode segment\n"));
	/* Lock the cdrnode to cause any cdrget()'s for the same
	 * device/dirent pair to find this cdrnode and block waiting
	 * for us to finish with it.  This will prevent cdrget() from
	 * using our cdrnode while we sleep in vms_delete(), while also
	 * preventing cdrget() from creating another cdrnode with the
	 * same device/dirent pair.
	 */
	cdrp->cn_flag |= (IGETLCK | ILOCK);

	/* clean up resources for the cdrnode */
	cdrclose(cdrp);

	/* clean up the segment of the cdrnode */
	cdrp->cn_seg = 0;
	vms_delete(sid);	/* can sleep */

	/* If somebody found our cdrnode in the hash table and are
	 * sleeping on it, they will have incremented the count.  We
	 * can just check for a positive count.
	 */
	cdrp->cn_flag &= ~(IGETLCK | ILOCK);
	if (cdrp->cn_count > 0)
	{
		cdrp->cn_flag &= ~IWANT;
		e_wakeup(&cdrp->cn_event);
	}
	return 1;
}

/*
 * NAME:	cdrclose(cdrp)
 *
 * FUNCTION:	This function is called by cdruncache()
 *		to free any resources of the cdrnode
 *		that are not used for an invalid cdrnode.
 *
 * PARAMETERS:	cdrp	- cdrnode to free memory for
 *
 * RETURN :	none
 */
void
cdrclose (struct cdrnode *	cdrp)
{
	struct cdrxd 	*xd;		/* cdrnode extent descriptor */
	struct cdrxd 	*xdnext;	/* next cdrnode extent descriptor */

	/* free the list of extent descriptors */
	xdnext = cdrp->cn_xdlist;
	while (xdnext)
	{
		xd = xdnext;
		xdnext = xd->cx_next;
		free(xd);
	}
	cdrp->cn_xdlist = NULL;

	/* free the symbolic link file buffer */
	if (IFTOVT(cdrp->cn_mode) == VLNK) {
		if (cdrp->cn_size > CN_PRIVATE &&
		    cdrp->cn_symfile != NULL)
			free(cdrp->cn_symfile);
	}
}

/*
 * NAME:	cdrbindseg(cdrp)
 *
 * FUNCTION:	This function creates a virtual memory segment and
 *		associates it with the persistent segment specified
 *		by cdrp.  On entry, the cdrnode must be locked by
 *		the caller and the dirent of the cdrnode must be valid.
 *		If the segment id field in the cdrnode is not zero,
 *		the VM segment is assumed to exist already.
 *		
 *		Returns 0. ok. ENOMEM if there is insufficient space
 *		to allocate the virtual memory data structures. EIO if
 *		there is a permanent i/o error.
 *		
 *
 * PARAMETERS:	cdrp	- pointer to cdrnode to bind to VM
 *
 * RETURN :	0	- success
 *		ENOMEM	- insufficient space to allocate the virtual
 *				memory data structures
 *		EIO	- permanent I/O error.
 *			
 */
int
cdrbindseg (struct cdrnode *	cdrp)
{
	int			rc;	/* return code			*/
	int			sid;	/* segment id for cdrnode	*/
	extern int		cdr_strategy(struct vnode *, struct buf *);
					/* CD-ROM fs stategy routine	*/

	/* If the segment id of the cdrnode is already filled in, assume
	 * that the cdrnode has already been bound to a segment
	 */
	if (cdrp->cn_seg)
		return 0;

	/* create the segment */
	rc = vms_create(&sid,
			V_CLIENT,
			CDRTOGP(cdrp),
			cdrp->cn_size,
			0,
			0);

	/* save the segment id in the cdrnode */
	if (rc == 0)
		cdrp->cn_seg = sid;

	return rc;
}

/*
 * NAME:	cdraccess (cdrp, mode, crp)
 *
 * FUNCTION:	This function checks the mode permission on a cdrnode.
 *		The mode must be one of IREAD, IWRITE or IEXEC in USR.  
 *		The mode is shifted to select the owner/group/other fields.
 *		
 *		DAC permissions are checked first.  Permissions which
 *		are not granted are then checked for their corresponding
 *		BYPASS_DAC_<perm> privilege.
 *
 * PARAMETERS:	cdrp	- cdrnode to check
 *		mode	- USR access mode to check (IREAD, IWRITE, or IEXEC)
 *		crp	- credential
 *
 * RETURN :	0	- success
 *		EACCES	- permission requested is denied
 *			
 */
int
cdraccess (
	struct cdrnode *	cdrp,	/* cdrnode to check access for	*/
	int			mode,	/* access mode to check for	*/
	struct ucred *		crp) 	/* cred pointer			*/
{
	/* Check to see if any access was requested */
	if (mode == 0)
		return 0;
	mode >>= OTHMODESHIFT;

	/* If we are owner of file, check owner permission bits. */
	if (crp->cr_uid == cdrp->cn_uid) {
		if (((cdrp->cn_mode >> OTHMODESHIFT) & mode) == mode)
			return 0;

	/* If we are not the owner of the file and we are a member of the
	 * group associated with the file, check the group permission bits.
	 */
	} else if (groupmember_cr(cdrp->cn_gid, crp)) {
		if (((cdrp->cn_mode >> GRPMODESHIFT) & mode) == mode)
			return 0;

	/* If we are not the owner of the file and we are not a member
	 * of the group associated with the file, check the other
	 * permission bits.
	 */
	} else if ((cdrp->cn_mode & mode) == mode)
		return 0;

	/* Access has not been granted so far. */

	/* If you have BYPASS_DAC privilege, you always get access.
	 * If the access mode is IREAD or IEXEC and you have
	 * BYPASS_DAC_READ or BYPASS_DAC_EXEC privilege respectively, 
	 * you get access.  The reason this check is postponed is so that 
	 * a priv_use audit record is not generated when access is 
	 * granted by normal DAC policy.
	 */
	if (privcheck_cr(BYPASS_DAC, crp) == 0)
		return 0;
	mode <<= USRMODESHIFT;
	if (mode & IREAD)
		if (privcheck_cr(BYPASS_DAC_READ, crp))
			return EACCES;
	if (mode & IEXEC)
		if (privcheck_cr(BYPASS_DAC_EXEC, crp))
			return EACCES;

	return 0;
}

/*
 * NAME:	cdrp_access(cdrp, flag, crp)
 *
 * FUNCTION:	This function is a common routine to check access
 *		permissions before an open.
 *
 * PARAMETERS:	cdrp	- cdrnode in question
 *		flag	- permissions to check
 *		crp	- credential
 *
 * RETURN :	0	- success
 *		EISDIR	- request to modify directory
 *		EACCES	- trucating an enforced record locked file
 *		errors from called subroutines
 */
int
cdrp_access (
	struct cdrnode *	cdrp,	/* cdrnode to check access for	*/
	int			flag,	/* permissions to check for	*/
	struct ucred *		crp)	/* pointer to the cred struct   */
{
	int			mode = 0;	/* mode to check for	*/

	if (flag & FWRITE)
		return EACCES;
	if (flag & FREAD)
		mode |= IREAD;

	if (flag & FEXEC)
	{
		if ((cdrp->cn_mode & IFMT) != IFREG)
			return EACCES;
		mode |= IEXEC;
	}
	return cdraccess(cdrp, mode, crp);
}

/*
 * NAME:	readcdr(cdrp, flags, ext, uiop)
 *
 * FUNCTION:	This function is common code for reading a cdrnode.
 *
 * PARAMETERS:	cdrp 	- pointer to the cdrnode that represents the 
 *			  object we want to read
 *		flags	- file open flags
 *		ext	- unused extension data
 *		uiop	- amount to read and where it goes
 *
 * RETURN :	0	- success
 *		EINVAL	- for negative offsets
 *		errors from subroutines
 * 
 * SERIALIZATION: This function should only be called with the cdrfs lock held.
 *			
 */
int
readcdr (
	struct cdrnode *	cdrp,	/* cdrnode for file to read	*/
	int			flags,	/* file open flags		*/
	caddr_t			ext,	/* unused extension data	*/
	struct uio *		uiop)	/* uio struct describing read	*/
{
	int			rem;	/* remaining data left in file	*/
	int			off;	/* starting offset within file	*/
	int			nbytes;	/* number of bytes to read	*/
	int			rc = 0;	/* return code			*/

	/* we must have the cdrfs_lock upon entry to this function */
	ASSERT(lock_mine(&cdrfs_lock));

	/* successfully read zero bytes */
	if (uiop->uio_resid <= 0)
		return 0;

	/* negative offsets not allowed */
	if (uiop->uio_offset < 0 || uiop->uio_offset > OFF_MAX)
		return EINVAL;

	/* map into virtual memory if necessary */
	if (cdrp->cn_seg == 0)
		if (rc = cdrbindseg(cdrp))
			return rc;

	/* Using the uio_resid allows a call with multiple I/O vectors to work.
	 * Since the file is contiguous in virtual address space, vm_move()
	 * can handle the I/O vectors.
	 */
	off = uiop->uio_offset;
	rem = cdrp->cn_size - off;

	if (rem > 0)
	{
		/* we can't move more than remaining bytes */
		nbytes = MIN(rem, uiop->uio_resid);

		BUGLPR(buglevel, 9, ("reading offset: 0x%x, length: 0x%x\n", off, nbytes));

		/* trace hooks */
		TRCHKL4T(HKWD_KERN_PFS|hkwd_PFS_READI,
				cdrp, cdrp->cn_seg, off, nbytes);

		/* actually move the data. mapcnt is incremented over the read to
		 * prevent deletion of the cdrnode's vmm segment by a forced umount.
		 */
		cdrp->cn_mapcnt++;
		/*
		 * We don't want to prevent any other readers from
		 * doing anything in the CDROM file system while we are
		 * reading the disk. Therefore we release the cdrfs lock.
		 */
		CDRFS_UNLOCK();
		rc = vm_move(cdrp->cn_seg, off, nbytes, UIO_READ, uiop);
		CDRFS_LOCK();
		cdrp->cn_mapcnt--;

		if (rc)
			BUGLPR(buglevel, 7, ("read error:  %d\n", rc));

		/* update the cdrnode access time */
		cdrp->cn_atime = time;
		BUGLPR(buglevel, 9, ("read 0x%x bytes\n", (off_t) uiop->uio_offset - off));
	}

	return rc;
}


/*
 *	cdrfs error list
 *
 * note: EFAULT is added to prevent panic on forced unmount
 * on active file system. (todo - we may want a through review 
 * of forced unmount logic.)
 */
int cdr_elist[] = {EIO, ESTALE, ENOTREADY, EMEDIA, EFAULT, 0};

/* 
 *	cdrfs_exception
 *
 * if CDRFS_EXCEPTION is set, scan for acceptable error condition. 
 * if none found, longjmp() to next handler.
 */
cdrfs_exception (rc, elist)
int rc;				/* return code to validate */
int *elist;			/* null terminated error list */
{
	int *err;

	if (rc & CDRFS_EXCEPTION) {
		rc &= ~CDRFS_EXCEPTION;
		
		for (err = elist; *err; err++)
			if (*err == rc)
				return rc;

		/* may want to do some error logging here */

		longjmpx (rc);
		panic ("cdr_exception: unexpected return from longjmp()");
	}
	return rc;
}

/*
 *	cdrfs_clrjmpx
 *
 * regain the cdrfs lock and release jump buffer
 */
cdrfs_clrjmpx (jbp)
label_t	*jbp;
{
	/* assert that we don't have the cdrfs_lock */
	assert(!(lock_mine(&cdrfs_lock)));

	CDRFS_LOCK();
	clrjmpx (jbp);
}
