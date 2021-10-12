static char sccsid[] = "@(#)85	1.5  src/bos/usr/ccs/lib/libPW/xalloc.c, libPW, bos411, 9428A410j 6/16/90 00:57:42";
/*
 * COMPONENT_NAME: (LIBPW) Programmers Workbench Library
 *
 * FUNCTIONS: xalloc, xfree, xfreeall
 *
 * ORIGINS: 3 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 */

# include	"stdio.h"

/*
 * FUNCTION: Allocate/free memory.
 *
 * RETURN VALUE DESCRIPTIONS:
 *	Xalloc returns the address of the allocated area on success,
 *	fatal() on failure.
 *	Xfree and xfreeall don't return anything.
 */

/*
	xalloc/xfree based on alloc/free in C library at one time.
	Also xfreeall() frees all memory allocated (calls brk(II)).
 
	Xfree always coalesces contiguous free blocks.
	Xalloc uses a first fit strategy.
	Xalloc always allocates words (rounds up).
	Xalloc actually allocates one more word than the
	amount requested.  The extra word (the first word of the
	allocated block) contains the size (in bytes) of the entire block.
	This size is used by xfree to identify contiguous blocks,
	and is used by xalloc to implement the first fit strategy.

	Bad things will happen if the size word is changed.
	Worse things happen if xfree is called with a
	garbage argument.

	Xalloc returns the address of the allocated area on success,
	fatal() on failure.
	Xfree and xfreeall don't return anything.
*/

struct fb {
	unsigned	f_size;
	char		*f_next;
};

struct fb Freelist={
	0,
	(char *)0x3FFFFFFF,
};

char *Freeend = (char *)0x3FFFFFFF;

# define SIZINCRO	10
# define MAX_PAGE       ulimit(3,0)
# define SLOP	(sizeof(int *))

unsigned Firstbrk, Lastbrk;

xalloc(asize)
unsigned asize;
{
	register unsigned usize;
	register struct fb *np, *cp;
	int	tot = 0;
	long	asizel = 0;
	long	lastbrkl = 0;
	char	*xcp;

	if((usize = asize) == 0)
		return(0);
	if (!Firstbrk)
		Firstbrk = Lastbrk = (unsigned)sbrk(0);
	usize += 2*sizeof(int *) -1;
	usize &= ~(sizeof(int *) -1);
	for(;;) {
		cp = &Freelist;
		while((np = (struct fb *)cp->f_next) != (struct fb *)Freeend) {
			if(np->f_size>=usize) {
			/*
				Don't break the block up if it
				is not more than SLOP bigger than the
				amount needed.
			*/
				if(usize+SLOP >= np->f_size)
					cp->f_next = np->f_next;
			/*
				Break the block into 2 pieces.
			*/
				else {
					cp = (struct fb *)(cp->f_next = (char *)(((int)np) + usize));
					cp->f_size = np->f_size - usize;
					cp->f_next = np->f_next;
					np->f_size = usize;
				}
				return((int)&np->f_next);
			}
			cp = np;
		}
	/*
		Nothing on the free list is big enough;
		get more core from the operating system.
	*/
		asize = usize<1024? 1024: usize;
		asize = (asize+511) & (~511);
		asizel = asize;
		lastbrkl = Lastbrk;
		if(((asizel + lastbrkl) > MAX_PAGE) ||
			(cp = (struct fb *)sbrk(asize)) == (struct fb *)-1) {
			xcp = (char *)sbrk(SIZINCRO);
			tot = SIZINCRO;
			while(((cp = (struct fb *)sbrk(SIZINCRO)) != (struct fb *)-1) &&
				(xcp != (char *)-1)) {
				tot += SIZINCRO;
				lastbrkl = Lastbrk = ((int) cp) + SIZINCRO;
				if((SIZINCRO + lastbrkl) > MAX_PAGE) {
					break;
				}
			}
			if((!tot) || (xcp == (char *)-1))
				return(fatal("out of space (ut9)"));
			cp = (struct fb *)xcp;
			cp->f_size = tot;
		}
		else {
			Lastbrk = ((int)cp) + asize;
			cp->f_size = asize;
		}
	/*
		Add new piece to free list.
	*/
		xfree(&cp->f_next);
	}
}


xfree(aptr)
char *aptr;
{
	register struct fb *ptr, *cp, *np;

	if (aptr && aptr < (char *)Lastbrk) {
		ptr = (struct fb *) ((unsigned)aptr - sizeof(int *));
		cp = &Freelist;
		while((np = (struct fb *)cp->f_next) < ptr)
			cp = np;
	/*
		Try to coalesce with the following block.
	*/
		if(((int)ptr) + ptr->f_size == ((int)np)) {
			ptr->f_size += np->f_size;
			ptr->f_next = np->f_next;
			np = ptr;
		} else
			ptr->f_next = (char *)np;
	/*
		Try to coalesce with the preceding block.
	*/
		if(((int)cp) + cp->f_size == ((int)ptr)) {
			cp->f_size += ptr->f_size;
			cp->f_next = ptr->f_next;
		} else
			cp->f_next = (char *)ptr;
	}
}


xfreeall()
{
	brk(Firstbrk);
	Lastbrk = (unsigned)Firstbrk;
	Freelist.f_size = 0;
	Freelist.f_next = Freeend;
}
