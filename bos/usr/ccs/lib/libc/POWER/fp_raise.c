static char sccsid[] = "@(#)67	1.5  src/bos/usr/ccs/lib/libc/POWER/fp_raise.c, libccnv, bos411, 9428A410j 7/8/94 13:19:55";
/*
 * COMPONENT_NAME: (LIBCCNV) C library floating-point code
 *
 * FUNCTIONS: fp_raise_xcp
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <fpxcp.h>
#include <fptrap.h>
#include "fpxcp_local.h"
#include <sys/signal.h>
#include <sys/systemcfg.h>

double __readflm(void);
double __setflm(double);

#define REQUIRE_WORKAROUND

/*
 * REQUIRE_WORKAROUND is defined due to a defect in the CSet++ compiler that
 * generates bad code for the __readflm() and __setflm() built in
 * functions.  The bug appears to occur when these are used in
 * conjunction with a tight loop that contains an "if" conditional in the
 * loop.  Moving the __readflm() and __setflm() to a function call
 * appears to avoid the bad code generation.
 */

#ifdef  REQUIRE_WORKAROUND

static double local_readflm(void);
static double local_setflm(double);
#define READFLM() local_readflm()
#define SETFLM(XX) local_setflm(XX)

static double
local_readflm(void)
  {
  return __readflm();
  }

static double 
local_setflm(double x)
  {
  return __setflm(x);
  }

#else  /* REQUIRE_WORKAROUND not defined */

#define READFLM() __readflm()
#define SETFLM(XX) __setflm(XX)

#endif  /* REQUIRE_WORKAROUND */

/*
 * NAME: fp_raise_xcp
 *                                                                    
 * FUNCTION:  
 *
 * Raise (force to occur) one or more floating point exceptions.
 *
 * NOTES:
 *
 * This will simulate the occurrence of each of the specified
 * floating point traps (in a predefined order), one by one, by
 * signalling the task's floating point trap handler.
 *
 * If the trap is disabled then the corresponding sticky 
 * bit is left set to indicate that this occurred.
 *
 * RETURNS:
 *
 * returns 0, normally, unless raise() is called in the
 * case of invalid convert or invalid square root, in 
 * which case the return value from raise() is our return
 * value as well.
 */

static struct Manual_traps {
	fpflag_t flag;		     /* Which bit in the FPSCR  */
	unsigned int hw_platforms;    /* mask of all implementations with this in HARDWARE */
	}
manual_traps[] = {
	{FP_INV_SQRT, (POWER_RS2) }, 
	{FP_INV_CVI,  (POWER_RS2 | POWER_601) },
	{FP_INV_VXSOFT, 0x0 }
  };

static fpflag_t traps[] = {
		FP_INV_SNAN,
		FP_INV_ISI,
		FP_INV_IDI,
		FP_INV_ZDZ,
		FP_INV_IMZ,
		FP_INV_CMP,
		FP_DIV_BY_ZERO,
		FP_UNDERFLOW,  
		FP_OVERFLOW,   
		FP_INEXACT
  };

int 
fp_raise_xcp( fpflag_t mask )
  {
  union {
        double x;
        struct { fpflag_t hi, lo; } i;
        } fpscr;
  int i, return_code;
  
  /*
   * raise all enabled exceptions; one by one.
   * priority to raise the exceptions is as follows:
   * 
   * - invalid
   *   dividebyzero
   *   underflow
   *   overflow
   *   inexact
   *
   * For all but invalid square root and invalid convert, 
   * can just set the hardware sticky bit and the trap will occur
   * if it is enabled.
   *
   * For invalid square root and invalid convert, must set
   * the software VXSQRT/VXCVI bits and, if the hardware invalid
   * op (VE) bit is set, and MSR(FE) bit is logically set, THEN, and Only 
   * then, do a `raise(SIGFPE)'.
   *
   * we can set the hardware or "logical hardware" sticky bits in
   * all cases, because the bit(s) actually causing a trap remains
   * computable.
   */
  
  return_code = 0;
  
  /*
   * The Invalid Convert, Invalid Square Root and
   * VXSOFT bits may or may not exists in the FPSCR,
   * depending on implementation.  If the bit exists
   * in the implementation, use move to fpscr to raise
   * the exception, since this gets FEX right.  Only
   * call raise() if the bit is not in the FPSCR and
   * the machine is in a state that can generate a trap.
   */

  for (i = 0; i < (sizeof(manual_traps) / sizeof(struct Manual_traps)); i++)
	 {
	 if ( mask & manual_traps[i].flag)
	     {
	     if ((_system_configuration.implementation & manual_traps[i].hw_platforms) == 0)
		 {
		 /* this exception is not implemented in hardware;
		  * process with raise() if necessary
		  */
		 OR_FPSCRX(manual_traps[i].flag | FP_ANY_XCP | FP_INVALID);
		 fpscr.x = READFLM();
		 fpscr.i.lo |= FP_ANY_XCP; /* update summary bit */
		 (void) SETFLM(fpscr.x);
		 if ((fpscr.i.lo & (fpflag_t) TRP_INVALID) &&
		     (fp_trapstate(FP_TRAPSTATE_QUERY) != FP_TRAPSTATE_OFF))
		     {
		     return_code |= raise(SIGFPE); /* will trap HERE */
		     }
		 }
	     else
		 {
		 /* this exception is implemented in hardware */
		 fpscr.x = READFLM();
		 fpscr.i.lo |= manual_traps[i].flag | FP_ANY_XCP;
		 (void) SETFLM(fpscr.x); /* can trap here */
		 }
	     }
	 }
  
  /*
   * now raise, one at a time, each of the hardware supported
   * traps. 
   */
  
  for (i = 0; i < (sizeof(traps) / sizeof(fpflag_t)); i++)
      {
      if (mask & traps[i])
	  {
	  fpscr.x = READFLM();
	  fpscr.i.lo |= (traps[i] | FP_ANY_XCP);
	  (void) SETFLM(fpscr.x);	/* can trap here */
	  }      
      }
       
  return return_code;
  }
     
     
