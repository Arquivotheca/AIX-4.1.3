/* @(#)02	1.3  src/bos/kernel/ldr/ff_alloc.h, sysldr, bos411, 9428A410j 1/31/93 18:09:49 */
/*
 * COMPONENT_NAME: (SYSLDR) Program Management
 *
 * FUNCTIONS: first fit allocator
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

#ifndef	_H_FFALLOC
#define	_H_FFALLOC

/*
 * The first fit allocator maintains a list of free fragments of memory
 * ordered by their virtual addresses. The first sizeof (space_t) bytes
 * of each free fragment holds a space_t structure, which provides
 * the size of the current fragment and the address of the next one.
 * The size of the current fragment includes the space used by the header.
 *
 * When a memory is allocated the header will contain the size
 * of the allocated fragment and a pointer to the fflist_t where it came from.
 *
 * The size and fflist pointer are used when the fragment is released to
 * know the size of what is being released and into which free list it goes.
 */

#include <sys/types.h>
#include <sys/lock_def.h>

/*
 * The size of space_t MUST be a power of two
 * FFALIGN_MASK depends on this.
 */
typedef struct space {
	union {
		struct space	*_sp_next;
		struct fflist	*_sp_fflist;
	} _u;
	size_t		 sp_size;
} space_t;

#define	sp_next		_u._sp_next
#define	sp_fflist	_u._sp_fflist

#define	FFALIGN		(sizeof (space_t))
#define	FFALIGN_MASK	(FFALIGN - 1)

typedef struct fflist {
	lock_t		 ff_lock;
	space_t		*ff_list;
	char		*ff_start;
	char		*ff_end;
} fflist_t;

/*
 * Fflists are declared and initialized with FF_DECLARE
 * to avoid having knowledge of the layout of fflist_t 
 * in multiple places.
 */
#define	 FF_DECLARE(name)				\
	 fflist_t name = {LOCK_AVAIL, NULL, NULL, NULL}
#define	 FF_ADDR(fflist,addr)				\
	 ((char *) (addr) >= (fflist)->ff_start &&	\
	  (char *) (addr) < (fflist)->ff_end)
int	 ff_init (fflist_t *fflist, size_t size);
void	*ff_alloc (fflist_t *fflist, size_t size, int align);
void	 ff_free (void *mem);


#endif	/* _H_FFALLOC */

