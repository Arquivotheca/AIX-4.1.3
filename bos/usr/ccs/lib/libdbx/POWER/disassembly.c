static char sccsid[] = "@(#)28    1.9  src/bos/usr/ccs/lib/libdbx/POWER/disassembly.c, libdbx, bos41J, 9518A_all 5/2/95 13:24:08";
/*
 *   COMPONENT_NAME: CMDDBX
 *
 *   FUNCTIONS: add_operand
 *		build_branch_op
 *		decode_Aform
 *		decode_Bform
 *		decode_DSform
 *		decode_Dform
 *		decode_Iform
 *		decode_Mform
 *		decode_SCform
 *		decode_XFLform
 *		decode_XFXform
 *		decode_XLform
 *		decode_XOform
 *		decode_XSform
 *		decode_Xform
 *		decode_instruction
 *		decode_opcode30
 *		decode_opcode31
 *		decode_opcode59
 *		decode_opcode63
 *		
 *
 *   ORIGINS: 26,27
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Copyright (c) 1982 Regents of the University of California
 */

#ifndef _KERNEL
#include <stdlib.h>
#else
#define NULL 0
#endif
#include "disassembly.h"
#include "ops.h"

#ifdef KDBX
extern int gen_reg_number [3]; 
extern int gen_reg_index;
#endif


/*
 * SUBCOMPONENT DESIGN PROLOG:
 *
 * SUBCOMPONENT NAME: Disassemble valid instruction sets
 *
 * FILES:
 *	disassembly.c	- This file was created from the old decode.c which
 *			  existed under dbx.  Relevant pieces were extracted
 *			  and moved into this new file.
 *	disassembly.h
 *	ops.c
 *	ops.h
 *
 * EXTERNAL FUNCTIONS:
 *	decode_instruction	- Called to create structure of disassembly
 *				  information for specified instruction
 *	SPR_name		- Returns common special purpose register name
 *	TO_ext			- Returns common trap options   
 *
 * EXTERNAL DATA:
 *	opcode_to_form	- Table maps opcodes to instruction form and valid
 *			  instruction set
 *	opcode30	- Table maps instructions of opcode 30 to description
 *	opcode31	- Table maps instructions of opcode 31 to description
 *	opcode63	- Table maps instructions of opcode 63 to description
 *	Aform		- Table maps A form instructions to description
 *	DSform		- Table maps DS form instructions to description
 *	opcode59	- Table maps instructions of opcode 59 to description
 *
 * DEPENDENCIES: NONE
 *
 * FUNCTION: Disassembles instructions based on specified instruction set and
 *	mnemonic set.  This code is used for dbx, adb, and crash.
 *
 * NOTES:
 *	decode_instruction() is the entry point for having an instruction
 *		disassembled.  This function is called with the specific
 *		instruction set and mnemonic set for disassembling.  This
 *		function extracts the opcode portion of the instruction and
 *		looks for it in the table opcode_to_form.  Then calls one of the
 *		decode_* functions to complete the disassembly.
 *
 *	Each of the decode_* functions map to an instruction form, documented in
 *	the hardware architecture document.  The tables in ops.c describe how to
 *	disassemble an instruction based on its opcode, and if necessary its
 *	extended.
 *
 * DATA STRUCTURES: NONE
 */

typedef enum { false, true } boolean;
typedef unsigned int Address;


/*
 * NAME: add_operand
 *
 * FUNCTION: Builds structure to indicate what operand is present
 *
 * NOTES: Fills in disassembly structure with information for next operand.
 *	Increments number_operands to indicate another operand has been added.
 *
 * PARAMETERS:
 *	disassembly	- Structure to add operand information into
 *	value		- Value associated with current operand
 *	op_type		- Type associated with current operand
 *
 * DATA STRUCUTURES: NONE
 *
 * RETURNS: NONE
 */
static void add_operand(inst *disassembly, unsigned value, 
                        operand_type op_type)
{
    disassembly->op[disassembly->number_operands].op_type = op_type;
    disassembly->op[disassembly->number_operands].cons_val.uvalue = value;
    (disassembly->number_operands)++;
}


/*
 * NAME: decode_Dform
 *
 * FUNCTION: Decode the D instruction format
 *
 * PARAMETERS:
 *	instruction		- Hex representation of instruction to
 *				  disassemble
 *	opcode			- Opcode part of instruction to disassemble
 *	current_instruction_set	- Instruction set to use for disassembling
 *	current_mnemonic_set	- Mnemonic set to use for disassembling
 *	disassembly		- Structure to be filled in with disassembled
 *				  instruction
 *
 * DATA STRUCTURES:
 *	Dform	- Contains information about D form instructions
 *
 * RETURNS: Returns 0 if the instruction is not valid for the specified
 *	instruction set.  Returns 1 if the instruction is successfully
 *	disassembled.
 */
static int decode_Dform(unsigned long	instruction,
                        unsigned long	opcode,
                        short		current_instruction_set,
                        short		current_mnemonic_set,
                        inst		*disassembly)
{
    unsigned long rt, RA, TO, BF, FRT,ufield;	/* Match D form fields */
    unsigned long L;				/* Match D form fields */
    long sfield;				/* Match D form fields */
    struct opcode_tab opcode_info;
    char *mnemonic;
    char *common_opt;
    short length, rc = 1;

    opcode_info = Dform[opcode];

    if( ! (current_instruction_set & opcode_info.valid_inst_set ) ) {
	/*
	 * This instruction is not valid for the specified instruction set
	 */
	rc = 0;
	disassembly->target_addr = opcode_info.valid_inst_set;
    } else {
	/*
	 * Fill in the mnemonic for this instruction
	 */
	if( current_mnemonic_set == PPC && opcode_info.alt_mnem != NULL ) {
	    strcpy(disassembly->mnemonic, opcode_info.alt_mnem);
	} else {
	    strcpy(disassembly->mnemonic, opcode_info.mnemonic);
	}

	/*
	 * Initialize number of operands
	 */
	disassembly->number_operands = 0;

	rt = TO = FRT = bits(instruction, 6, 10);
	RA = bits(instruction, 11, 15);
	BF = bits(instruction,6,8);

	/*
	 * ufield and sfield gets the bits 16-31
	 *
	 * ufield is the unsigned interpretation, and sfield is the signed
	 * interpretation.  The right shift of sfield causes the sign bit to be
	 * shifted in.
	 */
	ufield = instruction & 0xffff;
	sfield = ((long)instruction << 16) >> 16;

	switch(opcode_info.format) {
	    case 0:
		add_operand( disassembly, rt, gen_register );
		if (opcode != 15) {
		    add_operand( disassembly, RA, gen_register );
		    add_operand( disassembly, sfield, constant );
		} else {
		    /*
		     * Instruction cau or extended mnemonic liu/lis ?
		     */
		    if (RA != 0) {
			add_operand( disassembly, RA, gen_register );
		    } else {
			if( current_mnemonic_set == PPC ) {
			    strcpy(disassembly->mnemonic, "lis");
			} else {
			    strcpy(disassembly->mnemonic, "liu");
			}
		    }
		    add_operand( disassembly, ufield, uconstant );
		}
		break;
	    case 1:
		add_operand( disassembly, RA, gen_register);
		add_operand( disassembly, rt, gen_register);
		add_operand( disassembly, ufield, uconstant);
		break;
	    case 2:
		add_operand( disassembly, rt, gen_register );
		if ((opcode == 14) && (RA == 0)) {
		    /*
		     * Extended menmonic lil/li
		     */
		    if( current_mnemonic_set == PPC ) {
			strcpy(disassembly->mnemonic, "li");
		    } else {
			strcpy(disassembly->mnemonic, "lil");
		    }
		    add_operand( disassembly, sfield, constant );
		} else {
		    add_operand( disassembly, sfield, offset );
		    add_operand( disassembly, RA, gen_register );
		}
		break;
	    case 3: /* Trap immediate */
		common_opt = TO_ext(TO);
		if (common_opt) {
		    /*
		     * The 'i' part of the name will get added to the end
		     */
		    length = strlen(disassembly->mnemonic) - 1;
		    strcpy(&(disassembly->mnemonic)[length], common_opt);
		    strcat(disassembly->mnemonic,"i");
		} else {
		    add_operand( disassembly, TO, constant );
		}
		add_operand( disassembly, RA, gen_register );
		add_operand( disassembly, sfield, constant );
		break;
	    case 4:
		add_operand( disassembly, BF, cond_reg_field );
		if( current_instruction_set & PPC ) {
		    L = bits( instruction, 10,10 );
		    add_operand( disassembly, L, uconstant );
		}
		add_operand( disassembly, RA, gen_register );
		if (opcode == 11) {
		    /*
		     * Instruction cmpi
		     */
		    add_operand( disassembly, sfield, constant );
		} else {
		    add_operand( disassembly, ufield, uconstant );
		}
		break;
	    case 5:
		add_operand( disassembly, FRT, float_register );
		add_operand( disassembly, sfield, offset );
		add_operand( disassembly, RA, gen_register );
		break;
	    default:
		rc = 0;
		break;
	}
    }
    return( rc );
}


/*
 * NAME: build_branch_op
 *
 * FUNCTION: Construct an assembler-like branch instruction
 *
 * PARAMETERS:
 *	mnemonic		- Will be filled in with the branch mnemonic
 *	br_opt			- BO field of branch instruction
 *	cr_bit			- Condition register bit
 *	update_link		- LK field of branch instruction
 *	absolute		- AA field of branch instruction
 *	ext_op			- Extended opcode of instruction
 *	current_mnemonic_set	- mnemonic set to use for disassembly
 *
 * DATA STRUCUTURES: NONE
 *
 * NOTES:
 *	Extended mnemonics disassembled:
 *		bXX, bXXl, bXXa, bXXla, bdzXX, bdnXX, bXXr, bXXrl, bXXc, bXXcl:
 *			where XX is one of lt, gt, eq, so, ge, le, ne, ns
 *		bdz, bdn, bdzl, bdnl, bdza, bdna, bdzla, bdnla, bdzr, bdnr,
 *		bdzrl, bdnrl, br, bctr
 *
 * RETURNS: NONE
 */
static void build_branch_op(char *mnemonic, 
                            unsigned short br_opt, 
                            unsigned short cr_bit, 
                            unsigned short update_link, 
                            unsigned short absolute, 
                            unsigned short ext_op,
                            short current_mnemonic_set)
{
  char *c = mnemonic;	
  boolean uncond = false;	/* Unconditional br to count reg */
  boolean pos_cond = false;	/* Branch if condition is positive */
  boolean ctr_zero = false;	/* Branch if count register = 0 */
  boolean dec_ctr = false;	/* Decrement count register */

  *c++ = 'b';
  if (br_opt & 4) {
	/*
	 * Don't decrement count register
	 */
	if (br_opt & 16) {
		uncond = true;
	}
	if (br_opt & 8) {
		pos_cond = true;
        } 
  } else {
	/*
	 * Decrement count register
	 */	
	dec_ctr = true;
	if (br_opt & 2) {
		ctr_zero = true;
	}
	if (br_opt & 8) {
		pos_cond = true;
        } 
  }
  if (dec_ctr) {
	*c++ = 'd';
	*c++ = (ctr_zero) ? 'z' : 'n';
  }
  if (!uncond) {
	if (pos_cond) {
		switch(cr_bit) {
			case CR_LT:  *c++ = 'l'; *c++ = 't'; break;
			case CR_GT:  *c++ = 'g'; *c++ = 't'; break;
			case CR_EQ:  *c++ = 'e'; *c++ = 'q'; break;
			case CR_SO:  *c++ = 's'; *c++ = 'o'; break;
		}
	} else {
		switch(cr_bit) {
			case CR_LT:  *c++ = 'g'; *c++ = 'e'; break;
			case CR_GT:  *c++ = 'l'; *c++ = 'e'; break;
			case CR_EQ:  *c++ = 'n'; *c++ = 'e'; break;
			case CR_SO:  *c++ = 'n'; *c++ = 's'; break;
		}
	}
  }
  if (ext_op == 16) {
	if( current_mnemonic_set == PPC ) {
	    *c++ = 'l';
	}
	*c++ = 'r';
  } else if (ext_op == 528)  {
	*c++ = 'c';
	if( uncond || current_mnemonic_set == PPC ) {
	    /* Can't confuse with br conditional */
	    *c++ = 't';
	    *c++ = 'r';
	}
  }
  if (update_link)
	*c++ = 'l';
  if (absolute)
	*c++ = 'a';
  *c = '\0';
}


/*
 * NAME: decode_Bform
 *
 * FUNCTION: Decode the B instruction format
 *
 * PARAMETERS:
 *	addr			- Address of instruction to disassemble
 *	instruction		- Hex form of instruction to disassemble
 *	opcode			- Opcode of instruction to disassemble
 *	current_mnemonic_set	- mnemonic set to use for disassembly
 *	disassembly		- Structure to be filled in with disassembly
 *				  information
 *
 * DATA STRUCUTURES: NONE
 *
 * RETURNS: Returns 1 always.  Assumes not called if instruction is not valid.
 */
static int decode_Bform(Address		addr,
                        unsigned long	instruction,
                        unsigned long	opcode,
                        short		current_mnemonic_set,
                        inst           *disassembly)
{
   unsigned short AA, LK, BO, BI; 	/* Match B form fields */
   char cr_field;
   long target;

   AA = bits(instruction,30,30);
   LK = bits(instruction,31,31);
   BO = bits(instruction,6,10);
   BI = bits(instruction,11,15);

   /*
    * Get the target address for the branch instruction
    * Since this is cast to long, the right shift will shift in the sign bit
    */
   target = ((long)((instruction) & 0xfffc) << 16) >> 16;
   if (!AA)
	target += addr;
   disassembly->target_addr = target;

   /*
    * Get the extended mnemonic for the branch instruction
    * The operation on BI converts it to match the CR_ values; this is necessary
    * since we are only interested in the last two bits of this field; the other
    * bits are used for floating point only
    */
   build_branch_op(disassembly->mnemonic, BO, 1 << (3 - (BI&3)), LK, AA, 0,
		   current_mnemonic_set);

   /*
    * Build disassembly without target, added on later...
    */
   cr_field = (char) (BI>>2);
   if (cr_field) {	/* Not CR 0 ? */
      add_operand( disassembly, cr_field, cond_reg_field );
   }
   return( 1 );
}


/*
 * NAME: decode_Iform
 *
 * FUNCTION: Decode the I instruction format
 *
 * PARAMETERS:
 *	addr		- Address of instruction to disassemble
 *	instruction	- hex form of instruction to disassemble
 *	opcode		- Opcode of instruction to disassemble
 *	disassembly	- Structure to be filled in with disassembly information
 *
 * DATA STRUCUTURES: NONE
 *
 * RETURNS: Returns 1 always.  Assumes not called if instruction is not valid.
 */
static int decode_Iform(unsigned long	addr,
                        unsigned long	instruction,
                        unsigned long	opcode,
                        inst           *disassembly)
{
   long target;
   unsigned long AA, LK; 	/* Match I form fields */

   AA = bits(instruction,30,30);
   LK = bits(instruction,31,31);
   /*
    * Since this is cast to long, the right shift will shift in the sign bit
    */
   target = ((long)((instruction) & ~3) << 6) >> 6;
   if (AA) {
	strcpy( disassembly->mnemonic, (LK) ? "bla" : "ba");
   } else {
	target += addr;
	strcpy( disassembly->mnemonic, (LK) ? "bl" : "b");
   }
   disassembly->target_addr = target;
   return( 1 );
}


/*
 * NAME: decode_SCform
 *
 * FUNCTION: Decode the SC instruction format
 *
 * PARAMETERS:
 *	instruction		- hex form of instruction to disassemble
 *	opcode			- opcode of instruction to disassemble
 *	current_instruction_set	- instruction set to use for disassembly
 *	current_mnemonic_set	- mnemonic set to use for disassembly
 *	disassembly		- structure to be filled in with disassembly
 *				  information
 *
 * DATA STRUCUTURES: NONE
 *
 * RETURNS: Returns 0 if the instruction is not valid for the specified
 *	instruction set.  Returns 1 if the instruction is successfully
 *	disassembled.
 */
static int decode_SCform(unsigned long	instruction,
                         unsigned long	opcode,
                         short		current_instruction_set,
                         short		current_mnemonic_set,
                         inst          *disassembly)
{
   unsigned short SA, LK, LEV, FL1, FL2, SV; 	/* Match SC form fields */
   short rc = 1;

   SA = bits(instruction,30,30);
   LK = bits(instruction,31,31);
   if (SA) {
	SV = bits(instruction,16,29);
	if( current_mnemonic_set == PWR ) {
	    strcpy( disassembly->mnemonic, (LK) ? "svcla" : "svca");
	} else {
	    strcpy( disassembly->mnemonic, (LK) ? "scl" : "sc");
	}
	add_operand( disassembly, SV, uconstant );
   } else {
	if( current_instruction_set & PPC || current_instruction_set & COM ) {
	    /*
	     * This form is not valid for PowerPC or common mode
	     */
	    rc = 0;
	    disassembly->target_addr = ALL_SETS & !PPC & !COM;
	} else {
	    LEV = bits(instruction,20,26);
	    FL1 = bits(instruction,16,19);
	    FL2 = bits(instruction,27,29);
	    strcpy( disassembly->mnemonic, (LK) ? "svcl" : "svc" );
	    add_operand( disassembly, LEV, uconstant );
	    add_operand( disassembly, FL1, uconstant );
	    add_operand( disassembly, FL2, uconstant );
	}
   }
   return( rc );
}


/*
 * NAME: decode_XLform
 *
 * FUNCTION: Decode the XL instruction format
 *
 * PARAMETERS:
 *	instruction		- hex form of instruction to disassemble
 *	opcode			- opcode of instruction to disassemble
 *	current_instruction_set	- instruction set to use for disassembling
 *	current_mnemonic_set	- mnemonic set to use for disassembling
 *	disassembly		- structure to be filled in with disassembly
 *				  information
 *
 * DATA STRUCTURES:
 *	XLform	- table of XL form instructions
 *
 * RETURNS: Returns 0 if the instruction is not valid for the specified
 *	instruction set.  Returns 1 if the instruction is successfully
 *	disassembled.
 */
static int decode_XLform(unsigned long	instruction,
                         unsigned long	opcode,
                         short		current_instruction_set,
                         short		current_mnemonic_set,
                         inst          *disassembly)
{
   unsigned long ext_opcode;
   unsigned short LK, BO,  BI, BB; 	/* Match XL form fields */
   unsigned short BF, BFA, BT, BA; 	/* Match XL form fields */
   struct   opcode_tab opcode_info;
   char cr_field;
   short rc = 1;

   ext_opcode = bits(instruction,21,30);
   opcode_info = XLform[ext_opcode >> 4]; /* shift to get XL table index */

   if( ! (current_instruction_set & opcode_info.valid_inst_set ) ) {
     /*
      * This instruction is not valid for this instruction set
      */
     rc = 0;
     disassembly->target_addr = opcode_info.valid_inst_set;
   } else {
	if( current_mnemonic_set == PPC && opcode_info.alt_mnem != NULL ) {
	    strcpy( disassembly->mnemonic, opcode_info.alt_mnem);
	} else {
	    strcpy( disassembly->mnemonic, opcode_info.mnemonic);
	}
	switch(opcode_info.format) {
	    case 0:
		/*
		 * No operands for this format
		 */
		break;
	    case 1:
		BO = bits(instruction,6,10);
		BI = bits(instruction,11,15);
		LK  = bits(instruction,31,31);
		/*
		 * The operation on BI converts it to match the CR_ values; this
		 * is necessary since we are only interested in the last two
		 * bits of this field; the other bits are used for floating
		 * point only
		 */
		build_branch_op(disassembly->mnemonic, BO, 1 << (3 - (BI&3)),
				LK, 0, ext_opcode, current_mnemonic_set);
		cr_field = (char) (BI>>2);
		if (cr_field) {	/* Not CR 0 ? */
		    add_operand( disassembly, cr_field, cond_reg_field );
		}
		break;
	    case 2:
		BF  = bits(instruction,6,8);
		BFA = bits(instruction,11,13);
		add_operand( disassembly, BF, cond_reg_field );
		add_operand( disassembly, BFA, cond_reg_field );
		break;
	    case 3:
		BT = bits(instruction,6,10);
		BA = bits(instruction,11,15);
		BB = bits(instruction,16,20);
		add_operand( disassembly, BT, cond_reg_field );
		add_operand( disassembly, BA, cond_reg_field );
		add_operand( disassembly, BB, cond_reg_field );
		break;
	    default:
		rc = 0;
		break;
	}
   }
   return( rc );
}


/*
 * NAME: decode_Mform
 *
 * FUNCTION: Decode the M instruction format
 *
 * PARAMETERS:
 *	instruction		- hex form of instruction to disassemble.
 *	opcode			- opcode of instruction to disassemble.
 *	current_mnemonic_set	- mnemonic set to use for disassembly
 *	disassembly		- structure to be filled in with disassembly
 *				  information
 *
 * DATA STRUCTURES:
 *	Mforms		- Contains valid mnemonics for this form for PWR
 *	Mforms_ppc	- Contains valid mnemonics for this form for PPC
 *
 * RETURNS: Returns 0 if the instruction is not valid for the specified
 *	instruction set.  Returns 1 if the instruction is successfully
 *	disassembled.
 */
static char *Mforms[]  = { "rlimi", "rlimi.", "rlinm", "rlinm.",
			   "rlmi", "rlmi.", "rlnm", "rlnm."};

static char *Mforms_ppc[]  = { "rlwimi", "rlwimi.", "rlwinm", "rlwinm.",
			       "rlmi", "rlmi.", "rlwnm", "rlwnm."};


static int decode_Mform(unsigned long	instruction,
                        unsigned long	opcode,
                        short		current_mnemonic_set,
                        inst           *disassembly)
{
   unsigned short RS, RA, RB, SH;	/* Match M form fields */
   unsigned short MB, ME, Rc;		/* Match M form fields */
   unsigned short SH_RB;
   char *asm_mnemonic;
   short rc = 1;

   RS = bits(instruction,6,10);
   RA = bits(instruction,11,15);
   SH_RB = RB = SH = bits(instruction,16,20);
   MB = bits(instruction,21,25);
   ME = bits(instruction,26,30);
   Rc = instruction & 1;

   if( current_mnemonic_set == PPC ) {
	asm_mnemonic = Mforms_ppc[(opcode - 20) * 2 + Rc];
   } else {
	asm_mnemonic = Mforms[(opcode - 20) * 2 + Rc];
   }
   strcpy( disassembly->mnemonic, asm_mnemonic );

   switch( opcode ) {
	case 20:
	    add_operand( disassembly, RA, gen_register );
	    add_operand( disassembly, RS, gen_register );
	    add_operand( disassembly, SH_RB, uconstant );
	    add_operand( disassembly, MB, uconstant );
	    add_operand( disassembly, ME, uconstant );
	    break;
	case 21: /* sri and sli are special forms of rlinm */
	    add_operand( disassembly, RA, gen_register );
	    add_operand( disassembly, RS, gen_register );
	    if ((ME == 31) && (MB == (32-SH_RB))) {
		strcpy( disassembly->mnemonic, (Rc) ? "sri." : "sri" );
		add_operand( disassembly, MB, uconstant );
	    } else if ((MB == 0) && (SH_RB == (31-ME))) {
		strcpy( disassembly->mnemonic, (Rc) ? "sli." : "sli" );
		add_operand( disassembly, SH_RB, uconstant );
	    } else {
		add_operand( disassembly, SH_RB, uconstant );
		add_operand( disassembly, MB, uconstant );
		add_operand( disassembly, ME, uconstant );
	    }
	    break;
	case 22:
	case 23:
	    add_operand( disassembly, RA, gen_register );
	    add_operand( disassembly, RS, gen_register );
	    add_operand( disassembly, RB, gen_register );
	    add_operand( disassembly, MB, uconstant );
	    add_operand( disassembly, ME, uconstant );
	    break;
	default:
	    rc = 0;
	    break;
   }
   return( rc );
}


/*
 * NAME: decode_Xform
 *
 * FUNCTION: Decode the X instruction format
 *
 * PARAMETERS:
 *	instruction		- hex form of instruction to disassemble
 *	format			- format of instruction to disassemble
 *	ext_op			- Extended opcode of instruction to disassemble
 *	current_instruction_set	- instruction set to use for disassembly
 *	disassembly		- structure to be filled in with disassembly
 *				  information
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: Returns 0 if the instruction is not valid for the specified
 *	instruction set.  Returns 1 if the instruction is successfully
 *	disassembled.
 */
static int decode_Xform(unsigned long	instruction,
                        int		format,
                        unsigned long	ext_op,
                        short		current_instruction_set,
                        inst           *disassembly)
{
  unsigned short int rt,RA,RB,NB,SH,SPR,FRS,FRT;       /* Match X form fields */
  unsigned short int BF,BFA,I,BT,SR,FRA,FRB,TO;	       /* Match X form fields */
  unsigned short int L,SPR_FULL;			/* Match X form field */
  char *common_opt;
  short length;
  short rc = 1;
  spr_table *special_reg;
  int i, reg_number;

  FRS = FRT = TO = BT = rt = bits(instruction,6,10);
  FRB = NB  = SH = RB = bits(instruction,16,20);
  FRA = SR = RA = bits(instruction,11,15);
  SPR = bits(instruction,11,15);
  SPR_FULL = SPR | (bits(instruction,16,20) << 5);
  BFA = bits(instruction,11,13);
  I = bits(instruction,16,19);
  BF = bits(instruction,6,8);

  switch(format) 
  {
     case 0:
	add_operand( disassembly, RA, gen_register );
	add_operand( disassembly, rt, gen_register );
	  break;
     case 1:
	  /*
	   * This format has no operands
	   */
	  break;
     case 2:
	add_operand( disassembly, rt, gen_register );
	  break;
     case 3:
	if( (current_instruction_set & PWR) || SPR <= 9 ) {
	    reg_number = SPR;
	} else {
	    reg_number = SPR_FULL;
	}

        for (i = 0, special_reg = NULL;
             (i < SPR_SZ) && (special_reg == NULL); i++)
        {
          if (SPR_name[i].reg_number == reg_number)
            special_reg = &SPR_name[i];
        }

	if( special_reg != NULL &&
	    special_reg->valid_inst_set & current_instruction_set ) {
	    if( current_instruction_set & PPC &&
		special_reg->alt_reg_name != NULL ) {
		strcpy(&(disassembly->mnemonic)[2], special_reg->alt_reg_name);
	    } else {
		strcpy(&(disassembly->mnemonic)[2], special_reg->reg_name);
	    }
	    add_operand( disassembly, rt, gen_register );
	} else if( ext_op == 678 ) {	/* reserved register */
	    /*
	     * mfspr has a different format than mtspr
	     */
	    add_operand( disassembly, rt, gen_register );
	    add_operand( disassembly, reg_number, special_register );
	} else {
	    add_operand( disassembly, reg_number, special_register );
	    add_operand( disassembly, rt, gen_register );
	}
	break;
     case 4:
	add_operand( disassembly, rt, gen_register );
	add_operand( disassembly, RB, gen_register );
	break;
     case 5:
	add_operand( disassembly, rt, gen_register );
	add_operand( disassembly, RA, gen_register );
	break;
     case 6:
	add_operand( disassembly, RA, gen_register );
	add_operand( disassembly, RB, gen_register );
	break;
     case 7:
	add_operand( disassembly, rt, gen_register );
	add_operand( disassembly, RA, gen_register );
	add_operand( disassembly, RB, gen_register );
	break;
     case 8:
	if ((ext_op == 888) || (ext_op == 889)) {
	    if (rt == RB) {
		strcpy( disassembly->mnemonic, (ext_op == 889) ? "mr." : "mr" );
		add_operand( disassembly, RA, gen_register );
		add_operand( disassembly, rt, gen_register );
		break;
	    }
	}
	add_operand( disassembly, RA, gen_register );
	add_operand( disassembly, rt, gen_register );
	add_operand( disassembly, RB, gen_register );
	break;
     case 9:
	add_operand( disassembly, RA, gen_register );
	add_operand( disassembly, rt, gen_register );
	break;
     case 10:
	add_operand( disassembly, rt, gen_register );
	add_operand( disassembly, RA, gen_register );
	add_operand( disassembly, NB, uconstant );
	break;
     case 11:
	add_operand( disassembly, RA, gen_register );
	add_operand( disassembly, rt, gen_register );
	add_operand( disassembly, SH, uconstant );
	break;
     case 12:
	add_operand( disassembly, FRS, float_register );
	add_operand( disassembly, RA, gen_register );
	add_operand( disassembly, RB, gen_register );
	break;
     case 13:
	add_operand( disassembly, FRT, float_register );
	break;
     case 14:
	add_operand( disassembly, BF, cond_reg_field );
	add_operand( disassembly, BFA, uconstant );
	break;
     case 15:
	add_operand( disassembly, BF, cond_reg_field );
	add_operand( disassembly, I, uconstant );
	break;
     case 16:
	add_operand( disassembly, BT, uconstant );
	break;
     case 17:
	add_operand( disassembly, SR, uconstant );
	add_operand( disassembly, rt, gen_register );
	break;
     case 18:
	add_operand( disassembly, rt, gen_register );
	add_operand( disassembly, SR, uconstant );
	break;
     case 19:
	add_operand( disassembly, BF, cond_reg_field );
	add_operand( disassembly, FRA, float_register );
	add_operand( disassembly, FRB, float_register );
	break;
     case 20:
	add_operand( disassembly, BF, cond_reg_field );
	add_operand( disassembly, FRB, float_register );
	break;
     case 21:
	add_operand( disassembly, FRT, float_register );
	add_operand( disassembly, FRB, float_register );
	break;
     case 22:
	add_operand( disassembly, BF, cond_reg_field );
	if( current_instruction_set & PPC ) {
	    L = bits( instruction, 10, 10);
	    add_operand( disassembly, L, uconstant );
	}
	add_operand( disassembly, RA, gen_register );
	add_operand( disassembly, RB, gen_register );
	break;
     case 23:
	add_operand( disassembly, BF, cond_reg_field );
	break;
     case 24:
	common_opt = TO_ext(TO);
	if (common_opt) {
	    length = strlen(disassembly->mnemonic);
	    strcpy(&(disassembly->mnemonic)[length],common_opt );
	} else {
	    add_operand( disassembly, TO, constant );
	}
	add_operand( disassembly, RA, gen_register );
	add_operand( disassembly, RB, gen_register );
	break;
     case 26:
	add_operand( disassembly, RB, gen_register );
	break;
     default:
	rc = 0;
	break;
  }
  return( rc );
}


/*
 * NAME: decode_XFXform
 *
 * FUNCTION: Decode the XFX instruction format
 *
 * PARAMETERS:
 *	instruction		- hex form of instruction to disassemble
 *	ext_op			- extended opcode of instruction to disassemble
 *	disassembly		- structure to be filled in with disassembly
 *				  information
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: Returns 0 if the instruction is not valid for the specified
 *	instruction set.  Returns 1 if the instruction is successfully
 *	disassembled.
 */
static int decode_XFXform(unsigned long   instruction,
                          unsigned short  ext_op,
                          inst           *disassembly)
{
   unsigned long rt,FXM, tbr;	/* Match XFX form fields */
   short rc = 1;

   rt = bits(instruction,6,10);
   FXM = bits(instruction,12,19);
   tbr = bits(instruction,11,20);
   if( ext_op == 288 ) {
	if (FXM == 0xff) {
	    strcpy( disassembly->mnemonic, "mtcr" );
	} else {
	    strcpy( disassembly->mnemonic, "mtcrf" );
	    add_operand( disassembly, FXM, uconstant );
	}
	add_operand( disassembly, rt, gen_register );
   } else {
	if( tbr == 0x188 ) {
	    strcpy( disassembly->mnemonic, "mftb" );
	    add_operand( disassembly, rt, gen_register );
	} else if( tbr == 0x1a8 ) {
	    strcpy( disassembly->mnemonic, "mftbu" );
	    add_operand( disassembly, rt, gen_register );
	} else {
	    rc = 0;
	}
   }
   return( rc );
}

/*
 * NAME: decode_XOform
 *
 * FUNCTION: Decode the XO instruction format
 *
 * PARAMETERS:
 *	instruction	- hex form of instruction to disassemble
 *	format		- format of instruction to disassemble
 *	disassembly	- structure to be filled in with disassembly information
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: Returns 0 if the instruction is not valid for the specified
 *	instruction set.  Returns 1 if the instruction is successfully
 *	disassembled.
 */
static int decode_XOform(unsigned long	instruction,
                         int		format,
                         inst          *disassembly)
{
   unsigned long rt,RA,RB;	/* Match XO form fields */
   short rc = 1;

   rt = bits(instruction,6,10);
   RA = bits(instruction,11,15);

   switch(format)
   {
      case 0:
	add_operand( disassembly, rt, gen_register );
	add_operand( disassembly, RA, gen_register );
        break;
      case 1:
        RB = bits(instruction,16,20);
	add_operand( disassembly, rt, gen_register );
	add_operand( disassembly, RA, gen_register );
	add_operand( disassembly, RB, gen_register );
        break;
      default:
	rc = 0;
        break;
   }
   return( rc );
}

/*
 * NAME: decode_XSform
 *
 * FUNCTION: Decode the XS instruction format
 *
 * PARAMETERS:
 *	instruction	- hex form of instruction to disassemble
 *	format		- format of instruction to disassemble
 *	disassembly	- structure to be filled in with disassembly information
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: Returns 0 if the instruction is not valid for the specified
 *	instruction set.  Returns 1 if the instruction is successfully
 *	disassembled.
 */
static int decode_XSform(unsigned long instruction, 
                         int           format, 
                         inst         *disassembly)
{
   unsigned long RA,RS, SH;	/* Match XS form fields */
   short rc = 1;

   RS = bits(instruction,6,10);
   RA = bits(instruction,11,15);

   /*  make bit 30 the high order bit of the SH field  */
   SH = bits(instruction, 16, 20) | (bits(instruction, 30, 30) << 5);

   switch(format)
   {
      case 0:
	add_operand( disassembly, RA, gen_register );
	add_operand( disassembly, RS, gen_register );
	add_operand( disassembly, SH, uconstant );
        break;
      default:
	rc = 0;
        break;
   }
   return( rc );
}

/*
 * NAME: decode_opcode30
 *
 * FUNCTION: Decode instructions with opcode of 30
 *
 * PARAMETERS:
 *	instruction		- hex form of instruction to disassemble
 *	current_instruction_set	- instruction set to use for disassembly
 *	current_mnemonic_set	- mnemonic set to use for disassembly
 *	disassembly		- structure to be filled in with disassembly
 *				  information
 *
 * DATA STRUCTURES:
 *	opcode30	- Table which contains information of instructions with
 *			  opcode of 30.
 *
 * RETURNS: Returns 0 if the instruction is not valid for the specified
 *	instruction set.  Returns 1 if the instruction is successfully
 *	disassembled.
 */
static int decode_opcode30(unsigned long instruction, 
                           short         current_instruction_set,
			   short         current_mnemonic_set, 
                           inst         *disassembly)
{
  unsigned long RA, RS, RB, SH, MB;
  struct opcodeXX *search_results;
  int format;
  short rc = 1;
  int i, key;

  /*
   * Set search_results to entry in opcode30 table for this instruction
   */

  key = bits(instruction,27,31);
  for (i = 0, search_results = NULL;
       (i < OPCODE30_SZ) && (search_results == NULL); i++)
  {
    if (opcode30[i].key == key)
      search_results = &opcode30[i];
  }

  if( search_results == NULL ||
	! (current_instruction_set & search_results->valid_inst_set ) ) {
	/*
	 * This instruction is not valid with our current instruction set
	 */
	rc = 0;
	disassembly->target_addr = search_results->valid_inst_set;
  }
  else 
  {
    strcpy( disassembly->mnemonic, search_results->mnemonic );
  }

  RS = bits(instruction, 6, 10);
  RA = bits(instruction, 11, 15);
  MB = bits(instruction, 21, 26);

  switch(search_results->format) 
  {
    case 0:
      /*  make bit 30 the high order bit of the SH field  */
      SH = bits(instruction, 16, 20) | (bits(instruction, 30, 30) << 5);

      add_operand(disassembly, RA, gen_register);
      add_operand(disassembly, RS, gen_register);
      add_operand(disassembly, SH, uconstant);
      add_operand(disassembly, MB, uconstant);
      break;

    case 1:
      /*  must be an MDS form instruction  */
      RB = bits(instruction, 16, 20);

      add_operand(disassembly, RA, gen_register);
      add_operand(disassembly, RS, gen_register);
      add_operand(disassembly, RB, gen_register);
      add_operand(disassembly, MB, uconstant);
      break;
  }

  return( rc );
}


/*
 * NAME: decode_opcode31
 *
 * FUNCTION: Decode instructions with opcode of 31
 *
 * PARAMETERS:
 *	instruction		- hex form of instruction to disassemble
 *	current_instruction_set	- instruction set to use for disassembly
 *	current_mnemonic_set	- mnemonic set to use for disassembly
 *	disassembly		- structure to be filled in with disassembly
 *				  information
 *
 * DATA STRUCTURES:
 *	opcode31	- Table which contains information of instructions with
 *			  opcode of 31.
 *
 * RETURNS: Returns 0 if the instruction is not valid for the specified
 *	instruction set.  Returns 1 if the instruction is successfully
 *	disassembled.
 */
static int decode_opcode31(unsigned long instruction,
                           short         current_instruction_set,
                           short         current_mnemonic_set,
                           inst	        *disassembly)
{
    struct opcodeXX *search_results;
    int format;
    short rc = 1;
    int i, key;

    /*
     * Set search_results to entry in opcode31 table for this instruction
     */
    key = bits(instruction,21,31);
    for (i = 0, search_results = NULL;
         (i < OPCODE31_SZ) && (search_results == NULL); i++)
    {
      if (opcode31[i].key == key)
        search_results = &opcode31[i];
    }

    if( search_results == NULL ||
	! (current_instruction_set & search_results->valid_inst_set ) ) {
	/*
	 * This instruction is not valid with our current instruction set
	 */
	rc = 0;
	disassembly->target_addr = search_results->valid_inst_set;
    } else {
	if( current_mnemonic_set == PPC && search_results->alt_mnem != NULL ) {
	    strcpy( disassembly->mnemonic, search_results->alt_mnem );
	} else {
	    strcpy( disassembly->mnemonic, search_results->mnemonic );
	}

	format = search_results->format;
	switch(search_results->form) 
	{
	    case 4:
		rc = decode_Xform(instruction, format, key,
				  current_instruction_set, disassembly );
		break;
	    case 6:
		rc = decode_XFXform(instruction, search_results->key,
				    disassembly);
		break;
	    case 8:
		rc = decode_XOform(instruction, format, disassembly);
		break;
            case 9:
                rc = decode_XSform(instruction, format, disassembly);
                break;
	    default:
		rc = 0;
		break;
	}
    }
    return( rc );
}


/*
 * NAME: decode_Aform
 *
 * FUNCTION: Decode the A instruction format
 *
 * PARAMETERS:
 *	instruction	- hex form of instruction to disassemble
 *	format		- format of instruction
 *	disassembly	- structure to be filled in with disassembly information
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: Returns 0 if the instruction is not valid for the specified
 *	instruction set.  Returns 1 if the instruction is successfully
 *	disassembled.
 */
static int decode_Aform(unsigned long	instruction,
                        int		format,
                        inst           *disassembly)
{
    unsigned short FRT,FRA,FRB,FRC;	/* Match A form fields */
    short rc = 1;

    FRT = bits(instruction, 6,10);
    FRA = bits(instruction,11,15);
    FRB = bits(instruction,16,20);
    FRC = bits(instruction,21,25);

    switch(format) 
    {
	case 0:
	    add_operand( disassembly, FRT, float_register );
	    add_operand( disassembly, FRA, float_register );
	    add_operand( disassembly, FRB, float_register );
	    break;
	case 1:
	    add_operand( disassembly, FRT, float_register );
	    add_operand( disassembly, FRA, float_register );
	    add_operand( disassembly, FRC, float_register );
	    break;
	case 2:
	    add_operand( disassembly, FRT, float_register );
	    add_operand( disassembly, FRA, float_register );
	    add_operand( disassembly, FRC, float_register );
	    add_operand( disassembly, FRB, float_register );
	    break;
	case 3:
	    add_operand( disassembly, FRT, float_register );
	    add_operand( disassembly, FRB, float_register );
	    break;
	default:
	    rc = 0;
	    break;
    }
    return( rc );
}


/*
 * NAME: decode_opcode59
 *
 * FUNCTION: Decode instructions with opcode of 59
 *
 * PARAMETERS:
 *	instruction		- hex form of instruction to disassemble
 *	current_instruction_set	- Instruction set to use for disassembling
 *	disassembly		- structure to be filled in with disassembly
 *				  information
 *
 * DATA STRUCTURES:
 *	opcode59	- Table describing instruction with opcode of 59
 *
 * RETURNS: Returns 0 if the instruction is not valid for the specified
 *	instruction set.  Returns 1 if the instruction is successfully
 *	disassembled.
 */
static int decode_opcode59(unsigned long  instruction,
                           short          current_instruction_set,
                           inst          *disassembly)
{
    struct opcodeXX *search_results;
    int format;
    short rc = 1;
    int i, key;

    key = bits(instruction,26,31);
    for (i = 0, search_results = NULL;
         (i < OPCODE59_SZ) && (search_results == NULL); i++)
    {
      if (opcode59[i].key == key)
        search_results = &opcode59[i];
    }

    if( search_results == NULL ||
	! (current_instruction_set & search_results->valid_inst_set ) ) {
	/*
	 * Not a valid instruction
	 */
	disassembly->target_addr = search_results->valid_inst_set;
	return( 0 );
    }

    strcpy( disassembly->mnemonic, search_results->mnemonic );
    format = search_results->format;
    switch(search_results->form) 
    {
	case 9:
	    rc = decode_Aform(instruction, format, disassembly);
	    break;
	default:
	    rc = 0;
	    break;
    }
    return( rc );
}


/*
 * NAME: decode_XFLform
 *
 * FUNCTION: Decode the XFL instruction format
 *
 * PARAMETERS:
 *	instruction	- hex form of instruction to disassemble
 *	disassembly	- structure to be filled in with disassembly information
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: Returns 1 always.  Assumes not called if instruction is not valid.
 */
static int decode_XFLform(unsigned long  instruction,
                          inst          *disassembly)
{
    unsigned short FLM, FRB, Rc;	/* Match XFL form fields */

    Rc  = bits(instruction,31,31);
    FLM = bits(instruction,7,14);
    FRB = bits(instruction,16,20);
    if (FLM == 0xff) {
	strcpy( disassembly->mnemonic, (Rc) ? "mtfs." : "mtfs" );
    } else {
	strcpy( disassembly->mnemonic, (Rc) ? "mtfsf." : "mtfsf" );
	add_operand( disassembly, FLM, uconstant );
    }
    add_operand( disassembly, FRB, float_register );
    return( 1 );
}


/*
 * NAME: decode_opcode63
 *
 * FUNCTION: Decode instructions with opcode of 63
 *
 * PARAMETERS:
 *	instruction		- hex form of instruction to disassemble
 *	current_instruction_set	- instruction set to use for disassembling
 *	current_mnemonic_set	- mnemonic set to use for disassembling
 *	disassembly		- structure to be filled in with disassembly
 *				  information
 *
 * DATA STRUCTURES:
 *	opcode63	- Table describing X form instructions of opcode 63
 *	Aform		- Table describing A form instructions of opcode 63
 *
 * RETURNS: Returns 0 if the instruction is not valid for the specified
 *	instruction set.  Returns 1 if the instruction is successfully
 *	disassembled.
 */
static int decode_opcode63(unsigned long instruction,
                           short         current_instruction_set,
                           short         current_mnemonic_set,
                           inst		*disassembly)
{
    struct opcodeXX *search_results;
    int format;
    short rc = 1;
    int i, key;


    key = bits(instruction,21,31);
    for (i = 0, search_results = NULL;
         (i < OPCODE63_SZ) && (search_results == NULL); i++)
    {
      if (opcode63[i].key == key)
        search_results = &opcode63[i];
    }

    if (search_results == NULL) 
    {
      key = bits(instruction,26,31);
      for (i = 0, search_results = NULL;
           (i < AFORM_SZ) && (search_results == NULL); i++)
      {
        if (Aform[i].key == key)
          search_results = &Aform[i];
      }
    }

    if( search_results == NULL ||
	! (current_instruction_set & search_results->valid_inst_set ) ) {
	rc = 0;
	disassembly->target_addr = search_results->valid_inst_set;
    } else {
	if( current_mnemonic_set == PPC && search_results->alt_mnem != NULL ) {
	    strcpy( disassembly->mnemonic, search_results->alt_mnem );
	} else {
	    strcpy( disassembly->mnemonic, search_results->mnemonic );
	}
	format = search_results->format;
	switch(search_results->form) {
	    case 4:
		rc = decode_Xform(instruction, format, key,
				  current_instruction_set, disassembly);
		break;
	    case 7:
		rc = decode_XFLform(instruction, disassembly);
		break;
	    case 9:
		rc = decode_Aform(instruction, format, disassembly);
		break;
	    default:
		rc = 0;
		break;
	}
    }
    return( rc );
}


/*
 * NAME: decode_DSform
 *
 * FUNCTION: Decode the DS instruction format
 *
 * PARAMETERS:
 *	instruction	- hex form of instruction to be disassembled
 *	opcode		- Opcode of instruction to be disassembled
 *	disassembly	- structure to be filled in with disassembly information
 *
 * DATA STRUCTURES:
 *	DSforms	- Table describing valid mnemonics for this form
 *
 * RETURNS: Returns 0 if the instruction is not valid for the specified
 *	instruction set.  Returns 1 if the instruction is successfully
 *	disassembled.
 */
static char *DSforms[] = { "lfq", "lfqu", "", "INVALID", "stfq",
			   "stfqu" };
static char *opcode58[] = {"ld", "ldu", "lwa", ""};
static char *opcode62[] = {"std", "stdu", "", ""};

static int decode_DSform(
unsigned long	instruction,
unsigned long	opcode,
inst		*disassembly)
{
    unsigned long rt, RA;	/* Match DS form fields */
    long sfield;
    short rc = 1;
    int extended_op;

    rt = bits(instruction, 6, 10);
    RA = bits(instruction, 11, 15);
    extended_op = bits(instruction, 30, 31);

    if ((opcode < 56) || (opcode > 62)) {
	rc = 0;
    } else {
        switch (opcode)
        {
          case 58 :
            sfield = ((long)instruction << 16) >> 18;
            strcpy (disassembly->mnemonic, opcode58[extended_op]);
	    add_operand( disassembly, rt, gen_register );
            break;

          case 62 :
            sfield = ((long)instruction << 16) >> 18;
            strcpy (disassembly->mnemonic, opcode62[extended_op]);
	    add_operand( disassembly, rt, gen_register );
            break;

          default :
            sfield = ((long)instruction << 16) >> 16;
            strcpy( disassembly->mnemonic, DSforms[opcode - 56] );
	    add_operand( disassembly, rt, float_register );
        }

	add_operand( disassembly, sfield, offset );
	add_operand( disassembly, RA, gen_register );
    }
    return( rc );
}

/*
 * NAME: decode_instruction
 *
 * FUNCTION: Returns disassembly information for specified instruction
 *
 * PARAMETERS:
 *	instruction	- hex form of instruction to disassemble
 *	disassembly	- structure to be filled in with disassembly information
 *	iar		- Address of instruction to disassemble
 *	inst_set	- instruction set to use for disassembling
 *	mnemonic_set	- mnemonic set to use for disassembling
 *
 * DATA STRUCTURES:
 *	opcode_to_form	- Table describing instruction for all opcodes.
 *
 * RETURNS: Returns 0 if the instruction is not valid for the specified
 *	instruction set.  Returns 1 if the instruction is successfully
 *	disassembled.
 */
int decode_instruction(
unsigned long	instruction,
inst		*disassembly,
unsigned long	iar,
unsigned short	inst_set,
unsigned short	mnemonic_set)
{
    unsigned long opcode;
    short form;
    unsigned valid_inst_set;
    short rc = 0;
    int index;
    char *operands;
    boolean offset_found = false;
    spr_table   *special_reg;
    int i, reg_number;

    /*
     * Initialize target_addr field of disassembly structure.
     * All other fields will be filled in appropriately by disassembly routines
     */
    disassembly->target_addr = 0;
    disassembly->number_operands = 0;

    /*
     * Determine opcode
     */
    opcode = bits(instruction,0,5);

    /*
     * Determine instruction format
     */
    form = opcode_to_form[opcode].form;

    /*
     * Determine instruction set
     */
    valid_inst_set = opcode_to_form[opcode].valid_inst_set;

    /*
     * Determine if the opcode is valid for our current instruction set
     */
    if( ! (inst_set & valid_inst_set) ) {
	disassembly->target_addr = valid_inst_set;
	return( 0 );
    } else {
	/*
	 * Decode known instruction format
	 */
	switch(form) {
	case D_FORM:
	   rc = decode_Dform(instruction, opcode, inst_set, mnemonic_set,
			     disassembly);
	   break;
	case B_FORM:
	   rc = decode_Bform(iar, instruction, opcode, mnemonic_set,
			     disassembly);
	   break;
	case I_FORM:
	   rc = decode_Iform(iar,instruction, opcode, disassembly);
	   break;
	case SC_FORM:
	   rc = decode_SCform(instruction, opcode, inst_set, mnemonic_set,
			      disassembly );
	   break;
	case XL_FORM:
	   rc = decode_XLform(instruction, opcode, inst_set, mnemonic_set,
			      disassembly);
	   break;
	case M_FORM:
	   rc = decode_Mform(instruction, opcode, mnemonic_set, disassembly );
	   break;
	case EXTENDED:
	   /*
	    * More work to do... these instructions have extended opcodes.  Need
	    * to look at the extended opcode to determine how to disassemble
	    */
    	   switch(opcode)   /* Switch off of opcode and process from there */
    	   {
	   	case 30:
	   	   rc = decode_opcode30(instruction, inst_set, mnemonic_set,
					disassembly );
	   	   break;
	   	case 31:
	   	   rc = decode_opcode31(instruction, inst_set, mnemonic_set,
					disassembly );
	   	   break;
		case 59:
		   rc = decode_opcode59(instruction, inst_set, disassembly);
		   break;
	   	case 63:
	   	   rc = decode_opcode63(instruction, inst_set, mnemonic_set,
					disassembly );
	   	   break;
	   	default:
		    return( 0 );
	   	   break;
    	   }
	   break;
	case DS_FORM:
	   rc = decode_DSform(instruction, opcode, disassembly);
	   break;
	case INVALID_OP:
	default:
	    return( 0 );
	   break;
    	}
    }

    operands = disassembly->operands;

    operands[0] = '\0';

#ifdef KDBX
    gen_reg_index = 0;
#endif /* KDBX */

    for( index = 0; index < disassembly->number_operands; index++ ) 
    {
      if( index != 0 && offset_found != true )
      {
      /*
       * If this operand follows another, and it is not the register
       * part of an offset pair need to display a comma.
       */

        operands += sprintf (operands, ",");
      }

      switch( disassembly->op[index].op_type )
      {
	case gen_register:
          operands += sprintf (operands, "r%d", 
                               disassembly->op[index].cons_val.value);
#ifdef KDBX
          if (gen_reg_index <= 2) 
          {
            /* prevent gen_reg_number table overflow */
            gen_reg_number[gen_reg_index++] =
                             disassembly->op[index].cons_val.value;
          }
#endif /* KDBX */

          break;

        case float_register:
          operands += sprintf (operands, "fr%d", 
                               disassembly->op[index].cons_val.value);
          break;

        case special_register:
          operands += sprintf (operands, "%d", 
                               disassembly->op[index].cons_val.uvalue);
          break;

        case constant:
          if( disassembly->op[index].cons_val.value < 0 )
            operands += sprintf (operands, "%d", 
                                 disassembly->op[index].cons_val.value);
          else
            operands += sprintf (operands, "0x%x", 
                                 disassembly->op[index].cons_val.value);
          break;

        case uconstant:
          operands += sprintf (operands, "0x%x", 
                               disassembly->op[index].cons_val.uvalue);
          break;

        case offset:
          if( disassembly->op[index].cons_val.value < 0 )
            operands += sprintf (operands, "%d(", 
                                 disassembly->op[index].cons_val.value);
          else 
            operands += sprintf (operands, "0x%x(", 
                                 disassembly->op[index].cons_val.value);
          /*
           * Have hit first part of offset.  Set offset_found so we
           * remember to close the parenthesis after the second part
           * of the offset.
           */
           offset_found = true;
           break;

         case cond_reg_field:
           operands += sprintf (operands, "cr%d", 
                                disassembly->op[index].cons_val.value);
           break;

         default:
           operands += sprintf (operands, "0x%x", 
                                disassembly->op[index].cons_val.uvalue);
           break;
      }
      if( offset_found == true &&
         disassembly->op[index].op_type != offset ) 
      {
        /*
         * This is the register part of an offset pair.  Display
         * closing parenthesis and reset offset_found
         */
        offset_found = false;
        operands += sprintf(operands, ")");
      }
    }
    return( rc );
}
