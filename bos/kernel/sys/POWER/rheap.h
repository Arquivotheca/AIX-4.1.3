/* @(#)82	1.1  src/bos/kernel/sys/POWER/rheap.h, sysios, bos411, 9428A410j 4/14/94 14:19:02 */
#ifndef _H_RHEAP
#define _H_RHEAP
/*
 * COMPONENT_NAME: 	SYSIOS
 *
 * FUNCTIONS:		Contiguous Real Memory Heap
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
 */
#ifdef _RSPC

#include <sys/types.h>
#include <sys/lockl.h>
#include <sys/param.h>

#define MAX_RHEAP_PAGES	   (NBPW*NBPB)  /* # pages limited to bits/word */

struct real_heap {

	caddr_t	raddr;			/* real address of heap */
	caddr_t	vaddr;			/* virtual address of heap */
	int	size;			/* size in bytes of heap */
	lock_t	lock;			/* allocation lock */
	uint    free_list;		/* Allocation Bit Mask */
};

extern struct real_heap *rheap;

#endif /* _RSPC */

#endif /* _H_RHEAP */
