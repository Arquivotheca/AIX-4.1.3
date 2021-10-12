/* @(#)70        1.1  src/bos/kernel/sys/POWER/cache.h, sysml, bos411, 9428A410j 7/14/93 14:03:23 */
/*
 *   COMPONENT_NAME: SYSML
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_CACHE
#define _H_CACHE
#include <sys/types.h>

#ifdef _KERNSYS
#ifdef _NO_PROTO
extern void sync_cache_page_ppc();
extern void sync_cache_page_pwr();
extern void inval_icache_page_ppc();
#else
extern void sync_cache_page_ppc(caddr_t raddr);
extern void sync_cache_page_pwr(uint sid, uint pno);
extern void inval_icache_page_ppc(caddr_t raddr);
#endif
#else /* _KERNSYS */
#ifdef _KERNEL
#ifdef _NO_PROTO
extern int vm_cflush();
extern void cache_inval();
#else
extern int vm_cflush(caddr_t eaddr, int nbytes);
extern void cache_inval(caddr_t eaddr, int nbytes);
#endif
#else /* _KERNEL */
#ifdef _NO_PROTO
extern void _sync_cache_range();
#else
extern void _sync_cache_range(caddr_t eaddr, int nbytes);
#endif
#endif /* _KERNEL */
#endif /* _KERNSYS */
#endif /* _H_CACHE */
