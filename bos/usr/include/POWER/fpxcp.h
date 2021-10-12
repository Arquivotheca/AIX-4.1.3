/* @(#)01	1.16  src/bos/usr/include/POWER/fpxcp.h, sysfp, bos411, 9428A410j 5/13/93 08:54:21 */
#ifndef _H_FPXCP
#define _H_FPXCP
/*
 *   COMPONENT_NAME: SYSFP
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <signal.h>		/* for struct sigcontext * */

/* a fpflag_t contains exception sticky bits from the Floating Point
 * Status and Control Register (FPSCR) in the same positions as
 * the FPSCR
 */
typedef unsigned int fpflag_t;

/* a fpstat_t contains a copy of the FPSCR -- all bits */
typedef unsigned long fpstat_t;

/* floating point context structure, for use with fp_sh_trap_info()
 * function.  NOTE:  New code should not use this; use fp_sh_info()
 * instead.
 */
typedef struct fp_ctx {
	fpstat_t fpscr;		/* fpscr from mst structure */
	fpflag_t trap;		/* type of exception which caused the trap */
	}fp_ctx_t;

/* floating point signal handler information structure, for use with
 * fp_sh_info().
 */
typedef struct fp_sh_info {
	fpstat_t fpscr;		/* fpscr from mst structure */
	fpflag_t trap;		/* type of exception which caused the trap */
	short trap_mode;	/* trap mode in effect when trap taken */
	char flags;		/* instruction type flags */
	char extra;		/* reserved */
	}fp_sh_info_t;

/* the following macro yields size of fp_sh_info structure */
#define FP_SH_INFO_SIZE		(sizeof (struct fp_sh_info))

/* the following define masks which are used to interpret the contents
 * of the 'flags' field in the fp_sh_info structure.  This field should
 * be considered to be an array of bits, and a mask used to access it.
 */

#define FP_IAR_STAT	0x02	/* status of IAR */

/* if the FP_IAR_STAT bit is 1, the IAR in the sigcontext
 * structure points to the address of the instruction which
 * caused the exception.  If this bit is 0, the IAR is
 * somewhere beyond the excepting instruction, or the exception
 * was caused by an operation which consists of multiple
 * instructions.
 */

#ifdef _NO_PROTO
     int	fp_raise_xcp();
     fpflag_t	fp_read_flag();
     void	fp_clr_flag();
     void	fp_set_flag();
     fpflag_t	fp_swap_flag();
     void	fp_sh_trap_info(); /* use fp_sh_info instead */
     void 	fp_sh_set_stat();
     void	fp_sh_info();
#else /* _NO_PROTO */		/* Use ANSI C prototyping */
     int	fp_raise_xcp(fpflag_t mask);
     fpflag_t	fp_read_flag( void );
     void	fp_clr_flag(fpflag_t mask);
     void	fp_set_flag(fpflag_t mask);
     fpflag_t	fp_swap_flag(fpflag_t newxcp);
     /* new code should use fp_sh_info() instead of fp_sh_trap_info() */
     void	fp_sh_trap_info(struct sigcontext *scp, struct fp_ctx *fcp);
     void 	fp_sh_set_stat(struct sigcontext *scp, fpstat_t fpscr);
     void	fp_sh_info(struct sigcontext *scp, 
                           struct fp_sh_info *fcp, 
                           size_t struct_size);
#endif

#ifdef _NO_PROTO
    int		fp_invalid_op();
    int		fp_divbyzero();
    int		fp_overflow();
    int		fp_underflow();
    int		fp_inexact();
    int		fp_any_xcp();
    int		fp_iop_snan();
    int		fp_iop_infsinf();
    int		fp_iop_infdinf();
    int		fp_iop_zrdzr();
    int		fp_iop_infmzr();
    int		fp_iop_invcmp();
    int		fp_iop_sqrt();
    int		fp_iop_convert();
    int		fp_iop_vxsoft();
#else	/* _NO_PROTO */
    int		fp_invalid_op( void );
    int		fp_divbyzero( void );
    int		fp_overflow( void );
    int		fp_underflow( void );
    int		fp_inexact( void );
    int		fp_any_xcp( void );
    int		fp_iop_snan( void );
    int		fp_iop_infsinf( void );
    int		fp_iop_infdinf( void );
    int		fp_iop_zrdzr( void );
    int		fp_iop_infmzr( void );
    int		fp_iop_invcmp( void );
    int		fp_iop_sqrt( void );
    int		fp_iop_convert( void );
    int		fp_iop_vxsoft( void );
#endif

/*  ******* REQUIRED SUMMARY BITS *********** */

#define FP_INVALID      ((fpflag_t) 0x20000000)
#define FP_OVERFLOW     ((fpflag_t) 0x10000000)
#define FP_UNDERFLOW    ((fpflag_t) 0x08000000)
#define FP_DIV_BY_ZERO  ((fpflag_t) 0x04000000)
#define FP_INEXACT      ((fpflag_t) 0x02000000)

/*  ******* MACHINE SPECIFIC SUMMARY BITS *********** */

#define FP_ANY_XCP      ((fpflag_t) 0x80000000)
#define FP_ALL_XCP      ((fpflag_t) 0xBFF80700)	/* Bits 1 and 2 cannot 
						 * be (re)set */

/* ******** MACHINE SPECIFIC INVALID EXCEPTION DETAIL BITS ***********/
/*
 *    FP_INV_SNAN       Signalling NaN
 *    FP_INV_ISI        Infinity - Infinity
 *    FP_INV_IDI        Infinity / Infinity
 *    FP_INV_ZDZ        0/0
 *    FP_INV_IMZ        Infinity * 0
 *    FP_INV_CMP        Unordered Compare
 *    FP_INV_SQRT       Square Root of negative number
 *    FP_INV_CVI        Conversion to integer error
 *    FP_INV_VXSOFT	Software request
 */

#define FP_INV_SNAN     ((fpflag_t) 0x01000000)
#define FP_INV_ISI      ((fpflag_t) 0x00800000)
#define FP_INV_IDI      ((fpflag_t) 0x00400000)
#define FP_INV_ZDZ      ((fpflag_t) 0x00200000)
#define FP_INV_IMZ      ((fpflag_t) 0x00100000)
#define FP_INV_CMP      ((fpflag_t) 0x00080000)
#define FP_INV_SQRT     ((fpflag_t) 0x00000200)
#define FP_INV_CVI      ((fpflag_t) 0x00000100)
#define FP_INV_VXSOFT	((fpflag_t) 0x00000400)

#endif /* _H_FPXCP */


