/* @(#)83	1.7  src/bos/usr/include/typchk.h, cmdld, bos411, 9428A410j 3/24/93 21:39:59 */
#ifndef _H_TYPCHK
#define _H_TYPCHK
/*
 * COMPONENT_NAME: (CMDLD) XCOFF object file format definition
 *
 * FUNCTIONS: typchk.h 
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* There is one type check entry for every
 * parameter type checking hash string referenced
 * from the symbol table.
 *
 * The defined format for each entry is:
 * 2-Byte ... length field = 10 
 * 2-byte ... language id field as defined by the TB_<lang> in storclass.h. 
 * 4-byte ... general hash field
 * 4-byte ... language hash field
 *
 * References to the typchk string are to the language id portion (not the
 * length portion) and is an offset from the beginning of the ".typchk" 
 * raw data section.
 */

#define TYPCHKSZ	10		/* length of each typcheck entry */
					/* Do not use sizeof() */

typedef struct typchk{
	unsigned short	t_lang;		/* language id */
	unsigned char	t_ghash[4];	/* general hash field */
	unsigned char	t_lhash[4];	/* language specific hash field */
	} TYPCHK;

#endif /* _H_TYPCHK */
