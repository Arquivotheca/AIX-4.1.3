static char sccsid[] = "@(#)04  1.11  10/12/93 09:07:50";
#ifndef KERNEL
#define KERNEL
#endif
/*
 *   COMPONENT_NAME: SYSXPSLA
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/************************************************************************/
/* PURPOSE:     Device driver for MSLA on R2 interrupt handler and      */
/*              internal routines it calls.                             */
/*                                                                      */
/* ROUTINES:                                                            */
/*      cancel_sio                                                      */
/*      do_dma                                                          */
/*      fpsend_sol                                                      */
/*      fpsetup_sio                                                     */
/*      get_sense                                                       */
/*      ipl_msla                                                        */
/*      ipl_start                                                       */
/*      send_sol                                                        */
/*      send_uns                                                        */
/*      setup_sio                                                       */
/*      stat_accp                                                       */
/*      stop_strt                                                       */
/*      undo_dma                                                        */
/*                                                                      */
/*; bb 012990   Add a HALT_MSLA to the stop part of stop_strt to        */
/*;             eliminate the extra "not our interrupt" interrupt.      */
/*; bb 013190   Removed HALT_MSLA in stop_strt because of failure to    */
/*;             reconnect after 'stop_psla' issued.                     */
/*; bb 021390   Changed 'get_sense' to do 4bytes at a time.             */
/*; MJ 022090   Put back the removed dma channel disable ('d_mask')     */
/*;             back in 'ipl_start', because in some cases we ipl_start */
/*;             without deconfiguration.				*/
/*; bb 022890   Added two macros, SEND_SIO and SEND_STAT_ACCP to        */
/*              reduce bus io. To use them, compile with                */
/*              BUSTEST defined.                                        */
/*; MJ 031390   Took care of some changes for LINT.			*/
/*;		Modified parameter types of BUSMEM_ATT, BUSMEM_DET.	*/
/*; bb 040690   If HYDRA, do nothing in stat_accp().                    */
/************************************************************************/


/* INCLUDE FILES   */
#include "gswincl.h"

/* DEFINES         */
#include "gswdefs.h"

/* EXTERN ROUTINES */
#include "gswextr.h"

/* EXTERN DATA     */
#include "gswextd.h"

/*----------------------------------------------------------------------*/
/* Local macro for waiting on rap->scf flag on adapter.                 */
/*----------------------------------------------------------------------*/
#define MAXWT_STATACCP 1000000
#define WAIT_SCF(cnt) {                                                 \
	volatile int i;                                                 \
	for (i = 0; (rap->scf != 0) && (i < cnt) ; i++);                \
    }

  /*--------------------------------------------------------------------*/
  /* Macro to set fields in adapter comm area via 4 bus accesses        */
  /* instead of 7 bus accesses. This is done for each 5080 io operation.*/
  /* This macro replaces the following statements:                      */
  /*      rap->scf   = NormalScf;                                       */
  /*      rap->fsb   = 0;                                               */
  /*      rap->icc   = StatAccp;                                        */
  /*      rap->ccc   = cap->ccc;                                        */
  /*      rap->lda   = cap->lda;                                        */
  /*--------------------------------------------------------------------*/
#define SEND_STAT_ACCP                                                  \
   {                                                                    \
	ulong  *ulp;            /* ulong ptr to rap->scf        */      \
	ushort *usp;            /* short ptr to rap->ccc        */      \
	ushort *ucp;            /* short ptr to cap->ccc        */      \
	static ulong rapword =  /* value for 1st rap word       */      \
	       (NormalScf     << 24) +                                  \
	       (AutoSenseMode << 16) +                                  \
	       (StatAccp);                                              \
									\
	ulp = (ulong *)rap;                                             \
	usp = (ushort *)(ulp+1);                                        \
	ucp = (ushort *)cap + 2;                                        \
									\
	*ulp = rapword;                                                 \
	*usp = *ucp;                                                    \
   }

  /*--------------------------------------------------------------------*/
  /* Macro to set fields in adapter comm area via 2 bus accesses        */
  /* instead of 5 bus accesses. This is done for each 5080 io operation.*/
  /* This macro replaces the following statements:                      */
  /*      rap->fsb   = ccw_offset->flags;                               */
  /*      rap->icc   = Cmd;                                             */
  /*      rap->ccc   = ae_exp_cmd;                                      */
  /*      rap->lda   = g->lda;                                          */
  /*      rap->cnt   = ce_offset->tranlen;                              */
  /*--------------------------------------------------------------------*/
#define SEND_SIO                                                        \
   {                                                                    \
	ulong  *ulp;            /* ulong ptr to rap->ccc        */      \
	ushort *usp;            /* short ptr to rap->fsb        */      \
									\
	ulp = (ulong *)rap + 1;                                         \
	usp = (ushort *)rap + 1;                                        \
									\
	*ulp = (ae_exp_cmd << 24)         +                             \
	       (g->lda     << 16)         +                             \
	       ce_offset->tranlen;                                      \
	*usp = (*chp << 8) + Cmd;                                       \
   }


/*------------ C A N C E L _ S I O   --------------------------------*/
/*
 * NAME:        cancel_sio
 *
 * PURPOSE:     cancel any outstanding sio operations
 *
 * PARMS:       none
 *
 * RETURNS:     void
 *
 * ALGORITHM:
 */
void cancel_sio(dev)
dev_t dev;
{
   int i;
   int last;
   GSWCB *g;
   char *chp;

   PRINT(("cancel_sio: entered.dev=0x%x \n",dev));
   /*-------------------------------------------------------------------*/
   /* Set up loop for either one device (if dev = 0,1,2) or all         */
   /* minor devices (if dev = 3).                                       */
   /*-------------------------------------------------------------------*/
   i    = (dev == NumDevSupp) ? 0 : dev;
   last = (dev == NumDevSupp) ? NumDevSupp : (dev+1);


   for (; i < last; i++)
   {
	g = mgcb_ptr + i;
	if (g->io_pend_cnt)
	{
		g->io_pend_cnt = 0;
		AWAKE_SIO
		g->oflag.wokeup = TRUE;
		if (g->oflag.io_waiting)
		{
		    SIGAPPL(SIGIOINT);  /* send SIGIOINT if async, ... */
		    g->oflag.io_waiting = FALSE;
		}
	}
	chp   = (char *)&g->b;
	*chp  = 0;                      /* zero out cmd bits in g       */
	chp   = (char *)&g->c;
	*chp  = 0;                      /* zero out cmd bits in g       */
   }
   ae_exp_cmd       = UndefCmd;
   mf.sol_in_prog   = FALSE;
   mf.rmi_in_prog   = FALSE;
   mf.intio_in_prog = FALSE;
}

/*------------ D O _ D M A   ----------------------------------------*/
/*
 * NAME:        do_dma
 *
 * PURPOSE:     sets up for dma operation
 *
 * PARMS:       chanid      DMA channel id
 *              flags       flags
 *              baddr       buffer address
 *              count       transfer count
 *              location    user or kernel space transfer
 *              daddr       base bus address for DMA transfer
 *              dev         minor device number
 *              dp          struct xmem ptr
 *
 *
 * RETURNS:     int
 *
 * CALLING ROUTINE: send_uns, send_sol, fpsend_sol
 *
 * ALGORITHM:
 */
void do_dma(chanid, flags, baddr, count, daddr, location, dev,dp)
int   chanid;
int   flags;
char *baddr;
int   count;
char *daddr;
int   location;
dev_t  dev;
struct xmem *dp;
{
   GSWCB *g;
   caddr_t ioaddr;

   g = mgcb_ptr + dev;
   rap = (struct rm_comm_area *)
	   BUSMEM_ATT((ulong)BUS_ID,(ulong)(start_busmem+RtoMCommAdrOfst));
				/* add pg offset to daddr               */
   rap->adf = ((uint)daddr) | (((uint)baddr) & 0x00000FFF);
   BUSMEM_DET((ulong)rap);

   comm_lock = 1;               /* reset aftr DE or AE+DE in intrpt hdlr*/

   /*-------------------------------------------------------------------*/
   /* Check if operation involves user space or kernel space.           */
   /* If user space, get access to user bfr via io_att.                 */
   /* In either case, we get the dp struct from g->xm.dp area.          */
   /*-------------------------------------------------------------------*/
   switch(location)
   {
       case DMA_USER :
	   ioaddr = io_att( (ulong)dp->subspace_id,
			    (((ulong)baddr) & 0x0fffffff));


	   /*-----------------------------------------------------------*/
	   /* When dma between user space and adapter, buffer must be   */
	   /* pinned, xmattach.                                         */
	   /* .... PINNING WAS DONE IN GSWIO .....................      */
	   /* Pin complete pages, checking if buffer spans pages.       */
	   /* Offset+count+pg_size, then round down to nearest page.    */
	   /* Call d_master to set up the DMA operation.                */
	   /*-----------------------------------------------------------*/
	   d_master(
		    chanid,     /* DMA channel id                       */
		    flags,      /* flags                                */
		    ioaddr,     /* buffer address  (converted via io_att*/
		    count,      /* transfer count                       */
		    dp,         /* xmem descriptor ptr                  */
		    daddr);     /* bus address                          */
	   io_det((ulong)ioaddr);
	   break;

       case DMA_KERNEL :
	   /*-----------------------------------------------------------*/
	   /* Call d_master to set up the DMA operation.                */
	   /*-----------------------------------------------------------*/
	   dp->aspace_id = XMEM_GLOBAL;
	   d_master(
		    chanid,     /* DMA channel id                       */
		    flags,      /* flags                                */
		    baddr,      /* buffer address                       */
		    count,      /* transfer count                       */
		    dp,         /* xmem descriptor ptr                  */
		    daddr);     /* bus address                          */
	   break;

       default:
	   PRNTE(("do_dma: FAILURE - unknown location 0x %x.\n",location));
	   break;

   }
   /*-------------------------------------------------------------------*/
   /* Set g->dma_location so undo_dma knows when it is called.          */
   /*-------------------------------------------------------------------*/
   g->dma_location = location;          /* use it when DMA complete     */
   g->dma_chanid   = chanid;            /* use it when DMA complete     */
   g->dma_flags    = flags;             /* use it when DMA complete     */
   g->dma_baddr    = baddr;             /* use it when DMA complete     */
   g->dma_count    = count;             /* use it when DMA complete     */
   g->dma_dp       = dp;                /* use it when DMA complete     */
   g->dma_daddr    = daddr;             /* use it when DMA complete     */
}

/*------------ F P S E N D _ S O L   --------------------------------*/
/*
 * NAME:        fpsend_sol
 *
 * PURPOSE:     Perform the start dma operation associated
 *              with an INBOUND FPGI operation.
 * PARMS:       ccbp   ptr to ccb
 *              dev    minor device number
 *              dp     struct xmem ptr
 *
 * RETURNS:     void
 *
 * ALGORITHM:
 */
void fpsend_sol(ccbp,dev,dp)
struct ccb *ccbp;
dev_t   dev;
struct xmem *dp;
{
   char *chp;
   GSWCB *g;

   /*-------------------------------------------------------------------*/
   g = mgcb_ptr + dev;

   rap = (struct rm_comm_area *)
	   BUSMEM_ATT((ulong)BUS_ID,(ulong)(start_busmem+RtoMCommAdrOfst));
					/* add pg offset to daddr       */
   WAIT_SCF(MAXWT_STATACCP)             /* delay until rap->scf reset   */
   if (rap->scf != 0)                   /* very serious. adapter stuck. */
   {
	gerr_log(dev,NULL,fpsendsol_name,SCF_FLAG_HUNG,0,ras_unique[1]);
	return;
   }
   mf.fpsol_in_prog = TRUE;
   chp  = (char *)(&g->c);
   *chp = AEMask + DEMask;              /* want AE and DE returned      */

   ae_exp_cmd = fpccw_offset->cmdcode;
   chp  = (char *)(&g->b);              /* set bits in LD flags         */
   *chp = fpccw_offset->flags;

   /*------------------------*/
   /* Fill in the com area   */
   /*------------------------*/

   chp        = (char *)(&g->b);
   rap->fsb   = *chp;
   rap->icc   = Cmd;
   rap->ccc   = ae_exp_cmd;
   rap->lda   = g->lda;
   ae_exp_lda = g->lda;
   rap->cnt   = (ushort)(fpce_offset->tranlen);

  /*--------------------------------------------------------------------*/
  /* we need to know if the dma operation involves a user               */
  /*     buffer or a kernel buffer, because a user buffer requires      */
  /*     additional calls (like xmattach).                              */
  /*     The macro FCEX put this info in the 'res3' field of the CE.    */
  /*--------------------------------------------------------------------*/
   do_dma(
		 dma_chanid,                    /* DMA channel id       */
		 DMA_READ,                      /* flags                */
		 fpce_offset->memaddr,          /* buffer address       */
		 rap->cnt,                      /* length of transfer   */
		 g->dma_bus_addr,               /* base DMA bus adr     */
		 ce_offset->res3,               /* user or kernel bfr   */
		 dev,                           /* minor device         */
		 dp);                           /* minor device         */

   SET_TMR(IoTmrVal,0,IoTmrMask);
   INT_MSLA                         /* interrupt the adapter        */
   BUSMEM_DET((ulong)rap);
}


/*------------F P S E T U P _ S I O  --------------------------------*/
/*
 * NAME:        fpsetup_sio
 *
 * PURPOSE:     setup for an inbound FPGI sio operation
 *
 * PARMS:       ccbp     ptr to ccb
 *              dev    minor device number
 *              indx   indx of available user bfr
 *              dp     struct xmem ptr
 *
 * RETURNS:     void
 *
 * ALGORITHM:
 *       . disable interrupts
 *       . if comm area available,
 *               . enable intrpts
 *               . if lda is assigned to this device,
 *                       . setup addressability to ccb and ce's by
 *                         updating ce_offset and
 *                         ccw_offset
 *                       . call send_sol to issue dma operation
 *               . else
 *                       . free the comm area lock
 *       . else
 *               . save index of pending cmd
 *               . if queue overrun, log error
 *               . enable interrupts
 *       . return
 *
 */

void fpsetup_sio(ccbp,dev,indx,dp)
struct ccb *ccbp;
dev_t   dev;
int    indx;                            /* indx of available user bfr   */
struct xmem *dp;
{
   GSWCB *g;
   int oldmask;

   /*-------------------------------------------------------------------*/
   g = mgcb_ptr + dev;
   G_DISABLE
   if (comm_lock == 0)          /* is lock available?                   */
   /* comm area lock secured if we get here                             */
   {
	comm_lock = 1;          /* set lock                             */
	G_ENABLE                /* link to clrsb instruction x'95E7'    */

	if (g->lda <= LdaLimit)
	{
	    /*----------------------------------------------------------*/
	    /* Setup addressability to ccb and ce's.                    */
	    /*  There is one complication: the data area for the        */
	    /*  last ce/ccw is in the user's data space. All the        */
	    /*  other areas are in the AIX driver's data segment.       */
	    /*  use 'xmattach' for cross memory to user space.          */
	    /*----------------------------------------------------------*/

	    g->inbuf_indx = indx;       /* save indx of user's bfr      */
	    fphdr_offset = (struct ccb *)
			     &((g->fpcbp+indx)->rfp.ccb);
	    fpce_offset  = (struct com_elm *) (fphdr_offset + 1);
	    fpccw_offset = (struct ccw *)
			   &((g->fpcbp+indx)->ccw3.rmi);
	    fpce_offset++;              /* point to 1st data ce         */

	    fpsend_sol(ccbp,dev,dp);
	}
	else                            /* no lda match.                */
	    comm_lock = 0;              /* release lock                 */
   }
   else
   {    /* did not get lock.... put on que to process command later     */
	 rmiqhead       =  (rmiqhead +1) %  (NumDevSupp+1);
	 rmiq[rmiqhead] = dev;
	 if (rmiqhead == rmiqtail)      /*  q is overrun                */
		gerr_log(dev,NULL,fpsetupsio_name,CMD_QOVERRUN,
                         0,ras_unique[2]);
	 G_ENABLE                /* enable system interrupts            */
   }
}


/*------------ G E T _ S E N S E     --------------------------------*/
/*
 * NAME:        get_sense
 *
 * PURPOSE:     get unsolicited sense from adapter to AIX buffer
 *
 * PARMS:       dev    minor device number
 *
 * RETURNS:     int
 *
 * CALLING ROUTINE:
 *              gswintr
 *
 * ALGORITHM:
 *       . check that sense area exists
 *       . move sense data to AIX buffer area
 *       . update head index
 *       . return
 *
 */
int get_sense(dev)
dev_t   dev;
{
   int          i;
   int          rc = SENSE_PRESENT;
   int          nobuf;
   int          ybuf;
   int          update;

   ushort       head;                   /* local head index             */
   ushort       tail;                   /* local tail index             */

   uint        *sourp;                  /* source of sense (adapter)    */
   uint        *destp;                  /* dest of sense (AIX area)     */
   struct uns_sense *sense_ptr;         /* pointer to the sense area    */
   GSWCB *g;

   /*-------------------------------------------------------------------*/
   g = mgcb_ptr + dev;
   PRINT(("get_sense: dev=0x%x,g->sen_datap=0x%x\n",dev,g->sen_datap));
   update = TRUE;
   if (g->sen_hdrp)                     /* unsol sense area present     */
   {
	head = g->sen_hdrp->head;
	tail = g->sen_hdrp->tail;
					/* check there's room in area   */
	if ( tail != ((head + 1) % SENSE_MODULO) )
	{
		destp  = (uint *)(g->sen_datap+head);
		sourp  = (uint *)&(cap->vda.ch[0]);     /* comm_area    */
		sense_ptr = (struct uns_sense *)destp;

		for (i = 0; i < (sizeof(struct uns_sense) >> 2); i++)
		    *destp++ = *sourp++;                /* xfer words   */
		if(g->openmode == FPGI_MODE)
		  {
		   if((sense_ptr->pn_MSB & 0x04) && (sense_ptr->pn_LSB == 0))
		      bufstat[dev] = Conn_Compl;
		   else
		   {
		      nobuf = (sense_ptr->pn_MSB & 0x08);
		      ybuf  = (sense_ptr->pn_MSB & 0x10);
		      if(nobuf > 0)
		      {
			 bufstat[dev] = NoFpgiBuf;
			 update = 0;
		      }
		      if(ybuf > 0)
		      {
			 bufstat[dev] = YesFpgiBuf;
			 update = 0;
		      }
		   }/* END ELSE */
		}  /* END FPGI MODE */

		if(update)                      /* update head index    */
		{                               /* zero out ack status  */
		    g->sen_hdrp->head =  (head + 1) % SENSE_MODULO;
		    rc = 0;
		}
	}
	else                            /* sense area full!             */
		rc = SENSE_AREA_FULL;
   }
   else                                 /* no unsol sense area!         */
	rc =  NO_SENSE_AREA;
   return(rc);
}

/*------------I P L _ M S L A    ------------------------------------*/
/*
 * NAME:        ipl_msla
 *
 * PURPOSE:     ipl the MSLA adapter
 *
 * PARMS:       none
 *
 * RETURNS:     void
 *
 * ALGORITHM:
 *
 *       . halt the 68K processor
 *       . reset interrupts from the adapter
 *       . load ucode onto adapter
 *       . enable the adapter
 *       . enable DMA channel
 *       . set automatic get sense mode
 *       . start thr 68K processor
 *       . set timer
 */

void ipl_msla()
{
  int   rc;
  ulong  dma_bus_id = IOCC_BID;
  uchar *delay_ptr;                     /* for delay                    */
  uchar  delay_val;                     /* for delay                    */

  PRINT(("ipl_msla: entered \n"));

  RST_INTR_MSLA                         /* reset  interrupts on adapter */
  DIS_PARITY_POS                        /* disable parity checking      */
  ENA_MSLA_POS                          /* enable the adapter           */
  RST_MSLA                              /* reset adapter                */

  delay_ptr = (uchar *)BUSMEM_ATT((ulong)BUS_ID,(ulong)IoDelayAdr);
  delay_val = *delay_ptr;               /* delay > 1.25 us              */
  BUSMEM_DET((ulong)delay_ptr);

  HALT_MSLA                             /* stop adapter                 */
  rc = loaducode();                     /* load ucode                   */
  ENA_PARITY_POS                        /* enable parity checking       */
					/* initialize DMA               */

  /*rc = d_init(ddsp->sd.dma_level,MICRO_CHANNEL_DMA,dma_bus_id);*/
  rc = d_init(ddsp->sd.dma_level,MICRO_CHANNEL_DMA,bus_id);
  if (rc == DMA_FAIL)
  {
	PRNTE(("ipl_msla: FAILURE of dma enable. RETURNING NOW .... \n"));
	return;
  }
  dma_chanid = rc;                      /* save channel id              */
  mf.dma_enabled = TRUE;
  d_unmask(dma_chanid);                 /* enable the DMA channel       */
  rap = (struct rm_comm_area *)
	   BUSMEM_ATT((ulong)BUS_ID,(ulong)(start_busmem+RtoMCommAdrOfst));
  rap->smf = AutoSenseMode;
  BUSMEM_DET((ulong)rap);

  /***** switched the order of the next two cmds *******/
  ENA_INTR_MSLA                         /* enable interrupts on adapter */
  STRT_MSLA                             /* start the adapter            */

  SET_TMR(ProgRptTmrVal, 0, ProgRptTmrMask);
  return;
}     /* end ipl_msla  */


/*------------I P L _ S T A R T  ------------------------------------*/
/*
 * NAME:        ipl_start
 *
 * PURPOSE:     start an ipl of the msla
 *
 * PARMS:       none
 *
 * RETURNS:     void
 *
 * ALGORITHM:
 *
 */

void ipl_start()
{
  int    i;
  char  *tmpptr;

  /*--------------------------------------------------------------------*/
  /* Test and set lock                                                  */
  /*--------------------------------------------------------------------*/
  PRINT(("ipl_start: comm_lock = %d \n", comm_lock));
  if  (comm_lock)                       /***** not available        *****/
	mf.ipl_req = TRUE;              /*                  do it later */
  else
  {                                     /*initialize flags              */
	ae_exp_cmd = UndefCmd;

	if (mf.dma_enabled == TRUE)
	{
	    d_mask(dma_chanid);      
	    d_clear(dma_chanid);    
	    DIS_INTR_MSLA         
	    mf.dma_enabled = FALSE;
	}

  					/* zero 2 bytes of flags        */
	tmpptr = (char *)(&mf);         /* (flag_hw = 0;)               */
	*tmpptr++ = 0;
	*tmpptr   = 0;

	mf.ipl_req     = FALSE;
	mf.ipl_in_prog = TRUE;

	pndqhead = 0;                   /*clear que pointers            */
	pndqtail = 0;
	rmiqhead = 0;
	rmiqtail = 0;

	for (i = 0; i < NumDevSupp; i++)
	{                               /* invalidate lda's & cmd states*/
	    (mgcb_ptr+i)->lda = LdaLimit + 1;
	    tmpptr  = (char *)&((mgcb_ptr+i)->b);
	    *tmpptr = 0;                /* zero command state bits      */
	    tmpptr  = (char *)&((mgcb_ptr+i)->c);
	    *tmpptr = 0;                /* zero control state bits      */
	}

	/*--------------------------------------------------------------*/
	/* CANCEL ALL TIMERS                                            */
	/*--------------------------------------------------------------*/
	CANCEL_TMR(IoTmrMask);
	CANCEL_TMR(StopStrtTmrMask);
	CANCEL_TMR(ProgRptTmrMask);
	CANCEL_TMR(NrtrTmrMask);
	ipl_msla();
  }

} /* END  ipl_start */

/*------------ S E N D _ S O L       --------------------------------*/
/*
 * NAME:        send_sol
 *
 * PURPOSE:     Perform the start dma operation associated
 *              with a solicited sio.
 * PARMS:       ccbp   ptr to ccb
 *              dev    minor device number
 *              dp     struct xmem ptr
 *
 * RETURNS:     void
 *
 * ALGORITHM:
 *       . loop until status clear flag is cleared by the adapter
 *       . set flags for sol_in_progress, ae and de expected
 *       . set msla comm area fields (icc, ccc,lda,cnt,fsb)
 *       . start dma operation
 *       . setup a timer interrupt
 *       . interrupt the adapter
 *       . return
 */

void send_sol(ccbp,dev,dp)
struct ccb *ccbp;
dev_t   dev;
struct xmem *dp;
{
   char *chp;                           /* local character ptr          */
   GSWCB *g;

   PRINT(("send_sol: dev=0x%x,ccbp=0x%x,dp=0x%x\n",dev,ccbp,dp));
   /*-------------------------------------------------------------------*/
   g = mgcb_ptr + dev;

   rap = (struct rm_comm_area *)
	   BUSMEM_ATT((ulong)BUS_ID,(ulong)(start_busmem+RtoMCommAdrOfst));
   WAIT_SCF(MAXWT_STATACCP)             /* delay until rap->scf reset   */
   if (rap->scf != 0)                   /* very serious. adapter stuck. */
   {
	gerr_log(dev,NULL,sendsol_name,SCF_FLAG_HUNG,0,ras_unique[3]);
	return;
   }

   mf.sol_in_prog    = TRUE;            /* indicate a cmd is active     */

   chp  = (char *)(&g->c);              /* deque is required (for sio)  */
   *chp = AEMask + DEMask;              /* done in intr hdlr when get DE*/
   chp  = (char *)(&g->b);
   *chp = 0;

   if (ccbp->dev_opt & CCWLST)          /* if ccw list, copy            */
   {                                    /*   the chaining bits          */
	ae_exp_cmd = ccw_offset->cmdcode;
	*chp = ccw_offset->flags;
   }
   else                                         /* single cmd           */
						/* DOES THIS EVER HAPPEN?*/
	PRNTE(("send_sol: ERROR - entered 'single cmd' code.\n"));

						/* set flags            */

#ifdef BUSTEST
   SEND_SIO
#else
   rap->fsb   = *chp;
   rap->icc   = Cmd;
   rap->ccc   = ae_exp_cmd;
   rap->lda   = g->lda;
   rap->cnt   = (ushort)(ce_offset->tranlen);   /* # bytes to xfer      */
#endif  /***SEND_SIO***/

   ae_exp_lda = g->lda;
   BUSMEM_DET((ulong)rap);


  /*--------------------------------------------------------------------*/
  /* We need to know if the dma operation involves a user               */
  /*     buffer or a kernel buffer, because a user buffer requires      */
  /*     additional calls (like xmattach).                              */
  /*     The macro FCEX put this info in the 'res3' field of the CE.    */
  /*--------------------------------------------------------------------*/
   do_dma(
		 dma_chanid,                    /* DMA channel id       */
		 DMA_READ,                      /* flag means R and W   */
		 ce_offset->memaddr,            /* buffer address       */
		 ce_offset->tranlen,            /* length of transfer   */
		 g->dma_bus_addr,               /* base DMA bus adr     */
		 ce_offset->res3,               /* user or kernel bfr   */
		 dev,                           /* minor device         */
		 dp);                           /* xmem struct ptr      */

  SET_TMR(IoTmrVal,0,IoTmrMask);
  INT_MSLA                                     /* interrupt the adapter */
}

/*------------ S E N D _ U N S       --------------------------------*/
/*
 * NAME:        send_uns
 *
 * PURPOSE:     Perform the start dma operation associated
 *              with an RMI sio.
 *
 * PARMS:       dev    minor device number
 *
 * RETURNS:     void
 *
 * ALGORITHM:
 *       . loop until status clear flag is cleared by the adapter
 *       . set flags for rmi_in_progress, ae and de expected, sli
 *       . set msla comm area fields (icc, ccc,lda,cnt,fsb)
 *       . start dma operation
 *       . setup a timer interrupt
 *       . interrupt the adapter
 *       . return
 *
 */
void send_uns(dev)
dev_t   dev;
{
   char *chp;                           /* local character ptr          */
   GSWCB *g;

   PRINT(("send_uns: entered. dev = 0x%x \n",dev));
   /*-------------------------------------------------------------------*/
   g = mgcb_ptr + dev;
   rap = (struct rm_comm_area *)
	   BUSMEM_ATT((ulong)BUS_ID,(ulong)(start_busmem+RtoMCommAdrOfst));
   WAIT_SCF(MAXWT_STATACCP)             /* delay until rap->scf reset   */
   if (rap->scf != 0)                   /* very serious. adapter stuck. */
   {
	gerr_log(dev,NULL,senduns_name,SCF_FLAG_HUNG,0,ras_unique[4]);
	return;
   }
   mf.rmi_in_prog    = TRUE;            /* indicate an RMI is active    */
   g->c.ae_exp  = TRUE;                 /* expect adapter end to follow */
   g->c.de_exp  = TRUE;                 /* expect device  end to follow */
   chp  = (char *)(&g->b);              /* zero out 'c' flags and set   */
   *chp = SLIMask;                      /*    the SLI bit.              */

   rap->fsb   = *chp;                   /* contents of b structure      */
   rap->icc   = Cmd;
   rap->ccc   = Rmi;
   ae_exp_cmd = Rmi;
   ae_exp_lda = g->lda;
   rap->lda   = g->lda;
   rap->cnt   = RmiSize;
   BUSMEM_DET((ulong)rap);

   do_dma(
		 dma_chanid,                    /* DMA channel id       */
		 DMA_READ,                      /* flags                */
		 g->rmip,                       /* buffer address       */
		 RmiSize,                       /* length of transfer   */
		 g->dma_bus_addr,               /* base DMA bus adr     */
		 DMA_KERNEL,                    /* user or kernel bfr   */
		 dev,                           /* minor device         */
		 g->rmidp);                     /* struct xmem for RMI  */

   SET_TMR(IoTmrVal,0,IoTmrMask);
   INT_MSLA                             /* interrupt the adapter        */
}

/*------------S E T U P _ S I O  ------------------------------------*/
/*
 * NAME:        setup_sio
 *
 * PURPOSE:     setup for an outbound sio operationr
 *
 * PARMS:       ccbp     ptr to ccb
 *              dev      minor device number
 *              dp       struct xmem ptr
 *
 * RETURNS:     void
 *
 * ALGORITHM:
 *       . disable interrupts
 *       . if comm area available,
 *               . enable intrpts
 *               . if lda is assigned to this minor device,
 *                       . setup addressability to ccb and ce's by
 *                         updating ce_offset and
 *                         ccw_offset
 *                       . call send_sol to issue dma operation
 *               . else
 *                       . free the comm area lock
 *       . else
 *               . save index of pending cmd
 *               . if queue overrun, log error
 *               . enable interrupts
 *       . return
 *
 */

int setup_sio(ccbp,dev,dp)
struct ccb *ccbp;
dev_t   dev;
struct xmem *dp;
{
   int  oldmask;
   GSWCB *g;

   PRINT(("setup_sio: ccpb=0x%x,dev=0x%x,dp=0x%x\n",ccbp,dev,dp));
   /*-------------------------------------------------------------------*/

   g = mgcb_ptr + dev;
   G_DISABLE                    /* disable interrupts                   */
   if (comm_lock == 0)          /* is lock available?                   */
   {
	comm_lock = 1;          /* set lock                             */
	G_ENABLE                /* enable interrupts                    */
	if ((mgcb_ptr+dev)->lda <= LdaLimit)
	{
	    /*----------------------------------------------------------*/
	    /* setup addressability to ccb and ce's                     */
	    /*----------------------------------------------------------*/
	    hdr_offset =  ccbp;
	    ce_offset  = (struct com_elm *) (hdr_offset + 1);

	    /*----------------------------------------------------------*/
	    /* Check if ccw list present.                               */
	    /*----------------------------------------------------------*/
	    if (ccbp->dev_opt & CCWLST)
	    {
		ccw_offset = (struct ccw *)(ce_offset->memaddr);
		ce_offset++;            /* point to 1st data ce         */
	    }                           /* (cmd is in ccc of comm area) */
	    else
		ccw_offset = (struct ccw *)0;
	    send_sol(ccbp,dev,dp);
	}
	else                            /* no lda match.                */
	{
	    G_DISABLE                   /* disable interrupts           */
	    comm_lock = 0;              /* release lock                 */
	    G_ENABLE                    /* enable interrupts            */
	    return(-1);
	}
   }

   else /*--------------------------------------------------------------*/
   {    /* did not get lock.... put on que to process command later     */
	/* Also must save the parms to 'setup_sio' to use later in the  */
	/* interrupt handler when we re-issue setup_sio.                */
	/*--------------------------------------------------------------*/

	 g->pndccbp = ccbp;
	 g->pnddp   = dp;
	 pndqhead       =  (pndqhead +1) %  (NumDevSupp+1);
	 pndq[pndqhead] = dev;
	 if (pndqhead == pndqtail)      /*  q is overrun                */
		gerr_log(dev,NULL,setupsio_name,CMD_QOVERRUN,
                         0,ras_unique[5]);
	 G_ENABLE
   }
   return(0);
}


/*------------ S T A T _ A C C P     --------------------------------*/
/*
 * NAME:        stat_accp
 *
 * PURPOSE:     Set values in the r2_to_msla comm area
 *
 * PARMS:       none
 *
 * RETURNS:     void
 *
 * ALGORITHM:
 *       . loop until status clear flag is cleared by the adapter
 *       . set rt-msla comm area fields (icc, ccc,lda,cnt,fsb)
 *       . interrupt the adapter
 *       . return
 *
 */

void stat_accp(dev)
dev_t dev;
{
   PRINT(("stat_accp: dev=0x%x\n",dev));

#ifndef HYDRA

   WAIT_SCF(MAXWT_STATACCP)             /* delay until rap->scf reset   */
   if (rap->scf != 0)                   /* very serious. adapter stuck. */
   {
	gerr_log(dev,NULL,stataccp_name,SCF_FLAG_HUNG,0,ras_unique[6]);
	return;
   }

#ifdef BUSTEST
   SEND_STAT_ACCP
#else
   rap->scf   = NormalScf;
   rap->fsb   = 0;
   rap->icc   = StatAccp;
   rap->ccc   = cap->ccc;
   rap->lda   = cap->lda;
#endif  /***BUSTEST***/

   INT_MSLA                             /* interrupt the adapter        */

#endif  /***HYDRA***/

}

/*------------ S T O P _ S T R T     --------------------------------*/
/*
 * NAME:        stop_strt
 *
 * PURPOSE:     issue either a stop or a start msla
 *
 * PARMS:       dev    minor device number
 *
 * RETURNS:     void
 *
 * ALGORITHM:
 *       . loop until status clear flag is cleared by the msla
 *       . set msla comm area flags FSB, ICC and CCC
 *       . set adapter end expecting flag to either stop or start
 *       . setup a timer interrupt to wait a period of time then
 *         enter the exception handler, which logs an error if not cancel
 *       . interrupt the adapter
 *       . return
 *
 */
void stop_strt()
{
   PRINT(("stop_strt: rap->ccc=0x%x,mf=0x%x\n",rap->ccc, *((ushort *)&mf)));

   WAIT_SCF(MAXWT_STATACCP)             /* delay until rap->scf reset   */
   if (rap->scf != 0)                   /* very serious. adapter stuck. */
   {
	gerr_log((dev_t)0,NULL,stopstrt_name,SCF_FLAG_HUNG,0,ras_unique[7]);
	return;
   }
   rap->fsb = 0;
   rap->icc = Init;
   if (mf.start_in_prog)
   {
	rap->ccc   = StartCmd;
	ae_exp_cmd = StartCmd;
	SET_TMR(StopStrtTmrVal,0,StopStrtTmrMask);
	INT_MSLA                        /* interrupt the adapter        */
   }
   else
   {                                    /* stop in progress             */
	rap->ccc   = StopCmd;
	ae_exp_cmd = StopCmd;
	SET_TMR(StopStrtTmrVal,0,StopStrtTmrMask);
	INT_MSLA                        /* interrupt the adapter        */
	/**********************
	HALT_MSLA
	DIS_MSLA_POS
	**********************/
   }
}

/*------------ U N D O _ D M A    -----------------------------------*/
/*
 * NAME:        undo_dma
 *
 * PURPOSE:     cleanup after dma operation completes
 *
 * PARMS:       g           ptr to struct gswcb associated with this DMA
 *
 * RETURNS:     int
 *
 * CALLING ROUTINE: gswintr (after io completion)
 *
 * ALGORITHM:
 */
int undo_dma(g)
GSWCB *g;
{
   int rc;
   int ret = 0;
   caddr_t ioaddr;

   PRINT(("undo_dma: g=0x%x\n",g));
   /*-------------------------------------------------------------------*/
   /* Call d_complete to undo the d_master operation.                   */
   /* If DMA_USER, we need a converted 'baddr' value.                   */
   /*-------------------------------------------------------------------*/
   if (g->dma_location == DMA_USER)
	   ioaddr = io_att((ulong)g->dma_dp->subspace_id,
			   (((ulong)g->dma_baddr) & 0x0fffffff));
   else
	   ioaddr = g->dma_baddr;

   rc = d_complete(
	 g->dma_chanid,             /* DMA channel id                   */
	 g->dma_flags,              /* flags                            */
	 ioaddr,                    /* buffer address                   */
	 g->dma_count,              /* transfer count                   */
	 g->dma_dp,                 /* xmem descriptor ptr              */
	 g->dma_daddr);             /* address to pgm the DMA master    */
   /*-------------------------------------------------------------------*/
   /* Check if operation involves user space or kernel space.           */
   /*-------------------------------------------------------------------*/
   if (rc != DMA_SUCC)
	PRNTE(("undo_dma: d_complete FAILED. rc = 0x %x\n",rc));
   ret = rc;

   switch(g->dma_location)
   {
       case DMA_USER :
	   /*-----------------------------------------------------------*/
	   /* When dma between user space and adapter, buffer must be   */
	   /* xmdetach, unpinned.                                       */
	   /* Get access to user bfr via io_att for 'unpin'.            */
	   /*-----------------------------------------------------------*/
	   io_det((ulong)ioaddr);
	   rc = xmdetach(g->dma_dp);
	   if (rc != XMEM_SUCC)
	   {
		PRNTE(("undo_dma: FAILURE of xmdetach. rc = 0x%x\n",rc));
		ret = rc;
	   }
	   break;

       case DMA_KERNEL :
	   break;

       default:
	   PRNTE(("undo_dma: FAILURE - unknown location 0x%x.\n",
			g->dma_location));
	   break;

   }
   if ( g->dma_dp == (g->xm.dp + g->xm.indx) )  /* if using xm.dp,      */
	g->xm.indx++;                   /* increment for next one       */
   g->dma_location = NO_DMA_IN_PROG;    /* cleared for next one         */
   return(ret);
}

