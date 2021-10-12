/* @(#)00	1.10  src/bos/usr/include/storclass.h, cmdld, bos41B, 9504A 12/7/94 15:30:58 */
#ifndef	_H_STCLASS
#define _H_STCLASS
/*
 * COMPONENT_NAME: (CMDLD) XCOFF object file format definition
 *
 * FUNCTIONS: storclass.h
 *
 * ORIGINS: 3, 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *	STORAGE CLASSES - n_sclass field of syment
 */

#define C_EFCN		255	/* physical end of function */
#define C_NULL		0	/* logically deleted symbol table entry */
#define C_AUTO		1	/* automatic variable */
#define C_EXT		2	/* external symbol */
#define C_STAT		3	/* static */
#define C_REG		4	/* register variable */
#define C_EXTDEF	5	/* external definition */
#define C_LABEL		6	/* label */
#define C_ULABEL	7	/* undefined label */
#define C_MOS		8	/* member of structure */
#define C_ARG		9	/* function argument */
#define C_STRTAG	10	/* structure tag */
#define C_MOU		11	/* member of union */
#define C_UNTAG		12	/* union tag */
#define C_TPDEF		13	/* type definition */
#define C_USTATIC	14	/* undefined static */
#define C_ENTAG		15	/* enumeration tag */
#define C_MOE		16	/* member of enumeration */
#define C_REGPARM	17	/* register parameter */
#define C_FIELD		18	/* bit field */
#define C_BLOCK		100	/* ".bb" or ".eb" */
#define C_FCN		101	/* ".bf" or ".ef" */
#define C_EOS		102	/* end of structure */
#define C_FILE		103	/* file name */
#define C_LINE		104
#define C_ALIAS		105	/* duplicate tag */
#define C_HIDDEN	106	/* special storage class for external */
				/* symbols in dmert public libraries */
#define	C_HIDEXT	107	/* Un-named external symbol */
#define	C_BINCL		108	/* Marks beginning of include file */
#define	C_EINCL		109	/* Marks ending of include file */
#define	C_INFO		110	/* Comment string in .info section */

#include <dbxstclass.h>

/*	Only valid n_value for a C_NULL (deletable) symbol table entry */
#define C_NULL_VALUE	0x00DE1E00

/*	Source Language Identifiers   */

#ifndef TB_C
/*		Following are also defined in <sys/debug.h> */
#define	TB_C		0	/* C */
#define	TB_FORTRAN	1	/* Fortran */
#define	TB_PASCAL	2	/* Pascal */
#define	TB_ADA		3	/* Ada */
#define	TB_PLI		4	/* PL/I */
#define	TB_BASIC	5	/* BASIC */
#define	TB_LISP		6	/* LISP */
#define	TB_COBOL	7	/* COBOL */
#define	TB_MODULA2	8	/* Modula2 */
#define	TB_CPLUSPLUS	9	/* C++ */
#define	TB_RPG		10	/* RPG */
#define	TB_PL8		11	/* PL8, PLIX */
#define	TB_ASM		12	/* Assembly */
#define TB_RESERVED_1	251
#define TB_RESERVED_2	252
#define TB_RESERVED_3	253
#define TB_RESERVED_4	254
#define TB_RESERVED_5	255
#endif

#define TB_OBJECT	248	/* Object File	*/
#define TB_FRONT	249	/* entries collected to start of symbol table */
#define TB_BACK		250	/* entries collected to end of symbol table */


#endif /* _H_STCLASS */
