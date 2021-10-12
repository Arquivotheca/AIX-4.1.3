static char sccsid[] = "@(#)60  1.13  src/bos/kernext/disp/ped/config/mid_open.c, peddd, bos411, 9428A410j 11/3/93 11:10:49";
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: mid_open
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <sys/types.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/proc.h>
#include <sys/mdio.h>
#include <sys/conf.h>
#include <sys/file.h>
#include <sys/intr.h>
#include <sys/uio.h>
#include <sys/dma.h>
#include <sys/pin.h>
#include <sys/malloc.h>
#include <sys/device.h>
#include <sys/sleep.h>
#include <sys/sysmacros.h>
#include <sys/errids.h>
#include <sys/syspest.h>
#include <sys/systm.h>
#include <sys/errno.h>
#include "mid.h"
#define Bool unsigned
#include <sys/aixfont.h>
#include <sys/display.h>

#include "ddsmid.h"
#include "midddf.h"
#include "midhwa.h"
/*
#include "midksr.h"
#include "midrcx.h"
#include "midfifo.h"
*/
#include "hw_errno.h"


BUGXDEF(dbg_middd);
BUGXDEF(dbg_midddi);
BUGXDEF(dbg_middma);
BUGXDEF(dbg_midddf);
BUGXDEF(dbg_midrcx);
BUGXDEF(dbg_midevt);
BUGXDEF(dbg_midtext);

#ifndef BUGPVT
#define BUGPVT	99
#endif

#ifndef BUGVPD				/* Set to 0 in Makefile if debugging */
#define BUGVPD	BUGACT
#endif


extern long mid_intr ();








/***********************************************************************/
/*								       */
/* IDENTIFICATION: MID_OPEN					       */
/*								       */
/* DESCRIPTIVE NAME: MID_OPEN - Initializes the MIDDD3D adapter        */
/*								       */
/* FUNCTION:							       */
/*								       */
/*								       */
/*								       */
/* INPUTS:							       */
/***********************************************************************/

long  mid_open(devno,flag,chan,ext)
dev_t devno;		     /* Major and minor number of the device   */
long flag;		     /* flags for type of open. 	       */
long chan;		     /* Channel number ignored by this routine */
long ext;		     /* Extended system parameter - not used   */

{
	long rc = 0;
	long init_rc = 0;
	short i;
	struct ddsmid *ddsptr;
	midddf_t *ddf;
	struct phys_displays *pd;
	struct intr *intdat;	  /* interrupt level handler structure	*/
	int status;
	int b;
	unsigned int save_bus_acc = 0;
	ulong	old_bus;
	volatile unsigned long HWP;
	int  vpd_index = 0;
	label_t jmpbuf;

	/*--------------------------------------------------------------*/
	/* This routine will eventually only set the device open field. */
	/* For lft bring up it will invoke the mid_config routine which */
	/* eventually be invoked by the adapters command. After config	*/
	/* completes installing the driver into the devsw and allocates */
	/* the phys_display table, open will call mid_init to initialize*/
	/* the interrupt code and run diagnostic code to reset the	*/
	/* adapter.							*/
	/*--------------------------------------------------------------*/


#if DEBUG
/*
dbg_middd = 0;
*/
#endif

	BUGLPR(dbg_middd, BUGVPD, ("Entering mid_open.\n"));
	devswqry(devno,&status,&pd);

	for (; pd; pd = pd->next)
		if (pd->devno == devno)
			break;
	if (!pd)
		return -1;

	if (pd->open_cnt != 0)		   /* Driver is already opened */
	{
		BUGLPR(dbg_middd, 0,
			("INIT FAILED! OPEN COUNT IS NOT ZERO!\n"));
		return -1;
	}

	ddsptr = (struct ddsmid *)pd->odmdds;
	ddf = (midddf_t *) pd->free_area;


	/*--------------------------------------------------------------*/
	/* Set the open_cnt field in the physical display entry.	*/
	/*--------------------------------------------------------------*/

	pd->open_cnt = 1;

	/*--------------------------------------------------------------*/
	/* Initialize DMA channel					*/
	/*--------------------------------------------------------------*/

	pd->dma_chan_id =  d_init(ddsptr->dma_channel,
	MICRO_CHANNEL_DMA, BUS_ID);


	if (pd->dma_chan_id == DMA_FAIL) {
		BUGLPR(dbg_middd, 0,
		("DINIT FAILED channel # %d flag = %x  bus_id = %x\n",
		ddsptr->dma_channel, MICRO_CHANNEL_DMA,BUS_ID));
		return(-1);
	}


        /*------------------
        Initialize DDF data structures.
        ------------------*/

	BUGLPR(dbg_middd, BUGACT, ("Calling mid_initialize_DDF with: 0x%x\n",
		ddf));
        mid_initialize_DDF(ddf);


        /*------------------
	Initialize RCM structures.
        ------------------*/

	if (!init_rc) {

		BUGLPR(dbg_middd, BUGACT,
			("Calling mid_wid_init with:0x%x\n",
					&(ddf->mid_wid_data)));
		mid_wid_init(&(ddf->mid_wid_data));

	}
	


	/*--------------------------------------------------------------
	  Initialize the host initiated context DMA information
	  The DMA_FLAGS are set up in midswitch.c
	----------------------------------------------------------------*/

	ddf->ctx_DMA_host_init_base_bus_addr = 
		pd->d_dma_area[0].bus_addr + MID_CTX_HOST_INIT_BASE_BUSADDR;

	ddf->ctx_DMA_HI_channel = pd -> dma_chan_id ;


	BUGLPR(dbg_middd, 3, ("HI_ctx_bus_addr  = 0x%8X\n", 
				ddf->ctx_DMA_host_init_base_bus_addr));
	BUGLPR(dbg_middd, 3, ("HI_ctx DMA flags = 0x%8X\n", 
				ddf->ctx_DMA_HI_flags));
	BUGLPR(dbg_middd, 3, ("HI_ctx DMA chan  = 0x%8X\n",
				ddf->ctx_DMA_HI_channel));


	/*---------------------------------------------------------------
	  Initialize the adapter initiated context DMA information
	----------------------------------------------------------------*/ 

	ddf->ctx_DMA_adapter_init_base_bus_addr = 
		pd->d_dma_area[0].bus_addr + MID_CTX_ADAPTER_INIT_BASE_BUSADDR;

	ddf->ctx_DMA_AI_flags = DMA_READ | DMA_NOHIDE;
	ddf->ctx_DMA_AI_channel = pd -> dma_chan_id ;


	BUGLPR(dbg_middd, 3, ("AI_ctx_bus_addr  = 0x%8X\n", 
				ddf->ctx_DMA_adapter_init_base_bus_addr));
	BUGLPR(dbg_middd, 3, ("AI_ctx DMA flags = 0x%8X\n", 
				ddf->ctx_DMA_AI_flags));
	BUGLPR(dbg_middd, 3, ("AI_ctx DMA chan  = 0x%8X\n",
				ddf->ctx_DMA_AI_channel));



	/*----------------------------------------------------------------*
	   Init font dma bus addresses and structures.
	 *----------------------------------------------------------------*/

	ddf->font1_bus_addr = pd->d_dma_area[0].bus_addr +
						MID_FONT1_BASE_BUSADDR;
	ddf->font2_bus_addr = pd->d_dma_area[0].bus_addr +
						MID_FONT2_BASE_BUSADDR;

	BUGLPR(dbg_middd, 3, ("font1_bus_addr = 0x%x\n", ddf->font1_bus_addr));
	BUGLPR(dbg_middd, 3, ("font2_bus_addr = 0x%x\n", ddf->font2_bus_addr));


	/*----------------------------------------------------------------*
	    These assigns are simply to initialize font structures
	    to be used by the interrupt handler to manage useage of
	    the separate font DMA bus address ranges.
	 *----------------------------------------------------------------*/

	ddf->font_DMA[0].font_ID  = NULL;
	ddf->font_DMA[1].font_ID  = NULL;

	ddf->font_DMA[0].bus_addr = ddf->font1_bus_addr;
	ddf->font_DMA[1].bus_addr = ddf->font2_bus_addr;




#if DEBUG
/*
dbg_middd = 0;
*/
#endif



	BUGLPR(dbg_middd, BUGVPD, ("Leaving mid_open.\n"));
	return(0);

}

