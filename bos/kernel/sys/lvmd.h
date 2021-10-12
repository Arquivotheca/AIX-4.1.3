/* @(#)09	1.6  src/bos/kernel/sys/lvmd.h, liblvm, bos411, 9428A410j 6/16/90 00:31:49 */
#ifndef _H_LVMD
#define _H_LVMD

/*
 * COMPONENT_NAME: (SYSXLVM) Logical Volume Manager - 09
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *   This file contains the structures which are used by the LVM
 *   daemon to communicate with the LVM daemon device driver.
 */

struct lvmd_rd
	 /* structure to contain buffer addresses in user space to which
	    kernel data will be copied on a read to the LVM daemon device
	    driver */
       {
       struct pbuf * pbuf;
	 /* address of user structure into which the pbuf structure to
	    be proccessed will be copied from the kernel */
       struct bad_blk * bad_blk;
	 /* address of user structure into which a bad block structure
	    associated with the pbuf being processed will be copied from
	    the kernel */
       struct unique_id * vg_id;
	 /* address of user structure into which the volume group id of
	    the VG which contains the pbuf to be processed will be copied
	    from the kernel */
       };

struct lvmd_wr
	 /* structure to contain data and buffer addresses in user space
	    which are to be copied to the kernel on a write to the LVM
	    daemon device driver */
       {
       struct bad_blk * bad_blk;
	 /* address of user structure from which a bad block structure
	    associated with the pbuf being processed will be copied to
	    the kernel */
       char pb_derror;
	 /* the LVM daemon error value which indicates the status of the
	    pbuf which was just processed by the LVM daemon */
       char pad [3];
	 /* reserved */
       };



#ifndef  NULL
#define  NULL            ((void *)0)
#endif

#ifndef  FALSE
#define  FALSE           0
#endif


/* **********************************************************************
 *   Daemon returns between 0 and 63 are good returns.  Values          *
 *   between 64 and 127 are error return codes.  This is mechanism      *
 *   used to avoid problems with sign extension on char typedefs.       *
 ************************************************************************
 */

#define  LVMD_ERROR      64    /* unsuccessful return			*/
#define  LVMD_BADBBDIR   65    /* bad block directory is bad		*/

#define  LVMD_SUCCESS    0     /* successful return			*/
#define  LVMD_BBCRB      1     /* changed reloc blk sent back to kernel */


#define  LVMD_DD_DEV     "/dev/lvmd"  /* device name for LVM daemon dd	*/
#define  LVMD_DD_MOD     "/etc/lvmd_dd"  /* name of daemon dd module	*/

#define  LVMD_DD_MODE    S_IFCHR | S_IRUSR | S_IWUSR  /* device mode	*/
#define  LVMD_DD_MINOR   0     /* minor number for LVM daemon dd	*/
#define  LVMD_DD_OPEN    1     /* open state for LVM daemon dd		*/

#define  LVMD_DD_CFG     1     /* LVM daemon dd configuration request	*/
#define  LVMD_DD_INIT    2     /* LVM daemon dd config init request	*/
#define  LVMD_DD_PANIC   3     /* LVM daemon dd config panic request	*/

#define  LVMD_MISSPV     1     /* request to process missing PV		*/
#define  LVMD_STALEPP    2     /* request to process stale PP		*/
#define  LVMD_FRESHPP    3     /* request to add a bad block entry	*/
#define  LVMD_BBADD      4     /* request to add a bad block entry	*/
#define  LVMD_BBDEL      5     /* request to delete a bad block entry	*/
#define  LVMD_BBUPD      6     /* request to update a bad block entry	*/
#define  LVMD_RORELOC    7     /* change PV state to read only reloc	*/
#define  LVMD_OPNDALV    8     /* request to open a desc area LV	*/
#define  LVMD_CLSDALV    9     /* request to close a desc area LV	*/

#endif  /* _H_LVMD */
