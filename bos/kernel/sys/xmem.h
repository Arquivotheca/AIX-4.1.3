/* @(#)61	1.13  src/bos/kernel/sys/xmem.h, sysvmm, bos411, 9428A410j 2/1/93 21:38:29 */
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_XMEM
#define _H_XMEM

/*
 *	Memory Address Space Definitions
 * These values define the address space for kernel services requiring
 * address space information (usually via the segflag parameter). 
 */
#define USER_ADSPACE	0		/* user address space */
#define SYS_ADSPACE	1		/* system address space */
#define USERI_ADSPACE	2		/* user instruction address space */
/* value of 3 should be reserved for use by UIO_XMEM in uio.h */

/*
 *	Return Codes
 */

#define XMEM_SUCC (0)		/* successful completion	*/
#define XMEM_FAIL (-1)		/* unsuccessful completion	*/
 
/*
 *	Cross Memory Descriptor.
 *	The address space id describes what portions of memory the
 *	caller of xmemat() has the capability to access.
 *	The subspace id describes what portion of that particular
 *	address space is mapped by this descriptor.
 */

struct xmem 
{					/* cross memory descriptor */
	union {
		struct {
			short	version;
			short	flag;	/* address space of buffer */
		} s;
		int	_aspace_id;
	} _u;
#define	aspace_id	_u._aspace_id
#define	xm_flag		_u.s.flag
#define	xm_version	_u.s.version

#define XMEM_PROC   ( 0)		/* process address space */
#define XMEM_GLOBAL (-1)		/* global address space */
#define XMEM_INVAL  (-2)		/* access removed	*/
#define XMEM_PROC2  (-3)		/* process aspace, split portion */

	int	subspace_id;		/* portion of aspace for this */
	int	subspace_id2;		/* 2nd portion of aspace */
	char	*uaddr;			/* user buffer address */
};

/*
 *	Macros for cross memory attach/detach operations.
 */
#define xmemat(vaddr,count,dp) xmattach(vaddr,count,dp,SYS_ADSPACE)
#define xmemdt(dp) xmdetach(dp)
 
/*
 *	Entry points for cross memory operations.
 */
int xmattach();				/* setup for cross memory */
/*	char	*uaddr;	*/		/* user buffer address	  */
/*	int     count;	*/		/* user buffer length	  */
/*	struct xmem *dp;	*/	/* cross memory descriptor*/
/*	int     segflag;	*/	/* address space of buffer */

int xmemin();				/* cross memory copy in */
/*	char	*uaddr;	*/		/* address in other address space */
/*	char	*kaddr;	*/		/* address in global memory	*/ 
/*	int 	count;	*/		/* amount of data to copy	*/
/*	struct xmem *dp;	*/	/* cross memory descriptor	*/

 
int xmemout();				/* cross memory copy out */
/*	char	*kaddr;	*/		/* address in global memory 	*/
/*	char	*uaddr;	*/		/* address in other address space*/
/*	unsigned int count;	*/	/* amount of data to copy	*/
/*	struct xmem *dp;	*/	/* cross memory descriptor	*/
 
int xmdetach();				/* cross memory detach */
/*	struct xmem *dp;	*/	/* cross memory descriptor */


/*
 *	Entry points for kernel only cross memory operations.
 */
 
int xmemdma();				/* cross memory DMA setup */
/*	struct xmem *dp;	*/	/* cross memory descriptor */
/*	char	*uaddr;	*/		/* address in other address space */
/*	int	flags;	*/		/* operation flags */
#define	XMEM_HIDE	0x00000000	/* hide the page */
#define	XMEM_UNHIDE	0x00000001	/* unhide the page */
#define	XMEM_ACC_CHK	0x00000002	/* perform access checking */
#define	XMEM_WRITE_ONLY	0x00000004	/* intended transfer is outbound only*/
                                        /* (page can be read-only) */
/*	returns the real address */
 
int xmemacc();				/* cross memory access */
/*	struct xmem *dp;	*/	/* cross memory descriptor */
/*	char	*raddr;	*/		/* real address */

/*
 *	Return Codes from xmemacc()
 */

#define XMEM_RDONLY	0	/* readonly access allowed	*/
#define XMEM_RDWR	1	/* read/write access allowed	*/
#define XMEM_NOACC	2	/* no access allowed		*/
 
 
 
#endif /* _H_XMEM */

