static char sccsid[] = "@(#)59  1.4.1.4  src/bos/kernext/disp/ped/intr/fontintr.c, peddd, bos411, 9428A410j 3/31/94 21:34:28";
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: BUGLPR
 *		mid_font_request
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



#include <sys/types.h>
#include <sys/errno.h>
#include <sys/iocc.h>
#include <sys/dma.h>
#include <sys/xmem.h>
#include <sys/uio.h>
#include <sys/syspest.h>

#include <sys/intr.h>
#include <sys/display.h>
#include <sys/rcm.h>

#include "hw_dd_model.h"
#include "midddf.h"

#include "hw_typdefs.h"
#include "hw_regs_u.h"
#include "hw_regs_k.h"
#include "hw_macros.h"
#include "hw_PCBkern.h"		/* Mid_FontRequestReceived */
#include "hw_ind_mac.h"

#include "midhwa.h"

#include "mid_dd_trace.h"
MID_MODULE(dd_font);


/* --------------------------------------------------------------*/
/* mid_font_request:                                             */
/*								 */
/*	Handle font faults on Pedernales                         */
/*								 */
/* Flow of Control:                                              */
/*								 */
/* 	When a font fault occurs on the adapter, it raises       */
/*      an interrupt to the CPU called the font request.  This   */
/*      kind of interrupt (and others) is recognized by          */
/*      mid_intr() which then calls mid_font_request.            */
/*      Unpinning a font does not cause page fault so it can be  */
/*      done in the interrupt handler.  On the other hand pinning*/
/*      a font data can cause page fault so a kernel process     */
/*      called fkproc is created to do the pin.  Fkproc is       */
/*      created during configuration.  Along			 */
/*      with fkproc, a queue is also initialized to allow driver */
/*      to pass work request and data to fkproc, i.e, enqueuing  */ 
/*	a command.  Note the queue is in the common data of vtm  */
/*      (see vt.h)                                               */ 
/*                                                               */
/*      Pseudo-code:                                             */
/*                                                               */
/*              read adapter 's font request block               */
/*              send MID_FontQuestReceived()                     */
/*              if Unpin request                                 */
/*              {                                                */
/*                 unpin_font()                                  */
/*              }                                                */ 
/*                                                               */
/*              if pin request                                   */
/*              {                                                */
/*                 do we have any more font_DMA structure in     */
/*                 ddf to process request.                       */ 
/*                 if we do not,                                 */
/*                       exit (note ucode will hange)            */
/*                 else                                          */
/*                       dd_pinfont()                            */
/*              }                                                */ 
/*                                                               */
/* --------------------------------------------------------------*/
mid_font_request(pd,dsp_status)
struct phys_displays *pd;
ulong dsp_status;
{
	ulong unpin_fontid, pin_fontid;

	midddf_t * ddf = (midddf_t *) pd->free_area;
	label_t ipio_jmpbuf;

	short 	j;	/* index for font DMA bus_addr search loop         */
	char found;

	HWPDDFSetup;




        BUGLPR(dbg_dd_font,BUGNTA,("-> entering mid_font_request pd=%x\n",pd));

        BUGLPR(dbg_dd_font,BUGNTA, ("mid_font_request pincnt=%d\n",
		ddf->pin_count));



#if DEBUG
for ( i=0; i< MAX_PIN_FONTS_ALLOW; i++ )
{
        BUGLPR(dbg_dd_font,BUGNTA,
		("mid_font_request: font %d in ddf, fontid=%x\n",
		i,ddf->font_DMA[i].font_ID));
}
#endif




	/* Read font request block   */

	MID_RD_FRB_VALUE(MID_FRB_UNPIN_FONT_ID,unpin_fontid); 
	MID_RD_FRB_VALUE(MID_FRB_PIN_FONT_ID,pin_fontid);    

	MID_DD_ENTRY_TRACE(dd_font, 1, FONT_REQUEST, ddf,
                           0, pin_fontid, unpin_fontid, ddf->pin_count);


	BUGLPR(dbg_dd_font, BUGACT,
		("mid_font_request: font req bkk: pinid=%x unpinid=%x\n",
		pin_fontid,unpin_fontid));
	
        /* send font request received, so font request block can be reused */ 

        BUGLPR(dbg_dd_font, BUGNTA, ("mid_font_request: send font req rec\n"));
	MID_FontRequestReceived();

	/* ---------------- CASE: UNPIN A FONT --------------------*/

        /* if no font is requested to be unpinned, it will be 0 */

	if (unpin_fontid != 0)
        {


	   /*---------------------------------------------------------------
	      Find the matching font ID in the fond_DMA structure and unpin it.

	      After we unpin the font, remove the associated font ID from
	      the structure list.  This frees up this portion of address
	      space for the next font DMA.
	   ---------------------------------------------------------------*/
	   found = 0;

	   for ( j=0; j< MAX_PIN_FONTS_ALLOW; j++ )
	   {

		if ( unpin_fontid == ddf->font_DMA[j].font_ID )
		{
	   		BUGLPR(dbg_dd_font, BUGACT,
				("mid_font_request: call unpin_font\n"));
	   		unpin_font(pd, &(ddf->font_DMA[j]) );

			ddf->font_DMA[j].font_ID = NO_FONT_ID;
                        found = 1;
			break;	/* found it, no need to go further       */
		}  

	   }   

#if DEBUG
	   if (!found)
	   {
              BUGLPR(dbg_dd_font, 0,
		("mid_font_request: case unpin -> font id was not found in ddf\n"));
	   }
#endif
		
        }

	/* ---------------- CASE: PIN A FONT ----------------------*/

        /* if no font is requested to be pinned, it will be 0 */

	if (pin_fontid != 0)
        {

	   for ( j=0; j< MAX_PIN_FONTS_ALLOW; j++ )
           {
                /* 
                   check weather font is already pinned.  If so,
                   ignore the request.  Note the microcode will hange
                   waiting for the driver in this case 
                */
		if ( ddf->font_DMA[j].font_ID == pin_fontid )
		{

                   BUGLPR(dbg_dd_font, 0, ("mid_font_request: attempted to pin a pinned font, id=%x\n",pin_fontid));

	           MID_DD_EXIT_TRACE(dd_font, 1, FONT_REQUEST, ddf,
                     0, ddf->font_DMA[0].font_ID,ddf->font_DMA[1].font_ID, -1);

                   goto error;

		}
	   }

	   /*----------------------------------------------------------*
	     Search for an unused font DMA bus address range.  If font_ID
	     is NO_FONT_ID, this means that this address space is available.
	   *----------------------------------------------------------*/

	   found = 0;      /* false */

	   for ( j=0; j< MAX_PIN_FONTS_ALLOW; j++ )
	   {

		if ( ddf->font_DMA[j].font_ID == NO_FONT_ID )
		{

			ddf->font_DMA[j].font_ID     = pin_fontid;
			ddf->font_DMA[j].pd          = pd; 
			ddf->font_DMA[j].DMA_channel = pd->dma_chan_id; 

	   		BUGLPR(dbg_dd_font, BUGACT,
				("mid_font_request: call dd_pinfont\n"));
	   		dd_pinfont( &(ddf->font_DMA[j]) );

	 		found = 1;
			break;	/* found a free bus address, so leave.  */

		}

	   }

#if DEBUG
	   if (! found)
           {
              BUGLPR(dbg_dd_font, 0,
		("mid_font_request: case pin -> no more room in ddf for font %x\n",pin_fontid));
           }
#endif

        }


	MID_DD_EXIT_TRACE(dd_font, 1, FONT_REQUEST, ddf,
                0, ddf->font_DMA[0].font_ID, ddf->font_DMA[1].font_ID, found);

error:
        BUGLPR(dbg_dd_font, BUGNTA, ("exiting mid_font_request\n"));

}
