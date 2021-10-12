static char sccsid[] = "@(#)22	1.10  src/bos/usr/ccs/lib/libc/POWER/macros.c, libccnv, bos411, 9428A410j 7/30/93 09:03:59";
/*
 * COMPONENT_NAME: (LIBM) math library
 *
 * FUNCTIONS:	fp_invalid_op(), fp_overflow(), fp_underflow(), fp_divbyzero(),
 *		fp_any_xcp(), fp_inexact(), fp_any_enable(), fp_enable_all(),
 *		fp_enable(fptrap_t mask), fp_is_enabled(fptrap_t mask),
 *		fp_disable_all(), fp_disable(fptrap_t mask),
 *		fp_clr_flag(fpflag_t mask), fp_set_flag(fpflag_t mask),
 *		fp_read_flag(fpflag_t mask), fp_swap_flag(fpflag_t mask),
 *		fp_iop_snan(), fp_iop_infsinf(), fp_iop_infdinf(),
 *		fp_iop_zrdzr(), fp_iop_infmzr(), fp_iop_invcmp(), fp_read_rnd(),
 *		fp_iop_vxsoft(),
 *		fp_swap_rnd(int rnd)
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <float.h>
#include <fpxcp.h>
#include "fpxcp_local.h"
#include <fptrap.h>

double __readflm(void);
double __setflm(double);

#define FP_INV_DETAIL ( \
	FP_INV_SNAN	| \
	FP_INV_ISI	| \
	FP_INV_IDI	| \
	FP_INV_ZDZ	| \
	FP_INV_IMZ	| \
	FP_INV_CMP	| \
	FP_INV_SQRT	| \
	FP_INV_VXSOFT   | \
	FP_INV_CVI	)

/*
 * NAME: fp_invalid_op
 *                                                                    
 * FUNCTION: Test to see if the invalid operation exception flag is set in
 *		the floating point status register (FPSCR)
 *                                                                    
 * NOTES:  Exception sticky bit status is the OR of the hardware FPSCR
 *         and the software FPSCRX.
 *
 *         Using the (flags != 0) construct instead of something involving
 *         a branch makes the query-sticky-bit type functions much faster.
 *
 * RETURNS:	1 if invalid operation flag is set
 *		0 if invalid operation flag is not set
 *
 */

int 
fp_invalid_op(void)
{
	union {
		fpflag_t i[2];
		double	x;
	} fpscr;

	/* Read the current FPSCR */
	fpscr.x = __readflm();
	return( (int) (((fpscr.i[1] | READ_FPSCRX) & FP_INVALID) != 0) );
}


/*
 * NAME: fp_overflow
 *                                                                    
 * FUNCTION: Test to see if the overflow exception flag is set in
 *		the floating point status register (FPSCR)
 *                                                                    
 * NOTES:  Exception stick bit status is the OR of the hardware FPSCR
 *         and the software copy FPSCRX
 *
 * RETURNS:	1 if overflow flag is set
 *		0 if overflow flag is not set
 *
 */

int 
fp_overflow(void)
{
	union {
		fpflag_t i[2];
		double	x;
	} fpscr;

	/* Read the current FPSCR */
	fpscr.x = __readflm();

	return( (int) (((fpscr.i[1] | READ_FPSCRX) & FP_OVERFLOW) != 0) );
}


/*
 * NAME: fp_underflow
 *                                                                    
 * FUNCTION: Test to see if the underflow exception flag is set in
 *		the floating point status register (FPSCR)
 *                                                                    
 * NOTES:  Exception stick bit status is the OR of the hardware FPSCR
 *         and the software copy FPSCRX
 *
 * RETURNS:	1 if underflow flag is set
 *		0 if underflow flag is not set
 *
 */

int 
fp_underflow(void)
{
	union {
		fpflag_t i[2];
		double	x;
	} fpscr;

	/* Read the current FPSCR */
	fpscr.x = __readflm();

	return( (int) (((fpscr.i[1] | READ_FPSCRX) & FP_UNDERFLOW) != 0) );
}


/*
 * NAME: fp_divbyzero
 *                                                                    
 * FUNCTION: Test to see if the divide by zero exception flag is set in
 *		the floating point status register (FPSCR)
 *                                                                    
 * NOTES:  Exception stick bit status is the OR of the hardware FPSCR
 *         and the software copy FPSCRX
 *
 * RETURNS:	1 if divide by zero flag is set
 *		0 if divide by zero flag is not set
 *
 */

int 
fp_divbyzero(void)
{
	union {
		fpflag_t i[2];
		double	x;
	} fpscr;

	/* Read the current FPSCR */
	fpscr.x = __readflm();

	return( (int) (((fpscr.i[1] | READ_FPSCRX) & FP_DIV_BY_ZERO) != 0) );
}


/*
 * NAME: fp_inexact
 *                                                                    
 * FUNCTION: Test to see if the inexact exception flag is set in
 *		the floating point status register (FPSCR)
 *                                                                    
 * NOTES:  Exception stick bit status is the OR of the hardware FPSCR
 *         and the software copy FPSCRX
 *
 * RETURNS:	1 if inexact flag is set
 *		0 if inexact flag is not set
 *
 */

int 
fp_inexact(void)
{
	union {
		fpflag_t i[2];
		double	x;
	} fpscr;

	/* Read the current FPSCR */
	fpscr.x = __readflm();

	return( (int) (((fpscr.i[1] | READ_FPSCRX) & FP_INEXACT) != 0) );
}


/*
 * NAME: fp_any_xcp
 *                                                                    
 * FUNCTION: Test to see if the any exception flag is set in
 *		the floating point status register (FPSCR)
 *                                                                    
 * NOTES:  Exception stick bit status is the OR of the hardware FPSCR
 *         and the software copy FPSCRX
 *
 * RETURNS:	1 if any exception flag is set
 *		0 if no exceptions have occurred
 *
 */

int 
fp_any_xcp(void)
{
	union {
		fpflag_t i[2];
		double	x;
	} fpscr;

	/* Read the current FPSCR */
	fpscr.x = __readflm();

	return( (int) (((fpscr.i[1] | READ_FPSCRX) & FP_ANY_XCP) != 0) );
}


/*
 * NAME: fp_any_enable
 *                                                                    
 * FUNCTION: Test to see if any of the trap enable bits are set in
 *		the floating point status register (FPSCR)
 *                                                                    
 * NOTES:
 *
 * RETURNS:	1 if any trap enable bits are set
 * 		0 if no trap enable bits are set
 *
 */

int
fp_any_enable(void)
{
	union {
		fptrap_t i[2];
		double	x;
	} fpscr;

	/* Read the current FPSCR */
	fpscr.x = __readflm();

	return ( (fpscr.i[1] & (fptrap_t) FP_ENBL_SUMM) != 0);
}


/*
 * NAME: fp_enable_all
 *                                                                    
 * FUNCTION: Enables all the trap enable bits in
 *		the floating point status register (FPSCR)
 *                                                                    
 * NOTES:  Move all exception sticky bits from the hardware fpscr to
 *         the software copy before enabling any exceptions.
 *
 * RETURNS:	None
 *
 */

void 
fp_enable_all(void)
{
	union {
		fptrap_t i[2];
		double	x;
	} fpscr;
	fpflag_t soft_stat;

	/* Read the current FPSCR --> hard and soft */
	fpscr.x = __readflm();
	soft_stat = READ_FPSCRX;
	 
	/* move exception stick bits to software FPSCR.  Except
	 * keep FX, if set, in HW as well, since user may be
	 * branching on it
	 */
	soft_stat |= (fpflag_t) fpscr.i[1] & ALL_HW_STIKYBITS;
	fpscr.i[1] &= (fptrap_t) ~(FP_ALL_XCP & ~FP_ANY_XCP);

	/*
	 * move "logical hardware" FP_INV_SQRT and FP_INV_CVI
	 * to soft copy and zero the "logical hardware" 
	 * bits.
	 */
	soft_stat |= (soft_stat & FP_INV_SQRT) << SQRT_OFFSET;
	soft_stat |= (soft_stat & FP_INV_CVI)  << CVI_OFFSET;
	soft_stat |= (soft_stat & FP_INV_VXSOFT)  << VXSOFT_OFFSET;
	soft_stat &= ~(FP_INV_SQRT | FP_INV_CVI | FP_INV_VXSOFT);  

	/* Bitwise or in the appropriate bits */
	fpscr.i[1] |= (fptrap_t) FP_ENBL_SUMM;

	/* Write the new FPSCR --> hard and soft*/
	WRITE_FPSCRX(soft_stat);
	__setflm (fpscr.x);

}


/*
 * NAME: fp_enable
 *           
 * FUNCTION: Enables the trap enable bits specified by mask in
 *		the floating point status register (FPSCR)
 *                                                                    
 * NOTES:  Move all exception sticky bits from the hardware fpscr to
 *         the software copy before enabling any exceptions.
 *
 * RETURNS:	None
 *
 */

void 
fp_enable(fptrap_t mask)
{
	union {
		fptrap_t i[2];
		double	x;
	} fpscr;
	fpflag_t soft_stat;

	/* Make sure mask is for valid bits only */
	mask &= FP_ENBL_SUMM;

	if (mask)
	    {
	    /* Read the current FPSCR --> hard and soft */
	    fpscr.x = __readflm();
	    soft_stat = READ_FPSCRX;
	    
	    /* move exception stick bits to software FPSCR */
	    soft_stat |= fpscr.i[1] & ALL_HW_STIKYBITS;
	    fpscr.i[1] &= ~(FP_ALL_XCP & ~FP_ANY_XCP);

	    /*
	     * move "logical hardware" FP_INV_SQRT and FP_INV_CVI
	     * to soft copy and zero the "logical hardware" 
	     * bits.
	     */
	    soft_stat |= (soft_stat & FP_INV_SQRT) << SQRT_OFFSET;
	    soft_stat |= (soft_stat & FP_INV_CVI)  << CVI_OFFSET;
	    soft_stat |= (soft_stat & FP_INV_VXSOFT) << VXSOFT_OFFSET;
	    soft_stat &= ~(FP_INV_SQRT | FP_INV_CVI | FP_INV_VXSOFT);  

	    /* Bitwise or in the appropriate bits */
	    fpscr.i[1] |= mask;
	    
	    /* Write the new FPSCR --> hard and soft*/
	    WRITE_FPSCRX(soft_stat);
	    __setflm (fpscr.x);
	    }
}


/*
 * NAME: fp_is_enabled
 *                                                                    
 * FUNCTION: Test to see if the trap enable bit(s) is(are) set in
 *		the floating point status register (FPSCR)
 *                                                                    
 * NOTES:
 *
 * RETURNS:	1 if trap enable bit(s) is(are) set
 * 		0 if trap enable bit(s) is(are) not set
 *
 */

int
fp_is_enabled(fptrap_t mask)
{
	union {
		fptrap_t i[2];
		double	x;
	} fpscr;

	/* Read the current FPSCR */
	fpscr.x = __readflm();

	/* Make sure mask is for valid bits only */
	mask &= FP_ENBL_SUMM;


	return ( ((fpscr.i[1] & mask) == mask) != 0);
}


/*
 * NAME: fp_disable_all
 *                                                                    
 * FUNCTION: Clear all the trap enable bits for
 *		the floating point status register (FPSCR)
 *                                                                    
 * NOTES:
 *
 * RETURNS:	None
 *
 */

void 
fp_disable_all(void)
{
	union {
		fptrap_t i[2];
		double	x;
	} fpscr;

	/* Read the current FPSCR */
	fpscr.x = __readflm();

	/* Bitwise and in the complement of all the enable bits */
	fpscr.i[1] &= ~FP_ENBL_SUMM;

	/* Write the new FPSCR */
	__setflm (fpscr.x);

}


/*
 * NAME: fp_disable
 *                                                                    
 * FUNCTION: Clear the trap enable bits specified by mask for
 *		the floating point status register (FPSCR)
 *                                                                    
 * NOTES:
 *
 * RETURNS:	None
 *
 */

void 
fp_disable(fptrap_t mask)
{
	union {
		fptrap_t i[2];
		double	x;
	} fpscr;

	/* Read the current FPSCR */
	fpscr.x = __readflm();

	/* Make sure mask is for valid bits only */
	mask &= FP_ENBL_SUMM;

	/* Bitwise and in the complement of all the bits specified with mask */
	fpscr.i[1] &= ~mask;

	/* Write the new FPSCR */
	__setflm (fpscr.x);

}


/*
 * NAME: fp_clr_flag
 *                                                                    
 * FUNCTION: Clear the exception flags specified by mask in
 *		the floating point status register (FPSCR)
 *                                                                    
 * NOTES:
 *
 * RETURNS:	None
 *
 */

void 
fp_clr_flag(fpflag_t mask)
{
	union {
		fpflag_t i[2];
		double	x;
	} fpscr;
	fpflag_t soft_stat;
	
	/* Make sure mask is for valid bits only */
	mask &= FP_ALL_XCP;

	if (mask)
	    {
	    /* Read the current FPSCR --> hard & soft */
	    fpscr.x = __readflm();
	    soft_stat = READ_FPSCRX;
	    
	    /* modify mask so that it will clear both "copies" of
	     * the SQRT, VXSOFT, and CVI sticky bits
	     */

/***** !!!! WATCH OUT.  Must clear "logical software" position in
 *****                  in fpscrx, but not fpxcr.  Must clear "logical
 *****                  hardware" position in both.
 *****/

	    mask |= (mask & FP_INV_SQRT) << SQRT_OFFSET;
	    mask |= (mask & FP_INV_CVI) << CVI_OFFSET;
	    mask |= (mask & FP_INV_VXSOFT) << VXSOFT_OFFSET;
	    
	    /* Bitwise and in the complement of all the bits specified with mask */
	    /* Modify both the fpscr and the software status */
	    fpscr.i[1] &=  ~mask;
	    soft_stat  &=  ~mask;

	    /* if turned off last invalid op detail bit, turn off
	     * FP_INVALID as well 
	     */

	    if ((
		 (fpscr.i[1] & FP_INV_DETAIL) | 
		 (soft_stat & (FP_INV_DETAIL | FP_INV_CVI_SOFT | FP_INV_SQRT_SOFT))
		 ) == 0)
		{
		fpscr.i[1] &= ~FP_INVALID;
		soft_stat  &= ~FP_INVALID;
		}
	    

	    /* If only the exception summary bit is set, clear it also */
	    if (((fpscr.i[1] | soft_stat) & FP_ALL_XCP) == FP_ANY_XCP) 
		{
	        fpscr.i[1] &=  ~FP_ANY_XCP;
	        soft_stat &=  ~FP_ANY_XCP;
	        }
	    
	    /* Write the new FPSCR */
	    __setflm (fpscr.x);
	    WRITE_FPSCRX(soft_stat);
	    }
}


/*
 * NAME: fp_set_flag
 *                                                                    
 * FUNCTION: Set the exception flags specified by mask in
 *		the floating point status register (FPSCR)
 *                                                                    
 * NOTES:  Write in SOFTWARE fpscr only...emulates edge-triggered
 *         exceptions.  If user wants to take a trap, then
 *         fp_raise_xcp() should be used instead of this routine.
 *
 *         Update both the software & hardware FX bit.
 *
 * RETURNS:	None
 *
 */

void 
fp_set_flag(fpflag_t mask)
{
	union {
		fpflag_t i[2];
		double	x;
	} fpscr;
	fpflag_t soft_stat;
	
	/* Make sure mask is for valid bits only */
	mask &= FP_ALL_XCP;

	if (mask)
	    {
	    /* Set the exception summary bit */
	    mask |= FP_ANY_XCP;

	    /* if any invalid op detail bits are set,
	     * be sure to set invalid op summary as well
	     */
	    if (mask & FP_INV_DETAIL)
		{
		mask |= FP_INVALID;
		/* take care of SQRT/CVI as well.  This can be done
		 * inside of this branch since we will get to here
		 * if there are any detail bits on.  We've paid for the
		 * branch anyway, and avoids doing the work when it's 
		 * definitely not necessary.
		 */
		mask |= (mask & FP_INV_SQRT) << SQRT_OFFSET;
		mask |= (mask & FP_INV_CVI) << CVI_OFFSET;
		mask |= (mask & FP_INV_VXSOFT) << VXSOFT_OFFSET;
		mask &= ~(FP_INV_SQRT | FP_INV_CVI | FP_INV_VXSOFT);
		}
	    
	    /* 
	     * mask is used with a bitwise or to update
	     * the software fpscr
	     */
	    OR_FPSCRX(mask);
	    /* update FPSCR(FX) bit */
	    fpscr.x = __readflm();
	    fpscr.i[1] |= (fpflag_t) FP_ANY_XCP;
	    __setflm(fpscr.x);
	    }
}	

/*
 * NAME: fp_read_flag
 *                                                                    
 * FUNCTION: Read the floating point status register (FPSCR)
 *                                                                    
 * NOTES:
 *
 * RETURNS:	The FPSCR
 *
 */

fpflag_t 
fp_read_flag(void)
{
	union {
		fpflag_t i[2];
		double	x;
	} fpscr;
	fpflag_t all_flags;

	/* Read the current FPSCR */
	fpscr.x = __readflm();

	/* Or in bits from software status word */
	all_flags = (fpscr.i[1] & ALL_HW_STIKYBITS) | READ_FPSCRX;
	
	/* take care of SQRT/CVI as well */
	all_flags |= (all_flags & FP_INV_SQRT_SOFT) >> SQRT_OFFSET;
	all_flags |= (all_flags & FP_INV_CVI_SOFT) >> CVI_OFFSET;
	all_flags |= (all_flags & FP_INV_VXSOFT_SOFT) >> VXSOFT_OFFSET;
	all_flags &= ~(FP_INV_SQRT_SOFT | FP_INV_CVI_SOFT | FP_INV_VXSOFT_SOFT);

	return (all_flags); 
}


/*
 * NAME: fp_swap_flag
 *                                                                    
 * FUNCTION: Set the floating point status register (FPSCR)
 *		to the value specified by mask
 *                                                                    
 * NOTES:  Set only the software copy to emulate edge triggered
 *         traps.
 *
 * RETURNS:	Original FPSCR
 *
 */

fpflag_t 
fp_swap_flag(fpflag_t mask)
{
	union {
		fpflag_t i[2];
		double	x;
	} fpscr;
	fpflag_t soft_stat;

	/* Read the current FPSCR */
	fpscr.x = __readflm();

	/* construct 'old' status.  First read the software copy FPSCRX,
	 * and move the "software" copy of CVI and SQRT to
	 * the "hardware" position.  Then include the hardware FPSCR,
	 * taking care to mask out the SQRT and CVI bits ... who knows
	 * what's in those in the HW.
	 */
	soft_stat = READ_FPSCRX;
	soft_stat |= (soft_stat & FP_INV_SQRT_SOFT) >> SQRT_OFFSET;
	soft_stat |= (soft_stat & FP_INV_CVI_SOFT) >> CVI_OFFSET;
	soft_stat |= (soft_stat & FP_INV_VXSOFT_SOFT) >> VXSOFT_OFFSET;
	soft_stat &= ~(FP_INV_SQRT_SOFT | FP_INV_CVI_SOFT | FP_INV_VXSOFT_SOFT);
	soft_stat |= (fpscr.i[1] & ALL_HW_STIKYBITS);

	/* clear the hardware FPSCR.  We're going to write all
	 * new bits to software, with the possible exception of
	 * the exception summary bit, which if true will also be
	 * written to HW.
	 */
	fpscr.i[1] &= ~FP_ALL_XCP;

	/* mask contains the new sticky bits.  We will do some work
	 * to make sure it's valid.  First, make sure it contains
	 * only sticky bits.
	 */
	mask &= FP_ALL_XCP;

	/* Make sure that if any exception bits are on, the
	 * exception summary bit is also on.  This bit is set in 
	 * both HW and SW.
	 */
	if (mask)
	    {
	    fpscr.i[1] |= FP_ANY_XCP;
	    mask |= FP_ANY_XCP;
	    }

	/* Make sure that if any invalid op detail bits are on then
	 * the invalid op summary bit is on as well.  Also, take this
	 * occasion to move CVI and SQRT detail bits to "logical software"
	 * position  -- this only needs to be done if we take this branch..
	 */
	if (mask & FP_INV_DETAIL)
	    {
	    mask |= FP_INVALID;	/* turn on inv op summary bit */
	    /* now move the SQRT and CVI detail bits to the "software"
	     * position in the mask
	     */
	    mask |= (mask & FP_INV_SQRT) << SQRT_OFFSET;
	    mask |= (mask & FP_INV_CVI) << CVI_OFFSET;
	    mask |= (mask & FP_INV_VXSOFT) << VXSOFT_OFFSET;
	    mask &= ~(FP_INV_SQRT | FP_INV_CVI | FP_INV_VXSOFT);	    
	    }
	
	/* Write the modified hardware & software FPSCR */
	__setflm (fpscr.x);
	WRITE_FPSCRX(mask);

	return (soft_stat); 
}


/*
 * NAME: fp_iop_snan
 *                                                                    
 * FUNCTION: Test to see if the invalid opertation exception flag is set in
 *		the floating point status register (FPSCR)
 *	     This case is testing for an exception due to a signalling NaN
 *                                                                    
 * NOTES:
 *
 * RETURNS:	1 if invalid operation flag is set
 *		0 if invalid operation flag is not set
 *
 */

int 
fp_iop_snan(void)
{
	union {
		fpflag_t i[2];
		double	x;
	} fpscr;

	/* Read the current FPSCR */
	fpscr.x = __readflm();

	return (((fpscr.i[1] | READ_FPSCRX) & FP_INV_SNAN) != 0);
}


/*
 * NAME: fp_iop_infsinf
 *                                                                    
 * FUNCTION: Test to see if the invalid opertation exception flag is set in
 *		the floating point status register (FPSCR)
 *	     This case is testing for an exception due to INF - INF
 *                                                                    
 * NOTES:
 *
 * RETURNS:	1 if invalid operation flag is set
 *		0 if invalid operation flag is not set
 *
 */

int 
fp_iop_infsinf(void)
{
	union {
		fpflag_t i[2];
		double	x;
	} fpscr;

	/* Read the current FPSCR */
	fpscr.x = __readflm();

	return (((fpscr.i[1] | READ_FPSCRX) & FP_INV_ISI) != 0);
}


/*
 * NAME: fp_iop_infdinf
 *                                                                    
 * FUNCTION: Test to see if the invalid opertation exception flag is set in
 *		the floating point status register (FPSCR)
 *	     This case is testing for an exception due to INF / INF
 *                                                                    
 * NOTES:
 *
 * RETURNS:	1 if invalid operation flag is set
 *		0 if invalid operation flag is not set
 *
 */

int 
fp_iop_infdinf(void)
{
	union {
		fpflag_t i[2];
		double	x;
	} fpscr;

	/* Read the current FPSCR */
	fpscr.x = __readflm();

	return (((fpscr.i[1] | READ_FPSCRX) & FP_INV_IDI) != 0);
}


/*
 * NAME: fp_iop_zrdzr
 *                                                                    
 * FUNCTION: Test to see if the invalid opertation exception flag is set in
 *		the floating point status register (FPSCR)
 *	     This case is testing for an exception due to 0.0 / 0.0
 *                                                                    
 * NOTES:
 *
 * RETURNS:	1 if invalid operation flag is set
 *		0 if invalid operation flag is not set
 *
 */

int 
fp_iop_zrdzr(void)
{
	union {
		fpflag_t i[2];
		double	x;
	} fpscr;

	/* Read the current FPSCR */
	fpscr.x = __readflm();

	return (((fpscr.i[1] | READ_FPSCRX) & FP_INV_ZDZ) != 0);
}


/*
 * NAME: fp_iop_infmzr
 *                                                                    
 * FUNCTION: Test to see if the invalid opertation exception flag is set in
 *		the floating point status register (FPSCR)
 *	     This case is testing for an exception due to INF X 0.0
 *                                                                    
 * NOTES:
 *
 * RETURNS:	1 if invalid operation flag is set
 *		0 if invalid operation flag is not set
 *
 */

int 
fp_iop_infmzr(void)
{
	union {
		fpflag_t i[2];
		double	x;
	} fpscr;

	/* Read the current FPSCR */
	fpscr.x = __readflm();

	return (((fpscr.i[1] | READ_FPSCRX) & FP_INV_IMZ) != 0);
}


/*
 * NAME: fp_iop_invcmp
 *                                                                    
 * FUNCTION: Test to see if the invalid opertation exception flag is set in
 *		the floating point status register (FPSCR)
 *	     This case is testing for an exception due to a compare involving
 *		a NaN.
 *                                                                    
 * NOTES:
 *
 * RETURNS:	1 if invalid operation flag is set
 *		0 if invalid operation flag is not set
 *
 */

int 
fp_iop_invcmp(void)
{
	union {
		fpflag_t i[2];
		double	x;
	} fpscr;

	/* Read the current FPSCR */
	fpscr.x = __readflm();

	return (((fpscr.i[1] | READ_FPSCRX) & FP_INV_CMP) != 0);
}


/*
 * NAME: fp_iop_sqrt
 *                                                                    
 * FUNCTION: Test to see if the invalid opertation exception flag is set in
 *		the floating point status register (FPSCR)
 *	     This case is testing for an exception due to a sqrt for a 
 *		negative number.
 *                                                                    
 * NOTES:
 *
 * RETURNS:	1 if invalid operation flag is set
 *		0 if invalid operation flag is not set
 *
 */

int 
fp_iop_sqrt(void)
{
	union {
		fpflag_t i[2];
		double	x;
	} fpscr;

	/* Read the current FPSCR */
	fpscr.x = __readflm();
	
	return ((fpscr.i[1] & FP_INV_SQRT) |
		(READ_FPSCRX & (FP_INV_SQRT | FP_INV_SQRT_SOFT)))
		!= 0 ;	

}


/*
 * NAME: fp_iop_convert
 *                                                                    
 * FUNCTION: Test to see if the invalid opertation exception flag is set in
 *		the floating point status register (FPSCR)
 *	     This case is testing for an exception due to a conversion
 *		error from a double to an integer
 *                                                                    
 * NOTES:
 *
 * RETURNS:	1 if invalid operation flag is set
 *		0 if invalid operation flag is not set
 *
 */

int 
fp_iop_convert(void)
{
	union {
		fpflag_t i[2];
		double	x;
	} fpscr;

	/* Read the current FPSCR */
	fpscr.x = __readflm();
	
	return ((fpscr.i[1] & FP_INV_CVI) |
		(READ_FPSCRX & (FP_INV_CVI | FP_INV_CVI_SOFT)))
		!= 0 ;	

}

/*
 * NAME: fp_iop_vxsoft
 *                                                                    
 * FUNCTION: Test to see if the invalid opertation exception flag is set in
 *		the floating point status register (FPSCR)
 *	     This case is testing the "software" (VXSOFT) exception
 *                                                                    
 * NOTES:
 *
 * RETURNS:	1 if invalid operation flag is set
 *		0 if invalid operation flag is not set
 *
 */

int 
fp_iop_vxsoft(void)
{
	union {
		fpflag_t i[2];
		double	x;
	} fpscr;

	/* Read the current FPSCR */
	fpscr.x = __readflm();
	
	return ((fpscr.i[1] & FP_INV_VXSOFT) |
		(READ_FPSCRX & (FP_INV_VXSOFT | FP_INV_VXSOFT_SOFT)))
		!= 0 ;	

}



/*
 * NAME: fp_read_rnd
 *                                                                    
 * FUNCTION: Read the floating point rounding mode from the FPSCR
 *                                                                    
 * NOTES:
 *
 * RETURNS:	The ANSI rounding mode
 *
 */

fprnd_t 
fp_read_rnd(void)
{
	fprnd_t	rmode;
	union {
		fpflag_t i[2];
		double	x;
	} fpscr;

	/* Read the current FPSCR */
	fpscr.x = __readflm();

	rmode = (fprnd_t) (fpscr.i[1] & 0x3); 

	/* For RIOS, Round nearest is 0, round zero is 1, which is
	 * opposite of ANSI
	 */
	if (rmode == (fprnd_t) FP_RND_RZ)
		rmode = (fprnd_t) FP_RND_RN;
	else if (rmode == (fprnd_t) FP_RND_RN)
		rmode = (fprnd_t) FP_RND_RZ;

	return (rmode);
}


/*
 * NAME: fp_swap_rnd
 *                                                                    
 * FUNCTION: Set the floating point rounding mode in the FPSCR
 *                                                                    
 * NOTES:
 *
 * RETURNS:	The old ANSI rounding mode
 *
 */

fprnd_t 
fp_swap_rnd(fprnd_t rnd)
{
	fprnd_t	rmode;
	union {
		fpflag_t i[2];
		double	x;
	} fpscr;

	/* For RIOS, Round nearest is 0, round zero is 1, which is
	 * opposite of ANSI
	 */
	if (rnd == (fprnd_t) FP_RND_RZ)
		rnd = (fprnd_t) FP_RND_RN;
	else if (rnd == (fprnd_t) FP_RND_RN)
		rnd = (fprnd_t) FP_RND_RZ;

	fpscr.x = __setrnd (rnd);

	rmode = (fprnd_t) (fpscr.i[1] & 0x3); 

	/* For RIOS, Round nearest is 0, round zero is 1, which is
	 * opposite of ANSI
	 */
	if (rmode == (fprnd_t) FP_RND_RZ)
		rmode = (fprnd_t) FP_RND_RN;
	else if (rmode == (fprnd_t) FP_RND_RN)
		rmode = (fprnd_t) FP_RND_RZ;

	return (rmode);
}
  
