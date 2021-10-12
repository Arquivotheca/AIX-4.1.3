static char sccsid[] = "@(#)93	1.4  src/bos/kernel/vmm/POWER/v_power_rs2.c, sysvmm, bos411, 9428A410j 3/16/94 14:52:32";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:	p_enter_rs2(), p_rename_rs2(),
 *		p_remove_rs2(), p_remove_all_rs2(),
 * 		p_is_modified_rs2(), p_is_referenced_rs2(),
 *		p_clear_modify_rs2(),
 *		p_protect_rs2(), p_page_protect_rs2(),
 *		p_lookup_rs2()
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

#include "vmsys.h"
#include <sys/syspest.h>

/*
 * Machine-dependent routines for POWER_RS2.
 */

/*
 * This code manages the hardware Hashed Page Table (HTAB) and software
 * Physical-to-Virtual Table (PVT) for the RS2 platform.
 *
 * This platform has the following characteristics:
 *
 *	RS2
 *	---
 *	Y	aliasing supported by page tables
 *	N	aliasing supported by caches (no cache synonyms)
 *	N	combined data and instruction cache
 *	N	separate data and instruction cache operations
 *	N	data cache consistent with I/O transfers
 *
 * This platform does support virtual address aliasing in hardware.
 * However, this capability is not exploited due to complications involving
 * cache synonyms and I/O operations not being consistent with the cache.
 * Therefore, only one page table entry (PTE) exists for any given page frame.  
 * The PVT records information for each page frame including the HTAB index
 * of the PTE mapping the frame as well as reference and modify bits for the
 * frame which must be recorded independently of the HTAB due to page table
 * overflows.
 *
 * Each PTE is in one of the following states:
 *
 * PTE-invalid	valid bit is 0
 *		rs2_sid, rs2_page, rs2_key are undefined, no TLB entry
 *		referenced and modified bits are zero
 *		There are no active cache lines for the page frame
 *
 * PTE-valid	valid bit is 1
 *		rs2_sid, rs2_page, rs2_key give the mapping, may have TLB entry
 *		referenced and modified bits are valid
 *		There may be active cache lines for the page frame
 *
 * Each PVT entry is in one of the following states:
 *
 * Invalid	PTE index is PVNULL, no valid PTE for the frame
 *		reference and modified bits are zero
 *
 * Mapped	PTE index is the HTAB index of the valid PTE mapping the frame
 *		reference and modified bits are valid
 *
 * Unmapped	PTE index is PVNULL, no valid PTE for the frame
 *		reference and modified bits are valid
 *
 * Valid state transitions are:
 *
 * p_enter		Invalid, Mapped, Unmapped => Mapped
 * p_rename		Mapped, Unmapped => Mapped
 * p_remove		Mapped, Unmapped => Unmapped
 * p_remove_all		Mapped, Unmapped => Invalid
 *
 * The execution environment for these routines is:
 *
 * - data and instruction translate off
 * - all interrupts disabled
 */

void	p_enter_rs2();
void	p_rename_rs2();
void	p_remove_rs2();
void	p_remove_all_rs2();
int	p_is_modified_rs2();
int	p_is_referenced_rs2();
void	p_clear_modify_rs2();
void	p_protect_rs2();
void	p_page_protect_rs2();
int	p_lookup_rs2();

uint	p_inspte_rs2();
void	p_delpte_rs2();
int	p_getpte_rs2();

/*
 * Generate a random number in the range [0,x].
 * Low-order 8 bits of decrementer aren't implemented yet.
 */
#define	RANDOM(x)	((mfdec() >> 8) & (x))

/*
 * Invalidate TLB.
 */
#define INVTLB(sid,pno)	invtlb(sid,pno)

/*
 * Cache operations.
 */
#define	SYNCCACHE(sid,pno)	sync_cache_page_pwr(sid,pno)

/*
 * p_enter_rs2(type,sid,pno,nfr,key,attr)
 *
 * Insert a hardware page table entry.
 */
void
p_enter_rs2(type,sid,pno,nfr,key,attr)
uint	type;	/* type of operation */
uint	sid;	/* segment id */
uint	pno;	/* page number in segment */
uint	nfr;	/* page frame number */
uint	key;	/* storage protect key  */
uint	attr;	/* storage control attributes */
{
	uint ptex, ssid, spno;

	/*
	 * If the frame is mapped then ping-pong it to the new address
	 * otherwise insert it.
	 */
	if ((ptex = rs2_ptex(nfr)) != PVNULL)
	{
		/*
		 * Ensure cache consistency.
		 * For a virtually indexed cache we must prevent cache
		 * synonyms by flushing the cache when the virtual addresses
		 * differ in bits used by cache look-up.
		 * When setting up I/O access for a page (STARTIO) we must
		 * flush the cache if the page is modified to ensure that real
		 * memory contains the latest state of the page because I/O is
		 * not consistent with the data cache.
		 * When completing a page-in I/O (IOSYNC) we must flush the
		 * cache if the page is modified to ensure that real memory
		 * contains the latest state of the page because of separate
		 * instruction and data caches.
		 */
		ssid = rs2_sid(ptex);
		spno = PTEX2PNO_RS2(ptex);
		if (CACHE_DIFF(spno, pno) ||
		    (rs2_modbit(ptex) && (type == STARTIO || type == IOSYNC)))
		{
			SYNCCACHE(ssid,spno);
		}

		/*
		 * If source and target fall in the same hash class
		 * (i.e. when moving to and from I/O address) then there
		 * is no need to move it to a different PTE group.
		 */
		if (HASHF(ssid,spno) == HASHF(sid,pno))
		{
			rs2_sid(ptex) = sid;
			rs2_avpi(ptex) = AVPI_RS2(pno);
			rs2_key(ptex) = key;
			INVTLB(ssid,spno);
		}
		else
		{
			p_delpte_rs2(ptex);
			ptex = p_inspte_rs2(sid,pno,nfr,key);
		}
	}
	else
	{
		ptex = p_inspte_rs2(sid,pno,nfr,key);
	}

	/*
	 * For I/O completion operations clear the referenced and
	 * modified bits for the frame (in the PVT as well as the PTE).
	 */
	if (type == IODONE || type == IOSYNC)
	{
		rs2_refbit(ptex) = 0;
		rs2_modbit(ptex) = 0;
		rs2_swref(nfr) = 0;
		rs2_swmod(nfr) = 0;
	}
}

/*
 * p_rename_rs2(sid,pno,nfr,key,attr)
 *
 * Rename a hardware page table entry.
 */
void
p_rename_rs2(sid,pno,nfr,key,attr)
uint	sid;	/* segment id */
uint	pno;	/* page number in segment */
uint	nfr;	/* page frame number */
uint	key;	/* storage protect key  */
uint	attr;	/* storage control attributes */
{
	/*
	 * Just treat as a normal enter.
	 */
	p_enter_rs2(NORMAL, sid, pno, nfr, key, attr);
}

/*
 * p_remove_rs2(sid,pno,nfr)
 *
 * Delete a hardware page table entry for an alias address.
 */
void
p_remove_rs2(sid,pno,nfr)
uint		sid;	/* segment id */
uint		pno;	/* page number in segment */
uint		nfr;	/* page frame number */
{
	uint ptex, ssid, spno;

	/*
	 * If the frame is mapped at (sid,pno) delete it.
	 */
	if ((ptex = rs2_ptex(nfr)) != PVNULL)
	{
		ssid = rs2_sid(ptex);
		spno = PTEX2PNO_RS2(ptex);
		if (ssid == sid && spno == pno)
		{
			SYNCCACHE(ssid,spno);
			p_delpte_rs2(ptex);
		}
	}
}

/*
 * p_remove_all_rs2(nfr)
 *
 * Delete all hardware page table entries.
 */
void
p_remove_all_rs2(nfr)
uint	nfr;	/* page frame number */
{
	uint ptex, ssid, spno;

	/*
	 * If the frame is mapped delete it.
	 */
	if ((ptex = rs2_ptex(nfr)) != PVNULL)
	{
		ssid = rs2_sid(ptex);
		spno = PTEX2PNO_RS2(ptex);
		SYNCCACHE(ssid,spno);
		p_delpte_rs2(ptex);
	}

	/*
	 * Clear reference and change bits in PVT.
	 */
	rs2_swref(nfr) = 0;
	rs2_swmod(nfr) = 0;
}

/*
 * p_is_modified_rs2(nfr)
 *
 * Return the modified status of a page frame.
 */
p_is_modified_rs2(nfr)
uint	nfr;	/* page frame number */
{
	uint ptex;

	/*
	 * If the frame is mapped check the PTE as well as the PVT.
	 */
	if ((ptex = rs2_ptex(nfr)) != PVNULL)
	{
		return(rs2_modbit(ptex) || rs2_swmod(nfr));
	}
	else
	{
		return(rs2_swmod(nfr));
	}
}

/*
 * p_is_referenced_rs2(nfr)
 *
 * Return and reset the referenced status of a page frame.
 */
p_is_referenced_rs2(nfr)
uint     nfr;    /* page frame number */
{
	uint ptex;

	/*
	 * If the frame is mapped check and clear the PTE, and
	 * propogate the referenced status to the PVT entry.
	 */
	if (((ptex = rs2_ptex(nfr)) != PVNULL) && rs2_refbit(ptex))
	{
		rs2_swref(nfr) = 1;
		rs2_refbit(ptex) = 0;
		INVTLB(rs2_sid(ptex),PTEX2PNO_RS2(ptex));
	}

	if (rs2_swref(nfr))
	{
		rs2_swref(nfr) = 0;
		return(TRUE);
	}
	else
	{
		return(FALSE);
	}
}

/*
 * p_clear_modify_rs2(nfr)
 *
 * Clear the modified status of a page frame.
 */
void
p_clear_modify_rs2(nfr)
uint	nfr;	/* page frame number */
{
	uint ptex;

	/*
	 * If the frame is mapped also clear the PTE modify bit.
	 */
	if (((ptex = rs2_ptex(nfr)) != PVNULL) && rs2_modbit(ptex))
	{
		rs2_modbit(ptex) = 0;
		INVTLB(rs2_sid(ptex),PTEX2PNO_RS2(ptex));
	}
	rs2_swmod(nfr) = 0;
}

/*
 * p_protect_rs2(sid,pno,nfr,key)
 *
 * Lower permissions for a specified mapping of a page frame.
 */
void
p_protect_rs2(sid,pno,nfr,key)
uint	sid;	/* segment id */
uint	pno;	/* page number in segment */
uint	nfr;	/* page frame number */
uint	key;	/* storage protect key  */
{
	uint ptex, ssid, spno;

	/*
	 * Update the page protection if the frame is mapped to (sid,pno).
	 */
	if ((ptex = rs2_ptex(nfr)) != PVNULL)
	{
		ssid = rs2_sid(ptex);
		spno = PTEX2PNO_RS2(ptex);
		if (ssid == sid && spno == pno)
		{
			rs2_key(ptex) = key;
			INVTLB(sid,pno);
		}
	}
}

/*
 * p_page_protect_rs2(nfr,key)
 *
 * Lower permissions for all mappings of a specified page frame.
 */
void
p_page_protect_rs2(nfr,key)
uint	nfr;	/* page frame number */
uint	key;	/* storage protect key  */
{
	uint ptex;

	/*
	 * Update the page protection if the frame is mapped.
	 */
	if ((ptex = rs2_ptex(nfr)) != PVNULL)
	{
		rs2_key(ptex) = key;
		INVTLB(rs2_sid(ptex),PTEX2PNO_RS2(ptex));
	}
}

/*
 * p_inspte_rs2(sid,pno,nfr,key)
 *
 * Insert a PTE in the HTAB.
 */
uint
p_inspte_rs2(sid,pno,nfr,key)
uint     sid;    /* segment id */
uint     pno;    /* page number in segment */
uint     nfr;    /* page frame number    */
uint     key;    /* storage protect key  */
{
	uint ptex, hsel;

	/*
	 * Find a free PTE in the hash group for (sid,pno).
	 */
	ptex = p_getpte_rs2(sid,pno,&hsel);

	/*
	 * Word 0 - SID, hsel, avpi.
	 */
        rs2_sid(ptex) = sid;
	rs2_hsel(ptex) = hsel;
	rs2_avpi(ptex) = AVPI_RS2(pno);

	/*
	 * Word 1 - Real Page Number, protection key
	 */
	rs2_rpn(ptex) = nfr;
	rs2_key(ptex) = key;

	/*
	 * Page table entry index.
	 */
	rs2_ptex(nfr) = ptex;

	/*
	 * Now set the valid bit.
	 */
        rs2_valid(ptex) = 1;

	return(ptex);
}

/*
 * p_delpte_rs2(ptex)
 *
 * Delete a PTE from the HTAB.
 */
void
p_delpte_rs2(ptex)
uint     ptex;	/* PTE index */
{
        uint sid, pno, nfr;

	sid = rs2_sid(ptex);
	pno = PTEX2PNO_RS2(ptex);
	nfr = rs2_rpn(ptex);
	ASSERT(rs2_valid(ptex));
	ASSERT(rs2_ptex(nfr) == ptex);
	ASSERT(p_lookup_rs2(sid,pno) == nfr);

	/*
	 * Clear word containing valid bit.
	 */
	rs2_word0(ptex) = 0;

	/*
	 * Invalidate TLB.
	 */
	INVTLB(sid,pno);

	/*
	 * Preserve reference and modify bits in PVT.
	 */
	rs2_swref(nfr) |= rs2_refbit(ptex);
	rs2_swmod(nfr) |= rs2_modbit(ptex);

	/*
	 * Clear word containing reference and modify bits.
	 */
	rs2_word1(ptex) = 0;

	/*
	 * Update PVT entry.
	 */
	rs2_ptex(nfr) = PVNULL;
}

/*
 * p_getpte_rs2(sid,pno,hsel)
 *
 * Find a free PTE in the hash group for (sid,pno).
 */
int 
p_getpte_rs2(sid,pno,hsel)
uint     sid;    /* segment id */
uint     pno;    /* page number in segment */
uint	*hsel;  /* indicate which hash group entry was found in */
{
        uint hash1, hash2, ptex;
	uint ptex1, ptex2, rsid, rpno;
	uint word0, word1;

	/*
	 * Search primary hash group.
	 */
	*hsel = 0;
	hash1 = HASHF(sid,pno);
	for (ptex = FIRSTPTE(hash1); ptex < FIRSTPTE(hash1) + PTEGSIZE; ptex++)
	{
		if (!rs2_valid(ptex))
			goto found;
	}

	/*
	 * Search secondary hash group.
	 */
	*hsel = 1;
	hash2 = HASHF2(sid,pno);
	for (ptex = FIRSTPTE(hash2); ptex < FIRSTPTE(hash2) + PTEGSIZE; ptex++)
	{
		if (!rs2_valid(ptex))
			goto found;
	}

	/*
	 * The hash group is full, handle overflow.
	 * Overflow statistic must remain non-zero once an overflow occurs.
	 */
	vmker.overflows++;
	if (vmker.overflows == 0)
		vmker.overflows++;

	/*
	 * From here on, we always return a PTE index from the primary
	 * hash group so set hsel accordingly.
	 */
	*hsel = 0;

	/*
	 * Starting at a random entry in the primary hash group, search
	 * circularly for a secondary entry and if one is found remove it
	 * and return its index.
	 */
	ptex1 = FIRSTPTE(hash1) + RANDOM(7);
	for (ptex = ptex1; ptex < FIRSTPTE(hash1) + PTEGSIZE; ptex++)
	{
		if (rs2_hsel(ptex))
		{
			rsid = rs2_sid(ptex);
			rpno = PTEX2PNO_RS2(ptex);
			SYNCCACHE(rsid,rpno);
			p_delpte_rs2(ptex);
			goto found;
		}
	}

	for (ptex = FIRSTPTE(hash1); ptex < ptex1; ptex++)
	{
		if (rs2_hsel(ptex))
		{
			rsid = rs2_sid(ptex);
			rpno = PTEX2PNO_RS2(ptex);
			SYNCCACHE(rsid,rpno);
			p_delpte_rs2(ptex);
			goto found;
		}
	}

	/*
	 * All entries in the primary hash group are primary entries.
	 * Remove a random entry in the secondary hash group, move the
	 * random entry from the primary hash group to it and return
	 * the index of the random entry in the primary hash group.
	 */

	/*
	 * Remove a random entry in the secondary hash group.
	 */
	ptex2 = FIRSTPTE(hash2) + RANDOM(7);
	rsid = rs2_sid(ptex2);
	rpno = PTEX2PNO_RS2(ptex2);
	SYNCCACHE(rsid,rpno);
	p_delpte_rs2(ptex2);

	/*
	 * Save the contents of the primary hash group entry and remove it.
	 */
	word0 = rs2_word0(ptex1);
	word1 = rs2_word1(ptex1);
	rsid = rs2_sid(ptex1);
	rpno = PTEX2PNO_RS2(ptex1);
	SYNCCACHE(rsid,rpno);
	p_delpte_rs2(ptex1);

	/*
	 * Now move the saved contents to the secondary hash group.
	 * Note that the hardware won't find this entry (even though
	 * the valid bit is on) until it is marked as a secondary entry.
	 */
	rs2_word0(ptex2) = word0;
	rs2_word1(ptex2) = word1;
	rs2_hsel(ptex2) = 1;
	rs2_ptex(rs2_rpn(ptex2)) = ptex2;
	
	/*
	 * Return the free entry in the primary hash group.
	 */
	ptex = ptex1;

found:
	return(ptex);
}

/*
 * p_lookup_rs2(sid,pno)
 *
 * Look up the page frame for a specified virtual address.
 */
int 
p_lookup_rs2(sid,pno)
uint	sid;	/* segment id */
uint	pno;	/* page number in segment */
{
        uint hash, ptex;

	/*
	 * Search primary hash group.
	 */
	hash = HASHF(sid,pno);
	for (ptex = FIRSTPTE(hash); ptex < FIRSTPTE(hash) + PTEGSIZE; ptex++)
	{
		if (rs2_valid(ptex) && rs2_sid(ptex) == sid &&
		    rs2_avpi(ptex) == AVPI_RS2(pno) &&
		    rs2_hsel(ptex) == 0)
			return(rs2_rpn(ptex));
	}

	/*
	 * Search secondary hash group.
	 */
	hash = HASHF2(sid,pno);
	for (ptex = FIRSTPTE(hash); ptex < FIRSTPTE(hash) + PTEGSIZE; ptex++)
	{
		if (rs2_valid(ptex) && rs2_sid(ptex) == sid &&
		    rs2_avpi(ptex) == AVPI_RS2(pno) &&
		    rs2_hsel(ptex) == 1)
			return(rs2_rpn(ptex));
	}

        return(-1);
}
