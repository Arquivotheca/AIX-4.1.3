static char sccsid[] = "@(#)78  1.1  src/bos/kernel/ios/POWER/rmalloc.c, sysios, bos411, 9428A410j 4/14/94 14:10:17";
/*
 * COMPONENT_NAME: (SYSIOS) IO subsystem
 *
 * FUNCTIONS:   rmalloc, rmfree
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#ifdef _RSPC
 
#include <sys/inline.h>
#include <sys/param.h>
#include <sys/rheap.h>
#include <sys/systemcfg.h>

struct real_heap rh = {0};
struct real_heap *rheap = &rh;

/*
 * NAME: rmalloc
 *
 * FUNCTION: This routine allocates memory from the real heap and
 *           returns the effective address of the newly allocated buffer
 *
 * EXECUTION ENVIRONMENT:
 *           This routine is called from the process level only
 *
 * NOTES:
 *
 * RETURN VALUE DESCRIPTION: address of newly allocated buffer
 *			     NULL - couldn't allocate
 *
 * EXTERNAL PROCEDURES CALLED:
 *
 */
caddr_t 
rmalloc(int size,	/* number of bytes to allocate  */
	int align)	/* alignment requirements	*/
{

	int 		need,
                        mask,
                        slide,
                        i;
        char            found = FALSE,
                        shift,
                        num_free,
                        start_bit;

	if (!__rspc())
		/*
		 * Only supported on the RSPC class machines
		 */
		return(NULL);
	/*
	 * We can't allocate 0 bytes, and we don't support alignment
	 * to greater than page boundaries.
	 */
        if ((size == 0) || (align > 12)) return(NULL);

	/*
	 * serialize allocation
	 */
	(void) lockl(&(rheap->lock),LOCK_SHORT);

        need = (size + (PAGESIZE - 1)) / PAGESIZE;

	shift = 0;
        do {
                /*
                 * set slide to right word
                 */
                slide = rheap->free_list << shift;
                start_bit = clz32(slide);  	/* find first free resource */
                if (start_bit == (NBPW*NBPB)) {
                        /*
                         * if empty
                         */
			break;
                }
                /*
                 * shift out bits before first free
                 * while complementing -> now 1 = used, 0 = free
                 */
                slide = ~(slide << start_bit);
                shift += start_bit;
                /*
                 * count the number free in this word
                 */
                num_free = clz32(slide);
                if (num_free >= need) {
                        /*
                         * We found as many as we need
                         */
                         found = TRUE;
                } else {
                        /*
                         * Didn't find the amount we need, so 
                         * update shift and try again
                         */
                         shift += num_free;
                }
        } while (!found);
        if (found) {
                /*
                 * Mark as in use
                 */
                /*
                 * Create bit map to clear appropriate bits, setting those
                 * pages as in use
                 */
                mask = (((uint)0xFFFFFFFF << ((NBPW*NBPB) - need)) >> shift);
                /*
                 * clear appropriate bits
                 */
                rheap->free_list &= ~mask;

		unlockl(&(rheap->lock));
                return((caddr_t)(rheap->vaddr + (shift * PAGESIZE)));
        } else {
		unlockl(&(rheap->lock));
                return(NULL);
	}
}

/*
 * NAME: rmfree
 *
 * FUNCTION: This routine frees memory previously allocated from the 
 *	     real heap.
 *
 * EXECUTION ENVIRONMENT:
 *           This routine is called only on the process level.
 *
 * NOTES:
 *
 * RETURN VALUE DESCRIPTION: 0  = successfully freed
 *			     -1 = could not free/ not allocated
 *
 * EXTERNAL PROCEDURES CALLED:
 *
 */
int	
rmfree(caddr_t ptr,	/* specifies address of area to free */
       int     size)	/* specifies size of area to free    */	
{
	int	start_page, pages, mask; 

	if (!__rspc())
		/*
		 * Only supported on RSPC class machines
		 */
		return(-1);

	if ((rheap->size == 0) || (ptr < rheap->vaddr) || 
				(ptr > (rheap->vaddr + rheap->size)))
		/*
		 * not in the real heap
		 */
		return(-1);

	/*
	 * serialize allocation
	 */
	(void) lockl(&(rheap->lock),LOCK_SHORT);
	/*
	 * calculate starting page number
	 */
	start_page = ((int)ptr - (int)rheap->vaddr) / PAGESIZE;
	/*
	 * calculate number of pages to free
	 */
        pages = (size + (PAGESIZE-1)) / PAGESIZE;
	/*
	 * Create bit map to set appropriate bits, marking those
	 * pages as free
	 */
	mask = (((uint)0xFFFFFFFF << ((NBPW*NBPB) - pages)) >> start_page);
	/*
	 * mark appropriate bits as free
	 */
	rheap->free_list |= mask;
	unlockl(&(rheap->lock));
	return(0);
}

#endif /* _RSPC */
