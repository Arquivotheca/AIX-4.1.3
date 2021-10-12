static char sccsid[] = "@(#)33	1.53.1.1  src/bos/kernel/ldr/xmalloc.c, sysldr, bos41J, 9508A 2/15/95 10:34:51";
/*
 * COMPONENT_NAME: (SYSLDR) Program Management
 *
 * FUNCTIONS: pmalloc(), frmalloc(), xmalloc(), pgfree(), xmfree(),
 *            init_heap(), verify_empty_heap()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* this is a complete rewrite from scratch of xmalloc.
 * The allocator uses "quick cells" for power of two sizes less than a
 * page, and a traditional first fit allocator for pages.  The page
 * allocator could be improved if necessary.  All requests for less than
 * a page are rounded up to the next power of  two.  The heap data
 * structure contains a vector, called fr, with an entry for each
 * power of 2 size.
 *
 * This allocator must be able te retreive the size of a request at free
 * time since xmfree does not provide the size.  It does this without
 * tracking each request as follows -
 *
 * There is a vector "pages" with an entry for each page in the heap.  Each
 * entry may be in any of the states listed below in the P_ defines.  If
 * a page is in single page allocated state it may further be a container
 * for fragments all of the same size.  Thus, free can look up the page
 * containing its parameter and determine what size it is.
 *
 * Since small request are allocated from page sized pools, allocate and
 * free of small blocks is VERY fast.  Most applications should not need
 * special purpose storage allocators for small, high traffic allocation.
 *
 */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/errno.h>
#include	<sys/lock_def.h>
#include	<sys/syspest.h>
#include	<sys/trchkid.h>
#include	<sys/systm.h>
#include	<sys/intr.h>
#include	<sys/xmalloc.h>
#include	<sys/vmuser.h>

typedef	void *	heapaddr_t;

#ifdef DEBUG
/* make the individual debug hacks independent */
/* #define DEBUG1	/* clear or ruin storage to catch bugs */
#define DEBUG2	/* keep track of allocated space */
/*#define DEBUG3	/* use extra storage to catch people who use too much */
#define DEBUG4	/* count usage for measurement purposes */
#define DEBUG5	/* eliminate old construct and check calling interrupt level */
#endif
#ifdef DEBUG3
#define DEBUG2	/* prerequisite */
#endif

#ifdef	_KERNEL
	heapaddr_t	kernel_heap;
	heapaddr_t	pinned_heap;
#else
#undef DEBUG5        /* these checks only make sense for kernel compile */
#endif

#define	SANITY	0x48454150	/* The characters 'HEAP' */
#define HALFPGSIZE (PAGESIZE>>1)

/* all about alt .....
 * alt is 1 for the alternate heap, 0 for the primary heap.
 * but the alternate heap appears first in memory, immediately followed by the
 * primary heap.  thus heap[heap->alt] for the alt is heap[1], the primary.,
 * but also, heap[heap->alt] is heap[0] for the primary, again the primary
 */
struct	pages{
	uint	type:3;
	uint	size:5;		/* maximum page size 2**32 */
	uint	offset:24;      /* minimum page size in 32 bit machine 2**8*/
};
#define NO_PAGE		0xffffff	/*maximum value that fits in offset*/
#define	P_allocpage	0               /* if fragments, size is set, offset
					   is first free fragment in page */
#define	P_allocrange	1               /* size and offset not used */
#define	P_freepage	2               /* offset is page index of */
#define	P_freerange	3               /* next free page or range */
#define	P_freesize	4               /* offset is number of pages
					   in this freerange */
#define	P_freerangeend	5               /* offset is page index of
					   first page of this range */
#define	P_allocsize	6               /* offset is size of this
					   allocated range */
#define P_allocrangeend 7               /* offset is page index of
					   first page of this range */

struct	frag{			/* implies min alloc of 16 bytes */
	uint	next;		/* offset in page of next free fragment */
	uint	size;		/* amount of space on free chain from here */
	uint	nextpg;		/* page index of next page of fragments */
	uint	prevpg;		/* page index of previous page of fragments */
};

#ifdef DEBUG2
/* various stuff for keeping track of allocated space */
uint	first_time;
uint	rec_buf[1024*128];
uint	recording=1;	/* 1 means recording on - 0 is recording off */
#ifdef _KERNEL
uint	record_all=0;	/* 1 means record all heaps, 0 only kernel/pinned*/
#endif
struct heap	*rec_heap;
struct record *rec_hash[256];
#endif

uint
pmalloc(
struct	pages	*pages,
uint	num)
{
	uint	i,ip,j,k;
	i = pages[-1].offset;	/*anchor of page free list */
	if (i==NO_PAGE) return NO_PAGE;
	
	for(ip=-1;i!=NO_PAGE;ip=i,i=pages[i].offset) {
	/* Check for xmalloc page structure damaged. */
	ASSERT( (pages[i].type == P_freepage) ||
		 (pages[i].type == P_freerange) );
	switch(pages[i].type){
		case P_freepage:	
			if (num == 1) {
			pages[ip].offset = pages[i].offset;
			pages[i].type = P_allocpage;
			return i;
			}
			break;
		case P_freerange:	
			/* exact fit? */
			if (num == pages[i+1].offset){
				pages[ip].offset = pages[i].offset;
				pages[i].type=P_allocrange;
				pages[i+1].type=P_allocsize;
				if (num > 2)
					pages[i+num-1].type = P_allocrangeend;
				return i;
			}
			/* too small ? */
			if (num > pages[i+1].offset) break;
			/* allocate start of range - this guarantees that all fragments
			 * will preceed the (usually large) block of memory never used
			 * at all - and thus that fragments will be reused before
			 * allocating from the unused piece.  In effect, the allocation	
			 * must be left to right since the search for free space is
			 * left to right */
			k=pages[i+1].offset-num;	/* amount left - must be nonezero*/	
			j=i+num;
			/* j is first free page, k pages left free
			 * i is first allocated page, num pages allocated
			 */
			/* reformat remaining free space at end */
			if (k==1) pages[j].type = P_freepage;
			else {
				pages[j].type = P_freerange;
				pages[j+1].type = P_freesize;
				pages[j+1].offset = k;
				if (k>2) {
					pages[j+k-1].type = P_freerangeend;
					pages[j+k-1].offset = j;
				}
			}                    	
			/* rechain following free block to this new free block */
			pages[j].offset = pages[i].offset;
			/* rechain this new free block to previous free block */
			pages[ip].offset = j;
			/* format allocated space */
			if (num == 1) pages[i].type = P_allocpage;
			else {
				pages[i].type = P_allocrange;
				pages[i+1].type = P_allocsize;
				pages[i+1].offset = num;
				if (num > 2){
					pages[i+num-1].type = P_allocrangeend;
					pages[i+num-1].offset = i;
				}
			}
			return i;
		}/* end of switch */
	} /* end of "for" */
	return NO_PAGE;

}

int
frmalloc(
char   *area,
struct	heap	*heap,
struct	pages	*pages,
uint	size2)
{
	struct	frag	*frag;
	char   *fpage;
	uint	ipage;
	uint	size;
	uint	i,j,k;

	ipage = pmalloc(pages,1);
	if (ipage == NO_PAGE) return 1;
	frag = (void *)(area + (ipage<<PGSHIFT));
#ifdef	_KERNEL
	if (heap->pinflag && pin (frag, PAGESIZE, 0)) {
		(void) pgfree (area, heap, pages, ipage, 1);
		return 1;
	}
#endif
	heap->fr[size2] = ipage;
	pages[ipage].size = size2;
	pages[ipage].offset = 0;
	frag->nextpg = frag->prevpg = NO_PAGE;
	frag->size = PAGESIZE;
	size = (1<<size2);
	for(j=size;j < PAGESIZE;j+=size){
		frag->next=j;
		frag=(void *)((char *)frag + size);
	}
	frag->next = NO_PAGE;
	return 0;
}


#ifdef	DEBUG1
uint	xmalloc_pattern = 0x66666666;	/* for debug set memory to pattern */
uint	xmfree_pattern = 0x77777777;	/* for debug ruin free storage */
#endif
#ifdef	DEBUG2
char	trailer = 0xaa;	/* filler for space after allocated area */
#endif
void*
xmalloc(
uint	req_size,
uint	align,
heapaddr_t	heapp)
{
#ifdef DEBUG2
        int     nested_lock = 0;
#endif
	struct	heap	*heap;
	char   *area;
	struct	pages	*pages;
	struct	frag	*frag;
	char    *fpage;
	uint	ipage;
	char   *result;
	uint	size;
	uint	size2;	/*log2 of size*/
	uint	i,j,k;
#ifdef  DEBUG5
#include <sys/proc.h>
	extern struct proc	si_proc;
	ASSERT(heapp != &pinned_heap);	/* obsolete construct */
	ASSERT(heapp != &kernel_heap);	/* obsolete construct */
	ASSERT(curproc == &si_proc || csa->prev == NULL);
	ASSERT(curproc == &si_proc || csa->intpri == INTBASE);
#endif

#ifdef	_KERNEL
	TRCHKGT(HKWD_KERN_XMALLOC,req_size,align,heapp,NULL,NULL);
#endif
	heap = (struct heap *) heapp;

	if (req_size == 0) return NULL;

        /*
	 * a recursive call can be made to xmalloc() if DEBUG2 has been
	 * defined. Therefore it is necessary to determine if we already
	 * have the heap lock, so that xmalloc() can determine whether to
	 * unlock it when returning.
	 */
#ifdef DEBUG2
	if (!simple_lock_try(&(heap[heap->alt].lock)))
		if (!(nested_lock = lock_mine(&(heap[heap->alt].lock))))
			simple_lock(&(heap[heap->alt].lock));
#else
	simple_lock(&(heap[heap->alt].lock));
#endif

	ASSERT(heap[heap->alt].sanity == SANITY);
	ASSERT(heap[heap->alt-1].sanity == SANITY);
	ASSERT(align <= PGSHIFT);

	result = NULL;
	
	area = (char *)heap + heap->base;
	pages = (struct pages *)&heap[1+heap->alt]+1;
	size = req_size;
#ifdef DEBUG3
	size = size+4;	/* get enough extra space for a trailer */
#endif

	if (size > HALFPGSIZE || align == PGSHIFT) {
		size2 = PGSHIFT;
		size = (size+PAGESIZE-1) & ~(PAGESIZE-1);
		ipage = pmalloc(pages,size>>PGSHIFT);
		if (ipage == NO_PAGE) goto exit;
		result = area + (ipage<<PGSHIFT);
#ifdef	_KERNEL
		if (heap->pinflag && pin (result, size, 0)) {
			pgfree (area, heap, pages, ipage, size >> PGSHIFT);
			result = NULL;
			goto exit;
		}
#endif
		pages[ipage].size = 0;	/*mark as full page*/
#ifdef	DEBUG4
		heap->pagtot += size>>PGSHIFT;
		heap->pagused += size>>PGSHIFT;
#endif
		goto exit;
	}
	if (size <= 16) size2 = 4;
	else size2 = 32 - clz32(size-1);
	size2 = MAX(align,size2);

	size = (1<<size2);
#ifdef	DEBUG4
	heap->frtot[size2] += 1;
	heap->frused[size2] += 1;
#endif

	if (heap->fr[size2] == NO_PAGE) if (frmalloc(area,heap,pages,size2))
		 goto exit;
	
	ipage = heap->fr[size2];
	fpage = area + (ipage<<PGSHIFT) ;
	frag = (void *)(fpage + pages[ipage].offset);
	if (frag->next == NO_PAGE) {
		/* this page has only one fragment left - so use it and
		 * remove this page from the fragment page chain for this size
		 */
		ASSERT(frag->nextpg==NO_PAGE ||
			(pages[frag->nextpg].type==P_allocpage &&
			 pages[frag->nextpg].size==size2) );
		heap->fr[size2] = frag->nextpg;
		pages[ipage].offset = NO_PAGE;
		result = (void *)frag;
	}
	else {
		/* alocate the second fragment - the first is used to keep
		 * the fragment page size free and chain pointers whenever
		 * there is any space in the page
		 */
		ASSERT(
		    frag->next <= PAGESIZE-size && ((size-1)&frag->next) == 0);
		result = fpage + frag->next;
		frag->next = ((struct frag *)result)->next;
		frag->size -= size;
	}
		
exit:
		heap->amount += size;
#ifdef	DEBUG1
	{	uint	pat = xmalloc_pattern;
		uint	*w;
		extern int firstfitsize;
		if (result && (req_size != firstfitsize)) for(w=result;(char *)w<result+size;*w++=pat);
	}
#endif
#ifdef	DEBUG2
	{	char	pat = trailer;
		char	*w;
		if (result)
			for(w=(char *)result+req_size;(char*)w<result+size;w++)
			       *w = pat;
	}
#endif
#ifdef	DEBUG2
	{
		/* keep track of allocated space for debugging */
		struct record *record,*r;
		uint	i;
		void	*caller;
tryagain:
		if (recording && heap->rhash) {
		/* get callers address - platform dependent */
#ifdef _POWER
			struct	frame {
				struct frame *	next;
				int	junk;
				int	link;
				} *fp;
			fp = (struct frame *)get_stkp();
			fp = fp->next;
			caller = (void *)fp->link;
#endif
#ifdef _IBMRT
			/* in RT, callers lr is stored before arg vector */
			caller = *(((void**)(&req_size))-6);
#endif
			r = (struct record *)
				xmalloc((sizeof(struct record)),0,rec_heap);
			if (r == NULL) {
				recording = 0;
				printf("xmalloc: out of recording space");
				brkpoint();
				goto exit_debug2;
			}
			/* we remember which heap (primary or alt,
			 * e.g. kernel or pinned - in low bit of addr */
			r->addr = result+heap->alt;
			r->req_size = req_size;
			r->from = (uint)caller;
			i = ((uint)result>>16)+(uint)result;
			i = ((i>>8) + i) & 0x000000ff;
			r->next = (heap->rhash)[i];
			(heap->rhash)[i] = r;
		}
#ifdef _KERNEL
		/* in the kernel xmalloc, we don't want to record private heaps
		 * like the process private kernel heap in the main rec_hash
		 * since this leads to false indications of lost storage.
		 * It takes a kludge to get the kernel and pinned heap recording.
		 * This is it!
		 */
		else if (recording && (heap == kernel_heap || heap == pinned_heap)){
			heap->rhash = rec_hash;
			goto tryagain;
		}
#endif
	}
#endif
#ifdef DEBUG2

exit_debug2:
	if (!nested_lock)
		simple_unlock(&(heap[heap->alt].lock));
#else
	simple_unlock(&(heap[heap->alt].lock));
#endif
	return result;
}


int
pgfree(
char   *area,
struct	heap	*heap,
struct	pages	*pages,
uint		ipage,
uint		num)
{
	uint	ppage,npage;
	ASSERT(ipage<heap->numpages);
	ASSERT(ipage+num <= heap->numpages);
#ifdef	DEBUG4
	heap->pagused -= num;
#endif
#ifdef	DEBUG1
	{	/* ruin free pages */
		uint	pat = xmfree_pattern;
		uint	*w;
		uint	addr;
		if (pat != 0) {
			addr = area + (ipage<<PGSHIFT);
			for(w=addr;(char*)w<addr+(num<<PGSHIFT);*w++=pat);
			}
	}
#endif
#ifdef	_KERNEL
	if (heap->pinflag)
		unpin(area + (ipage<<PGSHIFT),num<<PGSHIFT);
	/* before paging is on, we release all pages ASAP.
	 * later, we don't since it performs better not to.
	 * It might be better yet to have an interface which	
	 * marked the pages unmodified without releasing them
	 */
	if (ps_not_defined || heap->vmrelflag)
		vm_release(area + (ipage<<PGSHIFT),num<<PGSHIFT);
#endif
	switch(pages[ipage-1].type){
	case P_freepage:
		ipage-=1;
		num+=1;
		break;
	case P_freesize:
		ipage-=2;
		num+=2;
		break;
	case P_freerangeend:
		ipage=pages[ipage-1].offset;
		num+=pages[ipage+1].offset;
		break;
	default:	/* previous is allocated */
		/*N.B. offset is uint, so NO_PAGE is a LARGE number */
		for(ppage = -1;
		    pages[ppage].offset < ipage;
		    ppage=pages[ppage].offset)
			 ;
		npage = pages[ppage].offset;
		pages[ppage].offset = ipage;
		pages[ipage].offset =npage;
	}

	/* at this point page ipages is pointed to by prev and points to next
		* num is the count of free pages at from ipage
		* we still must check for merge beyond
		*/

		npage = pages[ipage].offset;
		if (npage != NO_PAGE && ipage+num == npage) {
		num += (pages[npage].type == P_freepage) ?
			   1 : pages[npage+1].offset;
		npage = pages[npage].offset;
	}
	pages[ipage].offset = npage;
	if (num == 1) pages[ipage].type = P_freepage;
	else {
		pages[ipage].type = P_freerange;
		pages[ipage+1].type = P_freesize;
		pages[ipage+1].offset = num;
		if (num > 2 ) {
			pages[ipage+num-1].type = P_freerangeend;
			pages[ipage+num-1].offset = ipage;
		}
	}
	return 0;
}

int
xmfree(
char		*addr,
heapaddr_t	heapp)
{
	struct	heap	*heap;
	char   *area;
	struct	pages	*pages;
	struct	frag	*frag,*firstfrag;
	char   *fpage;
	uint	ipage,npage,ppage;
	int	result;
	uint	size2;	/*log2 of size*/
	uint	size;
	uint	i,j,k;
#ifdef DEBUG2
	int nested_lock = 0;
#endif

#ifdef  DEBUG5
#include <sys/proc.h>
	extern struct proc	si_proc;
	ASSERT(heapp != &pinned_heap);	/* obsolete construct */
	ASSERT(heapp != &kernel_heap);	/* obsolete construct */
	ASSERT(curproc == &si_proc || csa->prev == NULL);
	ASSERT(curproc == &si_proc || csa->intpri == INTBASE);
#endif
	
#ifdef	_KERNEL
	TRCHKGT(HKWD_KERN_XMFREE,addr,heapp,NULL,NULL,NULL);
#endif
	heap = (struct heap *) heapp;

	if (addr == NULL) return 0;
        /*
	 * a recursive call can be made to xmfree() if DEBUG2 has been
	 * defined. Therefore it is necessary to determine if we already
	 * have the heap lock, so that xmfree() can determine whether to
	 * unlock it when returning.
	 */
#ifdef DEBUG2
	if (!simple_lock_try(&(heap[heap->alt].lock)))
		if (!(nested_lock = lock_mine(&(heap[heap->alt].lock))))
			simple_lock(&(heap[heap->alt].lock));
#else
	simple_lock(&(heap[heap->alt].lock));
#endif

	ASSERT(heap[heap->alt].sanity == SANITY);
	ASSERT(heap[heap->alt-1].sanity == SANITY);

	result = 0;
	
	area = (char *)heap + heap->base;
	pages = (struct pages *)&heap[1+heap->alt]+1;
	ASSERT(addr >= area);
	if (addr < area) goto returnEINVAL;
	ipage = (addr-area)>>PGSHIFT;
	ASSERT(ipage < heap->numpages);
	if(ipage >= heap->numpages) goto returnEINVAL;
	if(pages[ipage].type > P_allocrange) goto returnEINVAL;
	size2 = pages[ipage].size;
	size = (1<<size2);
#ifdef	DEBUG2
	{
		/* keep track of allocated space for debugging */
		uint	i;
		struct record *r,*rp;
		if (recording && heap->rhash) {
			i = ((uint)addr>>16)+(uint)addr;
			i = ((i>>8) + i) & 0x000000ff;
			rp = (void *)&(heap->rhash)[i];
			r = (heap->rhash)[i];
			while(r) {
				if (r->addr == addr+heap->alt) break;
				rp = r;
				r = r->next;
				}
			if (r == NULL) {
				printf("xfree of unallocated address");
				brkpoint();
			}
			rp->next = r->next;
			{	char	pat = trailer;
				char	*w;
				for( w = (char *) addr+r->req_size;
				     (char*) w < addr+size;
				     w++)
					if (*w != pat) {
						printf(
						   "xmalloc clobbered trailer");
						brkpoint();
					}
			}
			if (xmfree((void *)r,rec_heap)) {
				printf(
				    "xmalloc allocation record keeping broken");
				brkpoint();
			}
		}
	}
#endif
	
	switch (pages[ipage].type){
		case P_allocrange:
			ASSERT((addr-area) == ipage<<PGSHIFT);
			size= (pages[ipage+1].offset << PGSHIFT);
			result =
			    pgfree(area,heap,pages,ipage,pages[ipage+1].offset);
			goto exit;
		case P_allocpage:
			if (pages[ipage].size != 0) break;
			ASSERT((addr-area) == ipage<<PGSHIFT);
			size = PAGESIZE;
			result =  pgfree(area,heap,pages,ipage,1);
			goto exit;
		default:
			result = 1;
			goto exit;
	}
	/* free of fragement - the hard case */
	ASSERT(0==((uint)addr & (size-1)));	/* is the alignment of addr
						   plausable */
	if ((uint)addr & (size-1))
		goto returnEINVAL;
	frag = (void *)addr;
	fpage = (void *)((uint)frag & ~(PAGESIZE-1));
#ifdef	DEBUG4
	heap->frused[size2] -= 1;
#endif
#ifdef	DEBUG1
	{	/* ruin free fragment */
		uint	pat = xmfree_pattern;
		uint	*w;
		if (pat != 0) {
			for(w=(void*)frag;(char*)w<(char *)frag+size;*w++=pat);
		}
	}
#endif
	if (pages[ipage].offset != NO_PAGE) {
		/* this page of fragments is already on a free chain
		 * and has at least one free fragment.  Put this new
		 * one as SECOND since the first one serves to record
		 * size and page chain pointers
		 */
		firstfrag = (void*)(fpage + pages[ipage].offset);
		frag->next = firstfrag->next;
		firstfrag->next = (uint)frag - (uint)fpage;
		firstfrag->size += size;
		if ( firstfrag->size == PAGESIZE &&
		     ( heap->fr[size2] != ipage ||
		       firstfrag->nextpg != NO_PAGE) ){
		       /* we have freed all the fragments in this
			* page.  It is not the only page on the chain.
		        */
			if(heap->fr[size2] == ipage){
				/* easy case - its first */
				ASSERT(pages[firstfrag->nextpg].type==P_allocpage &&
			 		 pages[firstfrag->nextpg].size==size2 );
				heap->fr[size2] = firstfrag->nextpg; /* not
								       NO_PAGE*/
                        }
			else {
				ppage = firstfrag->prevpg;
				npage = firstfrag->nextpg;
				/* fix up predecessor  - it must have one
				 * since its not first
				 */
				ASSERT(pages[ppage].type==P_allocpage &&
			 		 pages[ppage].size==size2 );
				fpage = area + (ppage<<PGSHIFT);
				frag = (void*)(fpage + pages[ppage].offset);
				frag->nextpg = npage;
				/* fix up sucessor if it has one */
				if (npage != NO_PAGE) {
					ASSERT(pages[npage].type==P_allocpage &&
			 			 pages[npage].size==size2 );
		    			fpage = area + (npage<<PGSHIFT);
		    			frag =
					   (void*)(fpage + pages[npage].offset);
		    			frag->prevpg = ppage;
				}
			}/* end of removing this page from the chain */
			result = pgfree(area,heap,pages,ipage,1);
		}/*end of remove case*/
	}/* end of fragment page already on free chain*/
	else {
		/* freeing first fragment of this page */
		pages[ipage].offset = (uint)frag - (uint)fpage;
		frag->size = size;
		frag->next = NO_PAGE;
		/* put this page on the front of the chain of pages of
		 * this size
		 */
		npage = frag->nextpg = heap->fr[size2];
		heap->fr[size2] = ipage;
		if ( npage != NO_PAGE ) {
			ASSERT(pages[npage].type==P_allocpage &&
	 			 pages[npage].size==size2 );
			fpage = area + (npage<<PGSHIFT);
			ASSERT(pages[npage].offset != NO_PAGE);
			frag = (void*)(fpage + pages[npage].offset);
			if (frag->size == PAGESIZE) {
				/* this was a full page left on because it
				 * was alone
				 */
				result = pgfree(area,heap,pages,npage,1);
				frag = (void*)addr; /* back to original
						       value */
				frag->nextpg = NO_PAGE;
			}
			else
				frag->prevpg = ipage;
		}
	}/* end of fragment page to be added to free chain when
	  * first fragment is freed
	  */
exit:
	heap->amount -= size;
returnresult:
#ifdef  DEBUG2
	if (!nested_lock)
		simple_unlock(&(heap[heap->alt].lock));
#else
	simple_unlock(&(heap[heap->alt].lock));
#endif
	return result;

returnEINVAL:
	result = EINVAL;
	goto returnresult;
}


heapaddr_t
init_heap(
char   *area,
uint	size,
heapaddr_t	heapp)

{
	struct	heap	*heap=(struct heap*)heapp;
	struct	heap	*altheap;
	struct	pages	*pages;
	uint	i,j,k;
	uint	numpages,hpages;
	uint	hsize;

	if (heap) {
	/* return address of secondary heap for this primary*/
	ASSERT(0==heap->alt);
#ifdef	_KERNEL
	if(heap == kernel_heap)
		heap[-1].pinflag = 1;
#endif
	return &heap[-1];
	}
#ifdef	DEBUG2
	if (first_time == 0) {
		first_time = 1;
		rec_heap = init_heap(
		        (char *)(((uint)rec_buf+PAGESIZE - 1) & ~(PAGESIZE-1)),
			((sizeof(rec_buf)) - 2*PAGESIZE) & ~(PAGESIZE-1),0);
		rec_heap->pinflag = 1;	/* pin entries so they are available
					   at dump time */
		rec_heap->rhash = NULL; /* avoid recursive recording */
	}
#endif
	ASSERT(0==((uint)area&(PAGESIZE-1)));
	ASSERT(0==(size&(PAGESIZE-1)));
	numpages = size >> PGSHIFT;
	hsize = 2*sizeof(struct heap) + (1+numpages)*sizeof(struct pages);
	hsize = (hsize+PAGESIZE-1) & ~(PAGESIZE-1);
	hpages = hsize >> PGSHIFT;
	numpages = numpages - hpages;
	altheap = (struct heap*)(area + (numpages<<PGSHIFT));
	heap = &altheap[1];
	heap->sanity = altheap->sanity = SANITY;
	heap->alt = 0;
	altheap->alt = 1;
	pages = (struct pages *)&altheap[2]+1;   /* remember +1 adds an
						    element size */
	heap -> base = (uint) area - (uint) heap;
	altheap -> base = (uint) area - (uint) altheap;
	simple_lock_init(&heap->lock);
	for(i=0;i<PGSHIFT;heap->fr[i]=altheap->fr[i]=NO_PAGE,i++);
#ifdef	DEBUG4
	for(i=0;i<PGSHIFT;heap->frtot[i]=heap->frused[i]=0,i++);
	heap->pagtot = heap->pagused = 0;
#endif
	{
	pages[-1].offset=0;
	pages[-1].type=P_allocpage;
	pages[0].offset = NO_PAGE;
	pages[0].type = P_freerange;
	pages[1].offset = numpages;
	pages[1].type = P_freesize;
	pages[numpages-1].offset=0;
	pages[numpages-1].type=P_freerangeend;
	}
	heap->numpages = altheap->numpages = numpages;
	heap->rhash = altheap->rhash = NULL;
#if defined(_KERNEL) && defined(DEBUG2)
	if (record_all)
		heap->rhash = altheap->rhash = rec_hash;
#endif
	return heap;
}

/* this interface is used by xmalloc testing code.  It is called when
 * the heap is expected to be empty ( nothing allocated) and verifys
 * that the heap has returned to its initial state.  This is tricky
 * since xmalloc defers frees of empty fragment pages.  The code
 * actually frees empty fragment pages and then checks.
 *
 * returns 0 if all is well, an error number otherwise
 */

int
verify_empty_heap(
heapaddr_t	heapp)
{
	struct	heap	*heap=(struct heap*)heapp;
	struct	heap	*altheap;
	struct	pages	*pages;
	int	i,j,k;
	uint	numpages;
	struct	frag	*frag;
	uint	ipage;
	char   *area;

	if ( heap[0].alt )
		return 1;

	area = (char *)heap + heap->base;
	pages = (struct pages *)&heap[1+heap->alt]+1;
	/* clean up fragment storage for both heaps */

	/* for each heap */
	for(i=-1;i<=0;i++){
		/* for each fragment size, verify that there is at most one page
		 * of fragments, and that that page is empty - then free it
		 */
		for(j=0;j<PGSHIFT;j++){
			if ( heap[i].fr[j] == NO_PAGE ) continue;
			ipage = heap[i].fr[j];
			if ( pages[ipage].size != j )
				return 2;
			frag = (void*)(area + (ipage<<PGSHIFT) + pages[ipage].offset);
			if (frag->size != PAGESIZE)
				return 3;
			if (frag->nextpg != NO_PAGE)
				return 4;
			pgfree(area,heap,pages,ipage,1);
			heap[i].fr[j] = NO_PAGE;
		}
	}
	
	/* now there should be no allocated pages at all - check the page array */

	
	numpages = heap->numpages;
	if ( pages[-1].offset != 0 ) return 10;
	if (pages[-1].type!=P_allocpage) return 11;
	if (pages[0].offset != NO_PAGE) return 12;
	if (pages[0].type != P_freerange) return 13;
	if (pages[1].offset != numpages) return 14;
	if (pages[1].type != P_freesize) return 15;
	if (pages[numpages-1].offset!=0) return 16;
	if (pages[numpages-1].type!=P_freerangeend) return 17;

        return 0 ; /* successful check */
}

