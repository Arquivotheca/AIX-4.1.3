static char sccsid[] = "@(#)91	1.9  src/bos/kernel/vmm/POWER/v_power_pc.c, sysvmm, bos412, 9445C412a 10/25/94 11:16:07";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:	p_enter_ppc(), p_rename_ppc(),
 *		p_remove_ppc(), p_remove_all_ppc(),
 * 		p_is_modified_ppc(), p_is_referenced_ppc(),
 *		p_clear_modify_ppc(),
 *		p_protect_ppc(), p_page_protect_ppc(),
 *		p_lookup_ppc()
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

#include "vmsys.h"
#include <sys/inline.h>

/*
 * Machine-dependent routines for POWER_PC.
 */

/*
 * This code manages the hardware Hashed Page Table (HTAB) and software
 * Physical-to-Virtual Table (PVT, PVLIST) for the PowerPC platform.
 *
 * This platform has the following characteristics:
 *
 *	PPC
 *	---
 *	Y	aliasing supported by page tables
 *	Y	aliasing supported by caches (no cache synonyms)
 *	N	combined data and instruction cache
 *	Y	separate data and instruction cache operations
 *	Y	data cache consistent with I/O transfers
 *
 * This platform does support virtual address aliasing in hardware.
 *
 * Each PTE is in one of the following states:
 *
 * PTE-invalid	valid bit is 0
 *		ppc_sid, ppc_page, ppc_key are 0, no TLB entry
 *		referenced and modified bits are zero
 *		all other fields are zero
 *		There are no active cache lines for the page frame
 *
 * PTE-valid	valid bit is 1
 *		ppc_sid, ppc_page, ppc_key give the mapping, may have TLB entry
 *		referenced and modified bits are valid
 *		There may be active cache lines for the page frame
 *
 * Each PVT entry is in one of the following states:
 *
 * Invalid	PTE index is PVNULL, no valid PTE for the frame
 *		reference and modified bits are zero
 *
 * Mapped	PTE index is 1st HTAB index in list of valid PTEs for the frame
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
 *
 * MP locking:
 *
 * The PMAP_NFR lock protects the PVT and PVLIST structures for a
 * particular page frame.  It also prevents eviction of any PTEs mapping
 * the given frame thus protecting the fields of those PTEs.
 * The PMAP_PTEG lock protects access to a PTE group (both primary
 * and secondary hash groups).  Note that it does not protect updates
 * to the ppc_valid, ppc_refbit, ppc_modbit, nor ppc_key fields since
 * these are modified by routines that acquire only the PMAP_NFR lock.
 * The invtlb() routine for MP uses a larx/stwcx wrapper around the
 * tlbie to ensure that only one TLB invalidate occurs at a time.
 *
 * Notes:
 *
 * Currently only p_enter_ppc, p_inspte_ppc, p_delpte_ppc, and
 * p_getpte_ppc are coded to handle page frame numbers which don't
 * correspond to real memory (e.g. for a T=0 I/O mapping).
 * The p_enter_ppc routine is the only one that is currently called
 * by machine-independent code with a value of 'nfr' that may not
 * correspond to real memory.
 */

void	p_enter_ppc();
void	p_rename_ppc();
void	p_remove_ppc();
void	p_remove_all_ppc();
int	p_is_modified_ppc();
int	p_is_referenced_ppc();
void	p_clear_modify_ppc();
void	p_protect_ppc();
void	p_page_protect_ppc();
int	p_lookup_ppc();

void	p_inspte_ppc();
void	p_delallpte_ppc();
void	p_delpte_ppc();
int	p_getpte_ppc();

/*
 * Since we run xlate off we can't use normal assert
 * macros which call printf when DEBUG is enabled.
 */
#ifdef _VMMDEBUG
/* Used to avoid spurious calls to v_reload in a debugger (kdb) running xlate off.
 */
#define P_ASSERT(p)	{if(!(p))brkpoint();}	
#else
#define P_ASSERT(p)	{__assert2(__assert1((unsigned)(p),0,99));}
#endif /* _VMMDEBUG */

/*
 * Generate a random number in the range [0,x].
 * Low-order 8 bits of decrementer aren't implemented yet.
 */
#define	RANDOM(x)	((mfdec() >> 8) & (x))

#ifdef _POWER_MP
/*
 * Synchronization required on MP for TLB invalidates.
 * Note that TLBSYNC is not supported on 601.
 */
#define TLBSYNC()	tlbsync()
#define MP_SYNC()				\
	if (__power_mp() && !__power_601()) {	\
		TLBSYNC();			\
		SYNC();				\
	}

/*
 * Storage defined for PMAP_xxx locks and TLB lock.
 * These locks must reside in V=R storage.  They are padded
 * to ensure that each lock resides in a different cache line
 * (assumed to be a maximum of 64 bytes).  This padding must
 * also be large enough to contain the lock instrumentation
 * data that is recorded by the rsimple_lock/unlock services.
 */
struct cache_line_lock pmap_nfr_lock[NUM_NFR_LOCKS] = { SIMPLE_LOCK_AVAIL };
struct cache_line_lock pmap_pteg_lock[NUM_PTEG_LOCKS] = { SIMPLE_LOCK_AVAIL };
struct cache_line_lock tlb_lock = { SIMPLE_LOCK_AVAIL };

#else /* _POWER_MP */

#define MP_SYNC()

#endif /* _POWER_MP */

/*
 * Invalidate TLB.
 * Note that on MP for 601 this routine must do a 'sync'
 * immediately following the tlbie.
 * Also for MP this routine must use a larx/stwcx sequence
 * to ensure that only one tlbie occurs at a time.
 */
#define INVTLB(sid,pno)	invtlb(sid,pno)

/*
 * Cache operations.
 */
#define	SYNCCACHE(raddr)	sync_cache_page_ppc(raddr)
#define	INVCACHE(raddr)		inval_icache_page_ppc(raddr)

/*
 * p_enter_ppc(type,sid,pno,nfr,key,attr)
 *
 * Insert a hardware page table entry.
 */
void
p_enter_ppc(type,sid,pno,nfr,key,attr)
uint	type;	/* type of operation */
uint	sid;	/* segment id */
uint	pno;	/* page number in segment */
uint	nfr;	/* page frame number */
uint	key;	/* storage protect key  */
uint	attr;	/* storage control attributes */
{
#ifdef	DEBUG
	uint oldnfr;
#endif	/* DEBUG */

	/*
	 * If this is a reload then the PMAP_NFR lock is
	 * already held and we treat this is a normal mapping.
	 */
	if (type == RELOAD)
	{
		type = NORMAL;
	}
	else
	{
		if (PHYS_MEM(nfr)) PMAP_NFR_MPLOCK(nfr);
	}

#ifdef	DEBUG
	/*
	 * A (sid,pno) may only be mapped to one page frame.
	 */
	oldnfr = p_lookup_ppc(sid,pno);
	P_ASSERT(oldnfr == -1 || oldnfr == nfr);
#endif	/* DEBUG */

	/*
	 * When completing a page-in I/O (IOSYNC) we must flush the
	 * cache if the page is modified to ensure that real memory
	 * contains the latest state of the page because of separate
	 * instruction and data caches.  If the page is not modified
	 * we have to invalidate the cache since we don't invalidate
	 * at page release time.  It is important to perform the
	 * cache operation after the real page has been initialized
	 * in order to remove any effects of speculative execution
	 * on the caches.  It is also important for MP to perform
	 * the cache operation before the page is visible at its
	 * normal address.
	 */
	if (type == IOSYNC)
	{
		/*
		 * See if the frame is modified by checking the PVT
		 * and also the PTE if the frame is mapped (normally
		 * the frame has one mapping at the I/O address but
		 * it may have been removed due to overflow).
		 */
		if (ppc_swmod(nfr) ||
		    (ppc_ptex(nfr) != PVNULL && ppc_modbit(ppc_ptex(nfr))))
		{
			SYNCCACHE(nfr << L2PSIZE);
		}
		else
		{
			INVCACHE(nfr << L2PSIZE);
		}
	}

	/*
	 * Delete any existing mappings for non-NORMAL operations.
	 */
	if (type != NORMAL)
	{
		p_delallpte_ppc(nfr);
	}

	/*
	 * For I/O completion operations clear the referenced and
	 * modified bits for the frame (in the PVT only since the
	 * PTE bits were just cleared in p_delallpte_ppc).
	 */
	if (type == IODONE || type == IOSYNC)
	{
		ppc_swref(nfr) = 0;
		ppc_swmod(nfr) = 0;
	}

	/*
	 * Insert the new mapping (advisory only since
	 * this may fail on MP).
	 */
	p_inspte_ppc(sid,pno,nfr,key,attr);

	if (PHYS_MEM(nfr)) PMAP_NFR_MPUNLOCK(nfr);
}

/*
 * p_rename_ppc(sid,pno,nfr,key,attr)
 *
 * Rename a hardware page table entry.
 */
void
p_rename_ppc(sid,pno,nfr,key,attr)
uint	sid;	/* segment id */
uint	pno;	/* page number in segment */
uint	nfr;	/* page frame number */
uint	key;	/* storage protect key  */
uint	attr;	/* storage control attributes */
{

	PMAP_NFR_MPLOCK(nfr);

	/*
	 * Delete any existing mappings.
	 */
	p_delallpte_ppc(nfr);

	/*
	 * Insert the new mapping (advisory only since
	 * this may fail on MP).
	 */
	p_inspte_ppc(sid,pno,nfr,key,attr);

	PMAP_NFR_MPUNLOCK(nfr);
}

/*
 * p_remove_ppc(sid,pno,nfr)
 *
 * Delete a hardware page table entry for an alias address.
 */
void
p_remove_ppc(sid,pno,nfr)
uint		sid;	/* segment id */
uint		pno;	/* page number in segment */
uint		nfr;	/* page frame number */
{
	uint ptex, ssid, spno;

	PMAP_NFR_MPLOCK(nfr);

	/*
	 * Search the pvlist for an entry mapped at (sid,pno) and delete it.
	 */
	ptex = ppc_ptex(nfr);
	while (ptex != PVNULL)
	{
		ssid = ppc_sid(ptex);
		spno = PTEX2PNO_PPC(ptex);
		if (ssid == sid && spno == pno)
		{
			PMAP_PTEG_MPLOCK(sid,pno);
			p_delpte_ppc(ptex,sid,pno,nfr);
			PMAP_PTEG_MPUNLOCK(sid,pno);
			break;
		}
		ptex = ppc_pvnext(ptex);
	}

	PMAP_NFR_MPUNLOCK(nfr);
}

/*
 * p_remove_all_ppc(nfr)
 *
 * Delete all hardware page table entries.
 */
void
p_remove_all_ppc(nfr)
uint	nfr;	/* page frame number */
{

	PMAP_NFR_MPLOCK(nfr);

	/*
	 * Delete any existing mappings.
	 */
	p_delallpte_ppc(nfr);

	/*
	 * Clear reference and change bits in PVT.
	 */
	ppc_swref(nfr) = 0;
	ppc_swmod(nfr) = 0;

	PMAP_NFR_MPUNLOCK(nfr);
}

/*
 * p_is_modified_ppc(nfr)
 *
 * Return the modified status of a page frame.
 */
p_is_modified_ppc(nfr)
uint	nfr;	/* page frame number */
{
	uint ptex, modstat;

	PMAP_NFR_MPLOCK(nfr);

	/*
	 * Search the pvlist for a modified entry and if found propogate
	 * the modified status to the PVT.
	 */
	ptex = ppc_ptex(nfr);
	while (ptex != PVNULL)
	{
		if (ppc_modbit(ptex))
		{
			ppc_swmod(nfr) = 1;
			break;
		}
		ptex = ppc_pvnext(ptex);
	}

	/*
	 * Now return the modification status in the PVT.
	 */
	modstat = ppc_swmod(nfr);

	PMAP_NFR_MPUNLOCK(nfr);

	return(modstat);
}

/*
 * p_is_referenced_ppc(nfr)
 *
 * Return and reset the referenced status of a page frame.
 */
p_is_referenced_ppc(nfr)
uint     nfr;    /* page frame number */
{
	uint ptex, refstat;
	char byte;
	volatile char *ref;

	PMAP_NFR_MPLOCK(nfr);

	/*
	 * Search the pvlist for a referenced entry and if found propogate
	 * the referenced status to the PVT.  Also, clear the referenced
	 * status of each entry.
	 */
	ptex = ppc_ptex(nfr);
	while (ptex != PVNULL)
	{
		if (ppc_refbit(ptex))
		{
			ppc_swref(nfr) = 1;

			/*
			 * On MP this needs to be a byte store
			 * not to interact w/ the modbit set by another cpu.
			 */
			ref = &ppc_refbyte(ptex);
			byte = *ref;
			byte &= 0xfe;
			*ref = byte;
			INVTLB(ppc_sid(ptex),PTEX2PNO_PPC(ptex));
		}
		ptex = ppc_pvnext(ptex);
	}

	/*
	 * Now return (and clear) the referenced status in the PVT.
	 */
	if (ppc_swref(nfr))
	{
		ppc_swref(nfr) = 0;
		refstat = TRUE;
	}
	else
	{
		refstat = FALSE;
	}

	PMAP_NFR_MPUNLOCK(nfr);

	return(refstat);
}

/*
 * p_clear_modify_ppc(nfr)
 *
 * Clear the modified status of a page frame.
 */
void
p_clear_modify_ppc(nfr)
uint	nfr;	/* page frame number */
{
	uint ptex;

	PMAP_NFR_MPLOCK(nfr);

	/*
	 * Clear the modified status of each entry in the pvlist.
	 */
	ptex = ppc_ptex(nfr);
	while (ptex != PVNULL)
	{
		if (ppc_modbit(ptex))
		{
			ppc_valid(ptex) = 0;
			SYNC();
			INVTLB(ppc_sid(ptex),PTEX2PNO_PPC(ptex));
			MP_SYNC();
			ppc_modbit(ptex) = 0;
			SYNC();
			ppc_valid(ptex) = 1;
		}
		ptex = ppc_pvnext(ptex);
	}

	/*
	 * Now clear the modification status in the PVT.
	 */
	ppc_swmod(nfr) = 0;

	PMAP_NFR_MPUNLOCK(nfr);
}

/*
 * p_protect_ppc(sid,pno,nfr,key)
 *
 * Lower permissions for a specified mapping of a page frame.
 */
void
p_protect_ppc(sid,pno,nfr,key)
uint	sid;	/* segment id */
uint	pno;	/* page number in segment */
uint	nfr;	/* page frame number */
uint	key;	/* storage protect key  */
{
	uint ptex, ssid, spno;

	PMAP_NFR_MPLOCK(nfr);

	/*
	 * Search the pvlist for an entry mapped at (sid,pno) and
	 * change its protection key.
	 */
	ptex = ppc_ptex(nfr);
	while (ptex != PVNULL)
	{
		ssid = ppc_sid(ptex);
		spno = PTEX2PNO_PPC(ptex);
		if (ssid == sid && spno == pno)
		{
			ppc_valid(ptex) = 0;
			SYNC();
			INVTLB(sid,pno);
			MP_SYNC();
			ppc_key(ptex) = key;
			SYNC();
			ppc_valid(ptex) = 1;
			break;
		}
		ptex = ppc_pvnext(ptex);
	}

	PMAP_NFR_MPUNLOCK(nfr);
}

/*
 * p_page_protect_ppc(nfr,key)
 *
 * Lower permissions for all mappings of a specified page frame.
 */
void
p_page_protect_ppc(nfr,key)
uint	nfr;	/* page frame number */
uint	key;	/* storage protect key  */
{
	uint ptex;

	PMAP_NFR_MPLOCK(nfr);

	/*
	 * Update the page protection of each entry in the pvlist.
	 */
	ptex = ppc_ptex(nfr);
	while (ptex != PVNULL)
	{
		ppc_valid(ptex) = 0;
		SYNC();
		INVTLB(ppc_sid(ptex),PTEX2PNO_PPC(ptex));
		MP_SYNC();
		ppc_key(ptex) = key;
		SYNC();
		ppc_valid(ptex) = 1;
		ptex = ppc_pvnext(ptex);
	}

	PMAP_NFR_MPUNLOCK(nfr);
}

/*
 * p_inspte_ppc(sid,pno,nfr,key,attr)
 *
 * Insert a PTE in the HTAB.
 */
void
p_inspte_ppc(sid,pno,nfr,key,attr)
uint     sid;   /* segment id */
uint     pno;   /* page number in segment */
uint     nfr;   /* page frame number    */
uint     key;   /* storage protect key  */
uint	attr;	/* storage control attributes */
{
	uint ptex, hsel;

	if (PHYS_MEM(nfr)) PMAP_NFR_LOCKHOLDER(nfr);

#ifdef _POWER_MP
	/*
	 * If the mapping already exists just return.
	 * XXX - Also restrict the maximum number of mappings per frame?
	 */
	if (PHYS_MEM(nfr))
	{
		ptex = ppc_ptex(nfr);
		while (ptex != PVNULL)
		{
			if (ppc_sid(ptex) == sid && PTEX2PNO_PPC(ptex) == pno)
				return;
			ptex = ppc_pvnext(ptex);
		}
	}
#endif /* _POWER_MP */

	/*
	 * Find a free PTE in the hash group for (sid,pno).
	 */
	PMAP_PTEG_MPLOCK(sid,pno);

	ptex = p_getpte_ppc(sid,pno,&hsel);
	if (ptex == PVNULL)
	{
		/*
		 * On MP we may not be able to acquire all necessary locks
		 * to allocate a PTE entry when the hash group is full so
		 * we just release locks and return.  The entry will be
		 * inserted at a later point if needed (i.e. reload fault).
		 */
		PMAP_PTEG_MPUNLOCK(sid,pno);
		return;
	}

	/*
	 * Word 0 - SID, hsel, avpi.
	 */
	ppc_word0(ptex) = PTE0_PPC(sid, hsel, AVPI_PPC(pno));

	/*
	 * Word 1 - Real Page Number, protection key, storage attributes
	 */
	ppc_word1(ptex) = PTE1_PPC(nfr, attr, key);

	/*
	 * For mappings of physical memory, insert at head
	 * of pvlist.
	 */
	if (PHYS_MEM(nfr))
	{
		ppc_pvpno(ptex) = PVPNO_PPC(pno);
		ppc_pvnext(ptex) = ppc_ptex(nfr);
		ppc_ptex(nfr) = ptex;
	}

	/*
	 * Now set the valid bit.
	 */
	SYNC();
        ppc_valid(ptex) = 1;

	PMAP_PTEG_MPUNLOCK(sid,pno);

	return;
}

/*
 * p_delallpte_ppc(nfr)
 *
 * Delete all hardware page table entries for a specified frame.
 */
void
p_delallpte_ppc(nfr)
uint	nfr;	/* page frame number */
{
	uint ptex, next;
	uint sid, pno;

	PMAP_NFR_LOCKHOLDER(nfr);

	/*
	 * Delete each entry in the pvlist.
	 */
	ptex = ppc_ptex(nfr);
	while (ptex != PVNULL)
	{
		next = ppc_pvnext(ptex);
		sid = ppc_sid(ptex);
		pno = PTEX2PNO_PPC(ptex);
		PMAP_PTEG_MPLOCK(sid,pno);
		p_delpte_ppc(ptex,sid,pno,nfr);
		PMAP_PTEG_MPUNLOCK(sid,pno);
		ptex = next;
	}
}

/*
 * p_delpte_ppc(ptex,sid,pno,nfr)
 *
 * Delete a PTE from the HTAB.
 */
void
p_delpte_ppc(ptex,sid,pno,nfr)
uint     ptex;	/* PTE index */
uint     sid;   /* segment id */
uint     pno;   /* page number in segment */
uint     nfr;   /* page frame number    */
{
        uint prev;

	if (PHYS_MEM(nfr)) PMAP_NFR_LOCKHOLDER(nfr);
	PMAP_PTEG_LOCKHOLDER(sid,pno);

#ifdef	DEBUG
	P_ASSERT(ppc_valid(ptex));
	P_ASSERT(p_lookup_ppc(sid,pno) == nfr);
#endif

	/*
	 * Clear word containing the valid bit.
	 */
	ppc_word0(ptex) = 0;

	/*
	 * Invalidate TLB.
	 */
	SYNC();
	INVTLB(sid,pno);
	MP_SYNC();

	/*
	 * For mappings of physical memory, preserve the
	 * reference and modify bits and remove from pvlist.
	 */
	if (PHYS_MEM(nfr))
	{
		ppc_swref(nfr) |= ppc_refbit(ptex);
		ppc_swmod(nfr) |= ppc_modbit(ptex);

		if (ptex == ppc_ptex(nfr))
		{
			ppc_ptex(nfr) = ppc_pvnext(ptex);
		}
		else
		{
			prev = ppc_ptex(nfr);
			while (ptex != ppc_pvnext(prev))
			{
				prev = ppc_pvnext(prev);
			}
			ppc_pvnext(prev) = ppc_pvnext(ptex);
		}
	}

	/*
	 * Clear word containing reference and modify bits.
	 */
	ppc_word1(ptex) = 0;
}

/*
 * Statistics on PTE replacement.
 *
 * # of primary entries moved from primary to secondary hash group
 * # of secondary entries removed from primary hash group
 * # of primary entries removed from secondary hash group
 * # of secondary entries removed from secondary hash group
 * # of abandoned replacements (due to locks not being available)
 * Count of which slot (0-7) things were moved/removed from to
 * determine how random the selection is.
 */
struct {
	uint	primary1;
	uint	primary2;
	uint	secondary1;
	uint	secondary2;
	uint	abandon1;
	uint	abandon2;
	uint	abandon3;
	uint	unused;
	uint	replace[PTEGSIZE];
} pmap_stat = { 0 };

/*
 * p_getpte_ppc(sid,pno,&hsel)
 *
 * Find a free PTE in the hash group for (sid,pno).
 */
int 
p_getpte_ppc(sid,pno,hsel)
uint     sid;    /* segment id */
uint     pno;    /* page number in segment */
uint	*hsel;	 /* indicate which hash group entry was found in */
{
        uint hash1, hash2, ptex;
	uint ptex1, ptex2;
	uint word0, word1;
	uint nfr, pvpno, secondary;

	PMAP_PTEG_LOCKHOLDER(sid,pno);

	/*
	 * Search the primary hash group for a free entry.
	 */
	*hsel = 0;
	hash1 = HASHF(sid,pno);
	for (ptex = FIRSTPTE(hash1); ptex < FIRSTPTE(hash1) + PTEGSIZE; ptex++)
	{
		/*
		 * Since a valid bit may be reset by a routine which
		 * doesn't hold the PMAP_PTEG lock we must check both
		 * words to find a free entry (assumes that frame zero
		 * which is mapped to sid zero and pno zero has non-zero
		 * key or wimg bits).
		 */
		if (ppc_word0(ptex) == 0 && ppc_word1(ptex) == 0)
			goto found;
	}

	/*
	 * Starting at a random entry in the primary hash group, search
	 * circularly for a secondary entry and if one is found remove it
	 * and return its index.  We do this before searching for a free
	 * entry in the secondary hash group to avoid long lived secondary
	 * entries.
	 */
	ptex1 = FIRSTPTE(hash1) + RANDOM(7);
	for (ptex = ptex1; ptex < FIRSTPTE(hash1) + PTEGSIZE; ptex++)
	{
		if (ppc_hsel(ptex))
		{
			vmker.overflows++;
			sid = ppc_sid(ptex);
			pno = HTAB2PNO_PPC(ptex); /* Can't use pvlist version */
			nfr = ppc_rpn(ptex);
			if (PHYS_MEM(nfr))
			{
				/*
				 * We are violating the lock hierarchy by
				 * acquiring another PMAP_NFR lock here so we
				 * do a lock try and if it fails we bail out.
				 */
				if (!PMAP_NFR_MPLOCK_TRY(nfr))
				{
					pmap_stat.abandon1++;
					return(PVNULL);
				}
			}
			p_delpte_ppc(ptex,sid,pno,nfr);
			pmap_stat.primary2++;
			pmap_stat.replace[ptex & (PTEGSIZE - 1)]++;
			if (PHYS_MEM(nfr))
			{
				PMAP_NFR_MPUNLOCK(nfr);
			}
			goto found;
		}
	}

	for (ptex = FIRSTPTE(hash1); ptex < ptex1; ptex++)
	{
		if (ppc_hsel(ptex))
		{
			vmker.overflows++;
			sid = ppc_sid(ptex);
			pno = HTAB2PNO_PPC(ptex); /* Can't use pvlist version */
			nfr = ppc_rpn(ptex);
			if (PHYS_MEM(nfr))
			{
				if (!PMAP_NFR_MPLOCK_TRY(nfr))
				{
					pmap_stat.abandon1++;
					return(PVNULL);
				}
			}
			p_delpte_ppc(ptex,sid,pno,nfr);
			pmap_stat.primary2++;
			pmap_stat.replace[ptex & (PTEGSIZE - 1)]++;
			if (PHYS_MEM(nfr))
			{
				PMAP_NFR_MPUNLOCK(nfr);
			}
			goto found;
		}
	}

	/*
	 * Search the secondary hash group for a free entry.
	 */
	*hsel = 1;
	hash2 = HASHF2(sid,pno);
	for (ptex = FIRSTPTE(hash2); ptex < FIRSTPTE(hash2) + PTEGSIZE; ptex++)
	{
		if (ppc_word0(ptex) == 0 && ppc_word1(ptex) == 0)
			goto found;
	}

	/*
	 * Remove a random entry in the secondary hash group, move the
	 * random entry from the primary hash group to it and return
	 * the index of the random entry in the primary hash group.
	 * From here on, we always return a PTE index from the primary
	 * hash group so set hsel accordingly.
	 */
	vmker.overflows++;
	*hsel = 0;

	/*
	 * Select and remove a random entry in the secondary hash group.
	 */
	ptex2 = FIRSTPTE(hash2) + RANDOM(7);
	sid = ppc_sid(ptex2);
	pno = HTAB2PNO_PPC(ptex2); /* Can't use pvlist version */
	nfr = ppc_rpn(ptex2);
	secondary = ppc_hsel(ptex2);
	if (PHYS_MEM(nfr))
	{
		if (!PMAP_NFR_MPLOCK_TRY(nfr))
		{
			pmap_stat.abandon2++;
			return(PVNULL);
		}
	}
	p_delpte_ppc(ptex2,sid,pno,nfr);
	if (secondary)
		pmap_stat.secondary2++;
	else
		pmap_stat.secondary1++;
	pmap_stat.replace[ptex2 & (PTEGSIZE - 1)]++;
	if (PHYS_MEM(nfr))
	{
		PMAP_NFR_MPUNLOCK(nfr);
	}

	/*
	 * Save the contents of the primary hash group entry and remove it.
	 */
	word0 = ppc_word0(ptex1);
	word1 = ppc_word1(ptex1);
	pvpno = ppc_pvpno(ptex1);
	sid = ppc_sid(ptex1);
	pno = HTAB2PNO_PPC(ptex1); /* Can't use pvlist version */
	nfr = ppc_rpn(ptex1);
	if (PHYS_MEM(nfr))
	{
		if (!PMAP_NFR_MPLOCK_TRY(nfr))
		{
			pmap_stat.abandon3++;
			return(PVNULL);
		}
	}
	p_delpte_ppc(ptex1,sid,pno,nfr);
	pmap_stat.primary1++;
	pmap_stat.replace[ptex1 & (PTEGSIZE - 1)]++;

	/*
	 * Now move the saved contents of the primary hash group entry
	 * to the secondary hash group.  We must mark the entry as a
	 * secondary entry and we must initially set the valid bit off
	 * to ensure an atomic update.
	 */
	ppc_word0(ptex2) = (word0 | PTE_HSEL_PPC) & ~PTE_VALID_PPC;
	ppc_word1(ptex2) = word1;
	SYNC();
	ppc_valid(ptex2) = 1;

	/*
	 * For mappings of physical memory, insert the entry
	 * at the head of the pvlist.
	 */
	if (PHYS_MEM(nfr))
	{
		ppc_pvpno(ptex2) = pvpno;
		ppc_pvnext(ptex2) = ppc_ptex(nfr);
		ppc_ptex(nfr) = ptex2;
		PMAP_NFR_MPUNLOCK(nfr);
	}
	
	/*
	 * Return the free entry in the primary hash group.
	 */
	ptex = ptex1;

found:
	return(ptex);
}

/*
 * p_lookup_ppc(sid,pno)
 *
 * Look up the page frame for a specified virtual address.
 * Note for MP that the caller holds the PMAP_NFR lock so
 * we don't need to lock here.
 */
int 
p_lookup_ppc(sid,pno)
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
		if (ppc_valid(ptex) && ppc_sid(ptex) == sid &&
		    ppc_avpi(ptex) == AVPI_PPC(pno) &&
		    ppc_hsel(ptex) == 0)
			return(ppc_rpn(ptex));
	}

	/*
	 * Search secondary hash group.
	 */
	hash = HASHF2(sid,pno);
	for (ptex = FIRSTPTE(hash); ptex < FIRSTPTE(hash) + PTEGSIZE; ptex++)
	{
		if (ppc_valid(ptex) && ppc_sid(ptex) == sid &&
		    ppc_avpi(ptex) == AVPI_PPC(pno) &&
		    ppc_hsel(ptex) == 1)
			return(ppc_rpn(ptex));
	}

        return(-1);
}
