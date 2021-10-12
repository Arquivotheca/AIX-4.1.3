/* @(#)94	1.7  src/bos/usr/ccs/lib/libdbx/POWER/ops.h, libdbx, bos41B, 9504A 12/15/94 19:44:10 */
/*
 *   COMPONENT_NAME: CMDDBX
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 26,27
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Copyright (c) 1982 Regents of the University of California
 */
#ifndef _h_ops
#define _h_ops
typedef long YOpCode;

#define	BPT	0x7D821008	/* breakpoint trap */

/* Special register fields */
#define SPR_DONTCARE	0
#define SPR_PPC		0x1f

#define SPR_MQ	    0
#define SPR_XER	    1
#define SPR_RTCU1   4
#define SPR_RTCL1   5
#define SPR_DEC1    6
#define SPR_LR	    8
#define SPR_CTR     9
#define SPR_IMR	   512
#define SPR_HID0   543
#define SPR_TID	   544
#define SPR_DSISR  576
#define SPR_DAR	   608
#define SPR_RTCU2  640
#define SPR_DABR   672
#define SPR_DEC2   704
#define SPR_SDR0   768
#define SPR_SDR1   800
#define SPR_SRR0   832
#define SPR_SRR1   864
#define SPR_DSAR   896
#define SPR_TSR	   928
#define SPR_ILCR   960
#define SPR_PID	   1023

/* Trap Options */
#define TO_LT	16
#define TO_GT	 8
#define TO_EQ	 4
#define TO_LLT   2
#define TO_LGT	 1

/*
 * opcode_tab: Structure for the D and XL forms.
 *
 *	format		- How to disassemble operands
 *	mnemonic	- PWR mnemonic; default if no alt_mnem
 *	alt_mnem	- PPC mnemonic
 *	valid_inst_set	- instruction sets this is valid for
 */
typedef struct opcode_tab { 
     unsigned long format;
     char *mnemonic;
     char *alt_mnem;
     unsigned short valid_inst_set;
};


/*
 * opcodeXX: Structure for opcode 31, 59 and 63: these include the X, XO, XFL,
 *	     XFX and A form.
 *
 *	key		- extended opcode
 *	form		- instruction format
 *	format		- how to disassemble operands
 *	mnemonic	- PWR mnemonic; default if no alt_mnem
 *	alt_mnem	- PPC mnemonic
 *	valid_inst_set	- instruction sets this is valid for
 */
typedef struct opcodeXX {
     unsigned short key;
     unsigned short form;
     unsigned long format;
     char *mnemonic;
     char *alt_mnem;
     unsigned short valid_inst_set;
};


/* Different instruction formats */

#define  D_FORM      0
#define  B_FORM      1
#define  I_FORM	     2
#define  SC_FORM     3
#define  X_FORM      4
#define  XL_FORM     5
#define  XFX_FORM    6
#define  XFL_FORM    7
#define  XO_FORM     8
#define  A_FORM      9
#define  M_FORM     10
#define  EXTENDED   11
#define  DS_FORM    12
#define  MD_FORM    13   /*  includes MDS form also  */
#define  INVALID_OP 14

#define OPCODE30_SZ     20
#define OPCODE31_SZ	348
#define OPCODE63_SZ	35
#define AFORM_SZ	22
#define OPCODE59_SZ	20
#define SPR_SZ		23

extern unsigned int instruction_set;
extern unsigned int mnemonic_set;

/*
 * Different instruction sets
 *
 * COM needs to be its own set so it can match with any of the sets or just COM
 * UNQ_NNN - is for those instructions that are in NNN but not required for PPC
 */
#define NOSET		0x0000
#define DEFAULT		0x0001
#define PWR		0x0002
#define PWRX		0x0004
#define PPC		0x0008
#define UNQ_601		0x0010
#define COM		0x0020
#define UNQ_603		0x0040
#define UNQ_604		0x0080
#define UNQ_620         0x0100

/*  it is not necessary to list the UNQ_6** instruction sets in ALL_SETS
    because the SET_6** is always used when comparing, and it contains
    PPC  */
#define ALL_SETS	(PWR|PWRX|PPC|COM)
#define ANY		ALL_SETS

/*
 * SET_NNN is used for the entire instruction set for NNN; This includes the
 *	   required PPC instructions, and those instructions unique to NNN.
 */
#define SET_601		(PPC|UNQ_601)
#define SET_603		(PPC|UNQ_603)
#define SET_604		(PPC|UNQ_604)
#define SET_620         (PPC|UNQ_620)


/*
 * opcode_table: Maps opcodes to forms and instruction set
 *
 *	form		- instruction form
 *	valid_inst_set	- instruction sets valid on
 */
typedef struct {
    char	form;
    unsigned	valid_inst_set;
} opcode_table;


typedef struct {
    unsigned short int	reg_number;
    char		*reg_name;
    char		*alt_reg_name;
    unsigned long	valid_inst_set;
} spr_table;
    
extern opcode_table opcode_to_form[];
extern struct opcode_tab Dform[];
extern struct opcode_tab XLform[];
extern struct opcodeXX opcode30[];
extern struct opcodeXX opcode31[];
extern struct opcodeXX opcode63[];
extern struct opcodeXX opcode59[];
extern struct opcodeXX Aform[];
extern char *TO_ext();
extern spr_table SPR_name[];
extern int spr_comp();

#endif /* _h_ops */
