/* @(#)75	1.6  src/bos/usr/include/loader.h, cmdld, bos411, 9428A410j 3/24/93 21:37:53 */
#ifndef	_H_LOADER
#define	_H_LOADER
/*
 * COMPONENT_NAME: (CMDLD) XCOFF object file format definition
 *
 * FUNCTIONS: loader.h 
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

/* Need definitions from these files	*/
#include <syms.h>
#include <reloc.h>

/* Header portion	*/
typedef struct ldhdr
{
	long	l_version;	/* Loader section version number	*/
	long	l_nsyms;	/* Qty of loader Symbol table entries	*/
	long	l_nreloc;	/* Qty of loader relocation table entries */
	ulong	l_istlen;	/* Length of loader import file id strings */
	long	l_nimpid;	/* Qty of loader import file ids.	*/
	ulong	l_impoff;	/* Offset to start of loader import	*/
				/*	file id strings			*/
	ulong	l_stlen;	/* Length of loader string table	*/
	ulong	l_stoff;	/* Offset to start of loader string table */
}LDHDR;

#define	LDHDRSZ	sizeof(LDHDR)

/* Symbol table portion	*/
typedef struct ldsym
{
	union
	{
		char		_l_name[SYMNMLEN];	/* Symbol name	*/
		struct
		{
			long	_l_zeroes;	/* offset if 0		*/
			long	_l_offset;	/* offset into loader string */
		} _l_l;
		char		*_l_nptr[2];	/* allows for overlaying */
	} _l;
	unsigned long		l_value;	/* Address field	*/
	short			l_scnum;	/* Section number	*/
	char			l_smtype;	/* type and imp/exp/eps	*/
						/* 0	Unused		*/
						/* 1	Import		*/
						/* 2	Entry point	*/
						/* 3	Export		*/
						/* 4	Unused		*/
						/* 5-7	Symbol type	*/
	char			l_smclas;	/* storage class	*/
	long			l_ifile;	/* import file id	*/
						/*	string offset	*/
	long			l_parm;		/* type check offset	*/
						/*	into loader string */
}LDSYM;

#define	LDSYMSZ	sizeof(LDSYM)

#define	l_name		_l._l_name
#define	l_nptr		_l._l_nptr[1]
#define	l_zeroes	_l._l_l._l_zeroes
#define	l_offset	_l._l_l._l_offset

#define	L_EXPORT	0x10
#define	L_ENTRY		0x20
#define	L_IMPORT	0x40
#define	LDR_EXPORT(x)	((x).l_smtype & L_EXPORT)
#define	LDR_ENTRY(x)	((x).l_smtype & L_ENTRY)
#define	LDR_IMPORT(x)	((x).l_smtype & L_IMPORT)
#define	LDR_TYPE(x)	((x).l_smtype & 0x07)

/* Relocation portion	*/
typedef struct ldrel
{
	ulong		l_vaddr;	/* Address field		*/
	long		l_symndx;	/* Loader symbol table index of */
					/* reloc value to apply. This field */
					/* is zero based where 0,1,2 are */
					/* text,data,bss and 3 is the first */
					/* symbol entry from above */
	unsigned short	l_rtype;	/* relocation type		*/
	short		l_rsecnm;	/* section number being relocated */
					/* one based index in scnhdr table */
}LDREL;

#define	LDRELSZ	sizeof(LDREL)

#endif	/* _H_LOADER */
