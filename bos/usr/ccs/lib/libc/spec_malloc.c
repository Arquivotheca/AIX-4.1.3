static char sccsid[] = "@(#)18  1.1  src/bos/usr/ccs/lib/libc/spec_malloc.c, libcmem, bos41J, 9508A 1/24/95 11:38:00";
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: free, mallinfo, malloc, mallopt, realloc
 *
 * ORIGINS: 26 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <errno.h>

/*
 * This is a very fast storage allocator.  It allocates blocks of a small 
 * number of different sizes, and keeps free lists of each size.  Blocks that
 * don't exactly fit are passed up to the next larger size.  In this 
 * implementation, the available sizes are 2^n-8 (or 2^n-18) bytes long.
 * This is designed for use in a virtual memory environment.
 */

/*
 * The overhead on a block is at least 8 bytes.  When free, this space
 * contains a pointer to the next free block, and the bottom two bits must
 * be zero.  When in use, the first byte is set to MAGIC, and the second
 * byte is the size index.  The remaining bytes are for alignment.
 * If range checking is enabled then a second word holds the size of the
 * requested block, less 1, rounded up to a multiple of sizeof(RMAGIC).
 * The order of elements is critical: ov_magic must overlay the low order
 * bits of ov_next, and ov_magic can not be a valid ov_next bit pattern.
 *
 * The overhead at the beginning of a block is a multiple of 8 bytes. 
 * This is done to insure the address returned to the user is on an 8 byte
 * boundary.  This helps performance on machines which enforce an 8 byte
 * boundary alignment on certain data types if the user's data structure
 * maps these types to an 8 byte boundary.  When range checking is enabled
 * there are 2 bytes of overhead at the end of a block.
 */

union	overhead {
	union	overhead *ov_next;	/* when free */
	struct {
		u_int	ovu_magic;	/* magic number */
		u_int	ovu_index;	/* bucket # */
#ifdef RCHECK
		u_short	ovu_ralign;	/* just for alignment */
		u_short	ovu_rmagic;	/* range magic number */
		u_int	ovu_size;	/* actual block size */
#endif
	} ovu;
#define	ov_magic	ovu.ovu_magic
#define	ov_index	ovu.ovu_index
#define	ov_rmagic	ovu.ovu_rmagic
#define	ov_size		ovu.ovu_size
};

#define	MAGIC		0xef		/* magic # on accounting info */

/*
 * nextf[i] is the pointer to the next free block of size 2^(i+3).  The
 * smallest allocatable block is 8 bytes.  The overhead information
 * precedes the data area returned to the user.
 */
#define	NBUCKETS 	30
#define BUCKET_BUMP	0x20
static union overhead *nextf[NBUCKETS];
static int block_incr[NBUCKETS] = {
	0x100, 0x100, 0x100, 0x100, 0x080, 0x080, 0x080, 0x080,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1
};

extern char *sbrk(int);
extern int getpagesize(void);
static void morecore(int);
static void fracture(union overhead *, int, size_t);
static void freeblock(union overhead *, int);
static int findbucket(union overhead *, int);
int __whatbucket(size_t);
extern void bcopy(char *, char *, int);
static void disclaim_free();

static	int pagesz;			/* page size */
static	int pagebucket;			/* page size bucket */
static	int sbrkadjust;			/* adjustment if sbrk rounds up */

/*
 * Declare alternate malloc entry points and a switch to enable them.
 * These are used to enable the Yorktown malloc (malloc_y.c) via a
 * call to mallopt.
 */

/* 
 *	Variables, defines for mallopt simulation and mallinfo.
 */
#define ALIGNSZ	  sizeof(double)	/* aligns anything (smallest grain) */
#define NUMLBLKS  100	/* default number of small blocks per holding block */
#define MAXFAST   0	/* default maximum size block for fast allocation */
static int maxfast = MAXFAST;	/* maximum size block for fast allocation */
static int fastbucket = 0;
static int numlblks = NUMLBLKS;	/* number of small blocks per holding block */
static int grain = ALIGNSZ;	/* size of small block */
static int fastmallocs = 0;	/* != 0, once small malloc after mallopt */

/*
 *      Variables for free save information
 */
#define MAX_FREE_SAVE 30
static  union overhead *free_save[MAX_FREE_SAVE];
static int num_free_saved = 0;
static int num_free_ovfl = 0;

/*
 * nmalloc[i] is the difference between the number of mallocs and frees
 * for a given block size.
 */
static	u_int nmalloc[NBUCKETS];

/*
 * NAME:	malloc
 *
 * FUNCTION:	malloc - memory allocator
 *
 * NOTES:	Malloc allocates and returns a pointer to at least
 *		'nbytes' of memory.
 *
 * RETURN VALUE DESCRIPTION:	NULL if the there is no more available
 *		memory or if the memory arena was corrupted.
 */

void *
malloc(size_t nbytes)
{
  	register union overhead *op, *_op;
  	register int bucket;
	register size_t amt, n;

	/* When zero bytes are requested NULL is returned (SVID behavior) */
	if (nbytes == 0) {
		errno = EINVAL;
		return (NULL);
	}

	/*
	 * Convert amount of memory requested into closest block size
	 * stored in hash buckets which satisfies request.
	 * Account for space used per block for accounting.
	 */

	if ((bucket = _WhatBucket(nbytes)) < 0)
		return (NULL);

	/*
	 * If nothing in hash bucket right now,
	 * request more memory from the system.
	 */
	op = _op = nextf[bucket];
	if (_op != NULL) {

	    /* remove from linked list */
	    nextf[bucket] = op->ov_next;
	    op->ov_index = bucket;
/*	    op->ov_magic = MAGIC; */

	    return op + 1;
	}

	morecore(bucket);
	if ((op = nextf[bucket]) == NULL)
	    return (NULL);

	nextf[bucket] = op->ov_next;
/*	op->ov_magic = MAGIC;*/
	op->ov_index = bucket;

  	return (op + 1);
}

/*
 * NAME:	free
 *
 * FUNCTION:	free - deallocate memory
 *
 * NOTES:	Free frees up the previously allocated block of memory
 *		'cp', making it available for later allocation.
 *
 * RETURN VALUE DESCRIPTION:	none
 */

void
free(void *cp)
{   
  	register int size;
	register union overhead *op;

	op = (union overhead *)((caddr_t)cp - sizeof (union overhead));

  	if (cp != NULL) 
	{
	    size = op->ov_index;
	    op->ov_next = nextf[size];
	    nextf[size] = op;
	}
}


/*
 * Allocate more memory to the indicated bucket.
 */
static void
morecore(int bucket)
{
        static int fastbucket;
  	register union overhead *op;
	register int sz;		/* size of desired block */
  	int amt;			/* amount to allocate */
  	int nblks;			/* how many blocks we get */
	int sbucket;			/* bucket to steal memory from */
	int n;

	/*
	 * First time malloc is called, setup page size and
	 * align break pointer so all data will be page aligned.
	 * Sbrk can round up our request to a higher boundary
	 * so check what comes back to assure page alignment.
	 */
	if (pagesz == 0) {
		pagesz = n = getpagesize();
		op = (union overhead *)sbrk(0);
  		n = n - sizeof (*op) - ((int)op & (n - 1));
		if ((int)n < 0)
			n += pagesz;
  		if (n) {
  			if (sbrk((int)n) == (char *)-1)
				return;
		}
		if (sbrk(0) != (char *)(n + (int)op))
			sbrkadjust = 1;

		sbucket = 0;
		amt = 8;
		while (pagesz > amt) {
			amt <<= 1;
			sbucket++;
		}
		pagebucket = sbucket;
	}

	/*
	 * sbrk_size <= 0 only for big, FLUFFY, requests
	 * or for a negative arg.
	 */
	sz = 1 << (bucket + 3);

	if (sz <= 0)
		return;

	if (sz < pagesz) {
	        nblks = block_incr[bucket];
	        block_incr[bucket] += BUCKET_BUMP;
		amt = nblks * sz;
	} else {
		amt = sz + pagesz;
		nblks = 1;
	}

	op = (union overhead *)sbrk(amt);
	if ((int)op != -1) {
		/* Make sure sbrk value is page aligned.  Since load()	*/
		/* and sbrk() calls may be made between malloc() calls	*/
		/* we have no way to guarantee sbrk value will remain   */
		/* page aligned after it is set in malloc.  Therefore,	*/
		/* it must be checked before each sbrk() call.		*/
		n = pagesz - sizeof (*op) - ((int)op & (pagesz - 1));
		if (n < 0)
			n += pagesz;
		if (n) {
			if ((char *) sbrk(n) > (char *)0) {
				op = (union overhead *)((int)op + n);
				if (sbrkadjust) {
					op++;
				}
			}
			else
			{
				sbrk(-amt);
				op = (union overhead *)-1;
			}
		}	
	}

	/* no more room! */
  	if ((int)op == -1) {
		/* try to steal room from a larger bucket */
		amt = ((sz < pagesz) ? sz : amt) - sizeof (*op);
		for (sbucket = bucket + 1; sbucket < NBUCKETS; sbucket++)
			if ((op = nextf[sbucket]) != NULL) {
				fracture(op, sbucket, (size_t)amt);
				nextf[sbucket] = op->ov_next;
				op->ov_next = nextf[bucket];
				nextf[bucket] = op;
				break;
			}
  		return;
	}
	op = op - sbrkadjust;		/* align data to page boundary */
	/*
	 * Add new memory allocated to that on
	 * free list for this hash bucket.
	 */
  	nextf[bucket] = op;
  	while (--nblks > 0) {
		op->ov_next = (union overhead *)((caddr_t)op + sz);
		op = (union overhead *)((caddr_t)op + sz);
  	}
	op->ov_next = (union overhead *)NULL;
}

/*
 * When a program attempts "storage compaction" as mentioned in the
 * old malloc man page, it realloc's an already freed block.  Usually
 * this is the last block it freed; occasionally it might be farther
 * back.  We have to search all the free lists for the block in order
 * to determine its bucket: 1st we make one pass thru the lists
 * checking only the first block in each; if that fails we search
 * ``realloc_srchlen'' blocks in each list for a match (the variable
 * is extern so the caller can modify it).  If that fails we just copy
 * however many bytes was given to realloc() and hope it's not huge.
 */
int realloc_srchlen = 4;	/* 4 should be plenty, -1 =>'s whole list */

/*
 * NAME:	realloc
 *
 * FUNCTION:	realloc - change size of previously allocated block
 *
 * NOTES:	Realloc changes the size of a previously allocated
 *		block of memory 'cp' to 'nbytes'.
 *
 * RETURN VALUE DESCRIPTION:	NULL if there is no available memory
 *		or if the memory memory has been corrupted.
 */

void *
realloc(void *cp, size_t nbytes)
{   
  	register size_t onb, i;
	union overhead *op;
  	char *res;
	int was_alloced = 0;
	int obucket;

  	if (cp == NULL)
  		return (malloc(nbytes));

	op = (union overhead *)((caddr_t)cp - sizeof (union overhead));
#if 0
	if (op->ov_magic == MAGIC) {
#endif
		was_alloced++;
		i = obucket = op->ov_index;
#if 0
	} else {
		/*
		 * Already free, doing "compaction".
		 *
		 * Search for the old block of memory on the
		 * free list.  First, check the most common
		 * case (last element free'd), then (this failing)
		 * the last ``realloc_srchlen'' items free'd.
		 * If all lookups fail, then assume the size of
		 * the memory block being realloc'd is the
		 * largest possible (so that all "nbytes" of new
		 * memory are copied into).  Note that this could cause
		 * a memory fault if the old area was tiny, and the moon
		 * is gibbous.  However, that is very unlikely.
		 */
		if ((int)(i = findbucket(op, 1)) < 0 &&
		    (int)(i = findbucket(op, realloc_srchlen)) < 0)
			i = NBUCKETS;
	}
#endif
	/* if new size is zero, the block pointed to is freed */
	if (nbytes == 0) {
		if (was_alloced)
			free(cp);
		return (NULL);
	}
	onb = 1 << (i + 3);		/* old number of bytes */
	if (onb < pagesz)
		onb -= sizeof (*op);
	else
		onb += pagesz - sizeof (*op);
	/* avoid the copy if same size block */
	if (was_alloced) {
		if (i) {
			i = 1 << (i + 2);	/* next smallest block */
			if (i < pagesz)
				i -= sizeof (*op);
			else
				i += pagesz - sizeof (*op);
		}
		if (nbytes <= onb && nbytes > i) {
			return(cp);
		}
		/* old block is not freed yet in case malloc fails */
	}

	res = malloc(nbytes);
	if (res == NULL)
  		return (NULL);
  	if (cp != res)		/* common optimization if "compacting" */
 		bcopy((char*)cp, res, (int)((nbytes < onb) ? nbytes : onb));
	if (was_alloced)	/* safe to free old block */
		free(cp);
  	return (res);
}

/*
 * Search ``srchlen'' elements of each free list for a block whose
 * header starts at ``freep''.  If srchlen is -1 search the whole list.
 * Return bucket number, or -1 if not found.
 */
static int
findbucket(union overhead *freep, int srchlen)
{
	register union overhead *p;
	register int i, j;

	for (i = 0; i < NBUCKETS; i++) {
		j = 0;
		for (p = nextf[i]; p && j != srchlen; p = p->ov_next) {
			if (p == freep)
				return (i);
			j++;
		}
	}
	return (-1);
}

#include <malloc.h>

/*
 * NAME:	mallopt
 *
 * FUNCTION:	mallopt - tune small block allocation algorithm
 *
 * NOTES:	Mallopt is included for SysV compatibility only.
 *		It has NO real function in this implementation.
 *		The malloc above should be faster in almost all
 *		cases.  Maybe someday the two will be merged.
 *
 * RETURN VALUE DESCRIPTION:	0, 1 on error
 */
/*      Mallopt - set options for allocation

	Mallopt provides for   control over the allocation algorithm.
	The cmds available are:

	M_MXFAST Set maxfast to value.  Maxfast is the size of the
		 largest small, quickly allocated block.  Maxfast
		 may be set to 0 to disable fast allocation entirely.

	M_NLBLKS Set numlblks   to value.  Numlblks is the number of
		 small blocks per holding block.  Value must be
		 greater than 0.

	M_GRAIN  Set grain to value.  The sizes of all blocks
		 smaller than maxfast are considered to be rounded
		 up to the nearest multiple of grain.    The default
		 value of grain is the smallest number of bytes
		 which will allow alignment of any data type.    Grain
		 will   be rounded up to a multiple of its default,
		 and maxsize will be rounded up to a multiple   of
		 grain.  Value must be greater than 0.

	M_KEEP   Retain data in freed   block until the next malloc,
		 realloc, or calloc.  Value is ignored.
		 This option is provided only for compatibility with
		 the old version of malloc, and is not recommended.

	returns - 0, upon successful completion
		  1, if malloc has previously been called or
		     if value or cmd have illegal values
*/

/* ARGSUSED */
int
mallopt(int cmd, int value)
{
	/* disallow changes once a small block is allocated */
	if (fastmallocs)  {
		return 1;
	}
	switch (cmd)  {
	    case M_MXFAST:
		if (value < 0)  {
			return 1;
		}
		/* round up to a multiple of grain size */
		maxfast = (value + grain - 1)/grain;
		fastbucket = _WhatBucket(maxfast);

		break;
	    case M_NLBLKS:
		if (value <= 1)  {
			return 1;
		}
		numlblks = value;
		break;
	    case M_GRAIN:
		if (value <= 0)  {
			return 1;
		}
		/* round grain up to a multiple of ALIGNSZ */
		grain = (value + ALIGNSZ - 1)/ALIGNSZ*ALIGNSZ;
		break;
	    case M_KEEP:
		break;
	    default:
		return 1;
	}
	return 0;
}

/*
 * NAME:	mallinfo
 *
 * FUNCTION:	mallinfo - return information describing current
 *		malloc arena.
 *
 * NOTES:	Mallinfo in its original state returned a
 *		structure describing the current state of
 *		the malloc family.  Here it is only included
 *		for compatibility with SysV.
 *
 * RETURN VALUE DESCRIPTION:	struct mallinfo
 */

struct mallinfo
mallinfo(void)
{
    static struct mallinfo info;
    register int i, j, k;
    register union overhead *p;

    fastbucket = _WhatBucket(maxfast);

    info.arena    = 0;		/* total space in arena */
    info.ordblks  = 0;		/* number of ordinary blocks */
    info.fordblks = 0;		/* space in free ordinary blocks */
    info.uordblks = 0;		/* space in ordinary blocks in use */
    info.smblks   = 0;		/* number of small blocks */
    info.fsmblks  = 0;		/* space in free small blocks */
    info.usmblks  = 0;		/* space in small blocks in use */

    /*
      Gather statistics for 'free' memory
    */
    for (i = 0; i < NBUCKETS; i++) {
	
	/* count number of free blocks in bucket */
	for (j = 0, p = nextf[i]; p; p = p->ov_next, j++);

	/* free space in this bucket */
	k = j * ((1 << (i + 3)) + (i < pagebucket ? 0 : pagesz));

	/*
	   j is number of free blocks in this bucket
	   k is bytes in free blocks in this bucket 
	*/
	info.arena += k;
	if (i > fastbucket) {	/* regular block */
	    info.ordblks += j;	
	    info.fordblks += k;
	} else {		/* small block */
	    info.smblks += j;
	    info.fsmblks += k;
	}
    }

    /*
      Gather statistics for 'used' memory
    */
    for (i = 0; i < NBUCKETS; i++) {

	/* used space in this bucket */
	k = nmalloc[i] * ((1 << (i + 3))
			  + (i < pagebucket ? 0 : pagesz));

	/*
	  nmalloc[i] is count of blocks allocated in i-th bucket
	  k          is bytes in blocks for this bucket
	*/
	if (i > fastbucket) {	/* regular block */
	    info.arena += k;
	    info.ordblks += nmalloc[i];
	    info.uordblks += k;
	} else {		/* small block */
	    info.smblks += nmalloc[i];
	    info.usmblks += k;
	}
    }
    
    return (info);
}

/*
 * Break up an old block of memory.  A new block is cut off the front
 * to satisfy a malloc request.  The remainder is put on the
 * free list as one or more blocks.
 */
static void
fracture(union overhead *op, int obucket, size_t nbytes)
{
	register size_t oamt, namt;	/* old and new block sizes */
	register size_t sz;		/* size of a memory block */
	register int nbucket, bucket;	/* new and work bucket numbers */

	oamt = 1 << (obucket + 3);
	if (oamt >= pagesz)
		oamt += pagesz;
	nbucket = _WhatBucket(nbytes);
	op->ov_index = nbucket;
	namt = 1 << (nbucket + 3);
  	nmalloc[obucket]--;
  	nmalloc[nbucket]++;
	/*
	 * Cut the new block off the front of the old block.  There
	 * will be a remainder to deal with here only when the new
	 * block is smaller than a page.  How much remainder depends on
	 * the old block size, but it will always be less than a page.
	 */
	if (namt < pagesz) {
		/* decide how much remainder gets handled here */
		if (oamt < pagesz) {
			sz = oamt - namt;
			oamt = 0;
		} else {
			sz = pagesz - namt;
			oamt -= pagesz;
		}
		op = (union overhead *)((caddr_t)op + namt);
		/* put namt sized blocks on the free list */
		do {
			freeblock(op, nbucket);
			op = (union overhead *)((caddr_t)op + namt);
		} while ((sz -= namt) > 0);
	} else {
		namt += pagesz;
		oamt -= namt;
		op = (union overhead *)((caddr_t)op + namt);
	}
	/*
	 * There may be no remainder left at this point, but if there
	 * is it will be in multiples of pagesz.  When multiple pages
	 * are left, take as large a block as will fit the remainder
	 * and put it on the free list.  Sometimes just one page is
	 * left, chop it up into smaller pieces and put them on the
	 * free list.  Keep doing this until there is nothing left.
	 */
	while (oamt > 0) {
		if (oamt > pagesz) {
			/* fit the largest block we can in the remainder */
			bucket = _WhatBucket(oamt - sizeof (*op));
			sz = (1 << (bucket + 3)) + pagesz;
			if (sz > oamt) {	/* wasn't an exact fit */
				bucket -= 1;
				sz = (1 << (bucket + 3)) + pagesz;
			}
			/* put our best fit on the free list */
			op->ov_next = nextf[bucket];
			nextf[bucket] = op;
			op = (union overhead *)((caddr_t)op + sz);
			oamt -= sz;
		} else {
			/* just one page left, decide how to chop it up */
			bucket = (namt < pagesz) ? nbucket : pagebucket - 1;
			sz = 1 << (bucket + 3);
			/* put sz sized blocks on the free list */
			do {
				freeblock(op, bucket);
				op = (union overhead *)((caddr_t)op + sz);
			} while ((oamt -= sz) > 0);
		}
	}
}

/*
 * Puts a block on the free list.
 */
static void
freeblock(union overhead *op, int bucket)
{
	op->ov_next = nextf[bucket];
	nextf[bucket] = op;
}

/*
 * Convert an amount of memory size into the closest block size
 * stored in hash buckets which will hold that amount.
 * Account for space used per block for accounting.
 * Return the bucket number, or -1 if request is too large.
 */
int
__whatbucket(size_t nbytes)
{
	register int bucket;
	register int amt, n;

	if (nbytes <= (n = pagesz - sizeof (union overhead))) {
		amt = 8;	/* size of first bucket */
		bucket = 0;
		n = -(sizeof (union overhead));
	} else {
		amt = pagesz;
		bucket = pagebucket;
	}
	while (nbytes > amt + n) {
		amt <<= 1;
		if ((int)amt <= 0)
			return (-1);
		bucket++;
	}
	return (bucket);
}

/*
 * disclaim_free - disclaim page space for blocks on the free list
 */
static void
disclaim_free()
{
  	register int i, size;
	register union overhead *op;

	for (i = 0; i < num_free_saved; i++){
		op = (union overhead *) ((caddr_t) free_save[i] -
			sizeof (union overhead));
		if (op->ov_magic == MAGIC)
			continue;
  		size = op->ov_index;
		disclaim(free_save[i],1<<(size+3),ZERO_MEM);
	}
	num_free_ovfl = num_free_saved = 0;
}
