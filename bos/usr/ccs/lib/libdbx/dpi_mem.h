/* @(#)34	1.3  src/bos/usr/ccs/lib/libdbx/dpi_mem.h, libdbx, bos411, 9428A410j 1/20/92 17:50:06 */
/*
 * COMPONENT_NAME: (libdbx) xde header file.
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/* 	
	DisStruct is used to store the information necessary for
	the disassembly windoe for xde.
*/
typedef struct dis {
	char * 	filename;
	int	lineno;
	unsigned int 	addr,
			hex_instr;
	char *		symb_addr;
	char 		mnemonic[8];
	char *		instruction;
} DisStruct;

typedef struct dpi_ldinfo {
	char	filename[255];
	uint 	textorg;
	uint 	dataorg;
	uint 	textsize;
	uint	datasize;
} MAP;

extern int loadcnt;
