static char sccsid[] = "@(#)74	1.30  src/bos/kernel/proc/POWER/m_ptrace.c, sysproc, bos41J, 9515A_all 4/5/95 17:39:51";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: copyoutx
 *		pt_setrun
 *		
 *   ORIGINS: 27,83
 *
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */


#include <sys/types.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/adspace.h>
#include <sys/except.h>
#include <sys/errno.h>
#include <sys/lockl.h>
#include <sys/uio.h>
#include <sys/pseg.h>
#include <sys/intr.h>
#include <sys/sleep.h>
#include <sys/syspest.h>
#include <sys/atomic_op.h>
#include "swapper_data.h"
#include "sig.h"

/*
 * Copy bytes to destination from trusted source.
 * if destination is in text or shared library 
 * do the copy with access key 0, ie. privileged key.
 * otherwise do the copyout with normal privilege.
 */
copyoutx(caddr_t src, volatile caddr_t dest, int count)
{
	uint sreg, srval, rc;
        label_t buf;

	sreg = (ulong) dest >> SEGSHIFT;
	srval = U.U_adspace.srval[sreg];
	if (sreg == TEXTSEG || sreg == SHTEXTSEG)
	{
		/* allow write to text segments */
		U.U_adspace.srval[sreg] = vm_setkey(srval,VM_PRIV);
	}

	/* copyout the data. to flush cache we have to load
	 * a segment register because in general destination
	 * is not addressable at this point.
	 */
	if ((rc = copyout(src, dest, count)) != 0)
		return (rc);

	dest = vm_att(srval, dest);

        if (setjmpx(&buf) != 0) {
                vm_det(dest);
                return EFAULT;
        }

	vm_cflush(dest, count);
        clrjmpx(&buf);
	vm_det(dest);

	/* reset key */
	if (sreg == TEXTSEG || sreg == SHTEXTSEG)
	{
		U.U_adspace.srval[sreg] = srval;
	}

	return rc;
}

/*
 * NAME: pt_setrun()
 *
 * FUNCTION: calls setrq() for traced process, routine must be pinned.
 *
 */
void
pt_setrun(tid_t *tlist)
{
	int		cnt;
	struct thread	*trt;
	tid_t		*tmp;


	/* 
	 * Don't continue the specified threads, if
	 * there is a thread in the process that is ready 
	 * to enter procxmt.
	 */
	trt = curthread;
	for (cnt = trt->t_procp->p_threadcount; cnt > 0; cnt--)
	{
		if (trt->t_flags & TTRCSIG) {
                        stop(trt->t_procp, trt->t_cursig);
                        return;
                }
		trt = trt->t_nextthread;
	}


	trt = THREADPTR(*tlist);

#ifdef _POWER_MP
        simple_lock(&proc_int_lock);
#endif

	/*
	 * Start thread list
	 */
	for ( cnt = 0; 
	      *tlist != NULL_TID && cnt < MAXTHREADS; 
	      tlist++, cnt++)
	{
		trt = THREADPTR(*tlist);
		if ((trt->t_state == TSSTOP) && 
		    (!(trt->t_flags & TSETSTATE) || trt->t_suspend))
		{
			switch(trt->t_wtype) {
			case TWEVENT :
                            if (trt->t_flags & TWAKEONSIG) {
                                 if (trt->t_flags & (TINTR|TSUSP))
                                      setrq(trt, E_WKX_PREEMPT, RQTAIL);
                                 else if (SIG_MAYBE(trt, trt->t_procp) &&
                                          check_sig(trt, 0)) {
                                      trt->t_result = THREAD_INTERRUPTED;
                                      trt->t_flags |= TSIGAVAIL;
                                      setrq(trt, E_WKX_PREEMPT, RQTAIL);
                                 }
                                 else {
                                      trt->t_flags &= ~TSIGAVAIL;
                                      trt->t_state = TSSLEEP;
                                 }
                                 break;
                            }
			    trt->t_state = TSSLEEP;
			    break;
			case TNOWAIT :
			    setrq(trt, E_WKX_PREEMPT, RQTAIL);
			    break;
			default :
			    assert(FALSE);
			}
	        	trt->t_procp->p_suspended--;
		}
	}

#ifdef _POWER_MP
        simple_unlock(&proc_int_lock);
#endif

}
