static char sccsid[] = "@(#)23	1.24  src/bos/kernel/vmm/POWER/userio.c, sysvmm, bos411, 9428A410j 5/17/94 14:03:11";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:	fubyte, fuword, subyte, suword
 *		long_copyinstr
 *
 * ORIGINS: 3 26 9 27 83
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

#include "vmsys.h"
#include <sys/types.h>
#include <sys/uio.h>

#include <sys/types.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/adspace.h>
#include <sys/except.h>
#include <sys/errno.h>
#include <sys/lockl.h>
#include <sys/uio.h>
#include "vmsys.h"
#include <sys/systm.h>

/*
 * Fetch a byte from source.  Returns -1 on error.
 */
fubyte(src)
caddr_t src;
{
	char c;
	int rc;

	rc = copyin(src, &c, sizeof(c));
	return( (rc == 0) ? c : -1);
}

/*
 * Fetch 4 bytes from source.  Returns -1 on error, which can't be
 * distinguished from a legitimate -1 value.
 */
fuword(src)
caddr_t src;
{

	int	w;
	int	rc;

	rc = copyin(src, &w, sizeof(w));
	return( (rc == 0) ? w : -1);
}

/*
 * Set the byte at destination to the given value. Return 0 for success,
 * -1 for failure.
 */
subyte(dest, val)
caddr_t dest;
char val;
{
	int rc;

	rc = copyout(&val, dest, sizeof(val));
	return((rc == 0) ? 0 : -1);
}

/*
 * Set the word at the given address to the given value. Return 0 for success,
 * -1 for failure.
 */
suword(dest, val)
caddr_t dest;
int val;
{
	int rc;

	rc = copyout(&val, dest, sizeof(val));
	return((rc == 0) ? 0 : -1);
}


/*
 * copy in a string to kernel space.
 * returns 0 if ok, E2BIG if insufficient space, and
 * exception value otherwise.
 */
int
long_copyinstr(from,to,max,actual)
caddr_t	from,to;
uint	max,*actual;
{
	int rc, hadlock, attach;
	caddr_t from1, to1;
	size_t max1;
	int	sregval;
	int	newsregval;

	/* give up kernel lock ?
	 */
	if (hadlock = (!CSA->prev && IS_LOCKED(&kernel_lock)))
		unlockl(&kernel_lock);

	if (curthread->t_flags & TKTHREAD)
	{
		attach = 0;
		from1 = from;
		max1 = (size_t) max;
	}
	else
	{
		attach = 1;
		sregval = as_geth(&U.U_adspace,from);
		from1 = vm_att(sregval,from);
		max1 = MIN((size_t) max,
			   (size_t) (SEGSIZE - ((uint)from & SOFFSET)));
	}

	/* xmemccpy returns 0 unless exception occurs.
	 * to1 is set on normal return to one past last
	 * character stored or NULL if max is exceeded. 
	 */
	to1 = to;
	rc = xmemccpy(&to1,from1,0,max1);
	if (rc == 0)
	{
		if (to1) 
		{
			/* Entire string was copied to target.
			 * Indicate the number of characters copied.
			 */
			*actual = to1 - to;
		}
		else
		{
			/* Either source string wasn't NULL-terminated
			 * or it crosses over to next segment.
			 */
			if (max1 != (size_t) max)
			{
				/* Source crosses over to next segment.
				 * Attach to next segment (using same sreg)
				 * and continue copy.
				 */
				to1 = to + max1;
				newsregval = as_geth(&U.U_adspace, from + max1);
				vm_seth(newsregval, from1);
				as_puth(&U.U_adspace, sregval);
				sregval = newsregval;
				from1 = (caddr_t) ((uint)from1 & SREGMSK);
				rc = xmemccpy(&to1,from1,0,(size_t)max-max1);
				if (rc == 0)
				{
					if (to1) 
					{
						/* Entire string was copied
						 * to target. Indicate the
						 * number of characters copied.
						 */
						*actual = to1 - to;
					}
					else
					{
						/* Source string wasn't NULL-
						 * terminated. Indicate the
						 * number of characters copied
						 * and not enough space to
						 * complete copy.
						 */
						rc = E2BIG;
						*actual = max;
					}
				}
			}
			else
			{
				/* Source string wasn't NULL-terminated.
				 * Indicate the number of characters copied
				 * and not enough space to complete copy.
				 */
				rc = E2BIG;
				*actual = max;
			}
		}
	}

	/* restore lock and sregs
	 */
	if (attach)
	{
		vm_det(from1);
		as_puth(&U.U_adspace, sregval);
	}
	if (hadlock)
		lockl(&kernel_lock,LOCK_SHORT);
	return rc;
}

