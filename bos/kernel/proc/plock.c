static char sccsid[] = "@(#)67  1.13.1.7  src/bos/kernel/proc/plock.c, sysproc, bos411, 9428A410j 6/21/94 19:30:21";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: PGBNDDN
 *		PGBNDUP
 *		plock
 *		plock_finddata
 *		plock_findtext
 *		plock_pinvec
 *		plock_unpinvec
 *		
 *   ORIGINS: 27, 3, 83
 *
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */


#include <sys/types.h>
#include <sys/errno.h>
#include <sys/user.h>
#include <sys/lock.h>
#include <sys/pseg.h>
#include <sys/trchkid.h>
#include <sys/priv.h>
#include <sys/syspest.h>
#include <sys/uio.h>
#include <sys/adspace.h>

#define	PGBNDUP(x)	( ((x)+PAGESIZE-1) & ~((long)(PAGESIZE-1)) )
#define	PGBNDDN(x)	( (x) & ~((long)(PAGESIZE-1)) )

/*
* The structure plock_vec is used to compute what address ranges need
* to be pinned or unpinned.  The functions plock_findtext and
* plock_finddata fill in the appropriate fields in the plock_vec then
* plock_pinvec is called to pin each area.  This makes cleanup easier
* in the failure case too since we know exactly what we've done so
* far.  MAX_PLOCK_SLOTS is 4 since the maximum number of things to
* pin is text, data, big data, and stack.  Plock_findtext and
* plock_finddata are also used in the UNLOCK case to figure out what
* things need to be unpinned (via plock_unpinvec).  The nlocks field
* in the structure identifies the number of valid iovec entries.
*/

#define	MAX_PLOCK_SLOTS	4
struct plock_vec
{
	short		nlocks;
	struct iovec	v[MAX_PLOCK_SLOTS];
};
static void plock_findtext(), plock_finddata(), plock_unpinvec();
static plock_pinvec();

/*
 * NAME: plock()
 *
 * FUNCTION: Locks/unlocks the process, text, or data in memory.
 *
 * EXECUTION ENVIRONMENT: This routine may only be called by a process
 *                        This routine may page fault
 *
 * RETURN VALUE DESCRIPTION:
 *	 0	= successful completion
 *	-1	= failed, errno is set to indicate the error
 */
int
plock (int op)
{
	struct plock_vec pv;
	struct iovec iov;
	int	n;
	int	err = 0;	/* value for u_error */

	TRCHKLT_SYSC(PLOCK, op);

	pv.nlocks = 0;

	if (!priv_chk(SET_PROC_RAC)) /* check for PIN_MEMORY system perm */
		err = EPERM;	/* permissions do not allow pinning memory */
	else
	{
		simple_lock(&U.U_handy_lock); /* serialize access to U.U_lock */

		switch (op)
		{
	   	case TXTLOCK:	/* pin text section in memory */
			if (U.U_lock & (PROCLOCK | TXTLOCK))
			{
				err = EINVAL;
				break;
			}

			/* Find the text area and pin it */

			plock_findtext(&pv);
			if (!(err = plock_pinvec(&pv)))
				U.U_lock |= TXTLOCK;
			break;

	 	case DATLOCK:	/* pin data section in memory */
			if (U.U_lock & (PROCLOCK | DATLOCK))
			{
				err = EINVAL;
				break;
			}

			/* Find the data area(s) and pin it (them). */

			plock_finddata(&pv);
			if (!(err = plock_pinvec(&pv)))
				U.U_lock |= DATLOCK;
			break;

		case PROCLOCK:	/* pin text & data section in memory */
			if (U.U_lock & (PROCLOCK | TXTLOCK | DATLOCK))
			{
				err = EINVAL;
				break;
			}

			/* Find the text and data areas, and pin them */

			plock_findtext(&pv);
			plock_finddata(&pv);
			if (!(err = plock_pinvec(&pv)))
				U.U_lock |= PROCLOCK;
			break;

		case UNLOCK:	/* unpin locked section in memory */
			if(!(U.U_lock & (PROCLOCK|TXTLOCK|DATLOCK)))
			{
				err = EINVAL;
				break;
			}

			/*
			* If the process has the text pinned, then
			* go find the text, likewise for data.  After
			* these calls the plock_vec describes the
			* areas than need to be unpinned
			*/

			if (U.U_lock & (PROCLOCK|TXTLOCK))
				plock_findtext(&pv);
			if (U.U_lock & (PROCLOCK|DATLOCK))
				plock_finddata(&pv);

			/* unpin what we found and clear the lock bits */
			plock_unpinvec(&pv);
			U.U_lock &= ~(PROCLOCK|TXTLOCK|DATLOCK);
			break;

	    	default:
			err = EINVAL;
			break;
		}

		simple_unlock(&U.U_handy_lock);
	}
	if (err)
	{
		u.u_error = err;
		return(-1);
	}
	return(0);
}

/*
* plock_findtext - Modify the plock_vec passed to reflect the current
*	location and size of the text and adjust nlocks.  No return
*	value.
*/

static void
plock_findtext(struct plock_vec *pvp)
{
	struct iovec *iov = &pvp->v[pvp->nlocks];

	pvp->nlocks++;
	iov->iov_base = U.U_tstart;
	iov->iov_len = U.U_tsize;
}

/*
* plock_finddata - Modify the plock_vec passed to reflect the current
*	location and size of the text area and adjust nlocks.  No return
*	value.
*/

static void
plock_finddata(struct plock_vec *pvp)
{
	int dsize, smax;
	struct iovec *iov = &pvp->v[pvp->nlocks];

	/*
	* If the process has big data, then there are possibly two areas
	* that will need to be pinned: one in segment 2, the other beginning
	* in segment 3.  It is possible that there is no data in seg 2.
	* This code assumes that pinu can cross segment boundaries.
	*/
	if (U.U_dsize > SEGSIZE)
	{
		if (U.U_sdsize)
		{
			pvp->nlocks++;
			iov->iov_base = (caddr_t)DATAORG;
			iov++->iov_len = U.U_sdsize;
		}

		pvp->nlocks++;
		iov->iov_base = (caddr_t)BDATAORG;
		iov++->iov_len = U.U_dsize - SEGSIZE;
	}
	else
	{
		pvp->nlocks++;
		iov->iov_base = (caddr_t)DATAORG;
		iov++->iov_len = U.U_dsize;
	}

	/* don't forget the stack */
	pvp->nlocks++;

	/*
	 * Calculate the stack addresses. 
	 */
	dsize = U.U_dsize < SEGSIZE ? U.U_dsize : U.U_sdsize;
	smax = SEGSIZE - K_REGION_SIZE - dsize;

	/* calculate down limits */
	if ((unsigned)U.U_smax < smax)
		smax = U.U_smax;

	iov->iov_len = PGBNDUP(smax);
	iov->iov_base = (caddr_t)((PRIVORG + U_REGION_SIZE) - PGBNDUP(smax));
}

/*
* plock_pinvec - Pin the address ranges described by the plock_vec.
*
* Return value - any failures from pinx or pinu.
*/

static
plock_pinvec(struct plock_vec *pvp)
{
	int rc, i;
	struct iovec *iov;

	/* Loop through the areas needing attention */

	for (rc = 0, i = 0, iov = pvp->v; i < pvp->nlocks; i++, iov++)
	{
		/* text must be pinned with pinx */
		if (((int)iov->iov_base >> SEGSHIFT) == TEXTSEG)
		{
			/* The TEXTSEG is hidden by KSTACKSEG.
			 * The address space could be changed by other
			 * threads in ptrace.
			 */
			caddr_t text_addr;
			vmhandle_t srval;

			srval = as_geth(&U.U_adspace,iov->iov_base);
			text_addr = vm_att(srval,iov->iov_base);
			rc = pinx((uint)text_addr,iov->iov_len);
			vm_det(text_addr);
			as_puth(&U.U_adspace,srval);
		}
		else
			rc = pinu(iov->iov_base, iov->iov_len, 
					(short)UIO_USERSPACE);

		/*
		* A pin call failed, probably due to lack of memory.
		* Now we must undo all of the damage done so far.  This
		* is where the plock_vec is handy.  All that needs to be
		* done is adjust the nlocks field to reflect the number
		* of regions successfully pinned and call plock_unpinvec.
		*/

		if (rc)
		{
			pvp->nlocks = i;
			plock_unpinvec(pvp);
			return rc;
		}
	}
	return 0;
}

/*
* plock_unpinvec - Unpin areas described in the plock_vec
*
* return value   - none
*
* Notes:  We deliberately ignore the return value from unpinu.  The
* alternative would be to panic since a failure implies that the code
* is broken.
*/

static void
plock_unpinvec(struct plock_vec *pvp)
{
	int i;
	struct iovec *iov;

	for (i = 0, iov = pvp->v; i < pvp->nlocks; i++, iov++)
		(void) unpinu(iov->iov_base, iov->iov_len,
				(short)UIO_USERSPACE);
}
