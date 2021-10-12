/* @(#)18	1.13  src/bos/kernel/sys/lvdd.h, sysxlvm, bos411, 9428A410j 3/4/94 17:21:22 */

#ifndef _H_LVDD
#define _H_LVDD

/*
 * COMPONENT_NAME: (SYSXLVM) Logical Volume Manager - 18
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1990
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * defines for Logical Volume Device Driver ext parameter for readx and
 * writex system calls and structure definition for XLATE ioctl.
 *
 * WRITEV, is passed through to the underlying disk driver.  HWRELOC and
 * UNSAFEREL are here to allow a consistent interface with the entire
 * class of disk device drivers.  Their use is invalid with the logical
 * volume manager.
 */

#define	LVDD_RSVD  0xf0000000	/* The upper 4 bits are used internally	*/

#define WRITEV	   0x0001	/* for this request perform physical	*/
				/* write verification			*/

#define HWRELOC	   0x0002	/* for this request perform hardware	*/
				/* bad block relocation			*/

#define UNSAFEREL  0x0004	/* for this request perform bad block	*/
				/* relocation even if unsafe		*/

#define RORELOC	   0x0008	/* for this request perform READ ONLY	*/
				/* bad block relocation			*/
				/* ie. relocate only existing defects	*/ 

#define	NO_MWC	   0x0010	/* only valid on writex call to inhibit	*/
				/* normal mirror write consistency	*/
				/* rules.				*/

#define	MWC_RCV_OP 0x0020	/* Set on readx call to indicate this	*/
				/* request is doing a mirror write	*/
				/* recovery operation. i.e. a resync	*/
				/* of one LTG				*/

#define RESYNC_OP  0x0080	/* for this request resync partition 	*/
				/* logical track group(LTG)		*/
 
#define AVOID_C1   0x0100	/* this request avoid the copy 1 (primary)  */
#define AVOID_C2   0x0200	/* this request avoid the copy 2 (secondary)*/
#define AVOID_C3   0x0400	/* this request avoid the copy 3 (tertiary) */

#define	AVOID_SHFT	8	/* number of places to shift to align AVOID */
				/* mask if built dynamically		    */

#define AVOID_MSK  (AVOID_C1 | AVOID_C2 | AVOID_C3)  /* Mask for avoid bits */

#define NUMCOPIES 3		/* max number of copies per logical part. */


/* arg structure passed in for Logical Volume Device Driver XLATE ioctl - 
 * returns the physical dev_t & physical block number for the logical block 
 * number & mirror copy specified.
 */

struct xlate_arg {

	int lbn;		/* logical block number to translate	*/
	int mirror; 		/* which copy to return pbn for:	*/
				/*		1=copy 1 (primary)	*/
				/*		2=copy 2 (secondary)	*/
				/*		3=copy 3 (tertiary)	*/
	dev_t p_devt;		/* physical dev_t (major/minor of disk)	*/
	int pbn;		/* physical block number on disk	*/
};


struct lpview {
	
	int 	lpnum;			/* logical partition number */
	struct  lview {
	   short pvnum;			/* physical volume number */
	   unsigned short ppnum;	/* physical partition number */
	   char  ppstate;		/* state of the physical partition */
           char  res[3];		/* reserved/padded space */
	} copies[NUMCOPIES];
};


/* logical volume driver io control commands */
#define LVIOC		('v'<<8)
#define	XLATE		(LVIOC|1)	/* translate lbn->pbn		    */
#define	GETVGSA		(LVIOC|2)	/* get a copy if the memory version */
					/* of the VGSA			    */
#define CACLNUP		(LVIOC|3)	/* Clean up the MWC cache and write */
					/* it to all PVs in the VG	    */
#define LP_LVIEW	(LVIOC|4)	/* send a logical view of a lp back */
#define PBUFCNT		(LVIOC|5)	/* increase pbuf pool size */

#endif /* _H_LVDD */
