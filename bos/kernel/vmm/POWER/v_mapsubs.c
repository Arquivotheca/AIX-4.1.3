static char sccsid[] = "@(#)03	1.49  src/bos/kernel/vmm/POWER/v_mapsubs.c, sysvmm, bos412, 9445C412b 11/4/94 12:37:03";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:	v_mapv, v_findiblk, v_homeok, v_growiblk, v_moreiblks,
 *		v_setsize
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

#include  "vmsys.h"
#include  "vmvars.h"
#include  <jfs/inode.h>
#include  <sys/inline.h>
#include  <sys/syspest.h>
#include  <sys/errno.h>
#include  "mplock.h"

/*
 * v_mapv(tsid,mapx,tfirst,npages)
 *
 * maps an interval of pages to the target segment.
 * mapx is the index of a vmapblk which specifies
 * the source segment, source and target origins.
 * tfirst is the first page in the target and npages
 * the number of pages to be mapped on this call.
 *
 * this routine is only called by vm_map(). it runs 
 * at VMM interrupt level with back-track enabled.
 *
 * input params:
 *
 * tsid - segment id of target
 *
 * mapx = index of a vmapblk.
 *
 * tfirst - first target page this call.
 *
 * npages - the number of pages to map this call.
 *
 * Return values  0     ok.
 *
 *                ENOMEM - out of space for xpts or early paging
 *				space allocation failed.
 *
 * NOTE -       
 *              if  vm_map() is called with src addr same as
 *              target address, then zero filled pages are
 *		given on faults for the target  segment. 
 *
 */

v_mapv(tsid,mapx,tfirst,npages)
int     tsid;
int     mapx;
int     tfirst;
int     npages;
{
        int p,rc,tsidx, tlast;
	int ssid, ssidx, sf, sl, chknull;
        union xptentry *xpt ,*v_findxpt();

        /* get xpt entry. if it is NULL we don't have to do anything
	 * if the source xpt is NULL and both source and target are
	 * not subject to inheritance (i.e have no parents). this is
	 * important when a whole segment is mapped but the source is
	 * sparsely populated. no inheritance implies the value of pages
	 * with NULL xpt is zero.
	 *
	 * For target segments that are sparse, we need to create
	 * a map block to ensure a zero filled page on page fault.
	 * If a map block is not created, an exception is generated
	 * on these pages when the xpt is not found.
	 */

        tsidx = STOI(tsid);
	SCB_LOCKHOLDER(tsidx);

        if ((xpt = v_findxpt(tsidx,tfirst)) == NULL && !scb_sparse(tsidx))
        {
		/* No need to acquire the vmap lock,
		 * the vmap block is not yet associated w/ an xpt
		 * and thus should not be accessed by another thread.
		 * No need to lock the source.
		 */
		ssid = pta_vmap[mapx]._u.sid;
		ssidx = (ssid == INVLSID) ? tsidx : STOI(ssid);
		chknull = (scb_parent(tsidx) == 0) && (npages == XPTENT)
			&& (scb_wseg(ssidx) && scb_parent(ssidx) == 0);
		if (chknull)
		{
			sf = tfirst - pta_vmap[mapx].torg + pta_vmap[mapx].sorg;
			sl = sf + XPTENT - 1;
			if ( !v_findxpt(ssidx,sf) && !v_findxpt(ssidx,sl))
				return 0;
		}
        }

	/* have to perform mapping. grow xpt if xpt is null.
	 */
	if (xpt == NULL)
	{
                if(rc = v_growxpt(tsidx,tfirst))
                        return(rc);
                xpt = v_findxpt(tsidx,tfirst);

	}
        else
        {
                /* page release the pages
                 */
                v_release(tsid,tfirst,npages,0);
        }

	/* update scb_maxvpn or scb_minvpn to reflect this
	 * mapping so that when segment is deleted we clean
	 * up xpt (even if pages are never referenced).
	 * On MP we must check again that the mapping
	 * is within scb_uplim now that we hold the SCB lock.
	 */
	tlast = tfirst + npages - 1;
	if (tlast > scb_uplim(tsidx))
		return EINVAL;
	scb_maxvpn(tsidx) = MAX(tlast,scb_maxvpn(tsidx));

	/* 
	* if it is an early allocation segment and 
	* a sparse allocation segment, then
	* adjust the paging space counts
	*/
        if ((scb_psearlyalloc(tsidx) && scb_sparse(tsidx)))
	{
		if (FETCH_AND_LIMIT(vmker.psfreeblks, npages, pf_npswarn))
		{
			scb_npseablks(tsidx) += npages;
		}
		else
		{
			return ENOMEM;
		}
			
	}			


        /* set xpt entries to mapblk
         */
        for (p = tfirst; p <= tfirst + npages - 1; p ++, xpt++)
        {
                xpt->word = 0;
                xpt->cdaddr = mapx;
                xpt->mapblk = 1;
                xpt->spkey = (p <= scb_sysbr(tsidx)) ? scb_defkey(tsidx)
						     : pf_kerkey;
        }

	/*
	 * Another thread could be faulting on an XPT block mapped by
	 * a previous call to v_mapv that references this vmap block
	 * so we must update the count carefully.
	 */
	FETCH_AND_ADD(pta_vmap[mapx].count, npages);

        return 0;
}

/*
 * v_findiblk(sidx,pno)
 *
 * returns a pointer to the xptentry for the page pno in the
 * segment specified or NULL if there is none. the segment is
 * assumed to be a persistent segment.
 *
 * segment register 12 (i.e PTASR) is loaded to address the 
 * .indirect segment if the page is covered by an indirect
 * block.
 */

#define PTSADDR  (PTASR*SEGSIZE)
union xptentry *
v_findiblk(sidx,pno)
int sidx;	/* index in sid table */
int pno;	/* page number in segment */
{
	struct inode * ip , * ipind;
	struct idblock * idptr;
	int k, n, noind, sglind;

	SCB_LOCKHOLDER(sidx);

	/* get inode pointer
	 */
	ip = GTOIP(scb_gnptr(sidx));
	noind = NOIND(ip->i_size);

	/* for small page numbers we use the inode table entry
	 * but only if it has no indirect blocks or if the 
	 * file is a .indirect. .indirect is treated specially
	 * to avoid indefinite-recursion of page faults. we use
	 * indirect blocks if they are present to simplify commit
	 * and delete code.
	 */
	if (pno < NDADDR)
	{
		if (noind || ip->i_number == INDIR_I)
			return (( union xptentry *) &(ip->i_rdaddr[pno]));
	}

	/* need an indirect block.
	 */
	if (noind)
		return(NULL);

	/* do we need a double indirect ?
	 */
	if (sglind = SGLIND(ip->i_size))
	{
		if (DBLIND((pno + 1)<< L2PSIZE))
			return(NULL);
	}

	/* get inode pointer for .indirect.
	 */
	ipind = ip->i_ipmnt->i_ipind;

	/* address .indirect using the PTASR
	 */
	(void)chgsr(PTASR, SRVAL(ipind->i_seg,0,0));

	/* calculate address of direct block.
	 */
	if (sglind)
	{
		k = PTSADDR + PSIZE * ip->i_vindirect;
	}
	else
	{
		n =  PTSADDR + PSIZE * ip->i_vindirect;
		idptr = (struct idblock *)n;		
		n = pno/(PSIZE/4);
		idptr = idptr + n;
		k = (idptr->id_vaddr) ? PTSADDR + PSIZE*idptr->id_vaddr : 0;
		if (k == 0)
			return NULL;
	}

	/* calculate address of pointer within direct block
	 */
	k = k + 4*( pno & (PSIZE/4 - 1));
	return ((union xptentry *) k) ;

}

/*
 * v_nohomeok(ptr)
 * 
 * sets the homeok bit in the page frame table entry for the 
 * page underlying ptr to false, this page is assumed to be 
 * in memory.
 */

v_nohomeok(ptr)
void * ptr;
{
	int nfr;

	nfr = v_lookup(SRTOSID(mfsri(ptr)),((uint) ptr & SOFFSET) >> L2PSIZE);
	ASSERT(nfr >= 0);
	pft_homeok(nfr) = 0;
}

/*
 * v_growiblk(sidx,pno)
 *
 * adds one or more indirect blocks to a segment to cover
 * a new page. allocates a disk-block to cover pno itself.
 *
 * normally one indirect block suffices. two blocks are
 * required if a double indirect block must be created
 * as well as a direct block to cover pno. three blocks
 * must be added when pno is bigger that PAGESIZE/4 - 1
 * (the last page covered by the first indirect block) and
 * there are pages covered by the inode but there is not
 * yet a first block to cover these pages (this first
 * block is in theory not needed but for uniformity the
 * architecture requires its presence).
 *
 * input parameters 
 *	
 *	sidx - index in scb table for segment
 *
 *	pno  - number of page which needs indirect block
 *
 * this procedure executes at VMM interrupt level with
 * backtracking enabled and only is called from within
 * the VMM interrupt handling code.
 *
 * this procedure uses the PTA segment register without
 * restoring it.
 *
 * Return values  - 0  ok
 *  		  - ENOMEM    .indirect is full
 *  		  - ENOSPC    not enuff disk -space
 *		  - VM_WAIT - not enuff free-frames now.
 */
v_growiblk(sidx,pno)
int sidx;
int pno;
{
	union xptentry *xpt, *v_findiblk();
	struct inode * ip , * ipind;
	uint rc, numind, disk, pdtx;
	int hint, fperpage, srval, indsidx, mine;

	SCB_LOCKHOLDER(sidx);

	/* get inode pointer. make sure its in memory
	 */
	ip = GTOIP(scb_gnptr(sidx));

	/* get inode pointer for .indirect and touch it.
	 */
	ipind = ip->i_ipmnt->i_ipind;

	/* address .indirect using the PTASR
	 */
	srval = chgsr(PTASR, SRVAL(ipind->i_seg,0,0));

	/* figure out how many indirect blocks will be needed.
	 */
	if (rc = howmanyind(ip, pno, &numind))
		return rc;

	/* make sure there are at least this many + 1 free-page frames
	 */
	if ((rc = v_spaceok(sidx, numind+1)))
		return rc; 

	/* make sure inodes is in memory.
	 * (after spaceok which could have stolen them).
	 */
	TOUCH(ip);
	TOUCH((char *)(ip + 1) - 1);
	TOUCH(ipind);
	TOUCH((char *)(ipind + 1) - 1);

	/*
	 * Acquire SCB lock for .indirect unless we are
	 * growing .indirect itself (we may add a frame to
	 * the .indirect SCB and need the lock to do this).
	 * Acquire FS lock before touchptrs which may add a new
	 * page of pointers to the .indirect free list.
	 */
	pdtx = scb_devid(sidx);
	indsidx = STOI(ipind->i_seg);
	if((mine = scb_indseg(sidx)) == 0)
		SCB_MPLOCK_S(indsidx);
	SCB_LOCKHOLDER(indsidx);
	FS_MPLOCK_S(pdtx);

	/* touch the free list pointers and also the 
	 * double indirect block of ip if any.
	 */
	if (rc = touchptrs(ip,numind))
		return rc;

	/* check disk quotas for the inode.
	 */
	fperpage = pdt_fperpage(pdtx);
	if (rc = v_allocdq(ip,fperpage,fperpage))
		goto out;

	/* now allocate enough disk blocks for the indirect
	 * blocks and the page pno.  try to allocate within
	 * the allocation group containing the disk inode.
	 */
	hint = (ip->i_number / scb_iagsize(sidx)) * scb_agsize(sidx);
	if (v_psalloc(pdtx,(numind + 1) * fperpage,hint,&disk))
	{
		rc = ENOSPC;
		goto out;
	}

	/* now make the new indirect blocks.
	 */
	makeindirect(ip,pno,numind,disk,fperpage);

	/* fill in the new disk-block for pno
	 */
	xpt = v_findiblk(sidx,pno);
	xpt->newbit = 1;
	xpt->fragptr = disk + (numind * fperpage);
	scb_newdisk(sidx) += 1;
	ip->i_nblocks += fperpage;

	/* update the disk quota.
	 */
	v_upddqblks(ip,fperpage,fperpage);

	ASSERT(rc == 0);
out:
	FS_MPUNLOCK_S(pdtx);
	if(mine == 0)
		SCB_MPUNLOCK_S(indsidx);

	return rc;
}

/*
 * howmanyind(ip,pno,numind)
 *
 * sets numind to the number of indirect blocks that 
 * have to be added to cover pno. touches the free-list
 * pointers in .indirect so that allocation can be done
 * later in makeindirect() without page faulting.
 *
 * on entry .indirect is mapped into VM using the PTASR.
 *
 *	return 0 ok
 *		- ENOMEM - .indirect is full
 */

static
howmanyind(ip,pno,numind)
struct inode *ip;
uint pno;
uint * numind;
{
	struct idblock * idptr;
	int dblind, k;
	
	/* if pno is covered by the first indirect block 
	 * only one indirect block is needed. 
	 */
	if (pno < PSIZE/4)
	{
		*numind = 1;
		return 0;
	}

	/* pno is bigger than covered by first indirect block.
	 * so we will always have to have a double indirect
	 * block. we may also need to have a first single indirect 
	 * block to cover the blocks covered by the inode
	 * This first indirect block is always made at the time the 
	 * segment gets a double-indirect block because the inode is not
	 * used to record disk addresses once it has indirect blocks.
	 */
	dblind = DBLIND(ip->i_size);
	*numind = (dblind) ? 1 : 2;
	if (ip->i_vindirect == 0)
		*numind += 1;

	return 0;
}

/*
 * touchptrs(n)
 *
 * touchs the free list pointers needed to allocate n indirect
 * blocks. .indirect is mapped into VM using the PTASR on entry.
 *
 *   returns 0	- ok
 *	     ENOMEM - .indirect is full
 *
 */

static
touchptrs(ip,n)
uint n;  /* number indirect blocks needed */
struct inode *ip;
{
	struct indir * ptr;
	int p, sid, rc, k;

	ptr = (struct indir *) PTSADDR;

	/* touch the double indirect block if it is one
	 */
	if (DBLIND(ip->i_size))
	{
		ASSERT(ip->i_vindirect);
		TOUCH(PTSADDR + PSIZE * ip->i_vindirect );
	}

	loop:
	/* first page on free list
	 */
	p = ptr->free;
	for (k = 0; k < n; k++)
	{
		if (p == 0)
		{
			sid = SRTOSID(mfsr(PTASR));
			if (rc = v_moreiblks(sid))
				return rc;
			goto loop;
		}
		TOUCH(&ptr->indptr[p]);
		/* next page on free list
		 * assertion says p is not a disk address.
		 */
		p = ptr->indptr[p];
		assert (!(p & NEWBIT));
	}

	return 0;
}

/*
 * makeindirect(ip,pno,numind,disk,fperpage)
 *
 * makes indirect blocks for segment ip.
 * this program should not page-fault and should 
 * never fail because of setup in v_growiblk().
 */
static
makeindirect(ip,pno,numind,disk,fperpage)
struct inode *ip;
int pno;  	/* page number to cover */
int numind;	/* number of indirect blocks needed */
uint disk;	/* first disk block to allocate */
int fperpage;	/* fragments per page */
{

	/* if one is needed it is always a single indirect block.
	 */
	if (numind == 1)
	{
		v_makeiblk(ip,pno,disk);
		if (pno < PSIZE/4)
			v_inodecopy(ip);
		return 0;
	}

	/* whenever there are two or more needed we always 
	 * have to make a double indirect block.
	 */
	v_makeiblk(ip, -1, disk);
	v_makeiblk(ip,pno,disk + fperpage);

	/* if there is a third indirect block to make
	 * it is a first-indirect block which did not
	 * cover pno.
	 */
	if (numind == 3)
	{
		v_makeiblk(ip,0,disk + (2 * fperpage));
		v_inodecopy(ip);
	}

	return 0;
}

/*
 * v_makeiblk(ip,pno,disk)
 *
 * make an indirect block.
 * this should never fail nor page fault because of prior setup.
 * if pno < 0 a double-indirect block is made. otherwise an 
 * indirect block covering pno.
 */
static
v_makeiblk(ip,pno,disk)
struct inode *ip;
int pno;
uint disk;
{
	int p, sid, k, ind;
	union xptentry * xpt;
	struct indir * ptr;

	/* take page in .indirect off free list. sets disk
	 * address in its xpt entry.
	 */
	ptr = (struct indir *) PTSADDR;
	p = ptr->free;
	ASSERT(p);
	assert (!(p & NEWBIT));
	ptr->free = ptr->indptr[p];
	xpt = (union xptentry *) (&ptr->indptr[p]);
	xpt->word = disk;
	xpt->newbit = 1;

	/* back the page in .indirect with a page-frame
	 */
	sid = SRTOSID(mfsr(PTASR));
	FS_LOCKHOLDER(scb_devid(STOI(sid)));
	v_backpage(sid,p,disk);

	/* was it a double indirect block ?
	 * if so copy single indirect info to it from inode table
	 * and set inode table entry to point to it.
	 */
	if (pno < 0)
	{
		ptr->page[p - FIRSTIND].root[0].id_vaddr = ip->i_vindirect;
		ptr->page[p - FIRSTIND].root[0].id_raddr = ip->i_rindirect;
		ip->i_vindirect = p;
		ip->i_rindirect = ptr->indptr[p];
		ip->i_size = MAX(ip->i_size, (PSIZE/4 + 1)*PSIZE);
		return 0;
	}

	/* is this the first indirect block for the segment?
	 */
	if (ip->i_rindirect == 0)
	{
		ip->i_vindirect = p;
		ip->i_rindirect = ptr->indptr[p];
		ip->i_size = MAX(ip->i_size, (pno + 1)*PSIZE);
		return 0;
	}

	/* segment had a double indirect block
	 */
	ind = ip->i_vindirect;
	k = pno/(PSIZE/4);   /* index in double indirect */
	ptr->page[ind - FIRSTIND].root[k].id_vaddr = p;
	ptr->page[ind - FIRSTIND].root[k].id_raddr  = ptr->indptr[p];
	ip->i_size = MAX(ip->i_size, (pno + 1)*PSIZE);

	/* set no homeok bit for the page frame underlying the double
	 * indirect block.
	 */
	v_nohomeok(&ptr->page[ind - FIRSTIND]);
	return 0;
}

/* 
 * v_inodecopy(ip)
 * copy the disk addresses in the inode to the first indirect
 * block of the file.
 */
static
v_inodecopy(ip)
struct inode *ip;
{
	int k,p;
	struct indir *ptr;
	struct idblock *idptr;

	/* set ptr to .indirect
	 */
	ptr = (struct indir *) PTSADDR;

	/* calculate  page p of first indirect block 
	 */
	if (SGLIND(ip->i_size))
		p = ip->i_vindirect;
	else
	{
		k = (uint)ptr + (PSIZE*ip->i_vindirect);
		idptr = (struct idblock *) k;
		p = idptr->id_vaddr;
	}

	/* copy disk addresses in inode to first indirect block
	 */
	for (k = 0; k < NDADDR; k ++)
		ptr->page[p - FIRSTIND].ptr[k] = ip->i_rdaddr[k];
		
	return 0;
}

/* 
 * v_backpage(sid,pno,disk)
 *
 * take a page frame from free list and associated with 
 * sid and pno and disk. initialize page to zeros.
 * this procedure is only used for backing a page in a
 * .indirect segment.
 */

static
v_backpage(sid,pno,disk)
int sid;
int pno;
uint disk;
{
	int nfr, sidx;
	int sidio, pnoio;

	/* page had better not be in memory
	 */
	assert(v_lookup(sid,pno) < 0);

	/* get page frame
	 */
	sidx = STOI(sid);
	SCB_LOCKHOLDER(sidx);
	FS_LOCKHOLDER(scb_devid(sidx));
        nfr = v_getframe(sid,pno);
        pft_devid(nfr) = scb_devid(sidx);
        pft_dblock(nfr) = disk;
	pft_newbit(nfr) = 1;
	pft_homeok(nfr) = 1;
	pft_journal(nfr) = 1;

	/* Insert the page at its I/O address and set
	 * the correct key to initialize the page.
	 */
	pft_key(nfr) = FILEKEY;
	sidio = IOSID(sid);
	pnoio = pno;
	P_ENTER(STARTIO,sidio,pnoio,nfr,pft_key(nfr),pft_wimg(nfr));
	pft_pagein(nfr) = 1;
	scb_maxvpn(sidx) = MAX(pno,scb_maxvpn(sidx));

	/* zero page 
	 */
	ZEROPAGE(sidio,pnoio);

	/* Insert page at normal address.
	 * This also clears the modified bit so we need
	 * to set it again.
	 */
	pft_pagein(nfr) = 0;
	P_ENTER(IODONE,sid,pno,nfr,pft_key(nfr),pft_wimg(nfr));
	pft_inuse(nfr) = 1;
	SETMOD(nfr);

	/* Insert on SCB list.
	 */
	v_insscb(sidx,nfr);

	return(0);
}
/*
 * v_moreiblks(sid)
 *
 * adds the next page of free indirect block pointers to the free
 * list to the .indirect segment specified.
 *
 * input parameters
 *		sid -	segment id of .indirect 
 * return values
 *		0 -ok
 *		ENOMEM - .indirect is full.
 *
 * this procedure runs with at VMM interrupt level with back-tracking
 * enabled.
 *
 */
v_moreiblks(sid)
int sid;
{
	struct indir *ptr;
	int k, srval, more;

	SCB_LOCKHOLDER(STOI(sid)); 
	FS_LOCKHOLDER(scb_devid(STOI(sid)));

	/* get addressability to the .indirect segment
	 */
	srval = chgsr(PTASR, SRVAL(sid,0,0));
	ptr = (struct indir *) PTSADDR;

	/* the more field is the index of the next free page
	 * of indirect pointers.
	 */
	if ((more = ptr->more)  >= MAXVPN)
		return(ENOMEM);

	/* put them on free list
	 */
	for (k = 0; k < PSIZE/4 - 1; k++)
		ptr->indptr[more + k] = more + k + 1;

	ptr->indptr[more + PSIZE/4 - 1] = ptr->free;
	ptr->free = more;
	ptr->more = more + PSIZE/4;
	(void)chgsr(PTASR,srval);

	return(0);
}

/*
 * v_setsize(ip,newsize)
 *
 * sets the i_size field of a persistent segment to newsize
 * provided newsize specifies an address in the page defined
 * by i_size and the page's allocation matches newsize. also
 * sets the storage protect key of the page to read-only unless
 * newsize corresponds to the last byte of this page. 
 *
 * this program runs at VMM interrupt level with back-tracking
 * enabled.
 *
 * return value 0.
 */

int
v_setsize(ip, newsize)
struct inode * ip;
uint newsize;
{
	
	int pagex, pno, sid, sidx, tsid, nfr, fperpage;

	sid = ip->i_seg;
	sidx = STOI(sid);

	SCB_MPLOCK_S(sidx);

	/* nothing to do if newsize is not in last page
	 */
	pagex = BTOPN(ip->i_size);
	if (pagex != BTOPN(newsize))
	{
		SCB_MPUNLOCK_S(sidx);
		return 0;
	}

	/* if newsize is page-aligned just set size.
	 */
	if ((newsize & POFFSET) == 0)
	{
		ip->i_size = newsize;
		SCB_MPUNLOCK_S(sidx);
		return 0;
	}

	/* find the page frame. if not in touch it.
	 */
	tsid = ITOS(sidx, pagex);
	pno = BASEPAGE(pagex);

	if ((nfr = v_lookup(tsid,pno)) < 0 || !pft_inuse(nfr)) 
	{
		(void)chgsr(TEMPSR, SRVAL(tsid, 0, 0));
		TOUCH( (TEMPSR << L2SSIZE) + (pno << L2PSIZE) );
		assert(0);
	}

	/* if the page's current allocation matches the new size,
	 * make the page readonly and set the size.  if it does
	 * not match, a mapper has stored into a partially backed
	 * page (pagex), resulting in a promotion to a full block.
	 */
	fperpage = pdt_fperpage(scb_devid(sidx));
	if (BTOFR(newsize,fperpage) == fperpage - pft_nfrags(nfr))
	{
		/* make sure the page is not already readonly.
		 */
		if (pft_key(nfr) != RDONLY)
		{
			pft_key(nfr) = RDONLY;
			P_PAGE_PROTECT(nfr, RDONLY);
		}
		ip->i_size = newsize;
	}

	SCB_MPUNLOCK_S(sidx);

	return 0;
}


/*
 * v_config(cfgp)
 *
 * sets configurable VMM values from a vmconfig structure 
 * constructed by vm_config().
 *
 * this program runs at VMM interrupt level with back-tracking
 * enabled.
 *
 * INPUT PARAMETER:	
 *
 *	cfgp	- point to vmconfig struct
 *
 * RETURN VALUE:
 *
 *	0
 */

int
v_config(cfgp)
struct vmconfig *cfgp;
{
	TOUCH(cfgp);
	TOUCH((char *)(cfgp + 1) - 1)

	/* set configurable values.
	 */
	vmker.maxpout = cfgp->maxpout;
	vmker.minpout = cfgp->minpout;

	return(0);
	
}
		
/*
 * this routine is called only from allociblk and freeiblk
 * and SR12 is mapped to .indirect of this file system.
 */
v_allociblk(sid, ppage)
int 	sid;
int	*ppage;
{
	struct indir *ptr;
	int rc, p, srval;

	FS_LOCKHOLDER(scb_devid(STOI(sid)));

	/* get addressability to the .indirect segment
	 */
	srval = chgsr(PTASR, SRVAL(sid,0,0));
	ptr = (struct indir *) PTSADDR;
	
	/* is free list empty ?
	 */
	if (ptr->free == 0)
	{
		if (rc = v_moreiblks(sid))
			goto out;
	}

	*ppage = p = ptr->free;
	assert (p && !(p & NEWBIT));
	ptr->free = ptr->indptr[p];
	rc = 0;

out:
	(void)chgsr(PTASR,srval);
	return rc;
}
	

v_freeiblk(sid, indblk)
int sid;
int indblk;
{
	struct indir *ptr;
	int srval;

	SCB_LOCKHOLDER(STOI(sid));
	FS_LOCKHOLDER(scb_devid(STOI(sid)));

	/* page release the page. free lock word.
	 * may have to wait on the page in I/O
	 */
	if (v_purge(sid, indblk))
		return VM_WAIT;

	/* get addressability to the .indirect segment
	 */
	srval = chgsr(PTASR, SRVAL(sid,0,0));
	ptr = (struct indir *) PTSADDR;

	/* assertion says that the current contents
	 * of indptr[indblk] is a disk block address.
	 */
	assert(NEWBIT & ptr->indptr[indblk]);
	ptr->indptr[indblk] = ptr->free;
	ptr->free = indblk;

	(void)chgsr(PTASR,srval);
	return 0;
}
