static char sccsid[] = "@(#)25        1.13.1.7  src/bos/kernel/s/auth/cred.c, syssauth, bos411, 9428A410j 7/6/94 15:28:27";

/*
 * COMPONENT_NAME:  TCBAUTH
 *
 * FUNCTIONS:  crget(), crfree(), crcopy(), crdup(), crset(),
 *             crlock(), crunlock(), credlock(), credunlock(), crref(),
 *             _crget(), _crfree(), _crcopy(), _crdup()
 *
 * ORIGINS: 27 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *   LEVEL 1,  5 Years Bull Confidential Information
 */

/*
 * Kernel service routines to manipulate credentials
 */

#include "sys/param.h"
#include "sys/systm.h"
#include "grp.h"
#include "sys/user.h"
#include "sys/intr.h"
#include "sys/malloc.h"
#include "sys/limits.h"
#include "sys/syspest.h"
#include "sys/lockl.h"
#include "sys/lock_alloc.h"
#include "sys/lockname.h"
#include "sys/cred.h"
#include "crlock.h"

extern  Simple_lock cred_lock;

/*
 * Routines to allocate and free credentials structures
 */

static void credalloc();

volatile int crtotal = 0;


#define	CL_MAGIC	(0x9357fdb1)
struct credlist {
	int	cl_magic;
	struct	credlist *cl_next;
};

static	struct	credlist *crfreelist = NULL;
Simple_lock cred_lock;

/*
 * Allocate a zeroed cred structure and increment its reference count.
 *
 */

/* _crget() : crget() wthout lock (internal function) */

struct ucred *
_crget()
{
	register struct ucred *cr;

	/*
	 * Get the cred lock, get the next cred off of the freelist,
	 * update the freelist, and unlock the cred list.
	 */
	if (! crfreelist) 
		(void)credalloc();	/* get a page of creds */

	/*
	 * I have a valid value in crfreelist and the cred list is
	 * locked - no one else can steal my cred before I get
	 * to it.
	 */

	cr = (struct ucred *) crfreelist;
	crfreelist = crfreelist->cl_next;

	/*
	 * We are assured that "cr" is non-NULL, so I will
	 * initialize it to all zeros and mark it as held.
	 * I also keep a count of total creds as a sanity check
	 * against the cred list being corrupted.
	 */

	assert(cr != 0);
	assert(((struct credlist *) cr)->cl_magic == CL_MAGIC);
	bzero((caddr_t)cr, sizeof(*cr));

	/*
	 * Don't do crhold here as it leads to dead lock
	 */
        cr->cr_ref = 1; 

	/*
	 * I have gotten the cred structure off the free list
	 * and pointed to the next free element - now I can
	 * unlock the cred list.
	 */
	return(cr);
}

struct ucred *
crget()
{
	register struct ucred *cr;

	/*
	 * Get the cred lock, get the next cred off of the freelist,
	 * update the freelist, and unlock the cred list.
	 */
	CRED_LOCK();
	if (! crfreelist) 
		(void)credalloc();	/* get a page of creds */

	/*
	 * I have a valid value in crfreelist and the cred list is
	 * locked - no one else can steal my cred before I get
	 * to it.
	 */

	cr = (struct ucred *) crfreelist;
	crfreelist = crfreelist->cl_next;

	CRED_UNLOCK();
	/*
	 * We are assured that "cr" is non-NULL, so I will
	 * initialize it to all zeros and mark it as held.
	 * I also keep a count of total creds as a sanity check
	 * against the cred list being corrupted.
	 */

	assert(cr != 0);
	assert(((struct credlist *) cr)->cl_magic == CL_MAGIC);
	bzero((caddr_t)cr, sizeof(*cr));

	/*
	 * Don't do crhold here as it leads to dead lock
	 */
        cr->cr_ref = 1; 

	/*
	 * I have gotten the cred structure off the free list
	 * and pointed to the next free element - now I can
	 * unlock the cred list.
	 */
	return(cr);
}

/*
 * Free a cred structure.
 * Throws away space when ref count gets to 0.
 */

void
_crfree(cr)
struct ucred *cr;
{
	assert(((struct credlist *)cr)->cl_magic != CL_MAGIC);

	if (--cr->cr_ref > 0) 
	{
		/*
		 * This cred structure is still being used. Simply return
		 */
		return;
	}

	/*
	 * Stick the cred structure back on the freelist.
	 */
	((struct credlist *)cr)->cl_next = crfreelist;
	((struct credlist *)cr)->cl_magic = CL_MAGIC;
   
	crfreelist = (struct credlist *)cr;
}

void
crfree(cr)
struct ucred *cr;
{

	/*
	 * Serialize access to the free list by holding the cred lock
	 */
	CRED_LOCK();
	_crfree (cr);
	CRED_UNLOCK();
}

/*
 * Copy cred structure to a new one and free the old one.
 */

struct ucred *
_crcopy(cr)
struct ucred *cr;
{
	struct ucred *newcr, *_crdup();

	ASSERT(cr != 0);

	/*
	 * No need to duplicate and copy if this is the only process
	 * having the cred
	 */
	if (cr->cr_ref == 1)
		return(cr);

	newcr = _crdup(cr);
	ASSERT(newcr != 0);
	_crfree(cr);
	return(newcr);
}

struct ucred *
crcopy(cr)
struct ucred *cr;
{
	struct ucred *newcr, *crdup();

	ASSERT(cr != 0);

	/*
	 * No need to duplicate and copy if this is the only process
	 * having the cred
	 */
	if (cr->cr_ref == 1)
		return(cr);

	newcr = crdup(cr);
	ASSERT(newcr != 0);
	crfree(cr);
	return(newcr);
}

/*
 * Dup cred struct to a new held one.
 */

struct ucred *
_crdup(cr)
struct ucred *cr;
{
	struct ucred *newcr;
	struct ucred *_crget();

	ASSERT(cr != 0);
	newcr = _crget();
	ASSERT(newcr != 0);
	*newcr = *cr;
	newcr->cr_ref = 1;

	return(newcr);
}

struct ucred *
crdup(cr)
struct ucred *cr;
{
	struct ucred *newcr;
	struct ucred *crget();

	ASSERT(cr != 0);
	newcr = crget();
	ASSERT(newcr != 0);
	*newcr = *cr;
	newcr->cr_ref = 1;

	return(newcr);
}

/*
 * Initialize cred freelist and initial struct user's cred.
 */

void
credinit() 
{
	/*
	 * Alloctate cred_lock and initialize
	 */
        lock_alloc(&cred_lock,LOCK_ALLOC_PAGED,CRED_LOCK_CLASS,-1);
        simple_lock_init(&cred_lock);

	/*
	 * Lock cred_lock before calling cred_alloc
	 */
	CRED_LOCK();

	(void)credalloc();	/* get a page of creds */

	CRED_UNLOCK();

	/*
	 * No need to take CRED_LOCK as the boot process is 
	 * single threaded
	 */
	U.U_cred = crget();	/* get one for initial u. */
}

/*
 * do a per page allocation for cred structs.
 * NOTE that it is used by crget() and credinit().
 * NOTE: should we keep track of how many pages of creds are gotten???
 * NOTE: should we print warning msgs if > about 4 pages gotten???
 * MP : The free list must be locked prior to calling credalloc()
 */
void
credalloc()
{
	struct ucred *cr;
	caddr_t	page;
	int	i;

	assert((page = (caddr_t)xmalloc(PAGESIZE, 12, kernel_heap)) != NULL);

	bzero(page, PAGESIZE);
	i = PAGESIZE / sizeof(struct ucred);	/* how many per page */
	cr = (struct ucred *)page;


	/*
	 * Step through the entire page of cred structures putting each
	 * one onto the free list
	 */

	while (i--) 
        {
		((struct credlist *)cr)->cl_next = crfreelist;
		((struct credlist *)cr)->cl_magic = CL_MAGIC;

		crfreelist = (struct credlist *)cr;
		crtotal++;
		cr++;
	}
}

/*
 * Lock the cred lock in the u structure
 */

void crlock()
{
	CR_LOCK();
}

/*
 * Unlock the cred lock in the u structure
 */

void crunlock()
{
	CR_UNLOCK();
}

/*
 * Lock the cred_lock lock
 */

void credlock()
{
	CRED_LOCK(); 	
}

/*
 * Unlock the cred_lock lock
 */

void credunlock()
{
	CRED_UNLOCK(); 	
}

/*
 * Return a pointer to the cred structure. Inc ref count
 */

struct ucred *
crref(void)
{
	struct ucred *temp;

	CRED_LOCK();
	temp = U.U_cred;
	assert(((struct credlist *)temp)->cl_magic != CL_MAGIC);
	temp->cr_ref++;
	CRED_UNLOCK();
	return(temp);
}

/*
 * set the process credentials
 */

void
crset(struct ucred *ucp)
{
	CRED_LOCK();
        if (U.U_cred)
		_crfree(U.U_cred);
        U.U_cred = ucp;
	CRED_UNLOCK();
}

void
crhold(struct ucred *ucp)
{
        CRED_LOCK();
	assert(((struct credlist *)ucp)->cl_magic != CL_MAGIC);
        ucp->cr_ref++;
        CRED_UNLOCK();
}

