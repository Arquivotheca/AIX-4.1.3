static char sccsid[] = "@(#)19	1.63.1.36  src/bos/kernel/pfs/isubs.c, syspfs, bos41J, 9516A_all 4/17/95 13:17:19";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: ifind, iget, _iget, ilock, ilocklst, imark,
 *            inoinit, iput, irele, iunhash, iactivity,
 *            iuncache, getip
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
#include "jfs/fsvar.h"
#include "jfs/commit.h"
#include "sys/errno.h"
#include "sys/vfs.h"
#include "sys/sleep.h"
#include "sys/malloc.h"
#include "sys/syspest.h"
#include "sys/sysinfo.h"
#include "vmm/vmsys.h"

/*
 *	inode cache
 */
int maxicache;			/* Maximum # of inodes in system	*/
int maxcachesize;		/* Maximum # of inodes allowed in cache */
int cachesize;			/* Number of inodes currently in cache	*/

struct genalloc *ino_ap;	/* Inode allocation pointer		*/

/*
 *	inode lru cache list
 *
 * offsets of ic_next and ic_prev must be same as in the inode
 */
struct {
	struct inode *ic_forw;		/* hash list offset, not used */
	struct inode *ic_back;
	struct inode *ic_next;		/* Cachelist offsets */
	struct inode *ic_prev;
} icachelist;

/*
 *	inode hash list anchor table
 */
int nhino;
struct hinode *hinode;

/* inode cache queue management macros */
#define INSERT_HASH(e, after)	insque(e, after)
#define REMOVE_HASH(e)		remque(e)
#define	NULL_HASH(e)				\
	((struct inode *)(e))->i_forw = 		\
	((struct inode *)(e))->i_back = ((struct inode *)(e))

#define INSERT_CACHE(e, after)	insque2(e, after)
#define REMOVE_CACHE(e)		remque2(e)
#define	NULL_CACHE(e)				\
	((struct inode *)(e))->i_next = 		\
	((struct inode *)(e))->i_prev = ((struct inode *)(e))

/*
 * inode cache lock
 */
Simple_lock	jfs_icache_lock;

int	ilock_cnt = 0;		/* inode lock occurrence */
int	vlock_cnt = 1<<15;	/* vnode lock occurrence */

BUGVDEF(isubsdbg, 0)

/*
 * combined vnode/inode for base (non-shadow) objects
 */
struct xnode {
	struct vnode	x_vnode;
	struct inode	x_inode;
};

/*
 * NAME:	inoinit ()
 *
 * FUNCTION:	Initialize xnode (vnode+inode) table
 *
 * PARAMETERS:	None
 *
 * RETURN :	None
 *			
 */
inoinit()
{
	extern struct fsvar fsv;
	extern int dqcachemax;

	struct hinode *hip;
	struct inode *ip;
	int n, rc, i;
	uint memsize;
	extern uint pfs_memsize();
	uint k;
	int numinodes;

        /* Initialize the inode cache lock */
        lock_alloc(&jfs_icache_lock,LOCK_ALLOC_PAGED,ICACHE_LOCK_CLASS,-1);
        simple_lock_init(&jfs_icache_lock);

	/* allocate the inode hash table. scale the number of hash
	 * buckets with memory size by the following:
	 * 1. for machines with 64M and less of memory will have 512 hash 
	 *     buckets.
	 * 2. every 64M of memory after will have a log 2 number of buckets.
	 */
	memsize = pfs_memsize();

	/* round up memsize to the nearest Meg */
	memsize = (memsize + ((1024*1024)-1) & ~(1024*1024));

	if (memsize < 64*1024*1024)
	{
		nhino = 512;
	}
	else
	{
		k = memsize/(64*1024*1024);
		for (i = 0; i < 32; i++)
		{
			if (k & 0x80000000)
				break;
			k <<= 1;
		}
		nhino =  (1 << (31-i)) * 512;
	}
		
	if ((hinode = (struct hinode *) malloc(nhino * sizeof(struct hinode))) == NULL)
		panic("Inoinit: unable to allocate inode hash table");

	/* max inactive inodes before LRU recycle of them:
	 * ((number of xnode table pages allocated per Mbyte of real memory) *
	 * (number of xnodes per page)) where
	 * the number of xnode table pages allocated per Mbyte of real memory
	 *  for real memory < 32Mbyte: 2
	 *  for real memory >= 32Mbyte: 4
	 *
	 * table size (max active inodes): 
	 * 2 * OPEN_MAX + max cache size
	 * (ref. OPEN_MAX in limits.h)
	 */
	if (memsize < 32*1024*1024)
		maxicache = (memsize / 0x80000) * (PSIZE / sizeof(struct xnode));
	else
		maxicache = (memsize / 0x40000) * (PSIZE / sizeof(struct xnode));

	rc = geninit((numinodes=2 * OPEN_MAX + maxicache), fsv.v_ninode,
		 sizeof(struct xnode), (caddr_t)&ip->i_forw - (caddr_t)ip,
		 "INODES", &fsv.v_inode);

	if (rc)
		panic("Inoinit: unable to allocate inode table");

	/* Number of dquots to allocate, worst case 2 per inode */
	dqcachemax=2 * numinodes;

	/* Save inode allocation pointer */
	ino_ap = fsv.v_inode;

	/* Limit maximum inode cache size to initial allocation */
	maxcachesize = fsv.v_ninode;

	/* initialize the hash table */
	for (hip = hinode; hip < &hinode[nhino]; hip++)
	{	hip->hi_forw = (struct inode *)hip;
		hip->hi_back = (struct inode *)hip;
		hip->hi_timestamp = 0;
		hip->hi_vget = 0;
	}

	/* Null hash icachelist */
	icachelist.ic_next = (struct inode *) &icachelist;
	icachelist.ic_prev = (struct inode *) &icachelist;

	return 0;
}


/*
 * NAME:	inogrow ()
 *
 * FUNCTION:	Allow inode table to grow
 *
 * EXECUTION ENVIRONMENT: called durring phase 2 system initialization.
 *
 * PARAMETERS:	None
 *
 * RETURN :	None
 *			
 */

void
inogrow()
{
	maxcachesize = maxicache;
}


/*
 * NAME:	imark (ip, flag)
 *
 * FUNCTION:	update i_flag and accessed, changed, or updated times 
 *		in an inode.
 *
 * PARAMETERS:	ip	- pointer to inode
 *		flag	- inode times to set
 *
 * RETURN :	None
 *			
 * NOTE:	readers hold INODE_LOCK() to serialize update of i_atime
 *		on entry/exit
 */

imark(ip, flag)
struct inode *ip;
int flag;
{
	void curtime(struct timestruc_t *);
	struct timestruc_t t;

	if (isreadonly(ip))
        	return;

	curtime(&t);	/* fetch current time */

	ip->i_flag |= flag;

	if (flag & IACC)
		ip->i_atime_ts = t;

	if (flag & IUPD)
		ip->i_mtime_ts = t;

	if (flag & ICHG)
		ip->i_ctime_ts = t;
}


/*
 * NAME:	iwritelocklist(n, va_ilist)
 *
 * FUNCTION:	Lock variable number of inodes in descending i_number
 *
 * PARAMETERS:	n	 - number of inodes
 *		va_ilist - varags inode list
 *
 * RETURN :	None
 *			
 */

iwritelocklist(n, va_ilist)
int n;				/* Number of elements	*/
va_list	va_ilist;		/* Varargs list		*/
{
	struct inode **ipp, *ip;
	int k, changes;
	struct inode *ipl[6];		

	ipp = (struct inode **) &va_ilist;

	for (k = 0; k < n; k++)
		ipl[k] = *ipp++;

	/* Bubble sort	*/
	do
	{	
		changes = 0;
		for (k = 0; k < n; k++)
			if ((k+1) < n && ipl[k+1]->i_number > ipl[k]->i_number)
			{	
				ip = ipl[k];	
				ipl[k] = ipl[k+1];
				ipl[k+1] = ip;
				changes++;
			}
	} while (changes);

	for (k = 0; k < n; k++)
		IWRITE_LOCK(ipl[k]);
}


/*
 * NAME:	ireadlockx(ip)
 *
 * FUNCTION:	wrapper to lock inode
 *
 * PARAMETERS:	ip	- inode we want to lock
 *
 * RETURN :	None
 *			
 */

ireadlockx(ip)
struct inode *ip;
{
	IREAD_LOCK(ip);
}


/*
 * NAME:	ireadunlockx(ip)
 *
 * FUNCTION:	wrapper to unlock inode
 *
 * PARAMETERS:	ip	- inode we want to unlock
 *
 * RETURN :	None
 *			
 */

ireadunlockx(ip)
struct inode *ip;
{
	IREAD_UNLOCK(ip);
}


/*
 * NAME:	iwritelockx(ip)
 *
 * FUNCTION:	wrapper to lock inode
 *
 * PARAMETERS:	ip	- inode we want to lock
 *
 * RETURN :	None
 *			
 */

iwritelockx(ip)
struct inode *ip;
{
	IWRITE_LOCK(ip);
}


/*
 * NAME:	iwriteunlockx(ip)
 *
 * FUNCTION:	wrapper to unlock inode
 *
 * PARAMETERS:	ip	- inode we want to unlock
 *
 * RETURN :	None
 *			
 */

iwriteunlockx(ip)
struct inode *ip;
{
	IWRITE_UNLOCK(ip);
}


/*
 * NAME:	ifind (dev)
 *
 * FUNCTION:	find the specified inode which MUST exist in 
 *		the inode cache.
 *
 * PARAMETERS:	dev	- device of wanted inode
 *
 * RETURN :	PANIC if the specified inode is not found.
 *
 * SERIALIZATION: ICACHE_LOCK() held on entry/exit
 */

struct inode *
ifind (dev)
dev_t  dev;  		/* device on which inode resides */
{
	struct inode	*ip; 
	struct hinode	*hip;

	IHASH(0, dev, hip);

	/* Search the hash list for this inode
	 */
	for (ip = hip->hi_forw; ip != (struct inode *) hip; ip = ip->i_forw)
		if (ip->i_number == 0 && dev == ip->i_dev)
			return ip;

	panic("ifind: failure");
}

/* NAME:	iget (dev, ino, ipp, doscan, vsfp)
 *
 * FUNCTION:	This function is an intermediate function to the actual
 *		iget() function call, now named _iget(). This function
 *		preserves the old semantics of iget() for other filesystems
 *		such as AFS which make calls to iget().
 */
iget (dev, ino, ipp, doscan, vfsp)
dev_t dev;
ino_t ino;
struct inode **ipp;
int doscan;
struct vfs *vfsp;
{
	struct hinode *hip;

	sysinfo.iget++;
	cpuinfo[CPUID].iget++;

	IHASH(ino, dev, hip);
	
	return(_iget(dev, ino, hip, ipp, doscan, vfsp));
}

/*
 * NAME:	_iget (dev, ino, hip, ipp, doscan, vfsp)
 *
 * FUNCTION:	get the specified inode where
 *		if vfsp != NULL, get also the vnode of the inode,
 *		if vfsp == NULL, get only the inode.
 *
 *		if doscan == 1, we need to scan the hash table for the inode.
 *		if doscan == 0, this is a new inode and we need not scan hash.
 *
 * 		reference of vnode and/or inode is acquired but not locked.
 *
 * PARAMETERS:	dev	- device of wanted inode
 *		ino	- inode number
 *		ipp	- returned inode
 *		doscan	- new inode, no scan needed
 *		vfsp	- vfs
 *
 * RETURN :	ENFILE	- If out of inodes
 *		errors from subroutines
 *			
 * SERIALIZATION: ICACHE_LOCK() held on entry/exit
 */
_iget (dev, ino, hip, ipp, doscan, vfsp)
dev_t dev;				/* Device for ino		*/
ino_t ino;				/* Establish ic inode for ino # */
struct hinode	*hip;			/* hash list where inode resides */
struct inode **ipp;			/* Return addr for found/new ip	*/
int doscan;				/* Is this a newly created inode? */
struct vfs	*vfsp;			/* Is this a device mount? */
{
	int	rc;
	struct inode	*ip;
	struct gnode	*gp;
	struct vnode	*vp;
	int	gen, type;
	extern struct vnodeops jfs_vops;
	extern time_t	time;	/* current time for new inode i_gen */
	extern gn_reclk_count;	/* occurrence number of gn_reclk lock */
	int cachelist;		/* is the inode from the cachelist? */
	int timestamp;		/* timestamp on hash chain modification */
	int active;		/* number of active references on a vnode */

	if (vfsp)
		ASSERT(vfsp->vfs_flag & VFS_DEVMOUNT);

	if (doscan == 0)
	{
		*ipp = NULL;
		goto getinode;
	}

loop:
	*ipp = NULL;

	/* Search the hash list for the specified inode
	 */
	for (ip = hip->hi_forw; ip != (struct inode *) hip; ip = ip->i_forw)
		if (ino == ip->i_number && dev == ip->i_dev) 
		{
		/* 
		 *	cache hit
		 *
		 * if IXLOCK is set, the inode is in transition
		 * (either being initialized or recycled/freed),
		 * wait for completion of transition.
		 * if i_mode == 0, the inode found has transformed
		 * as invalid and to be discarded.
		 */
			ip->i_count++;

			if (ip->i_locks & IXLOCK)
			{
				do {
					ip->i_locks |= IXWANT;
					e_sleep_thread(&ip->i_event,
						       &jfs_icache_lock,
						       LOCK_SIMPLE);
					if (ip->i_mode == 0)
					{
						iput(ip, NULL);
						goto loop;
					}
				} while (ip->i_locks & IXLOCK);

				goto getvnode;
			}

			/* remove from cachelist if it is being reactivated.
			 */
			if (ip->i_count == 1)
			{
				REMOVE_CACHE(ip);
				NULL_CACHE(ip);
				cachesize--;
			}

getvnode:
			/*
			 *	get vnode (inode cache hit)
			 *
			 * v_count specifies references of the vnode
			 * while i_count specifies number of vnodes
			 * referencing the base inode.
			 * (vnode management for soft mount (vfsp == NULL)
			 * is provided by LFS).
			 */
			if (vfsp)
			{
				vp = ip->i_gnode.gn_vnode;
				active = vp->v_count;
				fetch_and_add(&vp->v_count, 1);
				if (active)
				{
					/* vnode had been already activated.
					 */
					ip->i_count--;
				}
				else
				{
					/* either reactivated from cachelist
					 * (vnode stayed linked to underlying
					 * fs: v_vfsp != NULL) or
					 * object in dir-over-dir mount is
					 * being accessed in underlying fs after 
					 * being accessed in mounted fs first
					 * (base vnode had not been linked to
					 * underlying fs: v_vfsp == NULL)
					 */  
					if (vp->v_vfsp == NULL)
						vget(ip, vfsp);
				}
			}

			*ipp = ip;
			return 0;
		}

	/*
	 *	cache miss
	 */

	/*	get an in-memory inode.
	 *	1. set the timestamp on the hash list.
	 *	2. if we don't release the ICACHE_LOCK() while in getip()
	 *	   we simply continue.
	 *	3. if we release the lock, we must check the timestamp.
	 *	   a. timestamp not equal means something was inserted or
	 * 	      deleted, therefore we have to search again.
	 *	   b. timestamp is equal we just try to get an inode again.
	 */
getinode:
	timestamp = hip->hi_timestamp;

	switch (rc = getip(&ip, &cachelist))
	{
		case 0:
			break;

		case 1:
			if (timestamp != hip->hi_timestamp)
				goto loop;
			else
				goto getinode;

		case ENFILE:
			return rc;
	}

	/*
	 *	initialize inode
	 *
	 * either a clean vnode+inode is allocated from freelist
	 * or a vnode+inode has been recycled from cachelist.
	 */

	/* initialize inode before inserting into hash list so that
	 * concurrent _iget()s for this inode will wait on it.
	 */
	ip->i_dev = dev;
	ip->i_number = ino;
	ip->i_count = 1;
	ip->i_locks |= IXLOCK;
	ip->i_event = EVENT_NULL;

	/* insert the inode at head of hash list
	 */
	INSERT_HASH(ip, hip);
	NULL_CACHE(ip);

	/* increment the timestamp on the hash
	 */
	hip->hi_timestamp++;

	/* get mount inode for the inode
	 */
	ip->i_ipmnt = ifind(ip->i_dev);

	/* if the inode was recycled from the cache list,
	 * cleanup dquots of previous object.
	 */
	if (cachelist)
		putinoquota(ip);

	/* read on-disk inode into in-memory inode
	 */
	ICACHE_UNLOCK();
 	rc = iread(ip);
	ICACHE_LOCK();

	/* get dquots for the inode if not newly created object.
	 * (for newly created object, caller will get and check
	 * inode quota)
	 */
	if (rc == 0 && ip->i_nlink != 0)
		rc = getinoquota(ip);

	/* if initialization failed, free the inode.
	 */
	if (rc)
	{	
		/* mark the inode to be discarded and remove it from 
		 * hashlist to prevent later _iget() to find the inode.
		 */
		ip->i_mode = 0;
		REMOVE_HASH(ip);
		NULL_HASH(ip);

		/* wakeup sleepers (concurrent _iget() who found the inode) 
		 * who will iput() the inode on i_mode = 0. on the last 
		 * reference release, the inode will be freed.
		 */
		ip->i_locks &= ~IXLOCK;
		if (ip->i_locks & IXWANT)
		{	
			ip->i_locks &= ~IXWANT;
			e_wakeupx(&ip->i_event, E_WKX_NO_PREEMPT);
		}

		iput(ip, NULL);
		return rc;
	}

	/*
	 * continue to initialize the inode (should not fail.)
	 */

	gp = ITOGP(ip);

	/* if newly created inode (i_nlink = 0), initialize/update
	 * i_gen and zero out all other on-disk inode fields
	 * (caller of new inode creation (dev_ialloc()) will initialize 
	 * further from creation request parameters).
	 * otherwise initialize further based on on-disk inode information.
	 */
	if (ip->i_nlink == 0)
	{
		/* If the inode has never been allocated, 
		 * start the gen at the current time.
		 */
		if (ip->i_gen == 0)
			gen = time;
		else
			gen = ip->i_gen + 1;

		bzero(&ip->i_dinode, sizeof(ip->i_dinode));

		ip->i_gen = gen;
	}
	else
	{
		gp->gn_type = IFTOVT(ip->i_mode);

		/* for regular files and directories and big symbolic links 
	 	 * set indirect block page pointer to zero 
	 	 */
		type = ip->i_mode & IFMT;
		switch (type) {
		case IFLNK:
			if (ip->i_size <= D_PRIVATE)
				break;
		case IFREG:
		case IFDIR:
			ip->i_vindirect = 0;
			break;
		default:		
			break;
		}

		if (gp->gn_type == VCHR || gp->gn_type == VBLK)
			gp->gn_rdev = ip->i_rdev;
		else
			gp->gn_rdev = ip->i_dev;

		ip->i_flag = 0;
	}

	ip->i_cflag = (ip->i_nlink) ? 0 : CMNEW;
	ip->i_compress = 0;
	ip->i_movedfrag = NULL;
	ip->i_cluster = 0;

	if (!(ip->i_locks & ILOCKALLOC))
	{
		ilock_cnt++;

#ifdef _I_MULT_RDRS
		lock_alloc(&ip->i_rdwrlock,LOCK_ALLOC_PAGED,IRDWR_LOCK_CLASS,
			   ilock_cnt);
		lock_init(&ip->i_rdwrlock, 1);

		lock_alloc(&ip->i_nodelock,LOCK_ALLOC_PAGED,INODE_LOCK_CLASS,
			   ilock_cnt);
		simple_lock_init(&ip->i_nodelock);
#else /* simple lock */
		lock_alloc(&ip->i_rdwrlock,LOCK_ALLOC_PAGED,IRDWR_LOCK_CLASS,
			   ilock_cnt);
		simple_lock_init(&ip->i_rdwrlock);
#endif /* _I_MULT_RDRS */

		ip->i_locks |= ILOCKALLOC;
	}

	ip->i_openevent = EVENT_NULL;

	/* fill in the remaining gnode fields for this inode
	 */
	gp->gn_flags = 0;
	gp->gn_ops = &jfs_vops;
	gp->gn_seg = 0;
	gp->gn_mwrcnt = gp->gn_mrdcnt = 0;
	gp->gn_rdcnt = gp->gn_wrcnt = gp->gn_excnt = gp->gn_rshcnt = 0;
	if (!(ip->i_locks & GLOCKALLOC))
	{
		lock_alloc(&gp->gn_reclk_lock,LOCK_ALLOC_PAGED,RECLK_LOCK_CLASS,
		   gn_reclk_count++);
		simple_lock_init(&gp->gn_reclk_lock);
		ip->i_locks |= GLOCKALLOC;
	}
	gp->gn_reclk_event = EVENT_NULL;
	gp->gn_filocks = (struct filock *)NULL;
	gp->gn_data = (caddr_t)ip;

	/* get the vnode (on inode cache miss)
	 */
	vget(ip, vfsp);

	/* wakeup sleepers (concurrent _iget() who found the inode) 
	 */
	ip->i_locks &= ~IXLOCK;
	if (ip->i_locks & IXWANT)
	{	
		ip->i_locks &= ~IXWANT;
		e_wakeupx(&ip->i_event, E_WKX_NO_PREEMPT);
	}

	*ipp = ip;
	return 0;
}


/*
 * NAME:	getip(ipp, cachelist)
 *
 * FUNCTION:	allocate a in-memory inode.
 *
 * PARAMETERS:	ipp	- returned inode
 *		cachelist - set to 1 if inode allocated from cache list
 * 			    set to 0 if inode allocated from free list 
 *
 * RETURN :	0	- success without ICACHE_LOCK() has been released
 *		1	- success with ICACHE_LOCK() has been temporarily 
 *			  released
 *		ENFILE	- inode table overflow
 *			
 * SERIALIZATION: ICACHE_LOCK() held on entry/exit
 */
static
getip(ipp, cachelist)
struct inode **ipp;
int	*cachelist;
{
	struct inode *ip;
	struct vnode *vp;
	struct xnode *xp;
	struct hinode *hip;

	*ipp = NULL;

	/* if cachesize is under the max cachesize 
	 * try to allocate from the free list.
	 */
	if (cachesize < maxcachesize)
	{
		if ((xp = (struct xnode *) genalloc(ino_ap)) != NULL)
		{
			/* bind inode and vnode */
			ip = (struct inode *)&xp->x_inode;
			ip->i_locks = 0;

			vp = (struct vnode *)&xp->x_vnode;
			vp->v_vfsp = NULL;
			vp->v_next = NULL;

			ip->i_gnode.gn_vnode = (struct vnode *)vp;

			vp->v_gnode = (struct gnode*)&ip->i_gnode;

			*ipp = ip;
			*cachelist = 0;
			return 0;
		}
	}

	/* either cachesize reached cachesize limit or
	 * free list is empty: try to recycle from cache list.
	 * first check if there is anything on the cachelist.
	 */
	if ((ip = icachelist.ic_next) == (struct inode *) &icachelist)
		return ENFILE;


	/* if ICACHE_LOCK() had been temporarily released,
	 * rescan hashlist whether the specified inode had been entered
	 * by another thread.
	 */
	if (iuncache(ip))
	{
		/* if no _iget()s are pending on the inode deactivated.
		 * insert back at head of cachelist; 
		 * otherwise, yield the inode to holder of reference.
		 */
		if (ip->i_count == 0)
		{
			cachesize++;
			INSERT_CACHE(ip, &icachelist);
		}

		return 1;
	}

	/* remove from previous hash chain
	 */
	REMOVE_HASH(ip);

	*ipp = ip;
	*cachelist = 1;
	return 0;
}


/*
 * NAME:	iuncache (ip)
 *
 * FUNCTION:	deactivate the specified inode to recycle:
 *		remove inode from cache list,
 *		commit changes of inode and/or file, and/or
 *		release its virtual memory resources.
 *
 *		the lock resources are reused, and 
 *		linkage with quota struct is maintained.
 *
 * PARAMETERS:	ip	- inode to blast
 *
 * RETURN :	0 - if ICACHE_LOCK() has NOT been temporarily released.
 *		1 - if ICACHE_LOCK() has been temporarily released.
 *
 * SERIALIZATION: ICACHE_LOCK() held on entry/exit
 */

iuncache (ip)
struct inode *ip;
{
	int sid;

	/* remove from cache list.
	 */
	if (ip->i_count == 0)
	{	
		ASSERT(ip != ip->i_next);
		cachesize--;
		REMOVE_CACHE(ip);
		NULL_CACHE(ip);
	}

	/* if the inode is ready to be recycled,
	 * unlink vnode from previous vfs.
	 */ 
	sid = ip->i_seg;
	if (sid == 0 &&
	    (!(ip->i_flag & (IACC|ICHG|IFSYNC)) ||
	     isreadonly(ip)))
	{
		vput(ip);
		return 0;
	}

	/* mark the inode as in transition so that _iget() of this inode
	 * will wait for the completion of transition.
	 */
	ip->i_locks |= IXLOCK;
	ip->i_count++;
	ICACHE_UNLOCK();

	/* Mark its vm object inactive so further page faults due to stale
	 * references will fail: actual delete will occur when the last 
	 * reference is released.
	 */
	if (sid != 0) 
		vms_inactive(sid);

	/* if inode and/or file has changed then commit it
	 */
	if (ip->i_number > SPECIAL_I || ip->i_number == ROOTDIR_I) 
	{
		/* for regular files, mark to be synced 
		 * if file have been modified
		 */
		if (sid &&
		    ((ip->i_mode & IFMT) == IFREG) &&
		    (ip->i_flag & IUPD || ismodified(ip)))
			ip->i_flag |= IFSYNC;

		/* commit if anything has been changed
		 */
		if (ip->i_flag & (IACC|ICHG|IFSYNC))
		{
			IWRITE_LOCK(ip);
			commit(1, ip);
			IWRITE_UNLOCK(ip);
		}

	}

	/* flush segment if journalled
	 */
	if (ip->i_mode & IFJOURNAL) 
		iflush(ip);

	/* release vm resources associated with the inode.
	 */
	if (sid != 0) 
	{

		/* delete vm object: actual delete will occur when
	 	 * the last reference is released.
		 */
		isegdel(ip);

		/* flush and free its indirect block pages in .indirect segment
		 */
		if (ip->i_vindirect)
			ifreeind(ip);
	}

	ICACHE_LOCK();
	ip->i_count--;

	/* wakeup sleepers (_iget()s for this inode who found it).
	 * yield the uncached inode to the _iget() for this inode.
	 */
	ip->i_locks &= ~IXLOCK;
	if (ip->i_locks & IXWANT)
	{	
		ip->i_locks &= ~IXWANT;
		e_wakeupx(&ip->i_event, E_WKX_NO_PREEMPT);
	}

	return 1;
}


/*
 * NAME:	iput (ip, vfsp)
 *
 * FUNCTION:	put the specified inode where
 *		if vfsp != NULL, put also the vnode of the inode,
 *		if vfsp == NULL, put only the inode.
 *
 * 		reference of vnode and/or inode is released
 * 		on the last reference release, insert into cachelist
 *		(valid inode) or return to freelist (invalid inode).
 *
 * PARAMETERS:	ip	- unwanted inode
 *		vfsp	- vfs
 *
 * RETURN :	Errors from subroutines.
 *			
 * SERIALIZATION: ICACHE_LOCK() held on entry/exit
 */
iput(ip, vfsp)
struct inode	*ip;
struct vfs	*vfsp;
{
	struct vnode	*vp;

	assert(ip->i_count > 0);

	/* release vnode reference
	 */
	if (vfsp)
	{
		vp = ip->i_gnode.gn_vnode;
		assert(vp->v_count > 0);
		fetch_and_add(&vp->v_count, -1);
		if (vp->v_count)
			return 0;
	}

	/* release inode reference
	 */
	if (--ip->i_count > 0)
		return 0;

	/*
	 * last reference release
	 */
	ip->i_count = 0;

	/* If iput() is for an inode for which no "real"
	 * file exists don't re-deallocate the file!
	 */
	if (ip->i_mode && ((ip->i_nlink == 0)
	    || (ip->i_flag & IDEFER) || ip->i_compress))
	{
		/* mark the inode as in transition so that _iget() of 
		 * this inode will wait for the completion of transition.
	 	 */
		ip->i_locks |= IXLOCK;
		
		/* cleanup resources or fsync as required.
		 */
		iclose(ip);

		/* wakeup sleepers (_iget()s for this inode who found it).
	 	 * yield the inode to the _iget() for this inode.
	 	 */
		ip->i_locks &= ~IXLOCK;
		if (ip->i_locks & IXWANT)
		{	
			/* if the inode is being deleted,
			 * mark the inode to be discarded by previous
			 * _iget()s who found the inode, and remove it from
			 * hashlist to prevent later _iget() to find the 
			 * inode. (e.g., VNOP_VGET())
			 */
				if (ip->i_nlink == 0)
				{	
					ip->i_mode = 0;
					ip->i_gen++;
					REMOVE_HASH(ip);
					NULL_HASH(ip);
				}

				ip->i_locks &= ~IXWANT;
				e_wakeupx(&ip->i_event, E_WKX_NO_PREEMPT);

				return 0;
			}
		}

		/* insert at tail of cache list if it can be reactivated
		 */
		if (ip->i_nlink && ip->i_mode && !(ip->i_flag & IDEFER))
		{
			ip->i_flag &= (ICHG|IUPD|IACC|IFSYNC);
			cachesize++;
			INSERT_CACHE(ip, icachelist.ic_prev);
			return 0;
		}

		/* unhash inode and return to free list if should not 
		 * be reactivated
		 */
		iunhash(ip);
		return 0;
}

/*
 * NAME:	iclose (ip)
 *
 * FUNCTION:	Close processing for a segment when i_count becomes
 *		zero. this procedure is called from iput.
 *
 *		(1) if the i_nlink field is zero, the resources 
 *		associated with the inode are freed, including the 
 *		inode itself. 
 *
 *		(2) if it is a compressed file which has been
 *		modified, i/o is initiated to write it out,
 *		provided the flag vmker.noflush is 0.
 *
 * PARAMETERS:	ip	- pointer to inode to close
 *
 * RETURN :	errors from subroutines
 *			
 * SERIALIZATION: called only by iput() with ICACHE_LOCK() and 
 *		  IXLOCK on the inode
 */
static
iclose (ip)
struct inode *ip;
{
	int sid, rc , sr12save;
	label_t jbuf;

	/* clean up resources for inode with i_nlink = 0.
	 * permanent disk map and inode map should already have
	 * been taken care of by ctrunc() (i.e, at commit() with
	 * i_nlink = 0) or by itrunc().
	 */
	if (ip->i_nlink == 0)
	{
		/* mark the inode to be skipped in logsync()
		 */
		ip->i_cflag |= ICLOSE;

		ICACHE_UNLOCK();

		if (ip->i_seg != 0)
		{
			sr12save = mfsr(12);
			if (rc = setjmpx(&jbuf))
			{
				(void)chgsr(12,sr12save);
				ICACHE_LOCK();
				return rc;
			}

			/* free disk blocks from the working disk map and 
			 * free indirect block pages.
			 */
			ctrunc1(ip,0);

			/* delete vm segment.
			 */
			isegdel(ip);

			/* reestablish state
			 */
			clrjmpx(&jbuf);
		}	

		/* free any incore security stuff */
		sec_free(ip);

		/* update disk inode usage quota to reflect freed inode.
		 */
		freeiq(ip,1);

		/* free the disk inode from the working inode map.
		 */
		inofree(ip,ip->i_number);

		ICACHE_LOCK();

		return 0;
	}

	/* abandon updates if mapped deferred update
	 */
	if (ip->i_flag & IDEFER && ip->i_seg)
	{
		ICACHE_UNLOCK();
		isegdel(ip);
		ICACHE_LOCK();
		return(0);
	}

	/* write out compressed modified files
	 */
	if (ip->i_compress && vmker.noflush == 0 && ip->i_seg)
	{
		ICACHE_UNLOCK();
		if ((ip->i_flag & IUPD) || ismodified(ip))
			iflush(ip);
		ICACHE_LOCK();
	}

	return 0;
}


/*
 * NAME:	iunhash (ip)
 *
 * FUNCTION:	remove inode from inode cache
 *
 * PARAMETERS:	ip	- inode to blast
 *
 * RETURN :	0 - if no sleeps
 *		1 - if sleeps
 *
 * SERIALIZATION: ICACHE_LOCK() held on entry/exit.
 */

iunhash (ip)
struct inode *ip;
{
	int rc;
	struct gnode	*gp;
	struct vnode	*vp;

	/* free vnode
	 */
	vput(ip);

	/* remove from hash list
	 */
	REMOVE_HASH(ip);
	
	/* free dquots.
	 */
	rc = putinoquota(ip);

	/* free locks
	 */
	if (ip->i_locks & ILOCKALLOC)
	{
#ifdef _I_MULT_RDRS
		lock_free(&ip->i_rdwrlock);
		lock_free(&ip->i_nodelock);
#else /* simple lock */
		lock_free(&ip->i_rdwrlock);
#endif /* _I_MULT_RDRS */
	}
	if (ip->i_locks & GLOCKALLOC)
	{
		gp = ITOGP(ip);
		lock_free(&gp->gn_reclk_lock);
	}
	if (ip->i_locks & VLOCKALLOC)
	{
		vp = ip->i_gnode.gn_vnode;
		lock_free(&vp->v_lock);
	}

	/* insert on free list
	 */
	ip->i_forw = ip->i_back = NULL;
	ip->i_dev = 0xdeadbeef;
	ip->i_number = 0;
	ip->i_mode = 0;
	ip->i_count = 0;
	ip->i_locks = 0;
	ip->i_flag = 0;
	ip->i_seg = 0;
	ip->i_movedfrag = 0;
	ip->i_compress = 0;
	ip->i_cluster = 0;

	genfree(ino_ap, ip->i_gnode.gn_vnode);

	return rc;
}


/*
 * NAME:	i_sync()
 *
 * FUNCTION:	commits all regular files which have not been committed 
 *		since the last time jfs_sync() was invoked. 
 *
 * SERIALIZATION: guarantee to not to interfere by deferring the inode 
 *		with work in progress  
 */

Simple_lock	jfs_sync_lock;

void
i_sync()
{
	struct hinode *hip;
	struct inode *ip, *ipnext;
	struct gnode *gp;
	struct inode	*isyncip();

	SYNC_LOCK();

	/* visit each inode hash list and process the inodes.
	 */
	for (hip = hinode; hip < &hinode[nhino]; hip++)
	{
		/* secure the first inode
		 */
		ICACHE_LOCK();
		ip = isyncip(hip,hip);
		ICACHE_UNLOCK();

		while (ip != (struct inode *)hip)
		{
			if (IWRITE_LOCK_TRY(ip))
			{
				/* commit it if it is modified
				 */
				if (ip->i_seg &&
				    (ip->i_flag & IUPD ||
				     (ITOGP(ip)->gn_flags & GNF_WMAP &&
				      ismodified(ip))))
					ip->i_flag |= IFSYNC;

				if (ip->i_flag & (ICHG|IACC|IFSYNC))
					commit(1,ip);

				IWRITE_UNLOCK(ip);
			}

			ICACHE_LOCK();

			/* secure the next inode */
			ipnext = isyncip(ip, hip);

			/* release the current inode */
			iput(ip, NULL);

			ICACHE_UNLOCK();

			ip = ipnext;
		}
	}

	SYNC_UNLOCK();
}

/*
 *	isyncip()
 *
 * return inode to sync
 *
 * SERIALIZATION: called only by isync()
 *		  SYNC_LOCK()/ICACHE_LOCK() held on entry/exit.
 */
static 
struct inode *
isyncip(xip,hip)
struct inode *xip;
struct inode *hip;
{
	struct inode	*ip;

	ip = xip->i_forw;

	while (ip != hip)
	{
		if (!(ip->i_locks & IXLOCK) &&
		    ip->i_mode != 0 &&
		    ip->i_nlink != 0 &&
		    !ISPECIAL(ip) &&
		    !(ip->i_flag & IDEFER))
		{
			/* guarantee to stay put in hashlist
			 * (and stay out of cachelist not to be recycled)
			 */
			ip->i_count++;
			if (ip->i_count == 1)
			{
				cachesize--;
				REMOVE_CACHE(ip);
				NULL_CACHE(ip);
			}

			break;
		}

		ip = ip->i_forw;
	}

	return(ip);
}


/*
 * NAME:	ilogsync()
 *
 * FUNCTION:	initiates i/o for all modified journalled pages which 
 *		can be written to their home address or marks those 
 *		which can not so that they will be written when they 
 *		are committed. 
 *
 *		if iplog != NULL, sync only those inodes associated with log.
 *		if iplog == NULL, sync all inodes.
 *
 * return value - none
 *
 */

ilogsync(iplog)
struct inode *iplog;
{
	struct hinode	*hip;
	struct inode	*ip, *ipnext;
	struct inode	*ilogsyncip();

	/* visit each inode hash list and process the inodes.
	 */
	for (hip = hinode; hip < &hinode[nhino]; hip++)
	{
		/* secure the first inode
		 */
		ICACHE_LOCK();
		ip = ilogsyncip(hip, hip, iplog);
		ICACHE_UNLOCK();

		while (ip != (struct inode *)hip)
		{
			ip->i_cflag &= ~DIRTY;

			vcs_sync(ip, NULL);

			ICACHE_LOCK();

			/* secure the next inode */
			ipnext = ilogsyncip(ip, hip, iplog);

			/* release the current inode */
			iput(ip, NULL);

			ICACHE_UNLOCK();

			ip = ipnext;
		}
	}

	/* determine if we have to advance the
	 * sync point for all the logs or just
	 * a particular log
	 */
	if (iplog)
		vcs_sync(NULL, iplog);
	else
		vcs_sync(NULL, NULL);

	return 0;
}

/*
 *	ilogsyncip()
 *
 * return inode to logsync
 *
 * acquire reference of the inode to log sync so that getip()
 * will yield the inode to log sync 
 */
static 
struct inode *
ilogsyncip(xip, hip, iplog)
struct inode *xip;
struct inode *hip;
struct inode *iplog;
{
	struct inode	*ip;

	ip = xip->i_forw;

	while (ip != hip)
	{
		/* do we sync all inodes? or
		 * just those associated with this log?
		 */
		if (iplog != NULL)
		{
			if (iplog != ip->i_ipmnt->i_iplog)
				goto next;
		}

		/* before we sync the inode, make a few checks:
		 * 1. is this a journalled inode?
		 * 2. is this inode marked dirty?
		 * 3. is this inode currently being closed from an iput()
		 *    through iclose().
		 * 4. is this inode currently being iuncached and iunhashed
		 *    in iactivity().
		 */
		if ((ip->i_mode & IFJOURNAL) &&
		    (ip->i_cflag & DIRTY) &&
		    !(ip->i_cflag & ICLOSE) &&
		    !(ip->i_ipmnt->i_cflag & IACTIVITY))
		{
			/* guarantee to stay put in hashlist
			 * (and stay out of cachelist not to be recycled)
			 */
			ip->i_count++;
			if (ip->i_count == 1)
			{
				cachesize--;
				REMOVE_CACHE(ip);
				NULL_CACHE(ip);
			}

			break;
		}
next:
		ip = ip->i_forw;
	}

	return(ip);
}


/*
 * NAME:	iactivity (dev, forced)
 *
 * FUNCTION:	check if file system is quiescent to unmount, and
 *		clean up inodes of file system being unmounted.
 *
 * PARAMETERS:	dev	- device to check for activity
 *		forced  - forced unmount
 *
 * RETURN :	0	- success
 *		EBUSY	- busy
 *
 * SERIALIZATION: guarantee to remove the inode from inode cache
 *		by waiting on the inode if work in progress
 *
 *		higher layer serializes mount/umount/path name
 *		translation/file handle translation (i.e., no
 *		race with _iget()).
 */

iactivity (dev, forced)
dev_t dev;			/* device being unmounted */
int forced;			/* boolean (!0=>shutting down) */
{
	int rc = 0;
	struct hinode *hip;
	struct inode *ip, *ipnext, *ipmnt;
	int	iactivityip();

	SYNC_LOCK();

	ICACHE_LOCK();

	/* mark the mount inode as unmounting. this 
	 * flag is checked in ilogsyncip() and all inodes,
	 * in the file system which is being unmounted, are
	 * skipped.
	 */
	ipmnt = ifind(dev);
	ipmnt->i_cflag |= IACTIVITY;

	/* visit each inode hash list and process the inodes.
	 */
	for (hip = hinode; hip < &hinode[nhino]; hip++)
	{
		/* secure the first inode of hashlist
		 */
		ipnext = (struct inode *)hip;
		if (rc = iactivityip(&ipnext, hip, dev, forced))
		{
			ipmnt->i_cflag &= ~IACTIVITY;
			ICACHE_UNLOCK();
			SYNC_UNLOCK();
			return rc;
		}

		ip = ipnext;
		while (ip != (struct inode *)hip)
		{
			/* secure the next inode */
			if (rc = iactivityip(&ipnext, hip, dev, forced))
			{
				ipmnt->i_cflag &= ~IACTIVITY;
				ICACHE_UNLOCK();
				SYNC_UNLOCK();
				return rc;
			}

			/* close the current inode.
			 */
			if (ip->i_mode)
				iuncache(ip);

			iunhash(ip);
			ip = ipnext;
		}
	}

	ICACHE_UNLOCK();

	SYNC_UNLOCK();

	return 0;
}

/*
 *	iactivityip()
 *
 * return inode to free
 */
static 
iactivityip(xip, hip, dev, forced)
struct inode **xip;
struct inode *hip;
dev_t	dev;
int	forced;
{
	struct inode	*ip = *xip;
	struct inode	*iplast;

	/* save the inode we are currently holding
	 */
	iplast = ip;
loop:
        for (ip = ip->i_forw; ip != hip; ip = ip->i_forw)
        {
		if (ip->i_dev != dev)
			continue;

		/* meta-file inodes acquire single/only reference 
		 * at mount time, and remains active to be released 
		 * at end of umount time.
		 */
		if (ISPECIAL(ip) && ip->i_count == 1)
			continue;

		/* root inode of quiescent fs should have a single reference
		 * by the vfs to be released at end of umount time.
		 */
		if (ip->i_number == ROOTDIR_I &&
		    ip->i_count == 1)
			continue;

		/* increment the count on the inode to
		 * show we have interest in it. thus 
		 * guaranteeing the inode will remain on
		 * the hash list
		 */
		ip->i_count++;
			
		/* wait for completion of deactivation (iuncache()/iclose())
		 */
		if (ip->i_locks & IXLOCK)
		{
			do
			{
				ip->i_locks |= IXWANT;
				e_sleep_thread(&ip->i_event, &jfs_icache_lock,
						LOCK_SIMPLE);
				if (ip->i_mode == 0)
				{
					iput(ip, NULL);
					ip = iplast;
					goto loop;
				}
			}
			while (ip->i_locks & IXLOCK);
		}

		if (ip->i_count > 1 && !forced) 
		{
			/* check if quota file inode.
			 */
			if (dqactivity(ip))
			{
				iput(ip, NULL);
				continue;
			}
			iput(ip, NULL);
			return EBUSY;
		}

		/* remove from cachelist
	 	  */
		if (ip->i_count == 1)
		{
			cachesize--;
			REMOVE_CACHE(ip);
			NULL_CACHE(ip);
		}

		break;
        }

	*xip = ip;
        return 0;
}


/*
 * NAME:	dev_ialloc (pip, ino, mode, vfsp, ipp)
 *
 * FUNCTION:	Allocate new in-memory+on-disk inode
 *
 * PARAMETERS:	pip	- parent inode
 *		ino	- parent inode number
 *		mode	- rwx and IFMT mode
 *		vfsp	- vfs pointer
 *		ipp	- Returned inode
 *
 * RETURN:	If the routine fails an error number(errno) is 
 *		returned indicating failure.
 *
 * NOTE:	The inode allocated is locked upon return.
 */

dev_ialloc (pip, ino, mode, vfsp, ipp)
struct inode	*pip;			/* parent inode */
ino_t		ino;			/* Locate near this inode number */
mode_t		mode;			/* File	mode		*/
struct vfs	*vfsp;			/* vfs pointer		*/
struct inode	**ipp;			/* inode to return	*/
{
	int rc;				/* Return code		*/
	struct inode *ip;		/* varying inode	*/
	struct gnode *gp; 
	struct timestruc_t t;
	struct hinode *hip;
	
	*ipp = NULL;

	/* allocate on-disk inode.
	 */
	if (rc = ialloc(pip, &ino))
		return rc;

	IHASH(ino, pip->i_dev, hip);

	/* allocate in-memory inode
	 */
	ICACHE_LOCK();

	/* check to see if there were any
	 * vgets done on this hash chain.
	 * the count is incremented and decremented
	 * in jfs_vget(). if a vget was done on the
	 * hash we must call _iget() to rescan. 
	 */
	if (hip->hi_vget != 0)
		rc = _iget(pip->i_dev, ino, hip, &ip, 1, vfsp);
	else
		rc = _iget(pip->i_dev, ino, hip, &ip, 0, vfsp);

	ICACHE_UNLOCK();
	sysinfo.iget++;
	cpuinfo[CPUID].iget++;

	if (rc)
	{	
		inofree(pip, ino);
		return rc;
	}

	ASSERT(ip->i_nlink == 0 && ip->i_mode == 0);
	
	ip->i_nlink = 1;
	ip->i_mode = mode;
	if ((ip->i_mode & IFMT) == IFDIR) 
		ip->i_mode |= IFJOURNAL;

	/* set update times directly to avoid extra commit in iput()
	 */
	curtime(&t);
	ip->i_atime_ts = t;
	ip->i_mtime_ts = t;
	ip->i_ctime_ts = t;

	ip->i_flag = ICHG;

	gp = ITOGP(ip);

	switch (gp->gn_type = IFTOVT(ip->i_mode))
	{
		case VCHR:
		case VBLK:
			gp->gn_rdev = ip->i_rdev;
			break;

		default:
			gp->gn_rdev = ip->i_dev;
			break;
	}

	/* lock the inode to return
	 */
	IWRITE_LOCK(ip);
	*ipp = ip;

	return 0;
}


/*
 * NAME:	vget(ip, vfsp)
 *
 * FUNCTION:	initialize vnode of the inode at first _iget(). 
 *
 *	vfsp != NULL:
 *	initialize v_count = 1 and link vnode to vfs (v_vfsp = vfsp);
 *
 *	vfsp == NULL: 
 *	. inode does not have vnode (meta-inode);
 *	. inode is non-device mount object (vnode management is provided by 
 *	  iptovp()/vn_get()):
 *	. non-root object in dir-over-dir mount being accessed in mounted fs 
 *	  before being accessed in underlying fs
 *	  (covered inode of root of file-over-file or dir-over-dir mount is 
 *	  always accessed as part of mount).
 *	initialize v_count = 0 and leave vnode unlinked	(v_vfsp = NULL).
 *
 * SERIALIZATION: ICACHE_LOCK held on entry/exit 
 */
static
vget(ip, vfsp)
struct inode	*ip;
struct vfs	*vfsp;
{
	struct vnode	*vp = ip->i_gnode.gn_vnode;

	assert(vp->v_vfsp == NULL);

	vp->v_count = 0;
	vp->v_flag = 0;
	vp->v_vfsp = vfsp;
	vp->v_mvfsp = NULL;

	if (vfsp == NULL)
		return;

	vp->v_count = 1;
	vp->v_audit = NULL;
	vp->v_vfsgen = 0;

	if (!(ip->i_locks & VLOCKALLOC))
	{
		lock_alloc(&vp->v_lock,LOCK_ALLOC_PAGED,VNODE_LOCK_CLASS,
		   vlock_cnt++);
		simple_lock_init(&vp->v_lock);
		ip->i_locks |= VLOCKALLOC;
	}

	/* insert into vfs vnodelist */
	vp->v_vfsnext = vfsp->vfs_vnodes;
	vfsp->vfs_vnodes = vp;
	vp->v_vfsprev = NULL;
	if (vp->v_vfsnext != NULL)
		vp->v_vfsnext->v_vfsprev = vp;
}


/*
 * NAME:	vput(ip)
 *
 * FUNCTION:	finalize vnode of the inode at iunhash()
 *
 *	vfsp != NULL:
 *	unlink vnode from vfs (v_vfsp = vfsp);
 *
 *	vfsp == NULL: 
 *	. inode does not have vnode (meta-inode);
 *	. inode is non-device mount object (vnode management is provided 
 *	  by vn_free());
 *	. vnode of inode not initialized yet (inode initialization failure);
 *	 vnode is already freed 
 *		(iuncache/iunhash sequence),
 *
 * SERIALIZATION: ICACHE_LOCK held on entry/exit 
 */
static
vput(ip)
struct inode	*ip;
{
	struct vnode	*vp = ip->i_gnode.gn_vnode;
	struct vfs	*vfsp = vp->v_vfsp;

	if (vfsp == NULL)
		return;

	/* remove from vfs vnode list */
	if (vp == vfsp->vfs_vnodes)
		vfsp->vfs_vnodes = vp->v_vfsnext;
	if (vp->v_vfsnext != NULL)
		vp->v_vfsnext->v_vfsprev = vp->v_vfsprev;
	if (vp->v_vfsprev != NULL)
		vp->v_vfsprev->v_vfsnext = vp->v_vfsnext;

	vp->v_vfsp = NULL;

	vp->v_next = NULL;
}


/*
 * NAME:	iptovp (vfsp, ip, vpp)
 *
 * FUNCTION:	get a vnode give an non-device mount inode.  
 *		Checks for the vnode in the specified vfs.
 *		If not there create one and initialize the gnode,
 *		vnode and vfsp linkage.
 *
 * PARAMETERS:	vfsp	- virtual file system where we want this
 *			  vnode to live
 *		ip	- inode to create from
 *		vpp	- return newly created vnode
 *
 * RETURN :	ENOMEM	- if no space for new vnode
 *		else return zero
 *			
 * SERIALIZATION:
 *	IWRITE_LOCK serializes v_count of softmount fs vnodes.
 *	gnode vnodelist is serialized by IWRITE_LOCK AFTER
 *	it has been initialized by _iget()).
 */
 
iptovp(vfsp, ip, vpp)
struct vfs	*vfsp;
struct inode	*ip;
struct vnode	**vpp;
{
	int rc;
	struct vnode *vp;
	struct gnode *gnp = ITOGP(ip);
 
	*vpp = NULL;

	IWRITE_LOCK(ip);

	/* scan gnode vnodelist
	 */
	for (vp = gnp->gn_vnode; vp; vp = vp->v_next)
		if (vp->v_vfsp == vfsp)
		{
			vp->v_count++;
			IWRITE_UNLOCK(ip);

			ICACHE_LOCK();
			iput(ip, NULL);
			ICACHE_UNLOCK();

			*vpp = vp;
			return 0;
		}

	/* allocate a new vnode
	 */
	if ((rc = vn_get(vfsp, ITOGP(ip), &vp)) == 0)
	{
		vp->v_flag = (ip->i_number == ROOTDIR_I) ? V_ROOT : 0;
		*vpp = vp;
	}

	IWRITE_UNLOCK(ip);

	if (rc)
	{
		ICACHE_LOCK();
		iput(ip, NULL);
		ICACHE_UNLOCK();
	}

	return rc;
}
