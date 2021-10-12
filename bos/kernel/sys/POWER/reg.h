/* @(#)57       1.12.1.8  src/bos/kernel/sys/POWER/reg.h, sysproc, bos411, 9434B411a 8/22/94 16:51:58 */
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: FPR_loc
 *		GPR_loc
 *		ISWALIGN
 *		ISWRITEREG
 *		SPR_loc
 *		SPR_size
 *		VALID_ADDR
 *		VALID_FPR
 *		VALID_GPR
 *		VALID_SPR
 *		
 *   ORIGINS: 3, 27, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */


#ifndef	_H_REG
#define	_H_REG

/* macro defines for general purpose registers, all 32-bit, sequential no's */
#define	GPR0	0
#define	GPR1	1
#define	GPR2	2
#define	GPR3	3
#define	GPR4	4
#define	GPR5	5
#define	GPR6	6
#define	GPR7	7
#define	GPR8	8
#define	GPR9	9
#define	GPR10	10
#define	GPR11	11
#define GPR12	12
#define GPR13	13
#define GPR14	14
#define GPR15	15
#define GPR16	16
#define GPR17	17
#define GPR18	18
#define GPR19	19
#define GPR20	20
#define GPR21	21
#define GPR22	22
#define GPR23	23
#define GPR24	24
#define GPR25	25
#define GPR26	26
#define GPR27	27
#define GPR28	28
#define GPR29	29
#define GPR30	30
#define GPR31	31

/* miscellaneous special purpose registers - 32-bit, sequential no's */
#define	IAR	128		/* instruction address register	*/
#define	MSR	129		/* machine state register	*/
#define	CR	130		/* condition register		*/
#define	LR	131		/* link register		*/
#define	CTR	132		/* count register		*/
#define	XER	133		/* fixed point exception	*/
#define	MQ	134		/* multiply/quotient register	*/
#define	TID	135		/* tid register			*/
#define	FPSCR	136		/* floating point status reg	*/
#define FPINFO	138		/* floating point info reg	*/
#define FPSCRX  148             /* floating point sreg ext.     */

/* macro defines for floating point registers - 64-bit, sequential no's */
#define	FPR0	256
#define	FPR1	257
#define	FPR2	258
#define	FPR3	259
#define	FPR4	260
#define	FPR5	261
#define	FPR6	262
#define	FPR7	263
#define	FPR8	264
#define	FPR9	265
#define	FPR10	266
#define	FPR11	267
#define FPR12	268
#define FPR13	269
#define FPR14	270
#define FPR15	271
#define FPR16	272
#define FPR17	273
#define FPR18	274
#define FPR19	275
#define FPR20	276
#define FPR21	277
#define FPR22	278
#define FPR23	279
#define FPR24	280
#define FPR25	281
#define FPR26	282
#define FPR27	283
#define FPR28	284
#define FPR29	285
#define FPR30	286
#define FPR31	287

/* miscellaneous mapping for software conventions */
#define STKP	GPR1
#define PCP     GPR1
#define TOC	GPR2
#define ARG1	GPR3
#define ARG2	GPR4
#define ARG3	GPR5
#define ARG4	GPR6

/* calling convention values */
#define	STACK_FLOOR	256

#ifdef _KERNEL
/*
 * NAME:  Macro definitions for machine dependent ptrace values
 */

/* macros to validate register values, returns 1 or 0 */
#define	VALID_GPR(x)	((x) >= GPR0 && (x) <= GPR31)
#define VALID_SPR(x)    ((x) >= IAR  && (x) <= FPSCR)
#define	VALID_FPR(x)	((x) >= FPR0 && (x) <= FPR31)

/* macros to convert passed register to actual register address */
#define	GPR_loc(mp, x)	\
	(char *)(&(mp->gpr[(int)(x) - GPR0]))
#define SPR_loc(mp, x)	\
	(char *)(&(mp->iar) + (int)(x) - IAR)
#define	FPR_loc(mp, x)	\
	(char *)(&(mp->fpr[(int)(x) - FPR0]))

/* macros to provide data transfer sizes */
#define	GPR_size	(sizeof(uthr0.ut_save.gpr[0]))
#define SPR_size(x)	(sizeof(uthr0.ut_save.iar))
#define	FPR_size	(sizeof(uthr0.ut_save.fpr[0]))

/* is this word aligned */
#define ISWALIGN(x)	((int)(x) & (sizeof(int) - 1) ? 0 : 1)

/* is this a valid area for a debugger to be accessing */
#define VALID_ADDR(x) \
	(( (((int)(x) >> SEGSHIFT) == PRIVSEG && SEGOFFSET(x) >= U_REGION_SIZE)\
	 || ((int)(x) >> SEGSHIFT) == KERNEXSEG ) ? 0 : 1 )

#define ISWRITEREG(x)   \
        (VALID_GPR(((x)-(int *)&((struct mstsave *)0)->gpr[0])) || \
         VALID_SPR(((x)-(int *)&((struct mstsave *)0)->iar)) )

#endif /* _KERNEL */

#ifdef _KERNSYS
/*
* Construct problem-program MSR, allowing the user to specify certain
* bits.
*
*      MSR bit  forced to           Signal handler can specify
*      -------  ---------           --------------------------
*      EE        1                          AL
*      FP        0
*      FE        0
*      IE        0
*      ME        1
*      IP        0
*      IR        1
*      DR        1
*      PR        1 for user thread
*/
#define SANITIZE_MSR(x) (x) = (MSR_EE | MSR_ME | MSR_IR | MSR_DR | MSR_PR) | \
			       ((x) & (MSR_AL))

#define MASK_FP(x) \
	((char)(((x) >> FP_SYNC_IMP_S) & (FP_IMP_INT | FP_SYNC_TASK)))

#define UPDATE_FPINFO(x)						\
	(__power_pc() ? 						\
		MASK_FP(x) : 						\
		MASK_FP(x) & FP_SYNC_TASK ? 				\
			FP_SYNC_TASK : 					\
			(char)( ( MASK_FP(x) & FP_IMP_INT ) && 		\
						( __power_rs2()) ))
#endif /* _KERNSYS */

#endif /* _H_REG */
