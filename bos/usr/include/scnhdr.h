/* @(#)79	1.7  src/bos/usr/include/scnhdr.h, cmdld, bos411, 9428A410j 3/24/93 21:38:54 */
#ifndef	_H_SCNHDR
#define _H_SCNHDR
/*
 * COMPONENT_NAME: (CMDLD) XCOFF object file format definition
 *
 * FUNCTIONS: scnhdr.h 
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

typedef struct scnhdr {
	char	s_name[8];	/* section name */
	ulong	s_paddr;	/* physical address */
	ulong	s_vaddr;	/* virtual address */
	ulong	s_size;		/* section size */
	long	s_scnptr;	/* file ptr to raw data for section */
	long	s_relptr;	/* file ptr to relocation */
	long	s_lnnoptr;	/* file ptr to line numbers */
	ushort	s_nreloc;	/* number of relocation entries */
	ushort	s_nlnno;	/* number of line number entries */
	long	s_flags;	/* flags */
}SCNHDR;

#define	SCNHSZ	sizeof(SCNHDR)

/*
 * Define constants for names of "special" sections
 */

#define _TEXT	".text"
#define _DATA	".data"
#define _BSS	".bss"
#define	_PAD	".pad"
#define _LOADER ".loader"
#define _TYPCHK ".typchk"
#define _DEBUG	".debug"
#define _EXCEPT	".except"
#define _OVRFLO	".ovrflo"

/*
 * The low order 16 bits of s_flags is used as section "type"
 */

#define STYP_REG	0x00	/* "regular" section */
#define STYP_PAD	0x08	/* "padding" section */
#define	STYP_TEXT	0x20	/* section contains text only */
#define STYP_DATA	0x40	/* section contains data only */
#define STYP_BSS	0x80	/* section contains bss only */
#define STYP_EXCEPT	0x0100	/* Exception section */
#define STYP_INFO	0x0200	/* Comment section */
#define STYP_LOADER	0x1000	/* Loader section */
#define STYP_DEBUG	0x2000	/* Debug section */
#define STYP_TYPCHK	0x4000	/* Type check section */
#define STYP_OVRFLO	0x8000	/* Overflow section header 
					for handling relocation and 
					line number count overflows */

#endif /* _H_SCNHDR */
