/* @(#)18	1.7  src/bos/kernel/sys/POWER/sysdma.h, sysios, bos411, 9428A410j 6/15/90 17:50:23 */
#ifndef _H_SYSDMA
#define _H_SYSDMA
/*
 * COMPONENT_NAME: (SYSIOS) IO subsystem
 *
 * FUNCTIONS: Machine dependent extension to dma.h
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


/*	The following labels are for DMA slaves.			*/

#define		DMA_MAX		0x100000	/* maximum DMA transfer       */
#define		DMA_MAX8	DMA_MAX		/* max xfer for 8-bit device  */
#define		DMA_MAX16	DMA_MAX         /* max xfer for 16-bit device */
#define		DMA_MAX32	DMA_MAX		/* max xfer for 32-bit device */
 

/*
*	The following labels are for DMA masters.
*
*	A device driver or device handler should only allocate
*	a bus address in the window associated with its DMA channel.
*	These labels provide constants that can be used to calculate
*	the address of the window for each DMA channel.
*
*	window_start = DMA_BASE + (channel * DMA_DELTA)
*	window_end = DMA_BASE + (DMA_SIZE - 1)
*
*	TCWs map bus addresses to system real memory addresses.
*	The number of TCWs determine the maximum number of
*	block transfers that a DMA master can have at any instant
*	in time.
* 
*	number_of_TCWs = DMA_SIZE >> DMA_L2PSIZE;
*
*	note: because of alignment, even small transfers may require
*	more than one TCW.
* 
*/
#define         DMA_BASE        0x100000        /* start of first window   */
#define         DMA_SIZE        0x40000         /* Maximum  window size	   */
#define         DMA_DELTA       DMA_SIZE        /* offset to next window   */
#define         DMA_PSIZE       0x1000          /* DMA page size           */
#define         DMA_L2PSIZE     0xC             /* log 2 of DMA  page size */
 
 
#endif /*_H_SYSDMA */
