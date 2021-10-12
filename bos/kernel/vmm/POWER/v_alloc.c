static char sccsid[] = "@(#)32	1.10.1.14  src/bos/kernel/vmm/POWER/v_alloc.c, sysvmm, bos412, 9445C412b 11/4/94 12:36:57";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:   v_pgalloc, chkpgdef, v_psalloc, v_dfree, v_pbitalloc,
 *              v_pbitfree, v_alloc, v_bitfree, allocag, alloccp,
 *              allocany, allocpg, v_updtree, treemax, pbitset,
 *              v_pgnext, v_extendag, v_nextag, v_realloc, v_allocnxt
 *              v_touchmap, v_psfalloc, fallocag, fallocpg, v_fallocnxt
 *
 *
 * ORIGINS: 27 83
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

#include "vmsys.h"
#include <sys/errno.h>
#include <sys/buf.h>
#include <sys/conf.h>
#include <sys/device.h>
#include <sys/syspest.h>
#include <sys/low.h>
#include <sys/mstsave.h>
#include <sys/inline.h>
#include <sys/trchkid.h>
#include "mplock.h"

/* dmaptab[] is declared here and initialized so that it is
 * in the fixed part of the kernel (not common). dmaptab[x]
 * encodes the longest string of zeros in x as a string of
 * 1's of length equal to the longest string followed by zeros.
 */
uchar dmaptab[256] = {0};

/*
 * v_pgalloc(pdtx,block)
 *
 * allocate one block from the paging space device selected
 * by routine v_pgnext.
 * pdtx is set to the PDT index of the device selected.
 * block is set to the block number in the device of the
 * allocated block.
 *
 * RETURN VALUE
 *	0	- ok
 *	ENOSPC	- disk full
 */

int
v_pgalloc(pdtx,block)
int	*pdtx;		/* set to index in PDT */
int	*block;		/* set to block number of block allocated */
{
	struct vmdmap *p0;
	int rc;
	
	/* call chkpgdef() to perform threshold managment if paging
	 * space is at or below the warning threshold.
	 */
	if (vmker.psfreeblks <= pf_nextwarn)
	{
		if (rc = chkpgdef())
			return(rc);
	}

	/*
	 * Routine v_pgnext() returns with the PG lock held
	 * for the selected device if successful.
	 */
	PDT_MPLOCK();
	if ((*pdtx = v_pgnext()) < 0)
	{
		PDT_MPUNLOCK();
		return(ENOSPC);
	}
	PDT_MPUNLOCK();

	/* get addressability to paging space map
	 */
	(void)chgsr(TEMPSR,vmker.dmapsrval);
	p0 = (struct vmdmap *) ((TEMPSR << L2SSIZE) + (*pdtx << L2DMSIZE));

	/* PG lock already acquired by v_pgnext() */

	rc = v_alloc(p0,pdt_fperpage(*pdtx),p0->lastalloc,block);
	
	PG_MPUNLOCK(*pdtx);

	return rc;
}

/*
 * chkpgdef()
 *
 * this routine enforces paging space thresholds and is 
 * called each time a paging space block is to be allocated
 * the free block count is less than or equal to pf_nextwarn.
 *
 * pgspwarn() is called to send SIGDANGER to all processes
 * when the warning paging space threshold is reached.
 *
 * each time a kill threshold is reached, pgspkill() is called
 * to select a process to be sent SIGKILL.  a new kill threshold
 * is determine each time an existing threshold is reached.
 * ENOSPC is returned if the process selected by pgspkill()
 * is the current process.
 * 
 * ENOSPC is returned by this routine if the free block count
 * is at or below the reserved block count (vmker.psrsvdblks).
 *
 * RETURN VALUE
 * 
 *	0	- ok
 *	ENOSPC	- a disk block should not be allocated.
 */

static int
chkpgdef()
{
	pid_t	pid;
	struct	mstsave *mst;
	int	sidx;
	uint	vaddr, srval, pno;
	struct mstsave * curcsa;
	struct proc *proc_p = curproc;
	
	/* check for warning threshold.
	 */
	if (vmker.psfreeblks == pf_nextwarn)
	{
		/* if warning threshold reached call pgspwarn() and 
		 * set nextwarn up by pf_adjwarn to insure pgspwarn()
		 * is not called again until free space has increased 
		 * to at least nextwarn + pf_adjwarn.
		 */
		if (pf_nextwarn == pf_npswarn)
		{
			pgspwarn();
			pf_nextwarn = pf_npswarn + pf_adjwarn;
		}
		else
		{
			pf_nextwarn = pf_npswarn;
		}
		return(0);
	}

	/* determine if free paging space is above the
	 * initial kill threshold + pf_adjkill.
	 */
	if (vmker.psfreeblks > pf_npskill + pf_adjkill)
		return(0);

	/* check if next kill threshold must be set or 
	 * adjusted.
	 */
	if (vmker.psfreeblks > pf_nextkill + pf_adjkill)
		pf_nextkill = vmker.psfreeblks - pf_adjkill;

	/* next kill threshold reached ? if so, call pgspkill()
	 * and set the next threshold.
	 */
	pid = 0;
	if (vmker.psfreeblks <= pf_nextkill)
	{
		pid = pgspkill();
		pf_nextkill = vmker.psfreeblks - pf_adjkill;
	}

	/* check for backtracking fault.
	 */
	curcsa = CSA;
	if (curcsa->backt != ORGPFAULT)
		return(0);

	mst = curcsa->prev;

	/* get fault location
	*/

	vaddr = mst->except[EVADDR];
	srval = mst->except[ESRVAL];
	sidx = STOI(SRTOSID(srval));
	pno = (vaddr & SOFFSET) >> L2PSIZE;

	/* determine if process signalled by pgspkill() is the
	 * current process or reserved block threshold has
	 * been reached.
	 */
	if ((pid > 0 && pid == proc_p->p_pid) ||
	     vmker.psfreeblks <= vmker.psrsvdblks && chkpskill(proc_p))
	{

		/* return ENOSPC if user mode.
		 */
		if (v_isuser(mst))
			return(ENOSPC);

		/* return ENOSPC if kernel mode, address not in system
		 * space, and exception handler defined.
		 */

		if (!v_issystem(sidx,pno) && (mst->kjmpbuf || mst->excbranch))
			return(ENOSPC);
	}
	else if (pid != 0)
	{
		/* some process has been killed.  if in user mode indicate
		 * no space. processes using early paging space allocation can
                 * always satisfy a request for an allocated paging space
                 * block.
                 */

		if (v_isuser(mst))
		{
                        if (proc_p->p_flag & SPSEARLYALLOC &&
                                        PSEARLYALLOC(sidx, pno))
                                return(0);
                        else
				return(ENOSPC);
		}
	}

	return(0);
}

/*
 * v_psalloc(pdtx,nfrags,hint,first)
 *
 * allocate nfrags consecutive fragment from the file system
 * device specified. the fragments will be allocated near the
 * fragment number hint if hint is specified as non-zero. on
 * successful return first is set to the fragment number of
 * the first fragment allocated. 
 *
 * RETURN VALUES
 *	0	- ok
 *	ENOSPC	- no space on device
 */

int
v_psalloc(pdtx,nfrags,hint,first)
int	pdtx;	/* index in PDT */
int	nfrags;	/* number of fragments to allocate */
int	hint;	/* fragment number to allocate nearby or 0 */
int	*first; /* set to fragment number of first fragment allocated */
{
	struct vmdmap *p0;

	FS_LOCKHOLDER(pdtx);

	/* get pointer to page 0 of map
	 */
	(void)chgsr(TEMPSR,pdt_dmsrval(pdtx));
	p0 = (struct vmdmap *) (TEMPSR << L2SSIZE);
	return(v_alloc(p0,nfrags,hint,first));
}

/*
 * v_dfree(pdtx,daddr)
 *
 * free the fragments described by the specified daddr from the
 * working map of the specified device.  this routine may be called
 * for paging space or file system devices.
 *
 * RETURN VALUE
 *	none
 */

v_dfree(pdtx,daddr)
int	pdtx;	/* index of device in PDT */
frag_t	daddr;	/* block on device */
{
	int p;
	struct vmdmap *p0;

	FS_LOCKHOLDER(pdtx);

	/* get pointer p0 to first page of map
	 */
	if(pdt_type(pdtx) == D_PAGING)
	{
		p = (TEMPSR << L2SSIZE) + (pdtx << L2DMSIZE);
		p0 = (struct vmdmap *) p;
		(void)chgsr(TEMPSR,vmker.dmapsrval);
	}
	else
	{
		p0 = (struct vmdmap *) (TEMPSR << L2SSIZE);
		(void)chgsr(TEMPSR,pdt_dmsrval(pdtx));
	}

	v_bitfree(p0,daddr.addr,pdt_fperpage(pdtx) - daddr.nfrags);

	return 0;
}

/*
 * v_pbitalloc(mapsid,ptr,logage)
 *
 * marks as allocated in the permanent part of the map specified 
 * by mapsid  the blocks in the vmdlist pointed to by ptr.
 *
 * only the blocks in the first vmdlist structure pointed to
 * by ptr are processed (i.e. dlist structures chained to the
 * first are ignored).
 *
 * this procedure runs at VMM interrupt level with back-track
 * enabled.
 *
 * RETURN VALUES
 *	0	- ok
 */

int
v_pbitalloc(mapsid,ptr,logage)
int mapsid;     	/* segment id of map */
struct vmdlist *ptr; 	/* pointer to list of blocks to free */
int logage;     	/* applicable logage */ 
{
	FS_LOCKHOLDER(scb_devid(STOI(mapsid)));

	return(pbitset(mapsid,ptr,logage,1));
}

/*
 * v_pbitfree(mapsid,ptr,logage,option)
 *
 * marks as free bits in a permanent map, work map, or both.
 * mapsid specifies the segment id of the map, and ptr points
 * to a vmdlist structure specifiying the bits. 
 *
 * the option parameter specifies in which map the bits are 
 * freed : V_PMAP permanent map, V_WMAP work map, V_PWMAP
 * both maps. if V_PWMAP is specified the nblocks field in
 * the vmdlist structure is set to zero on return.
 *
 * only the blocks in the first vmdlist structure pointed to
 * by ptr are processed (i.e. dlist structures chained to the
 * first are ignored).
 *
 * this procedure runs at VMM interrupt level with back-track
 * enabled.
 *
 * RETURN VALUES
 *	0	- ok
 */

int
v_pbitfree(mapsid,ptr,logage,option)
int mapsid;     	/* segment id of map */
struct vmdlist *ptr; 	/* pointer to list of blocks to free */
int logage;     	/* applicable logage */ 
int option;     	/* V_PWMAP to update work map too */
{
	int k, fragno, nfrags, fperpage;
	struct vmdmap *p0;

	FS_LOCKHOLDER(scb_devid(STOI(mapsid)));

	/* clear bits in the permanent map ?  this is safe in the
	 * face of page faults because the bits should be marked
	 * as allocated in the work map. 
	 */
	if (option == V_PMAP || option == V_PWMAP)
		pbitset(mapsid,ptr,logage,0);

	/* if option specifies work map too we have to be careful
	 * about page faults.  note that the loop starts with the
	 * last block and works its way down. after each sucessful
	 * v_bitfree() nblocks is decremented, so that we don't try
	 * to free it again.
	 */
	if (option == V_PWMAP || option == V_WMAP )
	{
		/* establish addressibility to the allocation map
		 * and get the number of fragments per page.
		 */
		(void)chgsr(TEMPSR,SRVAL(mapsid,0,0));
		p0 = (struct vmdmap *) (TEMPSR << L2SSIZE);
		fperpage = ptr->fperpage;
		assert(fperpage);

		for (k = ptr->nblocks - 1; k >= 0; k -= 1)
		{
			/* pickup starting fragment number and number
			 * of fragments.
			 */
			fragno = ptr->da.fptr[k].addr;
			nfrags = fperpage - ptr->da.fptr[k].nfrags;

			/* free the fragments in the working map.
			 */
			v_bitfree(p0,fragno,nfrags);
			ptr->nblocks -= 1;
		}
	
	}
	return 0;
}

/*
 * v_alloc(p0,nfrags,pref,result)
 *
 * allocates nfrags contiguous fragments in a map.
 *
 * this routine first tries to satisify the request with fragments
 * nearby the specified hint (pref):  starting with the map double
 * word containing the hint, the next four double words are search
 * for sufficient free fragments.  if the request cannot be satisifed
 * within these double words, allocation is attempted from the
 * allocation group containing the hint.  if this fails, we attempt
 * to allocation from an allocation group with at least average free
 * space.  if the request cannot be satisified from this allocation
 * group, we try to allocate from anywhere in the map.  we also
 * attempt to allocation from anywhere in the map if the hint is
 * specified as zero.
 *
 * when attempting allocation within allocation groups or map pages
 * this routine uses lower level allocators that attempt to place
 * the allocations near the front of allocation groups or map pages.
 *
 * input parameters
 *
 *	p0 	- pointer to page 0 of map.
 *	nfrags 	- number of fragments to allocate
 *	pref  	- try to allocate nearby by this fragment.
 *		  pref is assumed to be in the allocation
 *		  group in which allocation is preferred.
 *		  if pref = 0 allocate anywhere.
 *	result 	- set to fragment number of first fragment  
 *		  allocated.
 *
 * return values-
 *	0 	- ok
 *      ENOSPC 	- fail
 */

int
v_alloc(p0,nfrags,pref,result)
struct vmdmap * p0;
uint	nfrags;
uint 	pref;
uint	*result;
{
	uint rem, word, wperag, wnext;
	uint nag, k, p, lp, anag, ap, fragno;
	struct vmdmap *p1;
	uchar *cp, c;
	uint version, dbperpage;

	/* check that pref is in the map and nfrags ok.
	 */
	ASSERT(pref < p0->totalags * p0->agsize && nfrags <= p0->clsize)

	/* try to allocate anywhere if the preference is zero.
	 */
	if (pref == 0)
		return (allocany(p0,0,nfrags,result));

	/* calculate the page and the word within the page corresponding
	 * to the preference.
	 */
	version = p0->version;
	dbperpage = WBITSPERPAGE(version);
	p = pref/dbperpage;
	rem = pref - p*dbperpage;
	word = rem >> L2DBWORD;

	/* calculate the first word of the next allocation group.  allocation
	 * group sizes are such that each double word is fully contained
	 * within an allocation group.
	 */
	nag = rem / p0->agsize;
	wperag = p0->agsize >> L2DBWORD;
	wnext = (nag + 1) * wperag;

	/* calculate the pointer to character within the leafword that
	 * covers the double word containing the word.  each leafword
	 * covers 8 words of the map. p1 is pointer to vmdmap for page p.
	 */
	if (version == ALLOCMAPV3)
	{
		p1 = p0 + p;
	}
	else
	{
		p1 = p0 + 9*(p >> 3);
		p1 = VMDMAPPTR(p1,p); 
	}

	lp = LEAFIND + (word >> 3); 
	cp = (uchar *) &p1->tree[lp] + ((word & 0x7) >> 1);
	
	/* try to allocate from the next four double words.
	 */
	c = DBFREEMSK(version,nfrags);
	for (k = 0; k < 4 && word < wnext ; k++, cp++, word += 2)
	{
		if (*cp >= c)
			return(alloccp(p0,p,cp,nfrags,result));
	}

	/* try to allocate in the same allocation group.
	 */
	if (allocag(p0,p,nag,nfrags,result) == 0)
		return 0;

	/* try to allocate in an allocation group with at least average
	 * free space. v_nextag() returns the starting fragment number
	 * of the an allocation group containing average free. pref is
	 * provided as the hint so that v_nextag() starts the search with
	 * the allocation group following the one containing pref.
	 */
	fragno = v_nextag(p0,pref);

	/* calculate the page and the allocation group within the page
	 * corresponding to fragno.
	 */
	ap = fragno / dbperpage;
	rem = fragno - ap * dbperpage;
	anag = rem / p0->agsize;

	/* attempt to allocate from this allocation group only if it
	 * is not the same as the one containing pref.
	 */
	if (anag != nag || ap != p)
	{
		if (allocag(p0,ap,anag,nfrags,result) == 0)
			return 0;
	}

	/* try to allocate any where.
	 */
	return (allocany(p0,p,nfrags,result));
}

/*
 * v_bitfree(p0,fragno,nfrags)
 *
 * frees a sequence of fragments in a working map.
 *
 * input parameters
 *	p0 	- pointer to page 0 of map.
 *	fragno 	- fragment number of first fragment
 *      nfrags 	- number of fragments starting at fragno. 
 *
 * all of the fragments are assumed to be in the same double-word.
 *
 * return value - 0.
 */

int
v_bitfree(p0,fragno,nfrags)
struct vmdmap * p0;
uint  fragno;
uint nfrags;
{
	struct vmdmap *p1;
	uint rem, word, p, bitno, nag, bitmask, wmask;
	uint *wmap, *pmap, version, dbperpage;

	/* check that the fragments are in the map.
	 */
	assert(nfrags);
	assert(fragno + nfrags <= p0->mapsize);

	/* calculate map page, map bit within the page, and
	 * starting word of double word corresponding to
	 * fragno.
	 */
	version = p0->version;
	dbperpage = WBITSPERPAGE(version);
	p = fragno / dbperpage;
	bitno = fragno - p * dbperpage;
	word = (bitno >> L2DBWORD) & (ONES << 1);

	/* p1 is pointer to vmdmap and wmap, pmap to the bit maps
	 */
	if (version == ALLOCMAPV3)
	{
		p1 = p0 + p;
		wmap = (uint *)(p1) + LMAPCTL/4;
		pmap = wmap + WPERPAGE;
	}
	else
	{
		p1 = p0 + 9*(p >> 3);
		wmap = (uint *)(p1 + 1 + (p & 0x7));
		pmap = wmap + WPERPAGEV4;
		p1 = VMDMAPPTR(p1, p);
		TOUCH(p1);
	}

	/* free the bits in the working map.  bits should be allocated
	 * in the working map.  they will normally NOT be allocated in
	 * the permanent map (exception in commit for i/o error or a
	 * bug in the system).  if any are allocated we keep the map
	 * conservative by not freeing it in the work map.  for paging
	 * space maps, the bits will always be free in the permanent map.
	 */
	bitmask = ONES << (32 - nfrags);
	rem = bitno & (DBPERDWORD - 1);
	if (rem >= 32)
	{
		/* second word only 
		 */
		wmask = bitmask >> (rem - 32);
		assert((wmap[word + 1] & wmask) == wmask);
		if(pmap[word + 1] & wmask)
			return 0;
		wmap[word + 1] &= ~wmask;
	}
	else
	{
		/* update first word
		 */
		wmask = bitmask >> rem;
		assert((wmap[word] & wmask) == wmask);
		if(pmap[word] & wmask)
			return 0;
		wmap[word] &= ~wmask;

		if (rem + nfrags > 32)
		{
			/* update second word.
		 	 */
			wmask = bitmask << (32 - rem);
			assert((wmap[word + 1] & wmask) == wmask);
			if(pmap[word + 1] & wmask)
				return 0;
			wmap[word + 1] &= ~wmask;
		}
	}

	/* update stats and update tree.
	 */
	nag = bitno / p0->agsize;
	p1->agfree[nag] += nfrags;
	p0->freecnt += nfrags;
	v_updtree(p1, word);

	return 0;
}

/*
 * allocag(p0,p,nag,nfrags,result)
 *
 * allocate nfrags from allocation group specified.  search for
 * nfrags contiguous free fragments proceeds from the front of
 * the allocation group.
 *
 * input parameters
 *	p0 	- pointer to first page of map
 *	p  	- page number in map
 *      nag 	- allocation group number relative to p.
 *	nfrags	- number of fragment to allocate.
 *	result	- pointer to answer.
 *
 * returns 
 *	0 	- ok
 *	ENOSPC 	- full.
 */

static int
allocag(p0,p,nag,nfrags,result)
struct vmdmap *p0;
uint	p;
uint	nag;
uint	nfrags;
uint	*result;
{
	uint wperag, lp, c, word, index, wnext, word1, wnext1;
	uchar *cp, *cp1, *cp2;
	struct vmdmap *p1;
	uint version, dbperpage;

	/* use page level allocator if the allocation group covers
	 * the whole page.
	 */
	version = p0->version;
	dbperpage = WBITSPERPAGE(version);
	if (p0->agsize == dbperpage)
		return(allocpg(p0,p,nfrags,result));

	/* calculate the words within the page corresponding to the
	 * start of this allocation group and the start of the next
	 * allocation group.
	 */
	wperag = p0->agsize >> L2DBWORD;
	word = nag * wperag;
	wnext = (nag + 1) * wperag;

	/* calculate pointer to character in leaf-word which
	 * covers the first double word of the ag. p1 is pointer to
	 * vmdmap for page p.
	 */
	if (version == ALLOCMAPV3)
	{
		p1 = p0 + p;
	}
	else
	{
		p1 = p0 + 9*(p >> 3);
		p1 = VMDMAPPTR(p1, p);
	}
	lp = LEAFIND + (word >> 3); 
	cp = (uchar *) &p1->tree[lp] + ((word & 0x7) >> 1);

	/* examine the characters of the first leafword if the first
	 * character of the ag is not at the start of a leafword (i.e.
	 * the leafword also covers map words of the previous ag).
	 */
	c = DBFREEMSK(version,nfrags);
	for ( ; (uint) cp & 0x3 && word < wnext; cp += 1, word += 2)
	{
		if (*cp >= c)
			return(alloccp(p0,p,cp,nfrags,result));
	}

	/* character is on a leafword boundry.  the scan is now conducted
	 * at the next level of the tree.  at this level, each character
	 * covers a leafword which covers 8 words of the map.  also, a
	 * character may cover a leafword that includes map words of the 
	 * next allocation group.  however, the scan should only involve
	 * characters that are exclusive to the current allocation group.
	 *
 	 * calculate the pointer to the character within the higher level
	 * node word which covers the leafword.
	 */
	lp = LEAFIND + (word >> 3); 
	index = (lp - 1) & 0x3;
	lp = (lp - 1) >> 2;
	cp1 = (uchar *) &p1->tree[lp] + index;
	
	/* calculate the first map word covered by the leafword that
	 * covers the start of the next allocation group.
	 */
	wnext1 = wnext & ~0x7;

	for ( ; word < wnext1 ; cp1++, cp += 4, word += 8)
	{
		if (*cp1 < c)
			continue;

		for (word1 = word, cp2 = cp; word1 < wnext1; cp2++, word1 += 2)
		{
			if (*cp2 < c)
				continue;

			return(alloccp(p0,p,cp2,nfrags,result));
		}

		/* higher level indicated that sufficient free resources
		 * existed below.
		 */
		assert(0);
	}

	/* drop back to the leafword level if the last leafword could not
	 * be handled at the higher level (i.e. the leafword also covers
	 * map words of the next ag).
	 */
	for ( ; word < wnext; cp += 1, word += 2)
	{
		if (*cp >= c)
			return(alloccp(p0,p,cp,nfrags,result));
	}

	return (ENOSPC);

}

/*
 * alloccp(p0,p,cp,nfrags,result)
 *
 * allocate nfrags from one of the 2 bitmap words covered by
 * character in leafword specified by cp.
 *
 * input parameters
 *	p0	- pointer to page 0 of map
 *	p	- page number of map
 *	cp	- pointer to a character of a leaf-index.
 *		  covering a double-word with enuff space.
 *	nfrags	- number of fragments to allocate.
 *	result	- pointer to answer
 *
 * return values 
 *	0 	- ok
 *	ENOSPC 	- no space.
 */

static int
alloccp(p0, p, cp, nfrags, result)
struct vmdmap *p0;
uint	p;
uchar   *cp;
uint    nfrags;
uint	*result;
{
	struct vmdmap *p1;
	uint word, bitmask, bitpos;
	uint bitno, nag, fragno, rem, wmask;
	uint version, dbperpage, *wmap;

	assert(nfrags <= CLDEFAULTV4);

	/* calculate the first word in the map which corresponds
	 * to cp. each character in the leafword covers 2 map words.
	 */
	version = p0->version;
	if (version == ALLOCMAPV3)
	{
		dbperpage = DBPERPAGE;
		p1 = p0 + p;
		wmap = (uint *)(p1) + LMAPCTL/4;
	}
	else
	{
		dbperpage = DBPERPAGEV4;
		p1 = p0 + 9*(p >> 3);
		wmap = (uint *)(p1 + 1 + (p & 0x7));
		p1 = VMDMAPPTR(p1, p);
		TOUCH(p1);
	}
	
	word = ((uint) cp - (uint) &p1->tree[LEAFIND]) << 1;

	/* determine the position in double word at which to allocate.
	 * v_findstring() returns the leftmost bit position in the passed
	 * double word where there are a string of at least nfrags free
	 * map bits.
	 */
	bitpos = v_findstring(wmap + word, nfrags);
	assert(bitpos < DBPERDWORD);

	/* calculate the bit number within the map page and the fragment
	 * number within the map of the starting fragment.
	 */
	bitno = (word << L2DBWORD) + bitpos;
	fragno = bitno + p * dbperpage;

	/* make sure fragments are less than current mapsize. because of
	 * v_extendag(), it is not an error to find a free fragment with
	 * a big fragment number.  however we can't allocate it.
	 */
	if (fragno + nfrags > p0->mapsize)
		return(ENOSPC);

	/* set the results and update statistics in page 0 and page p.
	 */
	*result = fragno;
	p0->freecnt -= nfrags;
	p0->lastalloc = fragno;
	nag = bitno/p0->agsize;
	p1->agfree[nag] -= nfrags;

	/* set bits in bit map.  nfrags is assumed to be <= 32.
	 */
	bitmask = ONES << (32 - nfrags);
	rem = bitno & (DBPERDWORD - 1);
	if (rem >= 32)
	{
		/* second word only.
		 */
		wmask = bitmask >> (rem - 32);
		assert((wmap[word + 1] & wmask) == 0)
		wmap[word + 1] |= wmask;
	}
	else
	{
		/* update first word
		 */
		wmask = bitmask >> rem;
		assert((wmap[word] & wmask) == 0)
		wmap[word] |= wmask;
		if (rem + nfrags > 32)
		{
			/* update second word.
			 */
			wmask = bitmask << (32 - rem);
			assert((wmap[word + 1] & wmask) == 0)
			wmap[word + 1] |= wmask;
		}
	}

	/* update tree.
	 */
	v_updtree(p1, word);

	return 0;

}

/*
 * allocany(p0,p,nfrags,result)
 * 
 * allocate nfrags from any allocation group that has space.
 * search starts in page p.
 *
 * input parameters
 *	p0 	- pointer to page 0 of map
 *	p  	- page number of map to start search.
 *	nfrags 	- number of contiguous fragments to allocate
 *	result 	- pointer to answer.
 *
 * return values 
 *	0 	- ok
 *	ENOSPC 	- no space.
 */

static int
allocany(p0, p, nfrags, result)
struct vmdmap *p0;
uint p;
uint nfrags;
uint *result;
{
	uint npages, k, page;
	uint dbperpage;

	/* get number of pages in map. check p.
	 */
	dbperpage = WBITSPERPAGE(p0->version);
	npages = (p0->mapsize + dbperpage - 1)/dbperpage;
	ASSERT(p < npages);

	/* try each page starting with p
	 */
	for (k = p; k < npages + p; k++)
	{
		page = (k < npages) ? k : k - npages;
		if (allocpg(p0, page, nfrags, result) == 0)
			return 0;

	}
	return ENOSPC;

}

/* 
 * allocpg( p0, p, nfrags, result)
 *
 * allocate nfrags from any place in a page.  search for nfrags
 * contiguous free fragments proceeds from the front of the page.
 *
 * input paramters
 *	p0 	- pointer to page 0 of map
 *	p  	- page number of map to search.
 *	nfrags 	- number of contiguous fragments to allocate
 *	result 	- pointer to answer.
 *
 * return values
 *	0 	- ok
 *	ENOSPC 	- no space
 */

static int
allocpg(p0, p, nfrags, result)
struct vmdmap *p0;
uint p;
uint nfrags;
uint *result;
{
	struct vmdmap *p1;
	uint k, n, lp, c, version;
	uchar *cp;

	version = p0->version;
	if (version == ALLOCMAPV3)
	{
		p1 = p0 + p;
	}
	else
	{
		p1 = p0 + 9*(p >> 3);
		p1 = VMDMAPPTR(p1, p);
	}

	/* starting at the root word, descend the search tree choosing
	 * leftmost characters in words within the tree that satisfy
	 * the request.
	 */
	lp = 0;
	c = DBFREEMSK(version,nfrags);
	for (k = 0; k < 4; k++)
	{
		/* examine the characters within the tree word.
		 */
		cp = (uchar *) &p1->tree[lp];
		for (n = 0; n < 4; n++)
		{
			if (*(cp + n) >= c)
				goto nextlevel;
		}
		assert(lp == 0);
		return ENOSPC;

		nextlevel:
		lp = 4 * lp + 1 + n;
	}

	/* at this point, cp points to a leafword and n is the
	 * first character in it covering a double word in the
	 * bitmap with sufficient resource.
	 */
	return(alloccp(p0,p,cp + n,nfrags,result));
}

/*
 * v_updtree(p1,ind)
 *
 * update vmdmap tree.
 *
 * input parameters:
 *	p1 	- pointer to page of map or control vmdmap.
 *	ind 	- index of the word which changed.
 *	          (may be either even or odd word of pair).	
 */

v_updtree(p1,ind)
struct vmdmap *p1;
int ind;
{
	uint word, n, lp, k, index, maxc, max;
	uchar *cp0, *cp1;
	uint *wmap , pno;
	struct vmdmap * p2;

	/* calculate the index of the first word of the double
	 * word.
	 */
	word = ind & (ONES << 1);
	
	/* determine the maximum number of contiguous free bits.
	 * for version 0 maps, the maximum is the longest string
	 * of free bits found within a character of the double word
	 * and is not to exceed the cluster size (p1->clsize).  for
	 * other versions, the maximum is the longest string of free
	 * bits found within the double word.
	 */
	if (p1->version == ALLOCMAPV3)
	{
		max = 0;
		maxc = p1->clmask;
		cp0 = (uchar *) (p1) + LMAPCTL + 4*word;
		for (n = 0; n < 8; n++, cp0++)
		{
			max = MAX(max,dmaptab[*cp0]);
			if (max >= maxc)
			{
				max = maxc;
				break;
			}
		}
	}
	else
	{
		/* p2->begin of page containing p1. wmap -> bitmap array  */
		p2 = (struct vmdmap *) ((uint)p1 & ~POFFSET); 
		pno = (((uint) p1 & POFFSET) >> 9);
		wmap = (uint *) (p2 + pno + 1);
		max = v_maxstring(wmap + word);
	}

	/* calculate pointers to leafword and to the character in it
	 * that corresponds to the double word.  each leafword cover 8
	 * words of map, so the low order 3-bits of word are shifted
	 * right by one in computing the character pointer (cp1).
	 */
	lp = LEAFIND + (word >> 3);
	cp0 = (uchar *) &p1->tree[lp];
	cp1 = cp0 + ((word & 0x7) >> 1);

	/* there are at most four levels of the tree to process.
	 */
	for (k = 0; k < 4; k++)
	{
		/* if old max is same as max, nothing to do.
	 	 */
		if (*cp1 == max)
			return 0;

	 	/* set the new maximum in the character.  once set,
		 * get the maximum for the tree word containing the
		 * character.
	 	 */
		*cp1 = max;
		max = treemax(cp0);

		/* get parent of lp. parent covers four words of the
		 * tree so division is by 4. calculate pointers to the
		 * tree word and character.
		 */
		index = (lp - 1) & 0x3;
		lp = (lp - 1) >> 2;
		cp0 = (uchar *) &p1->tree[lp];
		cp1 = cp0 + index;
	}

	return 0;
}

/*
 * treemax(cp0)
 *
 * returns the maximum sequence in a tree word.
 * cp0 points to first character of the word.
 */

static int
treemax(cp0)
uchar * cp0;
{
	uint max1, max2;
	max1 = MAX(*cp0, *(cp0+1));
	max2 = MAX(*(cp0+2), *(cp0+3));
	return (MAX(max1,max2));
}

/*
 * pbitset(mapsid,ptr,logage,bitval)
 *
 * set bits in the permanent part of a map to the value
 * specified by bitval.
 *
 *
 * INPUT PARAMETERS:
 *
 *  (1) mapsid sid of map
 *
 *  (2) ptr - ptr to vmdlist structure.
 *
 *  (3) logage - logage for updating age of pages of map affected
 *
 *  (4) bitval - 1 to mark as allocated, 0 to mark as free.
 *
 * Return value - 0
 */
	
static int
pbitset(mapsid,ptr,logage,bitval)
int mapsid;
struct vmdlist *ptr;
int logage;
int bitval;
{
	
	uint p, word, fragno, rem, nfr, bitno, bitmask, wmask;
	int mapsidx, diff, diff1, lsidx, k, nfrags, fperpage;
	struct vmdmap *p0, *p1;
	uint version, *wmap, *pmap, *lastp, dbperpage;

	FS_LOCKHOLDER(scb_devid(STOI(mapsid)));

	/* get addressability to map
	 */
	(void)chgsr(TEMPSR,SRVAL(mapsid,0,0));
	p0 = (struct vmdmap *) (TEMPSR << L2SSIZE);
	version = p0->version;
	dbperpage = WBITSPERPAGE(version);

	/* determine difference in logage from sync point.  diff is
	 * the number of bytes in log since sync point.
	 */
	LW_MPLOCK_S();

	mapsidx = STOI(mapsid);
	lsidx = scb_logsidx(mapsidx);
	logdiff(diff,logage,lsidx);

	/* this loop is run "backwards" (same as for freeing from
	 * the working map).
	 */
	lastp = NULL;
	fperpage = ptr->fperpage;
	assert(fperpage);
	for (k = ptr->nblocks - 1; k >= 0; k--)
	{
		/* get starting fragment number and the number of
		 * fragments for this vmdlist entry.
		 */
		fragno = ptr->da.fptr[k].addr;
		nfrags = fperpage - ptr->da.fptr[k].nfrags;
		assert(p0->mapsize >= fragno + nfrags);

		/* calculate map page, map bit within the page, and
	 	 * starting word of double word corresponding to
		 * fragno.
	 	 */
		p = fragno / dbperpage;
		bitno = fragno - p * dbperpage;
		word = (bitno >> L2DBWORD) & (ONES << 1);

		if (version == ALLOCMAPV3)
		{
			p1 = p0 + p;  /* pointer to page */
			wmap = (uint *) ((uint *)p1 + LMAPCTL/4);
			pmap = wmap + WPERPAGE;
		}
		else
		{
			p1 = p0 + 9*(p >> 3);
			wmap = (uint *)(p1 + 1 + (p & 0x7));
			pmap = wmap + WPERPAGEV4;
		}

		/* set the bits according to bitval.  the bits
		 * should be allocated in the working map.
		 */
		bitmask = ONES << (32 - nfrags);
		rem = bitno & (DBPERDWORD - 1);
		if (rem >= 32)
		{
			/* second word only.
		 	 */
			wmask = bitmask >> (rem - 32);
			assert((wmap[word + 1] & wmask) == wmask);
			if (bitval == 0)
				pmap[word + 1] &= ~wmask;
			else
				pmap[word + 1] |=  wmask;

		}
		else
		{
			/* update first word.
			 */
			wmask = bitmask >> rem;
			assert((wmap[word] & wmask) == wmask);
			if (bitval == 0)
				pmap[word] &= ~wmask;
			else
				pmap[word] |=  wmask;

			if (rem + nfrags > 32)
			{
				/* update second word.
			 	 */
				wmask = bitmask << (32 - rem);
				assert((wmap[word + 1] & wmask) == wmask);
				if (bitval == 0)
					pmap[word + 1] &= ~wmask;
				else
					pmap[word + 1] |=  wmask;
			}
		}

		/* update logage in page containing wmap. it is in memory so
		 * the v_lookup() will work.  diff is the number of bytes past
		 * sync point.
		 */
		if (wmap != lastp)
		{
			lastp = wmap;
			nfr = v_lookup(mapsid,((uint)wmap & SOFFSET)>>L2PSIZE);
			ASSERT(nfr >= 0);
			if (pft_logage(nfr) != 0)
			{
				logdiff(diff1,pft_logage(nfr),lsidx);
				if (diff < diff1)
					pft_logage(nfr) = logage;
			}
			else
				pft_logage(nfr) = logage;
		}
	}

	LW_MPUNLOCK_S();

	return 0;
}

/*
 * v_pgnext()
 *
 * return the PDT index of the next paging space device with
 * space on it.
 *
 * uses TEMPSR sregister.
 *
 * RETURN VALUES
 *
 *	pdtx	- index in device table.
 *
 *	-1	- no device has space
 *
 * returns with PG lock held for pdtx
 */

v_pgnext()
{
	int pdtx,k;
	struct vmdmap * p0;

	PDT_LOCKHOLDER();

	/* get addressability to paging space maps
	 */
	(void)chgsr(TEMPSR,vmker.dmapsrval);
	p0 = (struct vmdmap *) (TEMPSR << L2SSIZE);

	/*
	 * If less than pf_maxpdtblks blocks have been allocated
	 * from the current paging device then keep allocating
	 * from this same device if space remains.
	 *
	 * The test here for free paging space and the
	 * allocation by the caller must be atomic.
	 * We acquire the PG lock here and return with
	 * it held so the caller must release it after
	 * performing the allocation.
	 */
	if (pf_npdtblks < pf_maxpdtblks)
	{
		PG_MPLOCK(pf_pdtlast);
	    	if ((p0 + (pf_pdtlast << L2DMPAGES))->freecnt > 0)
		{
			pf_npdtblks += 1;
			return(pf_pdtlast);
		}
		PG_MPUNLOCK(pf_pdtlast);
	}
	pf_npdtblks = 1;

	/* try next pdt
	 */
	pdtx = (pf_pdtlast < pf_pdtmaxpg) ? pf_pdtlast + 1 : 0;
	if (pdt_type(pdtx) && pdt_avail(pdtx))
	{
		PG_MPLOCK(pdtx);
		if ((p0 + (pdtx << L2DMPAGES))->freecnt > 0)
		{
			pf_pdtlast = pdtx;
			return(pdtx);
		}
		PG_MPUNLOCK(pdtx);
	}

	/* loop thru list of pdts.
	 */
	for (k = pdtx; k <= pf_pdtmaxpg; k++)
	{
		if (pdt_type(k) && pdt_avail(k))
		{
			PG_MPLOCK(k);
			if ((p0 + (k << L2DMPAGES))->freecnt > 0)
			{
				pf_pdtlast = k;
				return(k);
			}
			PG_MPUNLOCK(k);
		}
	}

	for (k = 0; k < pdtx; k ++)
	{
		if (pdt_type(k) && pdt_avail(k))
		{
			PG_MPLOCK(k);
			if ((p0 + (k << L2DMPAGES))->freecnt > 0)
			{
				pf_pdtlast = k;
				return(k);
			}
			PG_MPUNLOCK(k);
		}
	}

	return(-1);
}

/*
 * v_extendag(p0,nfrags,fragno,pdtx)
 *
 * adds new fragments to an allocation group.  all of the
 * the fragments are asummed to be in same allocation group.
 * this is only called by vm_growmap().
 *
 * this executes at VMM interrupt level with back-tracking enabled.
 *
 * input parameters
 *	p0 	- pointer to page 0 of map.
 *	fragno 	- fragment number of first.
 *	nfrags  - number of fragments to free.
 *	pdtx    - pdt index
 *
 * return value
 *	0
 */

int
v_extendag(p0, nfrags, fragno, pdtx)
struct vmdmap *p0;
uint  nfrags;
uint  fragno;
int pdtx;
{
	struct vmdmap *p1;
	uint k, rem, word, p, firstw, lastw;
	uint nag, bitmask, nf;
	uint version, dbperpage, *wmap, *pmap;

	FS_LOCKHOLDER(pdtx);

	/* get version and dbperpage */
	version = p0->version;
	dbperpage = WBITSPERPAGE(version);

	/* get pointer to word in bitmap corresponding 
	 * to fragno. 
	 */
	p = fragno / dbperpage;
	rem = fragno - p * dbperpage;
	nag = rem / p0->agsize;
	firstw = word = rem >> L2DBWORD;
	bitmask = UZBIT >> (rem - (word << L2DBWORD));
	lastw = (rem + nfrags - 1) >> L2DBWORD;

	if (version == ALLOCMAPV3)
	{
		p1 = p0 + p;  /* pointer to page */
		wmap = (uint *)(p1) + LMAPCTL/4;
		pmap = wmap + WPERPAGE;
	}
	else
	{
		p1 = p0 + 9*(p >> 3);
		wmap = (uint *)(p1 + 1 + (p & 0x7));
		pmap = wmap + WPERPAGEV4;
		p1 = VMDMAPPTR(p1, p);
		TOUCH(p1);
	}

	/* free the bits in both work and perm maps.
	 */
	nf = nfrags;
	while (nf > 0)
	{
		nf = nf - 1;
		wmap[word] &= ~bitmask;
		pmap[word] &= ~bitmask;
		bitmask = bitmask >> 1;
		if (bitmask == 0)
		{
			word += 1;
			bitmask = UZBIT;
		}
	}

	/* update stats. allocation group can be new.
	 */
	p1->agfree[nag] += nfrags;
	p0->freecnt += nfrags;
	if (nag >= p1->agcnt)
	{
		p1->agcnt += 1;
		p0->totalags += 1;
	}

	/* update tree. start loop on even word of pair so that
	 * last word is always covered with loop increment of 2.
	 */
	for (k = firstw & (ONES << 1); k <= lastw; k += 2) 
		v_updtree(p1, k);

	return 0;
}

/*
 * v_nextag(p0,fragno)
 *
 * find the next allocation group which has at least average
 * free space. 
 *
 * this executes at VMM interrupt level with back-tracking enabled.
 *
 * input parameters
 *	p0 	- pointer to page 0 of map.
 *	fragno 	- fragment number in current allocation group.
 *
 * return value
 *	fragment number in next ag which has at least average free
 *	space.
 */

int
v_nextag(p0, fragno)
struct vmdmap *p0;
uint fragno;
{
	struct vmdmap *p1;
	uint k, avgfree, p, ag, nag, agsize, agperpage, totalags;
	uint version, dbperpage;

	/* calculate average free space.
	 */
	totalags = p0->totalags;
	avgfree = p0->freecnt / totalags;

	/* look at each allocation group starting with next one.
	 */
	agsize = p0->agsize;
	ag = fragno / agsize;
	version = p0->version;
	dbperpage = WBITSPERPAGE(version);
	agperpage = dbperpage / agsize;

	for (k = 0; k < totalags; k++)
	{
		ag = ag + 1;
		if (ag >= totalags)
			ag = 0;

		/* calculate page number and allocation group number
		 * relative to page for ag.
		 */
		p = ag / agperpage;
		nag = ag - p * agperpage;
		if (version == ALLOCMAPV3)
		{
			p1 = p0 + p;  /* pointer to page */
		}
		else
		{
			p1 = p0 + 9*(p >> 3);
			p1 = VMDMAPPTR(p1, p);
		}

		/* if enough free space return first fragment number of
		 * the ag.
		 */
		if (p1->agfree[nag] >= avgfree)
			return (ag * agsize);
	}

	return(fragno);
}

/*
 * v_realloc(pdtx,nb,old,result,newfrags) 
 *
 * free the fragments in the array old[]. allocate newfrags
 * fragments and set result to the number of the first allocated
 * fragment.
 * 
 * old[nb-1] is used as a hint as to where the new blocks are
 * to be allocated. 
 *
 * nb is assumed to be at least one and no more than FSCLSIZE.
 *
 * RETURN VALUE
 *	0	- ok
 *	ENOSPC	- disk full
 */

int
v_realloc(pdtx,nb,old,result,newfrags)
int	pdtx;		/* index in PDT */
int	nb; 		/* size of array old[] */
frag_t	*old;  		/* pointer to array of blocks to be freed */
uint	*result;	/* set to frag number of first frag allocated */
int     newfrags;  	/* number of fragments to allocate */  
{
	int rc, k, fperpage, nfrags;
	uint rem, w, agsize, nag, p, bitmask, wmask, bitno;
	uint mappage[FSCLSIZE], word[FSCLSIZE];
	struct vmdmap *p0, *p1;
	uint version, *wmap, dbperpage;

	FS_LOCKHOLDER(pdtx);

	assert(nb <= FSCLSIZE);
	assert(newfrags <= CLDEFAULTV4);

	/* get pointer to page 0 of map
	 */
	(void)chgsr(TEMPSR,pdt_dmsrval(pdtx));
	p0 = (struct vmdmap *) (TEMPSR << L2SSIZE);
	version = p0->version;
	dbperpage = WBITSPERPAGE(version);

	/* calculate the page numbers of the map that record the
	 * old fragments.  touch each page so that we can later
	 * free these fragments without faulting.
	 */
	fperpage = pdt_fperpage(pdtx);
	for (k = 0; k < nb; k++)
	{
		mappage[k] = old[k].addr / dbperpage;
		if (version == ALLOCMAPV3)
		{
			p1 = p0 + mappage[k];  /* pointer to page */
		}
		else
		{
			p1 = p0 + 9*(mappage[k] >> 3);
			wmap = (uint *)(p1 + 1 + (mappage[k] & 0x7));
			TOUCH(wmap);
		}
		TOUCH(p1);
	}

	/* try to allocate newfrags.
	 */
	if (rc = v_alloc(p0,newfrags,old[nb-1].addr, result))
		return rc;

	/* free the old fragments.
	 */
	agsize = p0->agsize;
	for (k = 0; k < nb; k++)
	{
		/* calculate map bit within the page and starting
		 * word of double word corresponding to the first
		 * fragment of the block.
		 */
		p = mappage[k];
		bitno = old[k].addr - p*dbperpage;
		word[k] = (bitno >> L2DBWORD) & (ONES << 1);

		/* determine the number of fragments to free.
		 */
		nfrags = fperpage - old[k].nfrags;

		/* free the bits in the map.  the bits should be allocated
		 * in the working map.
		 */
		bitmask = ONES << (32 - nfrags);
		rem = old[k].addr & (DBPERDWORD - 1);

		if (version == ALLOCMAPV3)
		{
			p1 = p0 + p;  /* pointer to page */
			wmap = (uint *) p1 + LMAPCTL/4;
		}
		else
		{
			p1 = p0 + 9*(p >> 3);
			wmap = (uint *)(p1 + 1 + (p & 0x7));
			p1 = VMDMAPPTR(p1, p);
		}

		if (rem >= 32)
		{
			/* second word only.
			 */
			wmask = bitmask >> (rem - 32);
			assert((wmap[word[k] + 1] & wmask) == wmask);
			wmap[word[k] + 1] &= ~wmask;
		}
		else
		{
			/* update first word.
			 */
			wmask = bitmask >> rem;
			assert((wmap[word[k]] & wmask) == wmask);
			wmap[word[k]] &= ~wmask;

			if (rem + nfrags > 32)
			{
				/* update second word.
				 */
				wmask = bitmask << (32 - rem);
				assert((wmap[word[k]+1] & wmask) == wmask);
				wmap[word[k] + 1] &= ~wmask;
			}
		}

		/* update stats.
		 */
		nag = bitno / agsize;
		p1->agfree[nag] += nfrags;
		p0->freecnt += nfrags;
	}

	/* update the tree.
	 */
	p = w = -1;
	for (k = 0; k < nb; k++)
	{
		/* only perform the update if word or map page
		 * is different from last time thru.
		 */
		if (w != word[k] || p != mappage[k])
		{
			w = word[k];
			p = mappage[k];
			if (version == ALLOCMAPV3)
			{
				p1 = p0 + p;  /* pointer to page */
			}
			else
			{
				p1 = p0 + 9*(p >> 3);
				p1 = VMDMAPPTR(p1, p);
			}
			v_updtree(p1, w);
		}
	}

	return 0;
}

/*
 * v_allocnxt(pdtx,prev, nfrags)
 *
 * attempt to allocate nfrag fragments immediately following the
 * the fragments specified by prev.  the requested fragments must be
 * free and reside within the same allocation group as prev.
 * 
 * RETURN VALUE
 *	0	- ok
 *	ENOSPC	- next is allocated or in different allocation group.
 */

int
v_allocnxt(pdtx, prev,nfrags)
int	pdtx;	/* index in PDT */
frag_t	prev;   /* fragment pointer of previous */
int 	nfrags; /* number of fragments needed */
{
	int fperpage;
	uint rem, word, p, nag, nextfrag, bitmask, wperag;
	uint fragno, prevfrags, bitno, wmask1, wmask2;
	struct vmdmap *p0,*p1;
	uint version, dbperpage, *wmap;

	FS_LOCKHOLDER(pdtx);

	/* get pointer to page 0 of map
	 */
	(void)chgsr(TEMPSR,pdt_dmsrval(pdtx));
	p0 = (struct vmdmap *) (TEMPSR << L2SSIZE);
	version = p0->version;
	dbperpage = WBITSPERPAGE(version);

	/* calculate map page, map bit within the page, and
	 * starting word of double word corresponding to the
	 * first fragment of prev.
	 */
	fragno = prev.addr;
	p = fragno / dbperpage;
	bitno = fragno - p * dbperpage;
	word = (bitno >> L2DBWORD) & (ONES << 1);

	/* determine the number of fragments covered by prev.
	 */
	fperpage = pdt_fperpage(pdtx);
	prevfrags = fperpage - prev.nfrags;

	/* determine the fragment numbers within the double word
	 * of prev's starting fragment and the fragment immediately
	 * following prev's last fragment.  the latter will be the
	 * starting fragment of the new allocation request.
	 */
	rem = fragno & (DBPERDWORD - 1);
	nextfrag = (rem + prevfrags  == DBPERDWORD) ? 0 : rem + prevfrags;

	/* determine if the new allocation request is wholly contained
	 * within a double word of the map.
	 */
	if (nextfrag + nfrags > DBPERDWORD) 
		return ENOSPC;

	/* determine if the new allocation request starts on the
	 * next double word.  if so, move to the first word of
	 * the next double word and check if this word is in the
	 * same allocation group as prev.
	 */
	nag = bitno / p0->agsize;
	if (nextfrag == 0)
	{
		word += 2;
		wperag = p0->agsize >> L2DBWORD;
		if (word >= (nag + 1) * wperag)
			return ENOSPC;
	}

	/* check if the fragments are free.  if so, mark them as
	 * allocated in the working map.
	 */
	if (version == ALLOCMAPV3)
	{
		p1 = p0 + p;  /* pointer to page */
		wmap = (uint *) p1 + LMAPCTL/4;
	}
	else
	{
		p1 = p0 + 9*(p >> 3);
		wmap = (uint *)(p1 + 1 + (p & 0x7));
		p1 = VMDMAPPTR(p1, p);
		TOUCH(p1);
	}
	bitmask = ONES << (32 - nfrags);
	if (nextfrag >= 32)
	{
		/* second word only. check for sufficient free.
		 */
		wmask2 = bitmask >> (nextfrag - 32);
		if (wmap[word + 1] & wmask2)
			return ENOSPC;
		wmap[word + 1] |= wmask2;
	}
	else
	{
		/* check first word.
		 */
		wmask1 = bitmask >> nextfrag;
		if (wmap[word] & wmask1)
			return ENOSPC;

		if (nextfrag + nfrags > 32)
		{
			/* check second word.
		 	 */
			wmask2 = bitmask << (32 - nextfrag);
			if (wmap[word + 1] & wmask2)
				return ENOSPC;
			wmap[word + 1] |= wmask2;
		}

		/* update first word.
		 */
		wmap[word] |= wmask1;
	}

	/* update stats and update tree.
	 */
	p0->freecnt -= nfrags;
	p1->agfree[nag] -= nfrags;
	v_updtree(p1,word);

	return 0;
}

/*
 * v_touchmap(pdtx,fragno)
 *
 * touchs the map page containing fragno within the disk
 * allocation map for pdtx.
 * 
 * RETURN VALUE
 *	none
 */
int
v_touchmap(pdtx,fragno)
int	pdtx;		/* index in PDT */
uint	fragno;  	/* fragment number within the map */
{
	uint p;
	struct vmdmap *p0, *p1;
	uint version, dbperpage, *wmap;

	FS_LOCKHOLDER(pdtx);

	/* get pointer to page 0 of map
	 */
	(void)chgsr(TEMPSR,pdt_dmsrval(pdtx));
	p0 = (struct vmdmap *) (TEMPSR << L2SSIZE);

	assert(fragno < p0->mapsize);

	/* determine map page number containing fragno.
	 */
	version = p0->version;
	dbperpage = WBITSPERPAGE(version);
	p = fragno / dbperpage;
	
	if (version == ALLOCMAPV3)
	{
		p1 = p0 + p;  /* pointer to page */
	}
	else
	{
		p1 = p0 + 9*(p >> 3);
		wmap = (uint *)(p1 + 1 + (p & 0x7));
		TOUCH(wmap);
	}

	/* touch the vmdmap.
	 */
	TOUCH(p1);
}


/*
 * v_allocit(pdtx,frag)
 *
 * allocate the fragments specified if they are free.
 * 
 * RETURN VALUE
 *	0	- ok
 *	EINVAL  - frag is out of range or crosses a double word boundary
 *	ESTALE	- frag is not free
 */

static int
v_allocit(pdtx, frag)
int	pdtx;	/* index in PDT */
frag_t	frag;   /* fragments to be allocated */
{
	int fperpage;
	uint  word, p, nextfrag, bitmask;
	uint fragno, nfrags, bitno, wmask1, wmask2, nag;
	struct vmdmap *p0,*p1;
	uint version, dbperpage, *wmap;

	FS_LOCKHOLDER(pdtx);

	/* get pointer to page 0 of map
	 */
	(void)chgsr(TEMPSR,pdt_dmsrval(pdtx));
	p0 = (struct vmdmap *) (TEMPSR << L2SSIZE);
	version = p0->version;
	dbperpage = WBITSPERPAGE(version);

	/* calculate map page, map bit within the page, and
	 * starting word of double word corresponding to the
	 * first fragment of frag.
	 */
	fragno = frag.addr;
	p = fragno / dbperpage;
	bitno = fragno - p * dbperpage;
	word = (bitno >> L2DBWORD) & (ONES << 1);
	nag = bitno / p0->agsize;

	/* check that frag is not too big
	 */
	fperpage = pdt_fperpage(pdtx);
	nfrags = fperpage - frag.nfrags;
	if (fragno + nfrags > p0->mapsize)
		return EINVAL;

	/* check that it is wholly contained within a
	 * double word of the map.
	 */
	nextfrag = fragno & (DBPERDWORD - 1);
	if (nextfrag + nfrags > DBPERDWORD) 
		return EINVAL;

	/* check if the fragments are free.  
	 */
	if (version == ALLOCMAPV3)
	{
		p1 = p0 + p;  /* pointer to page */
		wmap = (uint *) p1 + LMAPCTL/4;
	}
	else
	{
		p1 = p0 + 9*(p >> 3);
		wmap = (uint *)(p1 + 1 + (p & 0x7));
		p1 = VMDMAPPTR(p1, p);
		TOUCH(p1);
	}
	bitmask = ONES << (32 - nfrags);
	if (nextfrag >= 32)
	{
		/* second word only. 
		 */
		wmask2 = bitmask >> (nextfrag - 32);
		if (wmap[word + 1] & wmask2)
			return ESTALE;
		wmap[word + 1] |= wmask2;
	}
	else
	{
		/* check first word.
		 */
		wmask1 = bitmask >> nextfrag;
		if (wmap[word] & wmask1)
			return ESTALE;

		if (nextfrag + nfrags > 32)
		{
			/* check second word.
		 	 */
			wmask2 = bitmask << (32 - nextfrag);
			if (wmap[word + 1] & wmask2)
				return ESTALE;
			wmap[word + 1] |= wmask2;
		}

		/* update first word.
		 */
		wmap[word] |= wmask1;

	}

	/* actually allocate it
	 */
	p0->freecnt -= nfrags;
	p1->agfree[nag] -= nfrags;
	v_updtree(p1,word);

	return 0;
}

/*
 * v_allocfree(sid,frag, option)
 *
 * allocates a specific fragment if possible if option is true, 
 * frees it if option is 0.
 * 
 */
int
v_allocfree(sid, frag, option)
int	sid;	/* segment id of file */
frag_t	frag;   /* fragment to allocate or free */
int  	option; /* 1 to allocate 0 to free */
{
	int sidx, pdtx, rc;

	sidx = STOI(sid);
	pdtx = scb_devid(sidx);

	FS_LOCKHOLDER(pdtx);

	rc = (option) ? v_allocit(pdtx, frag) : v_dfree(pdtx, frag);

	return rc;
}

/*
 * v_fallocnxt(pdtx,cur,newfrags)
 *
 * attempt to allocate newfrags fragments immediately following the
 * the fragments specified by cur.  the requested fragments must be
 * free and reside within the same map double word as cur.
 *
 * RETURN VALUE
 *      0       - ok
 *      ENOSPC  - next fragments are allocated or in a different
 *                map double word.
 */

int
v_fallocnxt(pdtx, cur, newfrags)
int	pdtx;	   /* index in PDT */
frag_t	cur;       /* fragment pointer of current */
int 	newfrags;  /* number of fragments needed */
{
	int fperpage;
	uint rem, word, p, nextfrag, bitmask, wperag;
	uint fragno, curfrags, bitno, wmask1, wmask2;
	struct vmdmap *p0,*p1;
	uint version, dbperpage, *wmap;

	/* get pointer to page 0 of map
	 */
	(void)chgsr(TEMPSR,pdt_dmsrval(pdtx));
	p0 = (struct vmdmap *) (TEMPSR << L2SSIZE);
	version = p0->version;
	dbperpage = WBITSPERPAGE(version);

	/* calculate map page, map bit within the page, and
	 * starting word of double word corresponding to the
	 * first fragment of cur.
	 */
	fragno = cur.addr;
	p = fragno / dbperpage;
	bitno = fragno - p * dbperpage;
	word = (bitno >> L2DBWORD) & (ONES << 1);

	/* determine the number of fragments covered by cur.
	 */
	fperpage = pdt_fperpage(pdtx);
	curfrags = fperpage - cur.nfrags;

	/* determine the fragment numbers within the double word
	 * of cur's starting fragment and the fragment immediately
	 * following cur's last fragment.  the latter will be the
	 * starting fragment of the new allocation request.
	 */
	rem = fragno & (DBPERDWORD - 1);
	nextfrag = (rem + curfrags  == DBPERDWORD) ? 0 : rem + curfrags;

	/* determine if the new allocation request is wholly contained
	 * within the double word of the map containing cur.
	 */
	if (nextfrag == 0 || nextfrag + newfrags > DBPERDWORD) 
		return ENOSPC;

	/* check if the fragments are free.  if so, mark them as
	 * allocated in the working map.
	 */
	if (version == ALLOCMAPV3)
	{
		p1 = p0 + p;  /* pointer to page */
		wmap = (uint *) p1 + LMAPCTL/4;
	}
	else
	{
		p1 = p0 + 9*(p >> 3);
		wmap = (uint *)(p1 + 1 + (p & 0x7));
		p1 = VMDMAPPTR(p1, p);
		TOUCH(p1);
	}
	bitmask = ONES << (32 - newfrags);
	if (nextfrag >= 32)
	{
		/* second word only. check for sufficient free.
		 */
		wmask2 = bitmask >> (nextfrag - 32);
		if (wmap[word + 1] & wmask2)
			return ENOSPC;
		wmap[word + 1] |= wmask2;
	}
	else
	{
		/* check first word.
		 */
		wmask1 = bitmask >> nextfrag;
		if (wmap[word] & wmask1)
			return ENOSPC;

		if (nextfrag + newfrags > 32)
		{
			/* check second word.
		 	 */
			wmask2 = bitmask << (32 - nextfrag);
			if (wmap[word + 1] & wmask2)
				return ENOSPC;
			wmap[word + 1] |= wmask2;
		}

		/* update first word.
		 */
		wmap[word] |= wmask1;
	}

	/* update stats and update tree.
	 */
	p0->freecnt -= newfrags;
	p1->agfree[bitno/p0->agsize] -= newfrags;
	v_updtree(p1,word);

	return 0;
}
