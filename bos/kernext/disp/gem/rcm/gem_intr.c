static char sccsid[] = "@(#)18	1.12.3.3  src/bos/kernext/disp/gem/rcm/gem_intr.c, sysxdispgem, bos411, 9428A410j 1/19/93 12:20:28";
/*
 *   COMPONENT_NAME: SYSXDISPGEM
 *
 *   FUNCTIONS: *		adap_error
 *		bus_err
 *		dma_intr
 *		drp_intr
 *		fifo_sync
 *		gcp_intr
 *		gem_intr
 *		gem_timeout_event1310
 *		gem_timeout_imm
 *		gem_timeout_trav1294
 *		sys_fail
 *		thresh_hold
 *		vme_diag
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


#include "gemincl.h"
#include "gem_diag.h"
#include <sys/errno.h>
#define  DMA_EVENT 3

gem_intr(pd)
   struct phys_displays *pd;
{
	int i;
	struct g_r_dblk *g_r_dbp;       /* ptr to GCP to RIOS data bloc
						   in GCP comm area     */
	uint *d_r_cbp, *drp_r_cbp;      /* debug or error ctrl blk
						   in the DRP comm area */
	union  m_stat_r m_stat_r;       /* master status reg in MBC     */
	union ipnd_r ipnd_reg;          /* copy of the intr pend struct */
	union sys_ctl_r sys_ctl_r;      /* copy of system control reg   */
	uint seg_reg, intr_vector;
	uint *vme_iv_r_p;               /* Ptr to VME Interrupt Vector  */
	struct gcp_comm *gcp_com_p;
	struct drp_comm *drp_com_p;
	volatile uint *sys_ctl_p;
	volatile uint *geo_addr_p;
	volatile uint *status_reg;
	volatile uint *gem_ctl_reg;
	uint sav_gadr, status, tmp_val;
	uint start_addr;
	int parityrc, rc;
	label_t jmpbuf;
	struct gem_ddf *ddf;
	char *comp_nm;
	struct gmcrdslots *gcardslots;
	extern time_t time;

	int found = 0;                  /* set to TRUE if intr found    */
	int event = 0;                  /* set to TRUE if event occured */

#ifdef GEM_DBUG
	printf("gem_intr: entered\n");
#endif GEM_DBUG

	/****************************************************************/
	/* Initialize local pointers                                    */
	/****************************************************************/
	gcardslots = (struct gmcrdslots *)pd->interrupt_data.intr_args[1];
	ddf = (struct gem_ddf *) pd->free_area;
	comp_nm = pd->odmdds;

	/****************************************************************/
	/* Set up bus exception handler and get access to adapter       */
	/****************************************************************/
	if (parityrc = setjmpx(&jmpbuf))
	{
	   if (parityrc == EXCEPT_IO)
	   {
		gemlog(NULL,comp_nm,"gem_intr","setjmpx",
						parityrc,NULL,UNIQUE_1);
		return(INTR_SUCC);
	   }
	   else
	      longjmpx(parityrc);
	}

	/****************************************************************/
	/* Get a segment register and initialize local pointers         */
	/****************************************************************/
	seg_reg     = BUSMEM_ATT(BUS_ID,0x00);
	start_addr  = pd->interrupt_data.intr_args[0] + seg_reg;
	vme_iv_r_p  = (uint *) (start_addr + VME_INTR_VECT);
	gcp_com_p   = (struct gcp_comm *) start_addr;
	drp_com_p   = (struct drp_comm *) start_addr;
	sys_ctl_p   = (uint *) (start_addr + SYS_CONTROL);
	geo_addr_p  = (uint *) (start_addr + GEO_ADDR);
	gem_ctl_reg = (uint *) (start_addr + GEM_CONTROL);

	/****************************************************************/
	/* Save current Geographical Address Reg and select Magic card  */
	/****************************************************************/
	sav_gadr = *geo_addr_p;
	*geo_addr_p = gcardslots->magic;

	/****************************************************************/
	/* Check the Status Register for pending interrupts. If Status  */
	/* register is zero(no interrupts pending), then this is NOT    */
	/* our interrupt.                                               */
	/****************************************************************/
	status_reg = (uint *) (start_addr + STATUS_REG);
	status = (*status_reg) & STATUS_INT;
	if (status == 0x00)
	{
		/*******************************************************/
		/* Restore Geographical Address register, release      */
		/* segment register and return a non-zero value to     */
		/* indicate that this is not our interrupt             */
		/*******************************************************/
		*geo_addr_p = sav_gadr;
		BUSMEM_DET(seg_reg);
		clrjmpx(&jmpbuf);
		return(INTR_FAIL);
	}

	/****************************************************************/
	/*          Process the GEMINI Interrupt                        */
	/* Check each interrupt bit since the adapter can present       */
	/* multiple reasons with a single interrupt. Hovever, if there  */
	/* are indications that an error has been detected, then any    */
	/* other interrupt flags are ignore and error logged.           */
	/****************************************************************/

	/****************************************************************/
	/* Initialize the interrupt report data area in the ddf         */
	/****************************************************************/
	ddf->report.time = time;
	ddf->report.rcx = 0;
	ddf->report.wa = 0;
	ddf->report.event = 0;
	ddf->report.data[0] = 0;
	ddf->report.data[1] = 0;
	ddf->report.data[2] = 0;
	ddf->report.data[3] = 0;

	/****************************************************************/
	/* Read MBC Master Status and Interrupt Pending Registers       */
	/****************************************************************/
	m_stat_r.s_m_stat = *(uint *) (start_addr + MASTER_STAT);
	m_stat_r.s_m_stat &= M_STAT_MASK;
	ipnd_reg.s_ipnd_r = *(uint *) (start_addr + INTR_PENDING);
	ipnd_reg.s_ipnd_r &= INTR_PEND_MASK;

#ifdef GEM_DBUG
printf("Status Reg = %08x Inter Pending reg = %08x\n",status,
					  ipnd_reg.s_ipnd_r);
#endif GEM_DBUG

	/****************************************************************/
	/* Check that Master Status Register is not reporting error(s). */
	/* If the MBC Status register is zero, then there are no MBC    */
	/* errors, otherwise it is most likely a bus errer.             */
	/****************************************************************/
	if (!(m_stat_r.s_m_stat == 0))
	{
		/********************************************************/
		/* If this is a CPU BUS Timeout error, ignore this      */
		/* interrupt and continue processing.                   */
		/********************************************************/
		if (ddf->cmd == GEM_DIAG_MODE)
		     ddf->diaginfo[16] = (ulong) m_stat_r.s_m_stat;
		else
		{

		   rc = bus_err(&m_stat_r,comp_nm);
		   *((uint *) (start_addr + MASTER_STAT)) = 0x00;
		   if (rc == CPU_BUS_TIMEOUT)
		   {
		      tmp_val = ((*gem_ctl_reg) & GCR_MASK) | ENABLE_INTR;
		      *gem_ctl_reg = tmp_val;
		      m_stat_r.s_m_stat = 0;
		      found = 2;
		   }
		}
	}

	if (m_stat_r.s_m_stat == 0)
	{
		/********************************************************/
		/*          Check for Sync Counter Interrupts           */
		/********************************************************/
		if ((fifo_sync(&ipnd_reg,ddf,&event)) == SUCCESS)
			found = 1;

		/********************************************************/
		/*          Check for Threshold Interrupts              */
		/********************************************************/
		if ((thresh_hold(&ipnd_reg,ddf,start_addr,&event)) ==
								  SUCCESS)
			found = 2;

		/********************************************************/
		/*          Check for DMA Interrupts                    */
		/********************************************************/
		if ((dma_intr(&ipnd_reg,ddf,&event)) == SUCCESS)
			found = 1;

		/********************************************************/
		/*           Check for CVME BUS Interrupt Pending       */
		/********************************************************/
		if (ipnd_reg.b.cvme_bus_intr)
		{
			/************************************************/
			/* Read the Interrupt Vector Register to        */
			/* determine which card on the adapter caused   */
			/* the interrupt.                               */
			/* Note: Reading this register causes an IACK   */
			/*       cycle on the CVME Bus thus reseting the*/
			/*       interrupt                              */
			/************************************************/
			found = 1;
			intr_vector = (*vme_iv_r_p) & VME_INTR_MASK;
			if (ddf->cmd == GEM_DIAG_MODE)
				vme_diag(intr_vector,ddf);
			else
			{
			  switch (intr_vector)
			  {
			    case GCP_INTR:
				/****************************************/
				/* GCP is cause of CVME Interrupt.      */
				/* Select the GCP, initialize pointer   */
				/* GCP data blocks and read System      */
				/* Control register.                    */
				/****************************************/
				*geo_addr_p = gcardslots->gcp;
				g_r_dbp = (struct g_r_dblk *)
				     (gcp_com_p->g_r_hdbkof + start_addr -
								CVME_ADDR);
				sys_ctl_r.s_sys_ctl = *sys_ctl_p;

				/****************************************/
				/* If GCP Sysfail is on, call routine   */
				/* which will find and log error        */
				/****************************************/
				if (sys_ctl_r.b.sys_fail)
				{
					/********************************/
					/* Reset the CVME sysfail and   */
					/* Re-enable CVME interrupts.   */
					/* Restore Geographical Address */
					/* Register and clear sysfail   */
					/********************************/
					rc = sys_fail(&sys_ctl_r,0,comp_nm);
					tmp_val =
					    ((*gem_ctl_reg) & GCR_MASK) |
					    (ENABLE_CVME_INT | RESET_CVME);
					*gem_ctl_reg = tmp_val;
					tmp_val &= CLR_RESET_CVME;
					*gem_ctl_reg = tmp_val;
					*geo_addr_p = sav_gadr;
					BUSMEM_DET(seg_reg);
					i_reset(pd);
					clrjmpx(&jmpbuf);
					adap_error(ddf,pd,rc);
					return(INTR_SUCC);
				}
				else
				{
					/********************************/
					/* Not reporting a GCP hardware */
					/* error. Call routine to get   */
					/* GCP interrupt information    */
					/********************************/
					 gcp_intr(g_r_dbp,ddf,gcp_com_p,
					      start_addr,&event,comp_nm,pd);
					 found = 1;
				}

				break;

			    case DRP_INTR:
				/****************************************/
				/* DrP is cause of CVME Interrupt.      */
				/* Select the DrP, initialize pointer to*/
				/* DrP data blocks and read System      */
				/* Control register.                    */
				/****************************************/
				*geo_addr_p = gcardslots->drp;
				drp_r_cbp = (uint *)
				    (drp_com_p->d_r_intblk.dbg_erroff +
						   start_addr - CVME_ADDR);
				sys_ctl_r.s_sys_ctl = *sys_ctl_p;

				/****************************************/
				/* If DrP Sysfail is on, call routine   */
				/* which will find and log error        */
				/****************************************/
				if (sys_ctl_r.b.sys_fail)
				{
					/********************************/
					/* Reset the CVME sysfail and   */
					/* Re-enable CVME interrupts.   */
					/* Restore Geographical Address */
					/* Register and clear sysfail   */
					/********************************/
					rc = sys_fail(&sys_ctl_r,1,comp_nm);
					tmp_val =
					    ((*gem_ctl_reg) & GCR_MASK) |
					    (ENABLE_CVME_INT | RESET_CVME);
					*gem_ctl_reg = tmp_val;
					tmp_val &= CLR_RESET_CVME;
					*gem_ctl_reg = tmp_val;
					*geo_addr_p = sav_gadr;
					BUSMEM_DET(seg_reg);
					i_reset(pd);
					clrjmpx(&jmpbuf);
					adap_error(ddf,pd,rc);
					return(INTR_SUCC);
				}
				else
				{
					drp_intr(d_r_cbp,ddf,drp_com_p,
						     start_addr,comp_nm);
					found = 1;
				}

				break;

			    case SHP_INTR:
				/****************************************/
				/* SHP is cause of CVME Interrupt.      */
				/* Select the SHP and read the SYSTEM   */
				/* Control register.                    */
				/****************************************/
				*geo_addr_p = gcardslots->shp;
				sys_ctl_r.s_sys_ctl = *sys_ctl_p;

				/****************************************/
				/* If SHP Sysfail is on, call routine   */
				/* which will find and log error        */
				/****************************************/
				if (sys_ctl_r.b.sys_fail)
				{
					/********************************/
					/* Reset the CVME sysfail and   */
					/* Re-enable CVME interrupts.   */
					/* Restore Geographical Address */
					/* Register and clear sysfail   */
					/********************************/
					rc = sys_fail(&sys_ctl_r,2,comp_nm);
					tmp_val =
					    ((*gem_ctl_reg) & GCR_MASK) |
					    (ENABLE_CVME_INT | RESET_CVME);
					*gem_ctl_reg = tmp_val;
					tmp_val &= CLR_RESET_CVME;
					*gem_ctl_reg = tmp_val;
					*geo_addr_p = sav_gadr;
					BUSMEM_DET(seg_reg);
					i_reset(pd);
					clrjmpx(&jmpbuf);
					adap_error(ddf,pd,rc);
					return(INTR_SUCC);
				}

				break;

			    default:
				/****************************************/
				/* An invalid Vector was returned. This */
				/* is an Unknown CVME Interrupt(i.e.the */
				/* vector register contains an invalid  */
				/* value). Log the error and return     */
				/****************************************/
				gemlog(NULL,comp_nm,"gem_intr","gem_intr",
					     NULL,BAD_VECTOR_ID,UNIQUE_2);
				tmp_val = ((*gem_ctl_reg) &
					      GCR_MASK) | ENABLE_BERR_INT;
				*gem_ctl_reg = tmp_val;
				*geo_addr_p = sav_gadr;
				BUSMEM_DET(seg_reg);
				i_reset(pd);
				clrjmpx(&jmpbuf);
				adap_error(ddf,pd,BAD_VECTOR_ID);
				return(INTR_SUCC);
			  }
			}
			tmp_val = ((*gem_ctl_reg) & GCR_MASK) | ENABLE_CVME_INT;
			*gem_ctl_reg = tmp_val;
		}
		else
		  if (ipnd_reg.b.bus_err_intr)
		  {
			tmp_val = ((*gem_ctl_reg) & GCR_MASK) |
							  ENABLE_BERR_INT;
			*gem_ctl_reg = tmp_val;
			*geo_addr_p = sav_gadr;
			if (found == 0)
			    found = 2;
		  }
	}

	else
	{
		/********************************************************/
		/* The Master Status is not zero, clear this register   */
		/* and re-enable interrupts if this is a CPU Bus Timeout*/
		/********************************************************/
		if (ddf->cmd == GEM_DIAG_MODE)
		     ddf->diaginfo[16] = (ulong) m_stat_r.s_m_stat;
		else
		{
		    rc = bus_err(&m_stat_r,comp_nm);
		    *((uint *) (start_addr + MASTER_STAT)) = 0x00;
		}

		/********************************************************/
		/* Restore Geographical Address register                */
		/********************************************************/
		*geo_addr_p = sav_gadr;
		BUSMEM_DET(seg_reg);
		i_reset(pd);
		clrjmpx(&jmpbuf);
		if (rc != CPU_BUS_TIMEOUT)
		     adap_error(ddf,pd,rc);
		return(INTR_SUCC);
	}

	/****************************************************************/
	/* If the 'found' flag is off, then the Interrupt Handler       */
	/* cannot determine the source/cause of the interrupt           */
	/****************************************************************/
	if (!found)
	{
		/********************************************************/
		/* Spurious adapter interrupt. Log condition            */
		/********************************************************/
		gemlog(NULL,comp_nm,"gem_intr","gem_intr",NULL,
						  SPURIOUS_INTR,UNIQUE_4);
		tmp_val = ((*gem_ctl_reg) & GCR_MASK) | ENABLE_INTR;
		*gem_ctl_reg = tmp_val;
		*geo_addr_p = sav_gadr;
		BUSMEM_DET(seg_reg);
		i_reset(pd);
		clrjmpx(&jmpbuf);
		adap_error(ddf,pd,SPURIOUS_INTR);
		return(INTR_SUCC);
	}

	/****************************************************************/
	/* Restore the geographical address register                    */
	/****************************************************************/
	tmp_val = ((*gem_ctl_reg) & GCR_MASK) | ENABLE_INTR;
	*gem_ctl_reg = tmp_val;
	*geo_addr_p = sav_gadr;
	BUSMEM_DET(seg_reg);

	/****************************************************************/
	/* The interrupt has been serviced; reach here if is not an     */
	/* error interrupt. Check the ddf to determine if there is a    */
	/* process waiting for this event(synchronous) or it is an      */
	/* asynchronous event thus a callback routine is called         */
	/****************************************************************/
#ifdef GEM_DBUG
printf("gem_intr: ddf ptr  = %08x\n",ddf);
printf("    report.event   = %08x\n",ddf->report.event);
printf("    report.data[0] = %08x\n",ddf->report.data[0]);
printf("    report.data[1] = %08x\n",ddf->report.data[1]);
printf("    report.data[2] = %08x\n",ddf->report.data[2]);
printf("    report.data[3] = %08x\n",ddf->report.data[3]);
#endif GEM_DBUG

	/****************************************************************/
	/* If the adapter is NOT in diagnostic mode and a non-error     */
	/* interrupt was processed(found = TRUE), then determine how    */
	/* interrupt data is to be returned to the user.                */
	/****************************************************************/
	if ((ddf->cmd != GEM_DIAG_MODE) & (found == 1))
	{

		    /****************************************************/
		    /* If a process is running asynchronously and the   */
		    /* event takes place then pass control to the       */
		    /* callback routine which will place the ddf data   */
		    /*  into the HFT ring buffer.                       */
		    /****************************************************/
		    if (ddf->cmd & ASYNC_CMD)
		    {

			 if (event == DMA_EVENT)
			 {
			    (ddf->callback)
				  (ddf->callbackarg,ddf->callbackindx);
			 }
			 else
			{
			   (ddf->callback)(pd->cur_rcm,&(ddf->report));
			}
			ddf->cmd &= ~ASYNC_CMD;
		  }
		 /*******************************************************/
		 /* if a process is waiting for an interrupt and that   */
		 /* event has taken place (event = TRUE) then wakeup    */
		 /* the process.                                        */
		 /*******************************************************/
		 if (ddf->cmd & SYNC_WAIT_CMD)
		 {
		       e_wakeup(&(ddf->sleep_addr));
		       ddf->sleep_flags = 0x10;
		       ddf->cmd &= ~SYNC_WAIT_CMD;
		 }
	}

	i_reset(pd);
	clrjmpx(&jmpbuf);

#ifdef GEM_DBUG
	printf("gem_intr: exited\n");
#endif GEM_DBUG
	return(INTR_SUCC);

}

/************************************************************************/
/* SUBROUTINE:  ADAP_ERROR                                              */
/*                                                                      */
/* DESCRIPTION: This routine process adapter errors                     */
/************************************************************************/
adap_error(ddf,pd,rc)
  struct gem_ddf *ddf;
  struct phys_displays *pd;
  uint rc;
{

	if (ddf->cmd != NULL_CMD)
	{
	   ddf->report.event = ERROR_EVENT;
	   ddf->report.data[0] = rc;
	}

	if (ddf->cmd != GEM_DIAG_MODE)
	{
	    if (ddf->cmd & ASYNC_CMD)
	    {
	       if (!(ddf->cmd & DMA_IN_PROGRESS))
		    (ddf->callback)(pd->cur_rcm,&(ddf->report));
	    }
	    if (ddf->cmd & SYNC_WAIT_CMD)
	    {
		    e_wakeup(&(ddf->sleep_addr));
	    }
	}

	return(SUCCESS);

}

/************************************************************************/
/* SUBROUTINE:  FIFO_SYNC                                               */
/*                                                                      */
/* DESCRIPTION: This routine checks the contents of the Interrupt       */
/*              Pending Register for FIFO Sync Interrupts.              */
/************************************************************************/
fifo_sync(ipnd_reg,ddf,event)
  union ipnd_r *ipnd_reg;
  struct gem_ddf *ddf;
  int *event;
{
	/****************************************************************/
	/* Clear all but FIFO Sync bits from Interrupt Pending reg.     */
	/* Return 1 indicating 'found' if any Sync Counter interrupt    */
	/* bits are 'on' else return 0 indicating not found.            */
	/****************************************************************/
	if (ipnd_reg->s_ipnd_r & 0x000F)
	{
		/********************************************************/
		/* Fill in the event report and test each Sync Counter  */
		/* interrupt bit since there could be more than one     */
		/********************************************************/
		ddf->report.event = FIFO_SYNC_CNT_EVENT;
		*event = 1;
		if (ipnd_reg->b.fifo_sync0_intr)
		{
			/************************************************/
			/* SYNC COUNTER 0 Interrupt                     */
			/************************************************/
			if (ddf->cmd == GEM_DIAG_MODE)
				ddf->diaginfo[0]++;
			else
				ddf->report.data[0] = SYNC_CTR0;
		}

		if (ipnd_reg->b.fifo_sync1_intr)
		{
			/************************************************/
			/* SYNC COUNTER 1 Interrupt                     */
			/************************************************/
			if (ddf->cmd == GEM_DIAG_MODE)
				ddf->diaginfo[1]++;
			else
				ddf->report.data[0] |= SYNC_CTR1;
		}

		if (ipnd_reg->b.fifo_sync2_intr)
		{
			/************************************************/
			/* SYNC COUNTER 2 Interrupt                     */
			/************************************************/
			if (ddf->cmd == GEM_DIAG_MODE)
				ddf->diaginfo[2]++;
			else
				ddf->report.data[0] |= SYNC_CTR2;
		}

		if (ipnd_reg->b.fifo_sync3_intr)
		{
			/************************************************/
			/* SYNC COUNTER 3 Interrupt                     */
			/************************************************/
			if (ddf->cmd == GEM_DIAG_MODE)
				ddf->diaginfo[3]++;
			else
				ddf->report.data[0] |= SYNC_CTR3;
		}
		return(SUCCESS);
	}
	return(1);

}

/************************************************************************/
/* SUBROUTINE:  THRESH_HOLD                                             */
/*                                                                      */
/* DESCRIPTION: This routine checks the contents of the Interrupt       */
/*              Pending Register for thresh_hold interrupts.            */
/************************************************************************/
thresh_hold(ipnd_reg,ddf,st_addr,event)
  union ipnd_r *ipnd_reg;
  struct gem_ddf *ddf;
  uint st_addr;
  int *event;
{
	volatile uint *thresh_reg;

	/****************************************************************/
	/* Clear all but Threshold bits from Interrupt Pending reg.     */
	/* Return 0 indicating 'found' if any Threshold interrupt bits  */
	/* are 'on' else return 1 indicating not Threshold Interrupts   */
	/****************************************************************/
	if (ipnd_reg->s_ipnd_r & 0x0FF0)
	{
		/********************************************************/
		/* If the adapter is in Diagnostic Mode, then increment */
		/* the diagnostic count. Otherwise, we are servicing a  */
		/* HO/LO Threshold interrupt. The hardware has toggled  */
		/* the Enable bit of the Threshold HI/LO causing the    */
		/* interrupt off. The interrupt handler will toggle the */
		/* corresponding Threshold HI/LO interrupt 'on' and     */
		/* 'wakeup' or 'sleep' the process as required.         */
		/********************************************************/
		if ((ddf->sync_event_mask & THRESHOLD_EVENT) |
		     (ddf->async_event_mask & THRESHOLD_EVENT))
		{
		    ddf->report.event = THRESHOLD_EVENT ;
		    *event = 1;
		}
		else
		    *event = 0;

		if (ipnd_reg->b.hi_thresh0_intr)
		{
			/************************************************/
			/* High 0 Threshold Interrupt                   */
			/************************************************/
			if (ddf->cmd == GEM_DIAG_MODE)
				ddf->diaginfo[8]++;
			else
			{
#ifdef THRES_DEBUG
   printf("H0 ");
#endif THRES_DEBUG
			  /**********************************************/
			  /* This is a HI Threshold 0 interrupt. It has */
			  /* been raised because a process is about to  */
			  /* fill FIFO 0. Enable the LOW 0 Threshold    */
			  /* interrupt and put the process to 'sleep'.  */
			  /**********************************************/
			  thresh_reg = (uint *) (st_addr | INTR_LOW_0);
			  *thresh_reg = ENABLE_THRESH;
			  if (!(ddf->immpid_flags & SLEEPING))
			  {
			      ddf->immpid_flags |= SLEEPING;
			      ddf->watch_imm.ddf = ddf;
			      ddf->watch_imm.imm = ddf->imm_pid;
#ifdef TIMER
			      w_start(&ddf->watch_imm.wi);
#endif
			      uexblock(ddf->imm_pid);
			  }
			}
		}

		if (ipnd_reg->b.lo_thresh0_intr)
		{
			/************************************************/
			/* Low 0 Threshold Interrupt                    */
			/************************************************/
			if (ddf->cmd == GEM_DIAG_MODE)
				ddf->diaginfo[4]++;
			else
			{
#ifdef THRES_DEBUG
   printf("L0 ");
#endif THRES_DEBUG
			  /**********************************************/
			  /* This is a Low Threshold 0 interrupt. It has*/
			  /* been raised because FIFO 0 has crossed the */
			  /* low threshold boundary. Wakeup the current */
			  /* process if it is 'sleeping'.               */
			  /**********************************************/
			  if (ddf->immpid_flags & SLEEPING)
			  {
			       uexclear(ddf->imm_pid);
			       ddf->immpid_flags &= ~SLEEPING;
#ifdef TIMER
			       w_stop(&ddf->watch_imm.wi);
#endif
			  }
			}
		}

		if (ipnd_reg->b.hi_thresh1_intr)
		{
			/************************************************/
			/* High 1 Threshold Interrupt                   */
			/************************************************/
			if (ddf->cmd == GEM_DIAG_MODE)
				ddf->diaginfo[9]++;
			else
			{
#ifdef THRES_DEBUG
   printf("H1 ");
#endif THRES_DEBUG
			  /**********************************************/
			  /* This is a HI Threshold 1 interrupt. It has */
			  /* been raised because a process is about to  */
			  /* fill FIFO 1. Enable the LOW 1 Threshold    */
			  /* interrupt and put the process to 'sleep'.  */
			  /**********************************************/
			  if (ddf->immpid_flags & DRP_INTR_COMP)
				  ddf->immpid_flags &= ~DRP_INTR_COMP;
			  else
			  {
			    thresh_reg = (uint *) (st_addr | INTR_LOW_1);
			    *thresh_reg = ENABLE_THRESH;
			  }

			  if (!(ddf->immpid_flags & SLEEPING))
			  {
			      ddf->immpid_flags |= SLEEPING;
			      ddf->watch_imm.ddf = ddf;
			      ddf->watch_imm.imm = ddf->imm_pid;
#ifdef TIMER
			      w_start(&ddf->watch_imm.wi);
#endif
			      uexblock(ddf->imm_pid);
			  }
			}
		}

		if (ipnd_reg->b.lo_thresh1_intr)
		{
			/************************************************/
			/* Low 1 Threshold Interrupt                    */
			/************************************************/
			if (ddf->cmd == GEM_DIAG_MODE)
				ddf->diaginfo[5]++;
			else
			{
#ifdef THRES_DEBUG
   printf("L1 ");
#endif THRES_DEBUG
			  /**********************************************/
			  /* This is a Low Threshold 1 interrupt. It has*/
			  /* been raised because FIFO 1 has crossed the */
			  /* low threshold boundary. Wakeup the current */
			  /* process if it is 'sleeping'.               */
			  /**********************************************/
			  if (ddf->immpid_flags & SLEEPING)
			  {
			       uexclear(ddf->imm_pid);
			       ddf->immpid_flags &= ~SLEEPING;
#ifdef TIMER
			       w_stop(&ddf->watch_imm.wi);
#endif
			  }
			}
		}

		if (ipnd_reg->b.hi_thresh2_intr)
		{
			/************************************************/
			/* High 2 Threshold Interrupt                   */
			/************************************************/
			if (ddf->cmd == GEM_DIAG_MODE)
				ddf->diaginfo[10]++;
			else
			{
#ifdef THRES_DEBUG
   printf("H2 ");
#endif THRES_DEBUG
			  /**********************************************/
			  /* This is a HI Threshold 2 interrupt. It has */
			  /* been raised because a process is about to  */
			  /* fill FIFO 2. Enable the LOW 2 Threshold    */
			  /* interrupt and put the process to 'sleep'.  */
			  /**********************************************/
			  thresh_reg = (uint *) (st_addr | INTR_LOW_2);
			  *thresh_reg = ENABLE_THRESH;
			  if (!(ddf->travpid_flags & SLEEPING))
			  {
			      ddf->travpid_flags  |= SLEEPING;
			      ddf->watch_trav.ddf  = ddf;
			      ddf->watch_trav.trav = ddf->trav_pid;
#ifdef TIMER
			      w_start(&ddf->watch_trav.wt);
#endif
			      uexblock(ddf->trav_pid);
			  }
			}
		}

		if (ipnd_reg->b.lo_thresh2_intr)
		{
			/************************************************/
			/* Low 2 Threshold Interrupt                    */
			/************************************************/
			if (ddf->cmd == GEM_DIAG_MODE)
				ddf->diaginfo[6]++;
			else
			{
#ifdef THRES_DEBUG
   printf("L2 ");
#endif THRES_DEBUG
			  /**********************************************/
			  /* This is a Low Threshold 2 interrupt. It has*/
			  /* been raised because FIFO 2 has crossed the */
			  /* low threshold boundary. Wakeup the current */
			  /* process if it is 'sleeping'.               */
			  /**********************************************/
			  if (ddf->travpid_flags & SLEEPING)
			  {
			       uexclear(ddf->trav_pid);
			       ddf->travpid_flags &= ~SLEEPING;
#ifdef TIMER
			       w_stop(&ddf->watch_trav.wt);
#endif
			  }
			}
		}

		if (ipnd_reg->b.hi_thresh3_intr)
		{
			/************************************************/
			/* High 3 Threshold Interrupt                   */
			/************************************************/
			if (ddf->cmd == GEM_DIAG_MODE)
				ddf->diaginfo[11]++;
			else
			{
#ifdef THRES_DEBUG
   printf("H3 ");
#endif THRES_DEBUG
			  /**********************************************/
			  /* This is a HI Threshold 3 interrupt. It has */
			  /* been raised because a process is about to  */
			  /* fill FIFO 3. Enable the LOW 3 Threshold    */
			  /* interrupt and put the process to 'sleep'.  */
			  /**********************************************/
			  if (ddf->travpid_flags & DRP_INTR_COMP)
				  ddf->travpid_flags &= ~DRP_INTR_COMP;
			  else
			  {
			     thresh_reg = (uint *) (st_addr | INTR_LOW_3);
			     *thresh_reg = ENABLE_THRESH;
			  }
			  if (!(ddf->travpid_flags & SLEEPING))
			  {
			      ddf->travpid_flags |= SLEEPING;
			      ddf->watch_trav.ddf  = ddf;
			      ddf->watch_trav.trav = ddf->trav_pid;
#ifdef TIMER
			      w_start(&ddf->watch_trav.wt);
#endif
			      uexblock(ddf->trav_pid);
			  }
			}
		}

		if (ipnd_reg->b.lo_thresh3_intr)
		{
			/************************************************/
			/* Low 3 Threshold Interrupt                    */
			/************************************************/
			if (ddf->cmd == GEM_DIAG_MODE)
				ddf->diaginfo[7]++;
			else
			{
#ifdef THRES_DEBUG
   printf("L3 ");
#endif THRES_DEBUG
			  /**********************************************/
			  /* This is a Low Threshold 3 interrupt. It has*/
			  /* been raised because FIFO 3 has crossed the */
			  /* low threshold boundary. Wakeup the current */
			  /* process if it is 'sleeping'.               */
			  /**********************************************/
			  if (ddf->travpid_flags & SLEEPING)
			  {
			       uexclear(ddf->trav_pid);
			       ddf->travpid_flags &= ~SLEEPING;
#ifdef TIMER
			       w_stop(&ddf->watch_trav.wt);
#endif
			  }
			}
		}

		return(SUCCESS);
	}

	return(1);
}


/************************************************************************/
/* SUBROUTINE:  DMA_INTR                                                */
/*                                                                      */
/* DESCRIPTION: This routine checks the contents of the Interrupt       */
/*              Pending Register for DMA Complete interrupt.            */
/************************************************************************/
dma_intr(ipnd_reg,ddf,event)
  union ipnd_r *ipnd_reg;
  struct gem_ddf *ddf;
  int *event;
{
	/****************************************************************/
	/* Clear all but the DMA Complete bit from the Interrupt Pending*/
	/* Register.                                                    */
	/****************************************************************/
	if (ipnd_reg->s_ipnd_r & 0x4000)
	{
		/********************************************************/
		/* A DMA Operation has Completed. Turn off the DMA IN   */
		/* Progress flag and return an indication that the      */
		/* interrupt was processed.                             */
		/********************************************************/
		ddf->cmd &= ~DMA_IN_PROGRESS;
		*event = DMA_EVENT;
		return(SUCCESS);
	}
	return(1);

}

/************************************************************************/
/* SUBROUTINE:  SYS_FAIL                                                */
/*                                                                      */
/* DESCRIPTION: This routine checks the contents of the System Control  */
/*              Register to determine the cause of a sysfail. A pointer */
/*              to the contents of the System control Register is passed*/
/*              as a parameter.                                         */
/************************************************************************/
sys_fail(sys_ctl_r,id,comp_nm)
union sys_ctl_r *sys_ctl_r;
uint  id;
char *comp_nm;
{
	uint val;

	if (sys_ctl_r->b.ext_falt)
	{
	       switch (id)
	       {
		  case 0:
		       val = FAULT_GCP;
		       break;
		  case 1:
		       val = FAULT_DRP;
		       break;
		  case 2:
		       val = FAULT_SHP;
		       break;
		  default:
		       break;
		}
	}

	if (sys_ctl_r->b.ucod_falt)
	{
	       switch (id)
	       {
		  case 0:
		       val = UC_FAULT_GCP;
		       break;
		  case 1:
		       val = UC_FAULT_DRP;
		       break;
		  case 2:
		       val = UC_FAULT_SHP;
		       break;
		  default:
		       break;
		}
	}

	if (sys_ctl_r->b.prty_err_hi)
	{
	       switch (id)
	       {
		  case 0:
		       val = PARITY_ERR_HI_GCP;
		       break;
		  case 1:
		       val = PARITY_ERR_HI_DRP;
		       break;
		  case 2:
		       val = PARITY_ERR_HI_SHP;
		       break;
		  default:
		       break;
		}
	}

	if (sys_ctl_r->b.prty_err_lo)
	{
	       switch (id)
	       {
		  case 0:
		       val = PARITY_ERR_LO_GCP;
		       break;
		  case 1:
		       val = PARITY_ERR_LO_DRP;
		       break;
		  case 2:
		       val = PARITY_ERR_LO_SHP;
		       break;
		  default:
		       break;
		}
	}

	gemlog(NULL,comp_nm,"gem_intr","gem_intr",NULL,val,UNIQUE_5);
	return(SUCCESS);
}

/************************************************************************/
/* SUBROUTINE:  GCP_INTR                                                */
/*                                                                      */
/* DESCRIPTION: This routine will process an interrupt generated by the */
/*              GCP which is not hardware related.                      */
/************************************************************************/
gcp_intr(g_r_dbp,ddf,gcp_com_p,start_addr,event,comp_nm,pd)
  struct g_r_dblk *g_r_dbp;
  struct gem_ddf *ddf;
  struct gcp_comm *gcp_com_p;
  uint  start_addr;
  int  *event;
  char *comp_nm;
  struct phys_displays *pd;
{
	int dblk_flg;                   /* more data blocks to be proc. */
	uint * g_r_dbip;                /* int ptr to GCP-RIOS d.b.     */

	/****************************************************************/
	/* Process all GCP data blocks by going thru their links. Check */
	/* their reason codes; have to get that from the 4 bytes which  */
	/* are accessed.                                                */
	/****************************************************************/
	g_r_dbip = (uint *) g_r_dbp;
	dblk_flg = 1;
	while (dblk_flg)
	{
		/********************************************************/
		/* Set Data Block being processed flag                  */
		/********************************************************/
		*g_r_dbip = *g_r_dbip | IN_PROCESS;

		/********************************************************/
		/* Check reason code to determine why GCP interrupted   */
		/********************************************************/
		switch(*g_r_dbip & GCP_REQ_RES_MSK)
		{
		case (R_SYNC) :
			   ddf->report.event = FIFO_SYNC_EVENT;
			   *event = 1;
			break;

		case (R_PICK_OCCR) :
			  ddf->report.event   = PICK_EVENT;
			  ddf->report.data[0] = g_r_dbp->data[0];
			  ddf->report.data[1] = g_r_dbp->data[1];
			  ddf->report.data[2] = g_r_dbp->data[2];
			  ddf->report.data[3] = 0;
			  *event = 1;
			break;

		default :
			/*************************************************/
			/* unknown/invalid reason code                   */
			/*************************************************/
			gemlog(NULL,comp_nm,"gem_intr","gem_intr",
				       NULL,BAD_INTR_CODE,UNIQUE_8);
		       *g_r_dbip = CLR_BFLG;
		}

		if (g_r_dbp->dbk_lnkof == 0)
		{
			/************************************************/
			/* This is the last data block, set head and    */
			/* tail data block offsets to zero. The data    */
			/* collected in the DDF will be returned to the */
			/* user in the main routine.(only last block)   */
			/************************************************/
			gcp_com_p->g_r_hdbkof = 0;
			gcp_com_p->g_r_tdbkof = 0;
			dblk_flg = 0;

			/************************************************/
			/* Reset the busy flags in the GCP_RIOS and     */
			/* RIOS_GCP data blocks                         */
			/************************************************/
			*g_r_dbip = CLR_BFLG;
		}
		else
		{
			/************************************************/
			/* Move pointer to next data block structure    */
			/************************************************/
			g_r_dbp = (struct g_r_dblk *)(g_r_dbp->dbk_lnkof +
			    start_addr - CVME_ADDR);

			/************************************************/
			/* Reset the busy flags in GCP to RIOS data     */
			/* block.                                       */
			/************************************************/
			*g_r_dbip = CLR_BFLG;

			/************************************************/
			/* If the adapter is NOT in diagnostic mode and */
			/* a process is running asynchronously then     */
			/* give callback routine control to place DDF   */
			/* data on the HFT ring buffer.                 */
			/************************************************/
			g_r_dbip = (uint *) g_r_dbp;
			if ((ddf->cmd != GEM_DIAG_MODE) &
						  (ddf->cmd == ASYNC_CMD))
			{
			     (ddf->callback)(pd->cur_rcm,&(ddf->report));
			}
		}
	}

	return(SUCCESS);
}


/*************************************************************************/
/* SUBROUTINE:  DRP_INTR                                                 */
/*                                                                       */
/* DESCRIPTION: This routine will process an interrupt generated by the  */
/*              DRP which is not hardware related.                       */
/*************************************************************************/
drp_intr(d_r_cbp,ddf,drp_com_p,st_addr,comp_nm)
uint *d_r_cbp;
struct gem_ddf *ddf;
struct drp_comm *drp_com_p;
uint st_addr;
char *comp_nm;
{
	volatile uint *thresh_reg;

	/*****************************************************************/
	/* Check the reason code, have to get that from the accessed     */
	/* 4 byte word                                                   */
	/*****************************************************************/
	switch(*(int *)&(drp_com_p->d_r_intblk) & DRP_RES_MSK)
	{
	case (R_DRP_COMP1):
		  /*******************************************************/
		  /* A DRP Immediate FIFO Read Pixel Complete interrupt. */
		  /* Disable Low threshold interrupt, set a flag to      */
		  /* indicate that this interrupt has been received, and */
		  /* wake the current process if it is 'sleeping'.       */
		  /*******************************************************/
		  thresh_reg = (uint *) (st_addr | LOW_THRESH_1);
		  *thresh_reg = DISABLE_THRESH;
		  ddf->immpid_flags |= DRP_INTR_COMP;
		  if (ddf->immpid_flags & SLEEPING)
		  {
		      ddf->immpid_flags &= ~SLEEPING;
		      uexblock(ddf->imm_pid);
		  }
		break;

	case (R_DRP_COMP3):
		  /*******************************************************/
		  /* A DRP Traversal FIFO Read Pixel Complete interrupt. */
		  /* Disable Low threshold interrupt, set a flag to      */
		  /* indicate that this interrupt has been received, and */
		  /* wake the current process if it is 'sleeping'.       */
		  /*******************************************************/
		  thresh_reg = (uint *) (st_addr | LOW_THRESH_3);
		  *thresh_reg = DISABLE_THRESH;
		  ddf->travpid_flags |= DRP_INTR_COMP;
		  if (ddf->travpid_flags & SLEEPING)
		  {
		      ddf->travpid_flags &= ~SLEEPING;
		      uexblock(ddf->trav_pid);
		  }
		break;

	case (R_DRP_EXCP):
		  /*******************************************************/
		  /* A DRP Exception interrupt Log error                 */
		  /*******************************************************/
		gemlog(NULL,comp_nm,"gem_intr","gem_intr",NULL,
						     DRP_EXCEPT,UNIQUE_9);
		break;

	default:
		/*********************************************************/
		/* DrP Interrupt with invalid  reason code. Log error    */
		/*********************************************************/
		gemlog(NULL,comp_nm,"gem_intr","gem_intr",NULL,
						  BAD_DRP_CODE,UNIQUE_10);
		break;
	}

	/*****************************************************************/
	/* Clear DrP Interrupt Request Block                             */
	/*****************************************************************/
	*(int *)&drp_com_p->d_r_intblk = 0;

	return(SUCCESS);
}

/*************************************************************************/
/* SUBROUTINE:  BUS_ERR                                                  */
/*                                                                       */
/* DESCRIPTION: This routine will process CVME Buss Error detected by the*/
/*              MBC.                                                     */
/*************************************************************************/
bus_err(m_stat_r,comp_nm)
union  m_stat_r *m_stat_r;
char *comp_nm;
{

	uint rc;

	rc = SUCCESS;
	if (m_stat_r->b.sys_fail)
	{
		/*********************************************************/
		/* If Bus Parity Error. Log the error                    */
		/*********************************************************/
		if (m_stat_r->b.bus_prty)
	       {
			rc = BUS_PARITY_ERR;
		}
		/*********************************************************/
		/* If CVME Bus Timeout Error. Log the error              */
		/*********************************************************/
		if (m_stat_r->b.cvme_timout)
	       {
			rc = BUS_TIMEOUT;
	       }
		/*********************************************************/
		/* If CPU Bus Timeout Error do NOT LOG error             */
		/*********************************************************/
		if ((m_stat_r->b.cpu_timout) & (rc == 0))
		{
			rc  = CPU_BUS_TIMEOUT;
	       }
		else
			gemlog(NULL,comp_nm,"gem_intr","gem_intr",NULL,
							    rc,UNIQUE_11);
	}
	return(rc);
}

/*************************************************************************/
/* SUBROUTINE:  GEM_TIMEOUT_IMM        IMMEDIATE FIFO                    */
/*                                                                       */
/* DESCRIPTION: This routine will get control if a process is blocked    */
/*              and a Low 1 Threshold interrupt has not been receive     */
/*              within 5 seconds. The blocked process is allowed to      */
/*              continue processing.                                     */
/*************************************************************************/
void gem_timeout_imm(w)
 struct watch_imm *w;
{

      w_stop((struct watchdog *) w);
      w->ddf->immpid_flags &= ~SLEEPING;
      uexclear(w->imm);
}

/*************************************************************************/
/* SUBROUTINE:  GEM_TIMEOUT_TRAV       TRAVERSAL FIFO                    */
/*                                                                       */
/* DESCRIPTION: This routine will get control if a process is blocked    */
/*              and a Low 3 Threshold interrupt has not been receive     */
/*              within 5 seconds. The blocked process is allowed to      */
/*              continue processing.                                     */
/*************************************************************************/
void gem_timeout_trav(w)
 struct watch_trav *w;
{

      w_stop((struct watchdog *) w);
      w->ddf->travpid_flags &= ~SLEEPING;
      uexclear(w->trav);
}

/*************************************************************************/
/* SUBROUTINE:  GEM_TIMEOUT_TRAV       TRAVERSAL FIFO                    */
/*                                                                       */
/* DESCRIPTION: This routine will get control if a process is sleeping   */
/*              wating for an event and that event has not taken place   */
/*              within 5 seconds. The sleeping process is awakened.      */
/*************************************************************************/
void gem_timeout_event(w)
 struct watch_event *w;
{

      w->ddf->sleep_flags = 0x10;
      w_stop((struct watchdog *) w);
      w->ddf->report.event = ERROR_EVENT;
      e_wakeup(w->sleep_addr);
}


/*************************************************************************/
/* SUBROUTINE:  VME_DIAG                                                 */
/*                                                                       */
/* DESCRIPTION: This routine gets control when a CVME interrupt is       */
/*              detected and the adapter is in diagnostic mode.          */
/*************************************************************************/
vme_diag(intr_vector,ddf)
   uint intr_vector;
   struct gem_ddf *ddf;
{

     switch (intr_vector)
      {
	 case GCP_INTR:
	     /************************************************************/
	     /* GCP is cause of CVME Interrupt.                          */
	     /************************************************************/
	     ddf->diaginfo[12] = (ulong) intr_vector;
	     break;

	 case DRP_INTR:
	     /************************************************************/
	     /* DrP is cause of CVME Interrupt.                          */
	     /************************************************************/
	     ddf->diaginfo[13] = (ulong) intr_vector;
	     break;

	 case SHP_INTR:
	     /************************************************************/
	     /* SHP is cause of CVME Interrupt.                          */
	     /************************************************************/
	     ddf->diaginfo[14] = (ulong) intr_vector;
	     break;

	 case IMP_INTR:
	     /************************************************************/
	     /* IMP is cause of CVME Interrupt.                          */
	     /************************************************************/
	     ddf->diaginfo[15] = (ulong) intr_vector;
	     break;

	 default:
	     /************************************************************/
	     /* Invalid vector id                                        */
	     /************************************************************/
	     ddf->diaginfo[12] = ERROR;
	     ddf->diaginfo[13] = ERROR;
	     ddf->diaginfo[14] = ERROR;
	     ddf->diaginfo[15] = ERROR;
	     break;
      }

}
