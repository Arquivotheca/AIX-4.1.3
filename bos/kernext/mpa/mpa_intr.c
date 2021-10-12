static char sccsid[] = "@(#)84	1.11  src/bos/kernext/mpa/mpa_intr.c, sysxmpa, bos41J, 9520A_a 5/12/95 16:26:14";
/*
 *   COMPONENT_NAME: (SYSXMPA) MP/A SDLC DEVICE DRIVER
 *
 *   FUNCTIONS: dsr_timer
 *		get_rx_result
 *		mpa_intrA
 *		mpa_intrB
 *		mpa_offlvl
 *		q_irpt_results
 *		proc_tx_result
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

#include <sys/adspace.h>
#include <sys/errids.h>
#include <sys/device.h>
#include <sys/dma.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/malloc.h>
#include <sys/mpadd.h>
#include <sys/poll.h>
#include <sys/mbuf.h>
#include <sys/sleep.h>
#include <sys/sysmacros.h>
#include <sys/xmem.h>
#include <sys/trchkid.h>
#include <sys/ddtrace.h>

extern void recv_timer();
extern void ring_timer();
extern void xmit_timer();

/*
 * NAME: mpa_intrA
 *
 * FUNCTION: This procedure is the interrupt handler for level 3 interrupts.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment-Specific aspects, such as -
 *      Preemptable        : No
 *      VMM Critical Region: Yes
 *      Runs on Fixed Stack: Yes
 *      May Page Fault     : Yes
 *      May Backtrack      : Yes
 *
 * NOTES: This procedure checks the level and task registers to verify that
 *        this is our interrupt and processes it
 *
 * DATA STRUCTURES: external struct acb
 *
 * RETURN VALUE DESCRIPTION: Either INTR_SUCC (interrupt handled successfully)
 *                           or INTR_FAIL (interrput was not ours)
 */

int mpa_intrA ( struct intr *ih_structA )
{
   struct acb	   *acb;
   int             statreg=0;
   int             rc,cntr=0,result,count,i;
   uchar           wait_code=0;
   ulong           pos;
   int             byte_xmit = 0;
   int             byte_recv = 0;
   uchar           byte_sent;
   uchar           byte_read;


   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
   ³ the addr of the acb is the same ³
   ³ as ih_structA. Note: this struct³
   ³ must remain at the top of acb   ³
   ³ for this to work (see mpadd.h)  ³
   ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   acb = (struct acb *)ih_structA;

   DDHKWD5 (HKWD_DD_MPADD, DD_ENTRY_INTR,0,acb->dev,0,0,0,0);
   MPATRACE4("IhAE",acb->dev,acb,acb->ih_structA.level);

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
   ³ check the status reg for this interrupts card. ³
   ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   while ( ++cntr < 2) 
   {
      if( (statreg = PIO_GETC(acb, RD_STAT_OFFSET) ) == -1) 
      {
	 /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	 ³ check the pos2 settings to make sure they are correct  ³
	 ³ there seems to be a problem with setting pos regs, the ³
	 ³ value is not always stored as it should be, so if this ³
	 ³ pio fails retry setting the pos reg and try the pio    ³
	 ³ again.                                                 ³
	 ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

	 pos = MPA_IOCC_ATT;

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ try to put pos reg back right ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	 BUS_PUTC( pos + POS2, acb->pos2 );

	 IOCC_DET(pos);
      }
      else break;
   }
   if(cntr==2) 
   {
	 /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	 ³ If it gets to here you are hung anyway, so stop the machine. ³
	 ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	 MPATRACE2("IhA1",acb->dev);
         mpalog(acb,ERRID_MPA_ADPERR , __LINE__,__FILE__,0,0);
         shutdown_adapter(acb);
	 free_active_q(acb);
   	 async_status(acb, CIO_ASYNC_STATUS, MP_ADAP_NOT_FUNC, 0, 0, 0);
         return(INTR_FAIL);
   }

   cntr = 0;

   MPATRACE3("IhA2",acb->dev,statreg);

   if ( statreg&IRPT_PENDING ) 
   {
      /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
      ³ There are two possible causes for the irpt            ³
      ³  1. Receive needs service.                            ³
      ³     - results from previous receive available         ³
      ³  2. Transmit needs service                            ³
      ³     - results from previous transmit available        ³
      ³                                                       ³
      ³    I will read the results and store them in struct   ³
      ³  then attach the struct to a chain that is processed  ³
      ³  by the off level handler. This will free up the      ³
      ³  card and prevent irpt overruns and still get me out  ³
      ³  of this irpt handler in acceptable time.             ³
      ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

      ++acb->stats.ds.total_intr;   /* increment int. count */

      /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
      ³ Check recv irpt bit first every time, if its active,        ³
      ³ handle the recv irpt and wait for the recv irpt bit to      ³
      ³ drop on the card before exiting irpt handler. If there is   ³
      ³ no recv irpt check for xmit irpt and handle it. If there    ³
      ³ are no irpts log error and exit fail.                       ³
      ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
      if(statreg & (wait_code=RX_IRPT_ACTIVE) ) 
      {
	  ++acb->stats.ds.recv_intr_cnt;

	  if( (statreg & RX_RESULT_READY) ) 
	  {
	      rc=q_irpt_results(acb,RECV_RESULT);
	  }
	  else 
	  {
	      MPATRACE3("IhA3",acb->dev,statreg);
	      ++acb->stats.ds.recv_irpt_error;
	  }
      }
      if(statreg & (wait_code=TX_IRPT_ACTIVE) ) 
      {
      /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
      ³ If xmit irpt is active and there are no results ready , ³
      ³ this is an error. Log it and continue on to check the   ³
      ³ recvp irpt.                                             ³
      ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	  wait_code=TX_IRPT_ACTIVE;
	  ++acb->stats.ds.xmit_intr_cnt;

	  if( (statreg & TX_RESULT_READY) ) 
	  {
	      rc=q_irpt_results(acb,XMIT_RESULT);
	  }
	  else 
	  {
	      MPATRACE3("IhA4",acb->dev,statreg);
	      ++acb->stats.ds.xmit_irpt_error;
	  }
      }

      /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
      ³ It seems the MPA hardware is a bit slow in dropping the ³
      ³ irpt and status lines, therefore I am putting in a loop ³
      ³ to wait for all lines to drop before I clear this irpt. ³
      ³ This should insure that I don't get invalid interrupts. ³
      ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

      statreg = 0x0F;
      while( (statreg&wait_code) && (++cntr<LOOP_CNTR) ) 
      {
	  if( (statreg=PIO_GETC( acb, RD_STAT_OFFSET )) == -1)  
	  {
	 	MPATRACE3("IhA5",acb->dev,rc);
                mpalog(acb,ERRID_MPA_ADPERR , __LINE__,__FILE__,0,0);
                shutdown_adapter(acb);
	 	free_active_q(acb);
   	 	async_status(acb, CIO_ASYNC_STATUS, MP_ADAP_NOT_FUNC, 0, 0, 0);
		i_reset(&acb->ih_structA);
                return(INTR_SUCC);
	  }
	  /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	  ³  If the count exceeds 2 an iprt is hung so try to ³
	  ³  reset it here. There is a problem with the 8273  ³
	  ³  that can cause the xmit irpt to hang so read the ³
	  ³  TX result again.                                 ³
	  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	  if ( (cntr==2) && (wait_code==TX_IRPT_ACTIVE) ) 
	  {
	      if( (result=PIO_GETC( acb, RD_TX_IR_OFFSET)) == -1 ) 
	      {
	 	MPATRACE3("IhA6",acb->dev,rc);
		mpalog(acb,ERRID_MPA_ADPERR , __LINE__,__FILE__,0,0);
                shutdown_adapter(acb);
	 	free_active_q(acb);
   	 	async_status(acb, CIO_ASYNC_STATUS, MP_ADAP_NOT_FUNC, 0, 0, 0);
		i_reset(&acb->ih_structA);
                return(INTR_SUCC);
	      }
	      MPATRACE3("IhA7",acb->dev,result);
	  }
	  else 
	  /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	  ³ When the irpt is a receive irpt there is no need  ³
	  ³ to wait as long. It could be a valid receive irpt ³
	  ³ waiting to be taken.                              ³
	  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
		if ( (cntr==4) && (wait_code==RX_IRPT_ACTIVE) ) break;
      }
      if(cntr==LOOP_CNTR) 
      {
	   MPATRACE4("IhA8",acb->dev,rc,wait_code);
	   mpalog(acb,ERRID_MPA_ADPERR , __LINE__,__FILE__,0,0);
           shutdown_adapter(acb);
	   free_active_q(acb);
   	   async_status(acb, CIO_ASYNC_STATUS, MP_ADAP_NOT_FUNC, 0, 0, 0);
	   i_reset(&acb->ih_structA);
           return(INTR_SUCC);
      }

      if(acb->flags & IRPT_QUEUED) 
      {
	   MPATRACE4("IhA9",acb->dev,&(acb->ofl.offl_intr),rc);
	  /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	  ³ schedule an offlevel to handle the queued irpt and ³
	  ³ reset the IRPT_QUEUED flag.                        ³
	  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	  i_sched((struct intr *)&(acb->ofl.offl_intr));
	  acb->flags &= ~IRPT_QUEUED;

      }
      else
      {
	   MPATRACE4("IhAa",acb->dev,acb,rc);
	  if( (rc == XMIT_COMPLETE) &&
		(acb->flags & END_XMIT) )
	  {
		acb->flags &= ~END_XMIT;
#ifndef NO_STRM_FLG_CTRL
       		/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
       		³ Turn off flag stream mode if on (this was ³
       		³ modified for defect #091439)              ³
       		ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
       		if( acb->state.oper_mode_8273 & SET_FLAG_STREAM )
       		{
               		bzero(&acb->cmd_parms,sizeof(cmd_phase_t));
               		acb->cmd_parms.cmd=RESET_OPER_MODE_CMD;
               		acb->cmd_parms.parm[0]=RESET_FLAG_STREAM;
               		acb->state.oper_mode_8273 &= RESET_FLAG_STREAM;
               		acb->cmd_parms.parm_count=1;
               		if( (rc=que_command(acb)) )
               		{
                		MPATRACE3("IhAb",acb->dev,rc);
                		mpalog(acb,ERRID_MPA_ADPERR , __LINE__,__FILE__,0,0);
                		shutdown_adapter(acb);
                		free_active_q(acb);
                		async_status(acb, CIO_ASYNC_STATUS, MP_ADAP_NOT_FUNC, 0, 0, 0);
                		i_reset(&acb->ih_structA);
                		return(INTR_SUCC);
               		}
       		}
#endif
#ifndef NO_RTS_CTRL
                /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
                ³ drop RTS if on and not operating ³
                ³ in continuous carrier mode       ³
                ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
                if( (acb->state.port_b_8273 & SET_RTS) &&
                        !(acb->strt_blk.data_flags & DATA_FLG_C_CARR_ON) )
                {
                        bzero(&acb->cmd_parms,sizeof(cmd_phase_t));
                        acb->cmd_parms.cmd=RESET_8273_PORT_B_CMD;
                        acb->cmd_parms.parm[0]=RESET_8273_PORT_B_RTS;
                        acb->state.port_b_8273 &= RESET_8273_PORT_B_RTS;
                        acb->cmd_parms.parm_count=1;
                        if( ( rc=que_command(acb)) )
   			{
         			MPATRACE3("IhAc",acb->dev,rc);
                		mpalog(acb,ERRID_MPA_ADPERR , __LINE__,__FILE__,0,0);
                		shutdown_adapter(acb);
                		free_active_q(acb);
                		async_status(acb, CIO_ASYNC_STATUS, MP_ADAP_NOT_FUNC, 0, 0, 0);
                		i_reset(&acb->ih_structA);
                		return(INTR_SUCC);
                       }
		}
#endif
	   }
	   else
	   {
          	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
          	³ Restart the receive ³
          	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
          	if( (startrecv(acb)) )
          	{
                	MPATRACE3("IhAd",acb->dev,rc);
                	mpalog(acb,ERRID_MPA_ADPERR , __LINE__,__FILE__,0,0);
                	shutdown_adapter(acb);
                	free_active_q(acb);
                	async_status(acb, CIO_ASYNC_STATUS, MP_ADAP_NOT_FUNC, 0, 0, 0);
                	i_reset(&acb->ih_structA);
                	return(INTR_SUCC);
          	}
	  	if ( (acb->strt_blk.rcv_timeout != 0) &&
                	(!(acb->flags & RCV_TIMER_ON)) )
          	{
			acb->flags |= RCV_TIMER_ON;
                	timeout(recv_timer, (caddr_t)acb, (HZ/10) * 
				(int)acb->strt_blk.rcv_timeout );
	  	}
	   }
      }

      /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
      ³ reset with irpt struct of card just serviced ³
      ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
      i_reset(&acb->ih_structA);
      ++acb->stats.ds.irpt_succ;

      MPATRACE4("IhAX",acb->dev,statreg,rc);
      DDHKWD5 (HKWD_DD_MPADD, DD_EXIT_INTR, 0,acb->dev,0,0,0,0);

      return (INTR_SUCC);
   }

   MPATRACE3("IhAF",acb->dev,statreg);
   return ( INTR_FAIL ); /* not our interrupt, tell the FLIH */

}            /* mpa_intrA() */


/*
 * NAME: mpa_intrB
 *
 * FUNCTION: This procedure is the interrupt handler for level 4 interrupts.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment-Specific aspects, such as -
 *      Preemptable        : No
 *      VMM Critical Region: Yes
 *      Runs on Fixed Stack: Yes
 *      May Page Fault     : Yes
 *      May Backtrack      : Yes
 *
 * NOTES: This procedure checks the level and task registers to verify that
 *        this is our interrupt and processes it
 *
 * DATA STRUCTURES: external struct acb
 *
 * RETURN VALUE DESCRIPTION: Either INTR_SUCC (interrupt handled successfully)
 *                           or INTR_FAIL (interrput was not ours)
 */

int mpa_intrB ( struct intr *ih_structB )
{
   struct acb	  *acb;
   void 	  dsr_timer();
   int            cntr=0,modemstat,rc=0;
   int		  turn_on_mdm_chg=0; 
   int	  	  result=0,statreg=0;

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ get acb by decrementing addr ³
	³ Note: for this to work, the  ³
	³ ih_structB must remain 2nd in³
	³ the acb struct (see mpadd.h) ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	acb = (struct acb *)(--ih_structB);  

   	DDHKWD5 (HKWD_DD_MPADD, DD_ENTRY_INTR,0,acb->dev,0,0,0,0);
        MPATRACE3("IhBE",acb->dev,acb);

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ this interrupt handler is called when ³
	³ there are changes in modem signals, so³
	³ read the modem input port on the 8255 ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
        if( (modemstat = PIO_GETC( acb, PORT_A_8255)) == -1) 
	{
        	MPATRACE2("IhB1",acb->dev);
		if( acb->flags & STARTED_CIO ) /* ...already started */
		{
   	   		async_status(acb, CIO_ASYNC_STATUS, MP_ADAP_NOT_FUNC, 0, 0, 0);
		}
		else
		{ /* Fail the start */
			
   	   		async_status(acb, CIO_START_DONE, MP_ADAP_NOT_FUNC, 0, 0, 0);
		}
                mpalog(acb,ERRID_MPA_ADPERR , __LINE__,__FILE__,0,0);
		shutdown_adapter(acb);
	   	free_active_q(acb);
   		i_reset(&acb->ih_structB); 
                return(INTR_SUCC);
        }

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
        ³ Check modem status in 8255 Port A to see if it is ON ³
        ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	if (modemstat & MODEM_STAT_CHG) /* If on, this is our interrupt */
	{
		/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		³ read the 8273 PORT A register ³
		ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
        	bzero(&acb->cmd_parms,sizeof(cmd_phase_t)); 
        	acb->cmd_parms.cmd = READ_8273_PORT_A_CMD;
        	acb->cmd_parms.parm_count = 0;
        	acb->cmd_parms.flag = RETURN_RESULT;

        	if( (rc=que_command(acb)) )
        	{
        		MPATRACE4("IhB2",acb->dev,acb,rc);
			if( acb->flags & STARTED_CIO ) /* ...already started */
			{
   	   			async_status(acb, CIO_ASYNC_STATUS, MP_ADAP_NOT_FUNC, 0, 0, 0);
			}
			else
			{ /* Fail the start */
				
   	   			async_status(acb, CIO_START_DONE, MP_ADAP_NOT_FUNC, 0, 0, 0);
			}
		 	shutdown_adapter(acb);
	   		free_active_q(acb);
   			i_reset(&acb->ih_structB); 
			return(INTR_SUCC);
        	}

		/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		³ preserve the state of the 8273 PORT A in the acb ³
		ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
		acb->state.port_a_8273=result=acb->cmd_parms.result;

        	MPATRACE4("IhB3",acb->dev,acb->state.port_a_8273,modemstat); 
		/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		³ check for a change in the state of DSR ³
		³ - DSR dropped after line was active    ³
		³ - DSR just came on                     ³
		ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
		if( ((acb->flags & STARTED_CIO) &&  /* ...already started */
			(!(result & PORT_A_8273_PA2))) || /* and DSR down */
			((acb->flags & WAIT_FOR_DSR) && /* or waitin' for DSR */
			(result & PORT_A_8273_PA2)) )   /* and DSR is up */
		{
        		MPATRACE4("IhB4",acb->dev,acb,result);
			/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
			³ call timer to see if the state of DSR has ³
			³ really changed or if line is jittering    ³
			ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
			/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
			³ NOTE: this and other timeout calls in the ³
			³       driver seem to be compiler sensitive³
			³       so be aware that a problem may occur³
			³       with changes to the compiler        ³
			ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
			timeout( dsr_timer, (caddr_t)acb, HZ/20 );
		} 
		else
		{
			/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	 		³ otherwise, this is just a change in CTS, so... ³
			³ indicate that modem change bit should be turned³
			³ back on after resetting it.                    ³
			ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
			turn_on_mdm_chg = 1; 
		}

		/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		³ Reset the modem changed logic bits thru the 8255 ³
		³ Port B.  This will also reset the interrupt.	   ³
 		ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
		acb->state.port_b_8255 &= ~FREE_STAT_CHG;
        	MPATRACE4("IhB5",acb->dev,acb,acb->state.port_b_8255);
		if(PIO_PUTC( acb, PORT_B_8255, acb->state.port_b_8255)==-1) 
		{
        		MPATRACE2("IhB6",acb->dev);
			if( acb->flags & STARTED_CIO ) /* ...already started */
			{
   	   			async_status(acb, CIO_ASYNC_STATUS, MP_ADAP_NOT_FUNC, 0, 0, 0);
			}
			else
			{ /* Fail the start */
				
   	   			async_status(acb, CIO_START_DONE, MP_ADAP_NOT_FUNC, 0, 0, 0);
			}
                	mpalog(acb,ERRID_MPA_ADPERR , __LINE__,__FILE__,0,0);
			shutdown_adapter(acb);
	   		free_active_q(acb);
   			i_reset(&acb->ih_structB); 
                	return(INTR_SUCC);
        	}

		/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		³ Check modem status in 8255 Port A ³
		³ to see if it was reset            ³
		ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
		if( (modemstat = PIO_GETC( acb, PORT_A_8255)) == -1) 
		{
        		MPATRACE2("IhB7",acb->dev);
			if( acb->flags & STARTED_CIO ) /* ...already started */
			{
   	   			async_status(acb, CIO_ASYNC_STATUS, MP_ADAP_NOT_FUNC, 0, 0, 0);
			}
			else
			{ /* Fail the start */
				
   	   			async_status(acb, CIO_START_DONE, MP_ADAP_NOT_FUNC, 0, 0, 0);
			}
               		mpalog(acb,ERRID_MPA_ADPERR , __LINE__,__FILE__,0,0);
			shutdown_adapter(acb);
	   		free_active_q(acb);
   			i_reset(&acb->ih_structB); 
               		return(INTR_SUCC);
        	}

		if (modemstat & MODEM_STAT_CHG) /* if still ON */
		{   /* Failed to reset */
        		MPATRACE2("IhB8",acb->dev);
			if( acb->flags & STARTED_CIO ) /* ...already started */
			{
   	   			async_status(acb, CIO_ASYNC_STATUS, MP_ADAP_NOT_FUNC, 0, 0, 0);
			}
			else
			{ /* Fail the start */
				
   	   			async_status(acb, CIO_START_DONE, MP_ADAP_NOT_FUNC, 0, 0, 0);
			}
                        mpalog(acb,ERRID_MPA_ADPERR , __LINE__,__FILE__,0,0);
			shutdown_adapter(acb);
	   		free_active_q(acb);

   			i_reset(&acb->ih_structB); 
                        return(INTR_SUCC);
		}
		
		if( turn_on_mdm_chg )
		{
			/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
        		³ Enable the modem status change bit ³
        		³ so modem changes can cause irpts   ³
        		ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
        		acb->state.port_b_8255 |= FREE_STAT_CHG;
        		MPATRACE4("IhB9",acb->dev,acb->state.port_b_8255,modemstat);
        		if(PIO_PUTC( acb, PORT_B_8255, acb->state.port_b_8255)==-1)
        		{
        			MPATRACE2("IhBa",acb->dev);
				if( acb->flags & STARTED_CIO ) /* ...already started */
				{
   	   				async_status(acb, CIO_ASYNC_STATUS, MP_ADAP_NOT_FUNC, 0, 0, 0);
				}
				else
				{ /* Fail the start */
					
   	   				async_status(acb, CIO_START_DONE, MP_ADAP_NOT_FUNC, 0, 0, 0);
				}
               			mpalog(acb,ERRID_MPA_ADPERR , __LINE__,__FILE__,0,0);
                       		shutdown_adapter(acb);
                       		free_active_q(acb);
        		}
		}

   		i_reset(&acb->ih_structB); 
   		MPATRACE4("IhBX",acb->dev,modemstat,result);
		DDHKWD5 (HKWD_DD_MPADD, DD_EXIT_INTR, 0,acb->dev,0,0,0,0);
   		return ( INTR_SUCC );  /* interrupt serviced */
	}
   	MPATRACE3("IhBF",acb->dev,modemstat);
   	return ( INTR_FAIL );  /* not out interrupt, tell the FLIH */
}            /* mpa_intrB() */

/*
 * NAME: mpa_offlvl
 *
 * FUNCTION: High level description of what the procedure does
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment-Specific aspects, such as -
 *      Preemptable        : Maybe
 *      VMM Critical Region: Yes
 *      Runs on Fixed Stack: Yes
 *      May Page Fault     : no
 *      May Backtrack      : Yes
 *
 * NOTES: More detailed description of the function, down to
 *          what bits / data structures, etc it manipulates.
 *
 * DATA STRUCTURES: Effects on global data structures, similar to NOTES.
 *
 * RETURN VALUE DESCRIPTION: NONE
 */

int mpa_offlvl (t_offl_intr  *ofl_ptr)
{
    struct acb              *acb;           /* interrupting adapter's acb */
    int                     rc=0;
    irpt_elem_t             *irptp;
    dma_elem_t              *dmap;
    recv_elem_t             *recvp;
    xmit_elem_t             *xmitp;
    struct mbuf             *mbufp;
    caddr_t		    data_addr;      /* address of data in mbuf */
    cio_read_ext_t          rd_ext;
    uchar                   result;
    int                     recv_bytes = 0;
    void		    proc_tx_result();

    /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ Get the pointer to the acb from the offlevel ³
    ³ structure which was initialized at open time ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
    acb = (struct acb *)ofl_ptr->p_acb_intr;        

    MPATRACE4("OfLE",acb->dev,acb,&(ofl_ptr->offl_intr));

    /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ Process all elements on the active irpt q ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

    while( (irptp=acb->act_irpt_head) != NULL) 
    {
        MPATRACE4("OfL1",acb->dev,irptp,irptp->type_flag);
	switch(irptp->type_flag) 
        {

	   case RECV_RESULT:

		/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		³ I need to & out bits in last xfer part of the RIC ³
		³ and 0 the flags used. 			    ³
		ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
		result = (irptp->tp.rcv.RIC & RIC_MASK_LAST_BYTE);

                MPATRACE4("OfL2",acb->dev,irptp->type_flag,result);

		switch(result) 
		{

		     case RECV_GEN_OK:
		     case RECV_SEL_OK:

			  if ( acb->strt_blk.rcv_timeout != 0 )
    			  {
        			untimeout( recv_timer, (caddr_t)acb );
				acb->flags &= ~RCV_TIMER_ON;
    		       	  }
			  if( (recvp=acb->act_recv_head) == NULL ) 
			  {
			       ++acb->stats.ds.recv_not_handled;
			       acb->flags &= ~RECV_DMA_ON_Q;
                               MPATRACE3("Ofe1",acb->dev,result);
			       break;
			  }
			  if( (rc=startrecv(acb))) 
			  {
    			     MPATRACE3("Ofe2",acb->dev,rc);
			     break;
			  }
			  if ( (acb->strt_blk.rcv_timeout != 0) &&
                		(!(acb->flags & RCV_TIMER_ON)) )
        		  {
			  	acb->flags |= RCV_TIMER_ON;
                          	timeout(recv_timer, (caddr_t)acb, (HZ/10) * 
					(int)acb->strt_blk.rcv_timeout );
			  }
			  /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
			  ³ Set the flags to save the count and pass the data  ³
			  ³ received up the caller after it is known that the  ³
			  ³ receive pointer is valid (this change was made for ³
			  ³ defect #091410).                                   ³
			  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
			  acb->flags |= RC_SAVE_CNT;
			  acb->flags |= RC_TAP_USER;
			  ++acb->stats.ds.recv_completes;
			  break;
		     /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		     ³ following is the set of recv results that do ³
		     ³ cause the recv data to be lost.              ³
		     ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
		     case RECV_CRC_ERR:
			  ++acb->stats.ds.recv_crc_errors;
			  /* acb->flags |= RC_GET_PTRS; (a175612) */
		     	 /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		         ³ if the abort threshold has been hit, log an ³
		         ³ error otherwise just setup to receive again.³
		         ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
			  if( acb->stats.ds.recv_crc_errors >= RX_ABORT_THRESHOLD )
			  {
				acb->stats.ds.recv_crc_errors = 0;
			  	mpalog(acb,ERRID_MPA_RCVERR , __LINE__,__FILE__,MP_FRAME_CRC,0);
			  }
			  break;
		     case RECV_ABORTED:
			  ++acb->stats.ds.recv_aborts;
			  /* acb->flags |= RC_GET_PTRS; (a175612) */
		     	 /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		         ³ if the abort threshold has been hit, log an ³
		         ³ error otherwise just setup to receive again.³
		         ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
			  if( acb->stats.ds.recv_aborts >= RX_ABORT_THRESHOLD )
			  {
				acb->stats.ds.recv_aborts = 0;
			  	mpalog(acb,ERRID_MPA_RCVERR , __LINE__,__FILE__,MP_SDLC_ABORT,0);
			  }
			  break;
		     case RECV_BAD_FRAME:
			  ++acb->stats.ds.recv_frame_to_small;
			  /* acb->flags |= RC_GET_PTRS; (a175612) */
		     	 /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		         ³ if the abort threshold has been hit, log an ³
		         ³ error otherwise just setup to receive again.³
		         ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
			  if( acb->stats.ds.recv_frame_to_small >= RX_ABORT_THRESHOLD )
			  {
				acb->stats.ds.recv_frame_to_small = 0;
			  	mpalog(acb,ERRID_MPA_RCVERR , __LINE__,__FILE__,MP_RX_FRAME_ERR,0);
			  }
			  break;

			  /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
			  ³ The receiver is disabled  ³
			  ³ after the following.      ³
			  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

		     case RECV_DMA_OVERRUN:
			  ++acb->stats.ds.recv_dma_overruns;
			  acb->flags |= RC_GET_PTRS;
			  mpalog(acb,ERRID_MPA_RCVOVR , __LINE__,__FILE__,0,0);
			  break;
		     case RECV_MEM_OVERFLOW:
			  ++acb->stats.ds.recv_buf_overflow;
			  acb->flags |= RC_GET_PTRS;
			  mpalog(acb,ERRID_MPA_RCVOVR , __LINE__,__FILE__,0,0);
			  break;
		     case RECV_CARRIER_DOWN:
			  ++acb->stats.ds.recv_cd_failure;
			  acb->flags |= RC_GET_PTRS;
			  mpalog(acb,ERRID_MPA_RCVERR , __LINE__,__FILE__,MP_RX_ABORT,0);
			  break;
		     case RECV_PIO_ERROR:
			  ++acb->stats.ds.io_irpt_error;
			  acb->flags |= RC_GET_PTRS;
			  break;
		     /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		     ³ following is the set of recv results that do not ³
		     ³ cause the recv data to be lost. But the receiver ³
		     ³ is disabled and must be restarted.               ³
		     ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
		     case RECV_IDLE:
                          MPATRACE4("OfL3",acb->dev,result,acb->flags);
                          ++acb->stats.ds.recv_idle_detects;
                          /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
                          ³ If currently in RECEIVE mode, restart ³
                          ³ the receiver on the adapter.          ³
                          ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
                          if( acb->flags & RECEIVER_ENABLED )
                          {
                                acb->total_bytes = 0;
                                if(acb->station_type & GENERAL)
                                {
                                   acb->cmd_parms.cmd=GEN_RECEIVE_CMD;
                                   acb->cmd_parms.parm[0]=MAX_RECV_SIZE;
                                   acb->cmd_parms.parm[1]=(MAX_RECV_SIZE>>8);
                                   acb->cmd_parms.parm_count=2;
                                }
                                else
                                {
                                   acb->cmd_parms.cmd=SEL_RECEIVE_CMD;
                                   acb->cmd_parms.parm[0]=MAX_RECV_SIZE;
                                   acb->cmd_parms.parm[1]=(MAX_RECV_SIZE>>8);
                                   acb->cmd_parms.parm[2]=acb->station_addr;
                                   acb->cmd_parms.parm[3]=(uchar)BROADCAST_ADDR;
                                   acb->cmd_parms.parm_count=4;
                                }
                                if ( (rc=que_command(acb)) )
                                {
                                   MPATRACE4("Ofe3",acb->dev,result,rc);
                                }
                          }
                          break;
		     case RECV_EOP:
                          MPATRACE4("OfL4",acb->dev,result,acb->flags);
			  ++acb->stats.ds.recv_eop_detects;
                          /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
                          ³ If currently in RECEIVE mode, restart ³
                          ³ the receiver on the adapter.          ³
                          ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
                          if( acb->flags & RECEIVER_ENABLED )
                          {
                                acb->total_bytes = 0;
                                if(acb->station_type & GENERAL)
                                {
                                   acb->cmd_parms.cmd=GEN_RECEIVE_CMD;
                                   acb->cmd_parms.parm[0]=MAX_RECV_SIZE;
                                   acb->cmd_parms.parm[1]=(MAX_RECV_SIZE>>8);
                                   acb->cmd_parms.parm_count=2;
                                }
                                else
                                {
                                   acb->cmd_parms.cmd=SEL_RECEIVE_CMD;
                                   acb->cmd_parms.parm[0]=MAX_RECV_SIZE;
                                   acb->cmd_parms.parm[1]=(MAX_RECV_SIZE>>8);
                                   acb->cmd_parms.parm[2]=acb->station_addr;
                                   acb->cmd_parms.parm[3]=(uchar)BROADCAST_ADDR;
                                   acb->cmd_parms.parm_count=4;
                                }
                                if ( (rc=que_command(acb)) )
                                {
                                   MPATRACE4("Ofe4",acb->dev,result,rc);
                                }
                          }
                          break;
		     case RECV_IRPT_OVERRUN:
                          MPATRACE4("OfL5",acb->dev,result,acb->flags);
			  ++acb->stats.ds.recv_irpt_overruns;
                          /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
                          ³ If currently in RECEIVE mode, restart ³
                          ³ the receiver on the adapter.          ³
                          ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
                          if( acb->flags & RECEIVER_ENABLED )
                          {
                                acb->total_bytes = 0;
                                if(acb->station_type & GENERAL)
                                {
                                   acb->cmd_parms.cmd=GEN_RECEIVE_CMD;
                                   acb->cmd_parms.parm[0]=MAX_RECV_SIZE;
                                   acb->cmd_parms.parm[1]=(MAX_RECV_SIZE>>8);
                                   acb->cmd_parms.parm_count=2;
                                }
                                else
                                {
                                   acb->cmd_parms.cmd=SEL_RECEIVE_CMD;
                                   acb->cmd_parms.parm[0]=MAX_RECV_SIZE;
                                   acb->cmd_parms.parm[1]=(MAX_RECV_SIZE>>8);
                                   acb->cmd_parms.parm[2]=acb->station_addr;
                                   acb->cmd_parms.parm[3]=(uchar)BROADCAST_ADDR;
                                   acb->cmd_parms.parm_count=4;
                                }
                                if ( (rc=que_command(acb)) )
                                {
                                   MPATRACE4("Ofe5",acb->dev,result,rc);
                                }
                          }
                          break;

		     default:
			  mpalog(acb,ERRID_MPA_RCVERR , __LINE__,__FILE__,irptp->tp.rcv.RIC,0);
			  acb->flags |= RC_GET_PTRS;
			  break;
		}   /* end of switch based on result */
		if(acb->flags&RC_GET_PTRS) 
		{
		      	acb->flags &= ~RC_GET_PTRS;
		      	if( (dmap=acb->act_dma_head) == NULL ) 
		      	{
			    ++acb->stats.ds.recv_not_handled;
			    acb->flags &= ~RECV_DMA_ON_Q;
                            MPATRACE4("Ofe6",acb->dev,irptp->type_flag,acb->flags);
			    break;
			}
			if( dmap->dm_req_type!=DM_RECV ) 
		        {
			     ++acb->stats.ds.recv_not_handled;
			     acb->flags &= ~RECV_DMA_ON_Q;
                             MPATRACE4("Ofe7",acb->dev,irptp->type_flag,dmap->dm_req_type);
			     break;
			} 
			if( (recvp = dmap->p.recv_ptr) == NULL)  
		        {
			     ++acb->stats.ds.recv_not_handled;
			     acb->flags &= ~RECV_DMA_ON_Q;
                             MPATRACE3("Ofe8",acb->dev,irptp->type_flag);
			     break;
			}
			acb->flags |= RC_FREE_DMA;
			acb->flags |= RC_FREE_RECV;
			acb->flags |= RC_START_RECV;
		        recv_bytes = recvp->rc_count;
		}
		if(acb->flags&RC_SAVE_CNT) 
		{
		       acb->flags &= ~RC_SAVE_CNT;
		       if(irptp->tp.rcv.R1==0xFF) 
		       {
			   mpalog(acb,ERRID_MPA_RCVERR , __LINE__,__FILE__,0xff,0);
                           MPATRACE4("Ofe9",acb->dev,irptp->type_flag,recvp->rc_state);
			   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
			   ³ Check to see if this MBUF should be freed ³
			   ³ (this was added for defect #091440).      ³
			   ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
			   if(recvp->rc_state & RC_MBUF)
			   {
				m_freem((struct mbuf *)recvp->rc_mbuf);
				recvp->rc_state &= ~RC_MBUF;
			   }
			   free_recv_elem(acb, recvp);

			   ++acb->stats.ds.recv_lost_data;
			   break;
		       }
		       /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		       ³  Set the actual length of the recv. ³
		       ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
		       recvp->rc_count = irptp->tp.rcv.R1;
		       recvp->rc_count = (recvp->rc_count<<8)|irptp->tp.rcv.R0;
		       recv_bytes = recvp->rc_count;
		}
		if(acb->flags&RC_FREE_DMA) 
		{
		       acb->flags &= ~RC_FREE_DMA;
		       free_dma_elem(acb,dmap);
		}
		if(acb->flags&RC_FREE_RECV) 
		{
		       acb->flags &= ~RC_FREE_RECV;
		       free_recv_elem(acb,recvp);
		}
		if(acb->flags&RC_START_RECV) 
		{
		    acb->flags &= ~RC_START_RECV;
		    if(acb->flags & RECEIVER_ENABLED)
		    {
		       acb->flags &= ~RECEIVER_ENABLED;
		       if( (rc=startrecv(acb)) )
		       {
    		       	   	MPATRACE3("Ofea",acb->dev,rc);
		       }
		       if ( (acb->strt_blk.rcv_timeout != 0) &&
                		(!(acb->flags & RCV_TIMER_ON)) )
        	       {
			  	acb->flags |= RCV_TIMER_ON;
                          	timeout(recv_timer, (caddr_t)acb, (HZ/10) * 
					(int)acb->strt_blk.rcv_timeout );
		       }
		    }
		}
		if(acb->flags&RC_TAP_USER) 
		{
		       acb->flags &= ~RC_TAP_USER;
		       recvp->rc_state |= RC_COMPLETE;
		       /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		       ³ Update statistics counters. ³
		       ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
		       if (++acb->stats.cc.rx_frame_lcnt == 0) 
		       {
			       acb->stats.cc.rx_frame_mcnt++;
		       }
		       if ((acb->stats.cc.rx_byte_lcnt += recvp->rc_count) <
			       recvp->rc_count) 
 		       {
			       acb->stats.cc.rx_byte_mcnt++;
		       }
		       /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		       ³ Fill in the read extension fields. ³
		       ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
		       rd_ext.status = CIO_OK;

		       /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		       ³ Get data from recv elem ³
		       ³ set the packet length   ³
		       ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
		       mbufp = recvp->rc_mbuf;
		       mbufp->m_len = recvp->rc_count;

                       MPATRACE4("OfL6",acb->dev,OPENP.mpa_kopen.rx_fn,mbufp);

		       /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		       ³ Get address of recv data. ³
		       ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
		       data_addr=MTOD(mbufp,caddr_t);

		       DDHKWD3(HKWD_DD_MPADD,MPA_RECV_DATA,0,acb->dev,data_addr[0],data_addr[1]);


		       /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		       ³ Notify the kernel user of data received. ³
		       ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
		       (*(OPENP.mpa_kopen.rx_fn))(OPENP.mpa_kopen.open_id,&(rd_ext),mbufp);
                       free_recv_elem(acb,recvp);

		       ++acb->stats.ds.recv_sent;
		}

		break;

	   case XMIT_RESULT:
		result = irptp->tp.TIC;

                MPATRACE3("OfL7",acb->dev,result);

		switch(result) 
		{

                   case XMIT_EARLY_IRPT:
                        MPATRACE4("OfL8",acb->dev,irptp->type_flag,result);
                        ++acb->stats.ds.xmit_early_irpts;
                        acb->flags &= ~WAIT_FOR_EARLY_XMIT;
                        proc_tx_result(acb);
                        break;
                   case XMIT_COMPLETE:
                        MPATRACE3("OfL9",acb->dev,acb->flags);
                       	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
                       	³ If transmit complete was received before   ³
                       	³ the early transmit interrupt, log an error ³
                       	³ but try to recover if possible by going    ³
                       	³ down the early transmit path at this time. ³
                       	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
                        if( acb->flags & WAIT_FOR_EARLY_XMIT )
                        {
                                acb->flags &= ~WAIT_FOR_EARLY_XMIT;
			   	MPATRACE3("Ofeb",acb->dev,acb->flags);
                                proc_tx_result(acb);
                        }
                        /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
                        ³ If this is the end of the transmit phase ³
                        ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
                        if (acb->flags & END_XMIT)
                        {
                        	MPATRACE4("OfLa",acb->dev,acb->state.oper_mode_8273,acb->state.port_b_8273);
                                acb->flags &= ~END_XMIT;
#ifndef NO_STRM_FLG_CTRL
                		/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
                		³ Turn off flag stream mode if on (this was ³
                		³ modified for defect #091439)              ³
                		ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
                		if( acb->state.oper_mode_8273 & SET_FLAG_STREAM )
                		{
                        		bzero(&acb->cmd_parms,sizeof(cmd_phase_t));
                        		acb->cmd_parms.cmd=RESET_OPER_MODE_CMD;
                        		acb->cmd_parms.parm[0]=RESET_FLAG_STREAM;
                        		acb->state.oper_mode_8273 &= RESET_FLAG_STREAM;
                        		acb->cmd_parms.parm_count=1;
                        		if( (rc=que_command(acb)) )
                        		{
                                		MPATRACE3("Ofec",acb->dev,rc);
                                		break;
                        		}
                		}
#endif
#ifndef NO_RTS_CTRL
                                /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
                                ³ drop RTS if on and not operating ³
                                ³ in continuous carrier mode       ³
                                ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
                                if( (acb->state.port_b_8273 & SET_RTS) &&
                                        !(acb->strt_blk.data_flags & DATA_FLG_C_CARR_ON) )
                                {
                                        bzero(&acb->cmd_parms,sizeof(cmd_phase_t));
                                        acb->cmd_parms.cmd=RESET_8273_PORT_B_CMD;
                                        acb->cmd_parms.parm[0]=RESET_8273_PORT_B_RTS;
                                        acb->state.port_b_8273 &= RESET_8273_PORT_B_RTS;
                                        acb->cmd_parms.parm_count=1;
                                        rc=que_command(acb);
                                }
#endif
                        }
                        ++acb->stats.ds.xmit_completes;
                        break;
		   case XMIT_DMA_UNDERRUN:
                        MPATRACE4("OfLb",acb->dev,irptp->type_flag,result);
			mpalog(acb,ERRID_MPA_XMTUND, __LINE__,__FILE__,0,0);
			++acb->stats.ds.xmit_dma_underrun;
			acb->flags |= XM_DISCARD;
			proc_tx_result(acb);
			acb->flags &= ~XM_DISCARD;
                        /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
                        ³ If this is the end of the transmit phase ³
                        ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
                        if (acb->flags & END_XMIT)
                        {
                        	MPATRACE4("OfLc",acb->dev,acb->state.oper_mode_8273,acb->state.port_b_8273);
                                acb->flags &= ~END_XMIT;
#ifndef NO_RTS_CTRL
                                /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
                                ³ drop RTS if on and not operating ³
                                ³ in continuous carrier mode       ³
                                ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
                                if( (acb->state.port_b_8273 & SET_RTS) &&
                                        !(acb->strt_blk.data_flags & DATA_FLG_C_CARR_ON) )
                                {
                                        bzero(&acb->cmd_parms,sizeof(cmd_phase_t));
                                        acb->cmd_parms.cmd=RESET_8273_PORT_B_CMD;
                                        acb->cmd_parms.parm[0]=RESET_8273_PORT_B_RTS;
                                        acb->state.port_b_8273 &= RESET_8273_PORT_B_RTS;
                                        acb->cmd_parms.parm_count=1;
                                        rc=que_command(acb);
                                }
#endif
                        }
			break;
		   case XMIT_CL_TO_SEND_ERR:
                        MPATRACE4("OfLd",acb->dev,irptp->type_flag,result);
                        /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
                        ³ If this error occurred during a transmit ³
                        ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
			if( !(acb->flags & RECEIVER_ENABLED) )
			{
				mpalog(acb,ERRID_MPA_CTSDRP, __LINE__,__FILE__,0,0);
			}
			++acb->stats.ds.xmit_cts_errors;
			acb->flags |= XM_DISCARD;
			proc_tx_result(acb);
			acb->flags &= ~XM_DISCARD;
                        /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
                        ³ If this is the end of the transmit phase ³
                        ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
                        if (acb->flags & END_XMIT)
                        {
                        	MPATRACE4("OfLe",acb->dev,acb->state.oper_mode_8273,acb->state.port_b_8273);
                                acb->flags &= ~END_XMIT;
#ifndef NO_RTS_CTRL
                                /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
                                ³ drop RTS if on and not operating ³
                                ³ in continuous carrier mode       ³
                                ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
                                if( (acb->state.port_b_8273 & SET_RTS) &&
                                        !(acb->strt_blk.data_flags & DATA_FLG_C_CARR_ON) )
                                {
                                        bzero(&acb->cmd_parms,sizeof(cmd_phase_t));
                                        acb->cmd_parms.cmd=RESET_8273_PORT_B_CMD;
                                        acb->cmd_parms.parm[0]=RESET_8273_PORT_B_RTS;
                                        acb->state.port_b_8273 &= RESET_8273_PORT_B_RTS;
                                        acb->cmd_parms.parm_count=1;
                                        rc=que_command(acb);
                                }
#endif
                        }
			break;
		   case XMIT_ABORT_DONE:
                        MPATRACE4("OfLf",acb->dev,irptp->type_flag,result);
			++acb->stats.ds.xmit_aborts;
			acb->flags |= XM_DISCARD;
			proc_tx_result(acb);
			acb->flags &= ~XM_DISCARD;
                        /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
                        ³ If this is the end of the transmit phase ³
                        ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
                        if (acb->flags & END_XMIT)
                        {
                        	MPATRACE4("OfLg",acb->dev,acb->state.oper_mode_8273,acb->state.port_b_8273);
                                acb->flags &= ~END_XMIT;
#ifndef NO_RTS_CTRL
                                /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
                                ³ drop RTS if on and not operating ³
                                ³ in continuous carrier mode       ³
                                ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
                                if( (acb->state.port_b_8273 & SET_RTS) &&
                                        !(acb->strt_blk.data_flags & DATA_FLG_C_CARR_ON) )
                                {
                                        bzero(&acb->cmd_parms,sizeof(cmd_phase_t));
                                        acb->cmd_parms.cmd=RESET_8273_PORT_B_CMD;
                                        acb->cmd_parms.parm[0]=RESET_8273_PORT_B_RTS;
                                        acb->state.port_b_8273 &= RESET_8273_PORT_B_RTS;
                                        acb->cmd_parms.parm_count=1;
                                        rc=que_command(acb);
                                }
#endif
                        }
			break;
		   case XMIT_PIO_ERROR:
                        MPATRACE4("OfLh",acb->dev,irptp->type_flag,result);
			++acb->stats.ds.io_irpt_error;
			acb->flags |= XM_DISCARD;
			proc_tx_result(acb);
			acb->flags &= ~XM_DISCARD;
                        /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
                        ³ If this is the end of the transmit phase ³
                        ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
                        if (acb->flags & END_XMIT)
                        {
                        	MPATRACE4("OfLi",acb->dev,acb->state.oper_mode_8273,acb->state.port_b_8273);
                                acb->flags &= ~END_XMIT;
#ifndef NO_RTS_CTRL
                                /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
                                ³ drop RTS if on and not operating ³
                                ³ in continuous carrier mode       ³
                                ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
                                if( (acb->state.port_b_8273 & SET_RTS) &&
                                        !(acb->strt_blk.data_flags & DATA_FLG_C_CARR_ON) )
                                {
                                        bzero(&acb->cmd_parms,sizeof(cmd_phase_t));
                                        acb->cmd_parms.cmd=RESET_8273_PORT_B_CMD;
                                        acb->cmd_parms.parm[0]=RESET_8273_PORT_B_RTS;
                                        acb->state.port_b_8273 &= RESET_8273_PORT_B_RTS;
                                        acb->cmd_parms.parm_count=1;
                                        rc=que_command(acb);
                                }
#endif
                        }
			break;

		   default:
                        MPATRACE4("OfLj",acb->dev,irptp->type_flag,result);
			mpalog(acb,ERRID_MPA_ADPERR, __LINE__,__FILE__,irptp->tp.TIC,0);
			acb->flags |= XM_DISCARD;
			proc_tx_result(acb);
			acb->flags &= ~XM_DISCARD;
                        /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
                        ³ If this is the end of the transmit phase ³
                        ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
                        if (acb->flags & END_XMIT)
                        {
                        	MPATRACE4("OfLk",acb->dev,acb->state.oper_mode_8273,acb->state.port_b_8273);
                                acb->flags &= ~END_XMIT;
#ifndef NO_RTS_CTRL
                                /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
                                ³ drop RTS if on and not operating ³
                                ³ in continuous carrier mode       ³
                                ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
                                if( (acb->state.port_b_8273 & SET_RTS) &&
                                        !(acb->strt_blk.data_flags & DATA_FLG_C_CARR_ON) )
                                {
                                        bzero(&acb->cmd_parms,sizeof(cmd_phase_t));
                                        acb->cmd_parms.cmd=RESET_8273_PORT_B_CMD;
                                        acb->cmd_parms.parm[0]=RESET_8273_PORT_B_RTS;
                                        acb->state.port_b_8273 &= RESET_8273_PORT_B_RTS;
                                        acb->cmd_parms.parm_count=1;
                                        rc=que_command(acb);
                                }
#endif
                        }
			break;
		}

		break;

	   default:
		mpalog(acb,ERRID_MPA_ADPERR, __LINE__,__FILE__,rc,0);
                MPATRACE3("OfLl",acb->dev,result);
		break;
	}

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ Free the IRPT element. ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	free_irpt_elem(acb,irptp);

    }    /* end of while irpt elements left on the q */


    MPATRACE4("OfLX",acb->dev,acb,rc);

    return 0;
}              /* mpa_offlvl() */

/*
 * NAME: q_irpt_results
 *
 * FUNCTION: Allocates memory for an irpt results structure and
 *           reads the results from the card, then fills in the
 *           struct values and adds it to the irpt results struct
 *           chain.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment-Specific aspects, such as -
 *      Preemptable        : Maybe
 *      VMM Critical Region: Yes
 *      Runs on Fixed Stack: Yes
 *      May Page Fault     : no
 *      May Backtrack      : no
 *
 * NOTES:
 *
 *
 * DATA STRUCTURES: acb altered.
 *
 * RETURN VALUE DESCRIPTION: NONE
 */

int q_irpt_results (
		    struct acb *acb,
		    int type)
{
     irpt_elem_t     *irptp;
     int             rc,res,result=0,recv_bytes;
     dma_elem_t      *dmap;
     recv_elem_t      *recvp;

     /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
     ³ take element off the free_irpt q. ³
     ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
     if( (irptp = acb->irpt_free) == NULL) 
     {
	  ++acb->stats.ds.recv_not_handled;
	  acb->flags &= ~IRPT_QUEUED;
	  return(NULL);
     }
     acb->irpt_free=irptp->ip_next;
     irptp->ip_next = NULL;

     /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
     ³ Put this element on the active irpt q for this adapter ³
     ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
     if(acb->act_irpt_head==NULL) 
     {  /* its first one */
	 acb->act_irpt_head=irptp;
	 acb->act_irpt_tail=irptp;
     }
     else 
     {        /* its going on the end of a chain */
	 acb->act_irpt_tail->ip_next=irptp;
	 acb->act_irpt_tail=irptp;
     }
     irptp->ip_state |= IP_ACTIVE;
     irptp->type_flag = type;

     /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
     ³ Switch based on type of irpt. ³
     ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
     switch(type) 
     {
	case XMIT_RESULT:
		/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		³ set up bus access and get the io base addr ³
		ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
		if((result=PIO_GETC( acb, RD_TX_IR_OFFSET))==-1) 
     		{
		   mpalog(acb,ERRID_MPA_ADPERR,__LINE__,__FILE__,0,0);
		   irptp->tp.TIC = XMIT_PIO_ERROR;
		   return(irptp->tp.TIC);
		}
		irptp->tp.TIC = (uchar) result;
		if( (result == XMIT_COMPLETE) &&
			!(acb->flags & WAIT_FOR_EARLY_XMIT) )
		{
                     free_irpt_elem(acb,irptp);
                     acb->flags &= ~IRPT_QUEUED;
                     return(result);
		}
		break;
	case RECV_RESULT:

		/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		³ Read the first result byte from result reg ³
		ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
		
		if((rc=PIO_GETC( acb, RD_RX_IR_OFFSET))==-1) /* set up bus access and get the io base addr  */
		{
		   mpalog(acb,ERRID_MPA_ADPERR,__LINE__,__FILE__,0,0);
		   irptp->tp.rcv.RIC = RECV_PIO_ERROR;
		   return(irptp->tp.rcv.RIC);
		}
		irptp->tp.rcv.RIC = (uchar) rc;

		/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		³ If this is an idle detect irpt there are no more results, ³
		³ so don't try to read them.                                ³
		ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
		result = (irptp->tp.rcv.RIC & RIC_MASK_LAST_BYTE);
		if(result != RECV_IDLE) 
		{
		    /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		    ³ If card is in buffered mode there will be 4 more result ³
		    ³ bytes to read otherwise there will be only 2 more bytes.³
		    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
		    if(!(acb->state.oper_mode_8273&SET_BUFFERED_MODE)) 
		    {
			  if( (res = get_rx_result(acb)) == 300) 
			  {
				irptp->tp.rcv.R0 = 0xFF;
				irptp->tp.rcv.R1 = 0xFF;
				break;
			  }
			  else if(res == 301) 
			  {
				irptp->tp.rcv.RIC = RECV_PIO_ERROR;
				break;
			  }
			  else irptp->tp.rcv.R0 = (uchar) res;

			  if( (res = get_rx_result(acb)) == 300) break;
			  else if(res == 301) 
			  {
				irptp->tp.rcv.RIC = RECV_PIO_ERROR;
				break;
			  }
			  else irptp->tp.rcv.R1 = (uchar) res;
		    }
		    else  
		    {
			  if( (res = get_rx_result(acb)) == 300) 
			  {
				irptp->tp.rcv.R0 = 0xFF;
				irptp->tp.rcv.R1 = 0xFF;
				break;
			  }
			  else if(res == 301) 
			  {
				irptp->tp.rcv.RIC = RECV_PIO_ERROR;
				break;
			  }
			  else irptp->tp.rcv.R0 = (uchar) res;

			  if( (res = get_rx_result(acb)) == 300) break;
			  else if(res == 301) 
			  {
				irptp->tp.rcv.RIC = RECV_PIO_ERROR;
				break;
			  }
			  else irptp->tp.rcv.R1 = (uchar) res;

			  if( (res = get_rx_result(acb)) == 300) break;
			  else if(res == 301) 
			  {
				irptp->tp.rcv.RIC = RECV_PIO_ERROR;
				break;
			  }
			  else irptp->tp.rcv.ADR = (uchar) res;

			  if( (res = get_rx_result(acb)) == 300) break;
			  else if(res == 301) 
			  {
				irptp->tp.rcv.RIC = RECV_PIO_ERROR;
				break;
			  }
			  else irptp->tp.rcv.CNTL = (uchar) res;
		    }
		    recv_bytes = irptp->tp.rcv.R1;
		    recv_bytes = (recv_bytes<<8)|irptp->tp.rcv.R0;
		    acb->total_bytes += recv_bytes;

		}   /* end of if not idle detect irpt */

		/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		³ check to see if I need to restart the receiver ³
		ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
		switch(result) 
		{
		   case RECV_GEN_OK:
		   case RECV_SEL_OK:
			/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
			³ receiver is still active so just check the ³
			³ number of bytes received since last recv   ³
			³ command was issued, if its greater than    ³
			³ 60k restart receiver, else leave current   ³
			³ recv active                                ³
			ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
			if(acb->total_bytes > 61439) 
			{
			   acb->total_bytes = 0;
			   if(acb->station_type & GENERAL) 
			   {
			      acb->cmd_parms.cmd=GEN_RECEIVE_CMD;
			      acb->cmd_parms.parm[0]=MAX_RECV_SIZE;
			      acb->cmd_parms.parm[1]=(MAX_RECV_SIZE>>8);
			      acb->cmd_parms.parm_count=2;
			   }
			   else 
			   {
			      acb->cmd_parms.cmd=SEL_RECEIVE_CMD;
			      acb->cmd_parms.parm[0]=MAX_RECV_SIZE;
			      acb->cmd_parms.parm[1]=(MAX_RECV_SIZE>>8);
			      acb->cmd_parms.parm[2]=acb->station_addr;
			      acb->cmd_parms.parm[3]=(uchar)BROADCAST_ADDR;
			      acb->cmd_parms.parm_count=4;
			   }
			   if( (rc=que_command(acb)) ) 
			   {
			      break;
			   }
			   acb->flags |= RECEIVER_ENABLED;
			}
			/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		  	³ Check to see if the DMA was set up for receive. ³
			ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
			if( (dmap = acb->act_dma_head) != NULL )
			{
                             if( dmap->dm_req_type!=DM_RECV )
                             {
                             	  acb->flags &= ~RECV_DMA_ON_Q;
		   	     	  mpalog(acb,ERRID_MPA_RCVERR,__LINE__,__FILE__,MP_RX_ABORT,0);
			     	  free_irpt_elem(acb,irptp);
			     	  acb->flags &= ~IRPT_QUEUED;
			     	  return(result);
			     }
			}
			else
			{
                             acb->flags &= ~RECV_DMA_ON_Q;
		   	     mpalog(acb,ERRID_MPA_RCVERR,__LINE__,__FILE__,MP_RX_ABORT,0);
                             free_irpt_elem(acb,irptp);
                             acb->flags &= ~IRPT_QUEUED;
                             return(result);
			}

			/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
			³ since this receive was good and the MBUF ³
			³ associated with it will be freed by the  ³
			³ caller, turn off the RC_MBUF flag        ³
			ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
			recvp = (recv_elem_t *)dmap->p.recv_ptr;
			if(recvp != NULL)
			{
			     recvp->rc_state &= ~RC_MBUF;
			}
			else
			{
                             acb->flags &= ~RECV_DMA_ON_Q;
                             mpalog(acb,ERRID_MPA_RCVERR,__LINE__,__FILE__,MP_RX_ABORT,0);
                             free_irpt_elem(acb,irptp);
                             acb->flags &= ~IRPT_QUEUED;
                             return(result);
			}
			/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
			³ always d_complete current dma ³
			ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
			if( free_dma_elem(acb,dmap) )
			{
                             acb->flags &= ~RECV_DMA_ON_Q;
                             mpalog(acb,ERRID_MPA_RCVERR,__LINE__,__FILE__,MP_RX_ABORT,0);
                             free_irpt_elem(acb,irptp);
                             acb->flags &= ~IRPT_QUEUED;
                             return(result);
			}

			break;
		  /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		  ³ for all below, let the off level handle it ³
		  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
		   case RECV_CRC_ERR:
		   case RECV_ABORTED:
		   case RECV_BAD_FRAME:
		   case RECV_DMA_OVERRUN:
		   case RECV_MEM_OVERFLOW:
		   case RECV_CARRIER_DOWN:
		   case RECV_PIO_ERROR:
		   case RECV_IDLE:
		   case RECV_EOP:
		   case RECV_IRPT_OVERRUN:
			break;
		   default:
			mpalog(acb,ERRID_MPA_ADPERR,__LINE__,__FILE__,0,0);
			break;
		}
	     break;

	default:
	     mpalog(acb,ERRID_MPA_ADPERR,__LINE__,__FILE__,0,0);
	     break;
     }
     if (acb->irpt_free == NULL) 
     {
	mpalog(acb,ERRID_MPA_BFR,__LINE__,__FILE__,0,0);
	acb->flags |= NEED_IRPT_ELEM;
     }

     /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
     ³  Set the IRPT_QUEUED flag so offlevel will be scheduled. ³
     ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
     acb->flags |= IRPT_QUEUED;

     return(result);
}   /* q_irpt_results() */


/*
**  this function waits for receive results to be available then
**  reads and returns the resutls
**
**  Returns 301 for PIO error
**  Returns 300 for timeout error (waiting for result status)
**  Returns < 256 valid result value.
*/
int get_rx_result (struct acb *acb)
{
    int rc=0,cnt=0;

    /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ loop here waiting for the card to raise recv result read bit. ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
    while( !(rc&RX_RESULT_READY ) && ++cnt<LOOP_CNTR) 
    {
	 if((rc=PIO_GETC( acb, RD_STAT_OFFSET))==-1) 
	 {
	       mpalog(acb,ERRID_MPA_ADPERR,__LINE__,__FILE__,0,0);
	       return 301;
	 }
    }

    /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³  If it takes longer that 5 tries, assume there are no more results ³
    ³  else read and return the next result value.                       ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
    if(cnt==LOOP_CNTR) 
    {
	 rc= 300;
    }
    else  
    {
	 if((rc=PIO_GETC( acb, RD_RX_IR_OFFSET))==-1) 
         {
	       mpalog(acb,ERRID_MPA_ADPERR,__LINE__,__FILE__,0,0);
	       return 301;
	 }
    }
    return(rc);
} /* get_rx_result() */

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
³ dsr_timer() - this routine checks the status of DSR 50ms after being ³
³		interrupted when DSR came on.  It will indicate to the ³
³		caller that the line is active.			       ³
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
void dsr_timer(struct acb *acb)
{
	int rc=0;

   	MPATRACE4("DsrE",acb,acb->dev,rc);
        /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
        ³ read the 8273 PORT A register for status of DSR ³
        ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
       	bzero(&acb->cmd_parms,sizeof(cmd_phase_t)); 
       	acb->cmd_parms.cmd = READ_8273_PORT_A_CMD;
       	acb->cmd_parms.parm_count = 0;
       	acb->cmd_parms.flag = RETURN_RESULT;

       	if( (rc=que_command(acb)) )
       	{
   		MPATRACE4("Dsr1",acb,acb->dev,rc);
		shutdown_adapter(acb);
		free_active_q(acb);
        	async_status(acb,CIO_ASYNC_STATUS, MP_ADAP_NOT_FUNC,0,0,0); 
		return;
       	}

        if( acb->flags & STARTED_CIO ) /* DSR previously active */
	{
        	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		³ If DSR is still not on after 50ms interval ³
        	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
		if ( !(acb->cmd_parms.result & PORT_A_8273_PA2) )  
		{
   			MPATRACE4("Dsr2",acb,acb->dev,acb->cmd_parms.result);
                       	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
                       	³ turn off flag stream mode if on (this was ³
                        ³ modified for defect #091439)              ³   
                       	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
                       	if( acb->state.oper_mode_8273 & SET_FLAG_STREAM )
                       	{
                       		bzero(&acb->cmd_parms,sizeof(cmd_phase_t));
                       		acb->cmd_parms.cmd=RESET_OPER_MODE_CMD;
                       		acb->cmd_parms.parm[0]=RESET_FLAG_STREAM;
                       		acb->state.oper_mode_8273 &= RESET_FLAG_STREAM;
                       		acb->cmd_parms.parm_count=1;
                        	if( (rc=que_command(acb)) )
                        	{
   					MPATRACE4("Dsr3",acb,acb->dev,rc);
					shutdown_adapter(acb);
					free_active_q(acb);
        				async_status(acb,CIO_ASYNC_STATUS, MP_DSR_DROPPED,0,0,0); 
                                	return;
                        	}
                        }

			/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
                        ³ Since DSR dropped, turn off DTR ³
			³ and drop RTS			  ³ 
                        ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
                        bzero(&acb->cmd_parms,sizeof(cmd_phase_t));
                        acb->cmd_parms.cmd=RESET_8273_PORT_B_CMD;
                        acb->cmd_parms.parm[0]= RESET_8273_PORT_B_PB2; 
                        acb->cmd_parms.parm[0]&= RESET_8273_PORT_B_RTS; 
                        acb->cmd_parms.parm_count=1;
        
                        if( (rc=que_command(acb)) )
                        {
   				MPATRACE4("Dsr4",acb,acb->dev,rc);
				shutdown_adapter(acb);
				free_active_q(acb);
        			async_status(acb,CIO_ASYNC_STATUS, MP_DSR_DROPPED,0,0,0); 
                                return;
                        }
			/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
			³ Reflect new state of 8273 PORT B in ACB ³
			ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
                        acb->state.port_b_8273 &= RESET_8273_PORT_B_PB2;
                        acb->state.port_b_8273 &= RESET_8273_PORT_B_RTS;
         		mpalog(acb,ERRID_MPA_DSRDRP , __LINE__,__FILE__,0,0);
			async_status(acb, CIO_ASYNC_STATUS, MP_DSR_DROPPED, 0, 0, 0);
		}
	}
	else  /* DSR has not been active yet */
	{
        	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		³ If DSR is still on after 50ms interval ³
        	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
		if( acb->cmd_parms.result & PORT_A_8273_PA2 )
		{

			if (!(acb->state.port_b_8273 & SET_DTR) &&
				(acb->strt_blk.modem_flags & MF_CDSTL_ON) )
			{
   				MPATRACE4("Dsr5",acb,acb->dev,acb->strt_blk.modem_flags);
				untimeout( ring_timer, (caddr_t)acb);
                		/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
                		³  Now set data terminal ready ³
                		ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
                		bzero(&acb->cmd_parms,sizeof(cmd_phase_t));
                		acb->cmd_parms.cmd=SET_8273_PORT_B_CMD;
                		acb->cmd_parms.parm[0]=SET_DTR;
                		acb->cmd_parms.parm_count=1;
		
                		if( (rc=que_command(acb)) )
                		{
   				 	MPATRACE4("Dsr6",acb,acb->dev,rc);
        				async_status(acb,CIO_START_DONE, CIO_NOT_STARTED,0,0, 0); 
                        	 	return;
                		}
                		acb->state.port_b_8273 |=SET_DTR;
			}
			/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
			³ Inform caller that line is ACTIVE ³
			ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
        		async_status(acb,CIO_START_DONE, CIO_OK,0,0, 0); 
			acb->flags |= STARTED_CIO;
			acb->flags &= ~WAIT_FOR_DSR; /* no longer waiting...*/
		}
	}

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ Enable the modem status change bit ³
	³ so modem changes can cause irpts   ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
        acb->state.port_b_8255 |= FREE_STAT_CHG; /* turn stat chg on */
        if(PIO_PUTC( acb, PORT_B_8255, acb->state.port_b_8255)==-1)
        {
   	 	MPATRACE3("Dsr7",acb,acb->dev);
		shutdown_adapter(acb);
		free_active_q(acb);
        	async_status(acb,CIO_ASYNC_STATUS, MP_ADAP_NOT_FUNC,0,0,0); 
                mpalog(acb,ERRID_MPA_ADPERR , __LINE__,__FILE__,0,0);
        }
       	MPATRACE4("DsrX",acb,acb->dev,rc);
	return;
} /* dsr_timer */
/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
³ proc_tx_result() - this routine processes the result of the transmit ³
³               interrupt and either transmits the next frame of data  ³
³               or restarts the receiver.                              ³
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
void
proc_tx_result(struct acb *acb)
{
    int                     rc=0;
    dma_elem_t              *dmap;          /* pointer to a DMA element  */
    xmit_elem_t             *xmitp;         /* pointer to a XMIT element */



        MPATRACE4("PtxE",acb->dev,acb,acb->flags);

        if((dmap=acb->act_dma_head) == NULL)
        {
                ++acb->stats.ds.xmit_not_handled;
                MPATRACE3("Pte1",acb->dev,dmap);
                return;
        }
        if(dmap->dm_req_type!=DM_XMIT)
        {
                ++acb->stats.ds.xmit_not_handled;
                MPATRACE4("Pte2",acb->dev,dmap,dmap->dm_req_type);
                return;
        }
        if( (xmitp = dmap->p.xmit_ptr) != acb->act_xmit_head)
        {
                ++acb->stats.ds.xmit_not_handled;
                MPATRACE4("Pte3",acb->dev,xmitp,dmap);
                return;
        }
        /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
        ³ If this frame had the poll/final bit ³
        ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
        if( xmitp->xm_flags & XMIT_FINAL )
        {
                MPATRACE4("Ptx1",acb->dev,xmitp,dmap);
                /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
                ³ Cancel pending transmit failsafe timer ³
                ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
                untimeout( xmit_timer, (caddr_t)acb );
                acb->flags &= ~XMIT_TIMER_ON;

                acb->flags |= END_XMIT;
        }

        if (xmitp->xm_ack & CIO_ACK_TX_DONE)
        {
                MPATRACE4("Ptx2",acb->dev,acb->flags,xmitp->xm_writeid);
                if (! (acb->flags & XM_DISCARD) )
                {
                        /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
                        ³ Notify, calling process good xmit done.³
                        ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
                        async_status(acb, CIO_TX_DONE,
                                CIO_OK,xmitp->xm_writeid,xmitp->xm_mbuf,NULL);
                }
        }
        /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
        ³ Update statistics counters. ³
        ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
        if (++acb->stats.cc.tx_frame_lcnt == 0)
        {
                acb->stats.cc.tx_frame_mcnt++;
        }
        if ((acb->stats.cc.tx_byte_lcnt += xmitp->xm_length) <
                xmitp->xm_length)
        {
                acb->stats.cc.tx_byte_mcnt++;
        }

        /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
        ³ Free the DMA element. ³
        ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
        free_dma_elem(acb,dmap);

        /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
        ³ If the kernel user wants us to free the mbufs,³
        ³ do so now.                                    ³
        ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
        if (xmitp->xm_mbuf
                && !(xmitp->xm_ack & CIO_NOFREE_MBUF))
        {
                MPATRACE4("Ptx3",acb->dev,dmap,xmitp->xm_mbuf);
                (void) m_freem(xmitp->xm_mbuf);
        }

        /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
        ³ Free the XMIT element. ³
        ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
        free_xmit_elem(acb,xmitp);

        /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
        ³ Restart the receive ³
        ³ or next transmit.   ³
        ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
        if ( (rc=startrecv(acb)) )
        {
                MPATRACE3("Pte5",acb->dev,rc);
                return;
        }
        if ( (acb->strt_blk.rcv_timeout != 0) &&
                (!(acb->flags & RCV_TIMER_ON)) )
        {
                acb->flags |= RCV_TIMER_ON;
                timeout(recv_timer, (caddr_t)acb, (HZ/10) * (int)acb->strt_blk.rcv_timeout );
        }

        MPATRACE3("PtxX",acb->dev,rc);
        return;

}/* proc_tx_result() */
