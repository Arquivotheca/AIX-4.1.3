/* @(#)48	1.3.1.3  src/bos/usr/include/reloc.h, cmdld, bos411, 9428A410j 8/23/93 14:07:57 */
#ifndef	_H_RELOC
#define _H_RELOC
/*
 * COMPONENT_NAME: (CMDLD) XCOFF object file format definition
 *
 * FUNCTIONS: reloc.h
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
#include <sys/types.h>

typedef struct reloc {
	ulong	r_vaddr;	/* (virtual) address of reference */
	ulong	r_symndx;	/* index into symbol table */
	union
	{
		unsigned short _r_type;	/* old style coff relocation type */
		struct
			{
			char	_r_rsize;	/* sign and reloc bit len */
			char	_r_rtype;	/* toc relocation type */
			} _r_r;
	} _r;
}RELOC;

#define r_type	_r._r_type	/* old style relocation - original name */
#define r_rsize	_r._r_r._r_rsize	/* extract sign and bit len */
#define r_rtype	_r._r_r._r_rtype	/* extract toc relocation type */

#define R_LEN	0x1F		/* extract bit-length field */
#define R_SIGN	0x80		/* extract sign of relocation */
#define R_FIXUP	0x40		/* extract code-fixup bit */

#define RELOC_RLEN(x)	((x)._r._r_r._r_rsize & R_LEN)
#define RELOC_RSIGN(x)	((x)._r._r_r._r_rsize & R_SIGN)
#define RELOC_RFIXUP(x)	((x)._r._r_r._r_rsize & R_FIXUP)
#define RELOC_RTYPE(x)	((x)._r._r_r._r_rtype)

/*
 *	POWER and PowerPC - relocation types
 */
#define	R_POS	0x00	/* A(sym) Positive Relocation	*/
#define R_NEG	0x01	/* -A(sym) Negative Relocation	*/
#define R_REL	0x02	/* A(sym-*) Relative to self	*/
#define	R_TOC	0x03	/* A(sym-TOC) Relative to TOC	*/
#define R_TRL	0x12	/* A(sym-TOC) TOC Relative indirect load. */
			/*            modifiable instruction */
#define R_TRLA	0x13	/* A(sym-TOC) TOC Rel load address. modifiable inst */
#define R_GL	0x05	/* A(external TOC of sym) Global Linkage */
#define R_TCL	0x06	/* A(local TOC of sym) Local object TOC address */
#define R_RL	0x0C	/* A(sym) Pos indirect load. modifiable instruction */
#define R_RLA	0x0D	/* A(sym) Pos Load Address. modifiable instruction */
#define R_REF	0x0F	/* AL0(sym) Non relocating ref. No garbage collect */
#define R_BA	0x08	/* A(sym) Branch absolute. Cannot modify instruction */
#define R_RBA	0x18	/* A(sym) Branch absolute. modifiable instruction */
#define	R_RBAC	0x19	/* A(sym) Branch absolute constant. modifiable instr */
#define	R_BR	0x0A	/* A(sym-*) Branch rel to self. non modifiable */
#define R_RBR	0x1A	/* A(sym-*) Branch rel to self. modifiable instr */
#define R_RBRC	0x1B	/* A(sym-*) Branch absolute const. */
			/* 		modifiable to R_RBR */
#define R_RTB	0x04	/* A((sym-*)/2) RT IAR Rel Branch. non modifiable */
#define R_RRTBI	0x14	/* A((sym-*)/2) RT IAR Rel Br. modifiable to R_RRTBA */
#define R_RRTBA	0x15	/* A((sym-*)/2) RT absolute br. modifiable to R_RRTBI */


#define	RELSZ	10	/* Do not use sizeof(RELOC) */


/*
 *	original style - relocation types from coff
 *	UNUSED: The #defines retained for compatability
 */
#define R_ABS		0
#define	R_OFF8		07
#define R_OFF16		010
#define	R_SEG12		011
#define	R_AUX		013
#define R_DIR16	01
#define R_REL16	02
#define R_IND16	03
#define R_DIR24	04
#define R_REL24	05
#define R_OPT16	014
#define R_IND24	015
#define R_IND32	016
#define	R_DIR10		025
#define R_REL10		026
#define R_REL32		027
#define R_DIR32S	012
#define R_RELBYTE	017
#define R_RELWORD	020
#define R_RELLONG	021
#define R_PCRBYTE	022
#define R_PCRWORD	023
#define R_PCRLONG	024


#endif /* _H_RELOC */
