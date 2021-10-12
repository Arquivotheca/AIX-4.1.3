static char sccsid[] = "@(#)02	1.18  src/bos/kernel/vmm/POWER/v_compsubs.c, sysvmm, bos411, 9437C411a 9/15/94 08:47:36";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:	vm_ckproc, v_insertfrag, v_freefrags, v_rexpand
 *		v_sortframes, v_nextgroup, v_howmany, v_compress
 *		vm_frealloc, v_frealloc, vm_oldfrags, vm_initcomp, 
 *		v_movedaddr
 *
 * ORIGINS: 27
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

#include <sys/types.h>
#include <jfs/inode.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/errno.h>
#include "vmsys.h"
#include <sys/syspest.h>
#include <sys/mstsave.h>
#include <sys/systm.h>
#include <sys/malloc.h>
#include <sys/xmem.h>
#include <sys/jfsc.h>
#include "mplock.h"

/* NFRAGHASH is size of hash-anchor table for movedfrags.
 * encodepage and decodepage1 are only used while running
 * under the compression kproc so they may be used for MP
 * without further serialization.
 */
#define NFRAGHASH 256
static int * encodepage = 0;
static int * decodepage1 = 0;
static struct movedfrag * fraghtab[NFRAGHASH];

/*
 * compp is an exported symbol.  If the compression kernel extension
 * is available, compp will be filled in to point to the entry point
 * for compression and decompression.  Otherwise it will be null.
 */
int (*compp)(
	int	op,		/* compress, decompress, or query	*/
	int	type,		/* compression algorithm		*/
	caddr_t inbuf,		/* input buffer address	  		*/
	size_t	inlength,	/* input buffer length			*/
	caddr_t outbuf,		/* output buffer address		*/
	size_t	outlength) 	/* output buffer length			*/
			= (int(*)())NULL;

/* 
 * vm_ckproc()
 *
 * this is the program executed by the VMM kproc associated with
 * data compression. the kproc is created in xix_mount when a
 * a compressed file system is mounted read-write.
 *
 * this program runs just before the writing of pages which may 
 * require re-allocation of disk.
 *
 * its "input" is the list of such pages on the compio queue
 * (see v_disksubs.c).
 *
 * after compression and disk allocation, the pages are requeued for
 * normal disk i/o.
 *
 */

int
vm_ckproc()
{
	int nframes, rc, realloc, k, nb, pdtx, fperpage, sidx;
	int frames[16], nfr[FSCLSIZE], oldfrags[FSCLSIZE], nfrags[FSCLSIZE];

	/* get addressability to VMM segment
	 */
	mtsr(VMMSR, vmker.vmmsrval);

	nframes = 0;
	setpswap();
	while(1)
	{
		/* get next batch of page frames (same sid)
		 */
		if (nframes == 0)
		{
			nframes = vcs_getcio(16,frames,1);
			v_sortframes(nframes, frames);
		}

		/* get next group of frames in same cluster
		 */
		nb = v_nextgroup(&nframes, frames, nfr);

		/* determine number of fragments needed
		 * compressing  data if necessary.
		 */
		realloc = 0;
		for (k = 0; k < nb; k++)
		{
			nfrags[k] = v_howmany(nfr[k], &oldfrags[k]);
			realloc |= (nfrags[k] != oldfrags[k]);
		}

		/* reallocate if necessary
		 */
		rc = 0;
		if (realloc != 0)
		{
			rc = vm_frealloc(nb, nfr, nfrags);
			/* if there was error other then ENOSPC, re-expand 
			 * pages and write in uncompressed form.
			 */
			if (rc != 0 && rc != ENOSPC)
			{
				sidx = STOI(pft_ssid(nfr[0]));
				fperpage = pdt_fperpage(scb_devid(sidx));
				for (k = 0; k < nb; k ++)
				{
					if (nfrags[k] != fperpage) 
						v_rexpand(nfr[k]);
				}
			}
		}
				
		/* reque pages for i/o ?
		 */
		if (rc == 0 || realloc == 0 || rc != ENOSPC)
		{
			vcs_requeio(nb, nfr);
			continue;
		}

		/* try to reallocate one block at a time.
		 * if that fails we should already have a full
		 * block assigned so we re-expand page.
		 */
		for (k = 0; k < nb; k++)
		{
			if (oldfrags[k] != nfrags[k])
				if (vm_frealloc(1,&nfr[k],&nfrags[k]))
				{	
					v_rexpand(nfr[k]);
				}
			vcs_requeio(1, &nfr[k]);
		}

	}
}


/* v_insertfrag(bsid,pagex)
 *
 * inserts a movedfrag structure into frag hash table if not
 * already in table. if insertion is required, initializes
 * the 4 oldfrag pointers in the structure to zero.
 *
 * storage for a movedfrag structure is allocated by allocating
 * a lock-word. thus sizes must agree and next field must be in
 * first word of both structures.
 *
 * PARAMETERS bsid - base sid of segment
 *	      pagex - page number in file (i.e. absolute pno).
 *
 *
 *	returns - address of movedfrag structure
 *	        - NULL if no memory
 */

struct movedfrag *
v_insertfrag(bsid,pagex)
uint bsid;
uint pagex;
{
	uint hash, phash,rc, k, sidx;
	struct movedfrag * p;
	struct inode *ip;

	LW_LOCKHOLDER();
	/* and we also hold the inode lock
	 */

	/* is it already in table . low order 2-bits of pagex are
 	 * ignored becaused each movedfrag structure represents
	 * four pages.
	 */
	phash = pagex & (0xfffffffc);
	hash = (bsid ^ phash) & (NFRAGHASH - 1);

	for (p = fraghtab[hash]; p != NULL; p = p->next)
	{
		if (p->sid == bsid && p->pno == phash)
		{
			TOUCH(&p->olddisk[3]);
			return p;
		}
	}

	/* allocate a lock-word (movedfrag structure)
	 */
        if ((k = lanch.freelock) == 0)
        {
		if (rc = v_morelocks())
			return NULL;
		k = lanch.freelock;
        }

	/* assign to words other than next field first
	 */
	p = (struct movedfrag *) (&lword[k]);
	p->sid = bsid;
	p->pno = phash;
	p->olddisk[0] = p->olddisk[1] = p->olddisk[2] = p->olddisk[3] = 0;

	/* put movedfrag structure on inode list
	 */
	sidx = STOI(bsid);
	ip = GTOIP(scb_gnptr(sidx));
	p->nextsid = ip->i_movedfrag;

	/* remove from free list and insert on hash chain and inode list.
	 */
	lanch.freelock = lword[k].next;
	p->next = fraghtab[hash];
	fraghtab[hash] = p;
	ip->i_movedfrag = p;

	return p;
}
/* 
 * v_freefrags(ip)
 *
 * frees all movedfrag structures for a inode.
 * this procedure executes with back-tracking enabled at VMM level.
 */
int
v_freefrags(ip)
struct inode * ip;
{
	uint k, ind, phash, hash, bsid ;
	struct movedfrag *p, *q, *next, *q1, *nextsid;

	LW_LOCKHOLDER();
	/* and we also hold the inode lock
	 */

	bsid = ip->i_seg;
	ASSERT(bsid);

	for (q = ip->i_movedfrag; q != NULL; q = nextsid)
	{
		/* find q on hash chain */
		nextsid = q->nextsid;
		phash = q->pno;
		hash = (bsid ^ phash) & (NFRAGHASH - 1);
		
		/* q should be found. if not we loop */
		q1 = (struct movedfrag *) (&fraghtab[hash]);
		for (p = fraghtab[hash]; ; p = p->next)
		{
			if (p != q)
			{
				q1 = p;
				continue;
			}
			/* put p = q  on free list */
			ind = (struct lockword *)p - lword;
			q1->next = p->next;
			p->next = lanch.freelock;
			lanch.freelock = ind;
			break;
		}
		ip->i_movedfrag = nextsid;
	}

	return 0;
}


/*
 * v_rexpand(nfr)
 *
 * re-expands a compressed page.
 * this is used when it was not possible to re-allocate
 * disk for a page. it should have a full disk-block 
 * assigned to it.
 * 
 */
static int
v_rexpand(nfr)
{

	uint srsave, vaddr, sid, sidx;

	/* should have a full block assigned to it. 
	 */
	assert(pft_nfrags(nfr) == 0);
	ASSERT(pft_pageout(nfr));
	sid = IOSID(pft_ssid(nfr));
	sidx = STOI(sid);

	/* make page addressable using its IO virtual address.
	 */
	srsave = chgsr(TEMPSR, SRVAL(sid, 0,0));
	vaddr = (TEMPSR << L2SSIZE) + (pft_spage(nfr) << L2PSIZE);

	/* copy full page to decodepage1
	 */
	bcopy(vaddr, decodepage1, PSIZE);
	assert(compp);
	if (PSIZE != (*compp)(COMP_DECODE, pdt_comptype(scb_devid(sidx)), 
	    decodepage1, PSIZE, vaddr, PSIZE))
	{
		assert(0);
	}

	/* restore value of sreg.
	 */
	(void)chgsr(TEMPSR,srsave);
	return 0;
}
/*
 * v_expandit(nfr)
 *
 * this is used in conjuction with compression. it is called
 * from v_pfend() and decompresses the page. it is assumed
 * that the page is compressed (pft_nfrags(nfr) > 0)
 * A per-CPU buffer is used to perform the decompression.
 */
int
v_expandit(nfr)
{

	uint srsave, modbit, fperpage, fragsize, vaddr, nbytes;
	uint sid, pno, rc, sidx;
	struct ppda *ppda_p = PPDA;

	/* make page addressable. it can most easily be addressed
	 * at its io virtual address.
	 */
	sid = IOSID(pft_ssid(nfr));
	srsave = chgsr(TEMPSR, SRVAL(sid, 0,0));
	sidx = STOI(sid);
	pno = pft_spage(nfr);
	vaddr = (TEMPSR << L2SSIZE) + (pno << L2PSIZE);

	/* determine fragsize, etc.
	 */
	fperpage = pdt_fperpage(pft_devid(nfr));
	fragsize = PSIZE/fperpage;
	nbytes = pft_nfrags(nfr)*fragsize;

	/* copy partial page to per-CPU buffer and uncompress it.
	 */
	rc = 0;
	nbytes = PSIZE - nbytes;
	bcopy(vaddr, ppda_p->decompress, nbytes);
	assert(compp);
	if (PSIZE != (*compp)(COMP_DECODE, pdt_comptype(scb_devid(sidx)), 
	    ppda_p->decompress, nbytes, vaddr, PSIZE))
	{
		rc = EIO;
		bcopy(ppda_p->decompress,vaddr,nbytes);
		ASSERT(0);
	}

	/* restore sreg.
	 */
	(void)chgsr(TEMPSR,srsave);
	return rc;
}
/* 
 * v_sortframes(nf, frames)
 * sorts the frames according to page number in ascending order.
 */

static int
v_sortframes(nf, frames)
int nf;
int * frames;
{
	int k, n, min, temp;

	for (k = 0; k < nf; k++)
	{
		min = pft_pagex(frames[k]);
		for (n = k + 1; n < nf; n++)
		{
			if (pft_pagex(frames[n]) < min)
			{
				min = pft_pagex(frames[n]);
				temp = frames[k];
				frames[k] = frames[n];
				frames[n] = temp;
			}
		}
	}
}

/*
 * v_nextgroup(nframes, frames, nfr)
 *
 * moves page frames from frames[] to nfr[] which are in the 
 * same cluster (ie have the same page number / FSCLSIZE) and
 * have consecutive page numbers.
 *
 * on entry frames is sorted.
 *
 * sets nframes to the number of frames which remain after 
 * the move. the remaining frames are "moved up".
 *
 * return value = number of frames moved to nfr[].
 */
static int 
v_nextgroup(nframes, frames, nfr)
int * nframes;
int * frames;
int * nfr;
{
	uint k, nf,nmoved, cluster, mask, firstp, p;

	nf = *nframes;
	mask = ~ (FSCLSIZE - 1);
	firstp = pft_pagex(frames[0]);
	cluster = firstp & mask;

	for (k = 1; k < nf; k++)
	{
		p = pft_pagex(frames[k]);
		if (p != firstp + k || (p & mask) != cluster)
			break; 
	}
	nmoved = k;

	/* move frames to nfr[] and move remaining frames up
	 */
	for (k = 0; k < nmoved; k++)
		nfr[k] = frames[k];
	for (k = nmoved; k < nf; k++)
		frames[k - nmoved] = frames[k];

	/* set nframes to remaining and return number moved
	 */
	*nframes = nf - nmoved;
	return nmoved;
}

/*
 * v_howmany(nfr, oldfrags)
 * 
 * returns the number of required fragments for page. sets oldfrags to
 * current number of fragments. if it compressed by at least one fragment, 
 * sets page to the compressed value.
 */
static int
v_howmany(nfr, oldfrags)
int nfr;   /* page-frame number */
int  *oldfrags;
{
	uint  pdtx, sid, nbytes;
	uint  fperpage, fragsize;

	pdtx = pft_devid(nfr);
	fperpage = pdt_fperpage(pdtx);
	fragsize = PSIZE/fperpage;
	*oldfrags = fperpage - pft_nfrags(nfr);

	/* page can be addressed at its io virtual address
	 */
	sid = IOSID(pft_ssid(nfr));
	nbytes = v_compress(sid, pft_spage(nfr), fragsize);
	return (nbytes + fragsize - 1)/fragsize;
}

/* 
 * v_compress(sid,pno,fragsize)
 * 
 * if compression by at least fragsize is possible, compresses
 * it and returns the size in bytes after compression. otherwise
 * returns PSIZE; if it does compress, sets page to compressed value.
 *
 */
static int 
v_compress(sid, pno, fragsize)
int sid;
int pno;
int fragsize;
{
	uint input, newfrags, newlen, srsave, sidx;
	int  comptype;

	/* make page addressable. compress it to encodepage.
	 */
	srsave = chgsr(TEMPSR, SRVAL(sid,0,0));
	input = (TEMPSR << L2SSIZE) + (pno << L2PSIZE);
	sidx = STOI(sid);

	comptype = pdt_comptype(scb_devid(sidx));

	assert(compp);
	newlen = (*compp)(COMP_ENCODE, comptype, input, PSIZE, encodepage, 
				fragsize);
	assert(newlen != -1);
	newfrags = (newlen + fragsize - 1)/fragsize;
	
	/* if page compressed, copy compressed page to original
	 * page.
	 */		
	if (newfrags*fragsize < PSIZE) 
	{
		bcopy(encodepage, input, newlen);
	}

	/* restore sreg and return length.
	 */
	(void)chgsr(TEMPSR, srsave);
	return newlen;
	
}

/* vm_frealloc(nb, nfr, nfrags)
 *
 * this invokes vcs_frealloc via an exception handler. 
 * to catch EIO.
 */
static int
vm_frealloc(nb, nfr, nfrags)
int nb;
int *nfr;
int *nfrags; 
{
	int rc;

	/* invoke vcs_frealloc. 
	 */
	rc = vcs_frealloc_excp(nb, nfr, nfrags);
	return rc;
}

/*
 * v_frealloc(nb, nfr,nfrags)
 *
 * reallocates fragments for a group of pages in the same
 * cluster. 
 *
 * THIS CODE ASSUMES FSCLSIZE is at most 4.
 *
 * the number of frames is nb, nfr[] is the array of frames,
 * and nfrags[], the corresponding array of fragments needed.
 * the frames are sorted by page-number and have consecutive
 * page numbers (no gaps in page-numbers).
 *
 * if the current allocation for a page is old, the old 
 * allocation is recorded in the frag hash-table. the fragment
 * pointer in the inode/indirect block for the page is set to
 * the new allocation, with new bit set to one.
 *
 * if the current disk-allocation for a page is new, the 
 * current allocation is freed, and the corresponding 
 * fragment pointer in the inode/indirect blocks set with
 * the new-bit set to one.
 *
 * the case of nb = 1 is handled by truncating the current
 * allocation if the current allocation is new and large
 * enough AND it is not possible to re-allocate elsewhere.
 *
 * the new allocation is disjoint from the old allocation.
 * (i.e. they have no fragments in common).
 *
 * input parameters
 *
 *	nb = number of pages
 *	nfr[] - array of page frames
 *	nfrags[] - array of fragments needed per frame
 *
 *	returns 0 - ok
 *		ENOSPC.  couldn't allocate disk
 *		ENOMEM   couldn't allocate movedfrag structure
 *		VM_WAIT  wait for quota lock.
 *
 * the disk quota is not an issue; sufficient blocks were 
 * allocated already.
 *
 * this is called via vcs_frealloc() and executes with back-tracking
 * enabled.
 */
	
int
v_frealloc(nb, nfr,nfrags)
int nb;
int *nfr;
int *nfrags; 
{
	frag_t fptr;
	struct inode * ip;
	union xptentry * xpt, * v_findiblk();
	struct movedfrag *ptr;
	uint disk, sid, pagex, rc, sidx, hint, k, f, old[FSCLSIZE];
	uint pdtx, fperpage, nnew, newfrags, oldfrags;
	int nfreed;


	/* get xpt for the first page in group
	 */
	f = nfr[0];
	sid = pft_ssid(f);
	sidx = STOI(sid);

	/* get pdtx, etc
	 */
	pdtx = pft_devid(f);
	fperpage = pdt_fperpage(pdtx);

	/*
	 * Lock SCB to search inode addressing structures and
	 * to perform reallocations.  Lock PDT to protect
	 * state of .indirect pages.
	 */
	SCB_MPLOCK_S(sidx);
	FS_MPLOCK_S(pdtx);

	pagex = pft_pagex(f);
	xpt = v_findiblk(sidx, pagex);

	/* touch ip  and last entry in nfr[]
	 */
	ip = GTOIP(scb_gnptr(sidx));
	TOUCH(ip);
	TOUCH((char *)(ip + 1)  - 1);
	TOUCH(nfr + nb - 1);

	/* count number of fragments that will be needed
	 * (newfrags) . build array of current allocations
	 * which are new and will be freed (old[]). nnew
	 * is the number of such current allocations.
	 */
	nnew = newfrags = 0;
	for (k = 0; k < nb; k++)
	{
		assert((xpt + k)->fptr.nfrags == 0);
		if ((xpt + k)->newbit)
		{
			old[nnew++] = (xpt + k)->fragptr;
		}
		newfrags += nfrags[k];
	}
	oldfrags = fperpage*nb;

	/* test if quota is locked if we are giving something back.
	 */

	if (nfreed = oldfrags - newfrags)
	{
		if (rc = v_chkdqlock(ip))
		{
			FS_MPUNLOCK_S(pdtx);
			SCB_MPUNLOCK_S(sidx);
			return rc;
		}
	}

	/* if some or all are old, allocate a movedfrag data
	 * structure. (here is where we need to fix things up
	 * if we want FSCLSIZE bigger than 4).
	 */
	if (nnew != nb)
	{
		LW_MPLOCK_S();
		if ((ptr = v_insertfrag(BASESID(sid),pagex)) == NULL)
		{
			LW_MPUNLOCK_S();
			FS_MPUNLOCK_S(pdtx);
			SCB_MPUNLOCK_S(sidx);
			return ENOMEM;
		}
		LW_MPUNLOCK_S();
	}

	/* reallocate the blocks or allocate all new blocks.
	 * v_realloc() will free the blocks in old[].
	 */
	if (nnew > 0)
	{
		rc = v_realloc(pdtx,nnew, old, &disk, newfrags);
	}
	else
	{
		hint = pft_fraddr(nfr[nb - 1]);
		rc = v_psalloc(pdtx,newfrags,hint, &disk);
	}

	/* higher level code should try one block at a time
	 * if reallocation fails. 
	 */
	if (rc != 0)
	{
		if (nnew != nb || nb != 1) 
		{
			FS_MPUNLOCK_S(pdtx);
			SCB_MPUNLOCK_S(sidx);
			return ENOSPC;
		}

		/* free excess fragments making it look like 
		 * the allocation succeeded.
		 */
		if (oldfrags == newfrags) return 0;
		disk = pft_fraddr(nfr[0]);
		fptr.addr = disk + newfrags;
		fptr.nfrags = fperpage - (oldfrags - newfrags);
		v_dfree(pdtx,fptr);
	}

	/* update pfts and xpts. remember old allocations in
	 * movedfrag structure.
	 */
	for (k = 0; k < nb; k++)
	{
		f = nfr[k];
		if ((xpt + k)->newbit == 0)
		{
			/* remember old allocation in movedfrag structure.
		 	 */
			pagex = pft_pagex(f);
			ptr->olddisk[pagex & 0x3] = pft_dblock(f);
			scb_newdisk(sidx) += 1;
		}
		pft_dblock(f) = disk;
		pft_nfrags(f) = fperpage - nfrags[k];
		(xpt + k)->word = pft_dblock(f) | NEWBIT;
		disk += nfrags[k];
	}

	/* update inode and quota if necessary.
	 */
	if (nfreed)
	{
		ip->i_nblocks -= nfreed;
		v_upddqblks(ip,-nfreed , fperpage);
	}

	/* the FS lock is enough to interlock with v_sync,
	 * serialization with commit flush of .indirect pages
	 * is covered by the combit (and the scb lock on top).
	 */
	v_nohomeok(xpt);

	FS_MPUNLOCK_S(pdtx);
	SCB_MPUNLOCK_S(sidx);

	return 0;
}

/*
 * vm_oldfrags(ip,ptr,position)
 *
 * copies data from the moved frag structures of a segment to
 * the vmdlist structure pointed to by pointer. position is the
 * pointer to the first movedfrag structure to begin with. 
 *
 * (1) ip - pointer to inode
 *
 * (2) ptr - pointer to vmdlist structure. 
 *
 * (3) position- set to position of next movedfrag structure..
 *     if there are more to process. specify as NULL for first
 *     call to this procedure.
 *
 *  RETURN  VALUES
 *
 *      0       - ok
 *
 * this procedure is called from freeoldfrags(). the movedfrags
 * for the inode will not change because the segment is locked.
 */

int
vm_oldfrags(ip,ptr,position)
struct inode * ip;
struct vmdlist * ptr;
struct movedfrag ** position;
{
	struct movedfrag  *p1;
	uint savevmsr, n, k;

        /* get addressability to vmmdseg 
         */
        savevmsr = chgsr(VMMSR,vmker.vmmsrval);

	/* process the movedfrag structures up to space in
	 * in vmdlist structure.
	 */
	n = ptr->nblocks = 0;
	p1 = (*position == NULL) ? ip->i_movedfrag : *position;

	while (p1 != NULL && n < NDLIST - 4)
	{
		for (k = 0; k < 4; k ++)
		{
			if (p1->olddisk[k] != 0) 
				ptr->da.dblock[n++] = p1->olddisk[k];
		}
		p1 = p1->nextsid;
	}

	*position = p1;
	ptr->nblocks = n;
        (void)chgsr(VMMSR,savevmsr);
        return 0;
}


/*
 * vm_initcomp()
 * this allocates the memory needed by data compression . it is 
 * called from xix_mount().
 *
 * PARAMETERS: ronly - true if read-only filesystem
 *
 * RETURN : 	0 ok.
 *		ENOMEM - couldn't allocate stirage.
 */
int
vm_initcomp(ronly)
int ronly;
{
	int cpu, cpu2;

	/*
	 * Allocate the per-CPU decompress buffers.
	 * These are needed even for read-only filesystems.
	 */
	for (cpu = 0; cpu < NCPUS(); cpu++)
	{
		if (ppda[cpu].decompress == NULL)
		{
			if ((ppda[cpu].decompress =
				xmalloc(PSIZE,0,pinned_heap)) == NULL)
			{
				for (cpu2 = 0; cpu2 < cpu; cpu2++)
					xmfree(ppda[cpu2].decompress,
						pinned_heap);
				return ENOMEM;
			}
		}
	}

	if (ronly) return 0;

	if (encodepage == NULL)
		if ((encodepage = xmalloc(PSIZE,0,pinned_heap)) == NULL)
			return ENOMEM;

	if (decodepage1 == NULL)
		if ((decodepage1 = xmalloc(PSIZE,0,pinned_heap)) == NULL)
			return ENOMEM;


	return 0;
}

/* v_movedaddr(bsid,pagex,oldfrag,newfrag)
 *
 * change the disk fragments associated with a page from
 * oldfrag to newfrag.
 *
 * PARAMETERS:
 *	bsid - base segment identifier
 *	pagex - page number in file
 *	oldfrag - current frag_t associated with page
 *	newfrag - new frag_t for page
 *
 * RETURN VALUES:
 *	0 - success
 *	ESTALE - oldfrag is not current
 *	ENOMEM - couldn't allocate lockword or movedfrag structure.
 *
 * on entry, newfrag is assumed to have been allocated. if the
 * segment is not journaled, oldfrag has already been copied to
 * newfrag. if journaled, the page is brought into memory and
 * its lockword allocated and filled in, and its modbit set.
 *
 * this procedure is called via vcs_movedaddr() and executes with
 * back-tracking. it is only used in conjuction with defragmenting
 * a file system ; it is not called if the file is deferred update.
 * nor for journaled files other than directories. see xix_cntl.
 */
int
v_movedaddr(bsid, pagex, oldfrag, newfrag)
int bsid;
int pagex;
frag_t oldfrag;
frag_t newfrag;
{
        int rc, sidx, nfr, lw, pdtx, fperpage;
	struct movedfrag * ptr;
	union xptentry *xpt, *v_findiblk();
	uint vaddr, sid, pno, len;

	/* get sidx. if journalled touch page to bring into
	 * memory.
	 */
	sidx = STOI(bsid);

	/* scb lock needed to insure that the frame will not disappear
	 * if found by the lookup, or will not come after.
	 */
	SCB_LOCKHOLDER(sidx);

	pdtx = scb_devid(sidx);
	sid = ITOS(sidx, pagex);
	pno = BASEPAGE(pagex);
	if (scb_jseg(sidx))
	{
		mtsr(TEMPSR, SRVAL(sid, 0, 0));
       		vaddr = SR13ADDR + (pno << L2PSIZE);
		TOUCH(vaddr);
	}

	/* Protect state of .indirect pages.
	 */
	FS_MPLOCK_S(pdtx);

	/* get xpt entry for pagex
	 * check that oldfrag is current and committed.
	 */
	xpt = v_findiblk(sidx,pagex);
	if (xpt->fptr.new || xpt->fptr.nfrags != oldfrag.nfrags 
		|| xpt->fptr.addr != oldfrag.addr) 
	{
		FS_MPUNLOCK_S(pdtx);
		return ESTALE;
	}

	/* if pno is journalled, v_getlw() will make sure a
	 * lockword can be allocated below. lw is set if there
	 * already is one.
	 *
	 * we take the lw lock for both the lockword and
	 * the movedfrag structures which are taken from
	 * the same pool.
	 */
	LW_MPLOCK_S();

	if (scb_jseg(sidx))
	{
		if (rc = v_getlw(sid,pno,&lw))
		{
			LW_MPUNLOCK_S();
			FS_MPUNLOCK_S(pdtx);
			return rc;
		}

		/* don't try to handle extension memory case
		 */
		if (lw != 0 && lword[lw].extmem)
		{
			LW_MPUNLOCK_S();
			FS_MPUNLOCK_S(pdtx);
			return ESTALE;
		}
	}

	/* if page frame is in memory, make sure it is in inuse
	 * state.
	 * XXX In MP this could be moved before the locking of
	 * both fs and lw lock because it is no use to lock them
	 * if we fault here.
	 */ 
	nfr = v_lookup(sid, pno);
	if (nfr > 0)
	{
		if (pft_inuse(nfr) == 0)
		{
			mtsr(TEMPSR, SRVAL(sid, 0, 0));
       			vaddr = SR13ADDR + (pno << L2PSIZE);
			TOUCH(vaddr);
		}
	}

	/* allocate or find a movedfrag structure
	 */
	if ((ptr = v_insertfrag(bsid, pagex)) == NULL)
	{
		LW_MPUNLOCK_S();
		FS_MPUNLOCK_S(pdtx);
		return ENOMEM;
	}

	/* now everything should be in memory and it's ok to do it.
	 * make it look like a re-allocation of the fragments.
	 */
	ptr->olddisk[pno & 0x3] = xpt->fragptr;
	xpt->fptr = newfrag;
	xpt->fptr.new = 1;
	/* No need for atomic update, use as a flag only
	 */
	scb_newdisk(sidx) += 1;
	v_nohomeok(xpt);

	/* if page frame is in memory. change disk address
	 * fields in the pft. set newbit.
	 */ 
	if (nfr > 0)
	{
		pft_dblock(nfr) = xpt->fragptr;
		pft_newbit(nfr) = 1;
	}


	/* if pagex is journaled,  allocate a lockword that covers
	 * the fragments in the page.
	 */
	if (scb_jseg(sidx))
	{
		SETMOD(nfr);
		if (lw) lword[lw].home = xpt->fragptr;
		fperpage = pdt_fperpage(pdtx);
		len = (fperpage - oldfrag.nfrags)*(PSIZE/fperpage);
		rc = v_gettlock(sid,vaddr,len);
		assert(rc == 0);
	}

	LW_MPUNLOCK_S();
	FS_MPUNLOCK_S(pdtx);

	return 0;
}
