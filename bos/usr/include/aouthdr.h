/* @(#)17	1.8  src/bos/usr/include/aouthdr.h, cmdld, bos411, 9428A410j 5/6/94 09:00:31 */
#ifndef	_H_AHDR
#define _H_AHDR
/*
 * COMPONENT_NAME: (CMDLD) XCOFF object file format definition
 *
 * FUNCTIONS: aouthdr.h
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
#include <sys/types.h>

typedef	struct aouthdr {
	short	magic;		/* flags - how to execute		*/
			/* 0x0107 text & data contiguous, both writable	*/
			/* 0x0108 text is R/O, data in next section	*/
			/* 0x010B text & data aligned and may be paged	*/
	short	vstamp;		/* version stamp			*/
	long	tsize;		/* text size in bytes			*/
	long	dsize;		/* data size in bytes			*/
	long	bsize;		/* bss size in bytes			*/
	long	entry;		/* entry point descriptor address	*/
	ulong	text_start;	/* virtual address of beginning of text	*/
	ulong	data_start;	/* virtual address of beginning of data	*/
	ulong	o_toc;		/* address of TOC			*/
	short	o_snentry;	/* section number for entry point	*/
	short	o_sntext;	/* section number for text		*/
	short	o_sndata;	/* section number for data		*/
	short	o_sntoc;	/* section number for toc		*/
	short	o_snloader;	/* section number for loader		*/
	short	o_snbss;	/* section number for bss		*/
	short	o_algntext;	/* log (base 2) of max text alignment	*/
	short	o_algndata;	/* log (base 2) of max data alignment	*/
	char    o_modtype[2];	/* Module type field, 1L, RE, RO	*/
	uchar	o_cpuflag;	/* bit flags - cputypes of objects	*/
	uchar	o_cputype;	/* executable cpu type identification	*/
	ulong   o_maxstack;	/* max stack size allowed (bytes)	*/
	ulong   o_maxdata;	/* max data size allowed (bytes)	*/
	ulong   o_resv2[3];	/* reserved fields			*/
} AOUTHDR;

/* Defines to provide the original COFF field names */
#define	o_mflag		magic
#define	o_vstamp	vstamp
#define	o_tsize		tsize
#define	o_dsize		dsize
#define	o_bsize		bsize
#define	o_entry		entry
#define	o_text_start	text_start
#define	o_data_start	data_start

/*
 * Defines for the flags to identify characteristics of a collection
 * of object files link-edited together
 *	Bit-mapped field of the o_cpuflag field above
 */
#define	TOBJ_SAME	0x80	/* All object files have same cputype field. */
#define	TOBJ_COM	0x40	/* All object files are in a valid cputype */
				/*     subset. */
#define	TOBJ_EMULATE	0x20	/* One or more object files are of a  */
				/* cputype that would require emulated */
				/* instructions on the resultant cputype. */
				/* E.g., PWR objects promoted to PPC. */
#define	TOBJ_COMPAT	0x10	/* One or more object files are TCPU_ANY. */
				/* They have been ignored in determining */
				/* the resultant o_cputype value. */

/* 
 * Defines for the POWER and PowerPC CPU Types 
 *    The o_cputype field above
 *    The n_cputype field of C_FILE symbol table entry
 */

#define TCPU_INVALID	0	/* Invalid id - assumes POWER for old objects */
#define TCPU_PPC	1	/* PowerPC common architecture 32 bit mode */
#define TCPU_PPC64	2	/* PowerPC common architecture 64 bit mode */
#define TCPU_COM	3	/* POWER and PowerPC architecture common */
#define TCPU_PWR	4	/* POWER common architecture objects */
#define TCPU_ANY	5	/* Mixture of any incompatable POWER */
				/* and PowerPC architecture implementations */
#define TCPU_601	6	/* 601 implementation of PowerPC architecture */
#define TCPU_PWR1	10	/* RS1 implementation of POWER architecture */
#define TCPU_PWRX	224	/* RS2 implementation of POWER architecture */


#endif	/* _H_AHDR */
