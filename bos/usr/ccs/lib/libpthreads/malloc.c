static char sccsid[] = "@(#)30	1.2  src/bos/usr/ccs/lib/libpthreads/malloc.c, libpth, bos412, 9445C412b 11/2/94 12:01:37";
/*
 * COMPONENT_NAME: libpth
 * 
 * FUNCTIONS:
 *	malloc_fork_before
 *	malloc_fork_after
 *	_pthread_malloc_startup
 *	more_memory
 *	_pmalloc
 *	_pfree
 *	
 * ORIGINS:  71, 83
 *
 * LEVEL 1, 5 Years Bull Confidential Information
 *
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 *
 * OSF/1 1.2
 */

/*
 * File: malloc.c
 * Memory allocator for use with multiple threads.
 */

#ifdef	DEBUG_MAL
volatile int NBSBRK = 0;
int TSBRK[50];
#endif

#include "internal.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>

/* Assumptions:
 *	This malloc assumes the widest alignment requirement is 8 bytes
 *	(or a double word on most machines).
 */

/*
 * Need to allow alignment of arbitrary pointer types.
 */
#define	ALIGNBYTES	8			/* align on 8 byte boundary */
#define	ALIGNPOW2	3			/* LN2(ALIGNBYTES) */
#define MIN_SIZE	(ALIGNBYTES << 1)	/* min bucket size */

/*
 * The free list is an array of lists.
 * Each list contains buckets of the same (power of 2) size.
 * Buckets are used to satisfy malloc requests.
 * The malloc block header is first, immediately followed by the malloc data.
 *
 * Bucket size for list[i]:
 *		1 << (i + (ALIGNPOW2 + 1))
 * Max bucket size:
 *		(1 << NBUCKETS - 1 + ALIGNPOW2 + 1)
 * Malloc data size for list[i]:
 *		1 << (i + (ALIGNPOW2 + 1)) - sizeof(union header)
 */

#define	BUCKET_SIZE(i)	(1 << (i + (ALIGNPOW2 + 1)))

#define NBUCKETS	10	/* Number of bucket sizes */

/* Malloc block header.
 * Padded out so that the user data which follows it will retain
 * the same alignment (ie if the header is aligned so is the data).
 */
typedef union header {
	union header		*next;	/* allocated: next block on free list */
	struct free_list	*fl;	/* free: points to free list */
	char			align[ALIGNBYTES];	/* padding */
} *header_t;

/* Bucket list.
 */
typedef struct free_list {
	spinlock_t	lock;	/* spin lock for mutual exclusion */
	header_t	head;	/* head of free list for this size */
#ifdef	DEBUG_MAL
	int		in_use;	/* # mallocs - # frees */
#endif	/* DEBUG_MAL */
} *free_list_t;

private struct free_list	malloc_free_list[NBUCKETS];

private spinlock_t		malloc_lock;

/* Pool list.
 * Memory is taken from the system in large chunks which are used to
 * populate bucket lists. The pool list is a holding area for these
 * chunks. Not all of a chunk may be used at once.
 */
typedef struct pool {
	struct pool	*next;
	size_t	size;
} *pool_t;

#define	POOL_NULL	((pool_t) 0)
private pool_t		pool = POOL_NULL;
private size_t		pool_size = 0;

/* Minimum number of pages to allocate in a chunk.
 */
#define DEFAULT_MALLOC_POOL_SIZE    32
private unsigned int	malloc_pool_size = DEFAULT_MALLOC_POOL_SIZE;


/*
 * Function:
 *	malloc_fork_before
 *
 * Description:
 *	Prepare the malloc module for a fork by ensuring that no thread
 *	is in a malloc critical section.
 *	Note: 	since the malloc_lock is only taken under a malloc_free_list
 *		lock, acquiring the latter is sufficient.
 */
private void
malloc_fork_before(void)
{
	register int	i;

	for (i = 0; i < NBUCKETS; i++) {
		_spin_lock(&malloc_free_list[i].lock);
	}
}


/*
 * Function:
 *	malloc_fork_after
 *
 * Description:
 *	Called in the after a fork() to resume normal operation.
 */
private void
malloc_fork_after(void)
{
	register int	i;

	for (i = NBUCKETS - 1; i >= 0; i--) {
		_spin_unlock(&malloc_free_list[i].lock);
	}
}


/*
 * Function:
 *	_pthread_malloc_startup
 *
 * Description:
 *	Initialise locks.
 *	Register the fork handlers.
 *	This function is called from pthread_init().
 */
void
_pthread_malloc_startup(void)
{
	register int	i;

	for (i = NBUCKETS - 1; i >= 0; i--) {
		_spinlock_create(&malloc_free_list[i].lock);
	}
	_spinlock_create(&malloc_lock);

	if (pthread_atfork  (malloc_fork_before,
			      malloc_fork_after,
			      malloc_fork_after))
		INTERNAL_ERROR("_pthread_malloc_startup");
}


/*
 * Function:
 *	more_memory
 *
 * Parameters:
 *	size - The size of the memory chunks in the bucket
 *	fl - free list pointer of the empty bucket
 *
 * Return value:
 *	FALSE	No more memory could be allocated
 *	TRUE	the bucket was filled
 *
 * Description:
 *	This function maintains pools of memory from which the malloc buckets
 *      are filled. New blocks added to the free list are taken
 *	from the first pool in the list of pools that has enough memory.
 *	If the amount of memory for the new blocks is bigger that what
 *      is available from any pool or is bigger than a pools initial
 *	size (__page_size * malloc_pool_size) then more memory is allocated
 *	using sbrk(). The free list is assumed to be locked by the caller.
 */
private boolean_t
more_memory(unsigned int size, register free_list_t fl)
{
	register size_t	amount;
	register unsigned int	n;
	unsigned int		where;
	register header_t	h;
	int			r;

	/* Calculate how many new blocks we are going to chain onto the
	 * free list. If the bucket contains blocks of less than 1 page
	 * then we allocate a page anyway and divide that up into a number
	 * of blocks to put on the free list. If we are allocating blocks of
	 * more than 1 page then we allocate size * 50 bytes.
	 */
	if (size <= __page_size) {
		amount = __page_size;
		n = __page_size / size;

		/* We lose __page_size - n*size bytes here.
		 */
	} else {
		amount = (size_t) size * 50;
		n = 50;
	}

	_spin_lock(&malloc_lock);

	/* If the current pool (ie the one on the head of the pool list)
	 * has enough memory in it we will take it from there.
	 */
	if (amount <= pool_size) {
		pool_size -= amount;
		where = (unsigned int) pool + pool_size;

		/* Having removed the memory, check to see if it is depleted.
		 * If so we discard this pool and go to the next one in the
		 * chain and make it the current pool (if there is one).
		 */
		if (pool_size == 0) {
			pool = pool->next;
			if (pool != POOL_NULL) {
				pool_size = pool->size;
			}
		}

	} else {

		/* There is either no current pool or it does not have
		 * enough memory in it for this allocation. Search through
		 * the other pools in the list (if there are any) to see if
		 * they have enough memory for this request.
		 */
		where = (unsigned int) 0;
		if (pool != POOL_NULL) {
			register pool_t prev;
			register pool_t cur;

			prev = pool;
			while ((cur = prev->next) != POOL_NULL) {
				if (amount <= cur->size) {

					/* A pool with enough memory has been
					 * found. Take the memory and discard
					 * the pool if it is now empty.
					 */
					cur->size -= amount;
					where = (unsigned int)(cur + cur->size);
					if (cur->size == 0) {
						prev->next = cur->next;
					}
					break;
				}
				prev = cur;
			}
		}

		/* We may have been able to allocate some memory from one of
		 * the pools in the pools list. If not we have to allocate
		 * more memory with sbrk().
		 */
		if (where == (unsigned int) 0) {

			/* If the amount of memory being requested is bigger
			 * than a pools size then we just return the memory
			 * allocated. Otherwise we make a new pool with
			 * the amount remaining after we have removed the
			 * memory we need from the pools initial size
			 */
			if (amount >= (__page_size * malloc_pool_size)) {
#ifdef	DEBUG_MAL
TSBRK[NBSBRK] = amount * sizeof(int);
NBSBRK++;
#endif
			where = (unsigned int) sbrk(amount * sizeof(int));
				if (where == -1) {
					_spin_unlock(&malloc_lock);
					return (FALSE);
				}
			} else {
				pool_t new_pool = POOL_NULL;
#ifdef	DEBUG_MAL
TSBRK[NBSBRK] = __page_size*malloc_pool_size * sizeof(int);
NBSBRK++;
#endif
				new_pool =
		(pool_t) sbrk((__page_size*malloc_pool_size * sizeof(int)));
				if (new_pool == (pool_t) -1) {
					_spin_unlock(&malloc_lock);
					return (FALSE);
				}

				/* Save the current pool size in the pool
				 * structure and make the current pool and
				 * pool_size refer to the newly allocated
				 * memory.
				 */
				if (pool != POOL_NULL) {
					pool->size = pool_size;
				}
				new_pool->next = pool;
				pool = new_pool;
				pool_size = __page_size * malloc_pool_size
					    - amount;
				where = (unsigned int) pool + pool_size;
			}
		}
	}
	_spin_unlock(&malloc_lock);

	/* We have the memory now. The free list should be locked by the caller
	 * so we can chain the new blocks onto it.
	 */
	h = (header_t) where;
	do {
		h->next = fl->head;
		fl->head = h;
		h = (header_t) ((char *) h + size);
	} while (--n != 0);

	return (TRUE);
}


/*
 * Function:
 *	_pmalloc
 *
 * Parameters:
 *	size - the number of bytes requested
 *
 * Return value:
 *	0	the allocation failed
 *	otherwise a pointer to the newly allocated memory of the requested size
 *
 * Description:
 *	Memory is allocated in sizes which are powers of 2. These are taken
 *	from buckets which have memory of the same size chained onto them.
 *	If the bucket is empty, more_memory() is called to put at
 *	least one more element on the chain.
 */
void *
_pmalloc(register size_t size)
{
	register unsigned int	bucket, n;
	register free_list_t	fl;
	register header_t	h;

	/* An overhead of sizeof(union header) is needed for housekeeping.
	 * Check the resulting size is sensible and will be found in one
	 * of the buckets.
	 */
	size += sizeof(union header);
	if (size <= sizeof(union header)) {
		/*
		 * A size of <= 0 or so big that the size turned
		 * negative was requested
		 */
		return(0);
	}

	/* Find smallest power-of-two block size
	 * big enough to hold requested size plus header.
	 */
	bucket = 0;
	n = MIN_SIZE;
	while (n < size && bucket < NBUCKETS) {
		bucket += 1;
		n <<= 1;
	}
	if (bucket == NBUCKETS) {
		return(0);
	}

	/* We now have the index of the bucket that should contain memory
	 * big enough to satisfy this request. Lock the free list and check
	 * that it is not empty. If it is more blocks must be allocated for it.
	 */
	fl = &malloc_free_list[bucket];
	_spin_lock(&fl->lock);
	h = fl->head;
	if (h == 0) {

		/* Free list is empty; allocate more blocks.
		 */
		if (! more_memory(n, fl)) {
			_spin_unlock(&fl->lock);
			return(0);
		}
		h = fl->head;
	}

	/* Pop block from free list.
	 */
	fl->head = h->next;
#ifdef	DEBUG_MAL
	fl->in_use += 1;
#endif	/* DEBUG_MAL */
	_spin_unlock(&fl->lock);

	/* Store free list pointer in block header
	 * so we can figure out where it goes
	 * at free() time.
	 */
	h->fl = fl;

	/* Return pointer past the block header.
printf ("Allocated Address = 0x%x, size = %d\n",
		(int)((char *)h + sizeof(union header)), size);
	 */
	return ((void *)((char *)h + sizeof(union header)));
}


/*
 * Function:
 *	_pfree
 *
 * Parameters:
 *	base - pointer to the memory no longer needed.
 *
 * Description:
 *	Free up memory that was previously allocated with malloc. This is
 *	done by chaining the memory back onto the free list of the bucket
 *	of the correct size. Actually we don't know how much memory the
 *	pointer points to but hidden just before the memory is a pointer
 *	to the free list that the memory should be freed onto. Rudimentary
 *	sanity checks are made to ensure the pointer is not complete garbage.
 */
void
_pfree(void *base)
{
	register header_t	h;
	register free_list_t	fl;
	register int		bucket;

/*
printf("Desallocated Adress = 0x%x\n", (int) base);
*/

	if (base == 0) {
		return;
	}

	/* Find free list for block.
	 */
	h = (header_t) ((caddr_t)base - sizeof(union header));

	/* If base is garbage this may seg fault, but such is life ...
	 */
	fl = h->fl;
	bucket = fl - malloc_free_list;

	/* Check that the free list pointer we have found is legitimate by
	 * first checking the index into the buckets is valid and then by
	 * checking the pointer is the same as the address of the free list
	 * indicated by the index.
	 */
	if (bucket < 0 || bucket >= NBUCKETS) {
		return;
	}
	if (fl != &malloc_free_list[bucket]) {
		return;
	}

	/* Push block on free list.
	 */
	_spin_lock(&fl->lock);
	h->next = fl->head;
	fl->head = h;
#ifdef	DEBUG_MAL
	fl->in_use -= 1;
#endif	/* DEBUG_MAL */
	_spin_unlock(&fl->lock);
	return;
}


#ifdef	DEBUG_MAL
void
print_malloc_free_list(void)
{
  	register unsigned int	i, size;
	register free_list_t	fl;
	register unsigned int	n;
  	register header_t	h;
	int			total_used = 0;
	int			total_free = 0;
	int			Tsize[NBUCKETS];
	int			Tin_use[NBUCKETS];
	int			Tfree[NBUCKETS];
	int			Ttotal[NBUCKETS];

	for (i = 0; i < NBUCKETS; i++) {
			Tsize[i] = 0;
			Tin_use[i] = 0;
			Tfree[i] = 0;
			Ttotal[i] = 0;
	}

	printf("      Size     In Use       Free      Total\n");
  	for (i = 0, size = MIN_SIZE, fl = malloc_free_list;
	     i < NBUCKETS;
	     i += 1, size <<= 1, fl += 1) {
		_spin_lock(&fl->lock);
		if (fl->in_use != 0 || fl->head != 0) {
			total_used += fl->in_use * size;
			for (n = 0, h = fl->head; h != 0; h = h->next, n += 1)
				;
			total_free += n * size;
			Tsize[i] = size;
			Tin_use[i] = fl->in_use;
			Tfree[i] = n;
			Ttotal[i] = fl->in_use + n;
		}
		_spin_unlock(&fl->lock);
  	}
	for (i = 0; i < NBUCKETS; i++) {
		printf("%10d %10d %10d %10d\n",
			Tsize[i],
			Tin_use[i],
			Tfree[i],
			Ttotal[i]);
	}
  	printf("\n all sizes %10d %10d %10d\n",
		total_used, total_free, total_used + total_free);
	printf("\n nb of sbrk() = %d\n", NBSBRK);
	for (i = 0; i < NBSBRK; i++) {
		printf("\nsize on sbrk() = %d\n", TSBRK[i]);
	}
}
#endif	/* DEBUG_MAL */
