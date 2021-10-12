static char sccsid[] = "@(#)22        1.14.2.5  src/bos/kernext/disp/ped/dma/middma.c, peddd, bos411, 9428A410j 4/21/94 10:44:44";
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: diag_svc
 *              fifo_tokens
 *              pcb_tokens
 *              vttdma
 *              vttdma_setup
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
#include <sys/xmem.h>
#include <vt.h>
#include <sys/rcm_win.h>
#include <sys/aixgsc.h>
#include <sys/rcm.h>
#include <sys/syspest.h>

#include <sys/sleep.h>


#include "mid.h"
#include "rcm_mac.h"
#include "hw_dd_model.h"
#include "midhwa.h"
#include "midctx.h"             /* contains DMA_COMPLETE flags */
#include "midddf.h"
#include "hw_PCBkern.h"
#include "hw_macros.h"
#include "hw_typdefs.h"
#include "hw_errno.h"
#include "hw_regs_k.h"

#include "hw_regs_u.h"
#include "hw_FIFrms.h"

#include "mid_dd_trace.h"

MID_MODULE(middma);

BUGXDEF(dbg_middd);

#define MID_DMA_TIMEDOUT        0xFFFFFFFF

long fifo_tokens(ddf,middma)
struct midddf   *ddf;
struct mid_dma  *middma;
{
	long            size;           /* # of words in structure element   */
	ulong           *data;          /* scratch ptr to se's               */
	volatile long   i;              /* loop variable                     */
	long            rc = 0;         /* return code                       */
	long            flush = 0x40000;/* value to send FIFO to flush       */

	HWPDDFSetup;                    /* gain access to the adapter        */


	BUGLPR(dbg_middma, BUGNTA, ("Entering fifo_tokens\n"));

	size = middma->se_size;
	data = middma->se_data;


	BUGLPR(dbg_middma, BUGNTA, ("se_size = 0x%x\n", size));

	/*------------------------------------------------------------*/
	/* Send down the data to the fifo.                            */
	/*------------------------------------------------------------*/


	BUGLPR(dbg_middma, BUGNTA,("Dump middma struct...\n"));
	BUGLPR(dbg_middma, BUGNTA,("middma->flags=0x%x\n",
	                middma->flags));
	BUGLPR(dbg_middma, BUGNTA,("middma->se_size=0x%x\n",
	                middma->se_size));
	BUGLPR(dbg_middma, BUGNTA,("middma->se_data ptr=0x%x\n",
	                middma->se_data));
	BUGLPR(dbg_middma, BUGNTA, ("about to call fifo_\n"));


	MID_WR_FIFO(data,size);


	BUGLPR(dbg_middma, BUGNTA, ("Leaving fifo_tokens\n"));

	return(rc);
}









long pcb_tokens(ddf,middma)
struct midddf   *ddf;
struct mid_dma  *middma;
{
	long            size;           /* # of words in structure element   */
	ulong           *data;          /* scratch ptr to se's               */
	volatile long   i;              /* loop variable                     */
	long            rc = 0;         /* return code                       */
	HWPDDFSetup;                    /* gain access to the adapter        */

	BUGLPR(dbg_middma, BUGNTA, ("Entering pcb_tokens\n"));

	size = middma->se_size;
	data = middma->se_data;

	BUGLPR(dbg_middma, BUGNTA, ("se_size = 0x%x\n", size));
	BUGLPR(dbg_middma, BUGNTA, ("se_data = 0x%x\n", data));

	if ( middma->flags == MID_DMA_DIAGNOSTICS )
	{
	        DEFINE_PCB_REGISTER_ADDRESSES
	        _MID_WR_PCB_1_8_NOWAIT( data, 8 )
	}
	else
	{
	        MID_WR_PCB(data,size) ;
	}


	BUGLPR(dbg_middma, BUGNTA, ("Leaving pcb_tokens\n"));
	return(rc);
}








/*---------------------------------------------------------------------------*/
/*                                                                           */
/*   IDENTIFICATION: diag_svc                                                */
/*                                                                           */
/*   DESCRIPTIVE NAME:  Direct Memory access routine for Diagnostics         */
/*                                                                           */
/*   FUNCTION:  Provide hardware dependent function to support DMA           */
/*              operations for diagnostics microcode.                        */
/*                                                                           */
/*   INPUTS:    GSC Device pointer                                           */
/*              Pointer to gscdma structure describing transfer              */
/*              Pointer to callback function provided in the device-         */
/*                      independent layer                                    */
/*              Pointer to callback function argument.                       */
/*                                                                           */
/*   OUTPUTS:                                                                */
/*                                                                           */
/*   CALLED BY: gscdma (device-independent layer)                            */
/*                                                                           */
/*   CALLS:                                                                  */
/*                                                                           */
/*---------------------------------------------------------------------------*/

long diag_svc(pdev,arg,callback)
gscDev *pdev;
struct _gscdma *arg;
int (*callback)();
{

	struct mid_dma  *middma;
	ulong           old_hostaddr;
	struct midddf *ddf = (struct midddf *) pdev->devHead.display->free_area;
	long            rc = 0;
	volatile long   i;              /* loop variable                     */
	mid_dma_write_t *data;          /* scratch ptr to se's               */
	volatile ulong  dsp_status;     /* scratch data for pcb status word  */

	int     old_priority;

	HWPDDFSetup;                    /* gain access to the adapter        */

	BUGLPR(dbg_middma, BUGNTA, ("Entering (MID) diag_svc.\n"));

	middma = (struct mid_dma *) arg->dma_cmd;
	data = (mid_dma_write_t *)middma->se_data;

	BUGLPR(dbg_middma, BUGNTA, ("DUMPING ddma.....\n"));
	BUGLPR(dbg_middma, BUGNTA, ("middma->flags=0x%x\n",middma->flags));
	BUGLPR(dbg_middma, BUGNTA, ("middma->se_size=%d\n",middma->se_size));
	BUGLPR(dbg_middma, BUGNTA, ("middma->se_data=0x%x\n",middma->se_data));
	BUGLPR(dbg_middma, BUGNTA, (" \n"));

	BUGLPR(dbg_middma, BUGNTA, ("DUMPING mid_dma_wr.....\n"));
	BUGLPR(dbg_middma, BUGNTA, ("data->length=%d\n",data->length));
	BUGLPR(dbg_middma, BUGNTA, ("data->opcode=%d\n",data->opcode));
	BUGLPR(dbg_middma, BUGNTA, ("data->corr=%d\n",data->corr));
	BUGLPR(dbg_middma, BUGNTA, ("data->bflags=0x%x\n",data->bflags));
	BUGLPR(dbg_middma, BUGNTA, ("data->hostaddr=0x%x\n",data->hostaddr));
	BUGLPR(dbg_middma, BUGNTA, ("data->stride=%d\n",data->stride));
	BUGLPR(dbg_middma, BUGNTA, ("data->destinbp=%d\n",data->destinbp));
	BUGLPR(dbg_middma, BUGNTA, ("data->destinx=%d\n",data->destinx));
	BUGLPR(dbg_middma, BUGNTA, ("data->destiny=%d\n",data->destiny));
	BUGLPR(dbg_middma, BUGNTA, ("data->width=%d\n",data->width));
	BUGLPR(dbg_middma, BUGNTA, ("data->height=%d\n",data->height));
	BUGLPR(dbg_middma, BUGNTA, ("data->dbpos=%d\n",data->dbpos));
	BUGLPR(dbg_middma, BUGNTA, ("data->xrepl=%d\n",data->xrepl));
	BUGLPR(dbg_middma, BUGNTA, ("data->yrepl=%d\n",data->yrepl));
	BUGLPR(dbg_middma, BUGNTA, ("data->fgcolor=%d\n",data->fgcolor));
	BUGLPR(dbg_middma, BUGNTA, ("data->bgcolor=%d\n",data->bgcolor));


	BUGLPR(dbg_middma, BUGNTA, ("flags = 0x%x\n", middma->flags));
	BUGLPR(dbg_middma, BUGNTA, ("se_size = 0x%x\n", middma->se_size));
	BUGLPR(dbg_middma, BUGNTA, ("se_data = 0x%x\n", data));
#if 0
	for (i = 0;i < 16;i++)
	        BUGLPR(dbg_middma,BUGNTA,("el = 0x%x\n", data[i]));
#endif

	BUGLPR(dbg_middma, BUGNTA, ("past input printfs \n"));

	/*--------------------------------------------------------------------*/
	/* All mid dma structures have the host address at the third          */
	/* word of the structure.   Since this is the case, any structure     */
	/* can be used to dereference the structure element and load the      */
	/* bus address.  Arbitrarily I choose the dma read structure.         */
	/*--------------------------------------------------------------------*/

	old_hostaddr = data->hostaddr;
	data->hostaddr &= 0xFFF;
	data->hostaddr |= pdev->devHead.display->d_dma_area[0].bus_addr;
	BUGLPR(dbg_middma,BUGACT,("hostaddr = 0x%x\n", data->hostaddr));


	BUGLPR(dbg_middma, BUGNTA, ("about to open the bus\n"));
	PIO_EXC_ON();

	BUGLPR(dbg_middma, BUGNTA, ("just opened the bus\n"));

	/*------------------------------------------------------------*/
	/* Disable interrupt.                                         */
	/*------------------------------------------------------------*/

	old_priority = i_disable(pdev->devHead.display->interrupt_data.intr.priority-1);

	ddf->dma_sleep_flags = 0;

	switch (middma->flags) {
	        case MID_DMA_READ:
	        case MID_DMA_WRITE:
	                BUGLPR(dbg_middma, BUGNTA,("Dump middma struct...\n"));
	                BUGLPR(dbg_middma, BUGNTA,("middma->flags=0x%x\n",
	                                                middma->flags));
	                BUGLPR(dbg_middma, BUGNTA,("middma->se_size=0x%x\n",
	                                                middma->se_size));
	                BUGLPR(dbg_middma, BUGNTA,("middma->se_data ptr=0x%x\n",
	                                                middma->se_data));
	                BUGLPR(dbg_middma, BUGNTA, ("about to call fifo_\n"));
	                fifo_tokens(ddf,middma);
	        break;

	        case MID_DMA_PCB_READ:
	        case MID_DMA_PCB_WRITE:
	        case MID_DMA_SE_WRITE:
	        case MID_DMA_DIAGNOSTICS:
	                BUGLPR(dbg_middma, BUGNTA, ("about to call pcb_\n"));
	                pcb_tokens(ddf,middma);
	        break;

	        default:
	        BUGLPR(dbg_middma, 0, ("bad DMA flag 0x%x\n", middma->flags));

	        }


	/*-----------------------------------------------------------*/
	/* Do we want to force an error?                             */
	/*-----------------------------------------------------------*/

	if (middma->flags & MID_DMA_FORCE_ERROR)
	{
	        BUGLPR(dbg_middd, BUGACT, ("Forcing an error.\n"));
	        BUGLPR(dbg_middd, 0, ("NOT YET IMPLEMENTED.\n"));
	}

	ddf->dcallback = callback;
	data->hostaddr = old_hostaddr;

	if ( !( arg->flags & DMA_WAIT ) )
	{
	        BUGLPR(dbg_middma, BUGNTA,
	        ("DMA has been issued with no-wait.\n"));
	        ddf->dsp_status = 0;
	        ddf->cmd = DMA_NOWAIT;
	        i_enable(old_priority);
	        PIO_EXC_OFF();
	        BUGLPR(dbg_middma, BUGNTA, ("Leaving diag_svc.\n"));
	        return (0);
	}

	/*------------------------------------------------------------*/
	/* Enable interrupt.  Clear status word.                      */
	/*------------------------------------------------------------*/

	ddf->dsp_status = 0;
	ddf->cmd = DMA_WAIT_REQ;

	/*------------------------------------------------------------
	  This "while" loop is simply to ensure that we actually sleep
	  at this point.  There have been occaissions where sleep has
	  not worked, so this forces "e_sleep" calls until the flag is
	  set in the interrupt handler.
	------------------------------------------------------------*/

	while (ddf->dma_sleep_flags != 0x10)
	{
	        e_sleep( &(ddf->dma_sleep_addr), EVENT_SHORT );

	        if(ddf->dma_sleep_flags != 0x10)
	                BUGLPR(dbg_middma, 1, ("Did not sleep\n"));

	}

	i_enable(old_priority);


	/* ---------------------------------------------------------
	ddf->dma_result gets set in intr_dsp.c and midintr.c.  The
	values set are MID_DMA_NOERROR , if the DMA was successful,
	or MID_DMA_BAD_STRUCT, if the dsp status was 4006.  At present
	MID_DMA_TIMEDOUT is not used anywhere.  When the watch dog
	timer is implemented and it 'pops, we should set ddf->dma_result
	to MID_DMA_TIMEDOUT.
	------------------------------------------------------------ */

	middma->error = ddf->dma_result;

	if (middma->error == MID_DMA_TIMEDOUT)
	{

	        /*------------------------*/
	        /* Hard reset the adapter */
	        /*------------------------*/

	        BUGLPR(dbg_middma, 0, ("DMA timed out.\n"));

	        mid_init(pdev->devHead.display);

	        PIO_EXC_OFF();

	        copy_ps(pdev->devHead.display->visible_vt);

	        return -1;
	}

	BUGLPR(dbg_middma, BUGNTA,
	        ("DMA has been woken up: successful completion.\n"));

	PIO_EXC_OFF();
	BUGLPR(dbg_middma, BUGNTA, ("Leaving diag_svc with rc = %d.\n",rc));
	return rc;
}





long vttdma_setup(pdev, arg)
gscDev  *pdev;
gscdma  *arg;
{
	return(0);
}







long
vttdma(pdev,arg,callback,vp,indx)
gscDev  *pdev;
gscdma  *arg;
int     (*callback)();
struct  vtmstruc *vp;
int     indx;
{
	struct mid_dma  *middma;
	ulong           old_hostaddr;
	struct midddf *ddf = (struct midddf *) pdev->devHead.display->free_area;
	long            rc = 0;
	volatile long   i;              /* loop variable                     */
	mid_dma_write_t *data;          /* scratch ptr to se's               */
	volatile ulong  dsp_status;     /* scratch data for pcb status word  */
	struct _rcmProc *pproc;         /* pointer to rcm process structure  */

	int     old_priority;

	HWPDDFSetup;                    /* gain access to the adapter        */



	BUGLPR(dbg_middma, BUGNTA, ("Entering (MID) vttdma.\n"));

	middma = (struct mid_dma *) arg->dma_cmd;
	data = (mid_dma_write_t *)middma->se_data;






	MID_DD_ENTRY_TRACE (middma, 1, DMA, ddf, pdev,
	                        data->opcode, data->length, indx) ;


	BUGLPR(dbg_middma, BUGNTA, ("DUMPING ddma.....\n"));
	BUGLPR(dbg_middma, BUGNTA, ("middma->flags=0x%x\n",middma->flags));
	BUGLPR(dbg_middma, BUGNTA, ("middma->se_size=%d\n",middma->se_size));
	BUGLPR(dbg_middma, BUGNTA, ("middma->se_data=0x%x\n",middma->se_data));
	BUGLPR(dbg_middma, BUGNTA, (" \n"));

	BUGLPR(dbg_middma, BUGNTA, ("DUMPING mid_dma_wr.....\n"));
	BUGLPR(dbg_middma, BUGNTA, ("data->length=%d\n",data->length));
	BUGLPR(dbg_middma, BUGNTA, ("data->opcode=%d\n",data->opcode));
	BUGLPR(dbg_middma, BUGNTA, ("data->corr=%d\n",data->corr));
	BUGLPR(dbg_middma, BUGNTA, ("data->bflags=0x%x\n",data->bflags));
	BUGLPR(dbg_middma, BUGNTA, ("data->hostaddr=0x%x\n",data->hostaddr));
	BUGLPR(dbg_middma, BUGNTA, ("data->stride=%d\n",data->stride));
	BUGLPR(dbg_middma, BUGNTA, ("data->destinbp=%d\n",data->destinbp));
	BUGLPR(dbg_middma, BUGNTA, ("data->destinx=%d\n",data->destinx));
	BUGLPR(dbg_middma, BUGNTA, ("data->destiny=%d\n",data->destiny));
	BUGLPR(dbg_middma, BUGNTA, ("data->width=%d\n",data->width));
	BUGLPR(dbg_middma, BUGNTA, ("data->height=%d\n",data->height));
	BUGLPR(dbg_middma, BUGNTA, ("data->dbpos=%d\n",data->dbpos));
	BUGLPR(dbg_middma, BUGNTA, ("data->xrepl=%d\n",data->xrepl));
	BUGLPR(dbg_middma, BUGNTA, ("data->yrepl=%d\n",data->yrepl));
	BUGLPR(dbg_middma, BUGNTA, ("data->fgcolor=%d\n",data->fgcolor));
	BUGLPR(dbg_middma, BUGNTA, ("data->bgcolor=%d\n",data->bgcolor));


	/*--------------------------------------------------------------------*
	    Get the proces structure pointer.  Someday this will be passed
	    from the device independent layer, but for today, we will use
	    the FIND_GP macro.
	 *--------------------------------------------------------------------*/

	FIND_GP (pdev, pproc) ;

	ddf->pProc_dma = pproc ;

	ASSERT(pproc != NULL);
	BUGLPR(dbg_middma, BUGNTA, ("pproc = 0x%x\n", pproc));



	/*--------------------------------------------------------------------*
	    Echo the flags, size and data again.   (I don't know why)
	 *--------------------------------------------------------------------*/

	BUGLPR(dbg_middma, BUGNTA, ("flags = 0x%x\n", middma->flags));
	BUGLPR(dbg_middma, BUGNTA, ("se_size = 0x%x\n", middma->se_size));
	BUGLPR(dbg_middma, BUGNTA, ("se_data = 0x%x\n", data));
#if 0
	for (i = 0;i < 16;i++)
	        BUGLPR(dbg_middma,BUGNTA,("el = 0x%x\n", data[i]));
#endif

	BUGLPR(dbg_middma, BUGNTA, ("past input printfs \n"));





	/*--------------------------------------------------------------------*/
	/* All mid dma structures have the host address at the third          */
	/* word of the structure.   Since this is the case, any structure     */
	/* can be used to dereference the structure element and load the      */
	/* bus address.  Arbitrarily I choose the dma read structure.         */
	/*--------------------------------------------------------------------*/

	old_hostaddr = data->hostaddr;
	data->hostaddr &= 0xFFF;
	data->hostaddr |= pdev->devHead.display->d_dma_area[0].bus_addr;
	BUGLPR(dbg_middma,BUGACT,("hostaddr = 0x%x\n", data->hostaddr));


	BUGLPR(dbg_middma, BUGNTA, ("about to open the bus\n"));
	PIO_EXC_ON();

	BUGLPR(dbg_middma, BUGNTA, ("just opened the bus\n"));


	 /****************************************************************
	   Force our context to be current (and guard the domain to
	   any further context switches).
	 ****************************************************************/


	BUGLPR(dbg_middma, BUGNTA,("Guarding domain\n."));

	(*vp->display->pGSC->devHead.pCom->rcm_callback->make_cur_and_guard_dom)
	                ( (pproc-> pDomainCur[0]) );



	 /****************************************************************
	   Now disable interrupts to prevent the completion from occuring
	   before we are set up to handle it.
	 ****************************************************************/
	old_priority = i_disable(vp->display->interrupt_data.intr.priority-1);

	ddf->dma_sleep_flags = 0;

	switch (middma->flags) {
	        case MID_DMA_READ:
	        case MID_DMA_WRITE:
	                BUGLPR(dbg_middma, BUGNTA,("Dump middma struct...\n"));
	                BUGLPR(dbg_middma, BUGNTA,("middma->flags=0x%x\n",
	                                                middma->flags));
	                BUGLPR(dbg_middma, BUGNTA,("middma->se_size=0x%x\n",
	                                                middma->se_size));
	                BUGLPR(dbg_middma, BUGNTA,("middma->se_data ptr=0x%x\n",
	                                                middma->se_data));
	                BUGLPR(dbg_middma, BUGNTA, ("about to call fifo_\n"));
	                fifo_tokens(ddf,middma);
	        break;

	        case MID_DMA_PCB_READ:
	        case MID_DMA_PCB_WRITE:
	        case MID_DMA_SE_WRITE:
	        case MID_DMA_DIAGNOSTICS:
	                BUGLPR(dbg_middma, BUGNTA, ("about to call pcb_\n"));
	                pcb_tokens(ddf,middma);
	        break;

	        default:
	        BUGLPR(dbg_middma, 0, ("bad DMA flag 0x%x\n", middma->flags));

	        }


	/*-----------------------------------------------------------*/
	/* Do we want to force an error?                             */
	/*-----------------------------------------------------------*/

	if (middma->flags & MID_DMA_FORCE_ERROR)
	{
	        BUGLPR(dbg_middd, BUGACT, ("Forcing an error.\n"));
	        BUGLPR(dbg_middd, 0, ("NOT YET IMPLEMENTED.\n"));
	}

	ddf->dcallback = callback;
	data->hostaddr = old_hostaddr;

	if ( !( arg->flags & DMA_WAIT ) )
	{
	        BUGLPR(dbg_middma, BUGNTA,
	        ("DMA has been issued with no-wait.\n"));
	        ddf->dsp_status = 0;
	        ddf->cmd = DMA_NOWAIT;
	        i_enable(old_priority);
	        PIO_EXC_OFF();
	        BUGLPR(dbg_middma, BUGNTA, ("Leaving vttdma.\n"));
	        return (0);
	}

	/*------------------------------------------------------------*/
	/* Enable interrupt.                                          */
	/* Unguard domain                                             */
	/*------------------------------------------------------------*/

	ddf->cmd = DMA_WAIT_REQ;

	/*------------------------------------------------------------
	  This "while" loop is simply to ensure that we actually sleep
	  at this point.  There have been occaissions where sleep has
	  not worked, so this forces "e_sleep" calls until the flag is
	  set in the interrupt handler.
	------------------------------------------------------------*/

	while (ddf->dma_sleep_flags != 0x10)
	{
	        MID_DD_TRACE_PARMS (middma, 2, DMA_SLEEP, ddf, pdev,
	                        ddf->cmd, callback, middma->se_size) ;

	        e_sleep( &(ddf->dma_sleep_addr), EVENT_SHORT );

	        if(ddf->dma_sleep_flags != 0x10)
	                BUGLPR(dbg_middma, 1, ("Did not sleep\n"));

	}


	i_enable(old_priority);


	BUGLPR(dbg_middma, BUGNTA, ("Awake after DMA.\n",i));

	BUGLPR(dbg_middma, BUGNTA,("Unguard domain\n."));
	(*vp->display->pGSC->devHead.pCom->rcm_callback->unguard_domain)
	                ( &(vp->display->pGSC->domain[0] ) );

	/* ---------------------------------------------------------
	ddf->dma_result gets set in intr_dsp.c and midintr.c.  The
	values set are MID_DMA_NOERROR , if the DMA was successful,
	or MID_DMA_BAD_STRUCT, if the dsp status was 4006.  At present
	MID_DMA_TIMEDOUT is not used anywhere.  When the watch dog
	timer is implemented and it 'pops, we should set ddf->dma_result
	to MID_DMA_TIMEDOUT.
	------------------------------------------------------------ */

	middma->error = ddf->dma_result;

	/*-----------------------------------------------------------*/
	/* If DMA waited too long, reset the adapter.                */
	/*-----------------------------------------------------------*/

	if (middma->error == MID_DMA_TIMEDOUT)
	{

	        /*------------------------*/
	        /* Hard reset the adapter */
	        /*------------------------*/


	        mid_init(pdev->devHead.display);

	        PIO_EXC_OFF();

	        copy_ps(pdev->devHead.display->visible_vt);

	        return -1;
	}


	BUGLPR(dbg_middma, BUGNTA,
	        ("DMA has been woken up: successful completion.\n"));

	PIO_EXC_OFF();


	MID_DD_EXIT_TRACE (middma, 2, DMA, ddf, pdev,
	                        ddf->dma_result, rc, 0xFFF0) ;

	BUGLPR(dbg_middma, BUGNTA, ("Leaving vttdma with rc = %d.\n",rc));
	return rc;
}
