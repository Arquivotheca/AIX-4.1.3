static char sccsid[] = "@(#)92	1.3  src/bos/kernel/vmm/POWER/v_power_rs1.c, sysvmm, bos411, 9428A410j 1/14/94 07:09:21";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:	p_enter_rs1(), p_rename_rs1(),
 *		p_remove_rs1(), p_remove_all_rs1(),
 *		p_is_modified_rs1(), p_is_referenced_rs1(),
 *		p_clear_modify_rs1(),
 *		p_protect_rs1(), p_page_protect_rs1(),
 *		p_lookup_rs1()
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

/*
 * Machine-dependent routines for POWER_RS1 and POWER_RSC.
 */

/*
 * This code manages the Hash Anchor Table (HAT) and Page Frame Table (PFT)
 * for the RS1 and RSC platforms.
 *
 * These platforms have the following characteristics:
 *
 *	RS1	RSC
 *	---	---
 *	N	N	aliasing supported by page tables
 *	N	Y	aliasing supported by caches (no cache synonyms)
 *	N	Y	combined data and instruction cache
 *	N	N	separate data and instruction cache operations
 *	N	Y	data cache consistent with I/O transfers
 *
 * All code is written to the RS1 characteristics --
 * no optimizations are made for the RSC combined cache.
 *
 * These platforms utilize an inverted page table which only provides a
 * single virtual address mapping for any real address.  Virtual address
 * aliasing is provided by "ping-ponging" a page frame between the different
 * virtual addresses for the frame.
 *
 * Each PFT entry is in one of the following states:
 *
 * Invalid	The valid bit is 0
 *		rs1_sid, rs1_page, rs1_key are undefined, no TLB entry
 *		The referenced and modified bits are zero
 *		There are no active cache lines for the page frame
 * Mapped	The valid bit is 1
 *		rs1_sid, rs1_page, rs1_key give the mapping, may have TLB entry
 *		The referenced and modified bits are valid
 *		There may be active cache lines for the page frame
 * Unmapped	The valid bit is 0
 *		rs1_sid, rs1_page, rs1_key are undefined, no TLB entry
 *		The referenced and modified bits are valid
 *		There are no active cache lines for the page frame
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

void	p_enter_rs1();
void	p_rename_rs1();
void	p_remove_rs1();
void	p_remove_all_rs1();
int	p_is_modified_rs1();
int	p_is_referenced_rs1();
void	p_clear_modify_rs1();
void	p_protect_rs1();
void	p_page_protect_rs1();
int	p_lookup_rs1();

void	p_inspft_rs1();
void	p_delpft_rs1();

/*
 * Invalidate TLB.
 */
#define INVTLB(sid,pno)	invtlb(sid,pno)

/*
 * Cache operations.
 */
#define	SYNCCACHE(sid,pno)	sync_cache_page_pwr(sid,pno)

/*
 * p_enter_rs1(type,sid,pno,nfr,key,attr)
 *
 * Insert a hardware page table entry.
 */
void
p_enter_rs1(type,sid,pno,nfr,key,attr)
uint	type;	/* type of operation */
uint	sid;	/* segment id */
uint	pno;	/* page number in segment */
uint	nfr;	/* page frame number */
uint	key;	/* storage protect key  */
uint	attr;	/* storage control attributes */
{
	uint ssid, spno;

	/*
	 * If the frame is mapped then ping-pong it to the new address
	 * otherwise insert it.
	 */
	if (rs1_valid(nfr))
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
		ssid = rs1_sid(nfr);
		spno = rs1_page(nfr);
		if (CACHE_DIFF(spno, pno) ||
		    (rs1_modbit(nfr) && (type == STARTIO || type == IOSYNC)))
		{
			SYNCCACHE(ssid,spno);
		}

		/*
		 * If source and target fall in the same hash class
		 * (i.e. when moving to and from I/O address) then there
		 * is no need to move it to a different hash chain.
		 */
		if (HASHF(ssid,spno) == HASHF(sid,pno))
		{
			rs1_sidpno(nfr) = SIDPNO(sid,pno);
			rs1_key(nfr) = key;
			rs1_page(nfr) = pno;
			INVTLB(ssid,spno);
		}
		else
		{
			p_delpft_rs1(nfr);
			p_inspft_rs1(sid,pno,nfr,key);
		}
	}
	else
	{
		p_inspft_rs1(sid,pno,nfr,key);
	}

	/*
	 * For I/O completion operations clear the referenced and
	 * modified bits for the frame.
	 */
	if (type == IODONE || type == IOSYNC)
	{
		rs1_refbit(nfr) = 0;
		rs1_modbit(nfr) = 0;
	}
}

/*
 * p_rename_rs1(sid,pno,nfr,key,attr)
 *
 * Rename a hardware page table entry.
 */
void
p_rename_rs1(sid,pno,nfr,key,attr)
uint	sid;	/* segment id */
uint	pno;	/* page number in segment */
uint	nfr;	/* page frame number */
uint	key;	/* storage protect key  */
uint	attr;	/* storage control attributes */
{
	/*
	 * Just treat as a normal enter.
	 */
	p_enter_rs1(NORMAL, sid, pno, nfr, key, attr);
}

/*
 * p_remove_rs1(sid,pno,nfr)
 *
 * Delete a hardware page table entry.
 */
void
p_remove_rs1(sid,pno,nfr)
uint	sid;	/* segment id */
uint	pno;	/* page number in segment */
uint	nfr;	/* page frame number */
{
	/*
	 * If the frame is mapped at (sid,pno) delete it.
	 */
	if (rs1_valid(nfr) && rs1_sid(nfr) == sid && rs1_page(nfr) == pno)
	{
		SYNCCACHE(sid,pno);
		p_delpft_rs1(nfr);
	}
}

/*
 * p_remove_all_rs1(sid,pno,nfr)
 *
 * Delete all hardware page table entries.
 */
void
p_remove_all_rs1(nfr)
uint	nfr;	/* page frame number */
{
	uint sid;
	uint pno;

	/*
	 * If the frame is mapped delete it.
	 */
	if (rs1_valid(nfr))
	{
		sid = rs1_sid(nfr);
		pno = rs1_page(nfr);
		SYNCCACHE(sid,pno);
		p_delpft_rs1(nfr);
	}

	/*
	 * Clear reference and change bits (and other fields as well).
	 */
	rs1_word0(nfr) = 0;
}

/*
 * p_is_modified_rs1(nfr)
 *
 * Return the modified status of a page frame.
 */
p_is_modified_rs1(nfr)
uint	nfr;	/* page frame number */
{
	/*
	 * Assume the machine-independent code never calls this routine
	 * for an invalid frame.
	 */
	return(rs1_modbit(nfr));
}

/*
 * p_is_referenced_rs1(nfr)
 *
 * Return and reset the referenced status of a page frame.
 */
p_is_referenced_rs1(nfr)
uint     nfr;    /* page frame number */
{
	/*
	 * Assume the machine-independent code never calls this routine
	 * for an invalid frame.
	 */
	if (rs1_refbit(nfr))
	{
		rs1_refbit(nfr) = 0;

		/*
		 * Only invalidate the TLB if the frame is mapped.
		 */
		if (rs1_valid(nfr))
		{
			INVTLB(rs1_sid(nfr),rs1_page(nfr));
		}
		return(TRUE);
	}
	else
	{
		return(FALSE);
	}
}

/*
 * p_clear_modify_rs1(nfr)
 *
 * Clear the modified status of a page frame.
 */
void
p_clear_modify_rs1(nfr)
uint	nfr;	/* page frame number */
{
	rs1_modbit(nfr) = 0;

	/*
	 * Only invalidate the TLB if the frame is mapped.
	 */
	if (rs1_valid(nfr))
	{
		INVTLB(rs1_sid(nfr),rs1_page(nfr));
	}
}

/*
 * p_protect_rs1(sid,pno,nfr,key)
 *
 * Lower permissions for a specified mapping of a page frame.
 */
void
p_protect_rs1(sid,pno,nfr,key)
uint	sid;	/* segment id */
uint	pno;	/* page number in segment */
uint	nfr;	/* page frame number */
uint	key;	/* storage protect key  */
{
	/*
	 * Update the page protection if the frame is mapped to (sid,pno).
	 */
	if (rs1_valid(nfr) && rs1_sid(nfr) == sid && rs1_page(nfr) == pno)
	{
		rs1_key(nfr) = key;
		INVTLB(sid,pno);
	}
}

/*
 * p_page_protect_rs1(nfr,key)
 *
 * Lower permissions for all mappings of a specified page frame.
 */
void
p_page_protect_rs1(nfr,key)
uint	nfr;	/* page frame number */
uint	key;	/* storage protect key  */
{
	/*
	 * Update the page protection if the frame is mapped.
	 */
	if (rs1_valid(nfr))
	{
		rs1_key(nfr) = key;
		INVTLB(rs1_sid(nfr),rs1_page(nfr));
	}
}

/*
 * p_inspft_rs1(sid,pno,nfr,key)
 *
 * Insert a page frame on the h/w hash chain.
 */
void
p_inspft_rs1(sid,pno,nfr,key)
uint	sid;	/* segment id */
uint	pno;	/* page number in segment */
uint	nfr;	/* page frame number    */
uint	key;	/* storage protect key  */
{
        uint hash;

	/*
	 * Determine the HAT index.
	 */
        hash = HASHF(sid,pno);

	/*
	 * Update the entry.
	 */
        rs1_sidpno(nfr) = SIDPNO(sid,pno);
        rs1_key(nfr) = key;
        rs1_valid(nfr) = 1;

	/*
	 * Record the full page number.
	 */
	rs1_page(nfr) = pno;

        /*
         * Add it to the front of the hash chain.
         */
        rs1_next(nfr) = rs1_hat(hash);
        rs1_hat(hash) = nfr;
}

/*
 * p_delpft_rs1(nfr)
 *
 * Remove a page frame from the h/w hash chain.
 */
void
p_delpft_rs1(nfr)
uint     nfr;    /* page frame number */
{
        uint sid, pno, hash, prv;

	/*
	 * Determine the HAT index.
	 */
        sid = rs1_sid(nfr);
        pno = rs1_page(nfr);
        hash = HASHF(sid,pno);

	/*
	 * Mark the entry invalid.
	 */
	rs1_valid(nfr) = 0;

	/*
	 * Remove the entry from the hash chain.
	 */
        prv = rs1_hat(hash);
        if (prv == nfr)
        {
                rs1_hat(hash) = rs1_next(nfr);
        }
        else
        {
                while(rs1_next(prv) != nfr)
                {
                        prv = rs1_next(prv);
                }
                rs1_next(prv) = rs1_next(nfr);
        }

	/*
	 * Invalidate TLB.
	 */
	INVTLB(sid,pno);
}

/*
 * p_lookup_rs1(sid,pno)
 *
 * Look up the page frame for a specified virtual address.
 */
int 
p_lookup_rs1(sid,pno)
uint	sid;	/* segment id */
uint	pno;	/* page number in segment */
{
        int hash, nfr;

        hash = HASHF(sid,pno);

        for (nfr = FIRSTNFR(hash); nfr >= 0; nfr = rs1_next(nfr))
        {
                if (rs1_valid(nfr) && rs1_sid(nfr) == sid &&
		    rs1_page(nfr) == pno)
                        return(nfr);
        }

        return(-1);
}
