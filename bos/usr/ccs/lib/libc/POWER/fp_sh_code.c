static char sccsid[] = "@(#)68	1.4  src/bos/usr/ccs/lib/libc/POWER/fp_sh_code.c, libccnv, bos411, 9428A410j 5/13/93 08:55:11";
/*
 * COMPONENT_NAME: (LIBCCNV) libc floating point code
 *
 * FUNCTIONS:	fp_sh_trap_info(), fp_sh_set_stat(), fp_trap_info()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <fpxcp.h>
#include "fpxcp_local.h"
#include <fptrap.h>
#include <signal.h>
#include <sys/systemcfg.h>
#include <sys/mstsave.h>

#define HARDWARE_INVALID_TRAPS \
		(FP_INV_SNAN | \
		 FP_INV_ISI  | \
		 FP_INV_IDI  | \
		 FP_INV_ZDZ  | \
		 FP_INV_IMZ  | \
		 FP_INV_CVI  | \
		 FP_INV_SQRT | \
		 FP_INV_VXSOFT  | \
		 FP_INV_CMP)

#define SOFTWARE_INVALID_TRAPS \
		(FP_INV_SQRT | \
		 FP_INV_VXSOFT  | \
		 FP_INV_CVI)

#define FP_INV_DETAIL ( 	  \
	FP_INV_SNAN		| \
	FP_INV_ISI		| \
	FP_INV_IDI		| \
	FP_INV_ZDZ		| \
	FP_INV_IMZ		| \
	FP_INV_CMP		| \
	FP_INV_SQRT		| \
	FP_INV_CVI		| \
	FP_INV_SQRT_SOFT	| \
	FP_INV_VXSOFT_SOFT      | \
	FP_INV_CVI_SOFT 	)


#define FP_FEX 0x40000000

/* The following macro extracts just the
 * bits relative to the trapping mode from
 * fpinfo
 */
#define FPINFO_MODE (state->fpinfo & 0x9)

static fpflag_t traps[] =  {
		FP_OVERFLOW    | (fpflag_t) TRP_OVERFLOW,
		FP_UNDERFLOW   | (fpflag_t) TRP_UNDERFLOW,
		FP_DIV_BY_ZERO | (fpflag_t) TRP_DIV_BY_ZERO,
		FP_INEXACT     | (fpflag_t) TRP_INEXACT
  };

/*
 * NAME: fp_sh_info
 *                                                                    
 * FUNCTION:  
 *
 * From within a SIGFPE trap handler, obtain information about the 
 * process which raised the trap.
 *
 * RETURNS: nothing (void)
 */

void 
fp_sh_info(struct sigcontext *scp, 
	     struct fp_sh_info *fcp,
	     size_t struct_size
	     )
  {
  struct mstsave * state = &scp->sc_jmpbuf.jmp_context;
  fpflag_t user_trap = 0x0;	/* fcp->trap work area */
  fpstat_t user_fpscr = 0x0;	/* fcp->fpscr work area */
  
  int i;

  /*
   * construct "user view" of the fpscr
   *
   * first work with FPSCRX ... get it, move "logical software" to
   * "logical harware" bits for CVI/SQRT, and then erase all non-sticky
   * bits (including "logical software" CVI and SQRT).
   */

  user_fpscr = (fpstat_t) state->fpscrx;
  user_fpscr |= (user_fpscr & FP_INV_SQRT_SOFT) >> SQRT_OFFSET;
  user_fpscr |= (user_fpscr & FP_INV_CVI_SOFT) >> CVI_OFFSET;
  user_fpscr |= (user_fpscr & FP_INV_VXSOFT_SOFT) >> VXSOFT_OFFSET;
  user_fpscr &= FP_ALL_XCP;
  
  /* now OR-in the hardware FPSCR. */
  
  fcp->fpscr = (fpstat_t) state->fpscr | user_fpscr;

  /* 
   * start construction of the TRAP FLAG.  A sticky bit is true
   * in this IFF it is true in the hardware (or logical hardware)
   * fpscr, and the corresponding enable bit is true.
   *
   * This is tested in two passes: one for bits which are valid in
   * hardware; one for bits valid only in the software copy
   *
   * the invalid op is a special case, because there are many
   * invalid op sticky bits and only 1 enable bit.  Moreover,
   * we know that the specific invalid op sticky bits are good,
   * but have less confidence about the invalid op summary bit.
   * So, if invalid op is enabled we update mask with all invalid
   * detail bits, as well as forcing the invalid summary bit
   */

  if ((fptrap_t) state->fpscr & TRP_INVALID)
      {
      user_trap |= (((fpflag_t) state->fpscr) & HARDWARE_INVALID_TRAPS);
      user_trap |= ((fpflag_t) state->fpscrx) & SOFTWARE_INVALID_TRAPS;
      if (user_trap) 
	user_trap |= FP_INVALID;
      }
  
  /*
   * now take care or "normal" exceptions.  traps[] is a table
   * with an entry for each exception, and each entry consists
   * if the 'or' of the sticky bit and enable bit.  This is
   * used as a mask to 'and' test the state of the hardware
   * fpscr -- if true, that that exception bit should be 
   * included in the result.  Update mask with traps[i] 'and'
   * FP_ALL_XCP, to remove the enable bit.
   */

  for (i = 0; i < (sizeof(traps) / sizeof(fpflag_t)); i++)
      {
      if (((fpflag_t) state->fpscr & traps[i]) == traps[i])
	  {
	  user_trap |= (traps[i] & FP_ALL_XCP);
	  }
      }

  /* after the loop exits we've finished building user_trap, and
   * we can store it and return.
   */

  fcp->trap = user_trap;

  /* The rest is easier!  Next, update trapmode field.  
   * This is done by reading fpinfo in the signal context (state->fpinfo)
   */

  if (__power_pc())
      {
      switch (FPINFO_MODE)
	  {
	case PPC_OFF:
	  fcp->trap_mode = FP_TRAP_OFF;
	  break;
	case PPC_IMP:
	  fcp->trap_mode = FP_TRAP_IMP;
	  break;
	case PPC_IMP_REC:
	  fcp->trap_mode = FP_TRAP_IMP_REC;
	  break;
	case PPC_PRECISE:
	  fcp->trap_mode = FP_TRAP_SYNC;
	  break;
	default:
	  fcp->trap_mode = FP_TRAP_ERROR;
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
	  if (FPINFO_MODE & FP_SYNC_TASK)
	    fcp->trap_mode = FP_TRAP_SYNC;
	  else
	    fcp->trap_mode = FP_TRAP_OFF;
	  break;
	  
	  /* For Rios-2 there is a hierarchy for the
	   * bits.  If FP_SYNC_TASK is on, the process
	   * is is precise trap mode.  Otherwise,
	   * if the FP_IMP_INT bit is on the process is
	   * in imprecise mode, otherwise in no traps mode.
	   */
	  
	case POWER_RS2:
	  if (FPINFO_MODE & FP_SYNC_TASK)
	    fcp->trap_mode = FP_TRAP_SYNC;
	  else
	    if (FPINFO_MODE & FP_IMP_INT)
	      fcp->trap_mode = FP_TRAP_IMP;
	    else
	      fcp->trap_mode = FP_TRAP_OFF;
	  break;
	  
	default:
	  fcp->trap_mode = FP_TRAP_ERROR;
	  }	  
      }

  /* Finally, update the flags field. */

  fcp->flags = state->fpinfo & FP_IAR_STAT;

  /* all fields above can always be updated, because they are
   * included in the structure from the first day of use.  If
   * new fields are added to the structure, the strct_size parm
   * must be used to determine if the routine has been given a
   * structure large enough to contain the fields.
   */

  }

/*
 * NAME: fp_sh_trap_info
 *                                                                    
 * FUNCTION:  
 *
 * From within a SIGFPE trap handler, obtain information about the 
 * process which raised the trap.
 *
 * NOTES:
 *
 * This is depricated interface, and is maintained to support existing
 * code.  It calls the new interface, fp_trap_info(), to do the
 * actual work.
 *
 * RETURNS: nothing (void)
 */

void 
fp_sh_trap_info(struct sigcontext *scp, struct fp_ctx *fcp)
  {
  struct mstsave * state = &scp->sc_jmpbuf.jmp_context;
  struct fp_sh_info trap_info; 
  
  fp_sh_info(scp, &trap_info, FP_SH_INFO_SIZE);
  fcp->fpscr = trap_info.fpscr;
  fcp->trap = trap_info.trap;
  }
     
/*
 * NAME: fp_sh_set_stat
 *                                                                    
 * FUNCTION:  
 *
 * From within a SIGFPE trap handler, update the FPSCR and
 * FPSCRX in the user's process (after return from the
 * signal handler.
 *
 * This requires careful handling to get things distributed
 * correctly between FPSCR and FPSCRX.
 *
 * fpscr is the "new" LOGICAL FPSCR given us by the user.
 * FPSCR will refer to the hardware register, and FPSCRX
 * refers to the software copy.
 *
 * here is how we distribute the bits:
 *
 * 1.  Exception Sticky Bits (other than FX) which are TRUE (1) in fpscr
 *     --> set those bits in FPSCRX
 *     --> leave those bits in FPSCR alone
 *  
 * 2.  FX bit, if true in fpscr
 *     --> set FX true in FPSCR
 *     --> set FX true in FPSCRX
 *
 * 3.  Exception Sticky Bits which are FALSE (0) in fpscr
 *     --> clear those bits in FPSCR
 *     --> clear those bits in FPSCRX
 *
 * 4.  All bits OTHER THAN Exception Sticky Bits in fpscr
 *     --> update FPSCR to match fpscr
 *
 * RETURNS: nothing (void)
 */

void 
fp_sh_set_stat(struct sigcontext *scp, fpstat_t fpscr)
  {
  struct mstsave * state = &scp->sc_jmpbuf.jmp_context;
  fpstat_t work;

  /* get just sticky bits from proposed fpscr */
  work = fpscr & (fpstat_t) FP_ALL_XCP;

  /* move certain bits to "logical software" position */
  work |= (work & FP_INV_SQRT) <<  SQRT_OFFSET;
  work |= (work & FP_INV_CVI)  << CVI_OFFSET;
  work |= (work & FP_INV_VXSOFT)  << VXSOFT_OFFSET;
  work &= ~(FP_INV_SQRT | FP_INV_CVI | FP_INV_VXSOFT);
  
  /* if any detail bits are on, make sure FP_INVALID is
   * also on; and if no detail bits are on, make sure
   * FP_INVALID is off.
   */
  work |= FP_INVALID;
  if ((work & FP_INV_DETAIL) == 0x0)
    work &= ~FP_INVALID;

  /* if any exceptions are on, turn on FX as well */
  if (work)
    work |= FP_ANY_XCP;
  
  /* finished construction of FPSCRX */
  state->fpscrx = work;
  
  state->fpscr = (work & FP_ANY_XCP) |
      (fpscr & ~(FP_ANY_XCP | FP_ALL_XCP | FP_FEX));
  }
