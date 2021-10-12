/* @(#)57	1.3  src/bos/usr/include/lvmrec.h, liblvm, bos411, 9428A410j 3/4/94 17:32:14 */

#ifndef  _H_LVMREC
#define  _H_LVMREC

/*
 * COMPONENT_NAME: (LIBLVM) Logical Volume Manager - lvmrec.h
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *   This file contains the structure which describes the LVM
 *   information record.  This is physical block 7 of each hardfile.
 */

struct lvm_rec
	 /* structure which describes the physical volume LVM record */
       {
       long lvm_id;
	 /* LVM id field which identifies whether the PV is a member of
	    a volume group */
#define LVM_LVMID    0x5F4C564D   /* LVM id field of ASCII "_LVM" */
       struct unique_id vg_id;
         /* the id of the volume group to which this physical volume
            belongs */
       long lvmarea_len;
         /* the length of the LVM reserved area */
       long vgda_len;
	 /* length of the volume group descriptor area */
       daddr_t vgda_psn [2];
	 /* the physical sector numbers of the beginning of the volume
	    group descriptor area copies on this disk */
       daddr_t reloc_psn;
	 /* the physical sector number of the beginning of a pool of
	    blocks (located at the end of the PV) which are reserved for
	    the relocation of bad blocks */
       long reloc_len;
	 /* the length in number of sectors of the pool of bad block
	    relocation blocks */
       short int pv_num;
	 /* the physical volume number within the volume group of this
	    physical volume */
       short int pp_size;
	 /* the size in bytes for the partition, expressed as a power of
	    2 (i.e., the partition size is 2 to the power pp_size) */
       long vgsa_len;
	 /* length of the volume group status area */
       daddr_t vgsa_psn [2];
	 /* the physical sector numbers of the beginning of the volume
	    group status area copies on this disk */
       short int version;
	 /* the version number of this volume group descriptor and status
	    area */
#define  LVM_VERSION_1		1   /* first version - AIX 3.0 */
#define  LVM_STRIPE_ENHANCE	2   /* version with striped lv's - AIX 4.1 */
#define  LVM_MAX_VERSION	LVM_STRIPE_ENHANCE   /* max version # */
       char res1 [450];
         /* reserved area */
       };

#endif  /* _H_LVMREC */
