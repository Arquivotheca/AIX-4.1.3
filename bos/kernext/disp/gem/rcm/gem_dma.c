static char sccsid[] = "@(#)22	1.7.2.3  src/bos/kernext/disp/gem/rcm/gem_dma.c, sysxdispgem, bos411, 9428A410j 1/19/93 12:19:10";
/*
 *   COMPONENT_NAME: SYSXDISPGEM
 *
 *   FUNCTIONS: *		start_dma
 *		vttdma
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <sys/sleep.h>
#include <sys/errno.h>
#include "rcm_mac.h"
#include "gemincl.h"

vttdma(pdev,arg,callback,vp,indx)
gscDev *pdev;
gscdma *arg;
int (*callback)();
struct vtmstruc *vp;
int indx;
{
	struct gemini_data *ld;
	struct gem_ddf *ddf;
	struct _rcmProc *pproc;
	uint rc, oldlevel;
	uint thresh_save[16];
	static uint seg_reg;
	static uint gem_start;
	int parityrc;
	static label_t jmpbuf;
	volatile uint *thresh_reg;

#ifdef GEM_DBUG
	printf("gem_dma: entered\n");
#endif GEM_DBUG

	/***************************************************************/
	/* Initialize local variables                                  */
	/***************************************************************/
	FIND_GP(pdev,pproc);
	ddf = (struct gem_ddf *) vp->display->free_area;
	ld = (struct gemini_data *) vp->vttld;

	/***************************************************************/
	/* Check DMA arg flags                                         */
	/***************************************************************/
	if (arg->flags & DMA_WAIT)
	{
		if (parityrc = (setjmpx(&jmpbuf)))
		{
		    if (parityrc == EXCEPT_IO)
		    {
			gemlog(NULL,ld->component,"gem_dma",
			       "setjmpx",parityrc,NULL,UNIQUE_1);
			errno = EIO;
			return(EIO);
		     }
		     else
			  longjmpx(parityrc);
		}
		seg_reg = BUSMEM_ATT(BUS_ID,0);
		gem_start = (uint)((char *)ld->a_gem_gmem) + seg_reg;

		/*******************************************************/
		/* The request is to start the DMA operation and 'wait'*/
		/* for it to complete. To do this we will mask off     */
		/* system interrupts, enable DMA Complete interrupt on */
		/* the adapter and start the operation by setting the  */
		/* DMA Destination Address register and then sleeping. */
		/*******************************************************/
		ddf->cmd = DMA_IN_PROGRESS | SYNC_WAIT_CMD;
		oldlevel = i_disable(INTMAX);
		ddf->sleep_addr = EVENT_NULL;

		/*******************************************************/
		/* Sleep here to wait for DMA to complete              */
		/*******************************************************/
		ddf->watch_event.sleep_addr = (caddr_t) &ddf->sleep_addr;
		ddf->watch_event.ddf = ddf;
		w_start(&ddf->watch_event.w_event);

		start_dma(ld,arg,thresh_save,indx,gem_start);
		rc = e_sleep(&(ddf->sleep_addr), EVENT_SHORT);

		w_stop(&ddf->watch_event.w_event);
		i_enable(oldlevel);

		if (rc == EVENT_SUCC)
		{
		  /********************************************************/
		  /* The DMA Operation has completed                      */
		  /********************************************************/
	   /*       ddf->cmd = NULL_CMD;        */
		  ddf->sleep_addr = EVENT_NULL;

		  /********************************************************/
		  /* Restore Original Threshold settings (someone may have*/
		  /* changed them using for other reasons)                */
		  /********************************************************/
		  thresh_reg = (uint *) (gem_start | INTR_HIGH_0);
		  *thresh_reg++ = thresh_save[0];
		  *thresh_reg++ = thresh_save[1];
		  *thresh_reg++ = thresh_save[2];
		  *thresh_reg   = thresh_save[3];

		  thresh_reg = (uint *) (gem_start | INTR_HIGH_1);
		  *thresh_reg++ = thresh_save[4];
		  *thresh_reg++ = thresh_save[5];
		  *thresh_reg++ = thresh_save[6];
		  *thresh_reg   = thresh_save[7];

		  thresh_reg = (uint *) (gem_start | INTR_HIGH_2);
		  *thresh_reg++ = thresh_save[8];
		  *thresh_reg++ = thresh_save[9];
		  *thresh_reg++ = thresh_save[10];
		  *thresh_reg   = thresh_save[11];

		  thresh_reg = (uint *) (gem_start | INTR_HIGH_3);
		  *thresh_reg++ = thresh_save[12];
		  *thresh_reg++ = thresh_save[13];
		  *thresh_reg++ = thresh_save[14];
		  *thresh_reg   = thresh_save[15];

		 /***********************************************/
		 /* Invoke callback routine                     */
		 /***********************************************/
		 clrjmpx(&jmpbuf);
		 BUSMEM_DET(seg_reg);

		 (*callback)(pproc,indx);
		 rc = SUCCESS;
	     }
	 }       /* end of dma WAIT */
	else
	{
		/*******************************************************/
		/* 'wait' was not specified, thus the DMA operation is */
		/* started and control returned to caller.             */
		/*******************************************************/
		if (parityrc = (setjmpx(&jmpbuf)))
		{
		     if (parityrc == EXCEPT_IO)
		     {
			  gemlog(NULL,ld->component,"gem_dma",
				 "setjmpx",parityrc,NULL,UNIQUE_2);
			  errno = EIO;
			  return(EIO);
		      }
		      else
			    longjmpx(parityrc);
		}
		seg_reg = BUSMEM_ATT(BUS_ID,0);
		gem_start = (uint)((char *)ld->a_gem_gmem) + seg_reg;
		ddf->cmd = DMA_IN_PROGRESS | ASYNC_CMD;
		ddf->callback = callback;
		ddf->callbackarg = pproc;
		ddf->callbackindx = indx;
		oldlevel = i_disable(INTMAX);
		rc = start_dma(ld, arg, NULL,indx,gem_start);
		i_enable(oldlevel);
		clrjmpx(&jmpbuf);
		BUSMEM_DET(seg_reg);
	}

#ifdef GEM_DBUG
	printf("gem_dma: exiting\n");
#endif GEM_DBUG

	return(rc);
}

/**********************************************************************/
/*                                                                    */
/* START_DMA:  Subroutine                                             */
/*                                                                    */
/* Function:  Enable DMA interrupts and initiate DMA                  */
/*                                                                    */
/**********************************************************************/
int start_dma(ld, arg,tr_save,indx, gem_start)
struct gemini_data *ld;
gscdma *arg;
uint *tr_save[];
uint indx, gem_start;
{

	struct _gem_cmd *destptr;
	volatile unsigned int *dma_intr_ena;
	volatile unsigned int *dma_dest_addr;
	volatile uint *thresh_reg;

	/*******************************************************/
	/* Disable Threshold Interrupts and save the current   */
	/* Threshold values and set them to default values     */
	/*******************************************************/
	if (tr_save != NULL)
	{
	    thresh_reg = (uint *) (gem_start | INTR_HIGH_0);
	    tr_save[0] = *thresh_reg;
	    *thresh_reg++ = DISABLE_THRESH;
	    tr_save[1] = *thresh_reg;
	    *thresh_reg++ = DISABLE_THRESH;
	    tr_save[2] = *thresh_reg;
	    *thresh_reg++ = HIGH_DMA_THRESH;
	    tr_save[3]  = *thresh_reg;
	    *thresh_reg = LOW_DMA_THRESH;

	    thresh_reg = (uint *) (gem_start | INTR_HIGH_1);
	    tr_save[4] = *thresh_reg;
	    *thresh_reg++ = DISABLE_THRESH;
	    tr_save[5] = *thresh_reg;
	    *thresh_reg++ = DISABLE_THRESH;
	    tr_save[6] = *thresh_reg;
	    *thresh_reg++ = HIGH_DMA_THRESH;
	    tr_save[7] = *thresh_reg;
	    *thresh_reg = LOW_DMA_THRESH;

	    thresh_reg = (uint *) (gem_start | INTR_HIGH_2);
	    tr_save[8] = *thresh_reg;
	    *thresh_reg++ = DISABLE_THRESH;
	    tr_save[9] = *thresh_reg;
	    *thresh_reg++ = DISABLE_THRESH;
	    tr_save[10] = *thresh_reg;
	    *thresh_reg++ = HIGH_DMA_THRESH;
	    tr_save[11] = *thresh_reg;
	    *thresh_reg = LOW_DMA_THRESH;

	    thresh_reg = (uint *) (gem_start | INTR_HIGH_3);
	    tr_save[12] = *thresh_reg;
	    *thresh_reg++ = DISABLE_THRESH;
	    tr_save[13] = *thresh_reg;
	    *thresh_reg++ = DISABLE_THRESH;
	    tr_save[14] = *thresh_reg;
	    *thresh_reg++ = HIGH_DMA_THRESH;
	    tr_save[15] = *thresh_reg;
	    *thresh_reg = LOW_DMA_THRESH;
	 }

	/***********************************************************/
	/* Start the DMA operation by writing to the DMA           */
	/* Destination Address register on the adapter             */
	/***********************************************************/
	dma_intr_ena = (unsigned int *)(gem_start | ENA_DMA_COMP);
	*dma_intr_ena = ENA_DMA_INTR;
	dma_dest_addr = (unsigned int *)(gem_start | DMA_DEST_ADDR);
	destptr = (struct _gem_cmd *) arg->dma_cmd;
	*dma_dest_addr = destptr->dma_dest[indx];
	return(SUCCESS);
}
