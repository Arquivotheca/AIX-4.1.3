/* @(#)31	1.34  src/bos/kernel/vmm/POWER/vmpft.h, sysvmm, bos411, 9428A410j 7/21/94 16:23:12 */
#ifndef _h_VMPFT
#define _h_VMPFT

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

/*
 * definitions of the hardware page frame tables.
 * the tables are declared to handle the maximum real memory
 * (the array size is equal to the max number of page frames).
 * both tables reside in vmmdseg. in system initialization the
 * real storage underlying the hardware tables are allocated on
 * page boundaries consistent with alignment constraints.
 *
 */

#if defined(_POWER_RS1)	|| defined(_POWER_RSC)

/*
 * The hardware	page frame table structure for RS1, RSC.
 */
struct rs1pft
{
        union
        {
                int     _word0;
                struct
                {
			unsigned	_sidpno	:27;	/* segment id + pno */
			unsigned	_valid	:1;	/* entry is valid   */
			unsigned	_refbit	:1;	/* reference bit    */
			unsigned	_modbit	:1;	/* modified bit     */
			unsigned	_key	:2;	/* protection key   */
                } s1;
	} u1;
	int	_next;	/* next frame on hash chain */
	union
	{
                struct
                {
			ushort	_page;	/* full page number */
			ushort	_rsvd;	/* reserved	    */
			uint	_rsvd2;	/* reserved	    */
		} s1;
		struct
		{
			uint	_lock1;	/* h/w lock stuff -- not used */
			uint	_lock2;	/* h/w lock stuff -- not used */
                } s2;
	} u2;
};

/*
 * All the hardware dependent structures for RS1, RSC.
 */
struct rs1 {
	int		hat[2*MAXRPAGE];	/* hash	anchor table */
	struct rs1pft	pft[MAXRPAGE];		/* page	frame table */
};

/*
 * Macros for accessing	variables in the hardware pft for RS1, RSC
 * for PTE reload support (translate off).
 */
#define	rs1_hat(x)	(*((int *)vmrmap_raddr(RMAP_HAT) + x))
#define rs1_pft(x)	((struct rs1pft *)vmrmap_raddr(RMAP_PFT) + x)

#define	rs1_word0(x)	rs1_pft(x)->u1._word0
#define	rs1_sidpno(x)	rs1_pft(x)->u1.s1._sidpno
#define	rs1_sid(x)	(rs1_sidpno(x) >> 3)
#define	rs1_valid(x)	rs1_pft(x)->u1.s1._valid
#define	rs1_refbit(x)	rs1_pft(x)->u1.s1._refbit
#define	rs1_modbit(x)	rs1_pft(x)->u1.s1._modbit
#define	rs1_key(x)	rs1_pft(x)->u1.s1._key
#define	rs1_next(x)	rs1_pft(x)->_next
#define	rs1_page(x)	rs1_pft(x)->u2.s1._page

#endif	/* _POWER_RS1 || _POWER_RSC */

#ifdef	_POWER_RS2

/*
 * The hardware	page table entry structure for RS2.
 */
struct rs2pte
{
	union
	{
		int	 _word0;
		struct
		{
			unsigned	_valid	:1;	/* entry is valid    */
			unsigned	_sid	:24;	/* segment id	     */
			unsigned	_rsvd	:1;	/* reserved	     */
			unsigned	_hsel	:1;	/* hash selector     */
			unsigned	_avpi	:5;	/* abbrev page index */
		} s1;
        } u1;

        union
        {
		int	 _word1;
                struct
                {
			unsigned	_rpn	:20;	/* real page number */
			unsigned	_rsvd	:8;	/* reserved	    */
			unsigned	_refbit	:1;	/* reference bit    */
			unsigned	_modbit	:1;	/* modified bit     */
			unsigned	_key	:2;	/* protection key   */
                } s2;
         } u2;
};

/*
 * The "physical-to-virtual" table for RS2.
 */
struct rs2pvt {
	 union
	 {
		int	_word0;
         	struct
         	{
			unsigned	_swref	:1;	/* referenced bit */
			unsigned	_swmod	:1;	/* modified bit */
			unsigned	_rsvd	:8;	/* reserved */
			unsigned	_ptex	:22;	/* PTE index */
		} s1;
	} u1;
};

/*
 * All the hardware dependent structures for RS2.
 */
struct rs2 {
	struct rs2pte	pte[MAXPTES];
	struct rs2pvt	pvt[MAXRPAGE];
};

/*
 * Macros for accessing	variables in the hardware HTAB for RS2
 * for PTE reload support (translate off).
 */
#define rs2_pte(x)	((struct rs2pte *)vmrmap_raddr(RMAP_PFT) + x)

#define	rs2_word0(x)	rs2_pte(x)->u1._word0
#define	rs2_valid(x)	rs2_pte(x)->u1.s1._valid
#define	rs2_sid(x)	rs2_pte(x)->u1.s1._sid
#define	rs2_hsel(x)	rs2_pte(x)->u1.s1._hsel
#define	rs2_avpi(x)	rs2_pte(x)->u1.s1._avpi

#define	rs2_word1(x)	rs2_pte(x)->u2._word1
#define	rs2_rpn(x)	rs2_pte(x)->u2.s2._rpn
#define	rs2_refbit(x)	rs2_pte(x)->u2.s2._refbit
#define	rs2_modbit(x)	rs2_pte(x)->u2.s2._modbit
#define	rs2_key(x)	rs2_pte(x)->u2.s2._key

/*
 * Macros for accessing	the "physical-to-virtual" table	for RS2
 * for PTE reload support (translate off).
 */
#define rs2_pvt(x)	((struct rs2pvt *)vmrmap_raddr(RMAP_PVT) + x)

#define	rs2_swref(x)	rs2_pvt(x)->u1.s1._swref
#define	rs2_swmod(x)	rs2_pvt(x)->u1.s1._swmod
#define	rs2_ptex(x)	rs2_pvt(x)->u1.s1._ptex

#endif	/* _POWER_RS2 */

#ifdef	_POWER_PC

/*
 * The hardware	page table entry structure for 32-bit PowerPC.
 */
struct ppcpte
{
        union
        {
		int	 _word0;
                struct
                {
                        unsigned	_valid  :1;	/* entry is valid    */
			unsigned	_sid	:24;	/* segment id	     */
			unsigned	_hsel	:1;	/* hash selector     */
			unsigned	_avpi	:6;	/* abbrev page index */
                } s1;
        } u1;

        union
        {
		int	 _word1;
		struct
		{
			unsigned	_rpn	:20;	/* real page number */
			unsigned	_rsvd	:3;	/* reserved	    */
			unsigned	_refbit	:1;	/* reference bit    */
			unsigned	_modbit	:1;	/* modified bit     */
			unsigned	_wimg	:4;	/* storage control  */
			unsigned	_rsvd2	:1;	/* reserved	    */
			unsigned	_key	:2;	/* protection key   */
		} s2;
		struct
		{
			char		c1;
			char		c2; 
			char		c3; 		/* used to store byte */
			char		c4;
		} b2;
        } u2;
};

/*
 * The "physical-to-virtual" table anchor for PowerPC.
 */
struct ppcpvt {
        union
        {
		int	_word0;
                struct
                {
			unsigned	_swref	:1;	/* referenced bit */
			unsigned	_swmod	:1;	/* modified bit */
			unsigned	_rsvd	:8;	/* reserved */
			unsigned	_ptex	:22;	/* PTE index */
		} s1;
	} u1;
};

/*
 * A "physical-to-virtual" entry for PowerPC.
 */
struct ppcpvlist {
        union
        {
		int	_word0;
                struct
                {
			unsigned	_pno	:10; /* low 10 bits of pno */
			unsigned	_next	:22; /* index of next PTE */
		} s1;
	} u1;
};

/*
 * All the hardware dependent structures for PowerPC.
 */
struct ppc {
	struct ppcpte		pte[MAXPTES];
	struct ppcpvt		pvt[MAXRPAGE];
	struct ppcpvlist	pvlist[MAXPTES];
};

/*
 * Macros for accessing	variables in the hardware HTAB for PowerPC
 * for PTE reload support (translate off).
 */
#define ppc_pte(x)	((struct ppcpte *)vmrmap_raddr(RMAP_PFT) + x)

#define	ppc_word0(x)	ppc_pte(x)->u1._word0
#define	ppc_valid(x)	ppc_pte(x)->u1.s1._valid
#define	ppc_sid(x)	ppc_pte(x)->u1.s1._sid
#define	ppc_hsel(x)	ppc_pte(x)->u1.s1._hsel
#define	ppc_avpi(x)	ppc_pte(x)->u1.s1._avpi

#define	ppc_word1(x)	ppc_pte(x)->u2._word1
#define	ppc_rpn(x)	ppc_pte(x)->u2.s2._rpn
#define	ppc_refbit(x)	ppc_pte(x)->u2.s2._refbit
#define	ppc_modbit(x)	ppc_pte(x)->u2.s2._modbit
#define	ppc_wimg(x)	ppc_pte(x)->u2.s2._wimg
#define	ppc_key(x)	ppc_pte(x)->u2.s2._key

#define	ppc_refbyte(x)	ppc_pte(x)->u2.b2.c3
/*
 * Macros for accessing	the "physical-to-virtual" anchor for PowerPC
 * for PTE reload support (translate off).
 */
#define ppc_pvt(x)	((struct ppcpvt *)vmrmap_raddr(RMAP_PVT) + x)

#define	ppc_swref(x)	ppc_pvt(x)->u1.s1._swref
#define	ppc_swmod(x)	ppc_pvt(x)->u1.s1._swmod
#define	ppc_ptex(x)	ppc_pvt(x)->u1.s1._ptex

/*
 * Macros for accessing	a "physical-to-virtual" entry for PowerPC
 * for PTE reload support (translate off).
 */
#define ppc_pvlist(x)	((struct ppcpvlist *)vmrmap_raddr(RMAP_PVLIST) + x)

#define	ppc_pvnext(x)	ppc_pvlist(x)->u1.s1._next
#define	ppc_pvpno(x)	ppc_pvlist(x)->u1.s1._pno

#endif	/* _POWER_PC */

/*
 * The software	page frame table structure.
 */
struct pftsw
{
        union
        {
                uint _swbits;
                struct
                {
                        unsigned _inuse   :1;  /* in use (addressable) */
                        unsigned _pageout :1;  /* pageout state */
                        unsigned _pagein  :1;  /* page in state */
                        unsigned _free    :1;  /* free list     */
                        unsigned _slist   :1;  /* on scb list        */
                        unsigned _discard :1;  /* discard when i/o done */
                        unsigned _fblru   :1;  /* free list when i/o done */
                        unsigned _rdonly  :1;  /* sp key is read-only */
                        unsigned _journal :1;  /* page is in jseg */
                        unsigned _homeok  :1;  /* ok to write to home */
                        unsigned _syncpt  :1;  /* write to home        */
                        unsigned _newbit  :1;  /* disk block uncommitted */
                        unsigned _store   :1;  /* store operation on page */
                        unsigned _pgahead :1;  /* hidden - trigger pageahead */
			unsigned _cow     :1;	/* copy-on-write on store   */
			unsigned _mmap    :1;	/* mmap	fault /	client EOF  */
			unsigned _pfprot  :1;	/* client protection fault */
			unsigned _unused  :13;	/* not used		    */
			unsigned _key     :2;	/* source page protect key  */
                } s1;
        } u1;
	int	 _ssid;	 /* segment id of source object	*/
	union
	{
		uint	_pagex;	 /* page number in source object */
		struct
		{
			ushort	_spagehi;   	/* hi order bits pagex */
			ushort  _spage;		/* page num in source */
		} s1;
	} u2;
	union
	{
        	uint    _word3;	 /* 3rd word in pft */
		uint	_dblock; /* disk address info */
		struct
		{
			unsigned  _nfrags :  4;	/* # of	frags less than	page */
                        unsigned  _fraddr : 28; /* starting fragment number */
		} s1;
	} u3;
        int     _sidfwd;        /* next page on scb list */
        int     _sidbwd;        /* previous on scb list */
        union
        {
                struct
                {
                        int  _freefwd;  /* next on free list */
                        int  _freebwd;  /* previous on free list */
                } s1;

                struct
                {
			struct thread * _waitlist;/*  waitors on page frame */
                        int   _logage;		  /* log age of page */
                } s2;

                struct
                {
			struct thread * _waitlist;/*  waitors on page frame */
			union
			{
				struct
				{
					short	_pincnt_lt; /* Long Term  */
					short	_pincnt_st; /* Short Term */
				} s1;
				uint		_pincount;  /* Pins, page */
			} u7;
                } s3;
        } u4;
	union
	{
		uint	_word8;
		struct
		{
			unsigned _nonfifo: 16;	/* out of order	i/o	    */
			unsigned _devid  : 16;	/* index in PDT	table	*/
		} s1;
	} u5;
	union
	{
		uint	_word9;
		/*
		 * NOTE: Some of these fields are updated at interrupt level
		 * and so accesses to this word must be carefully serialized.
		 */
		struct
		{
			unsigned _nextio  :20;	/* next frame i/o list */
			unsigned _wimg	  : 4;	/* storage attributes */
			unsigned _rsvd	  : 2;	/* reserved */
			unsigned _ref	  : 1;	/* software reference bit */
			unsigned _mod	  : 1;	/* software modified bit */
			unsigned _xmemcnt : 4;	/* cross-memory	count */
		} s1;
	} u6;
	int	_next;		/* Next	entry on software hash chain */
	union
	{
		uint	_word11;
		struct
		{
			ushort	_alist;	/* List of alias table entries */
			ushort	_rsvd;	/* Reserved */
		} s1;
	} u7;
};

/*
 * Macros for accessing	variables in the software PFT.
 */
#define	pft_pftsw(x)	vmmdseg.pft[(x)]
#define	pft_swbits(x)	vmmdseg.pft[(x)].u1._swbits
#define	pft_inuse(x)	vmmdseg.pft[(x)].u1.s1._inuse
#define	pft_pageout(x)	vmmdseg.pft[(x)].u1.s1._pageout
#define	pft_pagein(x)	vmmdseg.pft[(x)].u1.s1._pagein
#define	pft_free(x)	vmmdseg.pft[(x)].u1.s1._free
#define	pft_slist(x)	vmmdseg.pft[(x)].u1.s1._slist
#define	pft_discard(x)	vmmdseg.pft[(x)].u1.s1._discard
#define	pft_fblru(x)	vmmdseg.pft[(x)].u1.s1._fblru
#define pft_rdonly(x)   vmmdseg.pft[(x)].u1.s1._rdonly
#define	pft_journal(x)	vmmdseg.pft[(x)].u1.s1._journal
#define	pft_homeok(x)	vmmdseg.pft[(x)].u1.s1._homeok
#define	pft_syncpt(x)	vmmdseg.pft[(x)].u1.s1._syncpt
#define	pft_newbit(x)	vmmdseg.pft[(x)].u1.s1._newbit
#define	pft_store(x)	vmmdseg.pft[(x)].u1.s1._store
#define	pft_pgahead(x)	vmmdseg.pft[(x)].u1.s1._pgahead
#define	pft_cow(x)	vmmdseg.pft[(x)].u1.s1._cow
#define	pft_mmap(x)	vmmdseg.pft[(x)].u1.s1._mmap
#define	pft_pfprot(x)	vmmdseg.pft[(x)].u1.s1._pfprot
#define	pft_key(x)	vmmdseg.pft[(x)].u1.s1._key
#define	pft_ssid(x)	vmmdseg.pft[(x)]._ssid
#define	pft_pagex(x)	vmmdseg.pft[(x)].u2._pagex
#define	pft_spage(x)	vmmdseg.pft[(x)].u2.s1._spage
#define	pft_spagehi(x)	vmmdseg.pft[(x)].u2.s1._spagehi
#define	pft_dblock(x)	vmmdseg.pft[(x)].u3._dblock
#define	pft_nfrags(x)	vmmdseg.pft[(x)].u3.s1._nfrags
#define	pft_fraddr(x)	vmmdseg.pft[(x)].u3.s1._fraddr
#define	pft_sidfwd(x)	vmmdseg.pft[(x)]._sidfwd
#define	pft_sidbwd(x)	vmmdseg.pft[(x)]._sidbwd
#define	pft_freefwd(x)	vmmdseg.pft[(x)].u4.s1._freefwd
#define	pft_freebwd(x)	vmmdseg.pft[(x)].u4.s1._freebwd
#define	pft_waitlist(x)	vmmdseg.pft[(x)].u4.s2._waitlist
#define	pft_logage(x)	vmmdseg.pft[(x)].u4.s2._logage
#define pft_pincntst(x) vmmdseg.pft[(x)].u4.s3.u7.s1._pincnt_st
#define pft_pincntlt(x) vmmdseg.pft[(x)].u4.s3.u7.s1._pincnt_lt
#define pft_pincount(x) vmmdseg.pft[(x)].u4.s3.u7._pincount
#define	pft_nonfifo(x)	vmmdseg.pft[(x)].u5.s1._nonfifo
#define	pft_devid(x)	vmmdseg.pft[(x)].u5.s1._devid
#define	pft_swbits2(x)	vmmdseg.pft[(x)].u6._word9
#define	pft_nextio(x)	vmmdseg.pft[(x)].u6.s1._nextio
#define	pft_wimg(x)	vmmdseg.pft[(x)].u6.s1._wimg
#define	pft_refbit(x)	vmmdseg.pft[(x)].u6.s1._ref
#define	pft_modbit(x)	vmmdseg.pft[(x)].u6.s1._mod
#define	pft_xmemcnt(x)	vmmdseg.pft[(x)].u6.s1._xmemcnt
#define	pft_next(x)	vmmdseg.pft[(x)]._next
#define	pft_alist(x)	vmmdseg.pft[(x)].u7.s1._alist

/*
 * Notes on page states:
 *
 * Bad		no bits set
 *		not on any list
 *		not h/w hashed
 * Free		pft_free = 1, pft_slist = 0
 *		on free list
 *		not h/w hashed
 * In-Use	pft_inuse = 1, pft_slist = 1
 *		on scb list
 *		may be h/w hashed
 * I/O		pft_pagein = 1 or pft_pageout = 1, pft_slist = 1
 *		on scb I/O list
 *		may be h/w hashed at I/O vaddr
 * PgAhead	pft_pgahead = 1, pft_slist = 1
 *		if I/O still going on then pft_pagein = 1, else pft_inuse = 1
 *		on scb list
 *		may be h/w hashed at I/O vaddr
 * Hidden	pft_slist = 1
 *		either:	pft_pincount > 0 and pft_inuse = 1, on scb list
 *		or: pft_pagein = 1 or pft_pageout = 1, on scb I/O list
 *		pft_xmemcnt is non-zero
 *		not h/w hashed
 */

/*
 * Repaging table entry.
 */
struct repage
{
        uint    _tag;    /* disk addr or scb serial number */
        ushort  _pno;    /* page number in segment */
        ushort  _next;   /* next on hash chain */
};

#define rpt_tag(x)   vmmdseg.rpt[(x)]._tag
#define rpt_pno(x)   vmmdseg.rpt[(x)]._pno
#define rpt_next(x)  vmmdseg.rpt[(x)]._next

/*
 * The alias page table structure.
 */
struct apt
{
	int	 _sid;	 /* segment id */
	union
	{
		uint	_word1;
		struct
		{
			ushort	_free; 	/* free list chain */
			ushort  _pno;	/* full page number */
		} s1;
	} u1;
	union
	{
		uint	_word2;
		struct
		{
			unsigned _valid   :1;	/* entry is valid */
			unsigned _pinned  :1;	/* entry is pinned */
			unsigned _key     :2;	/* page protection key  */
			unsigned _wimg    :4;	/* storage control bits */
			unsigned _rsvd    :4;	/* reserved */
			unsigned _nfr    :20;	/* page frame number */
		} s2;
	} u2;
	union
	{
		uint	_word3;
		struct
		{
			ushort	_next; 	/* next entry on alias hash chain */
			ushort  _anext;	/* next entry on page frame list */
		} s3;
	} u3;
};

/*
 * Macros for accessing	variables in the APT.
 */
#define	apt_sid(x)	vmmdseg.apt[(x)]._sid
#define	apt_free(x)	vmmdseg.apt[(x)].u1.s1._free
#define	apt_pno(x)	vmmdseg.apt[(x)].u1.s1._pno
#define	apt_valid(x)	vmmdseg.apt[(x)].u2.s2._valid
#define	apt_pinned(x)	vmmdseg.apt[(x)].u2.s2._pinned
#define	apt_key(x)	vmmdseg.apt[(x)].u2.s2._key
#define	apt_wimg(x)	vmmdseg.apt[(x)].u2.s2._wimg
#define	apt_nfr(x)	vmmdseg.apt[(x)].u2.s2._nfr
#define	apt_next(x)	vmmdseg.apt[(x)].u3.s3._next
#define	apt_anext(x)	vmmdseg.apt[(x)].u3.s3._anext

#endif	/* _h_VMPFT */
