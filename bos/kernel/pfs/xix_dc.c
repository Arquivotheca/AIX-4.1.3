static char sccsid[] = "@(#)07	1.13.1.8  src/bos/kernel/pfs/xix_dc.c, syspfs, bos41J, 9507C 2/14/95 13:44:35";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: dnlc_delete, dnlc_enter, dnlc_init,
 *            dnlc_lookup, dnlc_purge, dnlc_search
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

#include "jfs/jfslock.h"	/* contains # of initially allocated inodes */
#include "jfs/fsvar.h"		/* contains # of initially allocated inodes */
#include "sys/param.h"		/* Get system parameters		*/
#include "sys/dir.h"		/* contains dname_t typedef		*/
#include "sys/malloc.h"		/* contains dname_t typedef		*/

/*
 * Stats on usefulness of directory name lookup cache
 */
struct {
	int	hits;		/* hits that we can really use */
	int	misses;		/* cache misses */
	int	enters;		/* number of enters done */
	int	deletes;	/* number of deletes done */
	int	purges;		/* number of purges of cache */
	int	nam2long;	/* number of names > DCNAM */
} dc_stats;

# define	dc_hits		dc_stats.hits++
# define	dc_misses	dc_stats.misses++
# define	dc_enters	dc_stats.enters++
# define	dc_deletes	dc_stats.deletes++
# define	dc_purges	dc_stats.purges++
# define	dc_nam2long	dc_stats.nam2long++

/*
 *  directory name lookup cache (dnlc)
 */
# define	DCNAM 32		/* Max cached name size		*/

typedef struct dcbuf {
	struct dcbuf *dc_forw;	/* .. next buffer in hash list		*/
	struct dcbuf *dc_back;	/* .. last buffer in hash list		*/
	struct dcbuf *dc_next;  /* .. next buffer in lru		*/
	struct dcbuf *dc_prev;  /* .. previous buffer in lru	        */
	dev_t dc_pdev;		/* .. parent directory device #		*/
	ino_t dc_pino;		/* .. parent directory inode #		*/
	ino_t dc_ino;		/* .. cached inode #			*/
	int dc_namelen;		/* .. name length			*/
	char dc_name[DCNAM];	/* .. component name			*/
} dcbuf_t;

dcbuf_t *dc_cache;			/* dnlc entry pool */
int dc_cachesize;			/* number of cache entries */
int dc_initents;			/* initial number of cache entries */

/*
 *	dnlc hash list anchor table
 * 
 *	The dnlc hash list anchor table contains dc_hashsize elements.
 *	dc_hashsize is scaled by the size of real memory.
 */
typedef struct dchash {
	dcbuf_t	*hash_forw;
	dcbuf_t	*hash_back;
} dchash_t;

dchash_t *dc_hash;			/* dnlc hash list anchor table */
int dc_hashsize;			/* number of hash list anchors */

/*
 *	dnlc LRU cache list anchor
 * 
 * LRU list of cache entries for aging. 
 * Stubbed to be like dcbuf_t to allow for use of INSERT_CACHE() and 
 * REMOVE_CACHE(). Initialize as null hash.
 */
struct	dc_lru	{
	dcbuf_t *lru_forw;		/* hash chain, unused */
	dcbuf_t *lru_back;		/* hash chain, unused */
	dcbuf_t *lru_next;		/* LRU chain */
	dcbuf_t	*lru_prev;		/* LRU chain */
} dc_lru = {NULL, NULL, (dcbuf_t *)&dc_lru, (dcbuf_t *)&dc_lru};

/* dnlc queue management macro */
#define INSERT_HASH(e, after)	insque(e, after)
#define REMOVE_HASH(e)		remque(e)
#define	NULL_HASH(e)				\
	((dcbuf_t *)(e))->dc_forw = 		\
	((dcbuf_t *)(e))->dc_back = ((dcbuf_t *)(e))

#define INSERT_CACHE(e, after)	insque2(e, after)
#define REMOVE_CACHE(e)		remque2(e)
#define	NULL_CACHE(e)				\
	((dcbuf_t *)(e))->dc_next = 		\
	((dcbuf_t *)(e))->dc_prev = ((dcbuf_t *)(e))

/* pearson's string hash algorithm random permutation table
 */
unsigned char table[256] = {
0x8d, 0x23, 0x12, 0x6b, 0x4b, 0x1f, 0x4e, 0x05,
0x54, 0xcb, 0xbd, 0xd4, 0xb3, 0x69, 0xe1, 0x91,
0x7e, 0xaf, 0x31, 0x6e, 0x56, 0x72, 0x83, 0x8a,
0x67, 0x66, 0x36, 0x96, 0xaa, 0x42, 0x73, 0x3d,
0x29, 0xc1, 0xde, 0xe8, 0xa7, 0xfd, 0xa1, 0xd8,
0x1e, 0x41, 0x13, 0x2f, 0x37, 0xae, 0xb4, 0x97,
0xda, 0x6c, 0x75, 0xfc, 0xcc, 0xe3, 0x7d, 0x76,
0x8e, 0x02, 0xa0, 0x61, 0x8f, 0x63, 0x17, 0x0f,
0xee, 0x8c, 0x78, 0x4d, 0xf7, 0xfa, 0xc4, 0x48,
0x6a, 0x53, 0x86, 0x38, 0xdc, 0x08, 0x4f, 0xc8,
0xff, 0x21, 0xf8, 0x5e, 0xea, 0x07, 0x28, 0x7f,
0x58, 0x93, 0x2e, 0xc5, 0x8b, 0x4c, 0x50, 0xcd,
0xb6, 0x99, 0x2b, 0x80, 0x74, 0xd3, 0xf9, 0xbc,
0xa8, 0x9f, 0x20, 0x34, 0xc2, 0xd0, 0x22, 0x33,
0xd6, 0x01, 0xe7, 0x3c, 0xa9, 0x79, 0x1d, 0x81,
0x9c, 0xe5, 0xbe, 0x95, 0x89, 0x3e, 0xc9, 0xb1,
0x46, 0x5f, 0x88, 0x87, 0x45, 0xef, 0xac, 0xad,
0x6f, 0xb2, 0x47, 0xf6, 0xf2, 0xa2, 0x5a, 0x18,
0xb8, 0x9a, 0xe9, 0x2d, 0x39, 0x15, 0x59, 0x5b,
0x98, 0x1b, 0x16, 0xc0, 0xb7, 0xa5, 0x24, 0xce,
0x06, 0x25, 0x0a, 0x51, 0x70, 0xdd, 0xed, 0x40,
0x71, 0xd1, 0x3f, 0x1a, 0x03, 0xc3, 0xfb, 0xbf,
0xcf, 0x0e, 0x94, 0xdb, 0xca, 0xe0, 0x27, 0xf0,
0xf3, 0x11, 0x5c, 0xc6, 0x9d, 0xbb, 0x35, 0x85,
0xab, 0x14, 0x77, 0xf5, 0xd5, 0xec, 0x30, 0xb0,
0x6d, 0xa3, 0x65, 0xba, 0xb5, 0xc7, 0x9e, 0x3a,
0x04, 0x4a, 0x44, 0x90, 0xdf, 0x7a, 0x92, 0xf1,
0x0b, 0xe2, 0xa6, 0x7c, 0x60, 0x0c, 0x55, 0x0d,
0x57, 0x2a, 0x9b, 0x84, 0xf4, 0x19, 0xd2, 0x10,
0x7b, 0xe4, 0x62, 0xd7, 0x00, 0x5d, 0xe6, 0x2c,
0x49, 0x09, 0x43, 0x68, 0x52, 0xd9, 0x3b, 0x1c,
0x32, 0xfe, 0xeb, 0x82, 0x26, 0xb9, 0x64, 0xa4
};

/* name cache hashing algorithm
 * pearson's string hash algorithm
 */
#define NHASH(pdev, pino, name, namelen, hp)	\
{						\
	int i, c, even, odd;			\
	caddr_t cp;				\
						\
	even = odd = 0;				\
	cp = name;				\
	while ((c = *cp++) != 0)		\
	{					\
		even = table[even ^ c];		\
		if ((c = *cp++) == 0)		\
			break;			\
		odd = table[odd ^ c];		\
	}					\
	i = (even<<8) | odd;			\
	i += (int)pdev + (int)pino + namelen;	\
	i &= dc_hashsize - 1;			\
	hp = dc_hash + i;			\
}

/*
 *	dnlc serialization
 *
 *	The name cache is serialized with a simple lock.
 */
Simple_lock	jfs_ncache_lock;

/*
 *	dnlc exported interface wrappers 
 * 
 * the following wrappers (dc_) hide dnlc serialization
 */

/*
 *	dc_lookup (pdev, pino, nmp)
 */
ino_t
dc_lookup (pdev, pino, nmp)
dev_t pdev;		/* Parent directory dev # */
ino_t pino;		/* Parent directory ino # */
dname_t *nmp;		/* Pathname arg */
{
	ino_t	rc;
	ino_t	dnlc_lookup();

	rc = (ino_t) dnlc_lookup(pdev, pino, nmp);
	return rc;
}

/*
 *	dc_enter (pdev, pino, nmp, ino)
 */
void
dc_enter (pdev, pino, nmp, ino)
dev_t pdev;		/* Parent directory dev # */
ino_t pino;		/* Parent directory ino # */
dname_t *nmp;		/* Pathname arg */
{
	void	dnlc_enter();

	(void) dnlc_enter(pdev, pino, nmp, ino);
}

/*
 *	dc_delete (pdev, pino, nmp)
 */
void
dc_delete (pdev, pino, nmp)
dev_t pdev;		/* Parent directory dev # */
ino_t pino;		/* Parent directory ino # */
dname_t *nmp;		/* Pathname arg */
{
	void	dnlc_delete();

	(void) dnlc_delete(pdev, pino, nmp);
}

/*
 *	dc_purge (dev)
 */
void
dc_purge (dev)
dev_t dev;	
{
	void	dnlc_purge();

	(void) dnlc_purge(dev);
}

/*
 * NAME:	dnlc_init ()
 *
 * FUNCTION:	initialize dnlc at jfs initialization.
 *
 * PARMETERS:	None
 *
 * RETURN VALUE:  None
 *
 */
void
dnlc_init()
{
	dcbuf_t *dcp;
	dchash_t *hp;
	int i, k;
	uint memsize;
	extern struct fsvar fsv;
	extern uint pfs_memsize();

	/* compute hash list anchor table size
	 * scale the number of hash buckets with
	 * the size of memory
	 */
	memsize = pfs_memsize();

	/* round up memsize to the nearest Meg */
	memsize = (memsize + ((1024*1024)-1) & ~(1024*1024));

	if (memsize < 64*1024*1024)
	{
		dc_hashsize = 512;
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
		dc_hashsize =  (1 << (31-i)) * 512;
	}

	/* compute dnlc pool size:
	 * allocate one page/Mbyte of real memory for dnlc pool
	 */
	dc_cachesize = (memsize / 0x100000) * (PAGESIZE/sizeof(dcbuf_t));

	/* Allocate dnlc initial entries for phase 1 IPL
	 * (one dnlc entry is allocated for each initial inode).
	 * The remaining entries will be allocated by dnlc_grow
	 * once paging space is defined.  
	 */
	dc_initents = fsv.v_ninode;

	dc_hash = (dchash_t *) malloc (dc_hashsize * sizeof (dchash_t));
	dc_cache = (dcbuf_t *) malloc (dc_cachesize * sizeof (dcbuf_t));
	
	/* insert initial entries on the lru cachelist in lifo
	 */
	for (i = 0; i < dc_initents; i++)
	{
		dcp = dc_cache + i;
		dcp->dc_ino = 0;
		dcp->dc_namelen = 0;
		NULL_HASH(dcp);
		INSERT_CACHE(dcp, &dc_lru);
	}

	/* Null hash the hashlist anchors
	 */
	for (i = 0; i < dc_hashsize; i++)
	{
		hp = dc_hash + i;
		NULL_HASH(hp);
	}

	/* Initialize the dnlc lock
	 */
        lock_alloc(&jfs_ncache_lock,LOCK_ALLOC_PAGED,NCACHE_LOCK_CLASS,-1);
        simple_lock_init(&jfs_ncache_lock);
}

/*
 * NAME:	dnlc_grow
 *
 * FUNCTION:	expand the size of the dnlc after phase 1 IPL is completed
 *		ie paging space has been defined.
 *
 * RETURNS:
 *	None
 */
void
dnlc_grow()
{
	int i;
	dcbuf_t *dcp;

	NCACHE_LOCK();

	/* initialize the remaining dnlc entries
	 */
	dcp = &dc_cache[dc_initents];
	bzero(dcp, sizeof(dcbuf_t) * (dc_cachesize - dc_initents));

	/* insert the remaining dnlc entries on to the lru cachelist in lifo
	 */
	for (i = dc_initents ; i < dc_cachesize ; i++) 
	{
		NULL_HASH(dcp);
		INSERT_CACHE(dcp, &dc_lru);
		dcp++;
	}

	NCACHE_UNLOCK();
}

/*
 * NAME:	dnlc_search()
 *
 * FUNCTION:	This function is an internal interface to directory
 *		cache management to return a pointer to a dcbuf_t.
 *
 * PARMETERS:	pdev	- parent dev number
 *		pino	- parent inode number
 *		name	- component name
 *		namelen	- component name length
 *
 * RETURN VALUE: dcbuf_t if it exists
 *
 * SERIALIZATION: The NCACHE_LOCK needs to be held upon entry.
 *
 */
static
dcbuf_t *
dnlc_search (pdev, pino, name, namelen)
dev_t	pdev;			/* parent directory dev */
ino_t	pino;			/* parent directory inode # */
caddr_t	name;			/* Name to hash	*/
int namelen;			/* Name length */
{
	dcbuf_t *dcp; 			/* Ptr to cache buffer		*/
	dchash_t *hp;			/* Ptr to hashlist pointer */

	/* Check for names too long */
	if (namelen > sizeof (dcp->dc_name))
	{
		dc_nam2long;
		return NULL;
	}

	/* find the hash bucket for this name
	 */
	NHASH(pdev, pino, name, namelen, hp);

	/* now search the subpool
	 */
	for (dcp = hp->hash_forw; dcp != (dcbuf_t *) hp; dcp = dcp->dc_forw)
		if (dcp->dc_pino == pino
		    && dcp->dc_pdev == pdev
		    && dcp->dc_namelen == namelen 
		    && *name == *dcp->dc_name
		    && memcmp(dcp->dc_name, name, namelen) == 0)
		{
			/*
			 * cache hit
			 *
			 * move it to the head of hashlist
			 */
			REMOVE_HASH(dcp);
			INSERT_HASH(dcp, hp);
			return dcp;
		}

	return NULL;
}

/*
 * NAME:	dnlc_lookup ()
 *
 * FUNCTION:	This function is an external interface to directory
 *		cache management.  If a matching name is found in the
 *		directory cache the associated file handle is returned.
 *
 * PARMETERS:	pdev	- parent dev number
 *		pino	- parent inode number
 *		nmp	- Pathname argument
 *
 * RETURN VALUE: 0 - name not found
 *		 1 - name found *fhp initialized
 *
 */
ino_t
dnlc_lookup (pdev, pino, nmp)
dev_t pdev;		/* Parent directory dev # */
ino_t pino;		/* Parent directory ino # */
dname_t *nmp;		/* Pathname arg */
{
	dcbuf_t *dcp; 		/* Ptr to cache buffer	*/
	ino_t ino = 0;		/* Return value		*/

	NCACHE_LOCK();

	dcp = dnlc_search(pdev, pino, nmp->nm, nmp->nmlen);
	if (dcp)
	{
		/*
		 * cache hit:
		 *
		 * move to tail of LRU cachelist
		 */
		REMOVE_CACHE(dcp);
		INSERT_CACHE(dcp, dc_lru.lru_prev);

		ino = dcp->dc_ino;
		dc_hits;
	}
	else
		dc_misses;

	NCACHE_UNLOCK();

	return ino;
}

/*
 * NAME:	dnlc_enter ()
 *
 * FUNCTION:	This function is an external interface to directory
 *		cache management.  If a matching name is found in the
 *		directory cache the entry is moved to the front of hash list.
 *		If name not found least recently used dcbuf_t is retrieved
 *		and initialized to reflect current target.
 *
 * PARMETERS:	pdev	- parent dev number
 *		pino	- parent inode number
 *		nmp	- Pathname argument
 *		ino	- inode number to cache
 *
 * NOTE: This function is only called by jfs_lookup(). Since dnlc_lookup()
 *	 is called just prior to this function, and dnlc_search() is called
 *	 in dnlc_lookup, and the inode is locked, we need not do an
 * 	 extra search. We currently have code in the JFS which could enable
 *	 compex inode locking for readers/writers. If we enable this code,
 *	 then the serialization is wrong.
 */
void
dnlc_enter (pdev, pino, nmp, ino)
dev_t pdev;		/* Parent directory dev # */
ino_t pino;		/* Parent directory ino # */
dname_t *nmp;		/* Pathname arg */
int ino;		/* inode number */
{
	dcbuf_t *dcp;	/* Ptr to cache buffer */
	dchash_t *hp;	/* Ptr to hash list */

	NCACHE_LOCK();

	dc_enters;

	/* recycle least recently used entry at head of LRU cachelist */
	dcp = dc_lru.lru_next;

	/* move to tail of LRU cachelist
	 */
	REMOVE_CACHE(dcp);
	INSERT_CACHE(dcp, dc_lru.lru_prev);

	dcp->dc_namelen = nmp->nmlen;
	dcp->dc_pdev = pdev;
	dcp->dc_pino = pino;
	dcp->dc_ino = ino;
	copyname(dcp->dc_name, nmp->nm, sizeof (dcp->dc_name));

	/* remove from previous hashlist and 
	 * insert at head of new hashlist
	 */
	NHASH(dcp->dc_pdev, dcp->dc_pino, dcp->dc_name, dcp->dc_namelen, hp);
	REMOVE_HASH(dcp);
	INSERT_HASH(dcp, hp);

	NCACHE_UNLOCK();
}

/*
 * NAME:	dnlc_delete ()
 *
 * FUNCTION:	This function is an external interface to directory
 *		cache management.  If a matching name is found in the
 *		directory cache the entry is moved to back of hash list
 *		and made available for re-use.
 *
 * PARMETERS:	pdev	- parent dev number
 *		pino	- parent inode number
 *		nmp	- Pathname argument
 *
 * RETURN VALUE:  None
 *
 */
void
dnlc_delete (pdev, pino, nmp)
dev_t pdev;		/* Parent directory dev # */
ino_t pino;		/* Parent directory ino # */
dname_t *nmp;		/* Pathname arg */
{
	dcbuf_t *dcp; 		/* Ptr to cache buffer		*/

	NCACHE_LOCK();

	dc_deletes;

	dcp = dnlc_search(pdev, pino, nmp->nm, nmp->nmlen);
	if (dcp)
	{
		/*
		 * cache hit:
		 *
		 * move to head of LRU cachelist
		 */
		REMOVE_CACHE(dcp);
		INSERT_CACHE(dcp, &dc_lru);

		/* remove from hashlist */
		REMOVE_HASH(dcp);
		NULL_HASH(dcp);
	}

	NCACHE_UNLOCK();
}

/*
 * NAME:	dnlc_purge (dev)
 *
 * FUNCTION:	scan dnlc and deactivate entry associated with the device
 *		being unmounted at umount() time.
 *
 * PARMETERS:	dev - device to purge
 *
 * RETURN VALUE:  None
 *
 */
void
dnlc_purge (dev)
dev_t dev;		/* Device being unmounted */
{
	dcbuf_t *dcp;		/* Ptr to cache buffs	*/
	dcbuf_t *nxt;		/* Ptr to cache buffs	*/
	dchash_t *hp;		/* Ptr to hash list */
	int i;			/* generic counter */

	NCACHE_LOCK();

	dc_purges;

	/* scan each hash list */
	for (i = 0; i < dc_hashsize; i++)
	{	
		hp = dc_hash + i;
		nxt = hp->hash_forw;

		/* scan each entry in the hash list */
		while (nxt != (dcbuf_t *)hp)
		{	
			nxt = (dcp = nxt)->dc_forw;

			if (dcp->dc_pdev == dev)
			{	
				/* Remove from hashlist
				 */
				dcp->dc_ino = dcp->dc_pino = 0;
				REMOVE_HASH(dcp);
				NULL_HASH(dcp);

 				/* move to head of LRU cachelist.
				 */
				REMOVE_CACHE(dcp);
				INSERT_CACHE(dcp, &dc_lru);
			}
		}
	}

	NCACHE_UNLOCK();
}
