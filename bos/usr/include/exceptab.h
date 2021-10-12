/* @(#)65	1.2  src/bos/usr/include/exceptab.h, cmdld, bos411, 9428A410j 3/24/93 21:36:52 */

#ifndef	_H_EXCEPTAB
#define _H_EXCEPTAB

/*
 * COMPONENT_NAME: (CMDLD) XCOFF object file format definition
 *
 * FUNCTIONS: exceptab.h (Exception table structure)
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/********************************************************************** 
 *
 *	Structure used to define the .except section of the object file.
 *
 **********************************************************************/

/* When generated, there is one exception table entry for each
 * trap instruction in the executable code.
 * Exception entries are grouped on a per function
 * basis; the first entry in a function grouping
 * will have e_reason = 0 and in place of physical
 * address will be the symbol table index of
 * the function name.
 */
typedef struct exceptab
{
	union
	{
		long	e_symndx ;	/* sym. table index of function name
						if e_reason == 0	*/
		long	e_paddr ;	/* (physical) address of trap inst */
	}		e_addr ;
	char	e_lang ;		/* compiler language id code */
	char	e_reason ;		/* exception reason code */
} EXCEPTAB;

#define	EXCEPTSZ	6	/* Do not use sizeof(EXCEPTAB) */
#endif	/* _H_EXCEPTAB  */
