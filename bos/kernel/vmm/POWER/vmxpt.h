/* @(#)97	1.13  src/bos/kernel/vmm/POWER/vmxpt.h, sysvmm, bos411, 9428A410j 3/8/93 17:41:03 */
#ifndef _h_VMXPT
#define _h_VMXPT

/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
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
 * external page tables for a working storage segments are
 * allocated from the VMM's Page Table Area (PTA) segment.
 * this segment is only used for xpt's and is sufficient for
 * approximately 2**38 bytes of virtual addressing.
 *
 * for persistent and client segments there is no explicit xpt.
 * for persistent segment the VMM uses the file-system's inode
 * and indirect blocks that contain the disk addresses. the indirect
 * blocks of a file-system (mountable device) are accessed in its
 * own .indirect segment. the disk addresses contained in the inode
 * of open files are accessed in the inode table.
 */

/*
 * an xpt is  a tree consisting of a 1 kbyte root and up to 256
 * 1 kbyte direct blocks of xpt entry pointers. (the root has 256
 * pointers to the direct blocks and each direct block has 256 xpt
 * entries, each occupying 4 bytes).
 *
 * for working storage segments the cdaddr in an xpt is either
 * the address of a block in paging space or the index of a
 * map block in the PTA segment. in the former case the 28-bit
 * cdaddr is parsed as (pdtx,block) where pdtx is an index in the
 * paging device table and block is the block number in device.
 * pdtx is a 4-bit number (so there are at most 16-paging devices)
 * and block is a 24-bit number (so maximum disk size is 2**36 bytes)
 */

/* the VMM uses the file-system disk address fields in the
 * inodes or indirect blocks with the exception that the left
 * most bit of the fields may be set to one to indicate that
 * the disk block is new (uncommitted). this bit is not set in
 * the permanent disk copy of the indirect block or disk inode,
 * as the case may be.
 */

#define XPTCTL  0xf0000000      /* mask for control bits in xpt entry */

union xptentry
{
        struct working_xpte
        {
                unsigned  _spkey :2;    /* storage protect key */
                unsigned  _mapblk :1;   /* 0  cdaddr -> paging space */
                                        /* 1  cdaddr index of map block */
                unsigned  _zerobit :1;  /* page logically zero */
                unsigned  _cdaddr :28;  /* disk addr or index of mapblk */
        } ws;

        struct persistent_xpte
        {
                unsigned  _newbit  :1;   /* disk block is newly allocated */
                unsigned  _fragptr :31;  /* (nfrag,fragment addr) pair */
        } ps;

        frag_t  fptr;     /* 32-bit fragment address */

        uint  	word;     /* 32-bit word */
};

/* macros for addressing variables in xpte
 */
#define spkey   ws._spkey
#define mapblk  ws._mapblk
#define zerobit ws._zerobit
#define cdaddr  ws._cdaddr
#define newbit  ps._newbit
#define fragptr ps._fragptr

/* constants for xpt direct blocks and root
 */
#define XPTENT		256
#define L2XPTENT	8
#define BPERXPT1K	(XPTENT*PSIZE)

/* direct block of external page table entries.
 */
struct xptdblk
{
        union xptentry  xpte[XPTENT];
};

/* root of external page table. contains pointers to direct blocks.
 */
struct xptroot
{
        struct xptdblk  * xptdir[XPTENT];
};

/*
 * map blocks. vmapblks are used to map a range of pages of a
 * working or persistent storage segment (the source) to a working
 * storage segment (the target). a vmapblk may be shared in that there
 * may be more than one target segment. the count field in a vmapblk
 * is the sum of all xpt entries which point to it. the org fields
 * specify the beginning page numbers in the source and target segments.
 */

struct vmapblk
{
        union
        {
                int     sid;    /* sid of source segment */
                int     next;   /* next on free list     */
        } _u;
        int     torg;   /* first page in target  */
        int     sorg;   /* first page in source  */
        int     count;  /* number of xpte's pointing to it */
};


/*
 * layout of Area Page Map. there is one apm for each page of
 * a PTA segment, and the kth apm (i.e. apm[k]) describes the
 * allocation state of page k. an apm with free space on it is
 * kept on one of the anchors; each page being used for blocks
 * of the same size: 128,256,...4096 bytes. an apm with no free
 * space is not on any list. when a block is freed from a full
 * page the apm is put back on the anchor which corresponds to
 * the size of the block freed. apm[0]...apm[191] are never used
 * because the first 192 pages contain the apm array (128 pages)
 * and the xpt2 direct blocks (64 pages). free pages are described
 * by apms on the 4k anchor. when the 4k free list becomes empty it
 * is replenished  with the next page of apms. the variable hiapm
 * contains the index of the apm of this next page.
 */

struct apm
{
        uint   pmap;            /* page bit map                             */
        ushort fwd;             /* index of next APM entry                  */
        ushort bwd;             /* index of previous APM entry              */
};

/*
 * Page Table Area (PTA) defines
 */

#define APMSIZE         (sizeof (struct apm))     /* #bytes in an APM entry  */
#define APMENT          (PSIZE/APMSIZE)           /* # APM entries per page  */
#define APM4KFREE       32		/* max # of 4k free blks w/resources */
#define FIRSTAPM        192                       /* first valid APM entry   */
#define ANCHORS         6                         /* # of freelist anchors   */

/*
 * XPTxxx are indices in the anchor array in a PTA segment. we only
 * use the following 3 XPT block sizes: 128B, 1K, and 4 K. however
 * the non-used values  must NOT be deleted from the array because
 * the value of these indices are used in the code : the size in byte
 * of a block is 1 << (L2PSIZE - index) and the block number within
 * a page is offset >> (L2PSIZE - index) where offset is its offset
 * within the page.
 */

#define XPT128          5       /* 128 bytes */
#define XPT256          4       /* 256 bytes */
#define XPT512          3       /* 512 bytes */
#define XPT1K           2       /* 1Kb       */
#define XPT2K           1       /* 2Kb       */
#define XPT4K           0       /* 4Kb       */

#endif /* _h_VMXPT */
