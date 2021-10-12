/* @(#)29	1.3  src/bos/usr/ccs/lib/libdbx/POWER/disassembly.h, libdbx, bos41B, 9504A 12/15/94 19:15:40 */
/*
 *   COMPONENT_NAME: CMDDBX
 *
 *   FUNCTIONS: bits
 *		
 *
 *   ORIGINS: 26,27
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Copyright (c) 1982 Regents of the University of California
 */
#ifndef H_DISASSEMBLY
#define H_DISASSEMBLY

/* Condition register fields */
#define CR_LT	8
#define CR_GT	4
#define CR_EQ	2
#define CR_SO	1

/*
 * operand_type: describes the different types for the assembly instruction
 *		 operands
 */
typedef enum { gen_register, float_register, special_register, uconstant,
	   constant, offset, cond_reg_field } operand_type;

/*
 * operand: describes an operand for a disassembled instruction
 *
 *	cons_val.value:		Signed value of operand
 *	cons_val.uvalue:	Unsigned value of operand
 *	op_type:		Type of operand
 */
typedef struct {
    union {
	int		value;
	unsigned	uvalue;
    } cons_val;
    operand_type op_type;
} operand;

/*
 * Maximum length of an assembler instruction
 */
#define MAX_MNEMONIC_LENGTH	10

/* 
 * Maximum number of operands for an assembler instruction
 */
#define	MAX_NUM_OPERANDS	5

/*  The longest an operand can be is 12 characters :

      fr1234567890
or    cr1234567890

    and there can only be 5 operands - each is separated by
    a comma (max of 4).  Add room for an ending '\0'.  Of course,
    it is extrememly unlikely that we will ever have 10 digits
    for a register number, but just in case ...
 */

#define MAX_OPERAND_LENGTH      65

/*
 * inst: Describes disassembled assembler instruction
 *
 *	number_operands:	Number of operands for assembler instruction
 *	mnemonic:		Mnemonic for instruction
 *	op[]:			Description for each operand
 *	target_addr:		Set if there is a target address for the branch
 *				instruction 
 */
typedef struct {
    short	number_operands;
    char	mnemonic[MAX_MNEMONIC_LENGTH];
    operand	op[MAX_NUM_OPERANDS];
    unsigned int	target_addr;
    char        operands[MAX_OPERAND_LENGTH];
} inst;

int decode_instruction();

/*
 * bits: This macro will extract the specified bits (inclusive)
 *
 * x	- Item to extract bits from
 * n	- Extract bits starting from this one
 * m	- Extract bits ending at this one
 */
#define bits(x,n,m) ((x >> (31-m)) & ((1 << (m-n+1)) - 1))
#endif /* H_DISASSEMBLY */
