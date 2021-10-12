static char sccsid[] = "@(#)22  1.1  src/bos/kernel/vmm/POWER/vmpm.c, sysvmm, bos41J, 9513A_all 3/17/95 14:11:57";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS: 	pm_free_memory, pm_set_bitmap, pm_check_overflow
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "vmsys.h"
#include <sys/inline.h>
#include <sys/errno.h>
#include <sys/syspest.h>

/*
 * Global variable to record whether an overflow fault
 * occurs during hibernation.
 */
int pm_reloads = 0;

/*
 * pm_free_memory(int newfree)
 *
 * FUNCTION
 *
 * Routine to support Power Management hibernation state by performing
 * page-replacment to free up to 'newfree' page frames.
 *
 * INPUT PARAMETERS
 *
 * newfree	is the total number of free page frames desired
 *
 * EXECUTION ENVIRONMENT
 *
 * The caller must be running at process level at priority INTBASE.
 *
 * RETURN VALUES
 *
 * Returns the total number of free pages.  This value may be less than that
 * requested by 'newfree' in the case that page-replacement couldn't find
 * enough pages to free.
 */

pm_free_memory(int newfree)
{
	int rc;

	ASSERT(csa->prev == NULL && csa->intpri == INTBASE);

	/*
	 * Perform page replacement to attempt to free enough
	 * memory so that 'newfree' frames are free.
	 * We attempt to steal just file pages but if there aren't
	 * enough to satisfy the request then we'll steal any class
	 * of page.  We assume that the system is quiesced at this
	 * point such that all modified filesystem data has been sync'd
	 * and no modified filesystem pages exist in memory.  This is
	 * important since page-replacement will skip pages already in
	 * I/O state (and thus not free them).  This is required since
	 * we only wait for flbru I/O to complete and in order to ensure
	 * a stable free list we cannot have any pending I/O.
	 */
	if (newfree < vmker.numperm + vmker.numfrb)
	{
		(void) vcs_hfblru(newfree, LRU_FILE);
	}

	/*
	 * If there weren't enough file pages to satisfy the request or
	 * if there were and we still didn't achieve the target free frames
	 * then steal any class of page.
	 */
	if (newfree > vmker.numfrb)
	{
		(void) vcs_hfblru(newfree, LRU_ANY);
	}

	/*
	 * Wait for all page replacement I/O to complete so the
	 * frames show up on the free list and so the free list
	 * is stable.
	 */
	vcs_lruwait();

	/*
	 * Return the current number of free frames.
	 */
	return(vmker.numfrb);
}

/*
 * pm_set_bitmap(void *bitmap, int bitmapsize)
 *
 * FUNCTION
 *
 * Routine to support Power Management hibernation state by setting
 * a bit-map indicating which page frames are free (a '1' in the map
 * means the frame is free) and need not be preserved as part of the
 * hibernation image.
 *
 * INPUT PARAMETERS
 *
 * bitmap	is a pointer to the bitmap which is word aligned
 * bitmapsize	is the size of the bitmap in number of bits (# of 4KB frames)
 *		and which is an even number of words (i.e. a multiple of 32)
 *
 * EXECUTION ENVIRONMENT
 *
 * The caller must be running in the interrupt environment.
 * This function must reside in the pinned part of the kernel.
 *
 * RETURN VALUES
 *
 *      0       - success
 *	EINVAL	- invalid parameters
 */

pm_set_bitmap(void *bitmap, int bitmapsize)
{
	int rc;
	uint savevmmsr;
	uint *mapword, *firstword, *lastword;
	uint nfr, bitpos, mask;

	/*
	 * We must already be in an environment where the free
	 * list is stable.  This means no page-faults can occur
	 * and there is no pending I/O.  Thus we can do this
	 * without going into a VMM critical section.
	 */
	ASSERT(csa->prev != NULL && csa->intpri == INTMAX);

	/*
	 * Sanity check the bit map parameters.  We want to ensure
	 * that all VMM managed memory is preserved as part of the
	 * hibernation image.
	 */
	if ((int)bitmap % 4 || bitmapsize % 32 ||
	    bitmapsize < vmker.nrpages)
		return(EINVAL);

	/*
	 * First mark all bits '0' to indicate all frames are in use.
	 * The caller is assumed to deal with any bad memory or memory
	 * holes.
	 */
	firstword = (uint *)bitmap;
	lastword = (uint *)bitmap + (bitmapsize / 32) - 1;
	for (mapword = firstword; mapword <= lastword; mapword++)
		*mapword = 0;

	/*
	 * Now mark all bits corresponding to free frames a '1'.
	 * We need addressibility to the VMM data segment to
	 * access the free list.
	 */
	savevmmsr = chgsr(VMMSR,vmker.vmmsrval);
	for (nfr = pft_freefwd(FBANCH); nfr != FBANCH; nfr = pft_freefwd(nfr))
	{
		mapword = firstword + nfr / 32;
		bitpos = nfr % 32;
		mask = 1 << (31 - bitpos);
		*mapword |= mask;
	}
	(void)chgsr(VMMSR,savevmmsr);

	/*
	 * Save the current value of the reloads statistic
	 * for use by pm_check_overflow().
 	 */
	pm_reloads = vmker.reloads;

	return(0);
}

/*
 * pm_check_overflow()
 *
 * FUNCTION
 *
 * Routine to support Power Management hibernation state by checking
 * to see if a page table overflow fault occurred which might result
 * in an inconsistent state of the page table structures being saved
 * as part of the hibernation image.
 *
 * INPUT PARAMETERS
 *
 * None
 *
 * EXECUTION ENVIRONMENT
 *
 * The caller must have previously called pm_set_bitmap() and must be
 * running in the same environment required by that function.
 *
 * RETURN VALUES
 *
 *      0       - success
 *	EFAULT	- an overflow fault did occur
 */

pm_check_overflow()
{
	if (vmker.reloads != pm_reloads)
		return(EFAULT);
	else
		return(0);
}

