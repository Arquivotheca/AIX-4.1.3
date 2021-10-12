static char sccsid[] = "@(#)47	1.16  src/bos/kernel/vmm/POWER/v_fragsubs.c, sysvmm, bos412, 9445C412b 11/4/94 12:37:00";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:	v_movefrag, v_dirfrag, v_makefrag, v_relfrag,
 *		v_clrfrag, v_priorfrag
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
#include <sys/inline.h>
#include "mplock.h"

/*
 * NAME:	v_movefrag (sid, src, dest, nbytes, touch)
 *
 * FUNCTION:    copies a page or less of data from a source segment
 *		to a destination page within a persistent segment,
 *		allocating disk fragments for pages within the
 *		persistent segment as required by the copy operation.
 *
 *		only the last partial direct page may have less than
 *		a full blocks worth of fragments.  if the destination
 *		page is beyond the last partially backed direct page, 
 * 		the last page's allocation is extended to a full block.
 *
 *		v_movefrag() updates information in exception structure
 *		of the process' mst to indicate the progress made.  this
 *		information is also updated by v_movep() and the page
 *		fault handler and is used by the various VMM copy routines
 *		to determine the amount of data copied in the event of an
 *		exception.
 *
 *              this routine runs as a back-tracking VMM critical section
 *		and is called by the JFS through vcs_movefrag().
 *
 * PARAMETERS:
 *
 *	sid	- relative segment id
 *	sp	- pointer to vmsrclist struct describing the source
 *	dest	- starting destination adress
 *	nbytes	- number of bytes to move
 *	touched	- address of uint work area initialized to zero.
 *
 * RETURN VALUES:
 *
 *	0	- success
 *	ENOSPC 	- insufficient free disk resources.
 *	EDQUOT 	- user or group at quota limit.
 *	VM_WAIT	- process must wait.
 *
 */

v_movefrag(sid,sp,dest,nbytes,touched)
uint sid;
struct vmsrclist *sp;
uint dest;
int nbytes;
uint *touched;
{
        int pno, pagex, fperpage, lastp, lastf, mask, o, n;
	int sidx, nfrags, nfr, rc, i;
	uint src, src2, s, ssid;
	struct gnode *gp;
	struct inode *ip;
	struct mstsave *mst = CSA->prev;
	int msid, mpno;

	/* update the exception structure with starting source
	 * address.
	 */
	mst->except[EORGVADDR] = dest;

	/* get sidx.
	 */
	sidx = STOI(sid);

	SCB_LOCKHOLDER(sidx);

	/* get gnode and inode pointes and make sure the inode
	 * is in memory.
	 */
	gp = scb_gnptr(sidx);
	ip = GTOIP(gp);
	TOUCH(ip);
	TOUCH((char *)(ip + 1) - 1);

	/* get the relative and absoulte page numbers of the
	 * destination page.
	 */
	pno = (dest & SOFFSET) >> L2PSIZE;
	pagex = SCBPNO(sid,pno);

	/* get the page number and number of fragments for
	 * the last page of the file.
	 */
	fperpage = pdt_fperpage(scb_devid(sidx));
        lastp = BTOPN(ip->i_size);
	lastf = BTOFR(ip->i_size,fperpage);

	/* extend the last page's allocation to a full block if
	 * the destination page is beyond the last page and the
	 * last page is partially backed.
	 */
	if (pagex > lastp && lastp < NDADDR && lastf != fperpage) 
	{
		if (rc = v_makefrag(BASESID(sid),lastp,fperpage))
		{
			return(rc);
		}
	}


	/* to meet atomicity requirements, both the destination page and
	 * the source pages (1 or 2) must be accessed within this VMM
	 * back-tracking critical section.  however, the current mmap()
	 * implementation does not allow alias pages and their backing pages
	 * to be concurrently addressable.  as a result, we must detect the
	 * situation where the pages of the source are aliased to the
	 * destination page to prevent from back-tracking forever.  when
	 * detected, the passed *touched work area is updated to reflect
	 * which pieces of the source are alias pages and these pages are no
	 * longer accessed through their alias address but through their
	 * backing address (i.e. the copy operation will be destination page
	 * to destination page).
	 *
	 * this code should be removed if the mmap() implementation ever
	 * supports full page aliasing.
	 *
	 */

	/* make sure the work area is in memory.
	 */
	TOUCH(touched);

	/* check if the source is an mmap() segment and the destination
	 * file is mapped before preceeding with any more checks.
	 */
	ssid = SRTOSID(mfsri(sp->vecs[0].iov_base));
	ASSERT(!(scb_mseg(STOI(ssid)) && sp->nvecs > 1));
	if (scb_mseg(STOI(ssid)) && (gp->gn_mwrcnt || gp->gn_mrdcnt))
	{
		/* get source addresses.
		 */
		src = sp->vecs[0].iov_base;
		src2 = (src + sp->vecs[0].iov_len - 1) & ~POFFSET;
		src2 = ((src >> L2PSIZE) == (src2 >> L2PSIZE)) ? src : src2;

		for (s = src, mask = 0x1; ; mask <<= 1, s = src2)
		{
			/* skip the source page if it was previously
			 * found to be aliased to the destination page.
			 */
			if ((*touched & mask) == 0)
			{
				/* update the mst exception information
				 * with the address to touched and touch
				 * the source address.
				 */
				mst->except[EORGVADDR] = s;
				TOUCH(s);

				/* check if the source page is aliased to
				 * the destination page.  if so, update
				 * *touched. v_lookup may fail for alias
				 * addresses but ONLY for alias addresses.
				 */
				msid = SRTOSID(mfsri(s));
				mpno = (s & SOFFSET) >> L2PSIZE;
				nfr = v_lookup(msid,mpno);
				if (nfr < 0 ||
					(pft_ssid(nfr) == sid &&
					pft_pagex(nfr) == pagex))
					*touched |= mask;
			}
			if (s == src2)
				break;
		}
	}
	else
	{
		/* source pages are not aliased to the destination
		 * page.
		 */

		/* set exception address and touch source pages.
		 */
		for (i = 0; i < sp->nvecs; i++)
		{
			/* set exception address and touch first byte of
			 * for this element of the vmsrclist.
			 */
			mst->except[EORGVADDR] = src = sp->vecs[i].iov_base;
			TOUCH(src);
			
			/* set exception address and touch first byte of
			 * next page if this element of the vmsrclist ends
			 * in the next page.
			 */
			src2 = (src + sp->vecs[0].iov_len - 1) & ~POFFSET;
			if ((src >> L2PSIZE) != (src2 >> L2PSIZE))
			{
				/* update the mst exception information
				 * with the address to touched prior to
				 * the touch.
				 */
				mst->except[EORGVADDR] = src2;
				TOUCH(src2);
			}
		}
	}

	/* now onto the destination page.
	 */
	mst->except[EORGVADDR] = dest;

	/* check if fragments are applicable to the destination page
	 * (i.e. the page is within the direct block range and is
	 * the last page of the file or beyond the last page).
	 */
	if (pagex < NDADDR && pagex >= lastp)
	{
		/* determine the number of fragments required for
		 * the destination page to cover the copy.
		 */
		nfrags = BTOFR((dest & POFFSET) + nbytes,fperpage);

		/* check if the destination page is the last page and
		 * has sufficient fragments to cover the copy.  if so,
		 * make sure that the page is in memory and writeable.
		 * otherwise, allocate enough new fragments for the page
		 * to cover the copy operation.
		 */
		if (pagex == lastp && nfrags <= lastf)
		{
			TOUCH(dest);
			nfr = v_lookup(sid,pno);
			assert(nfr >= 0 && pft_dblock(nfr) != 0);

			/*
			 * We want to raise the protection on the page but we
			 * only know the correct key for the base address.
			 * Entering the base address using STARTIO causes
			 * other aliases to be removed so that a re-fault gets
			 * the correct protection.  This is more efficient than
			 * simply removing all page table entries since we
			 * avoid the fault for the base address and since no
			 * cache flush occurs (the page is read-only and thus
			 * is not modified).
			 */
               		if (pft_key(nfr) == RDONLY)
			{
               			pft_key(nfr) = FILEKEY;
				P_ENTER(STARTIO,sid,pno,nfr,pft_key(nfr),
					pft_wimg(nfr));
			}
		}
		else
		{
			/* allocate the new fragments.
			 */
        		if (rc = v_makefrag(BASESID(sid),pagex,nfrags))
			{
               			return(rc);
			}
		}
	}

	/* if the source page(s) are not aliased to the destination
	 * page simply copy the data.  otherwise, perform destintation
	 * page to destination page copies as appropriate.
	 * ASSUMES THAT PING PONG IS NOT USED ON SMP
	 */
	if (*touched == 0)
	{
		for (i = 0; i < sp->nvecs; i++)
		{
			bcopy(sp->vecs[i].iov_base,dest,sp->vecs[i].iov_len);
			dest += sp->vecs[i].iov_len;
		}
	}
	else
	{
		ASSERT(sp->nvecs == 1);

		/* if the first page of the source is aliased to 
		 * destination page copy from the destination page
		 * to destination page.  otherwise, copy from the
		 * source page.
		 */
		o = src & POFFSET;
		n = MIN(nbytes,PSIZE - o);
		s = (*touched & 0x1) ? (dest & ~POFFSET) + o : src;
		bcopy(s,dest,n);

		/* if the source range spanned a page boundry, handle
		 * the copy from the second page of the source, possibly
		 * performing a destination to destination copy.
		 */
		if (src != src2)
		{
			s = (*touched & 0x2) ? dest : src2;
			bcopy(s,dest + n,nbytes - n);
		}
	}

	/* make the page readonly if the file is mapped for update
	 * and the page is now partially backed.
	 */
	if (gp->gn_mwrcnt && BTOFR(ip->i_size,fperpage) < fperpage)
	{
		nfr = v_lookup(sid,pno);
		assert(nfr >= 0);
               	pft_key(nfr) = RDONLY;
		P_PAGE_PROTECT(nfr, RDONLY);
	}

	return(0);
}

/*
 * NAME:	v_dirfrag (sid, pno, nfrags)
 *
 * FUNCTION:    allocate fragments for a page within a persistent segment
 *		of a directory.  the page must be within the direct block
 *		range.
 *
 *		v_difrag() calls v_makefrag() to do the work and simply
 *		exists to handle VM_WAIT returns from v_makefrag().
 *		
 *              this routine runs as a back-tracking VMM critical section
 *		and is called by the JFS through vcs_dirfrag().
 *
 * PARAMETERS:
 *
 *	sid	- base segment identifier.
 *	pno	- page number within logical segment.
 *	nfrags	- number of fragments required for the page.
 *
 * RETURN VALUES:
 *
 *	0	- success
 *	ENOSPC 	- insufficient free disk resources.
 *	EDQUOT 	- user or group at quota limit.
 *	VM_WAIT	- process must wait.
 *
 */

int 
v_dirfrag(sid,pno,nfrags)
int sid;
int pno;
int nfrags;
{
	int rc;
			
	SCB_LOCKHOLDER(STOI(sid));

	/* call v_makefrag() to allocate the required number of
	 * fragments.  wait the process if appropriate.
	 */
	rc = v_makefrag(sid,pno,nfrags);

	return(rc);
}

/*
 * NAME:	v_makefrag (sid, pagex, nfrags)
 *
 * FUNCTION:    allocate fragments for a page in a persistent segment. the
 *		page must be within the direct block range and have less
 *		than nfrags allocated or be a compressed page.
 *
 *		if the page is currently backed by disk resources, the page
 *		will be made memory resident.  otherwise, a zero filled
 *		frame will be allocated for the page if one currently does
 *		not exist.  new fragments will be allocated for the page such
 *		that it is backed by nfrag fragments.  the page's xpt entry
 *		and, if appropriate, pft entry and lockword will be updated
 *		with the new disk address.
 *
 *		for journalled segments, a lockword will be allocated for
 *		the portion of the page that is currently covered by a
 *		committed fragment allocation.  the lockword is required
 *		because the disk allocation policies move pages with
 *		committed allocations to new sets of fragments.
 *
 *		on exit, the page is read/write and modified.
 *
 *              this routine runs as a back-tracking VMM critical section.
 *
 * PARAMETERS:
 *
 *	sid	- base segment identifier.
 *	pno	- page number within logical segment.
 *	nfrags	- number of fragments required for the page.
 *
 *
 * RETURN VALUES:
 *
 *
 *	0	- success
 *	ENOSPC 	- insufficient free disk resources.
 *	EDQUOT 	- user or group at quota limit.
 *	VM_WAIT	- process must wait.
 *
 */

int 
v_makefrag(sid,pno,nfrags)
int sid;
int pno;
int nfrags;
{
        int rc, sidx, nfr, lw, pdtx, fperpage, sid1, pno1, len;
	union xptentry *xpt, *v_findiblk();
	uint vaddr;
	frag_t old;
	int sidio, pnoio;
	int xptfragptr;

	/* get sidx, paging device table index and fragment per page.
	 * also calculate sid1 and pno1.
	 */
	sidx = STOI(sid);

	SCB_LOCKHOLDER(sidx);

	sid1 = ITOS(sidx, pno);
	pno1 = BASEPAGE(pno);
	pdtx = scb_devid(sidx);
	fperpage = pdt_fperpage(pdtx);

	/* the page must be within the direct block range.
	 * or it must be in a compressed segment.
	 */
	assert(pno < NDADDR || scb_compress(sidx));

	/* Protect state of .indirect pages.
	 */
	FS_MPLOCK_S(pdtx);

	/* get xpt entry for pno and pno's old (current) allocation.
	 */
	xpt = v_findiblk(sidx,pno);
	assert(xpt->word == 0 || fperpage - xpt->fptr.nfrags < nfrags);
	old = xpt->fptr;

	/* check if pno has an old allocation or is in memory.
	 * if so, touch the page to make sure that it is in
	 * an inuse state and mapped at (sid1,pno1).
	 */
        nfr = v_lookup(sid1,pno1);
	if (old.addr || nfr >= 0)
	{
		/* make sure that page is in memory.
		 */
		(void)chgsr(TEMPSR, SRVAL(sid1, 0, 0));
        	vaddr = SR13ADDR + (pno1 << L2PSIZE);
		TOUCH(vaddr);
	}
       
	/* if pno is not backed by a frame one will be allocated
	 * below, so we must make sure that a free frame exists.
	 */
        if (nfr < 0) 
	{
		assert(old.addr == 0);
		if (rc = v_spaceok(sidx, 1))
		{
			FS_MPUNLOCK_S(pdtx);
			return(rc);
		}
	}

	LW_MPLOCK_S();

	/* if pno is journaled or deferred update get the
	 * the lock word for pno, if one exists.
	 */
	if (scb_jseg(sidx) || scb_defseg(sidx))
	{
		if (rc = v_getlw(sid1,pno1,&lw))
		{
			LW_MPUNLOCK_S();
			FS_MPUNLOCK_S(pdtx);
			return(rc);
		}
	}

	/* allocate the required number of fragments for pno.
	 */
	if (rc = v_falloc(sidx,pno,xpt,nfrags))
	{
		LW_MPUNLOCK_S();
		FS_MPUNLOCK_S(pdtx);
		return(rc);
	}

	/* We get xpt->fragptr in a local variable so that we can
	 * safely release the FS lock and not fault anymore (i.e.
	 * due to pageout of .indirect).
	 */
	xptfragptr = xpt->fragptr;

	/* if pno is a journaled page and had a committed allocation,
	 * allocate a lockword to cover the committed portion of the page.
	 * Moved here so that we can release the lw lock earlier.
	 */
	if (scb_jseg(sidx) && old.addr && old.new == 0)
	{
		assert(old.addr != xpt->fptr.addr);
		len = (fperpage - old.nfrags)*(PSIZE/fperpage);
		rc = v_gettlock(sid1,vaddr,len);
		assert(rc == 0);
		lw = v_findlw(sid1,pno1);
		ASSERT(lw);
	}

	LW_MPUNLOCK_S();
	FS_MPUNLOCK_S(pdtx);
	
	/* if pno is not backed by a real page, allocate one and
	 * initialize it.  otherwise, make sure the existing page
	 * is read/write and marked as modified.
	 */
	if (nfr < 0)
	{
		/* allocate a page.
		 */
		nfr = v_getframe(sid1,pno1);

		/* Record correct protection key for the page.
		 * Insert the page at its I/O address and initialize the page.
		 */
		pft_key(nfr) = FILEKEY;
		sidio = IOSID(sid1);
		pnoio = pno1;
		P_ENTER(STARTIO,sidio,pnoio,nfr,pft_key(nfr),pft_wimg(nfr));

		/* put the page on scb list.
		 * before setting pft_pagein to avoid 
		 * inserting on the scb iolist.
		 */
		v_insscb(sidx,nfr);
		pft_pagein(nfr) = 1;

		/* if this is a journaled segment, mark the new page
		 * as journaled and writeable to its home location.
		 */
		if (scb_jseg(sidx))
		{
			pft_journal(nfr) = 1;
			pft_homeok(nfr) = 1;
		}

		/* zero the page.
		 */
		ZEROPAGE(sidio,pnoio);

		/* Insert page at normal address.
		 * This also clears the modified bit so we need to
		 * turn it back on.
		 */
		pft_pagein(nfr) = 0;
		P_ENTER(IODONE,sid1,pno1,nfr,pft_key(nfr),pft_wimg(nfr));
		pft_inuse(nfr) = 1;
		SETMOD(nfr);
	}
	else
	{
		/*
		 * We want to raise the protection on the page but we only
		 * know the correct key for the base address.  Entering the
		 * base address using STARTIO causes other aliases to be
		 * removed so that a re-fault gets the correct protection.
		 * This is more efficient than simply removing all page table
		 * entries since we avoid the fault for the base address and
		 * since no cache flush occurs .
		 */
		if (pft_key(nfr) == RDONLY)
		{
			pft_key(nfr) = FILEKEY;
			P_ENTER(STARTIO,sid1,pno1,nfr,pft_key(nfr),
				pft_wimg(nfr));
		}

		/* mark the page as modified so it will be written
		 * to disk.
		 */
		SETMOD(nfr);
	}

	/* if a lockword exists for the page, update the home address
	 * with the newly allocated fragments.
	 */
	if ((scb_jseg(sidx) || scb_defseg(sidx)) && lw)
	{
		assert(old.addr != 0);
		lword[lw].home = xptfragptr;

		/* if an extension memory address exists for pno, we
		 * don't want to update the disk address or devid in the
		 * pft:  the pft must describe the extension memory
		 * address.
		 */
		if (lword[lw].extmem != 0)
			goto finish;
	}

	/* set the devid and disk address fields in the pft.  set
	 * the newbit to indicate new allocation.
	 */ 
	pft_devid(nfr) = pdtx;
	pft_dblock(nfr) = xptfragptr;
	pft_newbit(nfr) = 1;

	finish:

	/* update maxvpn to reflect the page.
	 */
       	scb_maxvpn(sidx) = MAX(pno,scb_maxvpn(sidx));

	return(0);
}

/*
 * NAME:	v_relfrag (sid, pno, nfrags, oldfrags) 
 *
 * FUNCTION:    release fragments from a persistent page's current
 *		allocation and update oldfrags with released fragments.
 *		the page must be within the direct block range and nfrags
 *		must be non-zero.
 *
 *		the page's pft entry and xpt entry are updated to reflect
 *		the truncation of the page's current allocation.  the inode
 *		size is expected to reflect the current inode geometery.
 *
 *              this routine runs as a back-tracking VMM critical section
 *		and is called by the JFS through vcs_relfrag() with 
 *		scb_combit set for the segment.
 *
 * PARAMETERS:
 *
 *	sid	 - base segment identifier.
 *	pno	 - page number.
 *	nfrags	 - number of fragments to keep.
 *	oldfrags - result pointer to the fragment released.
 *
 * RETURN VALUES:
 *
 *	0	- ok
 *
 */

v_relfrag(sid,pno,nfrags,oldfrags)
int sid;
int pno;
int nfrags;
frag_t *oldfrags;
{
	union xptentry *xpt, *v_findiblk();
	int nfr, fperpage, newnfrags;
	struct inode *ip;
	uint sidx;

	/* page should not be beyond the direct range.
	 */
	assert(pno < NDADDR);
	assert(nfrags > 0);

	/* get sidx.
	 */
	sidx = STOI(sid);

	SCB_LOCKHOLDER(sidx);

	assert(scb_pseg(sidx));
	assert(!scb_defseg(sidx) && !scb_jseg(sidx));

	/* get inode pointer and make sure that the inode
	 * and oldfrags is in memory.
	 */
	ip = GTOIP(scb_gnptr(sidx));
	TOUCH(ip);
	TOUCH((char *)(ip + 1) - 1);
	TOUCH(oldfrags);

	/*
	 * Acquire FS lock to serialize with pageout of .indirect.
	 */
	FS_MPLOCK_S(scb_devid(sidx));

	/* get the xpt for the page.
	 */
	xpt = v_findiblk(sidx,pno);
	assert(xpt->fptr.addr != 0);

	/* determine the new nfrags value for the page.
	 */
	fperpage = pdt_fperpage(scb_devid(sidx));
	newnfrags = fperpage - nfrags;

	/* get page's frame number if it is memory resident.
	 */
	if ((nfr = v_lookup(sid,pno)) >= 0)
	{
		assert(pft_dblock(nfr) == xpt->fragptr);

		/* mark the page for discard if the page is in a pagein
		 * state.  the page's allocation should be changed only
		 * when the page is inuse and it will be touched below
		 * to insure that it is inuse.  however, we don't want
		 * to touch pagein pages since we may get exceptions on
		 * these pages, so we just discard these pages.
		 */
		if (pft_pagein(nfr))
		{
			pft_discard(nfr) = 1;
		}
		else
		{
			/* touch the page.
			 */
			(void)chgsr(TEMPSR,SRVAL(sid,0,0));
			TOUCH((TEMPSR << L2SSIZE) + (pno << L2PSIZE));

			/* update the pft with the new nfrags value and
			 * mark the page's allocation as new.
			 */
			pft_nfrags(nfr) = newnfrags;
        		pft_newbit(nfr) = 1;

			/* make the page readonly if the file is currently
			 * mapped for update.
			 */
			if (ITOGP(ip)->gn_mwrcnt)
			{
				pft_key(nfr) = RDONLY;
				P_PAGE_PROTECT(nfr, RDONLY);
			}
		}
	}

	/* update oldfrags with the released fragments.
	 */
	oldfrags->nfrags = fperpage - (newnfrags - xpt->fptr.nfrags);
	assert(oldfrags->nfrags > 0);
	oldfrags->addr = xpt->fptr.addr + nfrags;

	/* if the current allocation was old update scb_newdisk to
	 * reflect a new allocation for the segment.
	 */
	if (!xpt->newbit)
		scb_newdisk(sidx) += 1;

	/* update the xpt with the new nfrags value and mark
	 * the disk address as new.
	 */
	xpt->fptr.nfrags = newnfrags;
	xpt->newbit = 1;

	FS_MPUNLOCK_S(scb_devid(sidx));

	return(0);
}

/*
 * NAME:	v_clrfrag (sid, offset, nbytes)
 *
 * FUNCTION:    clears a portion of a page within a logical segment.
 *
 *		this routine insures that the clearing of the page will not
 *		cause a protection fault and is used by the JFS to clear
 *		partially backed file system pages as part of the truncate
 *		or clear file system operations.
 *
 *		in the presence of file mappers, partially backed pages are
 *		maintained as readonly and stores to these pages (i.e.
 *		clearing a portion of a page) normally result in protection
 *		faults and reallocation to full blocks. 
 *
 *		v_clrfrag() requires that the range to be clear (offset,
 *		offset + nbytes - 1) is wholly contained within a page
 *		within the direct block range.
 *
 *              this routine runs as a back-tracking VMM critical section
 *		and is called by the JFS through vcs_clrfrag().
 *
 * PARAMETERS:
 *
 *	sid	- base segment identifier.
 *	offset	- starting offset to clear within the logical segment. 
 *	nbytes	- number of bytes to clear.
 *
 * RETURN VALUES:
 *
 *	0	- ok 
 *
 */

v_clrfrag(sid,offset,nbytes)
uint sid;
uint offset;
int nbytes;
{
	int nfr, rdonly, pno, sidx, pagex;
	uint vaddr;

	assert((offset & POFFSET) + nbytes <= PSIZE);

	/* get sidx and page number.
	 */
	sidx = STOI(sid);
	pno = offset >> L2PSIZE;
	assert(pno < NDADDR);

	/* make the page addressable and make sure that it is
	 * in memory.
	 */
	(void)chgsr(TEMPSR, SRVAL(sid, 0, 0));
        vaddr = (TEMPSR << L2SSIZE) + offset;
	TOUCH(vaddr);

	/* locking after touch allows to not scoreboard
	 */
	SCB_MPLOCK(sidx);

	/* make the page writeable if it is readonly.
	 */
	nfr = v_lookup(sid,pno);
	assert(nfr >= 0 && pft_dblock(nfr) != 0);
	if (rdonly = (pft_key(nfr) == RDONLY))
	{
		/*
		 * We want to raise the protection on the page but we only
		 * know the correct key for the base address.  Entering the
		 * base address using STARTIO causes other aliases to be
		 * removed so that a re-fault gets the correct protection.
		 * This is more efficient than simply removing all page table
		 * entries since we avoid the fault for the base address and
		 * since no cache flush occurs (the page is read-only and thus
		 * is not modified).
		 */
		pft_key(nfr) = FILEKEY;
		P_ENTER(STARTIO,sid,pno,nfr,pft_key(nfr),pft_wimg(nfr));
	}

	/* clear the specified portion of the page.
	 */
	ZEROFRAG(sid,pno,offset & POFFSET);

	/* make the page readonly if it is currently mapped for
	 * update.
	 */
	if (rdonly)
	{
		pft_key(nfr) = RDONLY;
		P_PAGE_PROTECT(nfr, RDONLY);
	}

	SCB_MPUNLOCK(sidx);

	return(0);
}

/*
 * NAME:	v_priorfrag (sidx, pagex)
 *
 * FUNCTION:    extends the disk allocation of the last page of the 
 *		segment to a full block if the last page is partially
 *		backed by disk and is prior to pagex (i.e. the file
 *		is being extended).
 *
 *		this routine is called by the file system pager prior
 *		to allocating disk resources for pagex to insure that
 *		only the last page of the segment maybe partially backed.
 *
 *              this routine runs as a back-tracking VMM critical section.
 *
 * PARAMETERS:
 *
 *	sidx	- segment control block index.
 *	pagex	- page number of new page being added.
 *
 * RETURN VALUES:
 *
 *	0	- no extension need for the last page or extension
 *		  was successful.
 *	ENOSPC 	- insufficient free disk resources.
 *	EDQUOT 	- user or group at quota limit.
 *	VM_WAIT	- process must wait.
 *
 */

v_priorfrag(sidx,pagex)
int sidx;
int pagex;
{
	int lastp, fperpage;
	struct inode *ip;

	SCB_LOCKHOLDER(sidx);

	/* get the inode pointer and determine the last page
	 * of the file.
	 */
	ip = GTOIP(scb_gnptr(sidx));
	lastp = BTOPN(ip->i_size);

	/* no extension required if the new page is prior
	 * to the last page.
	 */
	if (pagex < lastp)
		return(0);

	/* no extension required if the last page is already
	 * backed by a full block.
	 */
	fperpage = pdt_fperpage(scb_devid(sidx));
	if (BTOFR(ip->i_size,fperpage) == fperpage) 
		return(0);

	assert(!scb_jseg(sidx));

	/* extend the last page's allocation to a full block.
	 */
	return(v_makefrag(ITOS(sidx,0),lastp,fperpage));
}

