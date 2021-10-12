static char sccsid[] = "@(#)73	1.5  src/bos/kernel/proc/POWER/power_emul.c, sysproc, bos411, 9434B411a 8/23/94 09:16:29";
/*
 * COMPONENT_NAME: SYSPROC 
 *
 * FUNCTIONS: power_emul
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
/*
 * Copyright (C) Bull S.A. 1994
 * LEVEL 1,  5 Years Bull Confidential Information
 */

/*
 * Function: This is the second level POWER emulation program in "front" of p_slih().
 * 	     It's called from the first level emulation program power_asm_emulate()
 *	     on power_pc machines.
 */

#ifdef _POWER_PC

#include <sys/types.h>
#include <sys/errno.h>
#include <sys/adspace.h>
#include <sys/vmuser.h>
#include <sys/except.h>
#include <sys/mstsave.h>
#include <sys/syspest.h>
#include <sys/machine.h>
#include <sys/seg.h>
#include <sys/low.h>

#define CODEMASK	0xFC0007FE	/* mask for primary + extend opcode	*/
#define PRIMMASK	0xFC000000	/* mask for primary opcode		*/
#define PRIMSHIFT	26		/* primary opcode shift			*/
#define EXTMASK		0x000007FE	/* mask for extend opcode		*/
#define RAMASK     	0x001F0000	/* ra field mask		   	*/
#define RASHIFT		16		/* ra field shift		  	*/
#define RBMASK		0x0000F800	/* rb field mask		  	*/
#define RBSHIFT		11		/* rb field shift		  	*/
#define RTMASK		0x03E00000	/* rt field mask		  	*/
#define RTSHIFT		21		/* rt field shift		  	*/
#define XVALMASK	0x001F0000	/* value field in X-form	  	*/
#define XVALSHIFT	16		/* shift for value field in X-form 	*/
#define XFXVALMASK	0x001FF800	/* value field in XFX-form	   	*/
#define XFXVALSHIFT	11		/* shift for value field in XFX-form	*/
#define RCMASK     	0x00000001	/* Rc field mask		   	*/
#define OEMASK     	0x00000400	/* OE field mask		   	*/
#define MBMASK     	0x000007C0	/* MaskBegin field mask in instr   	*/
#define MEMASK     	0x0000003E	/* MaskEnd   field mask in instr   	*/
#define MBMEREGMASK     0x0000001F	/* MaskBegin/MaskEnd field mask in reg 	*/
#define MBSHIFT     	6		/* MaskBegin shift right 		*/
#define MESHIFT     	1		/* MaskEnd   shift right		*/
#define MINUS_ONE  	0xFFFFFFFF   	/* for -1 shift right no algebraic 	*/

#define SO_BIT     	0x10000000   	/* SO bit in CR     	 	   	*/
#define EQ_BIT     	0x20000000   	/* EQ bit in CR     	 	   	*/
#define GT_BIT     	0x40000000   	/* GT bit in CR     	 	   	*/
#define LT_BIT     	0x80000000   	/* LT bit in CR     	 	   	*/
#define BIT0MASK     	0x80000000   	/* bit[0] mask in gpr  	 	   	*/
#define CAMASK     	0x20000000   	/* CA bit mask in XER 	   		*/
#define NOCAMASK     	0xDFFFFFFF  	/* to clear the carry bit in XER    	*/
#define RTCU      	4       	/* RTCU value  	     	 	   	*/
#define RTCL      	5       	/* RTCL value  	     	 	   	*/

/* extend opcodes for primary opcode = 31 */
#define CLF		0x000000EC	/* 0x7C0000EC */     
#define CLI		0x000003EC	/* 0x7C0003EC */ 
#define DCLST		0x000004EC	/* 0x7C0004EC */ 
#define CLCS		0x00000426	/* 0x7C000426 */ 
#define MFSR		0x000004A6	/* 0x7C0004A6 */ 
#define MFSRI		0x000004E6	/* 0x7C0004E6 */ 
#define MFSPR		0x000002A6	/* 0x7C0002A6 */ 
#define MFTB		0x000002E6	/* 0x7C0002E6 */ 
#define DIV		0x00000296	/* 0x7C000296 */ 
#define DIV_OE		0x00000696	/* 0x7C000696 */ 
#define MASKG		0x0000003A	/* 0x7C00003A */ 
#define MASKIR		0x0000043A	/* 0x7C00043A */ 
#define RRIB		0x00000432	/* 0x7C000432 */ 
#define SLE		0x00000132	/* 0x7C000132 */ 
#define SLEQ		0x000001B2	/* 0x7C0001B2 */ 
#define SLIQ		0x00000170	/* 0x7C000170 */ 
#define SLLIQ		0x000001F0	/* 0x7C0001F0 */ 
#define SLLQ		0x000001B0	/* 0x7C0001B0 */ 
#define SLQ		0x00000130	/* 0x7C000130 */ 
#define SRAIQ		0x00000770	/* 0x7C000770 */ 
#define SRAQ		0x00000730	/* 0x7C000730 */ 
#define SRE		0x00000532	/* 0x7C000532 */ 
#define SREA		0x00000732	/* 0x7C000732 */ 
#define SREQ		0x000005B2	/* 0x7C0005B2 */ 
#define SRIQ		0x00000570	/* 0x7C000570 */ 
#define SRLIQ		0x000005F0	/* 0x7C0005F0 */ 
#define SRLQ		0x000005B0	/* 0x7C0005B0 */ 
#define SRQ		0x00000530	/* 0x7C000530 */ 

/* primary opcode >> PRIMSHIFT */
#define RLMI		22		/* 0x58000000 */
 

#define DECODE_RA(instr,ra)				\
{ 							\
	ra = (instr & RAMASK) >> RASHIFT;               \
}


#define DECODE_RA_RB(mstp,instr,ra,rb,eaddr)		\
{							\
	ulong	vra, vrb;				\
							\
	ra = (instr & RAMASK) >> RASHIFT;		\
	if (ra == 0) 	vra = 0;			\
	else		vra = (mstp->gpr)[ra];		\
							\
	rb = (instr & RBMASK) >> RBSHIFT;		\
	vrb = (mstp->gpr)[rb];				\
	eaddr = (caddr_t)(vra + vrb);			\
}


#define DECODE_RT_VAL(mstp,instr,rt,val) 		\
{							\
	rt = (instr & RTMASK) >> RTSHIFT; 		\
	val = (instr & XVALMASK) >> XVALSHIFT;	\
}


#define DECODE_RT_RA_RB(mstp,instr,rt,ra,rb,eaddr)	\
{							\
	ulong	vra, vrb;				\
 							\
	rt = (instr & RTMASK) >> RTSHIFT;		\
	ra = (instr & RAMASK) >> RASHIFT;		\
	rb = (instr & RBMASK) >> RBSHIFT;		\
	if (ra == 0) 	vra = 0;			\
	else		vra = (mstp->gpr)[ra];		\
	vrb = (mstp->gpr)[rb];				\
	eaddr = (caddr_t)(vra + vrb);			\
}


#define DECODE_XFX(mstp,instr,rt,val) 			\
{							\
	rt = (instr & RTMASK) >> RTSHIFT; 		\
	val = (instr & XFXVALMASK) >> XFXVALSHIFT;	\
}

void	emulate_cli_clf(caddr_t p);
void	emulate_dclst(caddr_t p);
ulong	emulate_clcs(ushort val);

ulong   emulate_count = 0;	/* number of emulated instructions */


static ulong decode_rt (ulong instr) {
	return ((instr & RTMASK) >> RTSHIFT);
}

static ulong decode_ra (ulong instr) {
	return ((instr & RAMASK) >> RASHIFT);
}

static ulong decode_rb (ulong instr) {
	return ((instr & RBMASK) >> RBSHIFT);
}

/***************************************************************************** 
 *
 * NAME: power_emul
 *
 * FUNCTION:  emulate Power architecture instructions in Power PC machines
 * 
 * EXECUTION ENVIRONMENT:
 *
 *	 called from power_asm_emulate() only
 *
 *       It may page fault.
 *
 * Notes:  If instruction is successful emulated then return to
 *         power_asm_emulate() and finish interrupt.
 *         If not emulated then call p_slih().
 *
 *         Backtrack flag is turned on in csa to handle possible page faults.
 *
 *****************************************************************************
 */

int
power_emul(struct  mstsave *mstp,	/* pointer to the faulting mstsave	*/
	      caddr_t addr,		/* failing instr. effective address	*/
	      ulong   except,           /* exception type			*/
	      struct thread *threadp)   /* pointer to the current thread	*/
{
	ulong srval;			/* Value in segment register		*/
	register caddr_t  p;            /* pointer to failing instruction	*/
	caddr_t eaddr;			/* effective address of target		*/
	ulong	instr;  		/* failing instruction		 	*/
	ulong	opcode; 		/* opcode of failing instruction	*/
	ulong	popcode;		/* primary opcode			*/
	ulong	extopcode;		/* extend opcode			*/
	ulong	ra, rb, rt, val; 	/* values of fields in failing instr	*/
	int     rt_val;			/* return value in rt register		*/
	int     rc;			/* return code				*/
	ulong   val1, val2,		/* temporary value			*/
		mask, rot,		/* value of mask and rotate	 	*/
		w_sign;  		/* word of 32 sign bits			*/
	long long dividend, quotient;	/* ops 64 bits				*/	
	long	  divisor;		/* ops 32 bits				*/
	struct timestruc_t timer;	/* to hold a timer value		*/


	ASSERT(__power_pc());

	/* get the segment register value of the faulting address */
	srval = as_getsrval(&mstp->as, addr);

	/*
	 * Make the failing instruction addressible and then read it.
	 * Have to turn backtrack flag on because page faults may occur when
	 * fetching the instruction and/or performing emulation.
	 */
	p = vm_att(srval, addr);
	csa->backt = 1;
	instr = *((ulong *)p);
	vm_det(p);

	/*
	 * Decode the instruction and perform the emulation
	 */
	opcode = instr & CODEMASK;
	popcode = (instr & PRIMMASK) >> PRIMSHIFT;
	extopcode = instr & EXTMASK;

	switch (popcode) {

	case 31: switch(extopcode) {

		case CLF:
			DECODE_RA_RB(mstp,instr,ra,rb,eaddr);
			if (mstp->msr & MSR_DR)
			{
				srval = as_getsrval(&mstp->as, eaddr);
				p = vm_att(srval, eaddr);
				emulate_cli_clf(p);
				vm_det(p);
			}
			else    emulate_cli_clf(eaddr);

			if (ra != 0)  (mstp->gpr)[ra] = (ulong_t)eaddr;
			rc = EXCEPT_HANDLED;
			break;

		case CLI:
			/* Have the process killed if it issues cli in
			 * the user mode
			 */  
			if ((mstp->prev == NULL) && (mstp->msr & MSR_PR))
			{
				except = EXCEPT_PRIV_OP;
				rc = EXCEPT_NOT_HANDLED;
				break;
			}
				
	                DECODE_RA_RB(mstp,instr,ra,rb,eaddr);
			if (mstp->msr & MSR_DR)
			{
				srval = as_getsrval(&mstp->as, eaddr);
				p = vm_att(srval, eaddr);
				emulate_cli_clf(p);
				vm_det(p);
			}
			else    emulate_cli_clf(eaddr);
	
			if (ra != 0)  (mstp->gpr)[ra] = (ulong_t)eaddr;
			rc = EXCEPT_HANDLED;
	    		break;

		case DCLST:
			DECODE_RA_RB(mstp,instr,ra,rb,eaddr);
			if (mstp->msr & MSR_DR)
			{
				srval = as_getsrval(&mstp->as, eaddr);
				p = vm_att(srval, eaddr);
				emulate_dclst(p);
				vm_det(p);
			}
			else	emulate_dclst(eaddr);

			if (ra != 0)  (mstp->gpr)[ra] = (ulong_t)eaddr;
			rc = EXCEPT_HANDLED;
			break;

		case CLCS:
			ASSERT(!__power_601());
			DECODE_RT_VAL(mstp,instr,rt,val);
			/* will return 0 in RT if val between 0-11 or 16-31 */
			(mstp->gpr)[rt] = emulate_clcs(val);
			rc = EXCEPT_HANDLED;
			break;
	
		case MFSR:
			DECODE_RT_VAL(mstp,instr,rt,val);
			(mstp->gpr)[rt] = (mstp->as).srval[val];
			rc = EXCEPT_HANDLED;
			break;

		case MFSRI:
			DECODE_RT_RA_RB(mstp,instr,rt,ra,rb,eaddr);
			(mstp->gpr)[rt] =
				(mstp->as).srval[(ulong)eaddr>>SEGSHIFT];
			if (ra != 0 && ra != rt)
			{
				(mstp->gpr)[ra] = (ulong_t)eaddr;
			}
			rc = EXCEPT_HANDLED;
			break;
	
		case MFSPR:
			/* Emulate MF_RTCU and MF_RTCL only */
	                DECODE_RT_VAL(mstp,instr,rt,val);
	 		if (val != RTCU && val != RTCL)
			   	rc = EXCEPT_NOT_HANDLED;
			else {
	                	ASSERT(!__power_601());
				curtime_ppc(&timer);
				if (val == RTCU)
					(mstp->gpr)[rt] = timer.tv_sec;
				else
					(mstp->gpr)[rt] = timer.tv_nsec;
				rc = EXCEPT_HANDLED;
			}
			break;

/***************************************************************************
 *
 * Perform power instruction emulation (design 82034) for POWER_PC ships
 *
 ***************************************************************************
 */

		case DIV:
		case DIV_OE:
			/* div RT,RA,RB
			 * codeop    : 31-RT-RA-RB-OE-331-Rc
			 * pseudocode: ((RA)||(MQ))/RB into RT and remainder into MQ
			 */

			rt = decode_rt(instr);
			ra = decode_ra(instr);
			rb = decode_rb(instr);

			dividend = (((long long)((mstp->gpr)[ra])) << 32) | mstp->mq;
			divisor  = (long)((mstp->gpr)[rb]);

			quotient = dividend / divisor;	  /* 64 bits = 64 bits / 32 bits */

			rt_val = quotient; /* 32 bits right part of 64 bits */

			/* this line must be here!! Otherwise, asm code is false. */
			val2 = (ulong)(quotient >> 32); /* 32 bits left part of 64 bits */

			/* store quotient in RT */
			(mstp->gpr)[rt] = (ulong)rt_val;

			/* store remainder in MQ */
			/* Right, it's bad to call divi64 again but code generated */
			/* for mul and sub with "long long" type is not correct.   */
			val1 = mstp->mq = (ulong)(dividend % divisor);

			if (instr & OEMASK) {    /* OE bit is set */
				/* overflow if quotient cannot be represented in 32 bits */
				/* (-2**31 / -1) return MQ=0 and RT=-2**31 but overflow is */
				/* not detected because val2 is null in this special case. */
				if ((divisor == 0) ||
				    ((val2 == 0) && (rt_val < 0)) ||
				    ((val2 == -1) && (rt_val > 0))) {
					val2 = 1; /* to force overflow */
				}

				if (((long)val2 > 0) || ((long)val2 < -1 )) {
					/* overflow */
					mstp->xer |= 0x80000000; /* set XER[OV] */
					mstp->xer |= 0x40000000; /* set XER[SO] */
				} else
					/* no overflow */
					mstp->xer &= 0x7FFFFFFF; /* clear XER[OV] */
					/* don't modify XER[SO] */
			}

			if (instr & RCMASK) {    /* Rc bit is set */
				if (val1 == 0)
					mstp->cr |= EQ_BIT;
				else
					mstp->cr &= ~EQ_BIT;
				if ((long)val1 < 0)
					mstp->cr |= LT_BIT;
				else
					mstp->cr &= ~LT_BIT;
				if ((long)val1 > 0)
					mstp->cr |= GT_BIT;
				else
					mstp->cr &= ~GT_BIT;

				/* update CR0[SO] bit with XER[SO] bit */
				if (mstp->xer & BIT0MASK)
					/* set the SO bit */
					mstp->cr |= (BIT0MASK >> 3);
				else
					/* clear the SO bit */
					mstp->cr &= ~(BIT0MASK >> 3);
			}

			rc = EXCEPT_HANDLED;
			break;

		case MASKG:
			/* maskg RA,RS,RB
			 * codeop    : 31-RS-RA-RB-29-Rc
			 * pseudocode: mstart=RS[27-31], mstop=RB[27-31], mask into RA
			 */

			rt = decode_rt(instr);
			ra = decode_ra(instr);
			rb = decode_rb(instr);

			val1 = ((mstp->gpr)[rt] & MBMEREGMASK);
			val2 = ((mstp->gpr)[rb] & MBMEREGMASK);
			mask = -1; /* ulong cast */

			if (val1 < val2 + 1)
				(mstp->gpr)[ra] = ((mask >> val1) ^ (mask >> val2 + 1));
			else if (val1 > val2 + 1)
					(mstp->gpr)[ra] = (mask >> val1) | ~(mask >> val2 + 1);
			     else       /* val1 == val2 + 1 */
					(mstp->gpr)[ra] = mask;  /* all bits = 1 */

			if (instr & RCMASK)    /* Rc bit is set */
				update_cr0(mstp, ra);

			rc = EXCEPT_HANDLED;
			break;

		case MASKIR:
			/* maskir RA,RS,RB
			 * codeop    : 31-RS-RA-RB-541-Rc
			 * pseudocode: RS inserted into RA under control of mask in RB
			 */

			rt = decode_rt(instr);
			ra = decode_ra(instr);
			rb = decode_rb(instr);

			mask = (mstp->gpr)[rb];
			(mstp->gpr)[ra] = ((mstp->gpr)[ra] & ~mask) |
				          ((mstp->gpr)[rt] & mask);

			if (instr & RCMASK)    /* Rc bit is set */
				update_cr0(mstp, ra);

			rc = EXCEPT_HANDLED;
			break;

		case RRIB:
			/* rrib RA,RS,RB
			 * codeop    : 31-RS-RA-RB-537-Rc
			 * pseudocode: RS[0] bit rotated right RB[27-31] then inserted into RA.
			 */

			rt = decode_rt(instr);
			ra = decode_ra(instr);
			rb = decode_rb(instr);

			val1 = ((mstp->gpr)[rt] & BIT0MASK);  /* true if rt[0] bit = 1 */

			/* rotate right a forced set bit[0] */
			mask = (BIT0MASK >> ((mstp->gpr)[rb] & MBMEREGMASK));

			/* insert rotated bit in RA */
			if (val1)
				/* force to set the bit */
				(mstp->gpr)[ra] = (mstp->gpr)[ra] | mask;
			else
				/* force to clear the bit */
				(mstp->gpr)[ra] = (mstp->gpr)[ra] & ~mask;

			if (instr & RCMASK)    /* Rc bit is set */
				update_cr0(mstp, ra);

			rc = EXCEPT_HANDLED;
			break;

		case SLE:
			/* sle RA,RS,RB
			 * codeop    : 31-RS-RA-RB-153-Rc
			 * pseudocode: RS rotated left RB[27-31] and inserted into MQ.
			 *	       Logical AND between MQ and generated mask placed into RA.
			 */

			rt = decode_rt(instr);
			ra = decode_ra(instr);
			rb = decode_rb(instr);

			/* value to rotate */
			val = ((mstp->gpr)[rb] & MBMEREGMASK);

			/* MQ = rotate rt left */
			mstp->mq = rot =
				(((mstp->gpr)[rt] << val) | ((mstp->gpr)[rt] >> (32 - val)));

			/* logical and with shifted mask */
			mask = -1; /* ulong cast */
			(mstp->gpr)[ra] = (rot & (mask << val));

			if (instr & RCMASK)    /* Rc bit is set */
				update_cr0(mstp, ra);

			rc = EXCEPT_HANDLED;
			break;

		case SLEQ:
			/* sleq RA,RS,RB
			 * codeop    : 31-RS-RA-RB-217-Rc
			 * pseudocode: RS rotated left RB[27-31] and merged with MQ
			 *	       under control of generated mask then placed into RA.
			 *	       Rotated word inserted into MQ.
			 */

			rt = decode_rt(instr);
			ra = decode_ra(instr);
			rb = decode_rb(instr);

			/* value to rotate */
			val = ((mstp->gpr)[rb] & MBMEREGMASK);

			/* rotate rt left */
			rot = (((mstp->gpr)[rt] << val) | ((mstp->gpr)[rt] >> (32 - val)));

			/* merge with shifted mask */
			mask = MINUS_ONE << val;  
			(mstp->gpr)[ra] = ((rot & mask) | (mstp->mq & ~mask));

			/* Update now MQ */
			mstp->mq = rot;

			if (instr & RCMASK)    /* Rc bit is set */
				update_cr0(mstp, ra);

			rc = EXCEPT_HANDLED;
			break;

		case SLIQ:
			/* sliq RA,RS,SH
			 * codeop    : 31-RS-RA-SH-184-Rc
			 * pseudocode: RS rotated left SH and inserted into MQ.
			 *	       Logical AND between MQ and generated mask placed into RA.
			 */

			rt = decode_rt(instr);
			ra = decode_ra(instr);

			/* value to rotate */
			val = ((instr & RBMASK) >> RBSHIFT);

			/* MQ = rotate rt left */
			mstp->mq = rot =
				(((mstp->gpr)[rt] << val) | ((mstp->gpr)[rt] >> (32 - val)));

			/* logical and with shifted mask */
			mask = -1; /* ulong cast */
			(mstp->gpr)[ra] = (rot & (mask << val));

			if (instr & RCMASK)    /* Rc bit is set */
				update_cr0(mstp, ra);

			rc = EXCEPT_HANDLED;
			break;

		case SLLIQ:
			/* slliq RA,RS,SH
			 * codeop    : 31-RS-RA-SH-248-Rc
			 * pseudocode: RS rotated left SH and merged with MQ
			 *	       under control of generated mask then placed into RA.
			 *	       Rotated word inserted into MQ.
			 */

			rt = decode_rt(instr);
			ra = decode_ra(instr);

			/* value to rotate */
			val = ((instr & RBMASK) >> RBSHIFT); /* SH */

			/* rotate rt left */
			rot = (((mstp->gpr)[rt] << val) | ((mstp->gpr)[rt] >> (32 - val)));

			/* merge with shifted mask */
			mask = MINUS_ONE << val;  
			(mstp->gpr)[ra] = ((rot & mask) | (mstp->mq & ~mask));

			/* Update now MQ */
			mstp->mq = rot;

			if (instr & RCMASK)    /* Rc bit is set */
				update_cr0(mstp, ra);

			rc = EXCEPT_HANDLED;
			break;

		case SLLQ:
			/* sllq RA,RS,RB
			 * codeop    : 31-RS-RA-RB-216-Rc
			 * pseudocode: RS rotated left RB[27-31] and merged with MQ
			 *	       under control of generated mask then placed into RA.
			 *	       Merge and mask are RB[26] dependant.
			 */

			rt = decode_rt(instr);
			ra = decode_ra(instr);
			rb = decode_rb(instr);

			/* value to rotate */
			val = ((mstp->gpr)[rb] & MBMEREGMASK);

			/* rotate rt left */
			rot = (((mstp->gpr)[rt] << val) | ((mstp->gpr)[rt] >> (32 - val)));

			/* merge is RB[26] dependent */
			if ((mstp->gpr)[rb] & 0x00000020) {
				/* RB[26]=1 */
				mask = MINUS_ONE >> (32 - val);
				(mstp->gpr)[ra] = (mstp->mq & ~mask);
			} else {
				/* RB[26]=0 */
				mask = MINUS_ONE << val;
				(mstp->gpr)[ra] = ((rot & mask)| (mstp->mq & ~mask));
			}
			if (instr & RCMASK)    /* Rc bit is set */
				update_cr0(mstp, ra);

			rc = EXCEPT_HANDLED;
			break;

		case SLQ:
			/* slq RA,RS,RB
			 * codeop    : 31-RS-RA-RB-152-Rc
			 * pseudocode: RS rotated left RB[27-31] and inserted into MQ.
			 *	       Logical AND between MQ and generated mask placed into RA.
			 *	       Mask is RB[26] dependant.
			 */

			rt = decode_rt(instr);
			ra = decode_ra(instr);
			rb = decode_rb(instr);

			/* value to rotate */
			val = ((mstp->gpr)[rb] & MBMEREGMASK);

			/* MQ = rotate rt left */
			mstp->mq = rot =
				(((mstp->gpr)[rt] << val) | ((mstp->gpr)[rt] >> (32 - val)));

			/* mask is RB[26] dependent */
			if ((mstp->gpr)[rb] & 0x00000020)
				/* RB[26]=1 */
				mask = 0;
			else
				/* RB[26]=0 */
				mask = -1; /* ulong cast */

			/* logical and with shifted mask */
			(mstp->gpr)[ra] = (rot & (mask << val));

			if (instr & RCMASK)    /* Rc bit is set */
				update_cr0(mstp, ra);

			rc = EXCEPT_HANDLED;
			break;

		case SRAIQ:
			/* sraiq RA,RS,SH
			 * codeop    : 31-RS-RA-SH-952-Rc
			 * pseudocode: RS rotated right SH inserted into MQ then merged with
			 *	       word of 32 sign bits from RS under control of generated
			 *	       mask then placed into RA. XER[CA] is computed.
			 */

			rt = decode_rt(instr);
			ra = decode_ra(instr);

			/* value to rotate */
			val = ((instr & RBMASK) >> RBSHIFT); /* SH */

			/* MQ = rotate rt right val */
			mstp->mq = rot =
				(((mstp->gpr)[rt] << (32 - val)) | ((mstp->gpr)[rt] >> val));

			/* word of 32 sign bits from RT */
			if ((long)(mstp->gpr)[rt] < 0)
				w_sign = -1; /* ulong cast */
			else
				w_sign = 0;

			/* merge with shifted mask */
			mask = MINUS_ONE >> val;  
			(mstp->gpr)[ra] = ((rot & mask) | (w_sign & ~mask));

			/* produce XER[CA] */
			if ((rot & ~mask) == 0)
				/* clear CA */
				mstp->xer &= NOCAMASK;
			else
				/* CA = RS[0] */
				if ((mstp->gpr)[rt] & BIT0MASK)
					/* RS[0]=1: set CA */
					mstp->xer |= CAMASK;
				else
					/* RS[0]=0: clear CA */
					mstp->xer &= NOCAMASK;

			if (instr & RCMASK)    /* Rc bit is set */
				update_cr0(mstp, ra);

			rc = EXCEPT_HANDLED;
			break;

		case SRAQ:
			/* sraq RA,RS,RB
			 * codeop    : 31-RS-RA-RB-920-Rc
			 * pseudocode: RS rotated right RB[27-31] inserted into MQ then
			 *	       merged with word of 32 sign bits from RS under control
			 *	       of generated mask then placed into RA. Mask is RB[26]
			 *	       dependant and XER[CA] is computed.
			 */

			rt = decode_rt(instr);
			ra = decode_ra(instr);
			rb = decode_rb(instr);

			/* value to rotate */
			val = ((mstp->gpr)[rb] & MBMEREGMASK);

			/* MQ = rotate rt right val */
			mstp->mq = rot =
				(((mstp->gpr)[rt] << (32 - val)) | ((mstp->gpr)[rt] >> val));

			/* word of 32 sign bits from RT */
			if ((long)(mstp->gpr)[rt] < 0)
				w_sign = -1; /* ulong cast */
			else
				w_sign = 0;

			/* mask is RB[26] dependent*/
			if ((mstp->gpr)[rb] & 0x00000020)
				/* RB[26]=1 */
				mask = 0;
			else
				/* RB[26]=0 */
				mask = MINUS_ONE >> val;

			/* merge with shifted mask */
			(mstp->gpr)[ra] = ((rot & mask) | (w_sign & ~mask));

			/* produce XER[CA] */
			if ((rot & ~mask) == 0)
				/* clear CA */
				mstp->xer &= NOCAMASK;
			else
				/* CA = RS[0] */
				if ((mstp->gpr)[rt] & BIT0MASK)
					/*  RS[0]=1: set CA */
					mstp->xer |= CAMASK;
				else
					/*  RS[0]=0: clear CA */
					mstp->xer &= NOCAMASK;

			if (instr & RCMASK)    /* Rc bit is set */
				update_cr0(mstp, ra);

			rc = EXCEPT_HANDLED;
			break;

		case SRE:
			/* sre RA,RS,RB
			 * codeop    : 31-RS-RA-RB-665-Rc
			 * pseudocode: RS rotated right RB[27-31] and inserted into MQ.
			 *	       Logical AND between MQ and generated mask placed into RA.
			 */

			rt = decode_rt(instr);
			ra = decode_ra(instr);
			rb = decode_rb(instr);

			/* value to rotate */
			val = ((mstp->gpr)[rb] & MBMEREGMASK);

			/* MQ = rotate rt right val */
			mstp->mq = rot =
				(((mstp->gpr)[rt] << (32 - val)) | ((mstp->gpr)[rt] >> val));

			/* logical and with shifted mask */
			mask = -1; /* ulong cast */
			(mstp->gpr)[ra] = (rot & (mask >> val));

			if (instr & RCMASK)    /* Rc bit is set */
				update_cr0(mstp, ra);

			rc = EXCEPT_HANDLED;
			break;

		case SREA:
			/* srea RA,RS,RB
			 * codeop    : 31-RS-RA-RB-921-Rc
			 * pseudocode: RS rotated right RB[27-31] and inserted into MQ then
			 *	       merged with word of 32 sign bits from RS under control
			 *	       of generated mask then placed into RA. XER[CA] is computed.
			 */

			rt = decode_rt(instr);
			ra = decode_ra(instr);
			rb = decode_rb(instr);

			/* value to rotate */
			val = ((mstp->gpr)[rb] & MBMEREGMASK);

			/* MQ = rotate rt right val */
			mstp->mq = rot =
				(((mstp->gpr)[rt] << (32 - val)) | ((mstp->gpr)[rt] >> val));

			/* word of 32 sign bits from RT */
			if ((long)(mstp->gpr)[rt] < 0)
				w_sign = -1; /* ulong cast */
			else
				w_sign = 0;

			/* merge with shifted mask */
			mask = MINUS_ONE >> val;  
			(mstp->gpr)[ra] = ((rot & mask) | (w_sign & ~mask));

			/* produce XER[CA] */
			if ((rot & ~mask) == 0)
				/* clear CA */
				mstp->xer &= NOCAMASK;
			else
				/* CA = RS[0] */
				if ((mstp->gpr)[rt] & BIT0MASK)
					/* RS[0]=1: set CA */
					mstp->xer |= CAMASK;
				else
					/* RS[0]=0: clear CA */
					mstp->xer &= NOCAMASK;

			if (instr & RCMASK)    /* Rc bit is set */
				update_cr0(mstp, ra);

			rc = EXCEPT_HANDLED;
			break;

		case SREQ:
			/* sreq RA,RS,RB
			 * codeop    : 31-RS-RA-RB-729-Rc
			 * pseudocode: RS rotated right RB[27-31] and merged with MQ 
			 *	       under control of generated mask then placed into RA.
			 *	       Rotated word inserted into MQ.
			 */

			rt = decode_rt(instr);
			ra = decode_ra(instr);
			rb = decode_rb(instr);

			/* value to rotate */
			val = ((mstp->gpr)[rb] & MBMEREGMASK);

			/* rotate rt right val */
			rot = (((mstp->gpr)[rt] << (32 - val)) | ((mstp->gpr)[rt] >> val));

			/* merge with shifted mask */
			mask = MINUS_ONE >> val;  
			(mstp->gpr)[ra] = ((rot & mask) | (mstp->mq & ~mask));

			/* Update now MQ */
			mstp->mq = rot;

			if (instr & RCMASK)    /* Rc bit is set */
				update_cr0(mstp, ra);

			rc = EXCEPT_HANDLED;
			break;

		case SRIQ:
			/* sriq RA,RS,SH
			 * codeop    : 31-RS-RA-SH-696-Rc
			 * pseudocode: RS rotated right SH and stored into MQ.
			 *	       Logical AND between MQ and generated mask placed into RA.
			 */

			rt = decode_rt(instr);
			ra = decode_ra(instr);

			/* value to rotate */
			val = ((instr & RBMASK) >> RBSHIFT); /* SH */

			/* MQ = rotate rt right val */
			mstp->mq = rot =
				(((mstp->gpr)[rt] << (32 - val)) | ((mstp->gpr)[rt] >> val));

			/* logical and with shifted mask */
			mask = -1; /* ulong cast */
			(mstp->gpr)[ra] = (rot & (mask >> val));

			if (instr & RCMASK)    /* Rc bit is set */
				update_cr0(mstp, ra);

			rc = EXCEPT_HANDLED;
			break;

		case SRLIQ:
			/* srliq RA,RS,SH
			 * codeop    : 31-RS-RA-SH-760-Rc
			 * pseudocode: RS rotated right SH and merged with MQ.
			 *	       under control of generated mask then placed into RA.
			 *	       Rotated word inserted into MQ.
			 */

			rt = decode_rt(instr);
			ra = decode_ra(instr);

			/* value to rotate */
			val = ((instr & RBMASK) >> RBSHIFT); /* SH */

			/* rotate rt right val */
			rot = (((mstp->gpr)[rt] << (32 - val)) | ((mstp->gpr)[rt] >> val));

			/* merge with shifted mask */
			mask = MINUS_ONE >> val;  
			(mstp->gpr)[ra] = ((rot & mask) | (mstp->mq & ~mask));

			/* Update now MQ */
			mstp->mq = rot;

			if (instr & RCMASK)    /* Rc bit is set */
				update_cr0(mstp, ra);

			rc = EXCEPT_HANDLED;
			break;

		case SRLQ:
			/* srlq RA,RS,RB
			 * codeop    : 31-RS-RA-RB-728-Rc
			 * pseudocode: RS rotated right RB[27-31] merged with MQ under
			 *	       control of generated mask then placed into RA.
			 *	       Mask is RB[26] dependant.
			 */

			rt = decode_rt(instr);
			ra = decode_ra(instr);
			rb = decode_rb(instr);

			/* value to rotate */
			val = ((mstp->gpr)[rb] & MBMEREGMASK);

			/* rotate rt right val */
			rot = (((mstp->gpr)[rt] << (32 - val)) | ((mstp->gpr)[rt] >> val));

			/* merge is RB[26] dependent */
			if ((mstp->gpr)[rb] & 0x00000020) {
				/* RB[26]=1 */
				mask = MINUS_ONE << (32 - val);
				(mstp->gpr)[ra] = (mstp->mq & ~mask);
			} else {
				/* RB[26]=0 */
				mask = MINUS_ONE >> val;
				(mstp->gpr)[ra] = ((rot & mask) | (mstp->mq & ~mask));
			}
			if (instr & RCMASK)    /* Rc bit is set */
				update_cr0(mstp, ra);

			rc = EXCEPT_HANDLED;
			break;

		case SRQ:
			/* srq RA,RS,RB
			 * codeop    : 31-RS-RA-RB-664-Rc
			 * pseudocode: RS rotated right RB[27-31] and stored into MQ.
			 *	       Logical AND between MQ and generated mask placed into RA.
			 *	       Mask is RB[26] dependant.
			 */

			rt = decode_rt(instr);
			ra = decode_ra(instr);
			rb = decode_rb(instr);

			/* value to rotate */
			val = ((mstp->gpr)[rb] & MBMEREGMASK);

			/* MQ = rotate rt right val */
			mstp->mq = rot =
				(((mstp->gpr)[rt] << (32 - val)) | ((mstp->gpr)[rt] >> val));

			/* mask is RB[26] dependent */
			if ((mstp->gpr)[rb] & 0x00000020)
				/* RB[26]=1 */
				mask = 0;
			else
				/* RB[26]=0 */
				mask = -1; /* ulong cast */

			/* logical and with shifted mask */
			(mstp->gpr)[ra] = (rot & (mask >> val));

			if (instr & RCMASK)    /* Rc bit is set */
				update_cr0(mstp, ra);

			rc = EXCEPT_HANDLED;
			break;

		default:
			rc = EXCEPT_NOT_HANDLED;
			break;
		} /* end of switch on extended opcode */
		break;

	case RLMI:	/* rlmi RA,RS,RB,MB,ME
			 * codeop    : 22-RS-RA-RB-MB-ME-Rc
			 * pseudocode: RS rotated left RB[27-31] and inserted into RA
			 *	       under control of generated mask.
			 */

			rt = decode_rt(instr);
			ra = decode_ra(instr);
			rb = decode_rb(instr);

			/* rotate rt left */
			val = ((mstp->gpr)[rb] & MBMEREGMASK); /* value to rotate */
			rot = (((mstp->gpr)[rt] << val) | ((mstp->gpr)[rt] >> (32 - val)));

			/* mask */
			val1 = ((instr & MBMASK) >> MBSHIFT);    /* MB: Mask Begin */
			val2 = ((instr & MEMASK) >> MESHIFT);    /* ME: Mask End   */
			val = -1; /* ulong cast */

			/* logical and with shifted mask */
			if (val1 < val2 + 1) {
				mask = (val >> val1) ^ (val >> val2 + 1);
				(mstp->gpr)[ra] = (rot & mask) | ((mstp->gpr)[ra] & ~mask);

			} else if (val1 > val2 + 1) {
				mask = (val >> val1) | ~(val >> val2 + 1);
				(mstp->gpr)[ra] = (rot & mask) | ((mstp->gpr)[ra] & ~mask);

				} else        /* val1 == val2 + 1 */
					(mstp->gpr)[ra] = rot;

			if (instr & RCMASK)    /* Rc bit is set */
				update_cr0(mstp, ra);

			rc = EXCEPT_HANDLED;
			break;

	default:	rc = EXCEPT_NOT_HANDLED;
			break;
	} /* end of switch on primary opcode */


	/* turn backtrack flag off */
	csa->backt = 0;

	if (rc == EXCEPT_HANDLED) {
		/*
		 * resume at address next to the failing instruction
		 * and increment the emulation count.
		 */
		mstp->iar += sizeof(ulong); 	
		emulate_count++;
		return (rc);
	} else {
		/*
		 * No emulation for this exception so
		 * execute p_slih().
		 */
		p_slih(mstp, addr, except, threadp);
	}
}


int
update_cr0(struct  mstsave *mstp,   /* pointer to the faulting mstsave   */
	   ushort ri)               /* register index	     	         */
{
	/* Update CR0[EQ,LT,GT,SO] bits */
	if ((mstp->gpr)[ri] == 0)
		mstp->cr |= EQ_BIT;
	else
		mstp->cr &= ~EQ_BIT;

	if ((long)((mstp->gpr)[ri]) < 0)
		mstp->cr |= LT_BIT;
	else
		mstp->cr &= ~LT_BIT;

	if ((long)((mstp->gpr)[ri]) > 0)
		mstp->cr |= GT_BIT;
	else
		mstp->cr &= ~GT_BIT;

	/* put XER[SO] bit in CR[SO] */
	if (mstp->xer & BIT0MASK)
		/* set the SO bit */
		mstp->cr |= (BIT0MASK >> 3);
	else
		/* clear the SO bit */
		mstp->cr &= ~(BIT0MASK >> 3);
}

#endif /* _POWER_PC */

