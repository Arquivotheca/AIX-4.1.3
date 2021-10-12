static char sccsid[] = "@(#)37  1.2  src/bos/kernel/vmm/POWER/vmperf.c, sysvmm, bos411, 9428A410j 2/16/94 08:47:36";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:	memsize,
 *		vm_getframe, vm_freeframe, vm_isref,
 *		BF_enter, BF_remove
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "vmsys.h"

/*
 * Interfaces provided for performance tools.  These interfaces are not
 * intended to be exported -- the tools must find the entry points.
 * This module must be pinned and reside in V=R memory since it defines
 * the flag BF_on which is used by the v_reload routine.  The performance
 * tools may also depend on these routines being pinned.
 */

/*
 * memsize()
 *
 * Return the real memory size in bytes.
 */

uint
memsize()
{
	return((vmker.nrpages - vmker.badpages) * PAGESIZE);
}

/*
 * vm_getframe()
 *
 * Returns the frame number of the first frame on the free list
 * or -1 if the free list is empty.
 *
 */

int
vm_getframe()
{
	int nfr;
	
        nfr = vcs_getfree();

        return(nfr == FBANCH ? -1 : nfr);
}

/*
 * vm_freeframe(nfr)
 *
 * Inserts frame allocated via vm_getframe() back on the free list.
 */

void
vm_freeframe(nfr)
int	nfr;
{
        vcs_insfree(nfr);
}

/*
 * vm_isref(nfr)
 *
 * Returns and resets referenced status of a page frame.
 */

int
vm_isref(nfr)
int	nfr;
{
	return(ISREF(nfr));
}

/*
 * Interfaces for bigfoot tool.
 * Tool sets BF_on to non-zero value and modifies function descriptor
 * pointed to by BF_func to enable callouts to kernel extension.  A
 * dummy routine BF_kext and the function descriptor are defined in
 * assembler in vmvcs.s.
 */
int BF_on = 0;
extern BF_kext();
void (*BF_func)() = (void (*)())BF_kext;

/*
 * Callout to bigfoot tool to record page reference data.
 */
void
BF_enter(type,sid,pno,nfr,key,attr)
uint type,sid,pno,nfr,key,attr;
{
	(*BF_func)(type,sid,pno,nfr,key,attr);
}

/*
 * Interface for bigfoot tool to remove all h/w mappings of a frame.
 */
void
BF_remove(sid,pno,nfr)
int	sid;
int	pno;
int	nfr;
{
	P_REMOVE(sid,pno,nfr);
}
