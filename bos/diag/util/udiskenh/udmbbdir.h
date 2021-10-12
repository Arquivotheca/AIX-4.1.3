/* @(#)48	1.1  src/bos/diag/util/udiskenh/udmbbdir.h, dsaudiskenh, bos411, 9435A411a 8/22/94 15:23:26 */
/*
 *   COMPONENT_NAME: DSAUDISKMNT
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef LVMBBDIR
#define LVMBBDIR
#ifndef NULL
#define NULL            ((void *) 0)
#endif

#define LVM_BBRDERR      -500    /* read error on bad block directory  */
#define LVM_BBWRERR      -501    /* write error on bad block directory */
#define LVM_PVRDRELOC    -502    /* put PV in read only relocation     */
#define LVM_BBINSANE     -503    /* bad block directory is not sane    */
#define  LVM_RELOC_LEN   256    /* length in blocks of BB reloc pool */
#define  LVM_BBPRIM      1      /* use the primary bad block directory */
#define  LVM_BBBACK      2      /* use the backup bad block directory */
#define REL_DONE	0		/* software relocation completed    */
#define REL_PENDING	1		/* software relocation in progress  */
#define REL_DEVICE	2		/* device (HW) relocation requested */
#define REL_CHAINED	3		/* relocation blk structure exists  */
#define REL_DESIRED	8		/* relocation desired-hi order bit on*/
#define LVM_STRCMPEQ    0
#define DBSIZE          512


extern struct bb_entry* end_of_chain(struct bb_entry *array_base,
                              int array_size,
                              struct bb_entry *start) ;

extern void* rdbbdir (int pv_fd);   /* file descriptor for physical volume d
evice   */
#endif


