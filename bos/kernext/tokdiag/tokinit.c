static char sccsid[] = "@(#)36	1.37  src/bos/kernext/tokdiag/tokinit.c, diagddtok, bos411, 9428A410j 10/26/93 14:03:32";
/*
 * COMPONENT_NAME: (SYSXTOK) - Token-Ring device handler
 *
 * FUNCTIONS:  bringup_timer(), ds_act(),
 *             oflv_bringup(),
 *             reset_adap(), reset_phase2(), init_adap(), init_adap_phase0(),
 *             init_adap_phase1(), open_adap(), open_adap_phase0(),
 *             open_adap_pend(), open_timeout(),
 *             close_adap(),  ds_deact(),
 *             kill_act(), cfg_adap_parms()
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
#define TOKDEBUG_INIT   (1)
#define TOKDEBUG        (1)
*/

#include <sys/cblock.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/dma.h>
#include <sys/dump.h>
#include <sys/errno.h>
#include <sys/err_rec.h>
#include "tok_comio_errids.h"
#include <sys/file.h>
#include <sys/intr.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/lockl.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/param.h>
#include <sys/poll.h>
#include <sys/sleep.h>
#include <sys/time.h>
#include <sys/timer.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/watchdog.h>
#include <sys/xmem.h>
#include <sys/comio.h>
#include <sys/trchkid.h>
#include <sys/tokuser.h>
#include <sys/adspace.h>

#include "tokdshi.h"
#include "tokdds.h"
#include "tokdslo.h"
#include "tokproto.h"
#include "tokprim.h"




/*
* NAME: bringup_timer
*
* FUNCTION:
*
*  Adapter bringup timer function.
*  This timer function is used in the bringup and shutdown of the
*  Token-Ring adapter.
*
* EXECUTION ENVIRONMENT:
*
*       This routine executes on the timer level.
*
* NOTES:
*      This function will schedule the Token-Ring OFLV via the que_oflv()
*      function.  An error will occur when there is no room on the
*      OFLV queue. In this case, a the timo_lost RAS counter will be
*      incremented.
*
* (RECOVERY OPERATION:) Information describing both hardware and
*       software error recovery.
*
* (DATA STRUCTURES:) Effects on global data structures, similar to NOTES.
*
* RETURNS:
*      None.
*/
/*
*
*  INPUT:
*      A pointer to a time_data_t data structure.
*
*  OPERATION:
*
*      FUTURE FIX:
*          When no OFLV queue space is available, this recovery logic to this
*          may need to be added so that the timer is re-started.
*
*  RETURN CODE:
*
*/

void bringup_timer( struct trb *p_time)
{
  timer_data_t  *p_tdata;
  dds_t    *p_dds;

  p_tdata = (timer_data_t *)(p_time->func_data);
  p_dds = (dds_t *)(p_tdata->p_dds);

  /*
  *    Set flag to note that the timer is no longer running
  */
  p_tdata->run = FALSE;
   /*
   *   Schedule OFLV for the timer setter
   */
   if (que_oflv( &(p_dds->ofl),
                     &(p_tdata->owe) ) )
   {
       p_dds->ras.ds.timo_lost++;
   }
return;
}  /* end function bringup_timer() */

/*
*
* FUNCTION NAME = ds_act
*
* FUNCTION =  Initiate the Activation the Token-Ring Adapter.
*
* NOTES:
*
*       This function will initiate the bringup of the Token-Ring adapter.
*
*       This function assumes that:
*               1. The Adapter Control Area (ACA) has been successfully setup.
*               2. The Receive Chain is fully populated with pinned and
*                       d_master() PAGESIZE mbufs.
*               3. The Transmit and Receive Chain index pointers are
*                       setup to point to
*                       the first List element in the respective Chains.
*               4. The SLIH and OFF-LEVEL intr structures have been setup.
*               5. The SLIH has been successfully registered.
*
* RETURN CODES:
*      0   - Good return code.
*
*/

int
ds_act(dds_t *p_dds)
{      /* begin function ds_act */

  int rc=0, saved_int_lvl;
  offl_elem_t  owe;

   DEBUGTRACE2("actB", (ulong)p_dds);

	/* FUTURE FIX:
	 *	Do we want to allow ourselves to activate
	 *	if there has previously been a Hard failure
	 *	that caused us to bug_out()?
	 *	Research this.
	 */

  /*
   *   Check to see if opened in diagnostic mode, if so
   *   set the work area adapter initialization and open options
   *   to what is specified in the diagnostic options.
   */
   if ((p_dds->cio.mode == 'D') ||
       (p_dds->cio.mode == 'W'))
   {   /* put diagnostic options into affect */

   p_dds->wrk.adap_iparms.init_options =
                        p_dds->wrk.diag_iparms.init_options;


   p_dds->wrk.adap_iparms.rcv_burst_size =
                        p_dds->wrk.diag_iparms.rcv_burst_size;

   p_dds->wrk.adap_iparms.xmit_burst_size =
                        p_dds->wrk.diag_iparms.xmit_burst_size;

   p_dds->wrk.adap_iparms.dma_abort_thresh =
                        p_dds->wrk.diag_iparms.dma_abort_thresh;

   /* Now the open options */
   p_dds->wrk.adap_open_opts.options =
                        p_dds->wrk.diag_open_opts.options;

   p_dds->wrk.adap_open_opts.buf_size =
                        p_dds->wrk.diag_open_opts.buf_size;

   p_dds->wrk.adap_open_opts.xmit_buf_min_cnt =
                        p_dds->wrk.diag_open_opts.xmit_buf_min_cnt;

   p_dds->wrk.adap_open_opts.xmit_buf_max_cnt =
                        p_dds->wrk.diag_open_opts.xmit_buf_max_cnt;

   p_dds->wrk.adap_open_opts.node_addr1 =
                        p_dds->wrk.diag_open_opts.i_addr1;

   p_dds->wrk.adap_open_opts.node_addr2 =
                        p_dds->wrk.diag_open_opts.i_addr2;

   p_dds->wrk.adap_open_opts.node_addr3 =
                        p_dds->wrk.diag_open_opts.i_addr3;
       /*
       *   If opened in WRAP mode, force adapter into Wrap mode
       */
        if (p_dds->cio.mode == 'W')
                p_dds->wrk.adap_open_opts.options |= 0x8000;
        else
                p_dds->wrk.adap_open_opts.options &=0xefff;


   } /* endif */


        /*
        *  Set the adapter state to Phase 0 of the reset adapter
        *  sequence. 
        */

p_dds->wrk.reset_spin = 0;
p_dds->wrk.reset_retry = 0;
p_dds->wrk.adap_init_spin = 0;
p_dds->wrk.adap_init_retry = 0;
p_dds->wrk.adap_open_retry = 0;
p_dds->wrk.bcon_sb_sent = FALSE;

rc = i_disable(INTCLASS2);
p_dds->wrk.adap_state= RESET_PHASE0;
p_dds->wrk.adap_doa = ALIVE_N_WELL;
i_enable(rc);
rc = reset_adap(p_dds, &owe);

   DEBUGTRACE2("actE", (ulong)0);
   return(0);

} /* end function ds_act */

/*---------------------------------------------------------------------*/
/*                         OFLV BRINGUP                                */
/*---------------------------------------------------------------------*/
/*
*  FUNCTION:   oflv_bringup()
*
*  INPUT:      p_dds - pointer to DDS
*              p_owe - pointer to OFLV work element
*
*/
int oflv_bringup(dds_t *p_dds, offl_elem_t *p_owe)
{  /* begin function */

   unsigned int    rc=0;

  /*
   *   Determine what phase of the adapter
   *   activation sequence we are in.  Call
   *   the appropriate routine.
   */

   DEBUGTRACE2("oflB", p_owe->cmd);
   switch(p_owe->cmd)
   {
       case OFLV_ADAP_RESET:
       {
           rc = reset_adap(p_dds, p_owe);
           break;
       }
       case OFLV_ADAP_INIT:
       {
           rc = init_adap(p_dds, p_owe);
           break;
       }
       case OFLV_ADAP_OPEN:
       {
           rc = open_timeout(p_dds, p_owe);
           break;
       }
       case OFLV_ADAP_CLOSE:
       {
           p_dds->wrk.adap_state = NULL_STATE;
           e_wakeup(&(p_dds->wrk.close_event));
           p_dds->wrk.footprint = ACT_OFLV_BUP_0;
           TRACE2("FooT", (ulong)ACT_OFLV_BUP_0);
           break;
       }
       case OFLV_PROBATION:
           /*
           *   Our probation timer has popped
           *   We made it through the probationary period
           *   successfully. Start up the exodus of limbo.
           *
           *   If wrk.limbo != PROBATION, probably got a
           *   ring status interrupt == wire fault just before
           *   the timer popped 
           */
           TRACE2("FooT", (ulong)ACT_OFLV_BUP_1);
           if (p_dds->wrk.limbo == PROBATION) {
               rc = egress_limbo(p_dds);
           }
           else
           {
               TRACE2("FooT", (ulong)ACT_OFLV_BUP_6);
           }
           break;
       case OFLV_RECV_LIMBO_TIMEOUT:
           /*
            *  We did not get an SCB clear interrupt from
            *  the card. cycle limbo.
            */
           p_dds->wrk.footprint = ACT_OFLV_BUP_2;
           TRACE2("FooT", (ulong)ACT_OFLV_BUP_2);
	   if ( (p_dds->wrk.limbo != PARADISE) && 
		(p_dds->wrk.limbo != LIMBO_KILL_PEND) )
           	rc = cycle_limbo( p_dds );
	   else
           	TRACE2("FooT", (ulong)ACT_OFLV_BUP_6);
           break;

       case OFLV_FUNC_TIMEOUT:
           /*
           *   The setting of the functional address
           *   timed out. Cycle the recovery effort
           */
           p_dds->wrk.footprint = ACT_OFLV_BUP_3;
           TRACE2("FooT", (ulong)ACT_OFLV_BUP_3);
	   if ( (p_dds->wrk.limbo != PARADISE) && 
		(p_dds->wrk.limbo != LIMBO_KILL_PEND) )
           	rc = cycle_limbo( p_dds );
	   else
           	TRACE2("FooT", (ulong)ACT_OFLV_BUP_7);
           break;
       case OFLV_GROUP_TIMEOUT:
           /*
           *   The default is for OFLV_GROUP_TIMEOUT:
           *   The setting of the group address timed out.
           *   Cycle the recovery effort.
           */
           p_dds->wrk.footprint = ACT_OFLV_BUP_4;
           TRACE2("FooT", (ulong)ACT_OFLV_BUP_4);
	   if ( (p_dds->wrk.limbo != PARADISE) && 
		(p_dds->wrk.limbo != LIMBO_KILL_PEND) )
           	rc = cycle_limbo( p_dds );
	   else
           	TRACE2("FooT", (ulong)ACT_OFLV_BUP_8);
           break;
	default:
		/*
		 * Unknown cmd value.  This should NEVER happen
		 */
		p_dds->wrk.footprint = ACT_OFLV_BUP_5;
		TRACE3("FooT", (ulong)ACT_OFLV_BUP_5, (ulong)p_owe->cmd );
		break;
   }   /* end switch */

return(rc);

}  /* end function oflv_bringup */

/*--------------------------------------------------------------------*/
/***************        Reset Adapter                   ***************/
/*--------------------------------------------------------------------*/

/*

This routine will issue a reset then an enable to the Token-Ring
adapter.  After successful completion of this function, the adapter will be in
a state awaiting the initialization parameters.

RETURN CODES:
       0 - Successful completion
       ENETUNREACH - Reset of adapter failed.

If an unrecoverable hardware error occured, the failure reason will be stored
in p_dds->wrk.afoot.

ROUTINES CALLED:

RETRY LOGIC:


*/

int reset_adap(dds_t *p_dds, offl_elem_t *p_owe)

{  /* begin function reset_adap */
   offl_elem_t     owe;
        int     pio_attachment, sil;
        int     iocc_attachment;
   unsigned short rc=0;
   unsigned char pos4=0;
	int	i;	/* loop	control	*/
	int	tmp;	/* value read from delay reg (ignored)	*/
	int	delay_seg;
	uchar	pos5;


   DEBUGTRACE2("rset", p_dds->wrk.adap_state);
   switch(p_dds->wrk.adap_state)
   {
       case RESET_PHASE0:
       {
           /*
            *  Set timer  up
            */
            p_dds->wrk.p_bringup_timer->timeout.it_value.tv_sec
                        = FIVE_HUNDRED_MS / 1000;
            p_dds->wrk.p_bringup_timer->timeout.it_value.tv_nsec
                        = (FIVE_HUNDRED_MS * 1000);
            p_dds->wrk.time_data.owe.who_queued = OFFL_ADAP_BRINGUP;
            p_dds->wrk.time_data.owe.cmd = OFLV_ADAP_RESET;
            sil = i_disable(INTCLASS2);
            p_dds->wrk.adap_state = RESET_PHASE1;
	    p_dds->wrk.adap_doa = ALIVE_N_WELL;
            i_enable(sil);

           /*
           *   Get the current POS 4 Setting.
           *   Turn Off Parity.
           */
           iocc_attachment = attach_iocc(p_dds);
           pos4 = pio_read( p_dds, POS_REG_4);
           pos4 = pos4 & ~(MC_PARITY_ON);
           pio_write( p_dds, POS_REG_4, pos4);

	   /*
 	    *   Get the current POS 5 Setting and disable DMA arbitration
	    *   There is a timing window if the adapter is doing DMA when the
	    *   reset occurs which could cause a bus timeout.
 	    */
	   pos5 = pio_read(p_dds, POS_REG_5);
	   pos5 = pos5 | MC_ARBITRATION;
	   pio_write( p_dds, POS_REG_5, pos5);
           detach_iocc(p_dds, iocc_attachment);

           pio_attachment = attach_bus(p_dds);

          /*
           *   NOTE:
           *       If the adapter interrupts are not currently
           *       disabled, we need to disable them.  This is
           *       to compensate for the spurious interrupt
           *       generated by the Token-Ring adapter during
           *       the reset sequence.
           *
           */

            if (!p_dds->wrk.mask_int)
            {
               pio_write( p_dds, IMASK_DISABLE, 0x00 );
               sil = i_disable(INTCLASS2);
               p_dds->wrk.mask_int = TRUE;
               i_enable(sil);
            }

	    /*
	     * ensure that there is no DMA going on (wouldn't expect any)
	     * wait 100 usec
	     */
	    delay_seg = (uint)IOCC_ATT(p_dds->ddi.bus_id, DL_DELAY_REG ); 
	    i = 0;
	    while ( ++i < 100) 
		tmp = BUSIO_GETC(delay_seg); /* delay 1 microsecond */
	    IOCC_DET(delay_seg);

            pio_write( p_dds, RESET_REG, 0x00 );
            detach_bus( p_dds, pio_attachment );
            p_dds->wrk.time_data.run = TRUE;
            tstart(p_dds->wrk.p_bringup_timer);
            break;
       }       /* end case RESET_PHASE0 */
       case RESET_PHASE1:
       {
           /*
            *  Set timer for RESET_SPIN_DELTA.  This is done to
            *  check on the status at RESET_SPIN_DELTA intervals so
            *  we will know the results of the reset sooner than if we
            *  just checked the status after the reset timeout.
            *
            *  The reset timeout value is RESET_SPIN_DELTA*RESET_SPIN_TIME,
            *  where RESET_SPIN_TIME is the max number of times to pop a
            *  timer for a duration of RESET_SPIN_DELTA.
            *
            */
            p_dds->wrk.p_bringup_timer->timeout.it_value.tv_sec
                                = (RESET_SPIN_DELTA / 1000);
            p_dds->wrk.p_bringup_timer->timeout.it_value.tv_nsec
                                = (RESET_SPIN_DELTA % 1000) * 1000000;
            p_dds->wrk.time_data.owe.who_queued = OFFL_ADAP_BRINGUP;
            p_dds->wrk.time_data.owe.cmd = OFLV_ADAP_RESET;
            sil = i_disable(INTCLASS2);
            p_dds->wrk.adap_state = RESET_PHASE2;
            p_dds->wrk.reset_spin = 0;
            i_enable(sil);

            pio_attachment = attach_bus(p_dds);
            pio_write( p_dds, ENABLE_REG, 0x00 );
            detach_bus( p_dds, pio_attachment );

	    /*
 	     *   Get the current POS 5 Setting.
 	     *   Allow DMA arbitration
 	     */
            iocc_attachment = attach_iocc(p_dds);
	    pos5 = pio_read(p_dds, POS_REG_5);
	    pos5 = pos5 & ~(MC_ARBITRATION);
	    pio_write( p_dds, POS_REG_5, pos5);
	    detach_iocc(p_dds, iocc_attachment);

            p_dds->wrk.time_data.run = TRUE;
            tstart(p_dds->wrk.p_bringup_timer);
            break;
       }       /* end case RESET_PHASE1 */
       case RESET_PHASE2:
       {
           rc = reset_phase2(p_dds, p_owe);
           break;
       }   /* end case RESET_PHASE2 */
       default:
                /*
                *  FUTURE FIX:
                *       Add logic to handle the error situation
                *       when we fall thru to the default
                *       switch statement.
                */
                TRACE2("FooT", ACT_RESET_ADAP);
           break;

   }   /* end switch */
   return(rc);

}  /* end reset_adap() function */

/*---------------------------------------------------------------------*/
/*                         RESET PHASE 2                               */
/*---------------------------------------------------------------------*/

/*
*  FUNCTION:   reset_phase2()
*
*  INPUT:  p_dds - Pointer to dds_t
*          p_owe - Pointer to offl_elem_t
*
*/

int reset_phase2(dds_t *p_dds, offl_elem_t *p_owe)

{  /* begin function */
   unsigned short  rc=0;
        int     pio_attachment, sil;
        int     iocc_attachment;



        pio_attachment = attach_bus(p_dds);
        rc = pio_read(p_dds, STATUS_REG);
        detach_bus( p_dds,pio_attachment);

if ((rc & 0x00f0) == RESET_OK)
{      /* adapter reset worked! */
  /*
   *   NOTE:
   *       Since we successfully reset the adapter,
   *       we can now re-enable the adapter interrupts.
   *       The state in which the Token-Ring adapter is
   *       known to generate spurious interrupts has
   *       now passed.
   *
   */
   if (p_dds->wrk.mask_int)
   {
       pio_attachment = attach_bus(p_dds);
       pio_write( p_dds, IMASK_ENABLE, 0x00 );
       detach_bus( p_dds, pio_attachment );
       sil = i_disable(INTCLASS2);
       p_dds->wrk.mask_int = FALSE;
       i_enable(sil);
   }
  /*
   *   The next step in the adapter bringup is the
   *   adapter initialization
   *   Change the adap_state to ADAP_INIT_PHASE0 and
   *   call init_adap() routine.
   */

   sil = i_disable(INTCLASS2);
   p_dds->wrk.adap_state = ADAP_INIT_PHASE0;
   i_enable(sil);
   rc = init_adap(p_dds, p_owe);

} /* end if reset worked */
else if ( (rc & 0x00f0) == RESET_FAIL)
{
      /* reset did not work...Unrecoverable HW error */
   p_dds->wrk.footprint = ACT_RESET_PHS2_0;
   p_dds->wrk.afoot = rc;
     /*
     * If in Ring Recovery mode, to kick
     * off another retry cycle.
     * Otherwise, this is in response to the 1st activation...
     * Build the start done status block.
     */
   TRACE3("FooT", ACT_RESET_PHS2_0, (ulong)rc);

   if (p_dds->wrk.limbo != PARADISE)
   {
      /*
       *   No dice.  Go directly to Limbo. Do not collect $200.
       */
       rc = cycle_limbo(p_dds);
   }
   else if (p_dds->wrk.reset_retry >=RESET_MAX_RETRY)
   {
   /*
   *   Build the start done status block
   */
       p_dds->wrk.start_done_blk.code = CIO_START_DONE;
       p_dds->wrk.start_done_blk.option[0] =
                                           TOK_ADAP_INIT_FAIL;
   /*
   *   option[1] will be filled in with the netid
   *   when the ds_startblk() routine is called.
   */
       p_dds->wrk.start_done_blk.option[1] = NULL;
       p_dds->wrk.start_done_blk.option[2] = rc;
       p_dds->wrk.start_done_blk.option[3] = NULL;
       p_dds->wrk.adap_state = CLOSED_STATE;

      /*
       *  Log Permanent hardware error and return
       */
       logerr(p_dds, ERRID_TOK_ERR5);
       conn_done(p_dds);

       rc= ENETUNREACH;
   } /* endif */
   else
   {
       /*
       *   We have more retrys left in this activation thread.
       *
       *   NOTE:
       *       We must change the adapter state to RESET_PHASE0
       *       This is done so as to not go into an infinint loop
       *       of recursion.
       */
       TRACE2("FooT", ACT_RESET_PHS2_1);

       sil = i_disable(INTCLASS2);
       p_dds->wrk.adap_state = RESET_PHASE0;
       ++p_dds->wrk.reset_retry;
       i_enable(sil);
       rc = reset_adap(p_dds, p_owe);

   }   /* end if more reset retrys avail */
}
else if (p_dds->wrk.reset_spin >= RESET_SPIN_TIME)
{
   /*
   * If in Ring Recovery mode, then kick
   * off another retry cycle.
   * Otherwise, this is in response to the 1st activation...
   * Build the start done status block.
   */

   p_dds->wrk.footprint = ACT_RESET_PHS2_2;
   p_dds->wrk.afoot = rc;
   TRACE3("FooT", ACT_RESET_PHS2_2, (ulong)rc );

   if (p_dds->wrk.limbo != PARADISE)
   {
      /*
      *   We didn't make it out.  Try, try again...
      */
       rc = cycle_limbo(p_dds);

   }   /* end if in Ring Recovery mode */
   else if (p_dds->wrk.reset_retry >= RESET_MAX_RETRY)
   {

      /*
       *   Build the start done status block
       */
       p_dds->wrk.start_done_blk.code = CIO_START_DONE;

       p_dds->wrk.start_done_blk.option[0] = TOK_ADAP_INIT_TIMEOUT;

      /*
       *   option[1] will be filled in with the netid
       *   when the ds_startblk() routine is called.
       */
       p_dds->wrk.start_done_blk.option[1] = NULL;
       p_dds->wrk.start_done_blk.option[2] = rc;
       p_dds->wrk.start_done_blk.option[3] = NULL;

       p_dds->wrk.adap_state = CLOSED_STATE;

      /*
       *  Log permanent hardware error and return
       */
       logerr(p_dds, ERRID_TOK_ERR5);

       conn_done(p_dds);
       rc = ENETUNREACH;
   }   /* end if no more reset retrys left */
   else
   {       /* we have more Reset Retrys left */

       /*
       *   NOTE:
       *       We must change the adapter state to RESET_PHASE0
       *       This is done so as to not go into an infinint loop
       *       of recursion.
       */

       TRACE2("FooT", ACT_RESET_PHS2_3);
       sil = i_disable(INTCLASS2);
       p_dds->wrk.adap_state = RESET_PHASE0;
       ++p_dds->wrk.reset_retry;
       i_enable(sil);
       rc = reset_adap(p_dds, p_owe);

   }   /* end if more reset retrys left */
}   /* end if no more reset spin time */
else
{  /* we have more reset spin time to go */
   sil = i_disable(INTCLASS2);
   ++p_dds->wrk.reset_spin;
   i_enable(sil);

   p_dds->wrk.p_bringup_timer->timeout.it_value.tv_sec
                = (RESET_SPIN_DELTA / 1000 );
   p_dds->wrk.p_bringup_timer->timeout.it_value.tv_nsec
                = (RESET_SPIN_DELTA % 1000) * 1000000;
   p_dds->wrk.time_data.owe.who_queued = OFFL_ADAP_BRINGUP;
   p_dds->wrk.adap_state = RESET_PHASE2;
   p_dds->wrk.time_data.run = TRUE;
   tstart(p_dds->wrk.p_bringup_timer);
}  /* end more spin time left */

return(rc);
}  /* end function reset_phase2() */


/*--------------------------------------------------------------------*/
/***************        Initialize Adapter              ***************/
/*--------------------------------------------------------------------*/

/*
This routine will give the Token-Ring adapter the initialization parameters.
This routine assumes that the adapter has just been reset.

After successful completion of this routine, the adapter will be in a state
awaiting the open adapter command and the adapter open options.

RETURN CODES:
       0 - Successful completion
       ENETUNREACH - Adapter initialization failed
       rc  - Return code from the reset_adap() function.  Returning rc means
             that the resetting of the adapter failed.


ROUTINES CALLED:
       - reset_adap()


RETRY LOGIC:

       This function will retry the adapter initialization sequence
       ADAP_INIT_RETRY times.
*/

int init_adap(dds_t *p_dds, offl_elem_t *p_owe)

{  /* begin function init_adap */

   unsigned int rc=0;


   DEBUGTRACE2("inta", p_dds->wrk.adap_state);
switch (p_dds->wrk.adap_state)
{
   case ADAP_INIT_PHASE0:
   {
       rc = init_adap_phase0(p_dds, p_owe);
       break;
   }   /* end case ADAP_INIT_PHASE0 */
   case ADAP_INIT_PHASE1:
   {
       rc = init_adap_phase1(p_dds, p_owe);
       break;
   }   /* end case ADAP_INIT_PHASE1 */
   default:
       /*
       *   FUTURE FIX:
       *       Add logic to handle this fall through case
       */
        TRACE2("FooT", ACT_INIT_0);

       break;
}  /* end switch */

return(rc);
}  /* end function init_adap() */

/*---------------------------------------------------------------------*/
/*                         INIT ADAP PHASE 0                           */
/*---------------------------------------------------------------------*/

/*
*  FUNCTION: init_adap_phase0()
*
*  INPUT:  p_dds - pointer to DDS
*          p_owe - pointer to OFLV work element
*/
int init_adap_phase0(dds_t *p_dds, offl_elem_t *p_owe)
{  /* begin function */
   int j=0, error=FALSE;
        int     pio_attachment;
        int     iocc_attachment;
   unsigned short *p_adap_i_parms, t1;
   unsigned int rc, sil;
   unsigned char pos4=0;


       /*
       *   Get the current POS 4 Setting.
       *   Turn On Parity.
       */
       iocc_attachment = attach_iocc(p_dds);
       pos4 = pio_read( p_dds, POS_REG_4);
       pos4 = pos4 | MC_PARITY_ON;
       pio_write( p_dds, POS_REG_4, pos4);
       detach_iocc(p_dds, iocc_attachment);

   /* set up pointer to the adapter registers */
       p_adap_i_parms = (unsigned short *)
                    &(p_dds->wrk.adap_iparms.init_options);

       j=11;

        pio_attachment = attach_bus(p_dds);
        pio_write( p_dds, ADDRESS_REG, 0x0200 );

       while (j--)     /* Load adapter init. paramters */
       {
            pio_write( p_dds, AUTOINCR_REG, *p_adap_i_parms );
           ++p_adap_i_parms;
       } /* endwhile */

   /* read back the init parameters to verify they got there ok. */
   /* set up pointer to the adapter registers */
       p_adap_i_parms = (unsigned short *)
                    &(p_dds->wrk.adap_iparms.init_options);

       j=11;
        pio_write( p_dds, ADDRESS_REG, 0x0200 );
       while ( (j--) && !error)
       {
           if ( (t1 = pio_read( p_dds, AUTOINCR_REG ))
                    !=  *p_adap_i_parms)
           {
               error = TRUE;           /* init. parms did not get there ok */
           }
           else
               ++p_adap_i_parms;
       } /* end while */

       if (!error)
       {                  /* adap. init. parameters got there ok. */
       /* issue WRITE INTERRUPT Instruction (Execute) to adapter */
            pio_write( p_dds, COMMAND_REG, EXECUTE );
            detach_bus( p_dds, pio_attachment);

           p_dds->wrk.p_bringup_timer->timeout.it_value.tv_sec =
                                                (ADAP_INIT_SPIN_DELTA / 1000);

           p_dds->wrk.p_bringup_timer->timeout.it_value.tv_nsec
                        = (ADAP_INIT_SPIN_DELTA % 1000) * 1000000;
           p_dds->wrk.time_data.owe.who_queued = OFFL_ADAP_BRINGUP;
           p_dds->wrk.time_data.owe.cmd = OFLV_ADAP_INIT;
           p_dds->wrk.adap_state = ADAP_INIT_PHASE1;
           p_dds->wrk.time_data.run = TRUE;
           tstart(p_dds->wrk.p_bringup_timer);
       }   /* end if !error */
       else
       {
           TRACE3("FooT", ACT_IPHS0_0, (ulong)t1 );
           p_dds->wrk.footprint = ACT_IPHS0_0;
           p_dds->wrk.afoot = t1;
           detach_bus( p_dds,pio_attachment);

           if (p_dds->wrk.limbo != PARADISE)
           {
               /*
               *   start things over
               */
               rc = cycle_limbo(p_dds);
           }
           else if (p_dds->wrk.adap_init_retry >= ADAP_INIT_RETRY)
           {
               /*
               *   This is the 1st CIO_START thread.
               *   Build start done status block and end the show
               */
               TRACE2("FooT", ACT_IPHS0_1);
               p_dds->wrk.start_done_blk.code =CIO_START_DONE;
               p_dds->wrk.start_done_blk.option[0] =
                                                 TOK_ADAP_INIT_PARMS_FAIL;

               p_dds->wrk.start_done_blk.option[1] = NULL;
               p_dds->wrk.start_done_blk.option[2] = NULL;
               p_dds->wrk.start_done_blk.option[3] = NULL;

               sil = i_disable(INTCLASS2);
               p_dds->wrk.adap_state = CLOSED_STATE;
               i_enable(sil);
      /*
       *  Log permanent hardware error and return
       */
       logerr(p_dds, ERRID_TOK_ERR5);

               conn_done(p_dds);

           }   /* end if no more adap init retrys left */
           else
           {
              /*
               *   kick off another retry sequence
               *
               *   NOTE:
               *   Reset the individual reset retry variables
               *   to 0 so as to allow the reset_adap() routine(s)
               *   to start with a fresh reset cycle.
               *
               *   We must change the adapter state to RESET_PHASE0
               *   This is done so as to not go into an infinint loop
               *   of recursion.
               */
               TRACE2("FooT", ACT_IPHS0_2);
               sil = i_disable(INTCLASS2);
               p_dds->wrk.reset_retry= 0;
               p_dds->wrk.reset_spin= 0;
               ++p_dds->wrk.adap_init_retry;
               p_dds->wrk.adap_state = RESET_PHASE0;
               i_enable(sil);
               rc = reset_adap(p_dds, p_owe);

           }

       }   /* end if error */


}  /* end function init_adap_phase0() */

/*---------------------------------------------------------------------*/
/*                         INIT ADAP PHASE 1                           */
/*---------------------------------------------------------------------*/

int init_adap_phase1(dds_t *p_dds, offl_elem_t *p_owe)
{  /* begin function */
   unsigned short rc=0;
   int     pio_attachment, sil;


        pio_attachment = attach_bus(p_dds);
        rc = pio_read( p_dds, STATUS_REG );
        detach_bus( p_dds, pio_attachment );

   if ((rc & 0x0070) == INIT_ADAP_OK)
   {
           /*
           *   Set next adap state to OPEN_PHASE0
           */
       sil = i_disable(INTCLASS2);
       p_dds->wrk.adap_state = OPEN_PHASE0;
       i_enable(sil);
       rc = open_adap(p_dds, p_owe);

   }
   else if ( (rc & INIT_ADAP_FAIL) == INIT_ADAP_FAIL)
   {
           /*
           *   Initialization of adapter failed
           *   Check if in Ring Recovery mode, if so set adap state
           *   to RESET_PHASE0 and start all over.
           *
           */

        p_dds->wrk.footprint = ACT_IPHS1_0;
        p_dds->wrk.afoot = rc;
        TRACE3("FooT", ACT_IPHS1_0, (ulong)rc );

       if (p_dds->wrk.limbo != PARADISE)
       {
               /*
               *   kick whole thing off again
               */
           rc = cycle_limbo(p_dds);
       }
              /*
               *   If this is in response to an activation request,
               *   see if there are any more Adap. init retrys left, if so
               *   set adap_state to RESET_PHASE0 and try again. If no more
               *   retrys are available, build START DONE status block and
               *   terminate the activation sequence.
               */
       else if (p_dds->wrk.adap_init_retry >= ADAP_INIT_RETRY)
       {
              /*
               *   Build START DONE status block
               */
           TRACE2("FooT", ACT_IPHS1_1);
           p_dds->wrk.start_done_blk.code =CIO_START_DONE;
           p_dds->wrk.start_done_blk.option[0] =
                                                     TOK_ADAP_INIT_FAIL;

           p_dds->wrk.start_done_blk.option[1] = NULL;
           p_dds->wrk.start_done_blk.option[2] = rc;
           p_dds->wrk.start_done_blk.option[3] = NULL;

           sil = i_disable(INTCLASS2);
           p_dds->wrk.adap_state = CLOSED_STATE;
           i_enable(sil);

      /*
       *  Log permanent hardware error and return
       */
       logerr(p_dds, ERRID_TOK_ERR5);


           conn_done(p_dds);
           rc = ENETUNREACH;

       }   /* end if no more adap init retrys */
       else
       {
              /*
               *   Reset the individual reset retry variables
               *   to 0 so as to allow the reset_adap() routine(s)
               *   to start with a fresh reset cycle.
               *
               *   Set adap_state to RESET_PHASE0.
               *   Kick off another retry sequence by calling
               *   the reset_adap() routine.
               */
           TRACE2("FooT", ACT_IPHS1_2);
           sil = i_disable(INTCLASS2);
           p_dds->wrk.reset_retry= 0;
           p_dds->wrk.reset_spin= 0;
           ++p_dds->wrk.adap_init_retry;
           p_dds->wrk.adap_init_spin = 0;
           p_dds->wrk.adap_state = RESET_PHASE0;
           i_enable(sil);

           rc = reset_adap(p_dds, p_owe);

       }   /* end if more adap init retrys avail */
   }
   else if (p_dds->wrk.adap_init_spin >= ADAP_INIT_SPIN_TIME)
   {   /* no more spin time */
           /*
           *   Check if there is more Adapter Init. spin
           *   time left.  If so set timer, and
           *   adap_state=ADAP_INIT_PHASE1
           *
           *   If no spin time left,
           *       - check if in Ring Recovery Mode, if so set state
           *         to RESET_PHASE0 and set the timer.
           *       - otherwise, see if more adap. init retrys left,
           *         if so start over from RESET_PHASE0
           *         if not, build START DONE status block.
           *
           */

           TRACE2("FooT", ACT_IPHS1_3);
        p_dds->wrk.footprint = ACT_IPHS1_3;
        p_dds->wrk.afoot = rc;

       if (p_dds->wrk.limbo != PARADISE)
       {
                   /*
                   *   Kick whole thing off again.
                   */
               rc = cycle_limbo(p_dds);
       }
       else if (p_dds->wrk.adap_init_retry >= ADAP_INIT_RETRY)
       {   /* no more adap init retrys */
                      /*
                       *   This is in response to the activation
                       *   thread.
                       *
                       *   Build START DONE status block
                       */
           TRACE2("FooT", ACT_IPHS1_4);
               p_dds->wrk.start_done_blk.code =CIO_START_DONE;

               p_dds->wrk.start_done_blk.option[0] = TOK_ADAP_INIT_TIMEOUT;

               p_dds->wrk.start_done_blk.option[1] = NULL;

               p_dds->wrk.start_done_blk.option[2] = rc;

               p_dds->wrk.start_done_blk.option[3] = NULL;

               p_dds->wrk.adap_state = CLOSED_STATE;

      /*
       *  Log permanent hardware error and return
       */
       logerr(p_dds, ERRID_TOK_ERR5);

               conn_done(p_dds);

               rc = ENETUNREACH;

       }   /* end if no more adap init retrys */
       else
       {

              /*
               *   Reset the individual reset retry variables
               *   to 0 so as to allow the reset_adap() routine(s)
               *   to start with a fresh reset cycle.
               *
               *   Set adap_state to RESET_PHASE0.
               *   Kick off another retry sequence by calling
               *   the reset_adap() routine.
               */
           TRACE2("FooT", ACT_IPHS1_5);
                   sil = i_disable(INTCLASS2);
                   p_dds->wrk.reset_retry= 0;
                   p_dds->wrk.reset_spin= 0;
                   ++p_dds->wrk.adap_init_retry;
                   p_dds->wrk.adap_init_spin = 0;
                   p_dds->wrk.adap_state = RESET_PHASE0;
                   i_enable(sil);

                   rc = reset_adap(p_dds, p_owe);

       }   /* end if more adap init retrys available */
   }   /* end if no more spin time */
   else
   {   /* we have more adap init spin time */



       p_dds->wrk.p_bringup_timer->timeout.it_value.tv_sec =
                                                (ADAP_INIT_SPIN_DELTA / 1000);
       p_dds->wrk.p_bringup_timer->timeout.it_value.tv_nsec
                        = (ADAP_INIT_SPIN_DELTA % 1000) * 1000000;

       p_dds->wrk.time_data.owe.who_queued = OFFL_ADAP_BRINGUP;

       p_dds->wrk.time_data.owe.cmd = OFLV_ADAP_INIT;


       p_dds->wrk.time_data.run = TRUE;
       sil = i_disable(INTCLASS2);
       ++p_dds->wrk.adap_init_spin;
       i_enable(sil);
       tstart(p_dds->wrk.p_bringup_timer);

       rc = 0;

   }   /* end if we have more adap init spin time. */

}  /* end function init_adap_phase1() */

/*--------------------------------------------------------------------*/
/***************        Open  Adapter                   ***************/
/*--------------------------------------------------------------------*/

/*
This routine will issue an Open command to the adapter.  This will cause the
adapter to insert onto the Token-Ring.  The adapter open command usually takes
from 15 to 30 seconds to complete.  An adapter open command timeout of
100*OPEN_SPIN_TIME is used.

RETURN CODES:
       ENOBUFS     - Unable to get memory for the adapter open options.
       EIO         - d_complete of the adapter open options failed.
       ENETUNREACH - open of the adapter failed.
       rc          - Return code from the init_adap() function.  Returning rc
                     means that the initialization of the adapter failed.


ROUTINES CALLED:
       - xmalloc(), xmfree()
       - d_complete()
       - d_kmove()
       - i_enable(), i_disable()
       - delay()
       - reset_adap()
       - init_adap()

*/


int open_adap(dds_t *p_dds, offl_elem_t *p_owe)
{  /* begin function */

int rc=0;

   DEBUGTRACE2("opna", p_dds->wrk.adap_state);
   switch(p_dds->wrk.adap_state)
   {
       case OPEN_PHASE0:
       {
           rc = open_adap_phase0(p_dds, p_owe);
           break;
       }
       case OPEN_PENDING:
       {
           rc = open_adap_pend(p_dds, p_owe);
           break;
       }
       default:
           TRACE2("FooT", ACT_OPEN_0);
           break;
   }   /* end switch */

return(rc);
}  /* end function open_adap() */

/*---------------------------------------------------------------------*/
/*                         OPEN ADAP PHASE 0                           */
/*---------------------------------------------------------------------*/

/*
*
*  FUNCTION:   open_adap_phase0()
*
*  INPUT:  p_dds - pointer to DDS
*          p_owe - pointer to OFLV work element
*
*/

int open_adap_phase0 (
        dds_t *p_dds,
        offl_elem_t *p_owe)
{
   int sil, rc;
/* ----------------------------------------------------------------- */
  /*
   *   Move the adapter open options to the ACA via the d_kmove()
   *   kernel service, then issue a d_complete().
   *
   */
   /*
   *   FUTURE FIX:
   *       Add error recovery logic
   *       for the d_kmove()
   */

        rc = d_kmove (&(p_dds->wrk.adap_open_opts),
                 p_dds->wrk.p_d_open_opts,
                 sizeof(tok_adap_open_options_t),
                 p_dds->wrk.dma_chnl_id,
                 p_dds->ddi.bus_id,
                 DMA_WRITE_ONLY);

	if (rc == EINVAL) 	/* IOCC is NOT buffered */
	   bcopy(&(p_dds->wrk.adap_open_opts),
              p_dds->wrk.p_open_opts,
              sizeof(tok_adap_open_options_t));

        p_dds->wrk.time_data.owe.who_queued = OFFL_ADAP_BRINGUP;
        p_dds->wrk.time_data.owe.cmd = OFLV_ADAP_OPEN;
        p_dds->wrk.p_bringup_timer->timeout.it_value.tv_sec
                = OPEN_TIMEOUT /1000;
        p_dds->wrk.p_bringup_timer->timeout.it_value.tv_nsec
                = (OPEN_TIMEOUT % 1000) * 1000000;

        /* set state of the adapter open sequence */
       sil = i_disable(INTCLASS2);
       p_dds->wrk.adap_state = OPEN_PENDING;
       i_enable(sil);
        /*                                                                */
        /*  Issue the adapter open command to the adapter with the open   */
        /*  options in the DDS.                                           */
        /*                                                                */
        issue_scb_command(p_dds, ADAP_OPEN_CMD,
                        p_dds->wrk.p_d_open_opts);

        p_dds->wrk.time_data.run = TRUE;
        tstart(p_dds->wrk.p_bringup_timer);

        return(0);
} /* end open_adap_phase0() */

/*---------------------------------------------------------------------*/
/*                         OPEN ADAP PEND                              */
/*---------------------------------------------------------------------*/
/*
*  FUNCTION: open_adap_pend()
*
*  INPUT:  p_dds - pointer to DDS
*          p_owe - pointer to OFLV work element
*
*
*/

int
open_adap_pend(dds_t *p_dds, offl_elem_t *p_owe)
{
	unsigned int    rc=0, i, ope=0, sil;
	unsigned char   *addr;
	unsigned long   errid;

	/*
	 *   Cancel the open timeout timer
	 */

	sil = i_disable(INTCLASS2);
	if (p_dds->wrk.time_data.run)
		tstop(p_dds->wrk.p_bringup_timer);

	if ( (p_dds->wrk.adap_doa == DEAD_ON_ARRIVAL) ||
	     (p_owe->stat0 != 0x8000) )
	{ 
		TRACE4("FooT", ACT_OA_PEND_1, (ulong)p_dds->wrk.adap_doa,
			(ulong)p_owe->stat0);
		/* shows bad state of OPEN */
		p_dds->wrk.adap_state = CLOSED_STATE;
		p_dds->wrk.adap_doa = ALIVE_N_WELL; /* put us back on course */

		/* save the status 1/2 word that was returned */
		/* by the adapter */
		p_dds->wrk.open_fail_code = p_owe->stat0;
		p_dds->wrk.afoot = p_owe->stat0;
		p_dds->wrk.footprint = ACT_OA_PEND_1;

		if (p_dds->wrk.limbo != PARADISE)
		{
			/*
			 *   Almost! But no cigar...back into Limbo
			 *
			 */
			rc = cycle_limbo(p_dds);
		}
		else 
		{       /* Send Back good ack to caller */
			p_dds->wrk.adap_state = OPEN_STATE;
			p_dds->wrk.rs_bcon_cnt = 0;
			p_dds->wrk.rs_eserr_cnt = 0;
	
			addr = (uchar *)
				(&p_dds->wrk.adap_open_opts.node_addr1);
			for (i = 0; i < 6; i++)
				p_dds->wrk.tok_addr[i] = addr[i];

			/*
			 *  build the Start Done status block.
			 */
			p_dds->wrk.start_done_blk.code =CIO_START_DONE;
			p_dds->wrk.start_done_blk.option[0] = CIO_OK;

			/*
			 * move the network address in use to the start done
			 * block save area 
			 */
			bcopy (&(p_dds->wrk.tok_addr[0]),
			       &(p_dds->wrk.start_done_blk.option[2]),
			       TOK_NADR_LENGTH);

			rc = tok_recv_start(p_dds);

			rc = get_adap_point(p_dds);

			conn_done(p_dds);

			enter_limbo(p_dds, TOK_ADAP_INIT_FAIL, p_owe->stat0);
		}
	}   /* end if open adap cmd failed */
	else
	{       /* Open Was Successful */

		p_dds->wrk.adap_state = OPEN_STATE;
		p_dds->wrk.rs_bcon_cnt = 0;
		p_dds->wrk.rs_eserr_cnt = 0;

		addr = (unsigned char *)(&p_dds->wrk.adap_open_opts.node_addr1);
		for (i = 0; i < 6; i++)
			p_dds->wrk.tok_addr[i] = addr[i];

		if (p_dds->wrk.limbo==CHAOS)
		{
			/*
			 *   We're Open for business.
			 *   Start the exodus from limbo mode.
			 */
			TRACE2("FooT", ACT_OA_PEND_0);
			rc = egress_limbo(p_dds);
		}
		else
		{
			/*
			 *  build the Start Done status block.
			 */
			p_dds->wrk.start_done_blk.code =CIO_START_DONE;
			p_dds->wrk.start_done_blk.option[0] = CIO_OK;

			/*
			 * move the network address in use to the start done
			 * block save area 
			 */
			bcopy (&(p_dds->wrk.tok_addr[0]),
			       &(p_dds->wrk.start_done_blk.option[2]),
			       TOK_NADR_LENGTH);

			rc = tok_recv_start(p_dds);

			/*
			 *   Get the adapter's pointers
			 */
			rc = get_adap_point(p_dds);

			conn_done(p_dds);
		}   /* end if not comming out of limbo mode */

	}

	i_enable(sil);
	return(rc);
}  /* end function open_adap_pend() */

/*---------------------------------------------------------------------*/
/*                         OPEN TIMEOUT                                */
/*---------------------------------------------------------------------*/
/*
*  FUNCTION:   open_timeout()
*
*  INPUT:      p_dds - pointer to DDS
*              p_owe - pointer to OFLV Work Element
*
*/

int open_timeout( dds_t *p_dds, offl_elem_t *p_owe)
{  /* begin function */

  unsigned int rc = 0, sil;

if (p_owe->cmd == OFLV_ADAP_OPEN)
{      /* the adapter open timed out */
   sil = i_disable(INTCLASS2);
   p_dds->wrk.adap_state = CLOSED_STATE;
   i_enable(sil);
   p_dds->wrk.footprint = ACT_OA_TO_0;

  TRACE2("FooT", ACT_OA_TO_0);
   if (p_dds->wrk.limbo != PARADISE)
   {
      /*
       *   Not quite...try again.
       */
       rc = cycle_limbo(p_dds);
   }
   else if ( p_dds->wrk.adap_open_retry >= OPEN_MAX_RETRY)
   {
      /*
       *   Build START DONE status block
       *   option[1] will be filled in with the netid
       *   when the ds_startblk() routine is called.
       */
        TRACE2("FooT", ACT_OA_TO_1);
       p_dds->wrk.start_done_blk.code =CIO_START_DONE;
       p_dds->wrk.start_done_blk.option[0] = CIO_TIMEOUT;

       p_dds->wrk.start_done_blk.option[1] = NULL;
       p_dds->wrk.start_done_blk.option[2] = p_owe->stat0;
       p_dds->wrk.start_done_blk.option[3] = NULL;

      /*
       *  Log Permanent hardware error and return
       */
       logerr(p_dds, ERRID_TOK_ERR5);

       conn_done(p_dds);
       rc = ENETUNREACH;

   }   /* end if no more open retrys avail */
   else
  {
              /*
               *   We have more open retrys available
               *
               *   Reset the individual reset retry variables
               *   to 0 so as to allow the reset_adap() routine(s)
               *   to start with a fresh reset cycle.
               *
               *   Also reset the individual adap. init retry
               *   variables so as to allow the init_adap()
               *   routines to start with a fresh adap init
               *   cycle.
               *
               *   Set adap_state to RESET_PHASE0.
               *   Kick off another retry sequence by calling
               *   the reset_adap() routine.
               */
        TRACE2("FooT", ACT_OA_TO_2);
       sil = i_disable(INTCLASS2);
       p_dds->wrk.reset_retry= 0;
       p_dds->wrk.reset_spin= 0;
       p_dds->wrk.adap_init_retry = 0;
       p_dds->wrk.adap_init_spin = 0;
       ++p_dds->wrk.adap_open_retry;
       p_dds->wrk.adap_state = RESET_PHASE0;
       i_enable(sil);

       rc = reset_adap(p_dds, p_owe);


   }   /* end if more open retrys avail */
}  /* end if OFLV_ADAP_OPEN */

} /* end function open_timeout() */


/*--------------------------------------------------------------------*/
/***************        Close  Adapter                  ***************/
/*--------------------------------------------------------------------*/

int close_adap(dds_t *p_dds)
{
	int 		sil;
        volatile t_ssb	ssb;            /* system status block */
        int             i=0,rc;

	/*
	 *   NOTE:
	 *       The system close call CANNOT FAIL.
	 *       Take the necessary steps to accomplish this
	 *       no matter what.
	 *   If the adapter bringup timer is currently running, STOP IT!
	 */

	sil = i_disable(INTCLASS2);

	if (p_dds->wrk.time_data.run)
		tstop(p_dds->wrk.p_bringup_timer);

	p_dds->wrk.time_data.owe.who_queued = OFFL_ADAP_BRINGUP;
	p_dds->wrk.time_data.owe.cmd = OFLV_ADAP_CLOSE;
	p_dds->wrk.p_bringup_timer->timeout.it_value.tv_sec
        	= CLOSE_TIMEOUT /1000;
	p_dds->wrk.p_bringup_timer->timeout.it_value.tv_nsec
        	= (CLOSE_TIMEOUT % 1000) * 1000000;

	p_dds->wrk.adap_state = CLOSE_PENDING;

	issue_scb_command(p_dds, ADAP_CLOSE_CMD, NULL);

	p_dds->wrk.close_event = EVENT_NULL;
	p_dds->wrk.time_data.run = TRUE;
	tstart(p_dds->wrk.p_bringup_timer);
	/* following sleep leaves interrupts enabled for rest of system */
	SLEEP( &p_dds->wrk.close_event );

	i_enable(sil);

        /* 
         *  Loop reading the value of the SSB until the adapter closed bit
	 *  is set or one tenth second has passed.
	 *
	 *  Since the adapter is using DMA to update the SSB, it is necessary
         *  to d_kmove the image through the IOCC cache into system memory
         *  so that both the cache and memory have the same SSB image.
         */

	do {
        	rc = d_kmove (&ssb,
			WRK.p_d_ssb,
			sizeof(ssb),
			WRK.dma_chnl_id,
			DDI.bus_id,
			DMA_READ);
		if (rc == EINVAL) 	/* IOCC is NOT buffered */
			bcopy (WRK.p_ssb,
				&ssb,
				sizeof(ssb));

		if (ssb.stat0 & 0x8000)
			break;

		delay (HZ/100);

	} while ( ++i < HZ/10 );

	return(0); 

}  /* end function close adapter */

/*
* NAME: ds_deact
*
* FUNCTION:
*
*  Adapter de-activation routine.
*  This function is call on the very last close of the device handler.
*  It will de-activate the adapter if needed.
*
* EXECUTION ENVIRONMENT:
*
*       This routine executes on the process thread.
*
* NOTES:
*      This functions determines if we are in connected state (i.e. the
*      CIO_START has already completed successfully) or a connection
*      in progress state (i.e. the CIO_START has not finished asynchronously
*      yet).
*
*      If in the connected state (device_state = DEVICE_CONNECTED), check
*      first if we are in limbo, if so kill_limbo() and clean up. If not
*      in limbo, issue close_adap() to clean up.
*
*      If in the connection in progress state (device_state =
*      DEVICE_CONN_IN_PROG ), determine that exact adap_state and take
*      appropriate action.
*
*      The last thing that is done is to reset the POS registers
*      to the configuration values.
*
* (RECOVERY OPERATION:) Information describing both hardware and
*       software error recovery.
*
* (DATA STRUCTURES:) Effects on global data structures, similar to NOTES.
*
* RETURNS:
*      None.
*/
int ds_deact (dds_t *p_dds)
{
   int i, rc=0, pio_attachment;
   int sil;                    /* saved interrupt level */

   DEBUGTRACE2 ("dacB", (ulong)p_dds);

   /*
   *   disable interrupts so as to freeze the
   *   the state variables
   */
sil = i_disable(INTCLASS2);

if ( (p_dds->wrk.limbo == PARADISE) &&
     (p_dds->wrk.adap_state == OPEN_STATE) )
{  /* Up and running */
       /*
       *   FUTURE FIX:
       *
       *       Also add logic to stop any timers that
       *       might be running...also wakeup any process
       *       that might be sleeping?
       *
       *       Also add logic for when we are not in limbo
       *       but we have shut the adapter down due to the limbo
       *       entry thresholds being exeeded.
       */
       i_enable(sil);
       rc = close_adap(p_dds);

}  /* end up and running */
else if ( (p_dds->wrk.limbo == PARADISE) &&
          ( (p_dds->wrk.adap_state != DEAD_STATE) &&
            (p_dds->wrk.adap_state != NULL_STATE) ) )

{  /* in middle of activation */
   TRACE5("FooT", (ulong)ACT_DEACT_0, (ulong)p_dds->wrk.limbo, 
	(ulong)p_dds->wrk.adap_state, (ulong)p_dds->cio.device_state);
       i_enable(sil);
       rc = kill_act(p_dds);       /* Stop the activation */

}  /* end if middle of activation */
else if ( (p_dds->wrk.limbo == NO_OP_STATE) &&
          (p_dds->wrk.adap_state == NULL_STATE) )
{  /* we've had a fatal error */
	/* at this point there is nothing
	 * to do to shut down the device.
	 */
   TRACE5("FooT", (ulong)ACT_DEACT_1, (ulong)p_dds->wrk.limbo, 
	(ulong)p_dds->wrk.adap_state, (ulong)p_dds->cio.device_state);
	i_enable(sil);

}  /* end if fatal error occured */
else if ( (p_dds->wrk.limbo != PARADISE) &&
          (p_dds->wrk.limbo != NO_OP_STATE) )
{  /* we're in some phase of limbo */
      i_enable(sil);
      /*
      *   Red Zone!
      *   We are in some phase of limbo. Kill
      *   the recovery effort.
      */
   TRACE5("FooT", (ulong)ACT_DEACT_2, (ulong)p_dds->wrk.limbo, 
	(ulong)p_dds->wrk.adap_state, (ulong)p_dds->cio.device_state);
       rc = kill_limbo( p_dds );

}  /* end if in limbo */
else
{  /* Do nothing */
   /*
   *   We were never activated. There's nothing to undo
   */
   TRACE5("FooT", (ulong)ACT_DEACT_3, (ulong)p_dds->wrk.limbo, 
	(ulong)p_dds->wrk.adap_state, (ulong)p_dds->cio.device_state);

   i_enable(sil);
}  /* end do nothing case */



   /*                                                             */
   /*  Restore the POS registers to their values at config time.  */
   /*  Ignore POS 0 and 1 since they are read only.               */
   /*                                                             */
   pio_attachment = attach_iocc( p_dds );
   for (i = 2; i < 8; i++)
        pio_write(p_dds, POS_REG_0 + i, p_dds->wrk.cfg_pos[i]);
   detach_iocc( p_dds, pio_attachment );
   DEBUGTRACE1 ("dacE");
   return (rc);

} /* end ds_deact */



/*
* NAME: kill_act
*
* FUNCTION:
*
*  Terminates the activation of the adapter.
*
* EXECUTION ENVIRONMENT:
*
*      This routine executes on the process thread or on
*      the OFLV.
*
* NOTES:
*      This functions determines the current adap_state of the activation
*      sequence.  Once the adap_state is know, action is taken to un-do
*      whatever is necessary to queit the adapter and bring the adapter to
*      the initial state.
*
* (RECOVERY OPERATION:) Information describing both hardware and
*       software error recovery.
*
* (DATA STRUCTURES:) Effects on global data structures, similar to NOTES.
*
* RETURNS:
*      None.
*/
int
kill_act (dds_t *p_dds)
{
   int  rc=0, pio_attachment;
   int sil, adap_state;                    /* saved interrupt level */
    int		iocc_attachment;
    uchar	pos5;


   DEBUGTRACE2 ("kacB", (ulong)p_dds->wrk.adap_state);
   sil = i_disable(INTCLASS2);
    
	/* Save the current adapter state.
	 * Set the adapter state to ADAP_KILL_PENDING to
	 * indicate that we are in the process of killing 
	 * the adapter.
	 */

	adap_state = p_dds->wrk.adap_state;
	p_dds->wrk.adap_state = ADAP_KILL_PENDING;

       /*
       *   Take appropriate action.  This depends on
       *   the current adapter state.
       */
           switch (adap_state)
           {   /* what is our current adapter state? */
               case RESET_PHASE0:
               case RESET_PHASE1:
               case RESET_PHASE2:
               {
                   i_enable(sil);
		   
                   tstop( p_dds->wrk.p_bringup_timer );
                   pio_attachment = attach_bus( p_dds );
                   /*
                   * Disable adapter interrupts
                   */
                   if (!p_dds->wrk.mask_int)
                   {
                       pio_write( p_dds, IMASK_DISABLE, 0x00);
                       sil = i_disable(INTCLASS2);
                       p_dds->wrk.mask_int = TRUE;
                       i_enable(sil);
                   }

		   /*
 		   *  Get the current POS 5 Setting and disable DMA arbitration
		   *  There is a timing window if the adapter is doing DMA when
		   *  the reset occurs which could cause a bus timeout.
 		   */
           	   iocc_attachment = attach_iocc(p_dds);
		   pos5 = pio_read(p_dds, POS_REG_5);
		   pos5 = pos5 | MC_ARBITRATION;
		   pio_write( p_dds, POS_REG_5, pos5);
		   detach_iocc(p_dds, iocc_attachment);
		   delay(HZ);

                   /*
                   *   Reset the adapter
                   */

                   pio_write( p_dds, RESET_REG, 0x00 );
                   delay(HZ);
                   pio_write(p_dds, ENABLE_REG, 0x00);
                   delay(HZ);

		   /*
 		   *   Get the current POS 5 Setting.
 		   *   Allow DMA arbitration
 		   */
           	   iocc_attachment = attach_iocc(p_dds);
		   pos5 = pio_read(p_dds, POS_REG_5);
		   pos5 = pos5 & ~(MC_ARBITRATION);
		   pio_write( p_dds, POS_REG_5, pos5);
		   detach_iocc(p_dds, iocc_attachment);

                   /*
                   * Enable adapter interrupts
                   */
                   if (p_dds->wrk.mask_int)
                   {
                       pio_write( p_dds, IMASK_ENABLE, 0x00);
                       sil = i_disable(INTCLASS2);
                       p_dds->wrk.mask_int = FALSE;
                       i_enable(sil);
                   }
                   detach_bus( p_dds, pio_attachment );
                   break;
               }   /* end RESET cases */
               case ADAP_INIT_PHASE0:
               case ADAP_INIT_PHASE1:
               case OPEN_PHASE0:
               case CLOSED_STATE:
               {
                   /*
                   *   Do nothing..
                   *   FUTURE FIX:
                   *       May want to unmask interrupts or
                   *       acknowledge an interrupt. Research this
                   */
                   i_enable(sil);
                   tstop( p_dds->wrk.p_bringup_timer );
                   break;
               } /* end  ADAP INIT cases on 1st OPEN case */
               default:
               {   /* begin open pending case */
                  /*
                   *   Red Zone!
                   *   This is a very critical point, we are
                   *   in the process of shutting down the device AND
                   *   the adapter is in the process of comming all
                   *   the way up.  We must stop the adapter from
                   *   comming up.  We must also prevent ourselves
                   *   from handling the Open Completion interrupt
                   *   if we were too late in stopping it.
                   *
                   *   We will issue a reset to the adapter so
                   *   as to prevent any open completion interrupt
                   *   from being generated by the adapter.
                   *
                   *   FUTURE FIX:
                   *       Look into flushing out the OFLV work
                   *       queue.
                   */
                   i_enable(sil);
                   tstop( p_dds->wrk.p_bringup_timer );
                   pio_attachment = attach_bus( p_dds );

                  /*
                   * Disable adapter interrupts
                   */
                   if (!p_dds->wrk.mask_int)
                   {
                       pio_write( p_dds, IMASK_DISABLE, 0x00);
                       sil = i_disable(INTCLASS2);
                       p_dds->wrk.mask_int = TRUE;
                       i_enable(sil);
                   }

		   /*
 		   *  Get the current POS 5 Setting and disable DMA arbitration
		   *  There is a timing window if the adapter is doing DMA when
		   *  the reset occurs which could cause a bus timeout.
 		   */
           	   iocc_attachment = attach_iocc(p_dds);
		   pos5 = pio_read(p_dds, POS_REG_5);
		   pos5 = pos5 | MC_ARBITRATION;
		   pio_write( p_dds, POS_REG_5, pos5);
		   detach_iocc(p_dds, iocc_attachment);
                   delay(HZ);

                  /*
                   *   Reset the adapter
                   */

                   pio_write( p_dds, RESET_REG, 0x00 );
                   delay(HZ);
                   pio_write(p_dds, ENABLE_REG, 0x00);
                   delay(HZ);

		   /*
 		   *   Get the current POS 5 Setting.
 		   *   Allow DMA arbitration
 		   */
           	   iocc_attachment = attach_iocc(p_dds);
		   pos5 = pio_read(p_dds, POS_REG_5);
		   pos5 = pos5 & ~(MC_ARBITRATION);
		   pio_write( p_dds, POS_REG_5, pos5);
		   detach_iocc(p_dds, iocc_attachment);

                  /*
                   * Enable adapter interrupts
                   */
                   if (p_dds->wrk.mask_int)
                   {
                       pio_write( p_dds, IMASK_ENABLE, 0x00);
                       sil = i_disable(INTCLASS2);
                       p_dds->wrk.mask_int = FALSE;
                       i_enable(sil);
                   }



                   detach_bus( p_dds, pio_attachment );
                   break;
               }   /* end default ... open pending case */
           } /* endswitch on adapter state */

   sil = i_disable(INTCLASS2);
   p_dds->wrk.adap_state = CLOSED_STATE;
   i_enable(sil);

   DEBUGTRACE2 ("kacE", (ulong)p_dds->wrk.adap_state);
   return(rc);
}  /* end function kill_act() */


/*
 * NAME: kill_adap()
 *
 * FUNCTION:
 *      This function kill the adapter by issueing a soft
 *	reset to the adapter.
 *
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine executes ONLY on the interrupt level (SLIH).
 *
 * NOTES:
 * 
 *
 * RECOVERY OPERATION:
 *	None. 
 *
 * DATA STRUCTURES:
 *	Modifies the mask_int flag in the DDS.
 *	Sets the adap_doa=DEAD_ON_ARRIVAL flag in the DDS.
 *	Turns off parity in POS register 4.
 *
 * RETURNS: 
 */

int
kill_adap( dds_t *p_dds )
{ 	/* begin kill_adap() */

	int 	pio_attachment, sil;
	int	iocc_attachment;
	unsigned char pos4=0;
        uchar	pos5;
	int	i;	/* loop	control	*/
	int	tmp;	/* value read from delay reg (ignored)	*/
	int	delay_seg;

          /*
           *   Get the current POS 4 Setting.
           *   Turn Off Parity.
           */
	TRACE3("KILa", (ulong)p_dds, (ulong)p_dds->wrk.mask_int);
	iocc_attachment = attach_iocc(p_dds);
	pos4 = pio_read( p_dds, POS_REG_4);
	pos4 = pos4 & ~(MC_PARITY_ON);
	pio_write( p_dds, POS_REG_4, pos4);

	/*
 	*   Get the current POS 5 Setting and disable DMA arbitration
	*   There is a timing window if the adapter is doing DMA when the
	*   reset occurs which could cause a bus timeout.
	*
	*   DMA arbitration will not get turned back on until the "enable"
	*   register is written to later in error recovery.  Once the
	*   "reset" register is written to, the POS regs are unreadable
	*   until the "enable" register is written to.
 	*/
	pos5 = pio_read(p_dds, POS_REG_5);
	pos5 = pos5 | MC_ARBITRATION;
	pio_write( p_dds, POS_REG_5, pos5);
	detach_iocc(p_dds, iocc_attachment);

	/*
	 * ensure that there is no DMA going on
	 * wait 100 usec
	 */
	delay_seg = (uint)IOCC_ATT(p_dds->ddi.bus_id, DL_DELAY_REG ); 
	i = 0;
	while ( ++i < 100) 
		tmp = BUSIO_GETC(delay_seg); /* delay 1 microsecond */
	IOCC_DET(delay_seg);

	pio_attachment = attach_bus(p_dds);

          /*
           *   NOTE:
           *       If the adapter interrupts are not currently
           *       disabled, we need to disable them.  This is
           *       to compensate for the spurious interrupt
           *       generated by the Token-Ring adapter during
           *       the reset sequence.
           *
           */

	if (!p_dds->wrk.mask_int)
	{
		pio_write( p_dds, IMASK_DISABLE, 0x00 );
		sil = i_disable(INTCLASS2);
		p_dds->wrk.mask_int = TRUE;
		i_enable(sil);
	}

	pio_write( p_dds, RESET_REG, 0x00 );
	detach_bus( p_dds, pio_attachment );

	/*
	 * NOTE:
	 * Set the adapter killed by SLIH flag.
	 * Setting this flag will indicate to the key routines
	 * that make MAJOR operational direction changes that the
	 * adapter has been killed via reset underneath them.  MAJOR
	 * direction changes are:
	 *
	 *	1. going from the OPEN_PENDING to the
	 *	   OPEN_STATE in the 1st adapter activation thread.
	 *	
	 *	2. Going from the LIMBO_GROUP to the
	 *	   PARADISE state in the egress of limbo.
	 */

	p_dds->wrk.adap_doa = DEAD_ON_ARRIVAL;

	TRACE3("kilA", (ulong)p_dds, (ulong)pos4);
return(0);

} /* end kill_adap() */

/*
* NAME: cfg_adap_parms
*
* FUNCTION:
*
*  Resets all adapter parameters to the configuration values.
*
* EXECUTION ENVIRONMENT:
*
*      This routine executes on the process thread.
*
* NOTES:
*      This functions will reset the work variables for the adapter
*      initialization and open options to the configuration settings (for
*      the options that are configurable) and the non-configurable items
*      to the default settings.
*
*      This function assumes that the VPD has already been succuessfully
*      read from the adapter.  It will access the VPD's burned-in-address.
*
* RECOVERY OPERATION:
*      None.
*
* (DATA STRUCTURES:) Effects on global data structures, similar to NOTES.
*
* RETURNS:
*      None.
*/
int
cfg_adap_parms( dds_t  *p_dds )
{

   /*
   *   Set the Work area adapter initialization and open options to
   *   what was passed in at config time.
   */
   p_dds->wrk.adap_iparms.init_options = DF_ADAP_INIT_OPTIONS;
   p_dds->wrk.adap_iparms.cmd = NULL;
   p_dds->wrk.adap_iparms.xmit = NULL;
   p_dds->wrk.adap_iparms.rcv = NULL;
   p_dds->wrk.adap_iparms.ring = NULL;
   p_dds->wrk.adap_iparms.scb_clear = NULL;
   p_dds->wrk.adap_iparms.adap_chk = NULL;
   p_dds->wrk.adap_iparms.rcv_burst_size = DF_RCV_BURST_SIZE;
   p_dds->wrk.adap_iparms.xmit_burst_size = DF_TX_BURST_SIZE;
   p_dds->wrk.adap_iparms.dma_abort_thresh = DF_DMA_ABORT_THRESH;
   p_dds->wrk.adap_iparms.scb_add1 = NULL;
   p_dds->wrk.adap_iparms.scb_add2 = NULL;
   p_dds->wrk.adap_iparms.ssb_add1 = NULL;
   p_dds->wrk.adap_iparms.ssb_add2 = NULL;


   p_dds->wrk.adap_open_opts.options= p_dds->ddi.open_options;
   p_dds->wrk.adap_open_opts.buf_size = p_dds->ddi.buf_size;

       p_dds->wrk.min_packet_len = TOK_MIN_PACKET;
   if (p_dds->ddi.ring_speed == TOK_4M)
   {   /* 4 Mbps data rate */
      /*
       *   Set the MAX packet size
       */
       p_dds->wrk.max_packet_len = TOK_4M_MAX_PACKET;

       if (p_dds->ddi.buf_size == ADAP_BUF_SIZE_512)
       {   /* 512 byte adapter buffer size */
           p_dds->wrk.adap_open_opts.xmit_buf_min_cnt =
                                               ADAP_4M_TX_BUF_MIN_CNT_512;
           p_dds->wrk.adap_open_opts.xmit_buf_max_cnt =
                                               ADAP_4M_TX_BUF_MAX_CNT_512;
       }   /* end if 512 byte adapter buffer size */
       else
       {
           p_dds->wrk.adap_open_opts.xmit_buf_min_cnt =
                                               ADAP_4M_TX_BUF_MIN_CNT_1K;
           p_dds->wrk.adap_open_opts.xmit_buf_max_cnt =
                                               ADAP_4M_TX_BUF_MAX_CNT_1K;
       }

   }   /* end if 4Mbps data rate */
   else
   {   /* 16 Mbps data rate */
      /*
       *   Set the MAX packet size
       */
       p_dds->wrk.max_packet_len = TOK_16M_MAX_PACKET;

       if (p_dds->ddi.buf_size == ADAP_BUF_SIZE_512)
       {   /* 512 byte adapter buffer size */
           p_dds->wrk.adap_open_opts.xmit_buf_min_cnt =
                                               ADAP_16M_TX_BUF_MIN_CNT_512;
           p_dds->wrk.adap_open_opts.xmit_buf_max_cnt =
                                               ADAP_16M_TX_BUF_MAX_CNT_512;
       }   /* end if 512 byte adapter buffer size */
       else
       {
           p_dds->wrk.adap_open_opts.xmit_buf_min_cnt =
                                               ADAP_16M_TX_BUF_MIN_CNT_1K;
           p_dds->wrk.adap_open_opts.xmit_buf_max_cnt =
                                               ADAP_16M_TX_BUF_MAX_CNT_1K;
       }
   }   /* end if 16 Mbps data rate */



   p_dds->wrk.adap_open_opts.grp_addr1  = 0;
   p_dds->wrk.adap_open_opts.grp_addr2  = 0;
   p_dds->wrk.adap_open_opts.func_addr1 = 0;
   p_dds->wrk.adap_open_opts.func_addr2 = 0;
   p_dds->wrk.adap_open_opts.rcv_list_size = 0x0e;
   p_dds->wrk.adap_open_opts.xmit_list_size = 0x1a;
   p_dds->wrk.adap_open_opts.res1 =  NULL;
   p_dds->wrk.adap_open_opts.res2 = NULL;
   p_dds->wrk.adap_open_opts.prod_id_addr1 = NULL;
   p_dds->wrk.adap_open_opts.prod_id_addr2 = NULL;

   /*
   *   Check if we need to use the alternate network address.
   *   If so, set the open options accordingly.
   *   If not, plug in the burned in address from the VPD.
   */
   if (p_dds->ddi.use_alt_addr)
   {
       bcopy(&(p_dds->ddi.alt_addr[0]),
             &(p_dds->wrk.adap_open_opts.node_addr1),
             TOK_NADR_LENGTH);

       bcopy(&(p_dds->ddi.alt_addr[0]),
             &(p_dds->wrk.diag_open_opts.i_addr1),
             TOK_NADR_LENGTH);
   }
   else
   {
       bcopy(&(p_dds->wrk.tok_vpd_addr[0]),
             &(p_dds->wrk.adap_open_opts.node_addr1),
             TOK_NADR_LENGTH);

       bcopy(&(p_dds->wrk.tok_vpd_addr[0]),
             &(p_dds->wrk.diag_open_opts.i_addr1),
             TOK_NADR_LENGTH);
   } /* endif */




  /*
   *   Set the Work area adapter initialization and open options
   *   that may be set by a Diagnostic user to what is passed in
   *   at config time.
   */
   p_dds->wrk.diag_iparms.init_options =
                        p_dds->wrk.adap_iparms.init_options;

   p_dds->wrk.diag_iparms.rcv_burst_size =
                        p_dds->wrk.adap_iparms.rcv_burst_size;

   p_dds->wrk.diag_iparms.xmit_burst_size =
                        p_dds->wrk.adap_iparms.xmit_burst_size;

   p_dds->wrk.diag_iparms.dma_abort_thresh =
                        p_dds->wrk.adap_iparms.dma_abort_thresh;

   /* Now the open options */
   p_dds->wrk.diag_open_opts.options =
                        p_dds->wrk.adap_open_opts.options;

   p_dds->wrk.diag_open_opts.buf_size =
                        p_dds->wrk.adap_open_opts.buf_size;

   p_dds->wrk.diag_open_opts.xmit_buf_min_cnt =
                        p_dds->wrk.adap_open_opts.xmit_buf_min_cnt;

   p_dds->wrk.diag_open_opts.xmit_buf_max_cnt =
                        p_dds->wrk.adap_open_opts.xmit_buf_max_cnt;

   p_dds->wrk.diag_open_opts.i_addr1 =
                        p_dds->wrk.adap_open_opts.node_addr1;

   p_dds->wrk.diag_open_opts.i_addr2 =
                        p_dds->wrk.adap_open_opts.node_addr2;

   p_dds->wrk.diag_open_opts.i_addr3 =
                        p_dds->wrk.adap_open_opts.node_addr3;

}  /* end cfg_adap_parms */

/*
 * NAME: bug_out_cleanup()
 *
 * FUNCTION:
 *	This function will clean up after the bug_out() routine.
 *
 *
 * EXECUTION ENVIRONMENT:
 *	This routine runs on the process thread only.
 *
 * NOTES:
 * 
 *
 * RECOVERY OPERATION:
 *	None. 
 *
 * DATA STRUCTURES:
 *
 * RETURNS: 
 */

int
bug_out_cleanup( dds_t *p_dds )
{ /* begin bug_out_cleanup() */

	int rc=0, sil;

	/* determine what action was taken by 
	 * the bug_out() routine.  Take action to clean
	 * up after the bug_out() routine.
	 */
	switch ( p_dds->wrk.bugout )
	{
		case LMB_BUG_0:
		case LMB_BUG_6:
		case LMB_BUG_8:
		{
			(void)tx_undo(p_dds);

			(void)sb_undo(p_dds);

			(void)get_mem_undo(p_dds);

			break;
		} /* end case LMB_BUG_0, 6, 8 */
		case LMB_BUG_1:
		case LMB_BUG_2:
		case LMB_BUG_3:
		case LMB_BUG_4:
		case LMB_BUG_5:
		case LMB_BUG_7:
		default:
		{
			/* undo the receive block 
			 */
			rc = tok_recv_undo(p_dds);   

		/*
		 *  NOTE:
		 *  Must issue the d_complete() for the ACA before the call
		 *  to any TX undo routine.  The undo routines assume that the
		 *  ACA has been "un-hidden" via the d_complete() routine
		 *
		 *  Research what the state of the xmem descriptor is.
		 *  It may be set up incorrectly at this point
		 *
		 */

			rc = clean_tx(p_dds);

			sil = i_disable(INTCLASS2);
			p_dds->wrk.adap_state = NULL_STATE;
			i_enable(sil);

			rc = d_complete(p_dds->wrk.dma_chnl_id, DMA_READ,
					p_dds->wrk.p_mem_block, PAGESIZE,
					&(p_dds->wrk.mem_block_xmd),
					p_dds->wrk.p_d_mem_block);


			/* undo the transmit block 
			*/
			rc = tx_undo(p_dds); 

			/* undo the system block
			 */
			rc = sb_undo(p_dds);   

			/* Free control block 
			 */
			rc = get_mem_undo(p_dds);      

			break;
		}
	} /* end switch(bugout) */
	
	return(0);
} /* end bug_out_cleanup() */
