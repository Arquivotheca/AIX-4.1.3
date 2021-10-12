/* @(#)28	1.2  src/bos/kernel/sys/iostat.h, sysios, bos411, 9428A410j 6/16/90 00:30:04 */
#ifndef	_H_IOSTAT
#define	_H_IOSTAT
/*
 * COMPONENT_NAME: (SYSIOS) I/O Subsystem
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Iostat structure for tty
 */
struct ttystat
{
	long rawinch;		/* tty raw characters in count		*/
	long caninch;		/* tty canonical characters in count	*/
	long rawoutch;		/* tty characters out count		*/
};

/*
 * Iostat structure for disks
 */
struct dkstat
{
	char		diskname[32];	/* disk's logical name		*/
	struct dkstat	*dknextp;	/* ptr to next entry in chain	*/
	ushort		dk_status; 	/* disk entry status flags	*/ 
#define IOST_DK_BUSY	0x1		/* disk is currently busy	*/
	ulong		dk_time;  	/* amount of time disk is active*/
	ulong		dk_xrate;	/* kbytes/sec xfer rate capability*/	
	ulong		dk_bsize;	/* #bytes in a block for this disk*/
	ulong		dk_xfers;	/* #transfers to/from disk	*/
	ulong		dk_rblks;	/* #blocks read from disk	*/
	ulong		dk_wblks;	/* #blocks written to disk	*/
	ulong		dk_seek;	/* #seek operations for disks
					   with discrete seek commands	*/
};

/*
 * Kernel structure for keeping i/o statistics on disks
 */
struct iostat
{
	struct dkstat	*dkstatp;	/* ptr to linked list of disk
					   entries, one per configured disk */
	ulong		dk_cnt;		/* #dkstat structures in list	*/
};

#endif	/* _H_IOSTAT */
