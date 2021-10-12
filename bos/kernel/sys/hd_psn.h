/* @(#)44	1.11  src/bos/kernel/sys/hd_psn.h, sysxlvm, bos411, 9428A410j 6/16/90 00:28:49 */
#ifndef  _H_HD_PSN
#define  _H_HD_PSN

/*
 * COMPONENT_NAME: (SYSXLVM) Logical Volume Manager - 44
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
 *   This file contains the physical sector number (PSN) layout of
 *   reserved space on the hardfile.
 */


#define  PSN_IPL_REC     0     /* PSN of the IPL record			*/
#define  PSN_CFG_REC     1     /* PSN of the configuration record	*/
/*
 * The Mirror Write Consistency(MWC) records must stay contiguous.  The
 * MWC cache is written to each alternately by the LVDD.
 */
#define  PSN_MWC_REC0    2     /* PSN of the first MWC cache record	*/
#define  PSN_MWC_REC1    3     /* PSN of the second MWC cache record	*/
#define  PSN_LVM_REC     7     /* PSN of LVM information record		*/
#define  PSN_BB_DIR      8     /* beginning PSN of bad block directory	*/
#define  LEN_BB_DIR      22    /* length in sectors of bad block dir	*/
#define  PSN_CFG_BAK     64    /* PSN of the backup config record	*/
#define  PSN_LVM_BAK     70    /* PSN of backup LVM information record	*/
#define  PSN_BB_BAK      71    /* PSN of backup bad block directory	*/
#define  PSN_NONRSRVD    128   /* PSN of first non-reserved sector	*/

#endif /* _H_HD_PSN */
