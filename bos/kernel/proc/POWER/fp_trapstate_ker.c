static char sccsid[] = "@(#)29	1.5  src/bos/kernel/proc/POWER/fp_trapstate_ker.c, sysproc, bos411, 9428A410j 4/13/94 14:31:24";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: _fp_trapstate_ker
 *		
 *
 *   ORIGINS: 27,83
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/user.h>
#include <sys/machine.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/fp_cpusync.h>
#include <sys/systemcfg.h>
#include <fptrap.h>
#include <sys/systm.h>
#include <sys/mstsave.h>
#include <sys/syspest.h>/* to define the assert and ASSERT macros	*/

extern   void disown_fp();

/*
 * NAME: _fp_trapstate_ker()
 *
 * FUNCTION:
 *	_fp_trapstate_ker() system call; sets or queries a process's state
 *      as regards the MSR(FE and/or IE) bits.
 *
 * EXECUTION ENVIRONMENT:
 *      Preemptable, Interrupts enabled on input.
 *      Runs on Pinned Kernel Stack for SVCs.
 *      An AIX Signal will not be delivered until this code is through
 *        running. This serializes `u.u_save.fpinfo' updates.
 *      This code is pageable.
 *
 * NOTES:
 *
 *   INPUT: 
 *
 *   OUTPUT:
 */

int
_fp_trapstate_ker( int flag )
{
  
  int    rc;                      /* return value             */
  int work;			/* working copy of fpinfo */
  

  /* First, figure out what the current state is,
   * in terms of the contents of fpinfo and what
   * platform is running.
   */

  work = u.u_save.fpinfo & (FP_IMP_INT | FP_SYNC_TASK);
  
  /* for PowerPC, all combinations of the
   * fpinfo bits 0x8 and 0x1 are valid.
   */
  
  if (__power_pc())
      {
      switch (work)
	  {
	case PPC_OFF:
	  rc = FP_TRAPSTATE_OFF;
	  break;
	case PPC_IMP:
	  rc = FP_TRAPSTATE_IMP;
	  break;
	case PPC_IMP_REC:
	  rc = FP_TRAPSTATE_IMP_REC;
	  break;
	case PPC_PRECISE:
	  rc = FP_TRAPSTATE_PRECISE;
	  break;
	default:
	  return FP_TRAPSTATE_ERROR;
	  }
      }
  else
    /* Rios, RSC or Rios-2 */
      {
      switch (_system_configuration.implementation)
	  {
	  
	  /* For Rios-1 and RSC, if the SYNC_TASK bit is
	   * on the process is in precise trap mode, else
	   * no traps can be generated.
	   */
	  
	case POWER_RS1:
	case POWER_RSC:
	  if (work & FP_SYNC_TASK)
	    rc = FP_TRAPSTATE_PRECISE;
	  else
	    rc = FP_TRAPSTATE_OFF;
	  break;
	  
	  /* For Rios-2 there is a hierarchy for the
	   * bits.  If FP_SYNC_TASK is on, the process
	   * is is precise trap mode.  Otherwise,
	   * if the FP_IMP_INT bit is on the process is
	   * in imprecise mode, otherwise in no traps mode.
	   */
	  
	case POWER_RS2:
	  if (work & FP_SYNC_TASK)
	    rc = FP_TRAPSTATE_PRECISE;
	  else
	    if (work & FP_IMP_INT)
	      rc = FP_TRAPSTATE_IMP;
	    else
	      rc = FP_TRAPSTATE_OFF;
	  break;
	  
	default:
	  return FP_TRAPSTATE_ERROR;
	  }	  
      }

  /* take care of query case first; it doesn't
   * have to disown the FP unit
   */

  if (flag == FP_TRAPSTATE_QUERY)
      {
      return rc;
      }

  /*
   * Make sure this process does not own the fp unit.
   * This will force the SVC handler back-end to 
   * reset the FE, IE and FP bits for the task.
   */
  
  disown_fp( curthread->t_tid );
  
  if (flag != FP_TRAPSTATE_FASTMODE)
      {
      switch(flag)
	  {
	case FP_TRAPSTATE_PRECISE:
	  if (__power_pc())
	      {
	      u.u_save.fpinfo |= PPC_PRECISE;
	      }
	  else
	      {
	      u.u_save.fpinfo &= ~FP_IMP_INT;
	      u.u_save.fpinfo |= FP_SYNC_TASK;
	      }
	  return rc;

	case FP_TRAPSTATE_IMP:
	  if (__power_pc())
	      {
	      u.u_save.fpinfo &= ~PPC_PRECISE;
	      u.u_save.fpinfo |= PPC_IMP;
	      }
	  else
	      {
	      if (__power_rs2())
		  {
		  u.u_save.fpinfo &= ~FP_SYNC_TASK;
		  u.u_save.fpinfo |= FP_IMP_INT;
		  }
	      else
		  {
		  return FP_TRAPSTATE_UNIMPL;
		  }
	      }
	  return rc;
	  
	case FP_TRAPSTATE_IMP_REC:
	  if (__power_pc())
	      {
	      u.u_save.fpinfo &= ~PPC_PRECISE;
	      u.u_save.fpinfo |= PPC_IMP_REC;
	      }
	  else
	    return FP_TRAPSTATE_UNIMPL;
	  return rc;
	  
	case FP_TRAPSTATE_OFF:
	  u.u_save.fpinfo &= ~PPC_PRECISE;
	  return rc;

	default:
	  return FP_TRAPSTATE_ERROR;
	  }
      }
  else
      {
      /* FASTMODE */
	  switch (_system_configuration.implementation)
	      {
	      /* Note:  future implementations should have
	       *  appropriate case clauses added.
	       */

	    case POWER_601:
	      u.u_save.fpinfo |= PPC_PRECISE;
	      return FP_TRAPSTATE_PRECISE;

	    case POWER_RS1:
	    case POWER_RSC:
	      u.u_save.fpinfo &= ~FP_IMP_INT;
	      u.u_save.fpinfo |= FP_SYNC_TASK;
	      return FP_TRAPSTATE_PRECISE;
	      
	    case POWER_RS2:
	      u.u_save.fpinfo &= ~FP_SYNC_TASK;
	      u.u_save.fpinfo |= FP_IMP_INT;
	      return FP_TRAPSTATE_IMP;
	      
	    default:
	      /* A PowerPC platform we don't know about.
	       * The default is precise mode.
	       */
	      u.u_save.fpinfo |= PPC_PRECISE;
	      return FP_TRAPSTATE_PRECISE;
	      }
	  }
}  
