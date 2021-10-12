/* @(#)30	1.19.2.1  src/bos/kernel/sys/POWER/mstsave.h, sysproc, bos412, 9445C412a 10/25/94 11:43:47 */
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *   LEVEL 1,  5 Years Bull Confidential Information
 */


#ifndef _H_MSTSAVE
#define _H_MSTSAVE

/*
 *		Machine State Save Area -- R2 platform
 *
 *  Machine state save area. The fields for the segment registers,
 *  general purpose registers, and floating point registers are
 *  aligned to minimize the number of cache lines that contain them.
 *  Therefore this structure should be aligned on a cache line
 *  boundary.
 */

#include <sys/m_param.h>		/* for machine dependent defines*/

#define	NUM_KERNEL_BATS	3		/* number of BATS used in kernel mode */

struct mstsave	{

	struct mstsave	*prev;		/* previous save area		*/
	label_t		*kjmpbuf;	/* pointer to saved context	*/
	char		*stackfix;	/* stack fix pointer		*/
	char		intpri; 	/* interrupt priority		*/
	char		backt;		/* back-track flag		*/
	char		rsvd[2];	/* reserved			*/
	pid_t		curid;		/* copy of curid		*/

	int		excp_type;	/* exception type for debugger	*/
	ulong_t		iar;		/* instruction address register	*/
	ulong_t		msr;		/* machine state register	*/
	ulong_t		cr;		/* condition register		*/
	ulong_t		lr;		/* link register		*/
	ulong_t		ctr;		/* count register		*/
	ulong_t		xer;		/* fixed point exception	*/
	ulong_t		mq;		/* multiply/quotient register	*/
 	ulong_t		tid;		/* tid register			*/
	ulong_t		fpscr;		/* floating point status reg	*/
	char		fpeu;		/* floating point ever used	*/
	char            fpinfo;         /* floating point status flags  */

	char            pad[2];         /* res - pad to dword boundary  */
	ulong_t         except[5];	/* exception structure          */
	char            pad1[4];        /* old bus field                */

	ulong_t         o_iar;          /* old iar (for longjmp excpt)  */
        ulong_t         o_toc;          /* old toc (for longjmp excpt)  */
	ulong_t         o_arg1;         /* old arg1 (for longjmp excpt) */

	ulong_t		excbranch;	/* if not NULL, address to branch
					 * to on exception.  Used by
					 * assembler routines for low
					 * cost exception handling*/
	ulong_t         fpscrx;         /* software extension to fpscr  */
	ulong_t         o_vaddr;        /* Saved vaddr for vmexception  */

#ifdef _KERNSYS
	ulong_t		ioalloc;	/* ioallocation mask */
	struct {
		ulong_t batu;
		ulong_t batl;
	} dbats[NUM_KERNEL_BATS];	/* Save area for bats		*/
#else
	ulong_t		cachealign[7];	/* reserved			*/
#endif /* _KERNSYS */

	adspace_t	as;		/* segment registers            */
	ulong_t		gpr[NGPRS];	/* general purpose registers	*/
	double		fpr[NFPRS];	/* floating point registers	*/
};


/* 
 * Values for mstsave.fpeu
 */

#define FP_NEVER_USED	0		/* floating point never used	*/
#define FP_USED		1		/* floating point used	        */

/*
 * Values/fields of mstsave.fpinfo
 *
 * - programming note: This field is intended to be an array of state bits.
 *                     All code accessing `fpinfo' should use a mask.
 */

/* These define how fpinfo is interpreted for "Classic" Power platforms. */
#define FP_IMP_INT      0x01            /* run in imprecise mode        */
#define FP_SYNC_TASK    0x08            /* if set, run task at MSR(FE)=1*/

/* these define how fpinfo is interpreted for PowerPC platforms */
#define PPC_OFF 	0x0		/* FP trapping OFF */
#define PPC_IMP 	0x1		/* FP trapping Imprecise mode */
#define PPC_IMP_REC 	0x8	/* FP trapping Imprecise recoverable mode */
#define PPC_PRECISE 	0x9		/* FP trapping Precise mode */

/* This bit is used to indicate that a floating point interrupt
   was imprecise */
#define FP_INT_TYPE	0x02		/* type of fp interrrupt        */

#define FP_SYNC_IMP_S	8		/* shift between bits in FPINFO & MSR */

/*
 *  interrupt handler mstsave and stack pool
 */
#define FRAMESIZE               4096    /* mstsave stack frame size */
#define NUMBER_OF_FRAMES        11      /* Number of Frames per MST stack */

extern struct mststack  {
        char stack[FRAMESIZE-sizeof(struct mstsave)];
        struct mstsave save;
} mststack[];

#define FRAME_1(cpu_id) 	((NUMBER_OF_FRAMES*(cpu_id+1)) - 1)

#define PMAP_STK_SIZE		1024	/* size of V=R pmap stack */
extern char pmap_stack[];

#endif /*_H_MSTSAVE*/

