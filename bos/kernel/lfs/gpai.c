static char sccsid[] = "@(#)47	1.15  src/bos/kernel/lfs/gpai.c, syslfs, bos411, 9428A410j 6/9/94 17:50:56";

/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: gpai_alloc, gpai_free, gpai_srch, gpai_init
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/gpai.h>
#include <sys/malloc.h>
#include <sys/syspest.h>
#include <sys/lock_def.h>
#include <sys/fs_locks.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>

#define OBJ_ALLOCATED 0xffffffff

#define SPTOBJ(sp)		((caddr_t)(*sp) + sizeof(*sp))
#define	LOWBOUND(sp)		((caddr_t) SPTOBJ(sp))
#define	HIGHBOUND(sp, ap)	(sp + ap->a_pas)

static int gpa_lockocc = 0;	/* galloc struct lock occurrence */

/*
 * Initialize the allocation structure.
 *
 * Most of the fields are set up by the caller (usually with static
 * initialization).
 */
void
gpai_init(struct galloc *ap)
{
	int alloc_xs;	/* allocation requested that is excess */

	/* allocate and initialize the lock in the galloc struct */
	lock_alloc(&ap->a_lock,LOCK_ALLOC_PAGED,GPA_LOCK_CLASS,++gpa_lockocc);
	simple_lock_init(&ap->a_lock);

	/* adjust the allocation size down to what will hold an integral
	 * number of objects, plus the next allocation pointer.
	 */
	alloc_xs = (ap->a_pas % (objsize(ap))) - sizeof(caddr_t);
	if (alloc_xs < 0)
		alloc_xs += objsize(ap);
	ap->a_pas -= alloc_xs;

	/* clear the fields that are not the responsibility of the caller */
	ap->a_freeobj = NULL;
	ap->a_sat = NULL;
	ap->a_inuse = 0;
}

/*
 * Allocates an object of specified type and returns a pointer to it.
 * If there are no more free objects, a new pool of objects is malloc'ed
 * and freelist built.
 */

caddr_t *
gpai_alloc(ap)
register struct galloc *ap;
{
	register caddr_t *a;

	GPA_LOCK(ap);
	if (freeobj(ap) == NULL) {
		register caddr_t *sp, sp2;
		register int newsize;
		register i;
		caddr_t op, hp;
		ulong *freep;

		/* Find the addr of the last pool for this object */
		for (sp = &ap->a_sat, i = 0; *sp; sp = (caddr_t *)(*sp),i++);

		/* Allocate a new pool with an address poolptr */
		newsize = ap->a_pas;
		*sp = malloc(newsize);

		if (*sp == NULL)
			return(NULL);

		/* Zero out the new pool */
		bzero(*sp, newsize);

		/* Terminate pool chain */
		*(caddr_t *)(*sp) = NULL;

		/* Construct free list in the new object pool */
		freeobj(ap) = *sp + sizeof(caddr_t);
		hp = HIGHBOUND(*sp, ap);
		op = freeobj(ap);
		while ((op + objsize(ap)) <= hp)
		{	
			/* get pointer to user's structure */
			sp2 = op + sizeof(caddr_t);

			/* if the structure contains a lockword,
			 * then allocate and initialize it.
			 */
			if (ap->a_lkoffset)
			{
				caddr_t lp = &sp2[ap->a_lkoffset];
				lock_alloc(lp, LOCK_ALLOC_PAGED,
					   ap->a_lkclass, ap->a_lkoccur);
				if (ap->a_lktype == GPA_SIMPLE)
					simple_lock_init(lp);
				else
					lock_init(lp, 1);
			}

			/* lock occurrence doubles as structure count */
			ap->a_lkoccur++;

			/* point to next object and set freelist pointer */
			freep = (ulong *)op;
			op += objsize(ap);
			*freep = op;
		}
		/* Terminate the free list */
		*freep = NULL;
	}
	a = (caddr_t *)freeobj(ap);
	freeobj(ap) = *a;
	*a = OBJ_ALLOCATED;
	ap->a_inuse++;
	GPA_UNLOCK(ap);

	/* return pointer to user's structure */
	return(++a);
}

/*
 * Free the previously allocated object.
 */
void
gpai_free(ap, addr)
struct galloc *ap;
caddr_t *addr;
{
	int nested = 0;

	/* the following code allows objects to be freed by
	 * routines that are called from gpai_srch.
	 * (Yes, this is logically the same as:
	 *	if (!(nested = lock_mine(&ap->a_lock)))
	 *		GPA_LOCK(ap);
	 * but it saves a call to lock_mine for the normal case.)
	 */
	if (!simple_lock_try(&ap->a_lock))
		if (!(nested = lock_mine(&ap->a_lock)))
			GPA_LOCK(ap);
	/* adjust pointer to start of allocation object */
	--addr;
	assert(*addr == OBJ_ALLOCATED);
	*addr = freeobj(ap);
	freeobj(ap) = (caddr_t)addr;
	ap->a_inuse--;
	if (!nested)
		GPA_UNLOCK(ap);
}

/* Perform a sequential search of the galloc array for an object.  Return
 * a pointer to the first object that causes the (*func) subroutine to
 * return a non zero.
 */
char *
gpai_srch( struct galloc *ap,	    /* pointer to pool anchor */
	   int            type,     /* specific type for object in list */
	   int            typeoff,  /* offset in bytes to type in object */
	   int		(*func)(),  /* pointer to caller's search routine */
	   caddr_t	  sptr )    /* pointer to search routine arguments */
{
	caddr_t *sp;
	char	*optr;
	caddr_t  ha;
	int	 rc = -1;	/* return -1 if nothing found */
	int	 ocnt;

	/* prevent alloc/free for duration of search */
	GPA_LOCK(ap);

	/* set pointer to start of object's allocation */
	sp = &ap->a_sat;

	/* get count of active objects */
	ocnt = ap->a_inuse;

	/* Search the object pool, calling func() for each active object.  */
	while (*sp)
	{
		optr = SPTOBJ(sp);
		ha = (caddr_t)HIGHBOUND(*sp, ap);
		while (optr < ha)
		{
			if (*(long *)optr == OBJ_ALLOCATED)
			{
				caddr_t obj = optr + sizeof(caddr_t);

				if ((type == 0) || (type == *((int *)&obj[typeoff])))
				{
					/* call the caller's search routine */
					rc = (*func)(obj, sptr);

					/* done if search returns non-zero */
					if (rc != 0)
						goto done;
				}

				/* if all active objects searched, we're done */
				if (--ocnt == 0)
					goto done;
			}
			/* go to next node */
			optr += objsize(ap);
		}
		/* go to next allocation */
		sp = (caddr_t *)(*sp);
	}

done:
	GPA_UNLOCK(ap);
	return rc;
}

