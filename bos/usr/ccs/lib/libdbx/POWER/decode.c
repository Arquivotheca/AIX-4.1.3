static char sccsid[] = "@(#)89    1.38  src/bos/usr/ccs/lib/libdbx/POWER/decode.c, libdbx, bos41B, 9504A 12/14/94 10:23:24";
/*
 *   COMPONENT_NAME: CMDDBX
 *
 *   FUNCTIONS: addrprint
 *		branch_link
 *              convert
 *		decode
 *		defsysregs
 *		extractField
 *		getfptype
 *              get_data_type
 *		log16
 *		printcond
 *		printop
 *		typecast
 *
 *   ORIGINS: 26,27, 83
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Copyright (c) 1982 Regents of the University of California
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
*/


#ifdef KDBXRT
#include "rtnls.h"		/* MUST BE FIRST */
#endif
/*              include file for message texts          */
#include "dbx_msg.h" 
nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

#include "defs.h"
#include "decode.h"
#include "process.h"
#include "runtime.h"
#include "events.h"
#include "main.h"
#include "symbols.h"
#include "source.h"
#include "mappings.h"
#include "object.h"
#include "tree.h"
#include "eval.h"
#include "keywords.h"
#include "ops.h"
#include <search.h>
#include <sys/reg.h>
#include <signal.h>
#include "disassembly.h"
#ifndef KDBX
#include <sys/systemcfg.h>
#endif /* KDBX */

/*
 * Target machine dependent stuff.
 */

Bpinst BP_OP = BPT;
public Address pc;
public Address prtaddr;

char *regnames[] = { "$r0", "$stkp", "$toc", "$r3", "$r4", "$r5", "$r6", "$r7",
		     "$r8", "$r9", "$r10", "$r11", "$r12", "$r13", "$r14",
		     "$r15", "$r16", "$r17", "$r18", "$r19", "$r20", "$r21",
		     "$r22", "$r23", "$r24", "$r25", "$r26", "$r27", "$r28",
		     "$r29", "$r30", "$r31", "$iar", "$msr", "$cr", "$link",
		     "$ctr", "$xer", "$mq", "$tid", "$fpscr" };
char *regnames_601[] = { "$r0", "$stkp", "$toc", "$r3", "$r4", "$r5", "$r6",
			 "$r7", "$r8", "$r9", "$r10", "$r11", "$r12", "$r13",
			 "$r14", "$r15", "$r16", "$r17", "$r18", "$r19", "$r20",
			 "$r21", "$r22", "$r23", "$r24", "$r25", "$r26", "$r27",
			 "$r28", "$r29", "$r30", "$r31", "$iar", "$msr", "$cr",
			 "$link", "$ctr", "$xer", "$mq", NULL, "$fpscr" };
char *regnames_ppc[] = { "$r0", "$stkp", "$toc", "$r3", "$r4", "$r5", "$r6",
			 "$r7", "$r8", "$r9", "$r10", "$r11", "$r12", "$r13",
			 "$r14", "$r15", "$r16", "$r17", "$r18", "$r19", "$r20",
			 "$r21", "$r22", "$r23", "$r24", "$r25", "$r26", "$r27",
			 "$r28", "$r29", "$r30", "$r31", "$iar", "$msr", "$cr",
			 "$link", "$ctr", "$xer", NULL, NULL, "$fpscr" };

unsigned int	instruction_set = DEFAULT;
unsigned int	mnemonic_set = DEFAULT;

#ifdef KDBX
extern int is_diss;		/* set when disassembling instruction */
static char regs[256];		/* buffers for printing registers values */
static char regs_i[256];	/* temporary buffer for registers values */
int gen_reg_number [3]; 	/* hold the general registers names used */
				/* in an instruction to track their values */
				/* No instruction has more than 3 general */
				/* registers operands */
int gen_reg_index;		/* index in gen_reg_number */

static void track_reg_values();	/* use information in gen_reg_number to */
				/* retrieve registers actual values */

#endif /* KDBX */

/*
 * NAME: decode
 *
 * FUNCTION: Disassembles instructions
 *
 * NOTES: Determines what instruction set and mnemonic set should be used for
 *	disassembling instructions.  Calls decode_instruction() to do the
 *	disassembly.  Displays the disassembled instruction.
 *
 * PARAMETERS:
 *	addr		- Address of instruction to disassemble
 *	disassembly	- Structure to be filled in by decode_instruction()
 *			  describing the disassembled instruction.
 *
 * RECOVERY OPERATION: If the instruction is not valid will display "Invalid
 *	opcode" message.
 *
 * DATA STRUCTURES:
 *	instruction_set	- Used to see if the user has specified a particular
 *			  instruction set for disassembling the instruction.
 *	mnemonic_set	- Used to see if the user has specified a particular
 *			  mnemonic set for disassembling the instruction.
 *	rpt_output	- Used for displaying the disassembled instruction.
 *
 * RETURNS: None
 */
static int decode( addr, disassembly)
Address addr;
inst *disassembly;
{
    unsigned long instruction;
    short current_instruction_set;
    short current_mnemonic_set;
    short rc;
    boolean	offset_found = false;	/* Indicates when hit offset pair */
    spr_table	*special_reg, test;

    /*
     * Get instruction for specified address
     */
    iread(&instruction, addr, sizeof(instruction));

    /*
     * Set current_instruction_set to value specified by user setting
     * $instructionset variable.  If DEFAULT then user has not specified, use
     * the instruction set which corresponds to the hardware architecture we are
     * executing on.
     */
    if( instruction_set == DEFAULT ) {
	current_instruction_set = current_hardware;
    } else {
	current_instruction_set = instruction_set;
    }

    /*
     * Set current_mnemonic_set based on user set value of $mnemonics
     * If DEFAULT user has not set, use set which matches
     * current_instruction_set
     */
    if( mnemonic_set != DEFAULT ) {
	current_mnemonic_set = mnemonic_set;
    } else {
	switch( current_instruction_set ) {
	    case PWR:
	    case PWRX:
		current_mnemonic_set = PWR;
		break;
	    case PPC:
	    case SET_601:
	    case SET_603:
	    case SET_604:
	    case COM:
	    case ANY:
	    default:
		current_mnemonic_set = PPC;
		break;
	}
    }

    /*
     * Disassemble instruction using specified instruction set and mnemonics
     */
    rc = decode_instruction( instruction, disassembly, addr,
			     current_instruction_set, current_mnemonic_set );

    if( rc == 0 ) {
	/*
	 * Instruction is not valid for specified instruction set
	 */
	(*rpt_output)(stdout,  catgets(scmc_catd, MS_decode, MSG_386,
					"    Invalid opcode.") );
    } else {
	/*
	 * Display the disassembled instruction.  Start with the mnemonic
	 */
	if( disassembly->number_operands == 0 &&
	    disassembly->target_addr == 0 ) {
	    (*rpt_output)(stdout, "%8s", disassembly->mnemonic);
	} else {
	    (*rpt_output)(stdout, "%8s   ", disassembly->mnemonic);
	}

	/*
	 * Next, display all of the operands
	 */

        (*rpt_output)(stdout, "%s", disassembly->operands);
	if ((disassembly->number_operands != 0)
         && (disassembly->target_addr != 0)) 
        {
	    (*rpt_output)(stdout, "," );
	}
#ifdef KDBX
	track_reg_values();
#endif /* KDBX */
    }
    return( rc );
}


/*
 * NAME: printop
 *
 * FUNCTION: Translate an instruction from its numeric form into something more 
 *	readable.
 *
 * NOTES: Uses addrprint() to display the symbolic information for a given
 *	address.  Displays the instruction as hex, calls decode() to display the
 *	disassembled instruction.  If the instruction has a target address,
 *	displays the symbolic information for that target.
 *
 * PARAMETERS:
 *	addr		- Address to disassemble
 *	rangelimit	- 
 *
 * DATA STRUCTURES:
 *	rpt_output	- Used to display output
 *
 * RETURNS: The address of the next address to be disassembled
 */
public Address printop (addr, rangelimit)
Address addr;
int rangelimit;
{
    unsigned long instruction;
    Address newaddr;
    inst	disassembly;
    int		rc;

    /*
     * Add length of instruction to address, newaddr will be the address of the
     * next instruction
     */
    newaddr = addr + INST_SZ;

#ifdef KDBX
    if (addr == reg(SYSREGNO(IAR)))
	(*rpt_output)(stdout, "* ");
    else
        (*rpt_output)(stdout, "  ");
#endif /* KDBX */

    /*
     * Display symbolic information for instruction to disassemble
     */
    addrprint( rpt_output, stdout,addr,true,rangelimit);

    /*
     * Get instruction and display it in hex
     */
    iread(&instruction, addr, sizeof(instruction));

#ifdef KDBX
    if (varIsSet("$no_bininst"))
	    (*rpt_output)(stdout, "  ");
    else
#endif /* KDBX */
    (*rpt_output)(stdout, "%08x   ", instruction);

#ifdef KDBX
    /*
     * Blank registers buffer
     */
    strcpy (regs, "");

    /*
     * Set registers
     */
    setregs(process);
#endif /* KDBX */

    /*
     * Disassemble instruction and display it
     */
    rc = decode(addr, &disassembly);

    /*
     * If the disassembly has a target address display the symbolic information
     * for that address
     */
    if( rc != 0 && disassembly.target_addr != 0 ) {
	addrprint( rpt_output, stdout, disassembly.target_addr, false, 0);
    }

#ifdef KDBX
    /* If required and only when disassembling the current instruction,
     *  print the values of the registers  used in the instruction
     */
    if (varIsSet("$showregs") && !is_diss)
      (*rpt_output)(stdout, "\t%s", regs);
#endif /* KDBX */

    (*rpt_output)(stdout, "\n");

    /*
     * Update the addr pointer
     */
    addr = newaddr;
    return(addr);
}

#ifdef KDBX
/*
 * NAME: track_reg_values
 *
 * FUNCTION: add in regs buffer the name and the value of a the general
 *	     registers unsed in the current instruction.
 *	     information set by decode() in gen_reg_number and gen_reg_index.
 *
 * PARAMETERS: none
 *
 * RETURNS: none.
 */
static void track_reg_values()
{
    switch (gen_reg_index) {
    case 0:
	/* No registers , just return */
	return;
    case 1:
	/* One register */
	sprintf (regs,"%s=0x%x",
		regnames[gen_reg_number[0]], reg(gen_reg_number[0]));
    case 2:
	/* Two registers */
	if (gen_reg_number[0] == gen_reg_number[1]) {
		sprintf (regs,"%s=0x%x",
		    regnames[gen_reg_number[0]], reg(gen_reg_number[0]));
	} else {
		sprintf (regs,"%s=0x%x, %s=0x%x", 
		    regnames[gen_reg_number[0]], reg(gen_reg_number[0]),
		    regnames[gen_reg_number[1]], reg(gen_reg_number[1]));
	}
	break;
    case 3:
	/* Three registers */
	if (gen_reg_number[0] == gen_reg_number[1]) {
		if (gen_reg_number[0] == gen_reg_number[2]) {
		    /* r0=r1=r2 */
		    sprintf (regs,"%s=0x%x",
			regnames[gen_reg_number[0]], reg(gen_reg_number[0]));
		} else {
		    /* r0=r1 , r2 */
		    sprintf (regs,"%s=0x%x, %s=0x%x", 
		        regnames[gen_reg_number[0]], reg(gen_reg_number[0]),
		        regnames[gen_reg_number[2]], reg(gen_reg_number[2]));
		}
	} else {
		if (gen_reg_number[1] == gen_reg_number[2]) {
		    /* r0 , r1=r2 */
		    sprintf (regs,"%s=0x%x, %s=0x%x",
			regnames[gen_reg_number[0]], reg(gen_reg_number[0]),
			regnames[gen_reg_number[1]], reg(gen_reg_number[1]));
		} else {
		    /* r0 , r1 , r2 */
		    sprintf (regs,"%s=0x%x,%s=0x%x,%s=0x%x",
		        regnames[gen_reg_number[0]], reg(gen_reg_number[0]),
		        regnames[gen_reg_number[1]], reg(gen_reg_number[1]),
		        regnames[gen_reg_number[2]], reg(gen_reg_number[2]));
		}
	}
    }
}

#endif /* KDBX */

/*
 * Simply return whether an instruction is a branch_with_link
 */
public Boolean branch_link(instruction)
unsigned long instruction;
{
	unsigned long opcode, ext_op;
	boolean link;

	opcode = bits(instruction,0,5);
	link = (boolean) bits(instruction,31,31);
	switch (opcode) {
	   case 16: 		/* unconditional branch */
	   case 18: 		/* conditional branch */
		return link;
	   case 19: 		/* possibly branch register */
		ext_op = bits(instruction,21,30);
		if ((ext_op != 16) && (ext_op != 528)) {
		    return false;
		} else {
		    return link;
		}
	   default:		/* definitely not a branch */
		return false;
	}
}


/*
 * NAME: defsysregs
 *
 * FUNCTION: Define the names for the system or special-purpose registers.
 *
 * PARAMETERS: NONE
 *
 * RETURNS: NONE
 */
void defsysregs() {
    defregname(identname("$fp", true), STKP);
    defregname(identname("$FP", true), STKP);
    defregname(identname("$sp", true), STKP);
    defregname(identname("$SP", true), STKP);
    defregname(identname("$stkp", true), STKP);
    defregname(identname("$STKP", true), STKP);
    defregname(identname("$toc", true), GPR2);
    defregname(identname("$TOC", true), GPR2);
    defregname(identname("$pc", true), SYSREGNO(PROGCTR));
    defregname(identname("$PC", true), SYSREGNO(PROGCTR));
    defregname(identname("$iar", true), SYSREGNO(PROGCTR));
    defregname(identname("$IAR", true), SYSREGNO(PROGCTR));
    defregname(identname("$cr", true), SYSREGNO(CR));
    defregname(identname("$CR", true), SYSREGNO(CR));
    defregname(identname("$cs", true), SYSREGNO(CR));
    defregname(identname("$CS", true), SYSREGNO(CR));
    defregname(identname("$ctr", true), SYSREGNO(CTR));
    defregname(identname("$CTR", true), SYSREGNO(CTR));
    defregname(identname("$link", true), SYSREGNO(LR));
    defregname(identname("$LINK", true), SYSREGNO(LR));
    defregname(identname("$lr", true), SYSREGNO(LR));
    defregname(identname("$LR", true), SYSREGNO(LR));
    defregname(identname("$xer", true), SYSREGNO(XER));
    defregname(identname("$XER", true), SYSREGNO(XER));
    if( current_hardware & (PWR | PWRX | SET_601) ) {
	defregname(identname("$mq", true), SYSREGNO(MQ));
	defregname(identname("$MQ", true), SYSREGNO(MQ));
    }
    defregname(identname("$msr", true), SYSREGNO(MSR));
    defregname(identname("$MSR", true), SYSREGNO(MSR));
    if( current_hardware & (PWR | PWRX) ) {
	defregname(identname("$tid", true), SYSREGNO(TID));
	defregname(identname("$TID", true), SYSREGNO(TID));
	defregname(identname("$hid1", true), SYSREGNO(TID));
	defregname(identname("$HID1", true), SYSREGNO(TID));
    }
    defregname(identname("$fpsr", true), SYSREGNO(FPSCR));
    defregname(identname("$FPSR", true), SYSREGNO(FPSCR));
    defregname(identname("$fpscr", true), SYSREGNO(FPSCR));
    defregname(identname("$FPSCR", true), SYSREGNO(FPSCR));
}

/*
 * Extract a bit field from an integer.
 */

public integer extractField (s, range_type)
Symbol s;
Symbol range_type;
{
    integer nbytes, nbits, n, r, off, len;

    off = s->symvalue.field.offset;
    len = s->symvalue.field.length;
    nbytes = size(s);
    n = 0;
    if (nbytes > sizeof(n)) {
	(*rpt_output)(stdout,  catgets(scmc_catd, MS_decode, MSG_478,
			     "[bad size in extractField -- word assumed]\n") );
	nbytes = sizeof(n);
    }
    popn(nbytes, ((char *) &n) + (sizeof(Word) - nbytes));
    nbits = nbytes * BITSPERBYTE;
    r = n >> (nbits - ((off mod BITSPERBYTE) + len));

    /*  if this is c or C++ range and $hexints is not set
          and the type is signed  */
    /*  NOTE : if this is cobol, range_type will be NULL  */

    if (range_type && (range_type->class == RANGE)
     && !varIsSet("$hexints")
     && !range_type->symvalue.rangev.is_unsigned)
    {
      /*  sign extend the bit field by shifting left and then right  */
      r = (r << (BITSPERWORD - len)) >> (BITSPERWORD - len);
    }
    else
    {
      /*  treat the bit field as unsigned - just isolate the bits  */
      r &= ((1 << len) - 1);
    }
    return r;
}

typedef enum { schar_type, uchar_type, short_type, ushort_type, 
               int_type, uint_type, longlong_type, ulonglong_type,
               float_type, double_type, ldouble_type, other_type
             } data_types;

/*
 * NAME: get_data_type
 *
 * FUNCTION: return a value corresponding to a built-in type.
 *
 * PARAMETERS: input_symbol - symbol containing type information 
 *             input_size   - size
 *
 * RETURNS: enum value corresponding to a built-in type
 */

data_types get_data_type (input_symbol, input_size)
Symbol input_symbol;
int input_size;
{
  /*  if the symbol is an enum  */
  if (input_symbol->class == SCAL)
  {
    /*  reduce it down to the underlying range type  */
    input_symbol = rtype(input_symbol->type);
  }

  if (input_symbol->class == RANGE)
  {
    switch (input_size)
    {
      case sizeof (char) :
        if (input_symbol->symvalue.rangev.is_unsigned)
          return uchar_type;
        else
          return schar_type;
        break;
     case sizeof (short) :
        if (input_symbol->symvalue.rangev.is_unsigned)
          return ushort_type;
        else
          return short_type;
        break;
     case sizeof (int) :
        if (input_symbol->symvalue.rangev.is_unsigned)
          return uint_type;
        else
          return int_type;
        break;
     case sizeofLongLong :
        if (input_symbol->symvalue.rangev.is_unsigned)
          return ulonglong_type;
        else
          return longlong_type;
        break;
    }
  }
  else if (input_symbol->class == REAL)
  {
    switch (input_size)
    {
      case sizeof (float) :
        return float_type;
        break;
      case sizeof (double) :
        return double_type;
        break;
      case 2 * sizeof (double) :
        return ldouble_type;
        break;
    }
  }
  return (other_type);
}

/*
 * NAME: convert
 *
 * FUNCTION: convert a value to a new type.
 *
 * PARAMETERS: 
 *       input         - pointer to input data
 *       input_symbol  - symbol containing input type information 
 *       output        - pointer to area to place output data
 *       output_symbol - symbol containing output type information
 *
 * NOTES: When we get library support for long double
 *        conversions, we may want to change the handling
 *        of long doubles.  We currently get the value
 *        in a 2 step process.  The value is stored in 
 *        a quadf (struct containing array of 2 doubles).
 *        We currently copy one part at a time.  
 * RETURNS: 0 if successful, -1 otherwise
 */

int convert (input, input_symbol, output, output_symbol)
void *input;
Symbol input_symbol;
void *output;
Symbol output_symbol;
{
  int input_size = size(input_symbol);
  int output_size = size(output_symbol);
  data_types input_type = get_data_type(input_symbol, input_size);
  data_types output_type = get_data_type(output_symbol, output_size);
  LongLong val_longlong;
  quadf val_quad = {0.0, 0.0};
  Boolean ulonglong_input = false;

  /*  if only one of the symbols is a "base" type, attempt to treat
        the other as something compatible  */

  if ((input_type == other_type) && (output_type == other_type))
    return -1;
  else if (input_type == other_type)
  {
    if ((input_type = get_data_type(output_symbol, input_size))
            == other_type)
      return -1;
  }
  else if (output_type == other_type)
  {
    if ((output_type = get_data_type(input_symbol, output_size))
            == other_type)
      return -1;
  }

  if (input_symbol->class != REAL)
  {
    switch (input_type)
    {
      case schar_type :
        val_longlong = (LongLong) *(signed char *) input;
        break;
      case uchar_type :
        val_longlong = (LongLong) *(unsigned char *) input;
        break;
      case short_type :
        val_longlong = (LongLong) *(short *) input;
        break;
      case ushort_type :
        val_longlong = (LongLong) *(unsigned short *) input;
        break;
      case int_type :
        val_longlong = (LongLong) *(int *) input;
        break;
      case uint_type :
        val_longlong = (LongLong) *(unsigned int *) input;
        break;
      case longlong_type :
        val_longlong = *(LongLong *) input;
        break;
      case ulonglong_type :
        val_longlong = *(uLongLong *) input;
        ulonglong_input = true;
        break;
    }
    switch (output_type)
    {
      case schar_type :
        *(signed char *) output = (signed char) val_longlong;
        break;
      case uchar_type :
        *(unsigned char *) output = (unsigned char) val_longlong;
        break;
      case short_type :
        *(short *) output = (short) val_longlong;
        break;
      case ushort_type :
        *(unsigned short *) output = (unsigned short) val_longlong;
        break;
      case int_type :
        *(int *) output = (int) val_longlong;
        break;
      case uint_type :
        *(unsigned int *) output = (unsigned int) val_longlong;
        break;
      case longlong_type :
        *(LongLong *) output = val_longlong;
        break;
      case ulonglong_type :
        *(uLongLong *) output = (uLongLong) val_longlong;
        break;
      case float_type :
        if (ulonglong_input)
          *(float *) output = (float) (uLongLong) val_longlong;
        else
          *(float *) output = (float) val_longlong;
        break;
      case double_type :
        if (ulonglong_input)
          *(double *) output = (double) (uLongLong) val_longlong;
        else
          *(double *) output = (double) val_longlong;
        break;
      case ldouble_type :
        if (ulonglong_input)
          *(double *) output = (double) (uLongLong) val_longlong;
        else
          *(double *) output = (double) val_longlong;

        *(((double *) output) + 1) = 0.0;
        break;
    }
  }
  else
  {
    switch (input_type)
    {
      case float_type :
        val_quad.val[0] = (double) *(float *) input;
        break;
      case double_type :
        val_quad.val[0] = *(double *) input;
        break;
      case ldouble_type :
        val_quad.val[0] = *(double *) input;
        val_quad.val[1] = *(((double *) input) + 1);
        break;
    }
    switch (output_type)
    {
      case schar_type :
        *(signed char *) output = (signed char) val_quad.val[0];
        break;
      case uchar_type :
        *(unsigned char *) output = (unsigned char) val_quad.val[0];
        break;
      case short_type :
        *(short *) output = (short) val_quad.val[0];
        break;
      case ushort_type :
        *(unsigned short *) output = (unsigned short) val_quad.val[0];
        break;
      case int_type :
        *(int *) output = (int) val_quad.val[0];
        break;
      case uint_type :
        *(unsigned int *) output = (unsigned int) val_quad.val[0];
        break;
      case longlong_type :
        *(LongLong *) output = (LongLong) val_quad.val[0];
        break;
      case ulonglong_type :
        *(uLongLong *) output = (uLongLong) val_quad.val[0];
        break;
      case float_type :
        *(float *) output = (float) val_quad.val[0];
        break;
      case double_type :
        *(double *) output = val_quad.val[0];
        break;
      case ldouble_type :
        *(double *) output = val_quad.val[0];
        *(((double *) output) + 1) = val_quad.val[1];
        break;
    }
  }

  return 0; 
}

/*
 * NAME: typecast
 *
 * FUNCTION: perform a "cast" on the value on the stack (sp).
 *
 * PARAMETERS: p - input node 
 *
 * NOTE: the cast is performed "in place" on the stack and
 *         the stack pointer (sp) adjusted appropriately.
 *       this function only gets called if p->op is O_TYPERENAME  
 *
 * RETURNS: node
 */

void typecast (p)
Node p;  
{
  Symbol newtype, oldtype;
  void *data_pointer;
  int rc;

  oldtype = rtype (p->value.arg[0]->nodetype);
  newtype = rtype (p->nodetype);

  data_pointer = sp - size(oldtype);

  /*  call convert to perform the conversion  */
  /*  NOTE : conversion is done in place on the stack
               and the stack pointer is adjusted appropriately
               after returning if the conversion was done  */
  rc = convert (data_pointer, oldtype, data_pointer, newtype);

  /*  if convert knew how to do the conversion  */
  if (rc == 0)
  {
    /*  adjust the stack pointer appropriately  */
    sp += (size(newtype) - size(oldtype));
  }
  else
  {
    /*  left justify  */
    sp += size(newtype) - size(oldtype);
  }
                
  return; 
}

extern Boolean noexec;

/* 
 *  Print out the address of an instruction or a destination.  Nicely.
 */

public addrprint( report, f, addr, pad, rangelimit)
int (*report)( );
File f;
Address addr;
Boolean pad;
{
    static Address funcaddr = 0;
    static Symbol lastfunc = nil;
    static char *funcname = nil;
    static char blanks[] = "                                ";
    Symbol newfunc;
    int offset, padding;
#ifdef KDBX
    int curline;
#endif /* KDBX */

    if ((addr >= LOWADDR) || noexec) {   /* Find the containing function */
       (*report)( f, "0x%08x ",addr);
       newfunc = whatblock(addr);
       if (newfunc != lastfunc) {
           funcaddr = prolloc(newfunc);
	   if (!funcaddr)		 /* unnamed blocks have prolloc == 0 */
	      funcaddr = codeloc(newfunc);	
	   funcname = symname(newfunc);
	   lastfunc = newfunc;
       }
       offset = addr - funcaddr;
#ifdef KDBX
       curline = srcline(addr);
#endif /* KDBX */
       padding = (pad) ? log16(offset+rangelimit)-log16(offset)+1 : 1;
       if ((newfunc == program) || ((addr >> 28) != (funcaddr >> 28)))
	  (*report)( f,"(??\077) ");
       else if (offset)
#ifdef KDBX
	  if (curline)
	    (*report)( f,"(%s:%d+0x%x)%s",funcname, curline, offset,
				&blanks[32-padding]);
	  else
#endif /* KDBX */
	  (*report)( f,"(%s+0x%x)%s",funcname, offset, &blanks[32-padding]);
       else
#ifdef KDBX
	  if (curline)
	    (*report)( f,"(%s:%d)%s",funcname, curline, &blanks[32-padding-3]);
	  else
#endif /* KDBX */
	  (*report)( f,"(%s)%s",funcname, &blanks[32-padding-3]);
    } else {
       (*report)( f,"0x%x ",addr);
    }
}

log16 (n)
int n;
{
    int i;
    for (i = 0; n > 0; i++)
	 n /= 16;
    return --i;
}


getfptype(numregs)
int *numregs;
{
	*numregs = 32;
}


/*
 * Print out the condition status register in a meaningful fashion.
 */
printcond()
{
	Word cs; 
	Word crset[8];
	char crbuf[57];
	char *crptr = crbuf;
	int i;
	
	cs = reg(SYSREGNO(CR));

	if (cs != 0) {
		for (i = 0; i <= 7; i++) {
			crset[i] = bits(cs,(i*4),(i*4+3));
			if (crset[i] != 0) {
				*crptr++ = i + '0';
				*crptr++ = ':';
				if (crset[i] & CR_LT)
				    *crptr++ = 'l';
				if (crset[i] & CR_GT)
				    *crptr++ = 'g';
				if (crset[i] & CR_EQ)
				    *crptr++ = 'e';
				if (crset[i] & CR_SO)
				    *crptr++ = 'o';
				*crptr++ = ' ';
			}
		}	
		*crptr = '\0';
		(*rpt_output)(stdout,
				   "          Condition status = %s\n", crbuf);
	}
}
