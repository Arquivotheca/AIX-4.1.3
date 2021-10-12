/* @(#)04	1.41  src/bos/kernel/vmm/POWER/vmdefs.h, sysvmm, bos41J, 9513A_all 3/17/95 14:05:47 */
#ifndef _h_VMDEFS
#define _h_VMDEFS

/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

/*
 * System limits for appropriate hardware, etc -
 */
#define PSIZE           (1<<12)         /* page size in bytes */
#define L2PSIZE         12              /* log base 2 of page size */
#define NUMPAGES        (1<<16)         /* max number of pages in a segment */
#define MAXVPN          (NUMPAGES - 1)  /* largest page number in a segment */
#define L2SSIZE         28              /* log base 2 of segment size */
#define SREGMSK         0xf0000000      /* mask for sreg in address */
#define SOFFSET         0x0fffffff      /* mask for offset in a segment  */
#define POFFSET         0x00000fff      /* mask for byte offset in page */
#define STOIBITS        5               /* bits used in STOI and ITOS macros */
#define MAXPGSP         16              /* max number of paging devices      */
#define MAXLOGS         256             /* max number of logs          */
#define ORGPFAULT       0x03            /* original page fault - mst backt */
#define BKTPFAULT	0x01		/* backtracking page fault	*/

/*
 * Segment identifiers 0x7fffff and 0x7ffffe are reserved for
 * INVLSID and for special graphics use (see dispauth.h, machine.m4)
 * respectively. Additional values may be reserved by reducing SIDLIMIT.
 */
#define NUMSIDS         (1<<20)         /* max number of scbs    */
#define SIDLIMIT        NUMSIDS-2       /* limit of sid values valid in scb */
#define SIDIO(sid)      ((sid)| 0x800000)  /* high bit on = i/o */
#define IOSIDMASK       0x7fffff        /* mask off high (i/o) bit */
#define RSVDSID         0x7ffffe        /* sid reserved for graphics use */
#define NUMAMES         (1<<20)         /* max number of ames    */
#define SIDMASK         0x00700000      /* mask used to extract bits in sid */
#define NEXTSID         0x00100000      /* value to increment sid to get next */
#define MAXRPAGE        (1024*1024)     /* max# real pages (4GB) */
#define IOSEGMENT       0x80000000      /* io segment bit in sreg */

/* values for 'type' parameter to machine-dependent routine p_enter().
 */
#define NORMAL  1
#define STARTIO 2
#define IODONE  3
#define IOSYNC  4
#define RELOAD  5

/* values for 'type' parameter to routines v_insapt(), v_delapt()
 */
#define APTREG  0
#define APTPIN  1

/* number of reserved paging space blocks.
 */
#define NRSVDBLKS       64      

/* maximum repage table size.
 */
#define	MAXREPAGE	(1 << 16)

/* maximum number of alias page table entries.
 */
#define	MAXAPT		(1 << 16)

/*
 * indices for selecting repaging rate data structures
 */

#define RPFILE          0
#define RPCOMP          1
#define RPTYPES         2

/*
 * Reserved indexes (not sids) in the SCB array
 */

#define KERSIDX         0               /* kernel segment */
#define VMMSIDX         1               /* VMMs private segment */
#define PTASIDX         2               /* Start PTA segment */
#define DMAPSIDX        3               /* paging space disk maps */
#define KEXTSIDX        4               /* kernel extension segment */

/*
 * PFT index of free block anchor.
 */

#define FBANCH          1               /* pft anchor for free list */

/*
 * Thresholds for the free list - page replacement mechanism.
 */

#define FBLOWLIM        4               /* free block list lower limit */
#define FBUPLIM         8               /* free block list upper limit */
#define FBMAX           16              /* max# free blks to select at once */

/*
 * Pageahead threshold(s) -
 */

#define GROWPGAHEAD     1               /* log 2 number to grow pageahead */
#define MINPGAHEAD      2               /* min number of pageahead pages  */
#define MAXPGAHEAD      8               /* max number of pageahead pages  */

/*
 * Pinned page threshold(s) -
 */

#define PFMINPIN        64              /* min unfixed frames for a pin */
#define MAXPIN          0x3FFF3FFF      /* max long/short term pin counts */

/*
 * Segment registers used by the VMM, and the system -
 */

#define KERNSR           0               /* kernel's sreg       */
#define PROCSR           2               /* process private sreg      */
#define VMMSR           11               /* segreg for VMM Data Segment */
#define PTASR           12               /* segreg for PTA segment */
#define TEMPSR          13               /* all purpose segreg */
#define KERNXSR         14               /* kernel extension sreg */

#define TEXTSR           1               /* segment register for text */

/*
 * various defines for internal VMM use
 */
#define FBLRU           1
#define NOFBLRU         0
#define SZBIT           (1 << 31)                 /* signed zero bit set    */
#define UZBIT           ( (uint) (1 << 31))       /* unsigned zero bit set  */
#define ONES            0xffffffff                /* a word of one bits     */
#define SIO             1                         /* initiate i/o           */
#define NOSIO           0                         /* do not start i/o       */

/*
 * LED for running out of memory during system initialization.
 */
#define HALT_NOMEM      0x55800000

/*
 * defines for mst exception struct
 */

#define EORGVADDR     	0               /* original virutal address	*/
#define EISR     	1               /* dsisr or isisr value		*/
#define ESRVAL     	2               /* segment register value	*/
#define EVADDR     	3               /* virutal address		*/
#define EPFTYPE     	4               /* page fault type		*/

#define PVNULL          ((uint) 0x3fffff)       /* empty pvlist         */
#define APTNULL		((ushort) 0xffff)	/* empty aptlist	*/

/*
 * Software lock constants.
 */
#define NLOCKBITS       32              /* number of lockbits per page */
#define LINESIZE        (PSIZE/32)      /* size of line in bytes       */
#define L2LINESIZE      7               /* log 2 of linesize (PSIZE = 4096) */


#if defined(_POWER_RS2) || defined(_POWER_PC)

#define PTEGSIZE        8               /* Number of PTEs per group     */
#define L2PTEGSIZE      3               /* log 2 of number of PTEs per group */
#define MAXPTES         4*MAXRPAGE      /* Max number of PTEs */

#endif /* _POWER_RS2 || _POWER_PC */

#ifdef _POWER_PC

#define	PTE_HSEL_PPC	0x00000040	/* Hash selector bit in PTE */
#define	PTE_VALID_PPC	0x80000000	/* Valid bit in PTE */

#endif /* _POWER_PC */

/*
 * DSISR - data storage interrupt status register
 */
#define DSIO            0x80000000      /* always 0 for _POWER_PC */
#define DSPFT           0x40000000
#define DSLOCK          0x20000000      /* always 0 for _POWER_PC */
#define DSFLIO          0x10000000      /* always 0 for _POWER_PC */
#define DSPROT          0x08000000
#define DSLOOP          0x04000000      /* always 0 for _POWER_RS2 */
#define DSSYNCIO        DSLOOP          /* redefined for _POWER_PC */
#define DSSTORE         0x02000000
#define DSSEGB          0x01000000
#define DSBPR           0x00400000
#define DSSTAB          0x00200000      /* always 0 for _POWER_RS1/RSC/RS2 */

/*
 * ISI - instruction storage interrupt - SRR1
 */
#define ISPFT           0x40000000
#define ISLOCK          0x20000000      /* always 0 for _POWER_PC */
#define ISIO            0x10000000
#define ISPROT          0x08000000
#define ISLOOP          0x04000000      /* always 0 for _POWER_RS2/PC */
#define ISSTAB          0x00200000      /* always 0 for _POWER_RS1/RSC/RS2 */

/*
 * Long Term Pin values.
 * These will be incorrect if the order of two shorts unioned over an integer
 * changes in the compiler.  The long-term count, declared first, must occupy
 * the lower address in memory.
 * The INCrement and DECrement values here are for use as parameters to an
 * Atomic Add operation.
 * The LongTerm and ShortTerm macros are used to extract the value from a
 * combined pincount.  This operation is necessary because we often get the
 * combined pincount back from an atomic operation, and need to use exactly
 * that value, versus re-fetching it as a short, for some decision.
 * These definitions make the most significant bit of the signed short
 * pincounts, the sign bit, into a guard bit to protect from overflows.
 * This limits the effective range of pincounts to [0-32767] inclusive.
 */
#define	LONG_TERM	1	     /* Type of pin requested: Long Term */
#define	SHORT_TERM	0	     /* Type of pin requested: Short Term */

#define	LTPIN_INC	0x00010000   /* Add 1 to the long-term pincount */
#define	LTPIN_OVERFLOW	0x80000000   /* If bit is set, long-term overflowed */
#define LTPIN_DEC	0xFFFF0000   /* Subtract 1 from the long-term count */
#define	LTPIN_MASK	0xFFFF0000   /* Don't rely on sign extension behavior */
#define	LTPIN_NBITS	16

#define	STPIN_INC	0x00000001   /* Add 1 to the short-term pincount */
#define	STPIN_OVERFLOW	0x00008000   /* If bit set, short-term overflowed */
#define	STPIN_DEC	0xFFFFFFFF   /* Subtract 1 from the short-term count */
#define	STPIN_MASK	0xFFFF
#define	STPIN_NBITS	16

#define	LongTerm(pc)	((pc & LTPIN_MASK)>>STPIN_NBITS)
#define	ShortTerm(pc)	(pc & STPIN_MASK)

/*
 * If the resulting pincount from either type pin operation is one of the
 * values below, then the system-wide count of pinned pages needs to be
 * adjusted.  The same is true, in the opposite direction, if the integer
 * pincount becomes zero.
 */
#define	LTPIN_ONE	LTPIN_INC
#define	STPIN_ONE	STPIN_INC

/*
 * Values for ppda->lru
 *
 * This flag indicates when the VMM is in a backtrack critical section
 * or is performing page-replacement.  It is used by page-replacement
 * to avoid stealing pages carefully touched in backtrack critical sections.
 * WARNING: The define values must enforce the assumption that
 * we cannot wrap around if ppda->lru is non-null (Cf teststackfix).
 * UPDATE vmvcs.s accordingly for ml definitions.
 */
#define	VM_NOBACKT		0
#define	VM_BACKT		1
#define VM_INLRU		2

/*
 * LRU criteria (flags) for v_hfblru().
 */
#define LRU_ANY		0
#define LRU_FILE	1
#define LRU_UNMOD	2

#endif /* _h_VMDEFS */
