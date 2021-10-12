static char sccsid[] = "@(#)79  1.5.1.8  src/bos/kernext/disp/ped/font/midfont.c, peddd, bos411, 9428A410j 3/31/94 21:33:19";
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: dd_pinfont
 *		pinned_font_ready
 *		unpin_font
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

#include <sys/dma.h>
#include <sys/xmem.h>
#include <vt.h>
#include <sys/uio.h>

#include <sys/syspest.h>

#include <sys/intr.h>
#include <sys/display.h>

#include <sys/rcm.h>

#include "hw_dd_model.h"
#include "midddf.h"
#include <fkprocFont.h>

#include "hw_typdefs.h"
#include "hw_regs_u.h"
#include "hw_regs_k.h"
#include "hw_macros.h"
#include "hw_PCBkern.h"		/* Mid_FontRequestReceived */
#include "hw_ind_mac.h"

#include "mid_pos.h"		/* must precede midhwa.h */
#define NO_PIO_EXC	1         /* defeate PIO */
#include "midhwa.h"

#include "mid_dd_trace.h"
MID_MODULE(dd_font);

/* --------------------------------------------------------------*/
/* dd_pinfont:                                                   */
/*                 if two fonts are already pinned by adapter    */
/*                 {                                             */
/*                    log error against adapter                  */
/*                    return                                     */
/*                 }                                             */
/*                                                               */
/*                 increment pin count by 1                      */	
/*                                                               */
/*                 initialize queue element with necessary       */
/*                 data to pin this font                         */
/*                                                               */
/*                 enqueue (fsp_enq) queue element to pin        */
/*                 the requested font                            */
/*                                                               */
/*                 if fail to enqueue                            */
/*                     log error message                         */
/* --------------------------------------------------------------*/

dd_pinfont( font_DMA )
fkproc_font_pin_req_t * font_DMA;
{
	int i, rc, status=0;

	fkprocQE qe;       /* a queue element to enqueue a command to fkproc */

	ushort correlator;
	unsigned int pin_fontid = font_DMA->font_ID;

	struct phys_displays * pd;
	midddf_t * ddf;    /* pointer to device dependent data */

        BUGLPR(dbg_dd_font, BUGNTA, 
		("entering dd_pinfont fid=%x\n",pin_fontid));

	pd = font_DMA->pd;

	ddf = (midddf_t *) pd->free_area;

	MID_DD_ENTRY_TRACE(dd_font,1, PINFONT, ddf,
                               0,
	                       pin_fontid, font_DMA, ddf->pin_count);
			       

 	/*
           Note: by the time, dd_pinfont is called, we know we
           have not reach the limit.  Therefore, the pin count 
           is not really needed.  However, it provides us additional
           checking.  Since we have to remove font during hotkey.  For 
           this reason, it won't hurt to leave the code in as a safety
           valve. 

           Check if we reach the limit (again)
        */

	if (ddf->pin_count >= MAX_PIN_FONTS_ALLOW) 
        {        
           BUGLPR(dbg_dd_font, BUGACT,
		("dd_pinfont: can't have more than %d pinned fonts\n",
		MAX_PIN_FONTS_ALLOW));

	   MID_DD_EXIT_TRACE(dd_font,1, PINFONT, ddf,
                               0,
	                       pin_fontid, -1, -1);

           return(-1);
        }

	/* 
            Although the original design was to give up the graphics time 
            slice at this point in order for other context to be run while 
            we are (driver) processing the request.  The adapter promisses 
            to not handle a context switch during a font request. So, we
            don't have to invoke give_up_timeslice (rcm callback) any more.
        */

        /* when we pin a font, we increment the count by 1. */
        /* When we unpinit, we decrement the count by 1     */

        BUGLPR(dbg_dd_font, BUGACT,
		("dd_pinfont: pin count so far %d\n",ddf->pin_count));

	ddf->pin_count ++;

	qe.command = FKPROC_COM_PIN;    /* want to pin the font */

        /* The font ID (amongst other stuff) is passed in the 
	    request data structure.   */

	qe.request_data = (char *) font_DMA;


        BUGLPR(dbg_dd_font, BUGACT,("dd_pinfont: call enq &qe=%x\n",&qe));
	rc = (*pd->visible_vt->fsp_enq)(&qe, pd->lftanchor); /* enqueue work request*/
                                                   /* for fkproc()        */

	if (rc != FKPROC_Q_SUCC)        /* failed to enqueue */
        {
	  BUGLPR(dbg_dd_font,0,("failed to enq pin command for fontid %x\n",
		pin_fontid));
	  status = -1;
	}

	MID_DD_EXIT_TRACE(dd_font,1, PINFONT, ddf,
                               0,
	                       pin_fontid, status, ddf->pin_count);

        BUGLPR(dbg_dd_font, BUGNTA, ("exiting dd_pinfont\n"));
}


/* --------------------------------------------------------------*/
/* unpin_font:                                                   */
/*                                                               */
/*                    decrement font pin count by 1              */	
/*                                                               */
/*                    remove font from pinned_list               */
/*                                                               */
/*                    undo dma setup with d_complete()           */
/*                                                               */
/*                    remove cross memory pointer with xmdetach()*/
/*                                                               */
/*                    if(isKSRfont(fontid))                      */
/*                    {                                          */
/*                       unpin(fontaddr,len)                     */
/*                    }                                          */
/*                    else                                       */
/*                    {                                          */
/*                         enqueue unpin command to fkproc       */ 
/*                    }                                          */
/* --------------------------------------------------------------*/

unpin_font(pd, font_DMA)
struct phys_displays * pd;
fkproc_font_pin_req_t * font_DMA;
{
	label_t ipio_jmpbuf;
        int rc, status = 0;

	midddf_t * ddf = (midddf_t *) pd->free_area;
	ulong fontid;

        fkprocQE qe;     /* a queue element to enqueue work request to fkproc */

	HWPDDFSetup;


	fontid = font_DMA->font_ID;

        BUGLPR(dbg_dd_font, BUGNTA, ("->entering unpin_font, fid=%x\n",fontid));
        BUGLPR(dbg_dd_font, 1, (" font_DMA = 0x%8X\n", font_DMA));

	MID_DD_ENTRY_TRACE(dd_font,1, UNPINFONT, ddf,
                               0,
	                       fontid, font_DMA, ddf->pin_count);


        /* when we pin a font, we increment the counter by 1 */
        /* and when we unpin it, we decrement the count by 1 */

        BUGLPR(dbg_dd_font, BUGNTA, 
		("unpin_font: pin count so far %d\n",ddf->pin_count));
        ddf->pin_count --;            /* decrement pin count */	

        /* -------------------- 
           Font was pinned, DMA was setup; adapter has transferred font 
           data .  Note all of this was initiated by fkproc (device 
           independent code).  When the same font is not needed, adapter 
           requests the font to be unpinned (via interrupts) so we need to 
           call xmdetach and d_complete to undo DMA setup (xmattach,
           d_master).  
        ---------------------- */ 

        BUGLPR(dbg_dd_font, BUGNTA, ("unpin_font: call d_complete\n"));

        rc=d_complete(
			font_DMA-> DMA_channel,
			font_DMA-> flags,
			font_DMA-> sys_addr,
			font_DMA-> length,
			&(font_DMA-> xm),
			font_DMA-> bus_addr ) ;

        if (rc != DMA_SUCC)
        {
	      BUGLPR(dbg_dd_font,0,
		("unpin_font: d_complete failed:rc=%d errno=%d\n",rc,errno));
	      status |= 1;
        }

        BUGLPR(dbg_dd_font, BUGNTA, ("unpin_font: call xmdetach\n"));

        rc = xmdetach ( &(font_DMA->xm) ) ;

        if (rc != DMA_SUCC)
        {
            BUGLPR(dbg_dd_font,0,
		("unpin_font: xmdetach failed: rc=%d errno=%d\n",rc,errno));
	      status |= 2;
        }

        /* call kernel function to unpin the font */
        BUGLPR(dbg_dd_font, BUGACT,("before calling unpin addr=%x len=%d\n",
				font_DMA-> sys_addr, font_DMA-> length));

        /* -------------------
        when the micro-code asks us to unpin a font, we have to check
        if the font id is of type KSR or X.  If it is KSR, we can unpin
        the font within the interrupt hander.  Otherwise, we have to
        queue a request to the font process to ask it unpin the X font.

        The interrupt handler cannot unpin the X font because X fonts are
        in the shared memory created by X server.  Only the process which
        attached the shared memory to itself can access the data.  Since
        the attach was done by the font kernel process the unpin must 
        also be done by the font kernel process.  For KSR fonts, the font
        data is in the kernel space, therefore, the interrupt handler can
        unpin it.
        ----------------------- */ 
           
        if(isKSRfont(fontid))
        {
           rc = unpin( font_DMA-> sys_addr, font_DMA-> length );

           if (rc != 0) 
           { 
              /* there is problem unpinning the font */
	         BUGLPR(dbg_dd_font,0,("unpin error rc=%d addr=%x len=%d\n",
				rc, font_DMA-> sys_addr, font_DMA-> length));
              status |= 4;
           }
        }
        else  /* X font */ 
        {
	      qe.command = FKPROC_COM_UNPIN;
	      qe.font_addr = font_DMA-> sys_addr ;
	      qe.font_len = font_DMA-> length ;

              /* enqueue UNPIN command to font process */ 
	      rc = (*pd->visible_vt->fsp_enq)(&qe, pd->lftanchor); 
	      if (rc != FKPROC_Q_SUCC) 
              { 
	         BUGLPR(dbg_dd_font,0,
			("unpin_font: failed to enq UNPIN command\n"));
	         status |= 8;
              }
        }

	MID_DD_EXIT_TRACE(dd_font,1, UNPINFONT, ddf,
                               0,
	                       fontid, status, ddf->pin_count);

        BUGLPR(dbg_dd_font, BUGNTA, ("exiting unpin_font\n"));
}


/* --------------------------------------------------------------*/
/* pinned_font_ready:                                            */
/*                                                               */
/*   The code to pin a font is separated into 2 parts, device    */
/*   independence and device dependence.  The device independence*/
/*   is done by pin_font().  The all device dependent tasks are  */
/*   done in this routines.  They are:                           */
/*                                                               */ 
/*   - make sure context which caused font fault is active       */
/*     with make_cur_guard_domain                                */
/*   - get correlator                                            */ 
/*   - send PCB command to tell font is pinned  (this triggers   */
/*      DMA tranferring by adapter)                              */
/*   - enable context switching with ungard_domain               */ 
/*                                                               */
/*  Call by: pin_font                                            */
/* --------------------------------------------------------------*/

pinned_font_ready(qe, font_info)
fkprocQE * qe;
font_addr_n_len * font_info;      /* not need it.  Will remove some day */
{

	int old_interrupt_priority;

	fkproc_font_pin_req_t * request_data_ptr = qe->request_data;

	struct phys_displays * pd = request_data_ptr->pd;
	midddf_t * ddf = (midddf_t *) pd->free_area;
	ushort correlator;

	label_t ipio_jmpbuf;

	/* unsigned int tmp;*/
	int i;

	HWPDDFSetup;

        BUGLPR(dbg_dd_font, BUGNFO, 
		("-> entering pinned_font_ready faddr=%x, len=%d\n",
		request_data_ptr-> sys_addr, request_data_ptr-> length));

	MID_DD_ENTRY_TRACE(dd_font,1, FONT_READY, ddf,
                               0,
	                       request_data_ptr->font_ID, 
	                       request_data_ptr->length, 
	                       request_data_ptr->sys_addr); 

#if DEBUG
	for(i=0; i< MAX_PIN_FONTS_ALLOW ; i++)
	{
       	 	BUGLPR(dbg_dd_font,BUGNTA,
			("pinned_font_ready: pincnt=%d, saved id0 =%x \n",
			ddf->pin_count,ddf->font_DMA[i].font_ID));
	}
#endif

   	/* -----------------
           Microcode doesn't use the correlator for any purpose, so we just 
           pick some arbitrary number to meet the argumemetn lists Required by 
           PCB macro 
      	----------------- */

	correlator = 0xa0a0; 

	/*------------------------------------------------------------------*
	   We must disable interrupt for a couple of reasons:
	     . we are about to put the BIM into slow mode.  We don't want an
	        interrupt to be processed which could result in putting us
		back into fast mode.
 	     . we are about to write to the PCB.  Part of this mechanism
		requires us to reset the "PCB EMPTY" bit before writing into
		the PCB.  We don't want anyone else to interrupt during this
		window as the PCB would then be non-empty with no way for it
		to go empty.  

	    These windows (and then multifarious others) will/must get fixed
	     with the final PCB interlock fix.  This is just a bandaid for 
	     this particular window.
	 *------------------------------------------------------------------*/

#define INTR_PRIORITY (pd->interrupt_data.intr.priority)
        old_interrupt_priority = i_disable(INTR_PRIORITY);

        /*------------------
            Attach bus
        ------------------*/
        IPIO_EXC_ON();

        MID_SLOW_IO(ddf) ;


   	/* ----------------- 
      	send PCB command FontPinnedUnpinned to adapter 
      	----------------- */

    	BUGLPR(dbg_dd_font, 2, ("pinned_font_ready: call MID_FontPinned\n"));
        BUGLPR(dbg_dd_font,2,("busaddr = 0x%x\n", request_data_ptr->bus_addr));



   	MID_FontPinnedUnpinned(correlator, 
        			request_data_ptr-> bus_addr,
        			request_data_ptr-> length, 
               			request_data_ptr-> font_ID, 
				NO_FONT_ID ); 

        /*------------------
           Detach bus
        ------------------*/
        MID_FAST_IO(ddf) ;

        IPIO_EXC_OFF();
	
       	i_enable(old_interrupt_priority);
	

	MID_DD_EXIT_TRACE(dd_font,1, FONT_READY, ddf,
                               0,
	                       request_data_ptr->font_ID, 
	                       0, 
	                       0); 

    	BUGLPR(dbg_dd_font, BUGNFO, ("exiting pinned_font_ready\n"));
}
