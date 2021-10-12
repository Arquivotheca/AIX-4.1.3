/* @(#)19       1.33  src/bos/kernel/vmm/POWER/vmmacs.h, sysvmm, bos412, 9445C412a 10/25/94 11:31:53 */
#ifndef _h_VMMACS
#define _h_VMMACS

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
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

/*
 * Macros for machine-dependent	page table interfaces.
 * For PTE reload support these interfaces are called via the assembler
 * glue code to set up the proper environment.
 * The P_ENTER macro is hooked for the bigfoot performance tool.
 */

#define	P_ENTER(type,sid,pno,nfr,key,attr)			\
{								\
	extern int BF_on;					\
	if (BF_on) BF_enter(type,sid,pno,nfr,key,attr);		\
	pcs_enter(type,sid,pno,nfr,key,attr);			\
}
#define	P_RENAME(sid,pno,nfr,key,attr)	pcs_rename(sid,pno,nfr,key,attr)
#define	P_REMOVE(sid,pno,nfr)		pcs_remove(sid,pno,nfr)
#define	P_REMOVE_ALL(nfr)		pcs_remove_all(nfr)
#define	P_PROTECT(sid,pno,nfr,key)	pcs_protect(sid,pno,nfr,key)
#define	P_PAGE_PROTECT(nfr,key)					\
{								\
	v_aptkeyall(nfr,key);					\
	pcs_page_protect(nfr,key);				\
}
#define	P_LOOKUP(sid,pno)		pcs_lookup(sid,pno)

#define	ISREF(nfr)		pcs_is_referenced(nfr)
#define ISMOD(nfr)		(pft_modbit(nfr) ? 1 : pcs_is_modified(nfr))
#define CLRMOD(nfr)		{pft_modbit(nfr) = 0; pcs_clear_modify(nfr);}
#define SETMOD(nfr)		pft_modbit(nfr) = 1

/*
 * Synchronization required for page table updates.
 */
#define	SYNC()		__iospace_sync()

/*
 * Macros for other machine-dependent interfaces.
 */
#define	COPYPAGE(tsid,tpno,ssid,spno)	v_copypage(tsid,tpno,ssid,spno)
#define ZEROPAGE(sid,pno)		v_zpage(sid,pno)
#define ZEROFRAG(sid,pno,off)		v_zerofrag(sid,pno,off)

#if defined (_POWER_RS1) || defined (_POWER_RSC)

/*
 * sidvpn field	to put in hardware pft.
 */
#define	SIDPNO(sid,pno)		( (sid) << 3 ) | ( (pno) >> 13 )

/*
 * first page frame in a hash class
 */
#define	FIRSTNFR(hash)		( rs1_hat(hash) )

#endif /* _POWER_RS1 ||	_POWER_RSC */


#if defined (_POWER_RS2) || defined (_POWER_PC)

/*
 * Index of first PTE in a hash	group.
 */
#define	FIRSTPTE(hash)	((hash)	<< L2PTEGSIZE)

#endif /* _POWER_RS2 ||	_POWER_PC */


#ifdef _POWER_RS2

/*
 * Convert 16-bit page number to abbreviated virtual page index.
 * For RS2 the avpi field is 5 bits.
 */
#define	AVPI_RS2(pno)	((pno) >> 11)

/*
 * Convert PTE index to 16-bit page number.
 * Concatenate 5-bit avpi with 11 low order bits calculated by solving
 * hash formula for pno.
 */
#define PTEX2PNO_RS2(ptex) \
	( (rs2_avpi(ptex) << 11) | \
	  (rs2_hsel(ptex) ? ~(((ptex) / PTEGSIZE) ^ rs2_sid(ptex)) & 0x7ff \
	  		  : (((ptex) / PTEGSIZE) ^ rs2_sid(ptex)) & 0x7ff) )

#endif /* _POWER_RS2 */


#ifdef _POWER_PC

/*
 * Convert 16-bit page number to abbreviated virtual page index.
 * For PPC the avpi field is 6 bits.
 */
#define	AVPI_PPC(pno)	((pno) >> 10)

/*
 * Preserve 10 bits of page number in PVLIST entry.
 */
#define	PVPNO_PPC(pno)	((pno) & 0x3ff)

/*
 * Convert PTE index to 16-bit page number.
 * Concatenate 6-bit avpi with 10 low order bits from PVLIST entry.
 */
#define PTEX2PNO_PPC(ptex) \
	( (ppc_avpi(ptex) << 10) | ppc_pvpno(ptex) )

/*
 * Convert PTE index to 16-bit page number (works for T=0 mappings).
 * Concatenate 6-bit avpi with 10 low order bits calculated by solving
 * hash formula for pno.
 */
#define HTAB2PNO_PPC(ptex) \
	( (ppc_avpi(ptex) << 10) | \
	  (ppc_hsel(ptex) ? ~(((ptex) / PTEGSIZE) ^ ppc_sid(ptex)) & 0x3ff \
	  		  : (((ptex) / PTEGSIZE) ^ ppc_sid(ptex)) & 0x3ff) )

/*
 * Initialize words in PTE.
 */
#define PTE0_PPC(sid,hsel,avpi)	( ((sid) << 7) | ((hsel) << 6) | (avpi) )
#define PTE1_PPC(rpn,wimg,key)	( ((rpn) << 12) | ((wimg) << 3) | (key) )

#endif /* _POWER_PC */

/*
 * Hardware hash function.
 */
#define	HASHF(sid,vpn)	(((sid)	^ (vpn)) & vmker.hashmask)

/*
 * Secondary hardware hash function.
 */
#define	HASHF2(sid,vpn)	(~((sid) ^ (vpn)) & vmker.hashmask)

/* hash function for converting sid to index in scb array and
 * conversely from an index in the scb and a page to the sid
 * covering page. 
 */
#define  STOI(sid)  \
( ((sid) ^ (((sid) & vmker.stoimask) <<	vmker.stoibits)) & 0x000fffff )

#define  ITOS(ind, pno) \
( ((ind) ^ (((ind) & vmker.stoimask) <<	vmker.stoibits)) \
	+ (((pno) & 0x70000)<< 4) )

/*
 * Software hash functions.
 */
#define	SWHASH(sid,vpn)		(((sid)	^ (vpn)) & vmker.swhashmask)
#define	SWFIRSTNFR(hash)	( hattab[(hash)] )
#define	AHASH(sid,vpn)		(((sid)	^ (vpn)) & vmker.ahashmask)

/* macro for checking cache congruency.
 * determines if 2 page	numbers	differ in bits used by
 * cache lookup	(if they are in	different congruence classes)
 */
#define	CACHE_DIFF(p1,p2)	\
	(((p1) ^ (p2)) & ((vmker.cachealign - 1) >> L2PSIZE))

/* check if an sid is for an i/o virtual address
 */
#define	IOVADDR(sid)	( ((sid) & 0x00800000) ? 1 : 0 )

/* get sid and pno for i/o (pno	is unaffected) */
#define	IOSID(sid)	( (sid) | 0x00800000 )

/* check if segment is for i/o -- not going to the mmu */
#define	IOSEGCHK(val)	((val) & IOSEGMENT)

/* calculate the page number relative to scb corresponding to (sid,pno)
 */
#define	SCBPNO(sid, pno)	( (((sid) & 0x00700000) >> 4) + (pno) )

/* convert an sid to its base sid
 */
#define BASESID(sid)  ((sid) & 0xfffff)

/* convert a page relative to scb its pag relative to sid
 */
#define BASEPAGE(pno)  ((pno) & 0xffff)

/* get position	(zero-based) of	left-most zero bit in a	word */
#define	POSZBIT(bit,mask)    ( (bit) = clz32 (~(mask)) )

/* set a bit --	"bitpos" is zero-based and left-justified */
#define	SETBIT(mask,bitpos)    ( (mask)	| ( UZBIT >> (bitpos)) )

/* reset a bit -- "bitpos" is zero-based and left-justified */
#define	RESETBIT(mask,bitpos)	( (mask) & (~(UZBIT >> (bitpos))) )

/* Touch / Fault a page	into memory.  */
#define	TOUCH(p)    { char t = * (volatile char	*) (p);	}

/* determine if	APM "pmap" field is fully allocated  */
#define	FULLPMAP(blk_size,mask)	  \
		((mask)	== (SZBIT >> ((1 << (blk_size))	- 1) ))

/* extract pdt index from xpt entry for	paging space.	 */
#define	PDTIND(xpte)  (	((xpte)	& DMPDT) >> 24 )

/* extract  device block number	from xpt entry for paging space	*/
#define	PDTBLK(xpte) \
	  ( (xpte) & DMBLK )

/* compute xpt entry for working segment from pdtx and block  */
#define	XPTADDR(pdtx,bno) \
	  ( ((pdtx) << 24) | (bno) )

/* hash	function for repaging table.
 */
#define	RPHASH(tag,pno)	\
	( ((tag) ^ (pno)) & (vmker.rptsize - 1)	)

/* max and min of two integers */
#define	MIN(a,b) (((a)<(b))?(a):(b))
#define	MAX(a,b) (((a)>(b))?(a):(b))

/* convert size	in bytes to number of pages. */
#define	BTOPG(b)  (( (b) + PSIZE - 1) >> L2PSIZE)

/* convert  size in bytes to last page number in file. */
#define	BTOPN(b)  (((int)((b) -	1)) >> L2PSIZE)

/* size	in bytes of interval [0, end of	page P ] */
#define	PGTOB(P)  (( (P) + 1 ) << L2PSIZE)

/* macro for converting from mmap protection type to
 * corresponding page protection bits for key 1 srval
 */
#define VMPROT2PP(prot) \
	(((prot) & VM_PROT_WRITE) ? \
		 UDATAKEY : \
	 ((prot) & (VM_PROT_READ | VM_PROT_EXECUTE)) ? \
		 UTXTKEY  : \
		 UBLKKEY)

/*
 * This macro enforces the rule that no mapping for a page can
 * allow write access for a page which the VMM has read-only
 * (e.g. pages with no disk block).
 */
#define RDONLYPP(nfr,prot) \
	( (pft_key(nfr) == RDONLY && (prot) == UDATAKEY) ? RDONLY : (prot) )

/* macro for converting from mmap protection/mapping type to
 * flag to pass to VNOP_MAP, VNOP_UNMAP.
 */
#define VNOPFLAGS(entry) \
	(!((entry)->max_protection & VM_PROT_WRITE) || \
	 (entry)->needs_copy) ? SHM_RDONLY : 0;

/* determine if pno in sidx requires early paging space allocation */
#define PSEARLYALLOC(sidx,pno)          \
        (scb_psearlyalloc((sidx)) && (!scb_privseg((sidx)) ||       \
        ((pno) < BTOPG(U_REGION_SIZE))))

/*
 * Determine if a real page number corresponds to physical memory
 * (to exclude T=0 I/O mappings).
 */
#define PHYS_MEM(r)	((r) < vmker.nrpages)

#endif /* _h_VMMACS */
