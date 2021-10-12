/* @(#)21	1.6  src/bos/usr/include/POWER/fptrap.h, libccnv, bos411, 9428A410j 3/31/93 10:12:54 */
#ifndef _H_FPTRAP
#define _H_FPTRAP

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

/* a fptrap_t contains exception enable bits from the Floating Point
 * Status and Control Register (FPSCR) in the same positions as
 * the FPSCR
 */
typedef unsigned int fptrap_t;

#ifdef _NO_PROTO
	int	fp_any_enable();
	void	fp_enable_all();
	void	fp_enable();
	int	fp_is_enabled();
	void	fp_disable_all();
	void	fp_disable();
	int	fp_trap();
	void	fp_flush_imprecise();
	int	fp_trapstate();
#else /* _NO_PROTO */			/* Use ANSI C required prototyping */
	int	fp_any_enable(void);
	void	fp_enable_all(void);
	void	fp_enable(fptrap_t mask);
	int	fp_is_enabled(fptrap_t mask);
	void	fp_disable_all(void);
	void	fp_disable(fptrap_t mask);
	int	fp_trap(int flag);
	void	fp_flush_imprecise(void);
	int	fp_trapstate(int);
#endif

/* ******** ENABLE EXCEPTION DETAIL BITS ***********/

#define	FP_ENBL_SUMM	((fptrap_t) 0x000000F8)

#define	TRP_INVALID	((fptrap_t) 0x00000080)
#define	TRP_OVERFLOW	((fptrap_t) 0x00000040)
#define	TRP_UNDERFLOW	((fptrap_t) 0x00000020)
#define	TRP_DIV_BY_ZERO	((fptrap_t) 0x00000010)
#define	TRP_INEXACT	((fptrap_t) 0x00000008)

/* argument and return values for fp_trap */

#define FP_TRAP_SYNC     	1	/* precise trapping on */
#define FP_TRAP_OFF      	0	/* trapping off        */
#define FP_TRAP_QUERY    	2	/* query trapping mode */
#define FP_TRAP_IMP     	3	/* non-recoverable imprecise trapping on */
#define FP_TRAP_IMP_REC    	4	/* recoverable imprecise trapping on */
#define FP_TRAP_FASTMODE	128	/* select fastest available mode */
#define FP_TRAP_ERROR          -1	/* error condition */
#define FP_TRAP_UNIMPL         -2	/* requested mode not available */

/* argument and return values for fp_trapstate */

#define FP_TRAPSTATE_OFF	0	/* no trapping */
#define FP_TRAPSTATE_IMP	1	/* non-recoverable imprecise traps mode */
#define FP_TRAPSTATE_IMP_REC	2	/* recoverable imprecise traps mode */
#define FP_TRAPSTATE_PRECISE	3	/* precise traps mode */
#define FP_TRAPSTATE_QUERY	64	/* query current state */
#define FP_TRAPSTATE_FASTMODE	128	/* set fastest mode which will generate traps */
#define FP_TRAPSTATE_ERROR	-1	/* error condition */
#define FP_TRAPSTATE_UNIMPL	-2	/* requested state not implemented */

/* argument and return values for fp_cpusync */

#include <sys/fp_cpusync.h>

#endif /* _H_FPTRAP */



