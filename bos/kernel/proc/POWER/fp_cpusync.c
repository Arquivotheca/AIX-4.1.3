static char sccsid[] = "@(#)80	1.4  src/bos/kernel/proc/POWER/fp_cpusync.c, sysproc, bos411, 9428A410j 3/31/93 09:48:19";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: fp_cpusync
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992, 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <fptrap.h>
#include <sys/fp_cpusync.h>

extern int _fp_trapstate_ker(int);

/*
 * NAME: fp_cpusync()
 *
 * FUNCTION:
 *	replaces fp_cpusync() system call; sets or queries a process's state
 *      as regards the MSR(FE and/or IE) bits.
 *
 * EXECUTION ENVIRONMENT:
 *      Preemptable, Interrupts enabled on input.
 *      Runs on Pinned Kernel Stack for SVCs.
 *      An AIX Signal will not be delivered until this code is through
 *        running. This serializes `u.u_save.fpinfo' updates.
 *      This code is pageable.
 *
 *   INPUT: 
 *      flag - FP_SYNC_OFF
 *             FP_SYNC_ON
 *             FP_SYNC_QUERY
 *
 *   OUTPUT:
 *      For all valid input, above, this ALWAYS returns the
 *      previous state of the task as regards the `logical' setting
 *      of the FE bit.
 *
 *      If `flag', above is out of range the following is returned:
 *     
 *             FP_SYNC_ERROR
 *
 * NOTES:
 *
 * This is only intended to make existing code AIX 3.2 code work, and thus
 * only knows about precise trapping mode.
 */

int
fp_cpusync( int flag )
  {
  int	 old_state;		/* current state of process */

  /*
   * The only three argument that are valid are
   * 	FP_SYNC_OFF
   * 	FP_SYNC_ON
   * 	FP_SYNC_QUERY
   *
   * For FP_SYNC_ON and FP_SYNC_OFF, we set the requested mode
   * using fp_trapstate.  For FP_SYNC_QUERY, we query the current
   * state using fp_trapstate.  In all cases, fp_trapstate returns
   * either the previous or current state of the process, which is the
   * return code from this call.  However, the current or old state
   * can be something that fp_cpusync doesn't understand (i.e. an
   * imprecise mode) so the return code must be adjusted accordingly.
   * Only FP_TRAPSTATE_ON is identical to FP_SYNC_ON; any other mode
   * is equivalent to FP_SYNC_OFF to fp_cpusync.
   *
   * Any other argument generates a return of FP_SYNC_ERROR.
   */

  switch (flag)
      {
    case FP_SYNC_ON:
      old_state = _fp_trapstate_ker(FP_TRAPSTATE_PRECISE);
      break;
      
    case FP_SYNC_OFF:
      old_state = _fp_trapstate_ker(FP_TRAPSTATE_OFF);
      break;

    case FP_SYNC_QUERY:
      old_state = _fp_trapstate_ker(FP_TRAPSTATE_QUERY);
      break;

    default:
      return FP_SYNC_ERROR;
      }      

  if (old_state == FP_TRAPSTATE_PRECISE)
    return FP_SYNC_ON;
  else
    return FP_SYNC_OFF;
  }
     
