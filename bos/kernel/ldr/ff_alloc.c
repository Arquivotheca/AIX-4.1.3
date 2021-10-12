static char sccsid[] = "@(#)01	1.2  src/bos/kernel/ldr/ff_alloc.c, sysldr, bos411, 9428A410j 12/15/92 09:43:40";
/*
 * COMPONENT_NAME: (SYSLDR) Program Management
 *
 * FUNCTIONS: ff_init, ff_alloc, ff_free
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * First fit memory allocator.
 */

#include <sys/types.h>
#include <sys/malloc.h>
#include <sys/syspest.h>
#include <sys/param.h>
#include "ff_alloc.h"

#define	PAGEMASK	(PAGESIZE - 1)

/*
 * Initializes the pool of free memory
 * to contain an initial chunk of size bytes.
 * Returns 0 on failure, non-zero otherwise.
 */
int	ff_init (fflist_t *fflist, size_t size)
{
	int	 lock_rc;
	space_t	*space;

	if ((lock_rc = lockl (&fflist->ff_lock, LOCK_SHORT)) != LOCK_SUCC)
		panic ("ff_init: lock nest/fail\n");
	if (fflist->ff_list)
		panic ("ff_init: called more than once\n");
	if (size < FFALIGN)
		panic ("ff_init: size too small\n");
	size += FFALIGN_MASK;
	size &= ~FFALIGN_MASK;
	if (space = (space_t *) xmalloc (size, PGSHIFT, kernel_heap)) {
		fflist->ff_list = space;
		fflist->ff_start = (char *) space;
		fflist->ff_end = (char *) space + size;
		space->sp_next = (space_t *) NULL;
		space->sp_size = size;
	}
	unlockl (&fflist->ff_lock);
	return (int) space;
}


/*
 * Allocate size bytes of memory, return a pointer
 * to the allocated memory or NULL in case of failure.
 * The memory is aligned to a multiple of (1 << align).
 */
void	*ff_alloc (fflist_t *fflist, size_t size, int align)
{
	int	  lock_rc;
	space_t	**prev_space;
	space_t	 *temp_space;
	space_t	 *space;
	char	 *end_space;
	char	 *mem;
	char	 *end_mem;
	void	 *m;

	/*
	 * Allocation size should be a multiple of FFALIGN.
	 */
	if (size < FFALIGN)
		size = FFALIGN;
	else {
		size += FFALIGN_MASK;
		size &= ~FFALIGN_MASK;
	}
	m = NULL;

	align = (1 << align) - 1;
	if (align <= FFALIGN_MASK) 
		align = 0;

	if ((lock_rc = lockl (&fflist->ff_lock, LOCK_SHORT)) != LOCK_SUCC)
		panic ("ff_alloc: lock nest/fail\n");

	for (prev_space = &fflist->ff_list;
	     space = *prev_space;
	     prev_space = &space->sp_next) {

		/*
		 * Make mem be the next aligned address after space.
		 * Leave space for a space_t header.
		 */
		mem = (char *) (((int) (space + 1) + align) & ~align);

		/*
		 * Aligned fragment should fit in the memory covered by space.
		 * Note that enough bytes for the header exist before mem and
		 * that space->sp_size includes the bytes used by the space_t
		 * header pointed by space.
		 */
		end_mem = mem + size;
		end_space = (char *) space + space->sp_size;
		if (end_mem > end_space)
			continue;

		/*
		 * Space descriptor requires FFALIGN bytes.
		 */
		size += FFALIGN;

		if (space->sp_size == size) {
			/*
			 * Exact fit.
			 * space->sp_size == size && end_mem <= end_space
			 *	==>>
			 * mem == (char *) (space + 1)
			 */
			assert (mem == (char *) (space + 1));
			*prev_space = space->sp_next;
			space->sp_fflist = fflist;
		} else {
			if (end_mem < end_space) {
				/*
				 * Create a fragment for the bytes at the end.
				 */
				temp_space = (space_t *) end_mem;
				temp_space->sp_size = end_space - end_mem;
				temp_space->sp_next = space->sp_next;
				space->sp_next = temp_space;
			}

			temp_space = (space_t *) mem - 1;
			if (temp_space == space)
				*prev_space = space->sp_next;
			else
				space->sp_size =
					(char *) temp_space - (char *) space;

			temp_space->sp_size = size;
			temp_space->sp_fflist = fflist;
		}
		assert(! ((int) mem & align));
		m = mem;
		break;
	}

	unlockl (&fflist->ff_lock);
	return m;
}

/*
 * Deallocate a fragment of memory
 * coalescing (if possible) with the
 * previous and next fragments.
 * The arena where the memory was
 * allocated is recorded in the
 * descriptor.
 */
void	ff_free (void *mem)
{
	int		  lock_rc;
	int		  size;
	fflist_t	 *fflist;
	space_t		 *free_space;
	space_t		 *space;
	space_t		 *next_space;
	space_t		**prev_spacep;

	free_space = (space_t *) mem - 1;
	fflist = free_space->sp_fflist;

	if ((lock_rc = lockl (&fflist->ff_lock, LOCK_SHORT)) != LOCK_SUCC)
		panic ("ff_free: lock nest/fail\n");

	prev_spacep = &fflist->ff_list;
	while (next_space = *prev_spacep) {
		if (free_space < next_space) {
			/* merge with next? */
			if (next_space == (space_t *)
			   ((char *) free_space + free_space->sp_size)) {
				free_space->sp_size += next_space->sp_size;
				next_space = next_space->sp_next;
			}
			break;
		}
		prev_spacep = &next_space->sp_next;
	}
	/*
	 * When we exit the previous while, either:
	 *	- merged with next, next_space points to the next block
	 *	  after the one we merged with, which might be NULL; or
	 *	- next_space points to the block that should fallow
	 *	  free_space, i.e. there was some block at higher addres
	 *	  but no merge happened; or
	 *	- next_space is NULL, i.e. no next block (or empty list).
	 * In any case next_space has the propper value to store in the
	 * sp_next field of free_space, I postpone storing this value
	 * becouse the block might still merge with the previous block.
	 */

	/*
	 * Prev_spacep is used to keep track of the previous pointer
	 * where to link free_space.
	 * Also (if prev_spacep != &fflist.ff_list) to point to the
	 * previous space_t in the list.
	 * For the prev_spacep == &fflist.ff_list the code that merges
	 * with the previous one works ok even in the case when space
	 * is at a higher address than free_space.
	 */
	if (prev_spacep == &fflist->ff_list)
		space = *prev_spacep;
	else
		/*
		 * Prev_spacep points to the sp_next pointer, but also
		 * to the whole structure. The expression below is
		 * equivalent to:
		 *	space = (space_t *) prev_spacep;
		 * Becouse sp_next is the first field in a space_t,
		 * the offset substracted is just zero, this
		 * avoids "coding" knowledge about the space_t
		 * layout in this file.
		 */
		space = (space_t *)
		    ((char *) prev_spacep - (int) &((space_t *) 0)->sp_next);

	/*
	 * If the list was empty from the beggining; or
	 * was not empty but there will be no merge with previous block
	 */
	if (!space ||
	    free_space != (space_t *) ((char *) space + space->sp_size))
		*prev_spacep = free_space;		/* insert on list */
	else {
		space->sp_size += free_space->sp_size;	/* merge with prev */
		free_space = space;
	}

	/* link the rest of the list */
	free_space->sp_next = next_space;

	/*
	 * Release vmm resources of
	 * released + coalesced fragments.
	 */
	size = free_space->sp_size - FFALIGN;
	if (size >= PAGESIZE) {
		int	p, pg;
		p = (int) ++free_space;
		pg = (p + PAGEMASK) & ~PAGEMASK;
		size -= pg - p + ((p + size) & PAGEMASK);
		if (size != 0)
			assert (! vm_release (pg, size));
	}

	unlockl (&fflist->ff_lock);
}

#if 0
/*
 * Relase nbytes from the beggining of pre-allocated memory.
 * Returns a pointer to the beggining of the new fragment.
 * Returns NULL if trims all the fragment.
 * Panics if tries to trim more than allocated.
 */
void	*ff_trim (void *mem, int nbytes)
{
	space_t		*free_space;
	space_t		*new_space;

	if (nbytes <= FFALIGN)
		return mem;
	free_space = (space_t *) mem - 1;
	if (nbytes > free_space->sp_size)
		panic ("ff_trim: nbytes to large\n");

	nbytes &= ~FFALIGN_MASK;
	new_space = (space_t *) ((char *) free_space + nbytes);
	new_space->sp_fflist = free_space->sp_fflist;
	new_space->sp_size = free_space->sp_size - nbytes;
	free_space->sp_size = nbytes;
	ff_free ((void *) (free_space + 1));

	return (void *) (new_space + 1);
}
#endif

