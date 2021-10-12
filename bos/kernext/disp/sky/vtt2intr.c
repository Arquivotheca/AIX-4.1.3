static char sccsid[] = "@(#)11  1.26  src/bos/kernext/disp/sky/vtt2intr.c, sysxdispsky, bos411, 9428A410j 4/13/94 17:45:24";

/*------------
 *
 * COMPONENT_NAME:   sysxdispsky -- Entry Level Display Device Driver
 *
 * FUNCTIONS: KSR/MONITOR MODE device driver
 *
 * ORIGINS: 27
 *
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/***********************************************************************/
/*                                                                     */
/* IDENTIFICATION:  SKY_INTR                                           */
/*                                                                     */
/* DESCRIPTIVE NAME:  SKY_INTR - Interrupt handler for Entry display   */
/*                                                                     */
/* FUNCTIONS:   sky_intr                                               */
/*                                                                     */
/* INPUTS:  Interrupt structure pointer - same as phys display pointer */
/*                                                                     */
/* OUTPUTS:  None.                                                     */
/*                                                                     */
/* CALLED BY:  First level interrupt handler                           */
/*                                                                     */
/* CALLS:  sky_err - to log errors                                      */
/*         passed in call back functions for dma/event handling        */
/*                                                                     */
/***********************************************************************/
#include <sys/types.h>
/* #include <sys/ioacc.h> */
#include <sys/adspace.h>
#include <sys/proc.h>
#include <sys/mdio.h>
#include <sys/conf.h>
#include <sys/intr.h>
#include <sys/uio.h>
#include <sys/malloc.h>
#include <sys/device.h>
#include <sys/sysmacros.h>
#include <sys/errids.h>
#include <sys/systm.h>

#include <sys/display.h>
#include "vtt2def.h"
#include "vtt2ddf.h"
#include "vtt2regs.h"
#include "ddsent.h"

#include "skyras.h"		/* error codes for sky_err() */

extern int sky_err();

int sky_intr(intdat)
struct intr *intdat;
{

    struct phys_displays  *pd;
    struct sky_ddf  *ddf;
    struct sky_io_regs volatile *iop;
    char   int_type;
    caddr_t  segreg;
    extern time_t time;
    char   clear_op;
    struct ddsent *dds;
    int parityrc;
    volatile ulong *vram,i;
    label_t jmpbuf;

   /* This routine will be invoked when an interrupt occurs on the level*/
   /* passed to i_init routine by the device init routine. The code must*/
   /* go and look at the adapter and verify it was the cause of the     */
   /* interrupt.                                                        */
   /* If it did not cause the interrupt then it will return to the      */
   /* level interrupt handler with a code of INTR_FAIL.                 */

    segreg = BUSMEM_ATT(BUS_ID,0);
    pd = (struct phys_displays *) intdat;
    dds = (struct ddsent *) pd->odmdds;
    ddf = (struct sky_ddf *) pd->free_area;

    /* Set up exception handler */
    parityrc = setjmpx(&(jmpbuf));

    if (parityrc != 0)
    {
       if (parityrc == EXCEPT_IO)
       {
          BUSMEM_DET(segreg);
	  /* Log an error and return */
	  sky_err(NULL,"SKYDD","vtt2intr","setjmpx", EXCEPT_IO, 
                  SKY_SETJMP, SKY_UNIQUE_1);
	  return(INTR_SUCC);
       }
       else
	  longjmpx(parityrc);
    }


    /* load pointer to io space registers to see what kind it was */

    iop = dds->io_bus_addr_start + segreg;
    int_type = iop->int_status;

    int_type = int_type & iop->int_enable;
    clear_op = TRUE;


    if (int_type)
    {

      /* We have recieved a skyway interrupt */
      /* Handle it based on mode flag        */
      ddf = (struct sky_ddf *) pd->free_area;
      if ( ddf->cmd & EVENT_MODE)
      {
	if (ddf->cmd & NORPT_EVENT)
	{
	   /* No report event requested                          */
	   /* No processing needed for this type event           */
	   /* Pick or other device specific data would be read   */
	   /* from the adapter in this mode.                     */
	}
	if (ddf->cmd & SYNC_REQ || ddf->cmd & ASYNC_REQ)
	{
	   /* In event mode we have been asked to develop */
	   /* an interrupt report and return it to the    */
	   /* callback function                           */
	   ddf->report.event = int_type;
	   ddf->report.time  = time;
	   ddf->report.rcx   = 0;
	   ddf->report.wa    = 0;
	   ddf->report.data[0] = 0;
	   ddf->report.data[1] = 0;
	   ddf->report.data[2] = 0;
	   ddf->report.data[3] = 0;

	   /* Clear timer */
	   tstop(ddf->cdatime);
	   ddf->timerset = FALSE;
	   /* If it was a sync wait request then wake up process */
	   if (ddf->cmd & SYNC_WAIT_REQ)
	      e_wakeup(&ddf->sleep_addr);
	   else
	      /* Invoke event callback routine */
	      (ddf->callback)(pd->cur_rcm,&(ddf->report));

	   /* clear interrupt mask - This turns all INTS OFF !*/

	   iop->int_enable = 0x00;
	   ddf->cmd &= SYNC_OFF | EVENT_OFF;
	   ddf->s_event_mask = 0;

	}

      }

      if ( ddf->cmd & DIAG_MODE)
      {
	 if (int_type & FRAME_FLYBACK_MASK)
	   ddf->diaginfo[0]++;
	 if (int_type & VERT_BLANK_END_MASK)
	   ddf->diaginfo[1]++;
	 if (int_type & SPRITE_END_MASK)
	   ddf->diaginfo[2]++;
	 if (int_type & COP_ACCESS_REJECT_MASK)
	   ddf->diaginfo[3]++;
	 if (int_type & OP_COMPLETE_MASK)
	   ddf->diaginfo[4]++;
      }

      if ( ddf->cmd & DMA_WAIT_REQ)
      {
	 /* Clear timer */
	 tstop(ddf->cdatime);
	 ddf->timerset = FALSE;
	 /* Reset interrupt status and enable */
	 iop->int_status = 0xff;
	 iop->int_enable &= OP_COMPLETE_CLEAR;
	 /* Wake up the process */
	 e_wakeup(&(ddf->sleep_addr));
	 ddf->cmd &= DMA_COMPLETE;
      }
      else
      {
	if ( ddf->cmd & DMA_IN_PROGRESS)
	  {
	     /* Command in progress was a no wait dma  */
	     /* clean up is on interrupt level         */

	     iop->int_status = 0xff;
	     iop->int_enable &= OP_COMPLETE_CLEAR;
	     (ddf->dcallback)(pd->cur_rcm);
	     ddf->cmd &= DMA_COMPLETE;
	  }
      }
      /* Clear interrupt field in the adapter */
      iop->int_status &= int_type;

      BUSMEM_DET(segreg);
      i_reset(intdat);
      /* Clear exception handler */
      clrjmpx(&(jmpbuf));
      return(INTR_SUCC);

    }  /* Int_type != 0 */

    /* If we get here then it is not our interrupt */
    BUSMEM_DET(segreg);
    clrjmpx(&(jmpbuf));
    return(INTR_FAIL);
}



void cda_timeout (wd)
struct trb *wd;
{
   struct phys_displays *pd;
   struct sky_ddf *ddf;

   /* This routine will be invoked when the time has expired */

   pd = (struct phys_displays *) wd->func_data;
   ddf = (struct sky_ddf *) pd->free_area;

   if ( ddf->cmd & DMA_WAIT_REQ || ddf->cmd & EVENT_MODE)
   {
      /* If we get here timer has popped and dma has not completed */
      /* in one second wake up process                             */
      e_wakeup(&(ddf->sleep_addr));
      ddf->cmd &= DMA_COMPLETE;
      ddf->cmd &= EVENT_OFF;
      return;
   }




   if (ddf->jumpmode)
   {
      if (ddf->lastcount == ddf->jumpcount)
      {
	 ddf->timerset = FALSE;
	 ddf->scrolltime = FALSE;
      }
      else
      {
	 ddf->scrolltime = TRUE;
	 ddf->lastcount = ddf->jumpcount;
	 ddf->cdatime->timeout.it_value.tv_sec = 0;
	 ddf->cdatime->timeout.it_value.tv_nsec = 150000000;
	 ddf->timerset = TRUE;
	 tstart(ddf->cdatime);
      }
   }
   else
   {


      if (ddf->jumpcount > ddf->jthreshold)
      {
	 ddf->jumpmode = TRUE;
	 ddf->jumpcount = 0;
	 ddf->lastcount = -1;
	 ddf->cdatime->timeout.it_value.tv_sec = 0;
	 ddf->cdatime->timeout.it_value.tv_nsec = 200000000;
	 ddf->timerset = TRUE;
	 tstart(ddf->cdatime);
      }
      else
      {
	 ddf->jumpcount = 0;
	 ddf->timerset = FALSE;
      }

   }

}







