/* @(#)42       1.29  src/bos/kernel/ios/POWER/dma_hw.h, sysios, bos411, 9428A410j 3/15/93 18:35:18 */
/*
 * COMPONENT_NAME: (SYSIOS) IO subsystem
 *
 * FUNCTIONS:  DMA hardware interface
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
#ifndef _h_DMA_HW
#define _h_DMA_HW

#ifdef _POWER_RS
#include "dma_hw_pwr.h"
#endif /* _POWER_RS */

#ifdef _POWER_PC
#include "dma_hw_ppc.h"
#endif /* _POWER_PC */

#ifndef _NO_PROTO
/*
 *  	Allocate a DMA channel
 *
 *	returns: channel ID  = for use with other DMA services
 *		 DMA_FAIL    = already in use
 */
int d_init(	int channel, 		/* channel number */
		int flags, 		/* flags describing channel's use */
		vmhandle_t bid		/* seg reg value with Bus ID */
		);
/* 
 *	Free a DMA channel
 */
void d_clear(int channel_id		/* channel ID returned by d_init*/
		);
/* 
 *	Disable a DMA channel        
 */
void d_mask(int channel_id		/* channel ID returned by d_init*/
		);

/* 
 *	Enable a DMA channel         
 */
void d_unmask(int channel_id	/* channel ID returned by d_init*/
		);

/*
 * 	Setup for Slave DMA block mode transfer.
 */
void d_slave(int channel_id, 	/* channel ID returned by d_init*/
		  int flags,      	/* control flags             */
		  char *baddr,         	/* buffer address            */
		  size_t count,        	/* length of transfer        */
		  struct xmem *dp     	/* cross mem descrip         */
		);

/*
 * 	Setup for Master DMA block mode transfer.
 */
void d_master(int channel_id,	/* channel ID returned by d_init*/
		  int flags,            /* control flags */
		  char *baddr,          /* buffer address */
		  size_t count,         /* length of transfer */
		  struct xmem *dp,      /* cross mem descrip */
		  char *daddr           /* bus address */
		);

/*
 * 	Clean up after a DMA transfer
 */
int d_complete(int channel_id,       /* channel ID returned by d_init*/
		   int flags,            /* control flags */
		   char *baddr,          /* buffer address */
		   size_t count,         /* length of transfer */
		   struct xmem *dp,      /* cross mem descrip */
		   char *daddr           /* bus address */
		  );

/*
 *	Flush the processor cache and invalidate the next buffer 
 *	for dual buffered IOCCs.  NOTE: This routine is a NO-OP on
 *	PowerPC.
 */
int d_cflush(int channel_id, 	 /* channel ID returned by d_init*/
		 char *baddr,            /* buffer address */
		 size_t count,           /* length of transfer */
		 char *daddr             /* bus address */
		);

/*
 *	Flush the IOCC buffer associated with the given channel.  
 */
int d_bflush(int channel_id, 	 /* channel ID returned by d_init*/
		 vmhandle_t buid,        /* Bus ID handle */
		 char *daddr             /* bus address */
		);

/*
 *	Provide consistent buffer access between device and
 *	driver.  NOTE: not supported on PowerPC platform.
 *	Returns: EINVAL
 */	
int d_move(int channel_id,  	 /* channel ID returned by d_init*/
	       int flags,                /* control flags */
	       char *baddr,              /* buffer address */
	       size_t count,             /* length of transfer */
	       struct xmem *dp,          /* cross mem descrip */
	       char *daddr               /* bus address */
		);


#else

int d_init();			/* allocate a DMA channel       */
void d_clear();			/* deallocate a DMA channel     */
void d_mask();			/* disable a DMA channel        */
void d_unmask();		/* enable a DMA channel         */
void d_slave();			/* init block mode slave transfer */
void d_master();		/* init block mode master transfer */
int d_complete();		/* clean up after a DMA transfer*/
int d_cflush();			/* flush cache and invalidate buff */
int d_bflush();			/* flush buffer */
int d_move();			/* consistent data movement */

#endif /* not _NO_PROTO */


#endif /* _h_DMA_HW */


