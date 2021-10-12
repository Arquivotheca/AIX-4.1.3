static char sccsid[] = "@(#)69	1.4  src/bos/usr/ccs/lib/libc/POWER/fp_trap.c, libccnv, bos411, 9428A410j 3/31/93 10:21:09";
/*
 * COMPONENT_NAME: (LIBCCNV) C library floating point code
 *
 * FUNCTIONS: fp_trap()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992,1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <fpxcp.h>
#include <fptrap.h>
#include "fpxcp_local.h"

double __readflm(void);
double __setflm(double);

/*
 * NAME: fp_trap
 *                                                                    
 * FUNCTION:  Modify state of machine to enable/disable
 *            IEEE trapping, and return old state.
 *                                                                    
 * NOTES:     Moves all FPSCR exception sticky bits to
 *            software copy of FPSCR to prevent an exception
 *            being raised immediately.  I.E., NOT level sensitive.
 *
 * RETURNS:	previous trapping state of the process
 *
 */

int fp_trap(int trap_status)
  {
  union  {  double x;
            fpflag_t i[2];
            } fpscr;		/* copy of FPSCR */
  fpflag_t soft_stat;
  int current_state, new_state;
  int trapstate_arg;
  
  /*
   * take care of QUERY option
   */

  if (trap_status == FP_TRAP_QUERY)
      {
      current_state = fp_trapstate(FP_TRAPSTATE_QUERY);
      switch (current_state)
	  {
	case FP_TRAPSTATE_OFF:
	  return FP_TRAP_OFF;
	  
	case FP_TRAPSTATE_IMP:
	  return FP_TRAP_IMP;
	  
	case FP_TRAPSTATE_IMP_REC:
	  return FP_TRAP_IMP_REC;
	  
	case FP_TRAPSTATE_PRECISE:
	  return FP_TRAP_SYNC;
	  
	default:
	  return FP_TRAP_ERROR;
	  }
      }

  switch (trap_status)
      {
    case FP_TRAP_OFF:
      trapstate_arg = FP_TRAPSTATE_OFF;
      break;
      
    case FP_TRAP_SYNC:
      trapstate_arg = FP_TRAPSTATE_PRECISE;
      break;
      
    case FP_TRAP_IMP:
      trapstate_arg = FP_TRAPSTATE_IMP;
      break;
      
    case FP_TRAP_IMP_REC:
      trapstate_arg = FP_TRAPSTATE_IMP_REC;
      break;
      
    case FP_TRAP_FASTMODE:
      trapstate_arg = FP_TRAPSTATE_FASTMODE;
      break;
      
    default:
      return FP_TRAP_ERROR;
      }    


  /* 
   * copy all sticky bits in hardware fpscr to software fpscr,
   * then clear all sticky bits in hardware
   */

  fpscr.x = __readflm();	/* read hardware FPSCR */
  soft_stat = READ_FPSCRX;	/* read software FPSCR */
  soft_stat |= fpscr.i[1] & ALL_HW_STIKYBITS; /* copy from hardware */
  fpscr.i[1] &= ~FP_ALL_XCP;	/* clear hardware */

  /*
   * move "logical hardware" FP_INV_SQRT and FP_INV_CVI
   * to soft copy and zero the "logical hardware" 
   * bits.
   */
  soft_stat |= (soft_stat & FP_INV_SQRT) << SQRT_OFFSET;
  soft_stat |= (soft_stat & FP_INV_CVI)  << CVI_OFFSET;
  soft_stat |= (soft_stat & FP_INV_VXSOFT) << VXSOFT_OFFSET;
  soft_stat &= ~(FP_INV_SQRT | FP_INV_CVI | FP_INV_VXSOFT);  

  /* write software and hardware fpscr */
  __setflm (fpscr.x);
  WRITE_FPSCRX(soft_stat);

  /*
   * modify trapping state to new mode.  fp_trapstate will also
   * return the previous state of the process, which is also
   * our return from this function.  Note that we have already
   * narrowed down all possible arguments to the point where
   * we know that the code points for fp_trap and fp_trapstate
   * are the same.  
   */

  new_state = fp_trapstate(trapstate_arg);
  
  switch(new_state)
      {
    case FP_TRAPSTATE_OFF:
      return FP_TRAP_OFF;
      
    case FP_TRAPSTATE_IMP:
      return FP_TRAP_IMP;
      
    case FP_TRAPSTATE_IMP_REC:
      return FP_TRAP_IMP_REC;
      
    case FP_TRAPSTATE_PRECISE:
      return FP_TRAP_SYNC;
      
    case FP_TRAPSTATE_UNIMPL:
      return FP_TRAP_UNIMPL;
      
    default:
      return FP_TRAP_ERROR;
      }
  }

