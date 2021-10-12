/* @(#)42	1.13.1.7  src/bos/kernel/sys/POWER/pseg.h, sysproc, bos41J, 9508A 2/16/95 17:09:25 */
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */


#ifndef	_H_PSEG
#define	_H_PSEG

/*
 *	For the RT and R2 platforms, most of the regions of the
 *	user process are packaged into a single virtual memory
 *	segment, called the process private segment.  This is
 *	convenient, given the 801 relocate hardware.  It avoids
 *	wasting lots of address space giving each region a
 *	separate segment.  Other machines might not want to use
 *	this scheme.  Nothing fundamental depends on it.
 */

#include <sys/param.h>
#include <sys/user.h>
#include <sys/seg.h>


/*	The format of the process private segment PRIVSEG is as follows:

		 _______________________			___
UDATA = 0    -> | user r/w data and text|			   |
		|  u_dsize <= u_dmax	|			   |
		 -----------------------			   |
		|	unused		|			   |
		 -----------------------			   | U_REGION
		| stack for user mode	|			   |
		|  u_ssize <= u_smax	|			   |
USTACK_TOP   ->  -----------------------   <-	ERRNO		   |
		| standard syscall errno|			   |
		 ----------------------- 			   |
		| pointer to std errno  |			   |
		 =======================			===
		| standard kernel stack |	--		   |
U_BLOCK	     ->  -----------------------  --	  |		   |
		| standard uthread block|   |	  | one pinned page|
		 -----------------------    |	  |		   | K_REGION
		| user structure	|   |	--		   |
		| swappable proc info	|   |ublock		   |
		 -----------------------    |	--		   |
		| extra uthread blocks  |   |	  | pinned as used |
KHEAP	     ->	 -----------------------  --	--		   |
		| kernel heap for loader|			   |
SEGSIZE	     ->  -----------------------			---

 *  The addresses of the u-block and of the standard errno are exported by
 *  ipl.exp  All code should use these values rather than other constants.
 *  Everything defined here is derived from them.
 */

#define U_BLOCK		((ulong)&__ublock - PRIVORG)
#define ERRNO		((ulong)&errno - PRIVORG)

#define U_REGION_SIZE	(ERRNO + 2*sizeof(int))
#define K_REGION_SIZE	(SEGSIZE - U_REGION_SIZE)

/*
 *  The following defines are calculated from the preceeding values.
 */
#define USRSTACK	(PRIVORG + U_REGION_SIZE)
#define KHEAPSIZE	(SEGSIZE - U_BLOCK - sizeof(struct ublock))

/*
 *  The following defines are offsets in a process private segment
 *  to the appropriate data area. They can be used in pointer assignment
 *  in addressing portions of the process private segment
 */
#define USTACK_TOP	ERRNO
#define KHEAP		(U_BLOCK + sizeof(struct ublock))

/*	The format of the process private segment KSTACKSEG is as follows:

		 _______________________
0	     -> |			|
		|			|
		|	unused		|
		|			|
		|			|
		 -----------------------
		|			|
		|			|
		|  extra kernel stacks	|
		|			|
		|			|
SEGSIZE	     ->  -----------------------  <-	KSTACKTOP

 */

#define STACKTOUCH	(2 * 1024)		/* don't touch next segment */
#define KSTACKTOP	(KSTACKORG + SEGSIZE - STACKTOUCH)
#define KSTACKSIZE	(96 * 1024)

/*
 * mininum initial stack frame is STKMIN. this is machine dependent.
 */
#define STKMIN		56

#endif	/* _H_PSEG */
