static char sccsid[] = "@(#)79	1.9.2.17  src/bos/usr/ccs/bin/as/asmsgs.c, cmdas, bos411, 9428A410j 6/8/94 19:16:50";
/*
 * COMPONENT_NAME: (CMDAS) Assembler and Macroprocessor 
 *
 * FUNCTIONS: 
 *
 * ORIGINS:  3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/***************************************************************************
****************************** MESSAGE DEFINITIONS *************************
***************************************************************************/

char * msg_defs[] ={


/* message 001 */ "Redefinition of %s\n",	

/* message 002 */ "Nesting overflow\n",	
 
/* message 003 */ ".set operand undefined or forward reference\n",
 
/* message 004 */ "Invalid .globl symbol\n",
 
/* message 005 */ "invalid storage class\n",
 
/* message 006 */ "unknown ERRTOK in icsect errtok\n",
 
/* message 007 */ "alignment must be an absolute expression\n",
 
/* message 008 */ "illegal TOCOF name1\n",
 
/* message 009 */ "Begin/End block pseudo-op missing\n",
 
/* message 010 */ "illegal TOCOF name2\n",
 
/* message 011 */ "Illegal forward reference in .space\n",
 
/* message 012 */ ".space size not absolute\n",
 
/* message 013 */ ".space size is negative\n",
 
/* message 014 */ "illegal symbol type for rename\n",
 
/* message 015 */ "Invalid expression\n",
 
/* message 016 */ "invalid opcode or pseudo-op\n",
 
/* message 017 */ "unknown ERRTOK in errtok args\n",
 
/* message 018 */ "\".tc\" only allowed inside \".toc\" scope\n",
 
/* message 019 */ ".byte or .vbyte exp is externally defined \n",
 
/* message 020 */ ".short exp is externally defined \n",
 
/* message 021 */ "expr must be absolute\n",
 
/* message 022 */ "expr must be between 1 and 4\n",
 
/* message 023 */ "undefined symbol \"%s\"\n",
 
/* message 024 */ ".stab string does not contain \":\"\n",
 
/* message 025 */ "Invalid register, base register, or mask argument\n",
 
/* message 026 */ "Cannot create temporary file\n",
 
/* message 027 */ 
 "Warning - aligning with zeroes: .short not on half word boundary\n",
 
/* message 028 */ "Cannot reopen temporary file\n",
 
/* message 029 */ "error allocating sections\n",
 
/* message 030 */ "Cannot create %s\n",
 
/* message 031 */ "error allocating ESD\n",
 
/* message 032 */ "error allocating RLD\n",
 
/* message 033 */ "error allocating STRING TABLE\n",
 
/* message 034 */ "error allocating LINENO\n",
 
/* message 035 */ "Filename argument to -%c missing on Command Line\n",
 
/* message 036 */ "Unknown keyword option %s on Command Line\n",
 
/* message 037 */ "Only one input file allowed\n",
 
/* message 038 */ "Cannot open file %s\n",
 
/* message 039 */ "illegal VBYTE expression\n",
 
/* message 040 */ "illegal expression\n",
 
/* message 041 */ "Constant division by zero\n",
 
/* message 042 */ "Internal error: unknown operator\n",
 
/* message 043 */ "Invalid relocatable assembler expression\n",
 
/* message 044 */ "Invalid source character 0x%x\n",
 
/* message 045 */ "cannot open list file %s\n",
 
/* message 046 */ "cannot open temp source file %s\n",
 
/* message 047 */ "Nesting underflow\n",
 
/* message 048 */ "invalid type encountered when building External Symbols\n",
 
/* message 049 */ "HSH string table overflow\n",
 
/* message 050 */ "error allocating DEBUG TABLE\n" ,
 
/* message 051 */ "invalid sclass type of num=%d encountered\n",
 
/* message 052 */ "Invalid \".align\" argument\n",
 
/* message 053 */ ".org argument out of current csect\n",
 
/* message 054 */ "Invalid register number in .using\n",
 
/* message 055 */ "Invalid base address in .using\n",
 
/* message 056 */ "can only have a using on beginning of TOC\n",
 
/* message 057 */ "illegal external expression\n",
 
/* message 058 */ "Warning - label %s aligned with %s csect\n",
 
/* message 059 */ "Invalid register in .drop\n",
 
/* message 060 */ "Register in .drop not in use\n",
 
/* message 061 */ "Invalid toc entry within .toc scope\n",
 
/* message 062 */ "alignment must be between 0 and 31\n",
 
/* message 063 */ "illegal name for csect\n",
 
/* message 064 */ "Common size not absolute\n",
 
/* message 065 */ "Illegal operation\n",
 
/* message 066 */ "error allocating HASH (.hash) TABLE\n",	
 
/* message 067 */ "Illegal common storage class\n",
 
/* message 068 */ "hash already set for \"%s\"\n",
 
/* message 069 */ "illegal character (%c) in hash string\n",
 
/* message 070 */ "illegal symbol or symbol type for hash value\n",
 
/* message 071 */ "cannot assign function sclass\n",
 
/* message 072 */ "symbol not located in symbol table\n",
 
/* message 073 */ "Memory overflow\n",
 
/* message 074 */ "pseudo-op not within text sect\n",
 
/* message 075 */ "wrong number of arguments\n",
 
/* message 076 */ "unable to assign sect to linenumber\n",
 
/* message 077 */ "file_table full\n",
 
/* message 078 */ "illegal bit mask starting at %d\n",
 
/* message 079 */ "invalid type encountered when counting rlds\n",
 
/* message 080 */ "Branch target not on word boundry\n" ,
 
/* message 081 */ "Misaligned instruction\n",
 
/* message 082 */ "Too few arguments\n",
 
/* message 083 */ "Too many arguments\n",
 
/* message 084 */ "Internal error: Pass 1 op botch\n",
 
/* message 085 */ "illegal branch and execute subject instruction\n",
 
/* message 086 */ "address cannot be absolute\n",
 
/* message 087 */ "illegal expression type for branch address\n",
 
/* message 088 */ "branch address out of range\n",
 
/* message 089 */ "branch address not on halfword boundary\n",
 
/* message 090 */ "Invalid condition status bit specified\n",
 
/* message 091 */ "Invalid immediate operand\n",
 
/* message 092 */ "Invalid relocatable reference\n",
 
/* message 093 */ "Invalid relocatable displacement\n",
 
/* message 094 */ "Invalid relocatable assembler expression\n",
 
/* message 095 */ "Invalid condition status bit specified\n",
 
/* message 096 */ "Invalid branch address\n",
 
/* message 097 */ "Short displacement out of range\n",
 
/* message 098 */ "displacement not aligned properly\n",
 
/* message 099 */ "Invalid displacement\n",
 
/* message 100 */ "invalid d(r) argument\n",
 
/* message 101 */ "Invalid condition status bit specified\n",
 
/* message 102 */ "Jump target out of range\n",
 
/* message 103 */ "Instruction not supported\n",
 
/* message 104 */ "argument %d must be absolute\n",
 
/* message 105 */ "Invalid mask or register argument\n",
 
/* message 106 */ "displacement out of range\n",

/* message 107 */ "argument %d out of range\n",
 
/* message 108 */ "Warning - the alignment for label %s is incorrect\n" , 
 
/* message 109 */ 
 "Warning - aligning with zeroes:  not on full word boundary\n",
/* "Warning - aligning with zeroes: .long not on full word boundary\n",*/

/* message 110 */ "Warning - aligning with zeroes in program csect\n",

/* message 111 */ "Warning - Csect alignment has changed\n",

/* message 112 */ "Warning - illegal or not implemented yet \"%c\"\n",

/* message 113 */ "Warning - jump instruction can be used\n",

/* message 114 */ "Warning - short form can be used\n",

/* message 115 */ "Sort failed with status %d\n",
 
/* message 116 */ "Fatal error from %s\n",
 
/* message 117 */ "Assembler:\n",
 
/* message 118 */ "line %d: ", 

/* message 119 */ ".xref",

/* message 120 */ ".lst",

/* message 121 */ 
" SYMBOL          FILE            CSECT                   LINENO        \n",

/* message 122 */ "\n %s                              ", 

/* message 123 */ "\n %s                              ", 

/* message 124 */ 
 "\nFile# Line#  Name    Loc Ctr   Object Code         Source\n\n", 

/* message 125 */ "                          ",

/* message 126 */ "        ",

/* message 127 */ "          | ",

/* message 128 */ "        %.8x  ", 

/* message 129 */ "                  ",

/* message 130 */ "        %.8x  ", 

/* message 131 */ " %-6.6s %.8x  ",

/* message 132 */ "file#\tfile name\n",

/* message 133 */ "%3d\t%s\n",

/* message 134 */ "%-2d %6d | ", 

/* message 135 */ "        ",

/* message 136 */ "  ....  ",

/* message 137 */ "%-8.2x", 

/* message 138 */ "%-8.4x", 

/* message 139 */ "%-8.6x", 

/* message 140 */ "%.8x", 

/* message 141 */ "error ocurred in collect pointer\n",

/* message 142 */ "Error In Syntax \n",

/* message 143 */ ".function size not absolute\n",

/* message 144 */ "Warning - required initialized data ignored for %s csect\n",
 
/* message 145 */ 
 "\nFile# Line#  Name    Loc Ctr   Object Code  PowerPC     Source\n\n", 

/* message 146 */ 
 "\nFile# Line#  Name    Loc Ctr   Object Code  POWER       Source\n\n", 

/* message 147 */ "Invalid .machine assembly mode operand: %s\nValid values are:",

/* message 148 */ "Invalid .source language identifier operand: %s\n",

/* message 149 */ "Instruction %s is not implemented in the current assembly mode %s.\n",

/* message 150 */ "The first operand value of %d is not valid for PowerPC.\n A BO field of  6, 7, 14, 15, or greater than 20 is not valid.\n",

/* message 151 */ "This instruction form is not valid for PowerPC.\nThe register used in operand two must not be zero and\nmust not be the same as the register used in operand one.\n",


/* message 152 */ "Internal error related to the source program domain.\n Depending upon where you acquired this product, contact\nyour service representative or your approved supplier.\n",

/* message 153 */ "Warning - Instruction %s functions differently between POWER and PowerPC.\n",

/* message 154 */ "Ths second operand is not valid. For 32-bit\nimplementation, the second operand must have a value of zero.\n",

/* message 155 */ "Displacement must be divisible by 4.\n",

/* message 156 */ "The sum of operands 3 and 4 must be less than 33.\n",

/* message 157 */ "The value of operand 3 must be greater\nthan or equal to the value of operand 4.\n",

/* message 158 */ "Warning - Special-purpose register number 6 is used\nto designate the DEC register when the assembly mode is %s.\n",

/* message 159 */ " The d(r) format is not valid for operand %d.\n",

/* message 160 */ "Warning - A hash code value should be 10 bytes long.\n",

/* message 161 */ "A system problem occurred while processing file %s\n",

/* message 162 */ "Invalid -m flag assembly mode operand: %s\nValid values are:",

/* message 163 */ "The first operand's value (%d) is not valid for PowerPC.\nThe third bit of the BO field must be one for\nthe Branch-Conditional-to-Count-Register instruction.\n",

/* message 164 */ "This instructions form is not valid for PowerPC.\nRA, and RB if present in the instruction, cannot be in the range\n of registers to be loaded. Also, RA=RT=0 is not allowed.\n",
 
/* message 165 */ "The value of the first operand must be zero for PowerPC.\n",

/* message 166 */ "This instruction form is not valid for PowerPC.\nThe register used in operand two must not be zero.\n",

/* message 167 */" Specify a name with the -%c flag.\n",

/* message 168 */" %s is not a recognized flag.\n",

/* message 169 */ "Only one input file is allowed.\n",

/* message 170 */ "Usage: as -l[ListFile] -s[ListFile] -n Name -o ObjectFile\n\t\t\t[-w|-W] -x[XCrossFile] -u -m ModeName [InputFile]\n",

/* message 171 */ "The displacement must be greater than or equal to\n\
\t%d and less than or equal to %d.\n",

/* message 172 */ " The .extern symbol is not valid.\n",

/* message 173 */ "Warning - The immediate value for instruction %s is %d\n\
It may not be portable to a 64-bit machine if this value is to be treated \n\
as an unsigned value.\n",
/* message 174 */ "Too many .machine \"push\" instructions without \n\
corresponding .machine \"pop\" instructions.\n",

/* message 175 */"A .machine \"pop\" is seen without a matching \n\
.machine \"push\".\n",

/* message 176 */ "The .ref pseudo-op cannot appear in section %s.\n",

/* message 177 */ "The operand of the .ref %s is not a relocatable symbol.\n",

/* message 178 */ "The maximum number of sections or symbols that an expression \n\
can refer to has been exceeded.\n", 

/* message 179 */ 
 "\nFile# Line#  Mode Name    Loc Ctr   Object Code         Source\n\n", 

/* message 180 */ 
 "\nFile# Line#  Mode Name    Loc Ctr   Object Code  PowerPC     Source\n\n", 

/* message 181 */ 
 "\nFile# Line#  Mode Name    Loc Ctr   Object Code  POWER       Source\n\n",

/* message 182 */
 "Warning - Storage mapping class %s is not valid for .comm\n\
pseudo-op. RW is used as the storage mapping class for the object code.\n",

/* message 183 */ "TD csect only allowed inside \".toc\" scope\n",

/* message 184  */
"TOC anchor must be defined to use a TOC-relative\n\
reference to %s. Include a .toc pseudo op in the source.\n",

/* message 185 */
"Warning - Operand is missing from pseudo-op.\n"
};

int  num_msgs = sizeof msg_defs / sizeof msg_defs[0]; 
