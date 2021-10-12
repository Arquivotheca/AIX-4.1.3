static char sccsid[] = "@(#)93	1.9  src/bos/usr/ccs/lib/libdbx/POWER/ops.c, libdbx, bos41J, 9508A 2/3/95 10:28:10";
/*
 *   COMPONENT_NAME: CMDDBX
 *
 *   FUNCTIONS: SPR_name
 *		TO_ext
 *		opcode_comp
 *		
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
#ifndef _KERNEL
#include <stdlib.h>
#else
#define NULL 0
#endif
#include "ops.h"

/*
 * opcode_to_form: Table maps opcodes to instruction formats and
 *		   opcodes to valid instruction sets
 */
opcode_table opcode_to_form[] = {
       /* FORM		INSTRUCTION SET */

	{ INVALID_OP,	NOSET },		/* OPCODE 00 */
	{ INVALID_OP,	NOSET },   		/* OPCODE 01 */
	{ D_FORM,	UNQ_620 },   		/* OPCODE 02 */
	{ D_FORM,	ALL_SETS },   		/* OPCODE 03 */
	{ INVALID_OP,	NOSET },   		/* OPCODE 04 */
	{ INVALID_OP,	NOSET },   		/* OPCODE 05 */
	{ INVALID_OP,	NOSET },   		/* OPCODE 06 */
	{ D_FORM,	ALL_SETS },   		/* OPCODE 07 */
	{ D_FORM,	ALL_SETS },   		/* OPCODE 08 */
	{ D_FORM,	PWR|PWRX|UNQ_601 },   	/* OPCODE 09 */
	{ D_FORM,	ALL_SETS },   		/* OPCODE 10 */
	{ D_FORM,	ALL_SETS },   		/* OPCODE 11 */
	{ D_FORM,	ALL_SETS },   		/* OPCODE 12 */
	{ D_FORM,	ALL_SETS },   		/* OPCODE 13 */
	{ D_FORM,	ALL_SETS },   		/* OPCODE 14 */
	{ D_FORM,	ALL_SETS },   		/* OPCODE 15 */
	{ B_FORM,	ALL_SETS },   		/* OPCODE 16 */
	{ SC_FORM,	ALL_SETS },   		/* OPCODE 17 */
	{ I_FORM,	ALL_SETS },   		/* OPCODE 18 */
	{ XL_FORM,	ALL_SETS },   		/* OPCODE 19 */
	{ M_FORM,	ALL_SETS },   		/* OPCODE 20 */
	{ M_FORM,	ALL_SETS },   		/* OPCODE 21 */
	{ M_FORM,	PWR|PWRX|UNQ_601 },   	/* OPCODE 22 */
	{ M_FORM,	ALL_SETS },   		/* OPCODE 23 */
	{ D_FORM,	ALL_SETS },   		/* OPCODE 24 */
	{ D_FORM,	ALL_SETS },   		/* OPCODE 25 */
	{ D_FORM,	ALL_SETS },   		/* OPCODE 26 */
	{ D_FORM,	ALL_SETS },   		/* OPCODE 27 */
	{ D_FORM,	ALL_SETS },   		/* OPCODE 28 */
	{ D_FORM,	ALL_SETS },   		/* OPCODE 29 */
	{ EXTENDED,	UNQ_620 },   		/* OPCODE 30 */
	{ EXTENDED,	ALL_SETS },		/* OPCODE 31 */
	{ D_FORM,	ALL_SETS },   		/* OPCODE 32 */
	{ D_FORM,	ALL_SETS },   		/* OPCODE 33 */
	{ D_FORM,	ALL_SETS },   		/* OPCODE 34 */
	{ D_FORM,	ALL_SETS },   		/* OPCODE 35 */
	{ D_FORM,	ALL_SETS },   		/* OPCODE 36 */
	{ D_FORM,	ALL_SETS },   		/* OPCODE 37 */
	{ D_FORM,	ALL_SETS },   		/* OPCODE 38 */
	{ D_FORM,	ALL_SETS },   		/* OPCODE 39 */
	{ D_FORM,	ALL_SETS },   		/* OPCODE 40 */
	{ D_FORM,	ALL_SETS },   		/* OPCODE 41 */
	{ D_FORM,	ALL_SETS },   		/* OPCODE 42 */
	{ D_FORM,	ALL_SETS },   		/* OPCODE 43 */
	{ D_FORM,	ALL_SETS },   		/* OPCODE 44 */
	{ D_FORM,	ALL_SETS },   		/* OPCODE 45 */
	{ D_FORM,	ALL_SETS },   		/* OPCODE 46 */
	{ D_FORM,	ALL_SETS },   		/* OPCODE 47 */
	{ D_FORM,	ALL_SETS },   		/* OPCODE 48 */
	{ D_FORM,	ALL_SETS },   		/* OPCODE 49 */
	{ D_FORM,	ALL_SETS },   		/* OPCODE 50 */
	{ D_FORM,	ALL_SETS },   		/* OPCODE 51 */
	{ D_FORM,	ALL_SETS },   		/* OPCODE 52 */
	{ D_FORM,	ALL_SETS },   		/* OPCODE 53 */
	{ D_FORM,	ALL_SETS },   		/* OPCODE 54 */
	{ D_FORM,	ALL_SETS },   		/* OPCODE 55 */
	{ DS_FORM,	PWRX },   		/* OPCODE 56 */
	{ DS_FORM,	PWRX },   		/* OPCODE 57 */
	{ DS_FORM,	UNQ_620 },		/* OPCODE 58 */
	{ EXTENDED,	PPC },			/* OPCODE 59 */
	{ DS_FORM,	PWRX },			/* OPCODE 60 */
	{ DS_FORM,	PWRX },			/* OPCODE 61 */
	{ DS_FORM,	UNQ_620 },		/* OPCODE 62 */
	{ EXTENDED,	ALL_SETS },		/* OPCODE 63 */
};

#define X 9     /* space holder */

/*
 * Dform: Table maps D form instructions into description
 *	format		- Describes how operands should be disassembled
 *	mnemonic	- PWR mnemonic; default if no PPC specified
 *	alt_mnem	- PPC mnemonic
 *	valid_inst_set	- Which instruction sets allow this instruction
 */
struct opcode_tab Dform[] = {

/* OPCD   EO          format      mnemonic    alt_mnem  valid_inst_set */
/* ----   --          ------      --------    --------  -------------- */
/*   0,  XXX, */  {     X,        "RESERVED", NULL,	NOSET },
/*   1,  XXX, */  {     X,        "RESERVED", NULL,	NOSET },
/*   2,  XXX, */  {     3,        "tdi",      NULL,	UNQ_620 },
/*   3,  XXX, */  {     3,        "ti",       "twi",	ALL_SETS },
/*   4,  XXX, */  {     X,        "RESERVED", NULL,	NOSET },

/*   5,  XXX, */  {     X,        "RESERVED", NULL,	NOSET },
/*   6,  XXX, */  {     X,        "RESERVED", NULL,	NOSET },
/*   7,  XXX, */  {     0,        "muli",     "mulli",	ALL_SETS },
/*   8,  XXX, */  {     0,        "sfi",      "subfic",	ALL_SETS },
/*   9,  XXX, */  {     0,        "dozi",     NULL,	PWR|PWRX|UNQ_601 },

/*  10,  XXX, */  {     4,        "cmpli",    NULL,	ALL_SETS },
/*  11,  XXX, */  {     4,        "cmpi",     NULL,	ALL_SETS },
/*  12,  XXX, */  {     0,        "ai",       "addic",	ALL_SETS },
/*  13,  XXX, */  {     0,        "ai.",      "addic.",	ALL_SETS },
/*  14,  XXX, */  {     2,        "cal",      "addi",	ALL_SETS },

/*  15,  XXX, */  {     0,        "cau",      "addis",	ALL_SETS },
/*  16,  XXX, */  {     X,        "RESERVED", NULL,	NOSET },
/*  17,  XXX, */  {     X,        "RESERVED", NULL,	NOSET },
/*  18,  XXX, */  {     X,        "RESERVED", NULL,	NOSET },
/*  19,  XXX, */  {     X,        "RESERVED", NULL,	NOSET },

/*  20,  XXX, */  {     X,        "RESERVED", NULL,	NOSET },
/*  21,  XXX, */  {     X,        "RESERVED", NULL,	NOSET },
/*  22,  XXX, */  {     X,        "RESERVED", NULL,	NOSET },
/*  23,  XXX, */  {     X,        "RESERVED", NULL,	NOSET },
/*  24,  XXX, */  {     1,        "oril",     "ori",	ALL_SETS },

/*  25,  XXX, */  {     1,        "oriu",     "oris",	ALL_SETS },
/*  26,  XXX, */  {     1,        "xoril",    "xori",	ALL_SETS },
/*  27,  XXX, */  {     1,        "xoriu",    "xoris",	ALL_SETS },
/*  28,  XXX, */  {     1,        "andil.",   "andi.",	ALL_SETS },
/*  29,  XXX, */  {     1,        "andiu.",   "andis.",	ALL_SETS },

/*  30,  XXX, */  {     X,        "RESERVED", NULL,	NOSET },
/*  31,  XXX, */  {     X,        "RESERVED", NULL,	NOSET },
/*  32,  XXX, */  {     2,        "l",        "lwz",	ALL_SETS },
/*  33,  XXX, */  {     2,        "lu",       "lwzu",	ALL_SETS },
/*  34,  XXX, */  {     2,        "lbz",      NULL,	ALL_SETS },

/*  35,  XXX, */  {     2,        "lbzu",     NULL,	ALL_SETS },
/*  36,  XXX, */  {     2,        "st",       "stw",	ALL_SETS },
/*  37,  XXX, */  {     2,        "stu",      "stwu",	ALL_SETS },
/*  38,  XXX, */  {     2,        "stb",      NULL,	ALL_SETS },
/*  39,  XXX, */  {     2,        "stbu",     NULL,	ALL_SETS },

/*  40,  XXX, */  {     2,        "lhz",      NULL,	ALL_SETS },
/*  41,  XXX, */  {     2,        "lhzu",     NULL,	ALL_SETS },
/*  42,  XXX, */  {     2,        "lha",      NULL,	ALL_SETS },
/*  43,  XXX, */  {     2,        "lhau",     NULL,	ALL_SETS },
/*  44,  XXX, */  {     2,        "sth",      NULL,	ALL_SETS },

/*  45,  XXX, */  {     2,        "sthu",     NULL,	ALL_SETS },
/*  46,  XXX, */  {     2,        "lm",       "lmw",	ALL_SETS },
/*  47,  XXX, */  {     2,        "stm",      "stmw",	ALL_SETS },
/*  48,  XXX, */  {     5,        "lfs",      NULL,	ALL_SETS },
/*  49,  XXX, */  {     5,        "lfsu",     NULL,	ALL_SETS },

/*  50,  XXX, */  {     5,        "lfd",      NULL,	ALL_SETS },
/*  51,  XXX, */  {     5,        "lfdu",     NULL,	ALL_SETS },
/*  52,  XXX, */  {     5,        "stfs",     NULL,	ALL_SETS },
/*  53,  XXX, */  {     5,        "stfsu",    NULL,	ALL_SETS },
/*  54,  XXX, */  {     5,        "stfd",     NULL,	ALL_SETS },

/*  55,  XXX, */  {     5,        "stfdu",    NULL,	ALL_SETS }
};


/*
 * XLform: Table maps XL instructions to description
 *
 *	format		- Describes how to disassemble operands
 *	mnemonic	- PWR mnemonic; default if no PPC mnemonic
 *	alt_mnem	- PPC mnemonic
 *	valid_inst_set	- instruction sets which this is valid on
 */
struct opcode_tab XLform[] = {

/* OPCD   EO       format   mnemonic	   alt_mnem	valid_inst_set */
/* ----   --       ------   --------       --------	-------------- */
/*  19,    0,  */  {   2,   "mcrf",        NULL,	ALL_SETS },
/*  19,   16,  */  {   1,   "bcr or bcrl", "bclr or bclrl",ALL_SETS },
/*  19,   33,  */  {   3,   "crnor",       NULL,	ALL_SETS },
/*  19,   50,  */  {   0,   "rfi",         NULL,	ALL_SETS },
/*  19,  XXX,  */  {   X,   "RESERVED",    NULL,	NOSET },

/*  19,   82,  */  {   0,   "rfsvc",       NULL,	PWR|PWRX },
/*  19,  XXX,  */  {   X,   "RESERVED",    NULL,	NOSET },
/*  19,  XXX,  */  {   X,   "RESERVED",    NULL,	NOSET },
/*  19,  129,  */  {   3,   "crandc",      NULL,	ALL_SETS },
/*  19,  150,  */  {   0,   "ics",         "isync",	ALL_SETS },

/*  19,  XXX,  */  {   X,   "RESERVED",    NULL,	NOSET },
/*  19,  XXX,  */  {   X,   "RESERVED",    NULL,	NOSET },
/*  19,  193,  */  {   3,   "crxor",       NULL,	ALL_SETS },
/*  19,  XXX,  */  {   X,   "RESERVED",    NULL,	NOSET },
/*  19,  225,  */  {   3,   "crnand",      NULL,	ALL_SETS },

/*  19,  XXX,  */  {   X,   "RESERVED",    NULL,	NOSET },
/*  19,  257,  */  {   3,   "crand",       NULL,	ALL_SETS },
/*  19,  XXX,  */  {   X,   "RESERVED",    NULL,	NOSET },
/*  19,  289,  */  {   3,   "creqv",       NULL,	ALL_SETS },
/*  19,  XXX,  */  {   X,   "RESERVED",    NULL,	NOSET },

/*  19,  XXX,  */  {   X,   "RESERVED",    NULL,	NOSET },
/*  19,  XXX,  */  {   X,   "RESERVED",    NULL,	NOSET },
/*  19,  XXX,  */  {   X,   "RESERVED",    NULL,	NOSET },
/*  19,  XXX,  */  {   X,   "RESERVED",    NULL,	NOSET },
/*  19,  XXX,  */  {   X,   "RESERVED",    NULL,	NOSET },

/*  19,  XXX,  */  {   X,   "RESERVED",    NULL,	NOSET },
/*  19,  417,  */  {   3,   "crorc",       NULL,	ALL_SETS },
/*  19,  XXX,  */  {   X,   "RESERVED",    NULL,	NOSET },
/*  19,  449,  */  {   3,   "cror",        NULL,	ALL_SETS },
/*  19,  XXX,  */  {   X,   "RESERVED",    NULL,	NOSET },

/*  19,  XXX,  */  {   X,   "RESERVED",    NULL,	NOSET },
/*  19,  XXX,  */  {   X,   "RESERVED",    NULL,	NOSET },
/*  19,  XXX,  */  {   X,   "RESERVED",    NULL,	NOSET },
/*  19,  528,  */  {   1,   "bcc or bccl", "bcctr or bcctrl", ALL_SETS }
};


/*
 * opcode30: Table maps instructions with opcode of 30 to description
 *
 *	key		- Extended opcode; bits 27 through 31
 *	form		- Instruction format placeholder
 *      format          - format of the operands - also currently used
 *                          to determine instruction format
 *	mnemonic	- PWR mnemonic; default if no PPC mnemonic
 *	alt_mnem	- PPC mnemonic
 *	valid_inst_set	- instruction sets this instruction is valid on
 */

struct opcodeXX opcode30[OPCODE30_SZ] = {

/* key  form  format	mnemonic    alt_mnem     valid_inst_set */
/* ---  ----  ------	--------    --------     -------------- */
{  0,     X,     0,    "rldicl",    NULL,        UNQ_620 },
{  1,     X,     0,    "rldicl.",   NULL,        UNQ_620 },
{  2,     X,     0,    "rldicl",    NULL,        UNQ_620 },
{  3,     X,     0,    "rldicl.",   NULL,        UNQ_620 },
{  4,     X,     0,    "rldicr",    NULL,        UNQ_620 },
{  5,     X,     0,    "rldicr.",   NULL,        UNQ_620 },
{  6,     X,     0,    "rldicr",    NULL,        UNQ_620 },
{  7,     X,     0,    "rldicr.",   NULL,        UNQ_620 },
{  8,     X,     0,    "rldic",     NULL,        UNQ_620 },
{  9,     X,     0,    "rldic.",    NULL,        UNQ_620 },
{ 10,     X,     0,    "rldic",     NULL,        UNQ_620 },
{ 11,     X,     0,    "rldic.",    NULL,        UNQ_620 },
{ 12,     X,     0,    "rldimi",    NULL,        UNQ_620 },
{ 13,     X,     0,    "rldimi.",   NULL,        UNQ_620 },
{ 14,     X,     0,    "rldimi",    NULL,        UNQ_620 },
{ 15,     X,     0,    "rldimi.",   NULL,        UNQ_620 },
{ 16,     X,     1,    "rldcl",     NULL,        UNQ_620 },
{ 17,     X,     1,    "rldcl.",    NULL,        UNQ_620 },
{ 18,     X,     1,    "rldcr",     NULL,        UNQ_620 },
{ 19,     X,     1,    "rldcr.",    NULL,        UNQ_620 },
};

/*
 * opcode31: Table maps instructions with opcode 31 to description of
 *	     disassembly information
 *
 *	key		- Extended opcode, bits 21 through 31 of instruction
 *	form		- Instruction format
 *                          4 == X
 *                          6 == XFX
 *                          8 == XO
 *                          9 == XS
 *	format		- How to disassemble instruction operands
 *	mnemonic	- PWR mnemonic; default if no PPC mnemonic
 *	alt_mnem	- PPC mnemonic
 *	valid_inst_set	- instruction sets this instruction is valid for
 */
struct opcodeXX opcode31[OPCODE31_SZ] = {

/* key  form  format	mnemonic   alt_mnem     valid_inst_set */
/* ---  ----  ------	--------   --------     -------------- */
{   0,    4,      22,	"cmp",     NULL,        ALL_SETS },
{   8,    4,      24,	"t",       "tw",        ALL_SETS },
{   9,    4,      24,	"t",       "tw",        ALL_SETS },
{  16,    8,       1,	"sf",      "subfc",     ALL_SETS },
{  17,    8,       1,	"sf.",     "subfc.",    ALL_SETS },
{  18,    8,       1,	"mulhdu",  NULL,        UNQ_620 },
{  19,    8,       1,	"mulhdu.", NULL,        UNQ_620 },
{  20,    8,       1,	"a",       "addc",      ALL_SETS },
{  21,    8,       1,	"a.",      "addc.",     ALL_SETS },
{  22,    8,       1,	"mulhwu",  NULL,        PPC },
{  23,    8,       1,	"mulhwu.", NULL,        PPC },
{  38,    4,       2,	"mfcr",    NULL,        ALL_SETS },
{  39,    4,       2,	"mfcr",    NULL,        ALL_SETS },
{  40,    4,       7,	"lwarx",   NULL,        PPC },
{  41,    4,       7,	"lwarx",   NULL,        PPC },
{  42,    4,       7,	"ldx",     NULL,        UNQ_620 },
{  43,    4,       7,	"ldx",     NULL,        UNQ_620 },
{  46,    4,       7,	"lx",      "lwzx",      ALL_SETS },
{  47,    4,       7,	"lx",      "lwzx",      ALL_SETS },
{  48,    4,       8,	"sl",      "slw",       ALL_SETS },
{  49,    4,       8,	"sl.",     "slw.",      ALL_SETS },
{  52,    4,       9,	"cntlz",   "cntlzw",    ALL_SETS },
{  53,    4,       9,	"cntlz.",  "cntlzw.",   ALL_SETS },
{  54,    4,       8,	"sld",     NULL,        UNQ_620 },
{  55,    4,       8,	"sld.",    NULL,        UNQ_620 },
{  56,    4,       8,	"and",     NULL,        ALL_SETS },
{  57,    4,       8,	"and.",    NULL,        ALL_SETS },
{  58,    4,       8,	"maskg",   NULL,        PWR|PWRX|UNQ_601 },
{  59,    4,       8,	"maskg.",  NULL,        PWR|PWRX|UNQ_601 },
{  64,    4,      22,	"cmpl",    NULL,        ALL_SETS },
{  65,    4,      22,	"cmpl",    NULL,        ALL_SETS },
{  80,    8,       1,	"subf",    NULL,        PPC },
{  81,    8,       1,	"subf.",   NULL,        PPC },
{ 106,    4,       7,	"ldux",    NULL,        UNQ_620 },
{ 107,    4,       7,	"ldux",    NULL,        UNQ_620 },
{ 108,    4,       6,	"dcbst",   NULL,        PPC },
{ 109,    4,       6,	"dcbst",   NULL,        PPC },
{ 110,    4,       7,	"lux",     "lwzux",     ALL_SETS },
{ 111,    4,       7,	"lux",     "lwzux",     ALL_SETS },
{ 116,    4,       0,	"cntlzd",  NULL,        UNQ_620 },
{ 117,    4,       0,	"cntlzd.", NULL,        UNQ_620 },
{ 120,    4,       8,	"andc",    NULL,        ALL_SETS },
{ 121,    4,       8,	"andc.",   NULL,        ALL_SETS },
{ 136,    4,      24,	"td",      NULL,        UNQ_620 },
{ 137,    4,      24,	"td",      NULL,        UNQ_620 },
{ 146,    8,       1,	"mulhd",   NULL,        UNQ_620 },
{ 147,    8,       1,	"mulhd.",  NULL,        UNQ_620 },
{ 150,    8,       1,	"mulhw",   NULL,        PPC },
{ 151,    8,       1,	"mulhw.",  NULL,        PPC },
{ 166,    4,       2,	"mfmsr",   NULL,        ALL_SETS },
{ 167,    4,       2,	"mfmsr",   NULL,        ALL_SETS },
{ 168,    4,       7,	"ldarx",   NULL,        UNQ_620 },
{ 169,    4,       7,	"ldarx",   NULL,        UNQ_620 },
{ 172,    4,       6,	"dcbf",    NULL,        PPC },
{ 173,    4,       6,	"dcbf",    NULL,        PPC },
{ 174,    4,       7,	"lbzx",    NULL,        ALL_SETS },
{ 175,    4,       7,	"lbzx",    NULL,        ALL_SETS },
{ 208,    8,       0,	"neg",     NULL,        ALL_SETS },
{ 209,    8,       0,	"neg.",    NULL,        ALL_SETS },
{ 214,    8,       1,	"mul",     NULL,        PWR|PWRX|UNQ_601 },
{ 215,    8,       1,	"mul.",    NULL,        PWR|PWRX|UNQ_601 },
{ 236,    4,       6,	"clf",     NULL,        PWR|PWRX },
{ 238,    4,       7,	"lbzux",   NULL,        ALL_SETS },
{ 239,    4,       7,	"lbzux",   NULL,        ALL_SETS },
{ 248,    4,       8,	"nor",     NULL,        ALL_SETS },
{ 249,    4,       8,	"nor.",    NULL,        ALL_SETS },
{ 272,    8,       1,	"sfe",     "subfe",     ALL_SETS },
{ 273,    8,       1,	"sfe.",    "subfe.",    ALL_SETS },
{ 276,    8,       1,	"ae",      "adde",      ALL_SETS },
{ 277,    8,       1,	"ae.",     "adde.",     ALL_SETS },
{ 288,    6,       9,	"mtcrf",   NULL,        ALL_SETS },
{ 289,    6,       9,	"mtcrf",   NULL,        ALL_SETS },
{ 292,    4,       2,	"mtmsr",   NULL,        ALL_SETS },
{ 293,    4,       2,	"mtmsr",   NULL,        ALL_SETS },
{ 298,    4,       7,	"stdx",    NULL,        UNQ_620 },
{ 299,    4,       7,	"stdx",    NULL,        UNQ_620 },
{ 301,    4,       7,	"stwcx.",  NULL,        PPC },
{ 302,    4,       7,	"stx",     "stwx",      ALL_SETS },
{ 303,    4,       7,	"stx",     "stwx",      ALL_SETS },
{ 304,    4,       8,	"slq",     NULL,        PWR|PWRX|UNQ_601 },
{ 305,    4,       8,	"slq.",    NULL,        PWR|PWRX|UNQ_601 },
{ 306,    4,       8,	"sle",     NULL,        PWR|PWRX|UNQ_601 },
{ 307,    4,       8,	"sle.",    NULL,        PWR|PWRX|UNQ_601 },
{ 362,    4,       7,	"stdux",   NULL,        UNQ_620 },
{ 363,    4,       7,	"stdux",   NULL,        UNQ_620 },
{ 366,    4,       7,	"stux",    "stwux",     ALL_SETS },
{ 367,    4,       7,	"stux",    "stwux",     ALL_SETS },
{ 368,    4,      11,	"sliq",    NULL,        PWR|PWRX|UNQ_601 },
{ 369,    4,      11,	"sliq.",   NULL,        PWR|PWRX|UNQ_601 },
{ 400,    8,       0,	"sfze",    "subfze",    ALL_SETS },
{ 401,    8,       0,	"sfze.",   "subfze.",   ALL_SETS },
{ 404,    8,       0,	"aze",     "addze",     ALL_SETS },
{ 405,    8,       0,	"aze.",    "addze.",    ALL_SETS },
{ 420,    4,      17,	"mtsr",    NULL,        ALL_SETS },
{ 421,    4,      17,	"mtsr",    NULL,        ALL_SETS },
{ 429,    4,       7,	"stdcx.",  NULL,        UNQ_620 },
{ 430,    4,       7,	"stbx",    NULL,        ALL_SETS },
{ 431,    4,       7,	"stbx",    NULL,        ALL_SETS },
{ 432,    4,       8,	"sllq",    NULL,        PWR|PWRX|UNQ_601 },
{ 433,    4,       8,	"sllq.",   NULL,        PWR|PWRX|UNQ_601 },
{ 434,    4,       8,	"sleq",    NULL,        PWR|PWRX|UNQ_601 },
{ 435,    4,       8,	"sleq.",   NULL,        PWR|PWRX|UNQ_601 },
{ 464,    8,       0,	"sfme",    "subfme",    ALL_SETS },
{ 465,    8,       0,	"sfme.",   "subfme.",   ALL_SETS },
{ 466,    8,       1,	"mulld",   NULL,        UNQ_620 },
{ 467,    8,       1,	"mulld.",  NULL,        UNQ_620 },
{ 468,    8,       0,	"ame",     "addme",     ALL_SETS },
{ 469,    8,       0,	"ame.",    "addme.",    ALL_SETS },
{ 470,    8,       1,	"muls",    "mullw",     ALL_SETS },
{ 471,    8,       1,	"muls.",   "mullw.",    ALL_SETS },
{ 484,    4,       7,	"mtsri",   "mtsrin",    ALL_SETS },
{ 485,    4,       7,	"mtsri",   "mtsrin",    ALL_SETS },
{ 492,    4,       6,	"dcbtst",  NULL,        PPC },
{ 493,    4,       6,	"dcbtst",  NULL,        PPC },
{ 494,    4,       7,	"stbux",   NULL,        ALL_SETS },
{ 495,    4,       7,	"stbux",   NULL,        ALL_SETS },
{ 496,    4,      11,	"slliq",   NULL,        PWR|PWRX|UNQ_601 },
{ 497,    4,      11,	"slliq.",  NULL,        PWR|PWRX|UNQ_601 },
{ 528,    8,       1,	"doz",     NULL,        PWR|PWRX|UNQ_601 },
{ 529,    8,       1,	"doz.",    NULL,        PWR|PWRX|UNQ_601 },
{ 532,    8,       1,	"cax",     "add",       ALL_SETS },
{ 533,    8,       1,	"cax.",    "add.",      ALL_SETS },
{ 554,    4,       7,	"lscbx",   NULL,        PWR|PWRX|UNQ_601 },
{ 555,    4,       7,	"lscbx.",  NULL,        PWR|PWRX|UNQ_601 },
{ 556,    4,       6,	"dcbt",    NULL,        PPC },
{ 557,    4,       6,	"dcbt",    NULL,        PPC },
{ 558,    4,       7,	"lhzx",    NULL,        ALL_SETS },
{ 559,    4,       7,	"lhzx",    NULL,        ALL_SETS },
{ 568,    4,       8,	"eqv",     NULL,        ALL_SETS },
{ 569,    4,       8,	"eqv.",    NULL,        ALL_SETS },
{ 612,    4,       6,	"tlbi",    "tlbie",
				     PWR|PWRX|UNQ_601|UNQ_603|UNQ_604|UNQ_620 },
{ 613,    4,       6,	"tlbi",    "tlbie",
			 	     PWR|PWRX|UNQ_601|UNQ_603|UNQ_604|UNQ_620 },
{ 620,    4,       7,	"eciwx",   NULL,        UNQ_601|UNQ_603|UNQ_604|UNQ_620 },
{ 621,    4,       7,	"eciwx",   NULL,        UNQ_601|UNQ_603|UNQ_604|UNQ_620 },
{ 622,    4,       7,	"lhzux",   NULL,        ALL_SETS },
{ 623,    4,       7,	"lhzux",   NULL,        ALL_SETS },
{ 632,    4,       8,	"xor",     NULL,        ALL_SETS },
{ 633,    4,       8,	"xor.",    NULL,        ALL_SETS },
{ 662,    8,       1,	"div",     NULL,        PWR|PWRX|UNQ_601 },
{ 663,    8,       1,	"div.",    NULL,        PWR|PWRX|UNQ_601 },
{ 678,    4,       3,	"mfspr",   NULL,        ALL_SETS },
{ 679,    4,       3,	"mfspr",   NULL,        ALL_SETS },
{ 682,    4,       7,	"lwax",    NULL,        UNQ_620 },
{ 683,    4,       7,	"lwax",    NULL,        UNQ_620 },
{ 686,    4,       7,	"lhax",    NULL,        ALL_SETS },
{ 687,    4,       7,	"lhax",    NULL,        ALL_SETS },
{ 720,    8,       0,	"abs",     NULL,        PWR|PWRX|UNQ_601 },
{ 721,    8,       0,	"abs.",    NULL,        PWR|PWRX|UNQ_601 },
{ 726,    8,       1,	"divs",    NULL,        PWR|PWRX|UNQ_601 },
{ 727,    8,       1,	"divs.",   NULL,        PWR|PWRX|UNQ_601 },
{ 740,    4,       1,	"tlbia",   NULL,        UNQ_604 },
{ 741,    4,       1,	"tlbia",   NULL,        UNQ_604 },
{ 742,    6,       2,	"mftb",    NULL,        PPC },
{ 743,    6,       2,	"mftb",    NULL,        PPC },
{ 746,    4,       7,	"lwaux",   NULL,        UNQ_620 },
{ 747,    4,       7,	"lwaux",   NULL,        UNQ_620 },
{ 750,    4,       7,	"lhaux",   NULL,        ALL_SETS },
{ 751,    4,       7,	"lhaux",   NULL,        ALL_SETS },
{ 814,    4,       7,	"sthx",    NULL,        ALL_SETS },
{ 815,    4,       7,	"sthx",    NULL,        ALL_SETS },
{ 824,    4,       8,	"orc",     NULL,        ALL_SETS },
{ 825,    4,       8,	"orc.",    NULL,        ALL_SETS },
{ 868,    4,      26,	"slbie",   NULL,        UNQ_620 },
{ 869,    4,      26,	"slbie",   NULL,        UNQ_620 },
{ 876,    4,       7,	"ecowx",   NULL,        UNQ_601|UNQ_603|UNQ_604|UNQ_620 },
{ 877,    4,       7,	"ecowx",   NULL,        UNQ_601|UNQ_603|UNQ_604|UNQ_620 },
{ 878,    4,       7,	"sthux",   NULL,        ALL_SETS },
{ 879,    4,       7,	"sthux",   NULL,        ALL_SETS },
{ 888,    4,       8,	"or",      NULL,        ALL_SETS },
{ 889,    4,       8,	"or.",     NULL,        ALL_SETS },
{ 914,    8,       1,	"divdu",   NULL,        UNQ_620 },
{ 915,    8,       1,	"divdu.",  NULL,        UNQ_620 },
{ 918,    8,       1,	"divwu",   NULL,        PPC },
{ 919,    8,       1,	"divwu.",  NULL,        PPC },
{ 934,    4,       3,	"mtspr",   NULL,        ALL_SETS },
{ 935,    4,       3,	"mtspr",   NULL,        ALL_SETS },
{ 940,    4,       6,	"dcbi",    NULL,        PPC },
{ 941,    4,       6,	"dcbi",    NULL,        PPC },
{ 952,    4,       8,	"nand",    NULL,        ALL_SETS },
{ 953,    4,       8,	"nand.",   NULL,        ALL_SETS },
{ 976,    8,       0,	"nabs",    NULL,        PWR|PWRX|UNQ_601 },
{ 977,    8,       0,	"nabs.",   NULL,        PWR|PWRX|UNQ_601 },
{ 978,    8,       1,	"divd",    NULL,        UNQ_620 },
{ 979,    8,       1,	"divd.",   NULL,        UNQ_620 },
{ 982,    8,       1,	"divw",    NULL,        PPC },
{ 983,    8,       1,	"divw.",   NULL,        PPC },
{ 996,    4,       1,	"slbia",   NULL,        UNQ_620 },
{ 997,    4,       1,	"slbia",   NULL,        UNQ_620 },
{1004,    4,       6,	"cli",     NULL,        PWR|PWRX },
{1024,    4,      23,	"mcrxr",   NULL,        ALL_SETS },
{1025,    4,      23,	"mcrxr",   NULL,        ALL_SETS },
{1040,    8,       1,	"sfo",     "subfco",    ALL_SETS },
{1041,    8,       1,	"sfo.",    "subfco.",   ALL_SETS },
{1042,    8,       1,	"mulhdu",  NULL,        UNQ_620 },
{1043,    8,       1,	"mulhdu.", NULL,        UNQ_620 },
{1044,    8,       1,	"ao",      "addco",     ALL_SETS },
{1045,    8,       1,	"ao.",     "addco.",    ALL_SETS },
{1046,    8,       1,	"mulhwu",  NULL,        PPC },
{1047,    8,       1,	"mulhwu.", NULL,        PPC },
{1062,    4,       5,	"clcs",    NULL,        PWR|PWRX|UNQ_601 },
{1066,    4,       7,	"lsx",     "lswx",      ALL_SETS },
{1067,    4,       7,	"lsx",     "lswx",      ALL_SETS },
{1068,    4,       7,	"lbrx",    "lwbrx",     ALL_SETS },
{1069,    4,       7,	"lbrx",    "lwbrx",     ALL_SETS },
{1070,    4,      12,	"lfsx",    NULL,        ALL_SETS },
{1071,    4,      12,	"lfsx",    NULL,        ALL_SETS },
{1072,    4,       8,	"sr",      "srw",       ALL_SETS },
{1073,    4,       8,	"sr.",     "srw.",      ALL_SETS },
{1074,    4,       8,	"rrib",    NULL,        PWR|PWRX|UNQ_601 },
{1075,    4,       8,	"rrib.",   NULL,        PWR|PWRX|UNQ_601 },
{1078,    4,       8,	"srd",     NULL,        UNQ_620 },
{1079,    4,       8,	"srd.",    NULL,        UNQ_620 },
{1082,    4,       8,	"maskir",  NULL,        PWR|PWRX|UNQ_601 },
{1083,    4,       8,	"maskir.", NULL,        PWR|PWRX|UNQ_601 },
{1104,    8,       1,	"subfo",   NULL,        PPC },
{1105,    8,       1,	"subfo.",  NULL,        PPC },
{1132,    4,       1,	"tlbsync", NULL,        UNQ_603|UNQ_604|UNQ_620 },
{1133,    4,       1,	"tlbsync", NULL,        UNQ_603|UNQ_604|UNQ_620 },
{1134,    4,      12,	"lfsux",   NULL,        ALL_SETS },
{1135,    4,      12,	"lfsux",   NULL,        ALL_SETS },
{1170,    8,       1,	"mulhd",   NULL,        UNQ_620 },
{1171,    8,       1,	"mulhd.",  NULL,        UNQ_620 },
{1174,    8,       1,	"mulhw",   NULL,        PPC },
{1175,    8,       1,	"mulhw.",  NULL,        PPC },
{1190,    4,      18,	"mfsr",    NULL,        ALL_SETS },
{1191,    4,      18,	"mfsr",    NULL,        ALL_SETS },
{1194,    4,      10,	"lsi",    "lswi",       ALL_SETS },
{1195,    4,      10,	"lsi",    "lswi",       ALL_SETS },
{1196,    4,       1,	"dcs",    "sync",       ALL_SETS },
{1197,    4,       1,	"dcs",    "sync",       ALL_SETS },
{1198,    4,      12,	"lfdx",   NULL,         ALL_SETS },
{1199,    4,      12,	"lfdx",   NULL,         ALL_SETS },
{1232,    8,       0,	"nego",   NULL,         ALL_SETS },
{1233,    8,       0,	"nego.",  NULL,         ALL_SETS },
{1238,    8,       1,	"mulo",   NULL,         PWR|PWRX|UNQ_601 },
{1239,    8,       1,	"mulo.",  NULL,         PWR|PWRX|UNQ_601 },
{1254,    4,       7,	"mfsri",  NULL,         PWR|PWRX },
{1260,    4,       6,	"dclst",  NULL,         PWR|PWRX },
{1262,    4,      12,	"lfdux",  NULL,         ALL_SETS },
{1263,    4,      12,	"lfdux",  NULL,         ALL_SETS },
{1296,    8,       1,	"sfeo",   "subfeo",     ALL_SETS },
{1297,    8,       1,	"sfeo.",  "subfeo.",    ALL_SETS },
{1300,    8,       1,	"aeo",    "addeo",      ALL_SETS },
{1301,    8,       1,	"aeo.",   "addeo.",     ALL_SETS },
{1318,    4,       4,	"mfsrin", NULL,         PPC },
{1319,    4,       4,	"mfsrin", NULL,         PPC },
{1322,    4,       7,	"stsx",   "stswx",      ALL_SETS },
{1323,    4,       7,	"stsx",   "stswx",      ALL_SETS },
{1324,    4,       7,	"stbrx",  "stwbrx",     ALL_SETS },
{1325,    4,       7,	"stbrx",  "stwbrx",     ALL_SETS },
{1326,    4,      12,	"stfsx",  NULL,         ALL_SETS },
{1327,    4,      12,	"stfsx",  NULL,         ALL_SETS },
{1328,    4,       8,	"srq",    NULL,         PWR|PWRX|UNQ_601 },
{1329,    4,       8,	"srq.",   NULL,         PWR|PWRX|UNQ_601 },
{1330,    4,       8,	"sre",    NULL,         PWR|PWRX|UNQ_601 },
{1331,    4,       8,	"sre.",   NULL,         PWR|PWRX|UNQ_601 },
{1390,    4,      12,	"stfsux", NULL,         ALL_SETS },
{1391,    4,      12,	"stfsux", NULL,         ALL_SETS },
{1392,    4,      11,	"sriq",   NULL,         PWR|PWRX|UNQ_601 },
{1393,    4,      11,	"sriq.",  NULL,         PWR|PWRX|UNQ_601 },
{1424,    8,       0,	"sfzeo",  "subfzeo",    ALL_SETS },
{1425,    8,       0,	"sfzeo.", "subfzeo.",   ALL_SETS },
{1428,    8,       0,	"azeo",   "addzeo",     ALL_SETS },
{1429,    8,       0,	"azeo.",  "addzeo.",    ALL_SETS },
{1450,    4,      10,	"stsi",   "stswi",      ALL_SETS },
{1451,    4,      10,	"stsi",   "stswi",      ALL_SETS },
{1454,    4,      12,	"stfdx",  NULL,         ALL_SETS },
{1455,    4,      12,	"stfdx",  NULL,         ALL_SETS },
{1456,    4,       8,	"srlq",   NULL,         PWR|PWRX|UNQ_601 },
{1457,    4,       8,	"srlq.",  NULL,         PWR|PWRX|UNQ_601 },
{1458,    4,       8,	"sreq",   NULL,         PWR|PWRX|UNQ_601 },
{1459,    4,       8,	"sreq.",  NULL,         PWR|PWRX|UNQ_601 },
{1488,    8,       0,	"sfmeo",  "subfmeo",    ALL_SETS },
{1489,    8,       0,	"sfmeo.", "subfmeo.",   ALL_SETS },
{1490,    8,       1,	"mulldo", NULL,         UNQ_620 },
{1491,    8,       1,	"mulldo.",NULL,         UNQ_620 },
{1492,    8,       0,	"ameo",   "addmeo",     ALL_SETS },
{1493,    8,       0,	"ameo.",  "addmeo.",    ALL_SETS },
{1494,    8,       1,	"mulso",  "mullwo",     ALL_SETS },
{1495,    8,       1,	"mulso.", "mullwo.",    ALL_SETS },
{1518,    4,      12,	"stfdux", NULL,         ALL_SETS },
{1519,    4,      12,	"stfdux", NULL,         ALL_SETS },
{1520,    4,      11,	"srliq",  NULL,         PWR|PWRX|UNQ_601 },
{1521,    4,      11,	"srliq.", NULL,         PWR|PWRX|UNQ_601 },
{1552,    8,       1,	"dozo",   NULL,         PWR|PWRX|UNQ_601 },
{1553,    8,       1,	"dozo.",  NULL,         PWR|PWRX|UNQ_601 },
{1556,    8,       1,	"caxo",   "addo",       ALL_SETS },
{1557,    8,       1,	"caxo.",  "addo.",      ALL_SETS },
{1580,    4,       7,	"lhbrx",  NULL,         ALL_SETS },
{1581,    4,       7,	"lhbrx",  NULL,         ALL_SETS },
{1582,    4,      12,	"lfqx",   NULL,         PWRX },
{1584,    4,       8,	"sra",    "sraw",       ALL_SETS },
{1585,    4,       8,	"sra.",   "sraw.",      ALL_SETS },
{1588,    4,       8,	"srad",   NULL,         UNQ_620 },
{1589,    4,       8,	"srad.",  NULL,         UNQ_620 },
{1636,    4,       7,	"rac",    NULL,         PWR|PWRX },
{1637,    4,       7,	"rac.",   NULL,         PWR|PWRX },
{1646,    4,      12,	"lfqux",  NULL,         PWRX },
{1648,    4,      11,	"srai",   "srawi",      ALL_SETS },
{1649,    4,      11,	"srai.",  "srawi.",     ALL_SETS },
{1652,    9,       0,	"sradi",  NULL,         UNQ_620 },
{1653,    9,       0,	"sradi.", NULL,         UNQ_620 },
{1654,    9,       0,	"sradi",  NULL,         UNQ_620 },
{1655,    9,       0,	"sradi.", NULL,         UNQ_620 },
{1686,    8,       1,	"divo",   NULL,         PWR|PWRX|UNQ_601 },
{1687,    8,       1,	"divo.",  NULL,         PWR|PWRX|UNQ_601 },
{1708,    4,       1,	"eieio",  NULL,         PPC },
{1709,    4,       1,	"eieio",  NULL,         PPC },
{1744,    8,       0,	"abso",   NULL,         PWR|PWRX|UNQ_601 },
{1745,    8,       0,	"abso.",  NULL,         PWR|PWRX|UNQ_601 },
{1750,    8,       1,	"divso",  NULL,         PWR|PWRX|UNQ_601 },
{1751,    8,       1,	"divso.", NULL,         PWR|PWRX|UNQ_601 },
{1836,    4,       7,	"sthbrx", NULL,         ALL_SETS },
{1837,    4,       7,	"sthbrx", NULL,         ALL_SETS },
{1838,    4,      12,	"stfqx",  NULL,         PWRX },
{1840,    4,       8,	"sraq",   NULL,         PWR|PWRX|UNQ_601 },
{1841,    4,       8,	"sraq.",  NULL,         PWR|PWRX|UNQ_601 },
{1842,    4,       8,	"srea",   NULL,         PWR|PWRX|UNQ_601 },
{1843,    4,       8,	"srea.",  NULL,         PWR|PWRX|UNQ_601 },
{1844,    4,       9,	"exts",   "extsh",      ALL_SETS },
{1845,    4,       9,	"exts.",  "extsh.",     ALL_SETS },
{1902,    4,      12,	"stfqux", NULL,         PWRX },
{1904,    4,      11,	"sraiq",  NULL,         PWR|PWRX|UNQ_601 },
{1905,    4,      11,	"sraiq.", NULL,         PWR|PWRX|UNQ_601 },
{1908,    4,       0,	"extsb",  NULL,         PPC },
{1909,    4,       0,	"extsb.", NULL,         PPC },
{1938,    8,       1,	"divduo", NULL,         UNQ_620 },
{1939,    8,       1,	"divduo.",NULL,         UNQ_620 },
{1942,    8,       1,	"divwuo", NULL,         PPC },
{1943,    8,       1,	"divwuo.",NULL,         PPC },
{1956,	  4,	  26,	"tlbld",  NULL,		UNQ_603 },
{1957,	  4,	  26,	"tlbld",  NULL,		UNQ_603 },
{1964,    4,       6,	"icbi",   NULL,         PPC },
{1965,    4,       6,	"icbi",   NULL,         PPC },
{1966,    4,      12,	"stfiwx", NULL,         UNQ_603|UNQ_604|UNQ_620 },
{1967,    4,      12,	"stfiwx", NULL,         UNQ_603|UNQ_604|UNQ_620 },
{1972,    4,       0,	"extsw",  NULL,         UNQ_620 },
{1973,    4,       0,	"extsw.", NULL,         UNQ_620 },
{2000,    8,       0,	"nabso",  NULL,         PWR|PWRX|UNQ_601 },
{2001,    8,       0,	"nabso.", NULL,         PWR|PWRX|UNQ_601 },
{2002,    8,       1,	"divdo",  NULL,         UNQ_620 },
{2003,    8,       1,	"divdo.", NULL,         UNQ_620 },
{2006,    8,       1,	"divwo",  NULL,         PPC },
{2007,    8,       1,	"divwo.", NULL,         PPC },
{2020,	  4,	  26,	"tlbli",  NULL,		UNQ_603 },
{2021,	  4,	  26,	"tlbli",  NULL,		UNQ_603 },
{2028,    4,       6,	"dclz",   "dcbz",       ALL_SETS },
{2029,    4,       6,	"dclz",   "dcbz",       ALL_SETS }
};


/*
 * opcode63: Table maps instructions with opcode 63 to description of
 *	     disassembly information; A form are in separate table
 *
 *	key		- Extended opcode; bits 21 through 31 of the instruction
 *	form		- Instruction format
 *                          4 == X
 *                          7 == XFL
 *                          9 == A
 *	format		- How to disassemble operands
 *	mnemonic	- PWR mnemonic; default if no PPC mnemonic
 *	alt_mnem	- PPC mnemonic
 *	valid_inst_set	- instruction sets this instruction is valid on
 */

struct opcodeXX opcode63[OPCODE63_SZ] = {

/* key  form  format	mnemonic   alt_mnem     valid_inst_set */
/* ---  ----  ------	--------   --------     -------------- */
{   0,    4,     19,    "fcmpu",   NULL,        ALL_SETS },
{  24,    4,     21,    "frsp",    NULL,        ALL_SETS },
{  25,    4,     21,    "frsp.",   NULL,        ALL_SETS },
{  28,    4,     21,    "fcir",    "fctiw",	PWRX|PPC },
{  29,    4,     21,    "fcir.",   "fctiw.",	PWRX|PPC },
{  30,    4,     21,    "fcirz",   "fctiwz",	PWRX|PPC },
{  31,    4,     21,    "fcirz.",  "fctiwz.",	PWRX|PPC },
{  64,    4,     19,    "fcmpo",   NULL,        ALL_SETS },
{  65,    4,     19,    "fcmpo",   NULL,        ALL_SETS },
{  76,    4,     16,    "mtfsb1",  NULL,        ALL_SETS },
{  77,    4,     16,    "mtfsb1.", NULL,        ALL_SETS },
{  80,    4,     21,    "fneg",    NULL,        ALL_SETS },
{  81,    4,     21,    "fneg.",   NULL,        ALL_SETS },
{ 128,    4,     14,    "mcrfs",   NULL,        ALL_SETS },
{ 129,    4,     14,    "mcrfs",   NULL,        ALL_SETS },
{ 140,    4,     16,    "mtfsb0",  NULL,        ALL_SETS },
{ 141,    4,     16,    "mtfsb0.", NULL,        ALL_SETS },
{ 144,    4,     21,    "fmr",     NULL,        ALL_SETS },
{ 145,    4,     21,    "fmr.",    NULL,        ALL_SETS },
{ 268,    4,     15,    "mtfsfi",  NULL,        ALL_SETS },
{ 269,    4,     15,    "mtfsfi.", NULL,        ALL_SETS },
{ 272,    4,     21,    "fnabs",   NULL,        ALL_SETS },
{ 273,    4,     21,    "fnabs.",  NULL,        ALL_SETS },
{ 528,    4,     21,    "fabs",    NULL,        ALL_SETS },
{ 529,    4,     21,    "fabs.",   NULL,        ALL_SETS },
{1166,    4,     13,    "mffs",    NULL,        ALL_SETS },
{1167,    4,     13,    "mffs.",   NULL,        ALL_SETS },
{1422,    7,      9,    "mtfsf",   NULL,        ALL_SETS },
{1423,    7,      9,    "mtfsf.",  NULL,        ALL_SETS },
{1628,    4,      21,	"fctid",   NULL,        UNQ_620 },
{1629,    4,      21,	"fctid.",  NULL,        UNQ_620 },
{1630,    4,      21,	"fctidz",  NULL,        UNQ_620 },
{1631,    4,      21,	"fctidz.", NULL,        UNQ_620 },
{1692,    4,      21,	"fcfid",   NULL,        UNQ_620 },
{1693,    4,      21,	"fcfid.",  NULL,        UNQ_620 },
};


/*
 * Aform: Table maps A form instructions for opcode 63 to disassembly
 *		information
 *
 *	key		- Extended opcode; bits 26 through 31 of instruction
 *	form		- Instruction format
 *	format		- How to disassemble instruction operands
 *	mnemonic	- PWR mnemonic; default if no PPC mnemonic
 *	alt_mnem	- PPC mnemonic
 *	valid_inst_set	- instruction sets this instruction is valid on
 */
struct opcodeXX Aform[AFORM_SZ] = {

/* key  form  format	mnemonic   alt_mnem     valid_inst_set */
/* ---  ----  ------	--------   --------     -------------- */
{  36,    9,      0,    "fd",      "fdiv",      ALL_SETS },
{  37,    9,      0,    "fd.",     "fdiv.",     ALL_SETS },
{  40,    9,      0,    "fs",      "fsub",      ALL_SETS },
{  41,    9,      0,    "fs.",     "fsub.",     ALL_SETS },
{  42,    9,      0,    "fa",      "fadd",      ALL_SETS },
{  43,    9,      0,    "fa.",     "fadd.",     ALL_SETS },
{  44,    9,      3,    "fsqrt",   NULL,        PWRX|UNQ_620 },
{  45,    9,      3,    "fsqrt.",  NULL,        PWRX|UNQ_620 },
{  46,    9,      2,    "fsel",    NULL,        UNQ_603|UNQ_604|UNQ_620 },
{  47,    9,      2,    "fsel.",   NULL,        UNQ_603|UNQ_604|UNQ_620 },
{  50,    9,      1,    "fm",      "fmul",      ALL_SETS },
{  51,    9,      1,    "fm.",     "fmul.",     ALL_SETS },
{  52,    9,      3,    "frsqrte", NULL,        UNQ_603|UNQ_604|UNQ_620 },
{  53,    9,      3,    "frsqrte.",NULL,        UNQ_603|UNQ_604|UNQ_620 },
{  56,    9,      2,    "fms",     "fmsub",     ALL_SETS },
{  57,    9,      2,    "fms.",    "fmsub.",    ALL_SETS },
{  58,    9,      2,    "fma",     "fmadd",     ALL_SETS },
{  59,    9,      2,    "fma.",    "fmadd.",    ALL_SETS },
{  60,    9,      2,    "fnms",    "fnmsub",    ALL_SETS },
{  61,    9,      2,    "fnms.",   "fnmsub.",   ALL_SETS },
{  62,    9,      2,    "fnma",    "fnmadd",    ALL_SETS },
{  63,    9,      2,    "fnma.",   "fnmadd.",   ALL_SETS }
};


/*
 * opcode59: Table maps instructions with opcode of 59 to description
 *
 *	key		- Extended opcode; bits 21 through 31
 *	form		- Instruction format
 *	format		- How to disassemble instruction operands
 *	mnemonic	- PWR mnemonic; default if no PPC mnemonic
 *	alt_mnem	- PPC mnemonic
 *	valid_inst_set	- instruction sets this instruction is valid on
 */
struct opcodeXX opcode59[OPCODE59_SZ] = {

/* key  form  format	mnemonic    alt_mnem     valid_inst_set */
/* ---  ----  ------	--------    --------     -------------- */
{  36,     9,     0,    "fdivs",    NULL,        PPC },
{  37,     9,     0,    "fdivs.",   NULL,        PPC },
{  40,     9,     0,    "fsubs",    NULL,        PPC },
{  41,     9,     0,    "fsubs.",   NULL,        PPC },
{  42,     9,     0,    "fadds",    NULL,        PPC },
{  43,     9,     0,    "fadds.",   NULL,        PPC },
{  44,     9,     3,    "fsqrts",   NULL,        UNQ_620 },
{  45,     9,     3,    "fsqrts.",  NULL,        UNQ_620 },
{  48,     9,     3,    "fres",     NULL,        UNQ_603|UNQ_604|UNQ_620 },
{  49,     9,     3,    "fres.",    NULL,        UNQ_603|UNQ_604|UNQ_620 },
{  50,     9,     1,    "fmuls",    NULL,        PPC },
{  51,     9,     1,    "fmuls.",   NULL,        PPC },
{  56,     9,     2,    "fmsubs",   NULL,        PPC },
{  57,     9,     2,    "fmsubs.",  NULL,        PPC },
{  58,     9,     2,    "fmadds",   NULL,        PPC },
{  59,     9,     2,    "fmadds.",  NULL,        PPC },
{  60,     9,     2,    "fnmsubs",  NULL,        PPC },
{  61,     9,     2,    "fnmsubs.", NULL,        PPC },
{  62,     9,     2,    "fnmadds",  NULL,        PPC },
{  63,     9,     2,    "fnmadds.", NULL,        PPC },
};


/*
 * SPR_name: Table maps special purpose register numbers to description; must be
 *	     sorted on value of reg_number
 *
 *	reg_num1	- Special purpose register number, first half
 *	reg_num2	- Special purpose register number, second half
 *	reg_name	- Special purpose register name
 *	valid_inst_set	- instruction sets this register is avialable for
 */
spr_table SPR_name[SPR_SZ] = {

/* reg_number	reg_name	alt_reg_name	valid_inst_set */
/* ----------	--------	------------	-------------- */
{  SPR_MQ,	"mq",		NULL,		PWR|PWRX|UNQ_601 },
{  SPR_XER,	"xer",		NULL,		ALL_SETS },
{  SPR_RTCU1,	"rtcu",		NULL,		PWR|PWRX },
{  SPR_RTCL1,	"rtcl",		NULL,		PWR|PWRX|UNQ_601 },
{  SPR_DEC1,	"dec",		NULL,		PWR|PWRX|UNQ_601 },
{  SPR_LR,	"lr",		NULL,		ALL_SETS },
{  SPR_CTR,	"ctr",		NULL,		ALL_SETS },
{  SPR_IMR,	"imr",		NULL,		PWRX },
{  SPR_HID0,	"hid0",		NULL,		UNQ_601|UNQ_604 },
{  SPR_TID,	"tid",		"hid1",		PWR|PWRX|UNQ_601 },
{  SPR_DSISR,	"dsisr",	NULL,		PWR|PWRX|PPC },
{  SPR_DAR,	"dar",		NULL,		PWR|PWRX|PPC },
{  SPR_RTCU2,	"rtcu",		NULL,		PWRX },
{  SPR_DABR,	"rtcl",		"dabr",		PWRX|UNQ_601|UNQ_604 },
{  SPR_DEC2,	"dec",		NULL,		PWRX|PPC },
{  SPR_SDR0,	"sdr0",		NULL,		PWR|PWRX },
{  SPR_SDR1,	"sdr1",		NULL,		PWR|PWRX|PPC },
{  SPR_SRR0,	"srr0",		NULL,		PWR|PWRX|PPC },
{  SPR_SRR1,	"srr1",		NULL,		PWR|PWRX|PPC },
{  SPR_DSAR,	"dsar",		NULL,		PWRX },
{  SPR_TSR,	"tsr",		NULL,		PWRX },
{  SPR_ILCR,	"ilcr",		NULL,		PWRX },
{  SPR_PID,	"pid",		NULL,		UNQ_601|UNQ_604 }
};


/*
 * NAME: TO_ext
 *
 * FUNCTION: common trap options   
 *
 * PARAMETERS:
 *	TO	- trap option
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: NULL if no common trap option; otherwise returns common trap option
 */
char *TO_ext(TO)
unsigned long TO;
{
	switch (TO) {
		case TO_LT: 	        return("lt");
		case (TO_LT | TO_EQ):   return("le");
		case (TO_LT | TO_GT):   return("ne");
		case TO_GT: 	        return("gt");
		case (TO_GT | TO_EQ):   return("ge");
		case TO_LLT: 	        return("llt");
		case (TO_LLT | TO_EQ):  return("lle");
		case (TO_LLT | TO_LGT): return("lne");
		case TO_LGT: 	        return("lgt");
		case (TO_LGT | TO_EQ):  return("lge");
		case TO_EQ: 	        return("eq");
		default:		return((char *)0);
	}
}
