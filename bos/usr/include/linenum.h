/* @(#)65	1.7  src/bos/usr/include/linenum.h, cmdld, bos411, 9428A410j 3/24/93 21:37:34 */
#ifndef	_H_LNUM 
#define _H_LNUM
/*
 * COMPONENT_NAME: (CMDLD) XCOFF object file format definition
 *
 * FUNCTIONS: linenum.h 
 *
 * ORIGINS: 3, 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* There is one line number entry for every 
 * "breakpointable" source line in a code section.
 * Line numbers are grouped on a per function
 * basis; the first entry in a function grouping
 * will have l_lnno == 0 and in place of physical
 * address will be the symbol table index of
 * the function name.
 */
typedef struct lineno
{
	union
	{
		long	l_symndx ;	/* sym. table index of function name
						if l_lnno == 0	*/
		long	l_paddr ;	/* (physical) address of line number */
	}		l_addr ;
	unsigned short	l_lnno ;	/* line number */
} LINENO;

#define	LINESZ	6	/* Do not use sizeof(LINENO) */
#endif	/* _H_LNUM  */
