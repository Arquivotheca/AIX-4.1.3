static char sccsid[] = "@(#)56	1.27  src/bos/kernel/proc/sbreak.c, sysproc, bos411, 9428A410j 10/28/93 18:54:33";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: PGBNDUP
 *		sbreak
 *		
 *
 *   ORIGINS: 27,3
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <sys/types.h>
#include <sys/user.h>
#include <sys/pseg.h>
#include <sys/errno.h>
#include <sys/lock.h>
#include <sys/trchkid.h>
#include <sys/uio.h>

/*
 * NAME: sbreak
 *
 * FUNCTION: Changes data allocation in process private segment.
 *
 *	This is break system call, user uses sbrk() or brk().
 *
 * NOTE: This must work for positive and negative delta sizes.
 *	 This code assumes that pin() and unpin() will handle
 *	 exceptions that occur and simply return an error
 *
 * EXECUTION ENVIRONMENT:
 *	preemptable
 *	may page fault
 *
 * RETURNS:
 *	 0	upon success
 *	-1	if unsuccessful, errno indicates the reason
 */

#define	PGBNDUP(x)	( ((x)+PAGESIZE-1) & ~((long)(PAGESIZE-1)) )

int
sbreak (char * newend)	/* new first invalid address of data area */
{
	int	delta;	/* number of bytes to add to data area */
	int	segno;
	int	sid;
	int	rc;
	int 	downlim;
	int     dsize;
	int 	lock_held;


	if (!(lock_held = lock_mine(&U.U_handy_lock)))
		simple_lock(&U.U_handy_lock);

	/* number of bytes the data area grows */
	delta = (PGBNDUP((ulong)(newend - PRIVORG))) - u.u_dsize;

	segno = (unsigned) newend >> SEGSHIFT;

	/*
	 * The new break value is rejected if it is below the process
	 * private segment, or above the maximum big data segment number,
	 * or beyond the established resource limit and the limit isn't
	 * infinity, or is above the process private segment and big data 
	 * hasn't been enabled, or is trying to move from above the process 
	 * private segment back into the process private segment.
	 *
	 * we DO NOT check the rounded value.  This is intentional.
	 * as a consequence, is dmax is not on a page boundary
	 * the program can actually touch the rest of the page.
	 * We judge this better than fooling around with the
	 * values set by setrlimit.
	 */

	if ((segno < PRIVSEG) || (segno > BDATASEGMAX)
	|| ((unsigned) newend > (unsigned) u.u_dmax + PRIVORG
		&& u.u_dmax != RLIM_INFINITY)
	|| (segno > PRIVSEG && !(u.u_segst[segno].segflag & SEG_WORKING))
	|| ((unsigned)u.u_dsize > SEGSIZE && segno == PRIVSEG)
	|| (segno == PRIVSEG && (unsigned)newend > PRIVORG + U_REGION_SIZE))
	{
		if (!lock_held)
			simple_unlock(&U.U_handy_lock);
		u.u_error = ENOMEM;
		return -1;
	}

	/* Are we decreasing the size of a pinned data area? 
         * vms_limits will not fail in this case.
         */
	if (u.u_lock & (PROCLOCK | DATLOCK) && delta < 0)
		/*
		* Unpin previously pinned data from the new break
		* value to the old break value.
		*/
		(void) unpinu((caddr_t)(PRIVORG + u.u_dsize + delta),
				-delta, (short) UIO_USERSPACE);

	dsize = u.u_dsize + delta;

	/*
	 * If the new break is still in the process private segment,
	 * adjust the limits accordingly.
	 */
	if ((unsigned) newend < PRIVORG+SEGSIZE)
	{
		downlim = SEGSIZE - dsize - K_REGION_SIZE;
		if (u.u_smax < downlim)
			downlim = u.u_smax;
		if (vms_limits(SRTOSID(u.u_procp->p_adspace), dsize,
			downlim + K_REGION_SIZE))
		{
			if (!lock_held)
				simple_unlock(&U.U_handy_lock);
                        u.u_error = ENOMEM;
                        return -1;
                }
	}

	/* Adjust the limits on any secondary working storage segments. */

	for (segno = BDATASEG; segno <= BDATASEGMAX; segno++ )
	{
		if (!(u.u_segst[segno].segflag & SEG_WORKING))
			break;

		sid = SRTOSID(u.u_segst[segno].ss_srval);

		/*
		* If this segment contains the new break, then adjust
		* the upward limit of this segment to reflect the new 
		* break's offset within the segment.  If this segment
		* is wholly below the new break, then set the upward 
		* limit to be SEGSIZE (ie, no limit).  If this segment
		* is wholly above the new break, then set the upward
		* limit to zero (ie, no access).  In all cases, the
		* downward limit is zero.
		*/
		if ((unsigned) newend > segno << SEGSHIFT)
			if ((unsigned) newend < ((segno+1) << SEGSHIFT))
				rc = vms_limits(sid,SEGOFFSET(newend),0);
			else
				rc = vms_limits(sid,SEGSIZE,0);
		else
			rc = vms_limits(sid,0,0);

		if (rc)
                {
                       sbreak(newend-delta);   /* to iterate is human, ... */
			if (!lock_held)
				simple_unlock(&U.U_handy_lock);
                       u.u_error = ENOMEM;
                       return -1;
                }

	}

        /*
         * Stack should be included in the count below, but is not
         * because the vmm does not keep statistics on the stack.
         * u.u_ssize used to be included but is not anymore, because it
         * can be set to an unlimited value by setrlimit() and sbreak().
         */
        u.u_dsize = dsize;
        u.u_procp->p_size = btoc( u.u_tsize +
                (u.u_dsize > SEGSIZE ? u.u_dsize - SEGSIZE + u.u_sdsize :
                                       u.u_dsize) );
	/*
	* The only time the break value can be moved from DATASEG to
	* a higher segment is during exec.  In that case, a plock
	* cannot be in effect.  So pinning from the old break value
	* (newend-delta) for delta bytes will work for both the large
	* data process and the normal process.
	*/
	if (u.u_lock & (PROCLOCK | DATLOCK) && delta > 0)
		if (rc = pinu((caddr_t)(PRIVORG + u.u_dsize - delta),
				delta, (short)UIO_USERSPACE))
		{
			sbreak(newend-delta);	/* to recurse, divine */
			if (!lock_held)
				simple_unlock(&U.U_handy_lock);
			u.u_error = rc;
			return -1;
		}

	if (!lock_held)
		simple_unlock(&U.U_handy_lock);

	/* normal return */
	TRCHKGT_SYSC(SBREAK, newend, NULL, NULL, NULL, NULL);
	return 0;
}
