static char sccsid[] = "@(#)99	1.9  src/bos/kernel/pfs/genalloc.c, syspfs, bos411, 9428A410j 7/7/94 16:52:54";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: geninit, genalloc, genfree
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "jfs/genalloc.h"
#include "sys/param.h"
#include "sys/errno.h"
#include "sys/syspest.h"
#include "sys/malloc.h"

/*
 * Definitions
 */
genalloc_t *gen_table;		/* genalloc_t table */
#define GENT	(PAGESIZE/sizeof(genalloc_t))
int gent = GENT;		/* # of gen_table elements */
#define IGENT	(GENT/2)
int igent = IGENT;		/* # of initial gen_table elements */

/* get pointer to the freelist chain field of the object */
#define atofp(a, ap)	(*(caddr_t *)((caddr_t)(a) + (ap)->a_froff))

/* 
 * Declarations
 */
caddr_t genalloc();

/*
 * NAME:	geninit (ap)
 *
 * FUNCTION:	Commom table manager initialization.  Each "user"
 *		is responsible for calling geninit() to initialize
 *		its allocation.  The first caller of geninit() causes
 *		the allocation of the genalloc table itself.  Therefore
 *		the 1st entry in the genalloc array will always be for the
 *		genalloc allocation itself.
 *
 * PARAMETERS:	tblelms	- # of total elements in table
 *		nelms	- # of initial allocation
 *		objsz	- object size
 *		off	- offset in struct that is avail to be used as 
 *			  a free list pointer(must be on long word boundary).
 *		id	- Allocation id, debug info.  This is a char array
 *			  that can be used to identify the allocation when
 *			  doing postmortem analysis.
 *		app	- returned allocation info
 *
 * RETURNS:	0	- success
 *		EINVAL	- initial arguments invalid
 *		ENOMEM	- failure
 */

geninit (tblelms, nelms, objsz, off, handle, app)
int tblelms;			/* # entries in table */
int nelms;			/* initial # entries to initialize */
int objsz;			/* object size */
off_t off;			/* Offset in object to be used as freelist */
caddr_t handle;			/* object id, optional */
genalloc_t **app;		/* returned allocation info */
{
	genalloc_t *ap;
	caddr_t a;
	int sz, i;
	ulong *up;

	/*
	 * First check validity of the arguments
	 */

	if (objsz < sizeof (caddr_t) || nelms > tblelms || 
		off >= objsz || (off & 0x3) != 0)
		return EINVAL;

	/*
	 * allocate and initialize the genalloc_t table
	 */
	if (gen_table == NULL)
	{	caddr_t id = "GENTBL";
		genalloc_t x, *xp = &x;
		off_t 	xoff;

		/* Bootstrap useing x.  */
		gen_table = xp;
		freeobj (xp) = (caddr_t) xp;
		xoff = (caddr_t) &xp->a_froff - (caddr_t) xp;
		xp->a_froff = xoff;

		/* One level recursion */
		if (geninit(gent, igent, sizeof (genalloc_t), xoff, id, &xp))
			panic ("Geninit: cannot initialize gen_table");

		/* Adjust freeobj pointers and finish initialization
		 */
		(void) genalloc(xp);
		gen_table = (genalloc_t *) xp->a_table;
		*gen_table = *xp;

		/* Fall through to complete original request	*/
	}

	/*
	 * allocate and initialize the object table
	 */

	/* allocate the allocation structure of the object table
	 */
	ap = (genalloc_t *) genalloc(gen_table);
	if (ap == NULL)
	{	
		BUGPR(("Gen_init: gen_table overflow"));
		return ENOMEM;
	}

	/* allocate the object table
	 */
	sz = objsz * tblelms;
	if ((ap->a_table = (caddr_t) malloc((uint)sz)) == NULL)
		return ENOMEM;

	/* zero out the initial number of objects to initialize
	 */
	bzero(ap->a_table, nelms * objsz);

	/* initialize the allocation structure of the object table:
	 * address of end of table, object size and freelist offset in object.
	 */
	eotbl(ap) = ap->a_table + sz;
	objsize(ap) = objsz;
	ap->a_froff = off;

	/* construct free list of a singly-linked (linked via freelist 
	 * offset in object) NULL-terminated list in the new object table
	 * with the initial number of objects to initialize
	 */
	freeobj(ap) = a = ap->a_table;

	for (i = 1; i < nelms; i++, a += objsz)
		atofp(a, ap) = a + objsz;
	atofp(a, ap) = NULL;

	/* set high water mark of the object table with the address of 
	 * the next object of the last object of the freelist
	 */
	ap->a_hiwater = a + objsz;

	/* set the object id in the allocation structure of the object table
	 */
	strncpy (ap->a_handle, handle, sizeof (ap->a_handle));

	*app = ap;
	return 0;
}


/*
 * NAME:	genalloc (ap)
 *
 * FUNCTION:	Commom allocator for large tables
 *
 * PARAMETERS:	ap	- allocation struct as defined in <sys/genalloc.h>
 *
 * RETURNS:	Pointer to allocated object
 *		NULL on overflow
 *
 * SERIALIZATION: ICACHE_LOCK() held on entry/exit (getip()/getdp()).
 */

caddr_t
genalloc (ap)
genalloc_t *ap;
{
	caddr_t a;

	/* allocate new objects if freelist is empty
	 */
	if (freeobj(ap) == NULL)
	{
		caddr_t pb;
		int nelms, sz, nalloc, nfree, i;

		/* check for the object table overflow
		 */
		if (ap->a_hiwater >= eotbl(ap))
			return NULL;

		/* compute allocation size to allocate new object(s):
		 * remaining area of current page + MAX(PAGESIZE, object size)
		 */
		sz = MAX(PAGESIZE, objsize(ap));

		/* round up the start address to the page boundary */
		pb = (caddr_t)(((int) ap->a_hiwater + (sz-1)) & ~(sz-1));

		/* truncate allocation size to be within the table */
		nalloc = (pb + sz) - ap->a_hiwater;
		nfree = eotbl(ap) - ap->a_hiwater;
		nalloc = MIN(nalloc, nfree);
		if (nalloc < objsize(ap))
			return NULL;

		/* zero out the allocation area
		 */
		bzero(ap->a_hiwater, nalloc);

		/* insert object of allocation area into freelist
		 */
		nelms  = nalloc / objsize(ap);
		freeobj(ap) = a = ap->a_hiwater;
		for (i = 1; i < nelms; i++, a += objsize(ap))
			atofp(a, ap) = a + objsize(ap);
		atofp(a, ap) = NULL;

		/* set high water mark of the object table
	 	 */
		ap->a_hiwater = a + objsize(ap);
	}

	/* remove/return object from head of free list
	 */
	a = (caddr_t) freeobj(ap);
	freeobj(ap) = atofp(a, ap);
	atofp(a, ap) = NULL;

	return a;
}


/*
 * NAME:	genfree (ap, addr)
 *
 * FUNCTION:	Free objects previously allocated by the commom
 *		allocator, genalloc().
 *
 * PARAMETERS:	ap	- allocation struct as defined in <sys/genalloc.h>
 *		addr	- pointer to the object
 *
 * RETURNS:	None
 *
 * SERIALIZATION: ICACHE_LOCK() held on entry/exit (iunhash()/dquhash()).
 */

genfree (ap, addr)
genalloc_t *ap;
caddr_t *addr;
{
	/* insert at head of free list
	 */
	atofp(addr, ap) = freeobj(ap);
	freeobj(ap) = (caddr_t) addr;
}
