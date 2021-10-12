static char sccsid[] = "@(#)22	1.12.1.7  src/bos/usr/bin/adb/POWER/opcode.c, cmdadb, bos41B, 9504A 12/15/94 19:45:20";
/*
 * COMPONENT_NAME: (CMDADB) assembler debugger
 *
 * FUNCTIONS:	printins
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * SUBCOMPONENT DESIGN PROLOG:
 *
 * SUBCOMPONENT NAME: disassembled instruction output
 *
 * FILES: opcode.c 
 *	
 * EXTERNAL FUNCTIONS:	printins
 *
 * EXTERNAL DATA: dot
 *
 * DEPENDENCIES:
 *	component: libdbx
 *		   subcomponent: disassemble valid instruction sets
 *			files:	disassembly.h 
 *				disassembly.c
 *				ops.h
 *				ops.c
 *
 * FUNCTION: format and output the result of decode_instruction
 *
 * NOTES:    The function decode_instruction is called to disassemble an
 *		instruction at dot using the current instruction and mnemonic
 *		sets. The result of the call is formated and output via the
 *		adbpr function.
 *
 * DATA STRUCTURES: dot is incremented 4 to preserve the existing global 
 *                  side effect since not all calls restore dot.
 *
 *
 */
/*              include file for message texts          */
#include "adb_msg.h"
extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

#include "defs.h"
#include <search.h>
#include <sys/reg.h>
#include <signal.h>
#include "ops.h"
#include "disassembly.h"
#include <sys/systemcfg.h>

extern unsigned short current_hardware;
extern unsigned int instruction_set;
extern unsigned int mnemonic_set;

#define Address	    unsigned int
typedef enum { false, true } boolean;

/*
 * NAME: printins
 *
 * FUNCTION: Disassembles instructions
 *
 * NOTES: Determines what instruction set and mnemonic set should be used for
 *	  disassembling instructions.  Calls decode_instruction() to do the
 *	  disassembly.  Displays the disassembled instruction.
 *
 * PARAMETERS: N/A
 *
 * RECOVERY OPERATION: If the instruction is not valid will display an "Invalid
 *	opcode" message.
 *
 * DATA STRUCTURES:
 *	instruction_set	- Used to see if the user has specified a particular
 *			  instruction set for disassembling the instruction.
 *	mnemonic_set	- Used to see if the user has specified a particular
 *			  mnemonic set for disassembling the instruction.
 *
 * RETURNS: None
 */
void printins()
{
    unsigned long instruction;
    inst disassembly;
    Address addr = dot;
    short current_instruction_set;
    short current_mnemonic_set;
    short rc, index;
    boolean	offset_found = false;	/* Indicates when hit offset pair */
    spr_table	*special_reg, test;

    /*
     * Get instruction for specified address
     */
    instruction = get(dot,ISP);
    /* previous global side effect */
    dot += 4; 

    /*
     * Set current_instruction_set to value specified by user.
     * If DEFAULT then user has not specified, use the instruction set
     * which corresponds to the hardware architecture we are
     * executing on.
     */
    if( instruction_set == DEFAULT ) {
	current_instruction_set = current_hardware;
    } else {
	current_instruction_set = instruction_set;
    }

    /*
     * Set current_mnemonic_set based on value of set with $n command.
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
    rc = decode_instruction( instruction, &disassembly, addr,
			     current_instruction_set, current_mnemonic_set );

    if( rc == 0 ) {
	/*
	 * Instruction is not valid for specified instruction set
	 */
	adbpr(catgets(scmc_catd, MS_opcode, E_MSG_150,
					"    Invalid opcode."));
        return;
    } else {
	/*
	 * Display the disassembled instruction.  Start with the mnemonic
	 */
	if( disassembly.number_operands == 0 &&
	    disassembly.target_addr == 0 ) {
	    adbpr("%8s", disassembly.mnemonic);
	    return;
	} else {
	    adbpr("%8s   ", disassembly.mnemonic);
          }
      }

      adbpr ("%s", disassembly.operands);
        if( disassembly.target_addr != 0 ) {
            /*
             * We have a target address for a branch instruction
             */
            if (disassembly.number_operands != 0)
            {
                /*
                 * There was a preceding argument, need the comma separator
                 */
                adbpr( "," );
            }
            /*
             * Print the target
             */
            psymoff(disassembly.target_addr, IDSYM, "");
        }
}

