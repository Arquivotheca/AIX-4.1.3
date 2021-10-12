static char sccsid[] = "@(#)06	1.10.1.4  src/bos/kernext/psla/gswintr.c, sysxpsla, bos411, 9428A410j 10/12/93 09:08:13";
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
/*----------------------------------------------------------------------*/
/* PURPOSE:           Determine if this is an interrupt for the         */
/*                       MSLA    adapter. If not, return the code       */
/*                       indicating that the interrupt was not processed*/
/*                       by this routine.                               */
/*                                                                      */
/* DEPENDENCIES:                                                        */
/*                 gswdef.h - defines for msla dd                       */
/*                 gswdefs.h - defines for msl dd                       */
/*                 gswcb.h - structs for msla  dd                       */
/*                 gswdds.h - structs for msla dds                      */
/*                 gswextd.h - external data variables                  */
/*                                                                      */
/*                                                                      */
/*;bb 013090    Added decrement of att_cnt and turned off attached flag */
/*              for serious error.                                      */
/*;bb 013190    removed serious_err = TRUE in DE-UC case.               */
/*;bb 013190    bumped sense tail ptr after get_sense for DE+UC.        */
/*;bb 020290    T E M P   T E S T   put rst_intr_msla and oswald        */
/*;             is modifying a card to test for the spureous intrpts.   */
/*;bb 020590    Removed gerr_log call for MSLA_LM case. Not an error.   */
/*;bb 021390    Added ipf = 0 for sense error cases that return early.  */
/*;             Initialize 'reason' to 0.                               */
/*;MJ 021490    Added the test for 'slp_pending' before AWAKE in        */
/*;             UnSolDevReq and ATTN+DE+UE case to avoid awakening      */
/*;             processes that are not sleeping.			*/
/*;MJ 022190    Removed all instances of 'att_cnt' and 'att_done' in    */
/*;             i.g. serious error case.				*/
/*;MJ 022190    Set 'ipl_flag' to TRUE in serious error and stop_in_    */
/*;		prog cases.						*/
/*;MJ 022390    Moved 'undo_dma()' to after io_pending test in DE+UC.   */
/*;MJ 032090    Changed 'diag_mode' from a device dependent variable    */
/*;	        to a global one.					*/
/*;bb 032090    Add errlog entry for parity error and IPFnotSet error.  */
/*;bb 051090    Add PIO Parity Error logic (SET_PARITYJMPRC) and use    */
/*;             a local jmpbuf (for nested calls).                      */
/*;bb 051990    Add undo_dma in AdapterErr case stement if there is an  */
/*              outstanding DMA operation. Set serious_err = TRUE.      */
/*;bb 032891    Moved the ParityError call to errlog after the check    */
/*              for diagnostic mode. In diagnostic mode, a parity error */
/*              is forced, and we do not want to log it. No lines       */
/*              changed - only moved.                                   */
/*;bb 042291    Added code in rmi_in_prog section to AWAKE user if      */
/*              sleeping. This matches change in gsw23.c for sleeping   */
/*              in gswio if rmi_in_prog or sol_in_prog.                 */
/*;bb 050791    Set pndq[pndqtail] = -1 after i/o started for pending   */
/*              i/o.                                                    */
/*                                                                      */
/*;   Notes:                                                            */
/*;      . Chained CCWs - the driver can pass a collection of ccws.     */
/*;        Let's take the example of two ccws.                          */
/*;        The AIX dd creates a ccb header followed by 3 ce's.          */
/*;        ce1.adr points to the ccw list, ce2.adr points to the data   */
/*;        address of the 1st ccw and ce3.adr points to the data address*/
/*;        of the 2nd ccw. In general, 'n' chained ccws have 'n+1' ce's.*/
/*;        CC means command chaining in the diagram below.              */
/*;                                                                     */
/*;        --------------                                               */
/*;        |ccb header  |                                               */
/*;        |            |                                               */
/*;        |------------|                                               */
/*;        |ce1         |                                               */
/*;        |    adr-->>>>>>>>>>>>>>>>>>> ---------------------------    */
/*;        |            |                | ccw1:     ccwadr1    cc |    */
/*;        |            |                | ccw2:     ccwadr2  nocc |    */
/*;        |------------|                ---------------------------    */
/*;        |ce2         |                                               */
/*;        |    adr-->>>>>>>>>>>>>>>>>>> ccwadr1                        */
/*;        |            |                -----------                    */
/*;        |            |                | data    |                    */
/*;        |------------|                -----------                    */
/*;        |ce3         |                                               */
/*;        |    adr-->>>>>>>>>>>>>>>>>>> ccwadr2                        */
/*;        |            |                -----------                    */
/*;        |            |                | data    |                    */
/*;        |------------|                -----------                    */
/*;                                                                     */
/*;        The flow for SIO commands is as follows:  (for above eg)     */
/*;              . AIX gswio routine issues SIO with the 2 ccws above   */
/*;              . IOINIT routine gets control, realizes it as a        */
/*;                sio and therefore issues SETUP_IO                    */
/*;              . SETUP_IO issues SEND_SOL_MSLA_CMD.                   */
/*;              . SEND_SOL_MSLA     does the actual dma call, _stdma.  */
/*;                note: if there are 'n' ccws, there are 'n' dma calls.*/
/*;                      The cmd code in the ccw is put in the ccc field*/
/*;                      of the comm area. The collection of ccws are no*/
/*;                      sent to the msla in a dma operation.           */
/*;              . intrpt hdlr receives the AE+DE for the ccw1/ce2      */
/*;                and checks that there is command chaining.           */
/*;              . intrpt hdlr issues SEND_SOL_MSLA_CMD for the         */
/*;                ccw2/ce3 and exits.                                  */
/*;              . intrpt hdlr receives the AE+DE for the ccw2/ce3      */
/*;                and checks if there is more command chaining, which  */
/*;                there isnt, since it is the last ccw.                */
/*;                Intrpt handler ultimately notifies user of           */
/*;                completion of the full SIO.                          */
/*;                                                                     */
/*;        Key point: the intrpt hdlr is responsible for issuing        */
/*;              the additional io calls when multiple ce's exist and   */
/*;               it is also responsible for notifying the user after   */
/*;               last one is complete.                                 */
/*;                                                                     */
/*;                                                                     */
/*;    LOGIC:                                                           */
/*;                                                                     */
/*;       . check that interrupt is a MSLA interrupt. Return if not.    */
/*;       . find logical dev index by comparing LDA in adapter          */
/*;         communications area with LDA in g structure.                */
/*;       . Use the ICC field of the msla communication area to         */
/*;         determine the category of the interrupt                     */
/*;           . Cmd -                                                   */
/*;               . if data                                             */
/*;                 chaining or cmd chaining                            */
/*;               . check status bytes 1 and 2 for the following        */
/*;                   AE+DE                                             */
/*;                   AE+DE+ATTN+UC                                     */
/*;                   AE                                                */
/*;                   DE                                                */
/*;                   default                                           */
/*;                 Check comments within code for each of these cases. */
/*;           . Debug -                                                 */
/*;               . log an error                                        */
/*;           . Init -                                                  */
/*;               . (look at comments in the code)                      */
/*;           . Progress Code -                                         */
/*;               . Reset outstanding timer.                            */
/*;               . If prog code = 0 ("all ok and complete"),           */
/*;                   . start the msla                                  */
/*;                   . turn off the ipl_in_prog flag                   */
/*;                   . turn on start_in_prog flag                      */
/*;                   . turn off stop_in_prog flag                      */
/*;                   . turn off start_required flag                    */
/*;                   . call stop_strt                                  */
/*;               . Else                                                */
/*;                   . set up a timer for another progress code        */
/*;           . Unsolicited device request -                            */
/*;               . check status byte 1 for the following               */
/*;                   ATTN+DE+UE                                        */
/*;                   ATTN                                              */
/*;                   ATTN+UC                                           */
/*;                   default                                           */
/*;                 Check comments within code for each of these cases. */
/*;           . Adapter error -                                         */
/*;               . log an error                                        */
/*;               .                                                     */
/*;                                                                     */
/*;       . if no msla cmd active                                       */
/*;           . if cmd chaining, setup and start dma operation.         */
/*;           . if rmi pending, issue the rmi.                          */
/*;           . if a cmd is pending, issue sio ( which will start dma). */
/*;                                                                     */
/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/*      VRM interface structure include file                            */
/*----------------------------------------------------------------------*/

#include <string.h>
#include "gswincl.h"
#include "gswdefs.h"
#include "gswextr.h"
#include "gswextd.h"
#include "gswdiag.h"
 
#include <sys/trcctl.h>
#include <sys/trcmacros.h>
#include <sys/trchkid.h>

#ifdef FPGI
extern int fpintrpt();
#endif FPGI
 
int gswintr( ihp )
struct intr *ihp;
 
{
  static int counter_err = 0;
  register q_qel *this_qel;
  register q_hdr *this_hdr;
  register IQT   *tablep;
  char           *data_ptr;
  char           *compl_chk;
  enum g_type     type;
  dev_t           dev;
  int             i;
  int             ret_flag = TRUE;
  int             bad_data = 0;
  int             er       = FALSE;
  int             slot_siz = 0;
  int             data_len = 0;
  int             rc       = 0;
  int             sense_status = 0;
  int             serious_err = FALSE;  /* set if serious error occurs  */
  int             minusone = -1;        /* bad return code              */
  int             parityrc;             /* rc for setjmpx parity        */
  uint            ui = 0;               /* unit check bit flag in MSLA- */
                                        /* workstation comm. area.      */
  uint            tempuint;             /* local uint                   */
  ushort          reason = 0;           /* local ushort - see case Init */
  char            msla_stat;
  char           *chp;                  /* local char ptr               */
 
  union {
         int wd;
         char byt[4];
  }qual;
  struct ccb        *ccbp;              /* local ptr to a ccb header    */
  struct gswcb      *gbak;              /* local ptr to g structure     */
 
  struct stat1  st1;                    /* local - has comm area status1*/
  struct stat2  st2;                    /* local - has comm area status2*/
 
  char  *st1chp = (char *)&st1;         /* local - char ptr for status  */
  char  *st2chp = (char *)&st2;         /* local - char ptr for status  */

  ulong base_ioadr;                     /* local base io adr for adapter*/
  ulong base_memadr;                    /* local base mem adr for adaptr*/
  ulong segreg;                         /* local seg reg value          */
  struct gswcb   *g;
  label_t lclparbuf;                    /* local parity buffer          */

  TRCHKL1T(HKWD_PSLA_INTR,ihp);
  /*--------------------------------------------------------------------*/
  /* Check if our interrupt.                                            */
  /* But first, set up addressability to adapter addresses              */
  /*--------------------------------------------------------------------*/

  SET_PARITYJMPRC(0,&(lclparbuf),ras_unique[31],intr_name,INTR_SUCC,
		  NO_UNLOCK)
  segreg = BUSMEM_ATT((ulong)BUS_ID,(ulong)0);/*seg reg for adapter adrs*/
  base_ioadr  = segreg + start_busio;
  base_memadr = segreg + start_busmem;
  stat_rgp = (char *)               (base_ioadr  + RdStatIOAdr);
  cap      = (struct mr_comm_area *)(base_memadr + MtoRCommAdrOfst);
  rap      = (struct rm_comm_area *)(base_memadr + RtoMCommAdrOfst);

  msla_stat = *stat_rgp;
  if (msla_stat & 0x01)
  {
     PRNTE(("gswintr: NOT OUR INTERRUPT. msla_stat=0x%x,cap->ipf=0x%x\n",
	       msla_stat, cap->ipf));
     BUSMEM_DET(segreg);
     CLR_PARITYJMP(&(lclparbuf))
     TRCHKL1T(hkwd_INTR_R1,(uint)msla_stat);
     return(-1);
  }

  if ( (msla_stat & 0x02) == 0 )                /* parity even -> bad   */
  {
     if (diag_mode) {
	PRINT(("gswintr: In Diag. mode, parity intr.\n"));
	intr_cntptr->total_cnt++;
	intr_cntptr->parity_cnt++;
	BUSMEM_DET(segreg);
	CLR_PARITYJMP(&(lclparbuf))
        TRCHKL0T(hkwd_INTR_R2);
	i_reset(ihp);
	return(0);
     }
     PRNTE(("gswintr: BAD PARITY. msla_stat = 0x%x.\n",msla_stat));
     gerr_log(0,NULL,intr_name,(int)ParityError,msla_stat,ras_unique[0]);
     if (cap->ipf == 0)
     {
	 BUSMEM_DET(segreg);
	 CLR_PARITYJMP(&(lclparbuf))
         TRCHKL0T(hkwd_INTR_R3);
	 i_reset(ihp);
	 return(0);                             /* and exit             */
     }
     PRNTE(("gswintr: BAD PARITY. CONTINUE. msla_stat=0x%x.\n",msla_stat));
     PRNTE((".... cap = 0x%x 0x%x\n", *((uint *)cap),*(((uint *)cap)+1)  ));
  }                                             /* else continue thru   */

  if (diag_mode) {
	PRINT(("gswintr: In Diag. mode, general intr.\n"));
	intr_cntptr->total_cnt++;
	BUSMEM_DET(segreg);
	CLR_PARITYJMP(&(lclparbuf))
        TRCHKL0T(hkwd_INTR_R4);
	i_reset(ihp);
	return(0);
  }

  if (!cap->ipf)                                /* IPF not set          */
     {
	BUSMEM_DET(segreg);
	gerr_log(0,NULL,intr_name,(int)IPFnotSet,msla_stat,ras_unique[0]);
	CLR_PARITYJMP(&(lclparbuf))
        TRCHKL0T(hkwd_INTR_R5);
	i_reset(ihp);
	return(-1);                             /* return               */
     }

  if (mf.ipl_in_prog)                           /* during msla ipl      */
	HALT_MSLA                               /* avoids parity errs   */

  /*--------------------------------------------------------------------*/
  /* reach here if it is our interrupt.                                 */
  /* Determine minor device by checking LDA field.                      */
  /*--------------------------------------------------------------------*/
  for (i = 0; i < NumDevSupp; i++)              /* find minor device    */
  {
	if (cap->lda == (mgcb_ptr+i)->lda)
		break;
  }
  dev = i;                                      /* if not found, dev =  */

  TRCHKL5T(hkwd_INTR_DATA,dev,(int)cap->icc,(int)cap->ccc,
	   (uint)(*((char *)cap+1)),(uint)(*(short *)(&mf)));
  g = mgcb_ptr + (dev % NumDevSupp);            /*       NumDevSupp+1   */

#ifdef FPGI
/*att_done no longer exists* if((g->lda == cap->lda)&&(g->iflag.att_done)*/
      && (g->openmode == FPGI_MODE) )           /* got it!              */
	{
	   BUSMEM_DET(segreg);
	   CLR_PARITYJMP(&(lclparbuf))
	   i_reset(ihp);
	   return(0);
	   /* return(fpintrpt(g)); */   /* PUT BACK - WHEN FPGI DONE  */
	}
#endif FPGI


  /*--------------------------------------------------------------------*/
  /* Check ICC field of comm_area to determine the category -           */
  /*    Cmd             0                                               */
  /*    LinkSwitch      1                                               */
  /*    Diag            2                                               */
  /*    Debug           3                                               */
  /*    Init            4                                               */
  /*    ProgCode        5                                               */
  /*    UnsolDevReq     6                                               */
  /*    AdapterErr      7                                               */
  /*                                                                    */
  /*                                                                    */
  /*  NOTE:                                                             */
  /*    The original driver interrupt routine was coded by searching    */
  /*    for the error case situations first; then all else was ok.      */
  /*    This new version does the following:                            */
  /*    . determine the category code                                   */
  /*    . check the bits in the comm_area for the correct situations    */
  /*      that can occur (e.g., AE, AE+DE, DE, etc)                     */
  /*    . any other bit combinations are invalid and are logged as such.*/
  /*                                                                    */
  /*    Also note that the LDA value in the COMM area is not set to a   */
  /*    valid value until the config interrupt occurs. Before that time,*/
  /*    the LDA is 0. This is not an error for cases like ProgCode,     */
  /*    Debug, Diag, but is an error for UnSolDevReq and Cmd.           */
  /*                                                                    */
  /*                                                                    */
  /*  Legend:                                                           */
  /*    cap - communications area ptr                                   */
  /*    st1 - local variable holding 1st status byte from comm area     */
  /*    st2 - local variable holding 2nd status byte from comm area     */
  /*                                                                    */
  /*          st1 is "struct stat1"                                     */
  /*          st2 is "struct stat2"                                     */
  /*                                                                    */
  /*                                                                    */
  /*--------------------------------------------------------------------*/
  chp     = ((char *)cap) + 1;
  *st1chp = *chp;
  *st2chp = *(chp+1);

  PRINT(("gswintr: *st1chp=0x%x,*st2chp=0x%x,cap->icc=0x%x,cap->ccc=0x%x\n",
	 *st1chp,*st2chp,cap->icc,cap->ccc));
  PRINT(("gswintr: lda = 0x%x, cap->vda.ch[0-3] = 0x %x %x %x %x \n",
	   cap->lda,
	   cap->vda.ch[0],cap->vda.ch[1],cap->vda.ch[2],cap->vda.ch[3]));

  switch (cap->icc)
  {
    case Cmd:                           /* response to a cmd           0*/
    if (*st1chp & AE)                   /* reset any outstanding timer 0*/
	CANCEL_TMR(IoTmrMask);
    if (*st1chp & ~(AE | DE))           /* if non-successful,           */
    {
	if (g->oflag.io_waiting)        /* if async io operation,       */
	    SIGAPPL(SIGIOINT);          /* notify user last one failed  */
	g->op_res = *st1chp;            /* set op_res to status         */
    }
    else                                /* else                         */
	g->op_res = 0;                  /* zero it out (successful)     */

    if (dev >= NumDevSupp)
    {
	gerr_log(dev,NULL,intr_name,FATAL_IO_ERR,NoLdaDev,
		 ras_unique[1]);
        serious_err = TRUE;             /*     later                   0*/
        break;
    }
 
    if (g->b.dc || g->b.cc)             /* if cmd or data chaining     0*/
	ccbp = (struct ccb *)hdr_offset;/* ccbp is ptr to ccb header   0*/
 
    /*-----------------------------------------------------------------0*/
    /* Now ready to check status bits in stat1 and stat2.              0*/
    /* Stat1 has the 'normal' conditions like AE, AE+DE                0*/
    /*-----------------------------------------------------------------0*/
    switch (*st1chp)
    {
      case  AE+DE :                     /* normal completion          A0*/
 
	/*------------------------------------------------------------A0*/
        /* Check that the comm area lda matches the ae_expected_lda.  A0*/
        /* Check that we are expecting AE + DE.                       A0*/
        /* Make a single call to gerr_log   if any error occurred.    A0*/
        /* Turn off ae_exp and de_exp flags in struct ld.             A0*/
        /* If unsolicited rmi in progress, enque an unsol intr to vmi A0*/
        /*      that will eventually reach the AIX driver.            A0*/
	/*      Also, remove rmi request from the rmiq that is        A0*/
        /*      maintained to keep track of outstanding rmi's.        A0*/
        /*------------------------------------------------------------A0*/
        if (ae_exp_lda != cap->lda)     /* lda's dont match. Error.   A0*/
        {
	  gerr_log(dev,NULL,intr_name,FATAL_IO_ERR,UnMtchLda,ras_unique[2]);
          serious_err = TRUE;           /*     later                   0*/
          break;
        }
        if (!g->c.ae_exp ||        /* if we were not expecting   A0*/
            !g->c.de_exp)          /* AE or DE, we got an error! A0*/
        {
	  gerr_log(dev,NULL,intr_name,FATAL_IO_ERR,UnExpdAe_De,
		 ras_unique[3]);
          serious_err = TRUE;           /*     later                   0*/
          break;
        }
 
 
        /*------------------------------------------------------------A0*/
        /* Reach here if no error in case AE+DE                       A0*/
        /*------------------------------------------------------------A0*/
	rc = undo_dma(g);
	if (rc)
	{
	        gerr_log(dev,intr_name,undodma_name,BAD_UNDODMA,
		 rc,ras_unique[4]);
		serious_err = TRUE;
		break;
	}

        /*------------------------------------------------------------A0*/
        /* If data chaining -                                         A0*/
        /*      . set ae_exp to TRUE  (still expect more until last)  A0*/
        /*      . update ptr to ce  (ce_offset )                      A0*/
        /*      . update ptr to ccw (ccw_offset)                      A0*/
	/*      . issue send_sol                                      A0*/
        /*      . break out                                           A0*/
        /*------------------------------------------------------------A0*/
	if (g->b.dc)                    /* data    chaining?          A0*/
        {
            g->c.ae_exp = TRUE;
            ccw_offset++;
            ce_offset++;
	    send_sol(ccbp,dev,g->xm.dp+(g->xm.indx));     /*DMA call AA0*/
            break;
        }                               /*                            A0*/
 
        g->c.ae_exp = FALSE;       /* no longer waiting for AE   A0*/
        g->c.de_exp = FALSE;       /* no longer waiting for DE   A0*/
 
	if (mf.rmi_in_prog)
	{
		comm_lock = 0;          /* free comm_lock (io is done   */
		/*------------------------------------------------------*/
		/* must be ank, lpfk, pfk  .                            */
		/* x90 00 00 is CANCEL                                  */
		/* xA0 00 00 is ENTER                                   */
		/* x40 xx FF is LPFK                                    */
		/* x48 xx FF is LPFK                                    */
		/* all else are acceptable SMI                          */
		/*------------------------------------------------------*/
		data_ptr = g->rmip;
                switch (*data_ptr) {
                        case (BIT0 + BIT2):             /* cancel       */
                        case (BIT0 + BIT3):             /* enter        */
                            if (*(data_ptr+1) || *(data_ptr+2) )
                                type = Gsmi;
                            else
                                type = Gank ;
                            break;
                        case  BIT1 :                    /* lpfk         */
                            if (*(data_ptr+2) == 0xff)
                                type = Glpfk;
                            else
                                type = Gsmi;
                            break;
                        case (BIT1 + BIT4):             /* pfk          */
                            if (*(data_ptr+2) == 0xff)
                                type = Gpfk;
                            else
                                type = Gsmi;
                            break;
                        default:
                            type = Gsmi;
                            break;
                }
		ret_flag = FALSE;
	        TRCHKL1T(hkwd_INTR_R10,(uint)type);
					/* remove request from queue   0*/
		rmiq[rmiqtail] = (ushort)(-1);
		stat_accp(dev);         /* send status acpt to adap    0*/

		/*-----------------------------------------------------0*/
		/* GSWIO routine may be waiting to be awakened.        0*/
		/*-----------------------------------------------------0*/
		if (g->io_pend_cnt)
		    g->io_pend_cnt--;
		if (g->sleep_sio != (int *)EVENT_NULL )
		{
		    AWAKE_SIO
		    g->oflag.wokeup = TRUE;
		    g->oflag.io_waiting = FALSE;
		}

		chp  = (char *)(&g->b);
		*chp = 0;
		chp  = (char *)(&g->c);
		*chp = 0;
		ae_exp_cmd     = UndefCmd;
		mf.sol_in_prog = FALSE;
		mf.rmi_in_prog = FALSE;

		/*------------------------------------------------------*/
		/* Copy data to g->intp area to use later when io done. */
		/* Start the i/o and exit.                              */
		/*------------------------------------------------------*/
		bcopy(data_ptr,g->intp,RMI_MAX);
		this_hdr = (g->hdr_ptr) + (int)type;
		switch ((int)type)
		{
		  case Gsmi :
		      bcopy(data_ptr+1,&qual.byt[2],2); /* 2 bytes SMI  */
		      tablep = gfnd_qual(this_hdr->intrp,(int)type,qual.wd);
		      if (tablep == NULL)
			    ret_flag = TRUE;
		      else if
			((tablep->data_len) || (tablep->w_adr != (char *) -1))
		      {
					/* DMA read to malloced area    */
					/* then exit. Later, the io     */
					/*  will complete.              */
			    rc = gint_io(dev,tablep,g->intp+RMI_MAX);
			    if (rc)
			       PRNTE((
			       "gswintr: Failure of call to 'gint_io'\n"));
			    ret_flag = TRUE;
			    g->intio_type = (int)type;
		      }
		      break;
		  default :
		      tablep = this_hdr->intrp;
		      if (tablep == NULL)
			    ret_flag = TRUE;
		      else if (tablep->data_len)
		      {
					/* DMA read to malloced area    */
					/* then exit. Later, the io     */
					/*  will complete.              */
			    rc = gint_io(dev,tablep,g->intp+RMI_MAX);
                            if (rc)
			       PRNTE((
			       "gswintr: Failure of call to 'gint_io'\n"));
			    ret_flag = TRUE;
			    g->intio_type = (int)type;
		      }
		      break;
		}
		break;
	} /* end of rmi_in_prog case */
 
        /*------------------------------------------------------------A0*/
	/* The ccw and ce offsets are set at the end of the intrpt    A0*/
        /* handler were the rest of the cmd chaining processing is    A0*/
        /* done.                                                      A0*/
        /*------------------------------------------------------------A0*/
	if (!g->b.cc)                   /* not command chaining       A0*/
        {
	    stat_accp(dev);             /* send status acpt to adap  AA0*/
	    if (g->io_pend_cnt)
            {
		g->io_pend_cnt--;
		AWAKE_SIO
		g->oflag.wokeup = TRUE;
		g->oflag.io_waiting = FALSE;
            }
                                        /* clean up flags            AA0*/
                                        /* 0  chain flags in LD      AA0*/
	    chp  = (char *)(&g->b);
            *chp = 0;
            chp  = (char *)(&g->c);
            *chp = 0;
            ae_exp_cmd     = UndefCmd;
            mf.sol_in_prog = FALSE;
            mf.rmi_in_prog = FALSE;
        }
	comm_lock = 0;                  /* free comm area             A0*/
        break;  /*------------ end AE+DE -----------------------------A0*/
 
      case  AE+DE+ATTN+UC :             /* crnt cmd rejected          B0*/
                                        /* (non-error case as per     B0*/
                                        /*  orig dd )                 B0*/
	/********** REMOVE THIS LINE ****************/
	g->c.ae_exp = TRUE;             /* set up to repeat the cmd,  B0*/
	/********** REMOVE THIS LINE ****************/
	rc = undo_dma(g);
	if (rc)                                 /* failure              */
	{
	        gerr_log(dev,intr_name,undodma_name,BAD_UNDODMA,
		         rc,ras_unique[5]);
		serious_err = TRUE;
		break;
	}
	/********** REMOVE THESE TWO LINES ****************/
	SET_TMR(IoTmrVal,0,IoTmrMask);  /* set timer,                 B0*/
        INT_MSLA                        /* interrupt the msla         B0*/
	/********** REMOVE THESE TWO LINES ****************/
        break;  /*------------ end AE+DE+ATTN+UC ---------------------B0*/
 
 
      case  AE :
 
        /*------------------------------------------------------------C0*/
        /* Check that the comm area lda matches the ae_expected_lda.  C0*/
        /* Check that we are expecting AE.                            C0*/
        /* Make a single call to gerr_log   if any error occurred.    C0*/
        /* Turn off ae_exp flag in struct LD.                         C0*/
        /*------------------------------------------------------------C0*/
 
        if (ae_exp_lda != cap->lda)     /* lda's dont match. Error.   C0*/
        {                               /*                            C0*/
	  gerr_log(dev,NULL,intr_name,FATAL_IO_ERR,
		        UnMtchLda,ras_unique[6]);
          serious_err = TRUE;           /*     later                   0*/
          break;
        }
	if (!g->c.ae_exp)               /* if we were not expecting   C0*/
                                        /* AE      , we got an error! C0*/
        {
	  gerr_log(dev,NULL,intr_name,FATAL_IO_ERR,
		        UnExpdAe_De,ras_unique[7]);
          serious_err = TRUE;           /*     later                   0*/
          break;
        }
 
 
        /*------------------------------------------------------------C0*/
        /* Reach here if no error in case AE.                         C0*/
        /* Turn off AE_EXP flag.                                      C0*/
        /* Check if data chaining.                                    C0*/
        /*------------------------------------------------------------C0*/
 
        g->c.ae_exp = FALSE;       /* no longer waiting for AE   C0*/
 
 
        /*------------------------------------------------------------C0*/
        /* If data chaining -                                         C0*/
        /*      . set ae_exp to TRUE  (still expect more until last)  C0*/
        /*      . update ptr to ce  (ce_offset )                      C0*/
        /*      . update ptr to ccw (ccw_offset)                      C0*/
	/*      . issue send_sol                                      C0*/
        /*      . break out                                           C0*/
        /*------------------------------------------------------------C0*/
 
	if (g->b.dc)                    /* data    chaining?          C0*/
        {
	    rc = undo_dma(g);
	    if (rc)                             /* failure              */
	    {
	        gerr_log(dev,intr_name,undodma_name,BAD_UNDODMA,
		         rc,ras_unique[8]);
		serious_err = TRUE;
		break;
	    }
	    g->c.ae_exp = TRUE;
            ccw_offset++;
            ce_offset++;
	    send_sol(ccbp,dev,g->xm.dp+(g->xm.indx));     /*DMA call AC0*/
            break;
        }
 
        /*------------------------------------------------------------C0*/
        /* Send status accepted to msla.                              C0*/
        /*------------------------------------------------------------C0*/
	stat_accp(dev);
 
        break;  /*------------ end AE --------------------------------C0*/
 
 
      case  DE :
 
	/*------------------------------------------------------------D0*/
        /* Check that we are expecting DE.                            D0*/
        /* Make a single call to gerr_log   if any error occurred.    D0*/
        /* Turn off de_exp flag in struct LD.                         D0*/
        /*------------------------------------------------------------D0*/
 
        if (!g->c.de_exp)               /* if we were not expecting   D0*/
                                        /* DE      , we got an error! D0*/
        {
	  gerr_log(dev,NULL,intr_name,FATAL_IO_ERR,
		         UnExpdAe_De,ras_unique[9]);
          serious_err = TRUE;           /*     later                   0*/
          break;
        }
 
 
	if (g->dma_location != NO_DMA_IN_PROG)
	{
	    rc = undo_dma(g);
	    if (rc)                             /* failure              */
	    {
	        gerr_log(dev,intr_name,undodma_name,BAD_UNDODMA,
		         rc,ras_unique[10]);
		serious_err = TRUE;
		break;
	    }
	}
	/*------------------------------------------------------------D0*/
        /* If data chaining -                                         D0*/
        /*      . set ae_exp to TRUE  (still expect more until last)  D0*/
        /*      . update ptr to ce  (ce_offset )                      D0*/
        /*      . update ptr to ccw (ccw_offset)                      D0*/
	/*      . issue send_sol                                      D0*/
        /*      . break out                                           D0*/
        /*------------------------------------------------------------D0*/
 
	if (g->b.dc)                    /* data    chaining?          D0*/
        {
            g->c.ae_exp = TRUE;
            ccw_offset++;
            ce_offset++;
	    send_sol(ccbp,dev,g->xm.dp+(g->xm.indx));     /*DMA call AD0*/
            break;
        }
 

        /*------------------------------------------------------------D0*/
        /* Turn off DE_EXP flag.                                      D0*/
        /*------------------------------------------------------------D0*/
 
	g->c.de_exp = FALSE;            /* no longer waiting for DE    0*/
 
	if (!g->b.cc)                   /* not command chaining       D0*/
        {
	    stat_accp(dev);             /* send status acpt to adap  AD0*/
	    if (g->io_pend_cnt)
            {
		g->io_pend_cnt--;
		AWAKE_SIO
		g->oflag.wokeup = TRUE;
		g->oflag.io_waiting = FALSE;
            }
                                        /* clean up flags            AD0*/
                                        /* 0  chain flags in LD      AD0*/
            chp  = (char *)(&g->b);
            *chp = 0;
            chp  = (char *)(&g->c);
            *chp = 0;
            ae_exp_cmd     = UndefCmd;
            mf.sol_in_prog = FALSE;
            mf.rmi_in_prog = FALSE;
        }
	comm_lock = 0;                  /* free comm area             D0*/
        break;  /*------------ end DE --------------------------------D0*/
 
 
      default :                         /* not one of the above, so   E0*/
                                        /* some kind of error.        E0*/
        /*------------------------------------------------------------E0*/
        /* Check for UC.                                              E0*/
        /* There are multiple errors that could have happened -       E0*/
        /*      bus out check                                         E0*/
        /*      sync data check                                       E0*/
        /*      intervention required                                 E0*/
        /*      h/w error                                             E0*/
        /*      cmd reject                                            E0*/
        /*      struct field err                                      E0*/
        /*      and others defined in the design spec.                E0*/
        /*                                                            E0*/
        /* The spec says that many of these should be logged by the   E0*/
        /* driver.                                                    E0*/
        /*------------------------------------------------------------E0*/
	PRNTE(("gswintr: ERROR. default Cmd case .......\n"));
	PRNTE(("   *st1chp=0x%x,*st2chp=0x%x,g->io_pend_cnt=0x%x\n",
				*st1chp,*st2chp,g->io_pend_cnt));

	if (!mf.retry_in_prog &&                /* busy  - retry it.  E0*/
            cap->vda.uns_sen.bo_chk)
        {
	    PRINT(("gswintr: setting retry_in_prog\n"));
            mf.retry_in_prog = TRUE;
	    g->c.ae_exp = TRUE;         /* set up to repeat the cmd, AE0*/
	    SET_TMR(IoTmrVal,0,IoTmrMask);      /*set timer,         AE0*/
	    INT_MSLA                            /* interrupt msla    AE0*/
	    break;                              /* exit              AE0*/
        }
 
	reason      = FATAL_IO_ERR;
	serious_err = TRUE;
        mf.retry_in_prog = FALSE;
 
	if (st1.de)                             /* DE present         E0*/
        {
	    PRNTE(("gswintr: In the default Cmd case with DE on\n"));
	    if (g->io_pend_cnt ||               /* outstanding io or AE0*/
		mf.rmi_in_prog )                /*  rmi in progress  AE0*/
            {
/** Undo_dma moved inside this test **/
	        rc = undo_dma(g);
	        if (rc)                             /* failure              */
	        {
	           gerr_log(dev,intr_name,undodma_name,BAD_UNDODMA,
		         rc,ras_unique[11]);
		   serious_err = TRUE;
	       	   break;
	        }
		ui = FATAL_IO_ERR;
		if(st1.uc)                      /* if UC,           AAE0*/
		{
		    serious_err  = FALSE;
		    tempuint     = UnitChekOn;
                    sense_status = get_sense(dev);
		    PRNTE(("gswintr: sense = 0x%x %x %x %x \n",
			    cap->vda.ch[0],cap->vda.ch[1],cap->vda.ch[2],
			    cap->vda.ch[3]));
		    if (sense_status == SENSE_PRESENT)
		    {
			ui = BAD_SIO_SENSE;
                        reason = BAD_SIO_SENSE;
			g->sen_hdrp->tail = (g->sen_hdrp->tail + 1)
					       % SENSE_MODULO;
		    }
		}
		else
		    tempuint = 0;

		stat_accp(dev);                 /* Send stat accpt  AAE0*/
		if (mf.rmi_in_prog)             /*  rmi in progress AAE0*/
                {
		  gerr_log(dev,NULL,intr_name,NO_RMI_DATA,
                           tempuint,ras_unique[12]);
                  reason = NO_RMI_DATA;
		  rmiq[rmiqtail] = (ushort)(-1);
                }
                else 
		  gerr_log(dev,NULL,intr_name,ui,tempuint,ras_unique[13]);
 
                /*--------------------------------------------------AAE0*/
                /* Cleanup before exiting.                          AAE0*/
                /*--------------------------------------------------AAE0*/
		cancel_sio(dev);
                comm_lock = 0;
		break;                          /* exit case st1    AAE0*/
            }
 
            /*-------------------------------------------------------AE0*/
            /* Reach here if DE and serious error not handled above. AE0*/
            /*-------------------------------------------------------AE0*/
 
       	    gerr_log(dev,NULL,intr_name,FATAL_IO_ERR,
			   NoCmdOwner,ras_unique[14]);
	    serious_err = TRUE;                 /* fatal             AE0*/
            break;
        }
	else                                    /* No DE - wierd err AE0*/
        {
       	    gerr_log(dev,NULL,intr_name,FATAL_IO_ERR,
			   NoDeFailCmd,ras_unique[15]);
	    serious_err = TRUE;                 /* fatal             AE0*/
            break;
        }
	break;
                /*------------ end default of "case (*st1chp)" -------E0*/
    } /*------------ end switch (*st1chp" -----------------------------0*/
    break;  /*------------ end of "case CMD" of "switch (cap->icc)"----0*/

 
    case Debug:                         /* Debug                       3*/
 
    if((uint)(cap->ccc) <= 12)
    {
       if (counter_err < 32)            /* normally groups of 32        3*/
		counter_err++;
       else if (counter_err == 32)      /* normally groups of 32        3*/
		counter_err = 0;
       else {
	  gerr_log(dev,NULL,intr_name,FATAL_IO_ERR,StatErr,ras_unique[16]);
          counter_err = 0;
       }
    }
    break;  /*------------ end Debug ----------------------------------3*/
 
    case Init:                          /* Init                        4*/
 
    if (*st1chp == ATTN)                /* ATTN only                   4*/
    {
        switch (cap->ccc)               /* check reason code          A4*/
        {
 
          case DevNotPres :             /* device not present        AA4*/
	    reason = MSLA_NO_5080;
	    for (i = 0; i < NumDevSupp; i++)
		AWAKE_OPEN(i);
	    break;
          case DevNotResp :             /* device not responding     AA4*/
	    reason = MSLA_NO_RQ;
	    for (i = 0; i < NumDevSupp; i++)
		AWAKE_OPEN(i);
	    break;
	  case EnterMonMode :           /* enter monitor mode        AA4*/
	    reason = MSLA_LM;
	    for (i = 0; i < NumDevSupp; i++)
		AWAKE_OPEN(i);
	    break;
	  case LinkDown :               /* link down                 AA4*/
	    reason = MSLA_NO_HOST;
	    for (i = 0; i < NumDevSupp; i++)
		AWAKE_OPEN(i);
            break;
 
          case ConfigDataPres :         /* config data present       AA4*/
            /*-------------------------------------------------------AA4*/
            /* Check config lda field in comm area for non-zero lda  AA4*/
            /* For each device, adjust config_lda flag and lda value AA4*/
            /* in struct LD.                                         AA4*/
            /* If not configured, error log.                         AA4*/
            /*-------------------------------------------------------AA4*/
 
             gbak = g;
	     for (i = 0; i < NumDevSupp; i++)
	     {
		  g = mgcb_ptr + i;
		  if (cap->vda.conf_lda[i])
			g->lda = cap->vda.conf_lda[i];
                  else
		  {
		    g->lda                  = LdaLimit + 1;
                    g->iflag.not_configured = TRUE;
                    g->uns_res              = NOT_CONFIGURED;
		    /*********************
		    gerr_log(i,NULL,intr_name,NOT_CONFIGURED,
			   0,ras_unique[17]);
		    *********************/
                    ret_flag = TRUE;
                  }
	     }
	     g = gbak;
	     break;
 
          default:
	     break;
 
        } /* end "switch (cap->ccc)" */                         /*    A4*/
    }     /* end "if (st1 == ATTN)"  */                         /*     4*/
 
    else                                /* Not just an ATTN            4*/
    {
        /*-------------------------------------------------------------4*/
        /* Reach here if -                                             4*/
	/*        stop required or                                     4*/
        /*        start required  or                                   4*/
	/*        stop in progress or                                  4*/
	/*        start in progress or                                 4*/
	/*        error                                                4*/
        /*-------------------------------------------------------------4*/
      if ( ((cap->ccc == StartCmd) && (ae_exp_cmd == StartCmd)) ||
             ((cap->ccc == StopCmd ) && (ae_exp_cmd == StopCmd )) )
      {
	CANCEL_TMR(StopStrtTmrMask);            /* reset timer         4*/
	if (mf.need_start)                      /* start is required   4*/
        {
            mf.start_in_prog = TRUE;
            mf.stop_in_prog  = FALSE;
            mf.need_start    = FALSE;
            mf.need_stop     = FALSE;
            stop_strt();
        }
	else if (mf.need_stop)                  /* stop  is required   4*/
        {
            mf.start_in_prog = FALSE;
            mf.stop_in_prog  = TRUE;
            mf.need_start    = FALSE;
            mf.need_stop     = FALSE;
            stop_strt();
        }
	else if (mf.stop_in_prog)               /* stop is in progress 4*/
        {
            mf.stop_in_prog  = FALSE;
            ipl_flag = TRUE;
                                                        /* disable chan */
	    for (i = 0; i < NumDevSupp; i++)
		  (mgcb_ptr+i)->lda = LdaLimit + 1;

	    DIS_INTR_MSLA                               /* disable dvr  */
	    d_mask(dma_chanid);                         /* disable chan */
	    d_clear(dma_chanid);                        /* free DMA chan*/
            mf.dma_enabled = FALSE;                     /* set disabl fl*/
            comm_lock = 0;
        }
	else if (mf.start_in_prog)              /* start in progress   4*/
	{
	    mf.start_in_prog = FALSE;
	    comm_lock = 0;
	}
	else                            /* none of the 4 above cases   4*/
        {
	    PRNTE(("gswintr: Init case - none of the above.ccc = 0x%x\n",
		cap->ccc));
	    PRNTE(("   ae_exp_cmd = 0x%x,*st1chp = 0x%x\n",ae_exp_cmd,
		*st1chp));
        }
 
      } /* end of "if stop or start required " */
    } /* end of "else" for not attn only */
 
 
    if (reason)
    {
       PRINT(("gswintr: reason = 0x%x. dev = 0x%x\n",reason,dev));
       switch (reason) {
 
        case MSLA_LM :
		for (i = 0;   i < NumDevSupp ;i++) {
                        (mgcb_ptr+i)->iflag.gsw_switched = TRUE;
                        (mgcb_ptr+i)->iflag.not_to_ready = FALSE;
                        (mgcb_ptr+i)->iflag.io_allowed   = FALSE;
                        (mgcb_ptr+i)->uns_res = reason;
                }
		/**************************
       	        gerr_log(dev,NULL,intr_name,(int)reason,
			   0,ras_unique[18]);
		**************************/
                type = Klink_sw;
                break;
 
        case MSLA_NO_5080 :
		for (i = 0; i < NumDevSupp; i++) {
                        (mgcb_ptr+i)->iflag.io_allowed   = FALSE;
                        (mgcb_ptr+i)->iflag.not_to_ready = FALSE;
                        (mgcb_ptr+i)->uns_res = reason;
                }
       	        gerr_log(dev,NULL,intr_name,(int)reason,
			   0,ras_unique[19]);
                type = Kno_5080;
                break;
 
        case MSLA_NO_RQ :
        case MSLA_NO_HOST :
        default :
       	        gerr_log(dev,NULL,intr_name,(int)reason,
			   0,ras_unique[20]);
                ret_flag = TRUE;
       } /* end of uns_reason */
    }
    break;  /*------------ end Init -----------------------------------4*/
 
 
    case ProgCode:                      /* Progress Code               5*/
 
    /*-----------------------------------------------------------------5*/
    /* Reset outstanding timer.                                        5*/
    /* If prog code = 0 ("all ok and complete"),                       5*/
    /*      start the msla                                             5*/
    /*      turn off the ipl_in_prog flag                              5*/
    /*      turn on start_in_prog flag                                 5*/
    /*      turn off stop_in_prog flag                                 5*/
    /*      turn off start_required flag                               5*/
    /*      call stop_strt                                             5*/
    /*  Else                                                           5*/
    /*      set up a timer for another progress code                   5*/
    /*                                                                 5*/
    /*-----------------------------------------------------------------5*/
    CANCEL_TMR(ProgRptTmrMask);

    if (cap->vda.vdahw.vdahw1 == 0)
    {
        STRT_MSLA
        mf.ipl_in_prog   = FALSE;
        mf.stop_in_prog  = FALSE;
        mf.start_in_prog = TRUE;
        mf.need_start    = FALSE;
        stop_strt();
    }
    else
    {
	SET_TMR(ProgRptTmrVal, 0, ProgRptTmrMask);
    }
 
    break;  /*------------ end ProgCode -------------------------------5*/
                                        /*                              */
    case UnsolDevReq:                   /* Unsolicited dev request     6*/
 
    switch (*st1chp)
    {
      case  ATTN+DE+UE :                /* attn + dev end + unit excp A6*/
                                        /* (not ready to ready)       A6*/
        /*------------------------------------------------------------A6*/
        /* Find the lda that was configured.                          A6*/
        /*------------------------------------------------------------A6*/
            if ((cap->vda.nrtr_data.nrtr_lda[dev] == cap->lda) &&
                (cap->vda.nrtr_data.nrtr_lda[dev] != 0) )
            {                                   /* found the lda!    AA6*/
		g->lda = cap->lda;
 
		/*------------------------------------------------------*/
		/* get this when device comes online. should get one    */
		/* after attach. send wakeup to open routine who        */
		/* is waiting before it does set mode/rmi.              */
		/*------------------------------------------------------*/
 
                                        /* if didnt use openx, dont     */
                                        /* allow io (must close) if     */
                                        /* user switched to host while  */
                                        /* open.                        */
 
                if ( (!(g->oflag.link_sw)) && (g->iflag.gsw_switched) &&
                     (g->oflag.gsw_open) )
                        g->iflag.io_allowed = FALSE;
                else
                        g->iflag.io_allowed = TRUE;
 
		/*------------------------------------------------------*/
		/* a NRTR occurs when 508x is switched back from host   */
		/*------------------------------------------------------*/
                g->iflag.gsw_switched   = FALSE;
                g->iflag.not_configured = FALSE;
 
		g->iflag.not_to_ready = TRUE;
                if (g->oflag.slp_pending)
	        	AWAKE_OPEN(dev);
		if (g->oflag.link_sw)
		{
			type = Knot_to_ready;
			g->iflag.io_allowed = TRUE;
			break;
		}
                ret_flag = TRUE;
            }
        break;                          /* end "case ATTN+DE+UE"------A6*/
 
 
      case  ATTN :                      /* attn only                  B6*/
 
	/*------------------------------------------------------------B6*/
        /* Check for unsolicited rmi pending.                         B6*/
        /*     if so, ignore it                                       B6*/
	/*     if not, put dev on rmiq for processing later.          B6*/
        /*------------------------------------------------------------B6*/
 
        for (i = 0; i < NumDevSupp+1; i++)
        {
	    if (rmiq[i] == dev)         /* if found, break out       AB6*/
                break;                  /*                          AAB6*/
        }
	if (i > NumDevSupp)             /* none on queue(for this dev)  */
        {
	    rmiqhead       = ( (rmiqhead + 1) % (NumDevSupp + 1) );
	    rmiq[rmiqhead] = dev;
        }
        break;                          /* end "case ATTN"------------B6*/
 
 
      case  ATTN+UC :                   /* attn and uc                C6*/
 
        /*------------------------------------------------------------C6*/
	/* Check if minor device has been recognized yet.             C6*/
        /*------------------------------------------------------------C6*/
 
	if (dev >= NumDevSupp)          /* unattached dev intrpt      C6*/
        {                               /*     error                  C6*/
           /* Changed the err_type param from 'reason' to UNATT_DEV */
       	    gerr_log(dev,NULL,intr_name,UNATT_DEV,
			   0,ras_unique[21]);
            break;                      /* finished - so exit case   AC6*/
        }
 
        /*------------------------------------------------------------C6*/
        /* Get unsolicited sense data.                                C6*/
	/* Set 'data_ptr' to the sense data in the sense q,           C6*/
	/* bump sense queue 'tail' ptr to be ready for next sense.    C6*/
        /*------------------------------------------------------------C6*/
 
        sense_status = get_sense(dev);
        switch ( sense_status ) {
                        case SENSE_PRESENT :
			    ret_flag = FALSE;
                            data_ptr = (char *)( g->sen_datap +
                                       g->sen_hdrp->tail);
                            g->sen_hdrp->tail = (g->sen_hdrp->tail + 1)
                                               % SENSE_MODULO;
                            if  ((*data_ptr)  & ASYNC_ERR) {
                       	        gerr_log(dev,NULL,intr_name,
			                 BAD_ASY_SENSE,0,ras_unique[22]);
				cap->ipf = 0;   /* clear intrpt pend flg*/
				BUSMEM_DET(segreg);
				CLR_PARITYJMP(&(lclparbuf))
                                TRCHKL1T(hkwd_INTR_R6,(uint)BAD_ASY_SENSE);
				i_reset(ihp);
       				return(0);  
                            }
 
                            else switch(*(data_ptr + 1)) {
                                case(BIT0):
                                case(BIT0+BIT2):
                                case(BIT0+BIT5):
                                case(BIT0+BIT2+BIT5):
                                    type = Gpick;
                                    break;
                                case(BIT0+BIT6):
                                    type = Gtablet;
                                    break;
                                case(BIT1):
                                    type = Ggeop;
                                    bcopy(data_ptr+6,&qual.byt[0],2);
                                    bcopy(data_ptr+2,&qual.byt[2],2);
				    /*----------------------------------*/
				    /* Copy sense data to g->intp area  */
				    /* use later when GEOP i/o data     */
				    /* done. Start the i/o and exit.    */
				    /*----------------------------------*/
				    bcopy(data_ptr,g->intp,SEN_MAX);
				    this_hdr = g->hdr_ptr + Ggeop;
				    tablep   = gfnd_qual(
					       this_hdr->intrp,
					       (int)Ggeop,qual.wd);
				    if (tablep == NULL)
				       ret_flag = TRUE;
				    else if
					   ( (tablep->data_len) ||
					     (tablep->w_adr != (char *) -1) )
				    {
				       /* DMA read to malloced area    */
				       /* then exit. Later, the io     */
				       /*  will complete.              */
				       rc = gint_io(dev,
						    tablep,
						    g->intp+SEN_MAX);
                            	       if (rc)
				       PRNTE((
					"gswintr: FAILURE gint_io\n"));
				       ret_flag = TRUE;
				       g->intio_type = Ggeop;
				    }
                                    break;
                                case(BIT7):
                                    type = Gpgm_err;
                                    break;
                                default:
                       	            gerr_log(dev,NULL,intr_name,
			                 BAD_ASY_SENSE,0,ras_unique[23]);
				    cap->ipf = 0;       /* 0 ifg flag   */
				    BUSMEM_DET(segreg);
				    CLR_PARITYJMP(&(lclparbuf))
                                    TRCHKL1T(hkwd_INTR_R6,(uint)BAD_ASY_SENSE);
				    i_reset(ihp);
       				    return(0);  
                            }
                            break;
 
                        case SENSE_AREA_FULL :
                        case NO_SENSE_AREA :
                        case NO_SENSE_TIMEOUT :
                        default :
                       	    gerr_log(dev,NULL,intr_name,
			                 sense_status,0,ras_unique[24]);
                            bad_data = TRUE;
			    reason = sense_status;
                            break;
        }
        break;                          /* end "case ATTN+UC"---------C6*/
 
      default:
	  gerr_log(dev,NULL,intr_name,
		   BAD_UNSOL_DEV_REQ,*st1chp,ras_unique[25]);
	break;
    }   /* end "switch (*st1chp)" ------------------------------------C6*/
 
    break;  /*------------ end UnsolDevReq ----------------------------6*/
 
 
    case AdapterErr:                    /* Adapter error               7*/
                                        /*     log a h/w error         7*/
       gerr_log(dev,NULL,intr_name,
	  FATAL_IO_ERR,AdaptErr,ras_unique[26]);
       PRNTE(("gswintr: cap->vda.ch[0-3] = 0x %x %x %x %x \n",
	   cap->vda.ch[0],cap->vda.ch[1],cap->vda.ch[2],cap->vda.ch[3]));
       CANCEL_TMR(IoTmrMask);
       /*--------------------------------------------------------------7*/
       /* Check for outstanding DMA. if so, call undo_dma.             7*/
       /*--------------------------------------------------------------7*/
       for (i = 0; i < NumDevSupp; i++)
       {
	 if ( ((mgcb_ptr+i)->oflag.gsw_open) &&         /* device open 7*/
	    ((mgcb_ptr+i)->dma_location != NO_DMA_IN_PROG) )
	 {
	    PRNTE(("...dev= %d, dma_location = 0x%x \n",
		 i, (mgcb_ptr+i)->dma_location ));
	    rc = undo_dma((mgcb_ptr+i));
	    if (rc)                             /* failure             7*/
	    {
		gerr_log(i,intr_name,undodma_name,BAD_UNDODMA,
			 rc,ras_unique[32]);
		/***serious_err = TRUE;***/
		break;
	    }
	    break;                      /* only 1 DMA can be outstanding*/
	 }
       }
       serious_err = TRUE;              /* set serious_err flag.       7*/
       /***comm_lock = 0;***/           /* allow ipl_start to cleanup  7*/
       /***ipl_start();***/             /* ipl_adapter                 7*/
    break;  /*------------ end AdapterErr -----------------------------7*/
 
    case LinkSwitch:                    /* Link switch                 1*/
    case Diag:                          /* Diagnose                    2*/
    default:
	  gerr_log(dev,NULL,intr_name,
		   BAD_ICC,cap->icc,ras_unique[27]);
    break;  /*------------ end default ---------------------------------*/

  } /* end "switch (cap->icc)" */
 
 
  /*--------------------------------------------------------------------*/
  /* Reach here if completed processing of the specific interrupt.      */
  /* Do the following -                                                 */
  /*    . free the msla to rt comm area                                 */
  /*    . if during a msla ipl, re-start the 68000                      */
  /*    . if comm area available, clean up and look for pending cmds.   */
  /*--------------------------------------------------------------------*/
 
  cap->ipf = 0;                         /* clear interpt pend flag      */

  if (serious_err)
  {
	PRNTE(("gswintr: serious error code entered. \n"));
        ipl_flag = TRUE;
	for (i = 0; i < NumDevSupp; i++)
	{
	    (mgcb_ptr+i)->iflag.io_allowed   = FALSE;
	    (mgcb_ptr+i)->iflag.not_to_ready = FALSE;
	    (mgcb_ptr+i)->uns_res            = reason;
	}
	cancel_sio(NumDevSupp);         /* cancel all io operations     */
        comm_lock = 0;
	/*********ipl_start();Causes link switching in middle of appl ***/
	BUSMEM_DET(segreg);
	CLR_PARITYJMP(&(lclparbuf))
	TRCHKL0T(hkwd_INTR_R7);
	i_reset(ihp);
	return(0);                      /* exit - can do no more.       */
  }

  if (mf.ipl_in_prog)                   /* if during ipl before prog    */
  {                                     /*      code + 00 rcvd          */
        STRT_MSLA
  }
  if (comm_lock == 0)
  {                                     /* if so, we own the comm area  */
                                        /* and hence no msla cmds active*/
	/*--------------------------------------------------------------*/
        /* If cmd  chaining -                                           */
        /*      . set ae_exp to TRUE  (still expect more until last)    */
        /*      . update ptr to ce  (ce_offset )                        */
        /*      . update ptr to ccw (ccw_offset)                        */
	/*      . issue send_sol                                        */
        /*      . break out                                             */
	/* It is either a GEOP/SMI io operation or a solicited one.     */
        /*--------------------------------------------------------------*/
	if (g->b.cc)
	{
            ccw_offset++;
            ce_offset++;
	    if (mf.intio_in_prog)
		send_sol(ccbp,dev,g->intdp);              /*DMA call    */
	    else
		send_sol(ccbp,dev,g->xm.dp+(g->xm.indx)); /*DMA call    */
        }
 
	/*--------------------------------------------------------------*/
	/* Reach here if the chain of io operations is complete         */
	/* (i.e., we are not command chaining).                         */
	/* If the completed operation was from and outstanding          */
	/* GEOP/SMI io operation, fill in the queue element for         */
	/* processing later on in the interrupt handler                 */
	/*                                                              */
	/* the 'gint_io' call will put the GEOP/SMI extra data in the   */
	/*      g->intp area AFTER leaving room for the qel             */
	/*      header and the sense data from the originating ATTN+UC. */
	/*--------------------------------------------------------------*/
	else if (mf.intio_in_prog)
	{
	    mf.intio_in_prog = FALSE;
	    ret_flag         = FALSE;
	    type             = g->intio_type;
	    data_ptr         = g->intp; /* ptr to saved sense/ RMI data */
	}

	/*--------------------------------------------------------------*/
        /* If rmi pending and not expecting DE -                        */
        /*      . update head index                                     */
	/*      . rmiqtail must be updated BEFORE making the call.      */
	/*        (the logic of head and tail is such that the values   */
	/*        are incremented and then filled in).                  */
	/*      . issue the rmi                                         */
        /*--------------------------------------------------------------*/
	else if ( rmiqhead != rmiqtail)
        {
              gbak = g;
              g = mgcb_ptr +
		  rmiq[((rmiqtail + 1) % (NumDevSupp+1))];
              if (g->c.de_exp == FALSE)
                 {
			rmiqtail = (rmiqtail + 1) % (NumDevSupp+1);
			send_uns(rmiq[rmiqtail]);
                 }
             g = gbak;
        }
 
 
        /*--------------------------------------------------------------*/
        /* Reach here if not CC and not RMI pend and expecting DE       */
        /*--------------------------------------------------------------*/
        else                            /* not CC and not RMI pending   */
        {
            comm_lock = 0;
            if (mf.ipl_req)
		ipl_start();
                                        /* cmd pend & not expect DE     */
 
            /*----------------------------------------------------------*/
            /* If cmd pending and not expecting DE -                    */
            /*      . update head index                                 */
            /*      . issue sio                                         */
	    /* We saved the 'ccbp' and the 'dp' when we were unable     */
	    /* to start the io  operation in 'setup_sio'.               */
            /*----------------------------------------------------------*/
            else if ( pndqhead != pndqtail)
            {
                gbak = g;
                g = mgcb_ptr + pndq[((pndqtail + 1) % (NumDevSupp + 1))];
                if ( g->c.de_exp == FALSE )
                   {
                        pndqtail = (pndqtail + 1) % (NumDevSupp+1);
			rc = setup_sio(g->pndccbp,pndq[pndqtail],g->pnddp);
                        if (rc) { 
	                   PRNTE((
			    "gswintr: FAILURE setup_sio. rc = 0x%x\n", rc));
                       	   rc = EIO;
		        }
			pndq[pndqtail] = (ushort)(-1);  /* added 5/7/91 */
                   }
                g = gbak;
            }
        }
  }

  if (ret_flag)
  {
	BUSMEM_DET(segreg);
	CLR_PARITYJMP(&(lclparbuf))
        TRCHKL1T(hkwd_INTR_R6,reason);
	i_reset(ihp);
	return(INTR_SUCC);
  }
  PRINT(("gswintr: process event for QEL queue \n"));
  ret_flag = TRUE;

  /*--------------------------------------------------------------------*/
  /* Interrupt is processed. If dev not open, leave.                    */
  /*--------------------------------------------------------------------*/
  if (g->oflag.gsw_open == FALSE )
  {
	BUSMEM_DET(segreg);
	CLR_PARITYJMP(&(lclparbuf))
        TRCHKL0T(hkwd_INTR_R8);
	i_reset(ihp);
        return(0);                      /* exit - can do no more.      0*/
  }

  BUSMEM_DET(segreg);
  CLR_PARITYJMP(&(lclparbuf))

  /*--------------------------------------------------------------------*/
  /* at this point, type = type of input device causing interrupt.      */
  /* . if EVENT, send signal if sig enabled.                            */
  /* . if REQUEST, do not send signal but disable request queue         */
  /*--------------------------------------------------------------------*/
 
  /*--------------------------------------------------------------------*/
  /*  note: when either NO_5080 or LINK_SW occur, a single interrupt    */
  /*        is received. it must be put on any device's queue that is   */
  /*        ENABLED via openx. for example, if the 5081 and both ports  */
  /*        have been openx'ed with link_sw set, these types of         */
  /*        interrupts must be handled in the following do loop.        */
  /* If not a link switch or no_5080, a test at the end of the loop     */
  /* provides the mechanism for exiting from loop after just one passs. */
  /*--------------------------------------------------------------------*/
 
  for (i = 0; i < NumDevSupp; i++)      /* needed for LINK_SW/NO_5080   */
  {
    bad_data = 0;
    if ((type == Klink_sw) || (type == Kno_5080) )
    {
        g = mgcb_ptr + i;
        er = FALSE;
        if (!(g->oflag.link_sw))        /* user didnt openx with linksw */
                continue;
    }
    this_hdr = (g->hdr_ptr) + (int)type;

    switch ((int)type)
    {
        case Gsmi :
        case Ggeop :
                tablep = gfnd_qual(this_hdr->intrp,(int)type,qual.wd);
                if (tablep == NULL)
                  {
			TRCHKL2T(hkwd_INTR_R9,(int)type,tablep);
			i_reset(ihp);
        		return(0);     /* exit - can do no more.      0*/
                  }
                break;
        default :
                tablep = this_hdr->intrp;
                break;
    }
    /*------------------------------------------------------------------*/
    /* If tablep == 0, then interrupt type was never enabled. return.   */
    /*------------------------------------------------------------------*/
    if (tablep == 0)
    {
	if ( (type != Klink_sw) && (type != Kno_5080) )
		break;                  /* go down to exit              */
	else
		continue;               /* go thru 'for' loop again     */
    }

    /*------------------------------------------------------------------*/
    /* For GEOP/SMI, the sense/rmi data is in g->intp buffer.           */
    /*------------------------------------------------------------------*/
 
    data_len = dflt_size[(int)type] ;
    data_len += tablep->data_len;

    switch(tablep->flags.mode)
    {
        case GREQUEST :
 
                this_qel                 = (q_qel *)g->req_data;
                tablep->flags.dirty_slot = FALSE;
                tablep->flags.mode       = GDISABLE;
                g->oflag.req_intr        = TRUE;
                break;
 
        case GEVENT :
                                        /*------------------------------*/
                                        /* integer size slots           */
                                        /*------------------------------*/
		slot_siz = sizeof(q_qel)+((data_len + 3) & ZERO_LOW2BITS);
                this_qel = gfnd_qel(dev,slot_siz);
                if (this_qel == NULL) {
                        gerr_log(dev,intr_name,fndqel_name,
				NO_QEL,0,ras_unique[28]);
                        er = TRUE;
                        break;
                }
                g->oflag.req_intr = FALSE;

                if ( (g->signal) && (g->oflag.q_chng_state) ) {
                        if (!(g->oflag.slp_pending) ) {
				SIGAPPL(g->signal);
                        }
                        else g->oflag.sig_pending = TRUE;
                }
                g->oflag.q_chng_state = FALSE;
                break;
        case GDISABLE :
                er = TRUE;
                break;
        default :
                if ((type != Klink_sw) && (type != Kno_5080) )
                    gerr_log(dev,NULL,intr_name,
			      BAD_MODE,0,ras_unique[29]);
                er = TRUE;
                break;
    }
    if ( er == TRUE) continue;

    /*-----------------------------------------------------------------*/
    /* read additional data if specified by data_len                   */
    /*-----------------------------------------------------------------*/
    bzero(this_qel,sizeof(q_qel));
    this_qel->data_len = data_len;
    this_qel->time     = time;
    this_qel->type     = (enum input_type)(BIT7 << (int)type);

    /*-----------------------------------------------------------------*/
    /* Put data ('normal' plus extra) into queue element.              */
    /* 'data_ptr' points to the data ('normal' plus extra)             */
    /*-----------------------------------------------------------------*/

    if (bad_data)
	    this_qel->flags.bad_data = TRUE;
    else {
	    if (data_len)                   /* if data_len non-zero */
		    bcopy(data_ptr,this_qel+1,data_len);
    }
    g->oflag.wokehdr = TRUE;            /* flag for routines sleeping   */
    AWAKE_EVENTQ
					/* if not link_sw or no_5080,   */
					/* break from for  loop and exit*/
    if ( (type != Klink_sw) && (type != Kno_5080) )
        	break;
   } 
   /* end of for loop */
 
  TRCHKL1T(hkwd_INTR_R6,reason);
  i_reset(ihp);
  return(INTR_SUCC);
 
  /*--------------------------*/
} /* end of interrupt routine */
  /*--------------------------*/
 


