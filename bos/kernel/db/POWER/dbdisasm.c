static char sccsid[] = "@(#)40	1.11  src/bos/kernel/db/POWER/dbdisasm.c, sysdb, bos41J, 9518A_all 5/2/95 10:50:28";

/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS: debug_opcode
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/seg.h>
#include <sys/systemcfg.h>
#include "debvars.h"
#include "dbdebug.h"
#include <disassembly.h>
#include <ops.h>

/*
 * EXTERNAL PROCEDURES CALLED:
 */
extern	int	decode_instruction();
extern char	*getterm();

#define SREG(x)  ((x)>>SEGSHIFT)        /* Segment reg. #               */
#define IS_IOSPACE(x) ((x) & 0x80000000)

/*
 * NAME: debug_opcode
 *
 * FUNCTION:
 *   decode the instruction string
 *
 *
 * RETURN VALUE: TRUE on success, FALSE if can't access address
 *
 */


int
debug_opcode(instr_addr,instr_descr)
ulong	instr_addr;
struct descr *instr_descr;
{
	ulong	sj_inst;	/* sj machine lang instr and the instr addr */
	inst	disasm;		/* disassembly structure */
	int	rc;

 	/* if i/o address don't decode it. */
	if(IS_IOSPACE(debvars[IDSEGS+SREG(instr_addr)].hv)) {
		printf("Bad IAR ");
		return(FALSE);
	}

	/* set sj_inst to contents of IAR */
   	if (!get_from_memory(instr_addr,1/*always virt*/,&sj_inst,
		sizeof(sj_inst))) {
		printf("Bad IAR ");
		return FALSE; 
	}

	/* xlate instruction to ascii string and get D_EA if it is a branch */
	rc = decode_instruction(sj_inst, &disasm, instr_addr, 
 			(__power_pc() ? PPC : PWR),(__power_pc() ? PPC : PWR));

	if (!rc){
		printf("invalid instruction encountered ");
		return FALSE; 
	}

	sprintf(instr_descr->D_Mnemonic, "%8s   %s",
			disasm.mnemonic, disasm.operands);
	if (disasm.target_addr)
		instr_descr->D_EA = (caddr_t) disasm.target_addr;
	else
		instr_descr->D_EA = (caddr_t) -1;
	instr_descr->D_Len = sizeof(INSTSIZ);
	instr_descr->D_NSI = (caddr_t)instr_addr + sizeof(INSTSIZ);

	/* D_EA holds addr of the branch or -1 if not a branch */
	instr_descr->D_Target = instr_descr->D_EA;

	/* this removes left over targets after branches */
	if (instr_descr->D_NSI == instr_descr->D_Target)
		instr_descr->D_Target = (caddr_t)-1;

	/* set the Brbit flag  if branch instruction */
	instr_descr->D_Brbit = (instr_descr->D_Target != (caddr_t)-1);

	/* 	Debug output
	printf(" instr_descr->D_Mnemonic:%s", instr_descr->D_Mnemonic);
	printf(" instr_descr->D_NSI:%x", instr_descr->D_NSI);
	printf(" instr_descr->D_EA:%x", instr_descr->D_EA);
	printf(" instr_descr->D_Brbit: %x", instr_descr->D_Brbit);
	printf(" instr_descr->D_Len: %x", instr_descr->D_Len);
	printf(" instr_descr->D_Target: %x\n", instr_descr->D_Target);
	*/

	return TRUE; 
}

/*
 * NAME: branch_taken
 *
 * FUNCTION:
 *   Given a branch instruction address, determine whether the branch will be
 *   taken.  Note: the IAR passed *must* be for a branch.
 *
 * RETURN VALUE: TRUE if branch will be taken, FALSE otherwise.
 *
 */
/*
Since we can assume that we have we have been passed a branch op,
we know that if it's conditional, the BO field is bits 6-10, and the BI
field is bits 11-15.  We then have three categories of things to
consider:
	1. Is this an unconditional branch?
	2. Does the CR bit specified by BI match the condition 
	requirements specified in BO?
	3. Does the state of CTR match the requirements in BO?

The bits in BO can be summarized as follows:
	6 - If on, don't care about CR
	7 - Branch if matches bit BI in CR
	8 - If on, don't care about CTR
	9 - If on, branch if CTR==0.  If off, branch if CTR!=0.
	10 - Branch prediction - ignored here.

So, we will break the instruction decode into these three parts, once
we get hold of the actual opcode.  Note that we must check for CTR == 1,
since the branch decrements CTR before testing it.
*/

/* Define constants/macros needed to pick out individual opcode bits. */
#define OPCODE_MASK	0xfc000000
#define CR_DONTCARE	0x02000000
#define CR_MATCH	0x01000000
#define CTR_DONTCARE	0x00800000
#define CTR_EQ_ZERO	0x00400000
#define BI_MASK		0x001f0000
#define BI_VALUE(x)	(((x)&BI_MASK)>>16)

int
branch_taken(instr_addr)
ulong	instr_addr;
{
	ulong	sj_inst;	/* sj machine lang instr and the instr addr */
	ulong	cr_mask;	/* Variable to use to dynamically mask CR */

 	/* if i/o address don't decode it. */
	if(IS_IOSPACE(debvars[IDSEGS+SREG(instr_addr)].hv)) {
		printf("Bad IAR ");
		return(FALSE);
	}

	/* set sj_inst to contents of IAR */
   	if (!get_from_memory(instr_addr,1/*always virt*/,&sj_inst,
		sizeof(sj_inst))) {
		printf("Bad IAR ");
		return FALSE; 
	}

	/* Is this an unconditional branch?  We're done. */
	if ((sj_inst & OPCODE_MASK) == 0x48000000)
		return TRUE;

	/* Do we care about the CR's? */
	if (!(sj_inst & CR_DONTCARE)) {
		cr_mask = 0x80000000;		/* Initialize to bit 0 */
		cr_mask >>= BI_VALUE(sj_inst);	/* Move to bit BI */
		/* Does the proper CR bit match bit 7 in the opcode? */
		if (((debvars[IDCR].hv & cr_mask) && !(sj_inst & CR_MATCH)) ||
		    (!(debvars[IDCR].hv & cr_mask) && (sj_inst & CR_MATCH)))
			return FALSE;		/* No, no branch */
	}

	/* Do we care about CTR? */
	if (!(sj_inst & CTR_DONTCARE)) {
		/* Check value of CTR against bit 9 in opcode */
		if (((debvars[IDCTR].hv != 1) && (sj_inst & CTR_EQ_ZERO)) ||
		   ((debvars[IDCTR].hv == 1) && !(sj_inst & CTR_EQ_ZERO)))
			return FALSE;		/* No match, no branch */
	}

	/* If we make it here, then we're taking the branch. */
	return TRUE; 
}

ulong
debug_disasm(addr,number,virt)
ulong addr;
int     number, virt;
{
	int i, rc;
	ulong sj_inst;				/* Instruction holder	   */
	inst	disasm;		/* disassembly structure */

	if (number >= 0x200) number = 0x200;	/* to prevent long dumps  */
	addr = ((addr+3)/4)*4;			/* Round down to word bdy */
	for (i=1; i<=number; i++,addr+=4) {	/* Display number opcodes */
		if (i == ((i/20)*20)) {		/* Time to ask for a break?*/

			printf("Press ENTER to continue or x to exit:\n");
			if (*getterm()=='x')  {
				printf("Disassembly terminated.\n");
				return 0;
			}
			printf("\n");
		}

		/* Display the current address */
 		if (virt) {
			printf("%08x  ",addr);
		}
		else {
			printf("R %06x  ",addr);
		}
		/* set sj_inst to opcode at current address */
		if (!get_from_memory(addr,virt,&sj_inst, sizeof(sj_inst))) {
			printf("Address not in memory\n ");
			return FALSE; 
		}
		/* xlate instruction to ascii string, ignore branch EA's */
		rc = decode_instruction(sj_inst, &disasm, addr, 
			(__power_pc() ? PPC : PWR),(__power_pc() ? PPC : PWR)); 

		if (!rc)
			printf(" %08x: invalid instruction encountered\n",
					sj_inst);
		else
			/* print out the opcode and the mnemonic */
			printf(" %08x: %8s   %s\n", sj_inst, disasm.mnemonic,
						disasm.operands);

	}
	return TRUE;
}

