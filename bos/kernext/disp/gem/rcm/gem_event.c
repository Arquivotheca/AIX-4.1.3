static char sccsid[] = "@(#)77	1.4.3.4  src/bos/kernext/disp/gem/rcm/gem_event.c, sysxdispgem, bos411, 9428A410j 10/20/93 13:25:51";
/*
 *   COMPONENT_NAME: SYSXDISPGEM
 *
 *   FUNCTIONS: *		BUGVDEF
 *		enable_vme_intr
 *		gem_async_mask
 *		gem_enable_event
 *		gem_sync_mask
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

/***********************************************************************/
/* COMPONENT_NAME: (GEMINIDD) AIX Virtual Display Driver               */
/*                                                                     */
/* FUNCTIONS: GEM_SYNC_MASK                                            */
/*                                                                     */
/* DESCRIPTION:       This routine set up the mask for synchronous     */
/*                    events and indicates the function to call        */
/*                    when the event completes.                        */
/*                                                                     */
/***********************************************************************/
gem_sync_mask(vp, event_req, callback)
     struct vtmstruc *vp;
     struct wait_event *event_req;
     int (*callback)();
{
      struct gem_ddf *ddf;
      uint oldlevel, tst_mask;
      int rc = EVENT_SUCC;

#ifdef GEM_DBUG
 printf("gem_syncm: entered\n");
#endif GEM_DBUG

      /*****************************************************************/
      /* Initialize local variables. Use FIND_GP macro to determine if */
      /* the calling process is a graphics process, then setup to point*/
      /* to RCM proc structure.                                        */
      /*****************************************************************/
      ddf = (struct gem_ddf *) vp->display->free_area;
      FIND_GP(vp->display->pGSC,vp->display->cur_rcm);

      /*****************************************************************/
      /* If the event request is 'wait'(synchronous wait) then set up  */
      /* command in ddf and sleep to 'wait' for event.                 */
      /*****************************************************************/
      if (event_req->wait == gto_WAIT)
      {
	   /************************************************************/
	   /* Mask off class 3 interrupts                              */
	   /************************************************************/
	   oldlevel = i_disable(INTMAX);
	   ddf->sync_event_mask |= event_req->mask;

	   /************************************************************/
	   /* If there are no events to report(report.event = 0), then */
	   /* put the process to sleep, else the event has taken place */
	   /* to we just have to return the results.                   */
	   /************************************************************/
	   if (ddf->report.event == 0)
	   {
		/*******************************************************/
		/* Request is to 'wait' for a synchronous event        */
		/*******************************************************/
		ddf->sleep_addr = EVENT_NULL;

		/*******************************************************/
		/* Verify event request                                */
		/*******************************************************/
		tst_mask = PICK_EVENT | FIFO_SYNC_EVENT |
				   THRESHOLD_EVENT | FIFO_SYNC_CNT_EVENT;
		if ((event_req->mask == 0) |
					 (event_req->mask & ~tst_mask))
		{
			i_enable(oldlevel);
			gemlog(vp,NULL,"gem_sync_mask","gem_sync_mask",
						   ERROR,NULL,UNIQUE_1);
			return(ERROR);
		}
		else
			 ddf->cmd |= SYNC_WAIT_CMD;

		/*******************************************************/
		/* Enable adapter interrupts                           */
		/*******************************************************/
		rc = enable_vme_intr(vp,1);
		if (rc != SUCCESS)
		{
		    i_enable(oldlevel);
		    return(rc);
		}

		/*******************************************************/
		/* Sleep here to wait for event                        */
		/*******************************************************/
		ddf->watch_event.sleep_addr = (caddr_t) &ddf->sleep_addr;
		ddf->watch_event.ddf = ddf;
		w_start(&ddf->watch_event.w_event);

		/* HAK added because sleep does not always work   */
		ddf->sleep_flags = 0;
	        do 
		{    
		   rc = e_sleep(&(ddf->sleep_addr), EVENT_SHORT);
	        }             
		while (ddf->sleep_flags == 0);
		w_stop(&ddf->watch_event.w_event);
	   }

	   /************************************************************/
	   /* If we have successfully serviced the event, then clear   */
	   /* ddf command and sleep addr fields and return control     */
	   /* to the RCM.                                              */
	   /************************************************************/
	   if (rc == EVENT_SUCC)
	   {
	      /*********************************************************/
	      /* Cleanup                                               */
	      /*********************************************************/
	      ddf->sleep_addr = EVENT_NULL;
	      ddf->sync_event_mask = 0x00;

	      /*********************************************************/
	      /* Move report data into event structure                 */
	      /*********************************************************/
	      event_req->report.event   = ddf->report.event;
	      event_req->report.time    = ddf->report.time;
	      event_req->report.data[0] = ddf->report.data[0];
	      event_req->report.data[1] = ddf->report.data[1];
	      event_req->report.data[2] = ddf->report.data[2];
	      event_req->report.data[3] = ddf->report.data[3];
	      ddf->report.event = 0x00;
	      i_enable(oldlevel);
	   }
	   else
	      /*********************************************************/
	      /* error                                                 */
	      /*********************************************************/
	      {
		 i_enable(oldlevel);
		 return(ERROR);
	      }
      }

      else
	 /**************************************************************/
	 /* If the event request is 'block' then block the current     */
	 /* process using the Immediate FIFO then 'sleep' the process  */
	 /**************************************************************/
	 if (event_req->wait == BLOCKIMM)
	 {
	      ddf->immpid_flags = SLEEPING;
	      uexblock(ddf->imm_pid);
	 }
	 else

	   /************************************************************/
	   /* If the event request is 'block' then block the current   */
	   /* process using the Traversal FIFO then 'sleep' the process*/
	   /************************************************************/
	   if (event_req->wait == BLOCKTRAV)
	   {
		ddf->travpid_flags = SLEEPING;
		uexblock(ddf->trav_pid);
	   }

	   /**********************************************************/
	   /* No wait requested.                                     */
	   /**********************************************************/
	   else
	   {
	      /*********************************************************/
	      /* Initialize the ddf                                    */
	      /*********************************************************/
	      ddf->callback = callback;
	      ddf->sync_event_mask |= event_req->mask;
	      FIND_GP(vp->display->pGSC,vp->display->cur_rcm);
	      tst_mask = PICK_EVENT | FIFO_SYNC_EVENT |
				 THRESHOLD_EVENT | FIFO_SYNC_CNT_EVENT;
	      if ((event_req->mask == 0) | (event_req->mask & ~tst_mask))
	      {
		      gemlog(vp,NULL,"gem_sync_mask","gem_sync_mask",
						 ERROR,NULL,UNIQUE_2);
		      return(ERROR);
	      }
	      else
		       ddf->cmd |= SYNC_NOWAIT_CMD;

	      /*********************************************************/
	      /* Insure that adapter interrupts are enabled.           */
	      /*********************************************************/
	      rc = enable_vme_intr(vp,1);
	 }

#ifdef GEM_DBUG
 printf("gem_syncm: exited\n");
#endif GEM_DBUG

      return(rc);
}

/***********************************************************************/
/* COMPONENT_NAME: (GEMINIDD) AIX Virtual Display Driver               */
/*                                                                     */
/* FUNCTIONS: GEM_ASYNC_MASK                                           */
/*                                                                     */
/* DESCRIPTION:       This routine set up the mask for asynchronous    */
/*                                                                     */
/***********************************************************************/
gem_async_mask(vp, event_req, callback)
     struct vtmstruc *vp;
     struct async_event *event_req;
     int (*callback)();
{
      struct gem_ddf *ddf;
      uint tst_mask;
      uint rc = 0;

#ifdef GEM_DBUG
 printf("gem_asyncm: entered\n");
#endif GEM_DBUG

      /*****************************************************************/
      /* Initialize local variables                                    */
      /*****************************************************************/
      ddf = (struct gem_ddf *) vp->display->free_area;

      /*****************************************************************/
      /* Check event request code. If event number is zero(0), then    */
      /* we must disable adapter interrupts (CVME ONLY)                */
      /*****************************************************************/
      if(event_req->num_events == 0)
      {
	   ddf->cmd = NULL_CMD;
	   ddf->async_event_mask = 0;
	   /************************************************************/
	   /* Disable adapter interrupts (CVME ONLY)                   */
	   /************************************************************/
	   rc = enable_vme_intr(vp,0);
      }
      else
      {
	   /************************************************************/
	   /* Initialize the ddf data structure. Use FIND_GP macro to  */
	   /* determine if the calling process is a graphics process.  */
	   /* If so, setup to point to RCM structure.                  */
	   /************************************************************/
	   ddf->callback = callback;
	   ddf->async_event_mask |= event_req->mask;
	   FIND_GP(vp->display->pGSC,vp->display->cur_rcm);
	   tst_mask = PICK_EVENT | FIFO_SYNC_EVENT |
				 THRESHOLD_EVENT | FIFO_SYNC_CNT_EVENT;
	   if ((event_req->mask == 0) | (event_req->mask & ~tst_mask))
	   {
		    gemlog(vp,NULL,"gem_async_mask","gem_async_mask",
						 ERROR,NULL,UNIQUE_1);
		    return(ERROR);
	   }
	      else
		       ddf->cmd |= ASYNC_CMD;

	   /************************************************************/
	   /* Insure that adapter interrupts are enabled.              */
	   /************************************************************/
	   rc = enable_vme_intr(vp,1);
      }

#ifdef GEM_DBUG
 printf("gem_asyncm: exited\n");
#endif GEM_DBUG

      return(rc);
}

/***********************************************************************/
/*                                                                     */
/* FUNCTION: GEM_ENABLE_event                                          */
/*                                                                     */
/*                                                                     */
/* DESCRIPTION:                                                        */
/*                                                                     */
/***********************************************************************/
gem_enable_event(vp, event_req)
     struct vtmstruc *vp;
     struct enable_event *event_req;
{
      struct gem_ddf *ddf;

#ifdef GEM_DBUG
 printf("gem_enable_event: entered\n");
#endif GEM_DBUG

     /*****************************************************************/
      /* Get addressability to the ddf and save process id if event    */
      /* mask indicates that this is to be done.                       */
      /*****************************************************************/
      ddf = (struct gem_ddf *) vp->display->free_area;
      if (event_req->e_event == SAVE_IMM_PID)
      {
	  ddf->imm_pid = getpid();
	  ddf->immpid_flags = AWAKE;
      }
      else
	if (event_req->e_event == SAVE_TRAV_PID)
	{
	    ddf->trav_pid = getpid();
	    ddf->travpid_flags = AWAKE;
	}

#ifdef GEM_DBUG
 printf("gem_enable_event: exited\n");
#endif GEM_DBUG
      return(SUCCESS);

}

/************************************************************************/
/* Subroutine:  ENABLE_VME_INTR                                         */
/* Description: Enables/Disables cVME adapter interrupts                */
/************************************************************************/
enable_vme_intr(vp,enable)
     struct vtmstruc *vp;
     uint enable;
{
      struct gemini_data *ld;
      uint seg_reg, parityrc;
      volatile unsigned int *gc_r_p;
      label_t jmpbuf;

      ld = (struct gemini_data *) vp->vttld;
      if (parityrc = (setjmpx(&jmpbuf)))
      {
	 if (parityrc == EXCEPT_IO)
	 {
	     gemlog(vp,NULL,"gem_sync_mask","setjmpx",parityrc,
						  NULL,UNIQUE_1);
	     errno = EIO;
	     return(EIO);
	 }
	 else
	     longjmpx(parityrc);
     }

      seg_reg = BUSMEM_ATT(BUS_ID,0x00);
      gc_r_p = (unsigned int *)((ld->a_gem_gmem) + GEM_CONTROL | seg_reg);

      /************************************************************/
      /* Enable or disable CVME Interrupts on the adapter         */
      /************************************************************/
      if (enable == 1)
	 *gc_r_p = ENABLE_INTR;
      else
	 *gc_r_p = 0x04000000;
      clrjmpx(&jmpbuf);
      BUSMEM_DET(seg_reg);
      return(SUCCESS);
}

