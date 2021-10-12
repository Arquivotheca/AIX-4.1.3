/* @(#)78	1.9  src/bos/usr/include/filehdr.h, cmdld, bos411, 9428A410j 9/11/93 14:11:24 */
#ifndef	_H_FHDR 
#define _H_FHDR
/*
 * COMPONENT_NAME: (CMDLD) XCOFF object file format definition
 *
 * FUNCTIONS: filehdr.h 
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

typedef struct filehdr {
	unsigned short	f_magic;	/* magic number */
					/* Target machine on which the
					   object file is executable */
	unsigned short	f_nscns;	/* number of sections */
	long		f_timdat;	/* time & date stamp */
	long		f_symptr;	/* file pointer to symtab */
	long		f_nsyms;	/* number of symtab entries */
	unsigned short	f_opthdr;	/* sizeof(optional hdr) */
	unsigned short	f_flags;	/* flags */
} FILHDR;


/*
 *	Bits for f_flags: (Most were defined for original COFF)
 *
 *	F_RELFLG	relocation info stripped from file
 *	F_EXEC		file is executable (i.e. no unresolved
 *				external references)
 *	F_LNNO		line nunbers stripped from file
 *	F_LSYMS		local symbols stripped from file
 *	F_MINMAL	this is a minimal object file (".m") output of fextract
 *	F_UPDATE	this is a fully bound update file, output of ogen
 *	F_SWABD		this file has had its bytes swabbed (in names)
 *	F_AR16WR	this file has the byte ordering of an AR16WR machine
 *				(e.g. 11/70) 
 *				(it was created there, or was produced by conv)
 *	F_AR32WR	this file has the byte ordering of an AR32WR machine
 *				(e.g. vax)
 *	F_AR32W		this file has the byte ordering of an AR32W machine 
 *				(e.g. S370,POWER,3b,maxi)
 *	F_PATCH		file contains "patch" list in optional header
 *	F_NODF		(minimal file only) no decision functions for
 *				replaced functions
 *	F_DYNLOAD	file is dynamically loadable and executable (i.e.,
 *				external references resolved via imports,
 *				may contain exports and loader relocation)
 *	F_SHROBJ	file is shared object
 *	F_LOADONLY	file can be loaded by the system loader, but it is
 *			ignored by the linker if it is a member of an archive.
 */

#define	F_RELFLG	0x0001
#define	F_EXEC		0x0002
#define	F_LNNO		0x0004
#define	F_LSYMS		0x0008
#if 0
#define	F_MINMAL	0x0010
#define	F_UPDATE	0x0020
#define	F_SWABD		0x0040
#endif /* 0 */
#define	F_AR16WR	0x0080
#define	F_AR32WR	0x0100
#define	F_AR32W		0x0200
#if 0
#define	F_PATCH		0x0400
#define	F_NODF		0x0400
#endif /* 0 */
#define	F_DYNLOAD	0x1000
#define	F_SHROBJ	0x2000
#define	F_LOADONLY	0x4000

/*
 *	Magic Numbers
 */
#define	X386MAGIC	0514
	/* IBM POWER and PowerPC */
#define	U802TOCMAGIC	0737	/* readonly text segments and TOC	*/

#define	U802WRMAGIC	0730	/* writeable text segments        	*/
#define	U802ROMAGIC	0735	/* readonly sharable text segments	*/
	/* IBM RT */
#define	U800TOCMAGIC	0637	/* readonly text segments and TOC	*/

#define	U800WRMAGIC	0630	/* writeable text segments        	*/
#define	U800ROMAGIC	0635	/* readonly sharable text segments	*/

#define	FILHSZ	sizeof(FILHDR)
#endif	/* _H_FHDR  */
