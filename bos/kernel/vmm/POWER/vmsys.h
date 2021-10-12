/* @(#)90       1.22  src/bos/kernel/vmm/POWER/vmsys.h, sysvmm, bos411, 9428A410j 7/11/94 10:11:44 */
#ifndef _h_VMSYS
#define _h_VMSYS

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
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

#include <sys/types.h>
#include <sys/adspace.h>
#include <sys/proc.h>
#include "sys/pseg.h"
#include <sys/vmdisk.h>
#include "vmdefs.h"
#include "vmpfhdata.h"
#include "vmpft.h"
#include "vmscb.h"
#include "vmlock.h"
#include "vmxpt.h"
#include "vmmacs.h"
#include "mplock.h"
#include <sys/vmker.h>
#include <sys/vmuser.h>
#include <sys/vminfo.h>
#include <sys/systemcfg.h>

/*
 * FUNCTION: This defines the layout of the VMM data segment
 *           and page table area segments (PTA) and the
 *           kernel resident VMM data structure vmkerdata.
 *           this together with the includes above define all
 *           of the VMM's data structures.
 *
 */


/*
 * vmmdseg contains the control data of the VMM. it occupies its
 * own segment and must be made addressable at run-time by loading
 * the  segment register VMMSR  . in binding the kernel vmmdseg
 * is imported at the corresponding address VMMSR<<28. keeping all
 * the variables in vmmdseg eliminates the need to allocate addresses
 * in the segment for each of them but makes the referencing of them
 * by name more awkward (since the prefix vmmdseg is needed). macros
 * are provided which simplify the naming. CAUTION: the order in
 * which the structures pdt thru scb appear in vmmdseg should not be
 * changed without fixing up vmsi.
 */

/* array bounds for paging device table.
 */
#define PDTSIZE         512

struct vmmdseg
{
	union {				/* hardware dependent data ... */
#if defined(_POWER_RS1)	|| defined(_POWER_RSC)
		struct rs1	rs1;	/* ... for _POWER_RS1, _POWER_RSC */
#endif /* _POWER_RS1 ||	_POWER_RSC */
#ifdef _POWER_RS2
		struct rs2	rs2;	/* ... for _POWER_RS2 */
#endif /* _POWER_RS2 */
#ifdef _POWER_PC
		struct ppc	ppc;	/* ... for _POWER_PC */
#endif /* _POWER_PC */
	} hw;
	int		 hat[MAXRPAGE];	  /* software hash anchor table	*/
	struct pftsw	 pft[MAXRPAGE];   /* software defined pft   */
	ushort		 ahat[MAXAPT];    /* alias hash anchor table */
	struct apt	 apt[MAXAPT];     /* alias page table */
	ushort       	 rphat[MAXREPAGE];/* repaging hat	*/
	struct repage	 rpt[MAXREPAGE];  /* repaging table	*/
        struct pdtentry  pdt[PDTSIZE];    /* paging device table */
        struct pfhdata   pf;              /* control and other data */
	struct lockanch	 lockanch;	  /* lock stuff */
        struct scb       scb[NUMSIDS];    /* segment control blocks */
	struct ame	 ame[NUMAMES];    /* address map entries */

	/* lockwords occupy rest of segment starting on the next
	 * page boundary.
	 */
};

extern  struct vmmdseg vmmdseg;

#define hattab  vmmdseg.hat
#define ahattab vmmdseg.ahat
#define rphtab  vmmdseg.rphat
#define lword   vmmdseg.lockanch.lwptr
#define lanch   vmmdseg.lockanch

/*
 * layout of the PAGE TABLE (PTA) segment. at run-time the
 * the segment register PTASR must be loaded to address it.
 * in binding the kernel the PTA segment is imported at
 * the corresponding address. all xpts and vmap blocks are
 * allocated from it.
 */

union ptaseg
{
        /* control anchors (keep its size <= 8*192 bytes) */
        struct
        {
                struct xptroot root;    /* root of xpt2 */
                int    hiapm;           /* index next page of apms */
                int    vmapfree;        /* index free vmapblk      */
                int    notused;         /* spare field             */
                int    anchor[6];       /* index of first apm entry  */
		int    freecnt;		/* free 4k blocks with resources */
		int    freetail;	/* tail of 4k free list */
        } s1;

        /* the APM (area page map) and  direct xpt blocks of xpt2 */
        struct
        {
                struct apm apm[SEGSIZE/PSIZE]; /* one apm per page */
                struct xptdblk xptdblk[256];   /* direct xpt blocks */
        } s2;

        /* pages of the segment (overlays entire segment) */
        struct
        {
                int  word[PSIZE/4];
        } page[SEGSIZE/PSIZE];

        /* map blocks. (overlays entire segment)      */
        struct vmapblk  vmapblk[SEGSIZE/16];    /* vmap blocks */

};

extern union ptaseg ptaseg;

/* macros for accessing variables in ptaseg   */
#define pta_root        ptaseg.s1.root
#define pta_vmapfree    ptaseg.s1.vmapfree
#define pta_hiapm       ptaseg.s1.hiapm
#define pta_anchor      ptaseg.s1.anchor
#define pta_apm         ptaseg.s2.apm
#define pta_xptdblk     ptaseg.s2.xptdblk
#define pta_page        ptaseg.page
#define pta_vmap        ptaseg.vmapblk
#define pta_freecnt     ptaseg.s1.freecnt
#define pta_freetail    ptaseg.s1.freetail

#endif /* _h_VMSYS */
