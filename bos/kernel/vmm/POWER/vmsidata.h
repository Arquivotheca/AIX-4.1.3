/* @(#)79       1.11.1.2  src/bos/kernel/vmm/POWER/vmsidata.h, sysvmm, bos411, 9428A410j 12/7/93 07:11:17 */
#ifndef  _h_VMSIDATA
#define  _h_VMSIDATA

/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

/*
 * data structure used in initialization of virtual memory manager.
 */

struct vmsidata
{
        /* input params */
        uint    rmapptr;        /* pointer to ram_bit_map        */
        int     rmapsize;       /* size in words of the bit map  */
        int     rmapblk;        /* bytes represented per bit     */
        dev_t   pgdev;          /* initial paging device        */
        int     pgsize;         /* size of device in 4k blocks  */

        /* variables used during si   */
        int     memsize;        /* real memory size in pages (include bad) */
        int     memround;       /* memsize rounded up to power of 2 */
        int     logmsize;       /* log base 2 of memround          */
        int     badpages;       /* number of bad pages in (0,memsize) */
        int     numfrb;         /* number of free blocks */
        uint    freemem;        /* addr next free page frame    */
};

#endif /* _h_VMSIDATA */
