static char sccsid[] = "@(#)20	1.2.1.5  src/bos/kernel/vmm/POWER/vmdump.c, sysvmm, bos41B, 412_41B_sync 12/20/94 15:11:20";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager	
 *
 * FUNCTIONS: vm_dump
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

#include "vmsys.h"
#include <sys/types.h>
#include <sys/dump.h>
#include <sys/xmem.h>
#include <sys/inline.h>

#define	VMM_DUMP	"vmm"
#define	DUMP_ENTRIES	(17+PDTSIZE)
#define	DUMP_VMKER	"vmker"
#define	DUMP_VMINFO	"vminfo"
#define	DUMP_HWPFT	"hwpft"
#define	DUMP_HWHAT	"hwhat"
#define	DUMP_HWPVT	"hwpvt"
#define	DUMP_HWPVLIST	"hwpvlist"
#define	DUMP_SWHAT	"swhat"
#define	DUMP_SWPFT	"swpft"
#define	DUMP_APTHAT	"apthat"
#define	DUMP_APT	"apt"
#define	DUMP_RPT	"rpt"
#define	DUMP_PDT	"pdt"
#define	DUMP_PFHDATA	"pfhdata"
#define	DUMP_LOCKANCH	"lockanch"
#define	DUMP_SCBS	"scbs"
#define	DUMP_AMES	"ames"
#define DUMP_LOCKWORD	"lockword"
#define DUMP_DMAPS	"dmap000"
char * dmapno(int n);
/* VMM component dump table.
 */
struct
{
	struct cdt_head  _cdt_head;
	struct cdt_entry  entry[DUMP_ENTRIES];
} vmmcdt;

/* Cross-memory descriptors to access VMM dump data.
 */
#define DUMP_XMEM_KER	0
#define	DUMP_XMEM_VMM	1
#define	DUMP_XMEM_DMAP	2
struct xmem xmem_dump[3];

/* Internal routine.
 */
static void vm_addcdt();

/* vm_dump(op)
 * 
 * Fill in VMM dump table and return its address.
 * 
 * INPUT PARAMETERS:
 *   op is 1 when dump is started (to get dump data information)
 *   op is 2 when dump is complete (to queisce operations if necessary).
 * 
 * We wait until our dump routine is called to actually fill in
 * the dump table because we have a better idea at this time of
 * how much data to dump (e.g. number of SCBs).
 *
 * This routine and all the data it references must be pinned.
 *
 * The current amount of VMM data included in the dump is approximately
 * 1/40th of the size of real memory in the system
 * (a little more than 6MB on a system with 256MB of real memory).
 */
struct cdt *
vm_dump(op)
int op;
{
 	int savesr, lword_begin;
	extern int g_ksrval;
	int i;

	if (op == 1)
	{
		/* Set up to address VMM data segment.
		 */
		savesr = chgsr(VMMSR, vmker.vmmsrval);

		/* Fill in cross-memory descriptors.
		 */
		xmem_dump[DUMP_XMEM_KER].aspace_id = 0;
		xmem_dump[DUMP_XMEM_KER].subspace_id = g_ksrval;
		xmem_dump[DUMP_XMEM_VMM].aspace_id = 0;
		xmem_dump[DUMP_XMEM_VMM].subspace_id = vmker.vmmsrval;

		/* Initialize dump table header.
		 */
		vmmcdt.cdt_magic = DMP_MAGIC;
		strncpy(vmmcdt.cdt_name, VMM_DUMP,
			sizeof(vmmcdt.cdt_name));
		vmmcdt.cdt_len = sizeof(struct cdt_head);

		/* Add dump table entry for
		 * vmker.
		 */
		vm_addcdt(DUMP_VMKER,
			sizeof(struct vmkerdata),
			&vmker,
			&xmem_dump[DUMP_XMEM_KER]);

		/* Add dump table entry for
		 * vminfo.
		 */
		vm_addcdt(DUMP_VMINFO,
			sizeof(struct vminfo),
			&vmminfo,
			&xmem_dump[DUMP_XMEM_KER]);

		/* Add dump table entry for
		 * hardware PFT.
		 */
		vm_addcdt(DUMP_HWPFT,
			vmrmap_size(RMAP_PFT),
			vmrmap_eaddr(RMAP_PFT),
			&xmem_dump[DUMP_XMEM_VMM]);

		/* Dump hw HAT
		 */
		if (vmrmap_size(RMAP_HAT))
		{
			vm_addcdt(DUMP_HWHAT,
				vmrmap_size(RMAP_HAT),
				vmrmap_eaddr(RMAP_HAT),
				&xmem_dump[DUMP_XMEM_VMM]);
		}
		/* It is zero for RS1
		 */
		if (vmrmap_size(RMAP_PVT))
		{
			vm_addcdt(DUMP_HWPVT,
				vmrmap_size(RMAP_PVT),
				vmrmap_eaddr(RMAP_PVT),
				&xmem_dump[DUMP_XMEM_VMM]);
		}
		/* It is non-zero on PowerPc    
		 */
		if (vmrmap_size(RMAP_PVLIST))
		{
			vm_addcdt(DUMP_HWPVLIST,
				vmrmap_size(RMAP_PVLIST),
                                vmrmap_eaddr(RMAP_PVLIST),
				&xmem_dump[DUMP_XMEM_VMM]);
		}

		/* Add dump table entry for
		 * software HAT.
		 */
		vm_addcdt(DUMP_SWHAT,
			sizeof(int) * vmker.nrpages,
			&vmmdseg.hat[0],
			&xmem_dump[DUMP_XMEM_VMM]);

		/* Add dump table entry for
		 * software PFT.
		 */
		vm_addcdt(DUMP_SWPFT,
			sizeof(struct pftsw) * vmker.nrpages,
			&vmmdseg.pft[0],
			&xmem_dump[DUMP_XMEM_VMM]);

		/* Add dump table entry for
		 * Alias Page Table HAT.
		 */
		vm_addcdt(DUMP_APTHAT,
			sizeof(ushort) * MAXAPT,
			&vmmdseg.ahat[0],
			&xmem_dump[DUMP_XMEM_VMM]);

		/* Add dump table entry for
		 * Alias Page Table
		 */
		vm_addcdt(DUMP_APT,
			sizeof(struct apt) * MAXAPT,
			&vmmdseg.apt[0],
			&xmem_dump[DUMP_XMEM_VMM]);

		/* Add dump table entry for
		 * RPT.
		 */
		vm_addcdt(DUMP_RPT,
			sizeof(struct repage) * MIN(MAXREPAGE, vmker.nrpages),
			&vmmdseg.rpt[0],
			&xmem_dump[DUMP_XMEM_VMM]);

		/* Add dump table entry for
		 * PDT.
		 */
		vm_addcdt(DUMP_PDT,
			sizeof(struct pdtentry) * PDTSIZE,
			&vmmdseg.pdt[0],
			&xmem_dump[DUMP_XMEM_VMM]);

		/* Add dump table entry for
		 * pfhdata.
		 */
		vm_addcdt(DUMP_PFHDATA,
			sizeof(struct pfhdata),
			&vmmdseg.pf,
			&xmem_dump[DUMP_XMEM_VMM]);

 		/* Add dump table entry for
 		 * lockanch.
 		 */
 		vm_addcdt(DUMP_LOCKANCH,
 			sizeof(struct lockanch),
 			&vmmdseg.lockanch,
 			&xmem_dump[DUMP_XMEM_VMM]);
 
		/* Add dump table entry for
		 * SCBs.
		 * Try to include up to the highest scb ever used,
		 * but limit it to a size based on # of real pages.
		 * Maximum size is 16 bytes for each page of real memory.
		 */
		vm_addcdt(DUMP_SCBS,
			sizeof(struct scb) * MIN(pf_hisid, vmker.nrpages / 4),
			&vmmdseg.scb[0],
			&xmem_dump[DUMP_XMEM_VMM]);

		/* Add dump table entry for
		 * AMEs.
		 * Try to include up to the highest ame ever used,
		 * but limit it to a size based on # of real pages.
		 * Maximum size is 16 bytes for each page of real memory.
		 */
		vm_addcdt(DUMP_AMES,
			sizeof(struct ame) * MIN(pf_hiame, vmker.nrpages / 4),
			&vmmdseg.ame[0],
			&xmem_dump[DUMP_XMEM_VMM]);

 		/* Add dump table entry for
 		 * LOCKWORDs.
 		 * Begin lockword array on next page after ames.
 		 * Now only dumps 1 page or 128 lockwords.
 		 */
         	lword_begin = &vmmdseg.ame[NUMAMES];
         	lword_begin = (lword_begin + PSIZE - 1) & (~(PSIZE - 1));
 		vm_addcdt(DUMP_LOCKWORD,
 			PSIZE,
 			lword_begin,
 			&xmem_dump[DUMP_XMEM_VMM]);

		
		/* Dump the diskmaps of paging devices and file systems
		 */
		for (i = 0; i < PDTSIZE; ++i)
		{
			if ((pdt_type(i) == D_PAGING) && pdt_avail(i))
			{
			xmem_dump[DUMP_XMEM_DMAP].aspace_id = 0;
			xmem_dump[DUMP_XMEM_DMAP].subspace_id = vmker.dmapsrval;
			vm_addcdt(dmapno(i),DMAPSIZE,i*DMAPSIZE,
					&xmem_dump[DUMP_XMEM_DMAP]);
			}
			else if (pdt_type(i) == D_FILESYSTEM)
			{
			xmem_dump[DUMP_XMEM_DMAP].aspace_id = 0;
			xmem_dump[DUMP_XMEM_DMAP].subspace_id = pdt_dmsrval(i);
			vm_addcdt(dmapno(i),DMAPSIZE,0,
					&xmem_dump[DUMP_XMEM_DMAP]);
			}
		}
		/* Restore vmmsr.
		 */
		(void)chgsr(VMMSR, savesr);
		
		return((struct cdt *)&vmmcdt);

	}
	else
		return((struct cdt *)NULL);
}


/* Add entry to VMM component dump table.
 */
static
void
vm_addcdt(name, len, ptr, xp)
char *name;
int len;
char *ptr;
struct xmem *xp;
{
	struct cdt_entry tmp;
	int num;

	/* Fill in local table entry
	 */
	strncpy(tmp.d_name, name, sizeof(tmp.d_name));
	tmp.d_len = len;
	tmp.d_ptr = ptr;

	/* Must initialize d_segval field to srval rather than passing
	 * a cross-memory descriptor since that's the supported interface.
	tmp.d_xmemdp = xp;
	 */
	tmp.d_segval = xp->subspace_id;

	/* Determine how many entries already added
	 */
	num = (vmmcdt.cdt_len - sizeof(struct cdt_head)) /
		sizeof(struct cdt_entry);

	/* Add new entry if it doesn't exceed table size.
	 */
	if (num < DUMP_ENTRIES)
	{
		vmmcdt.entry[num] = tmp;
		vmmcdt.cdt_len += sizeof(struct cdt_entry);
	}
}

char  dmap[8] = DUMP_DMAPS;
char * dmapno(int n)
{
	dmap[6] = '0' + n % 10;
	dmap[5] = '0' + n % 100 / 10;
	dmap[4] = '0' + n / 100;
	return dmap;
}
