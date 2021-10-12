static char sccsid[] = "@(#)74	1.18  src/bos/kernext/tokdiag/toklimbo.c, diagddtok, bos411, 9428A410j 10/26/93 14:15:16";

/*
 * COMPONENT_NAME: (SYSXTOK) - Token-Ring device handler
 *
 * FUNCTIONS:  enter_limbo(), egress_limbo(), cycle_limbo(),
 *             kill_limbo(), bug_out()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
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
 * NAME: enter_limbo
 *
 * FUNCTION:
 *      Prepares the Token-Ring device driver for entering Network
 *      recovery mode (limbo for short).  This routine initiates
 *      the cleanup of the TX and RCV chains, logs the error,
 *      notifies all attached users via a asynchronous status
 *      block, and then initiates the re-cycling of the adapter.
 *
 *
 * EXECUTION ENVIRONMENT:
 *
 *     This routine executes on the OFLV.
 *
 * NOTES:
 *     The enter_limbo() function assumes that the limbo state variable
 *     has been checked prior to calling.  This routine should never
 *     be called if we are in some form of limbo.  It should only be
 *
 *     FIX COMMNETS!
 *
 *     called under the following conditions:
 *         1. adap_state = OPEN_STATE &&
 *            limbo = PARADISE
 *         2. A valid enter limbo condition has occurred.
 *
 * (RECOVERY OPERATION:) Information describing both hardware and
 *      software error recovery.
 *
 * (DATA STRUCTURES:) Effects on global data structures, similar to NOTES.
 *
 * RETURNS: What this code returns (NONE, if it has no return value)
 */

int
enter_limbo( dds_t *p_dds,             /* pointer to DDS */
                unsigned int reason,       /* reason for entering limbo */
                unsigned short ac )        /* Adapter code */
{
        int rc=0;
        offl_elem_t     owe;
        int sil;       /* saved interrupt level value */
	int ndx;
	open_elem_t	*open_ptr;


   TRACE4 ("elmB", (ulong)p_dds, (ulong)reason,
      (ulong)ac); /* enter_limbo begin */

  /*
   *   NOTE:
   *   This check is key.  We may be in the process of
   *   shutting down the adapter in response to the last
   *   close...OR we may be in the middle of downloading
   *   microcode...OR we may be in the middle of the 1st activation
   *   request (i.e. 1st CIO_START).
   *
   *   In either situation we want to make sure that if
   *   we get a limbo entry condition that we DO NOT
   *   enter limbo.
   */
sil = i_disable(INTCLASS2);

if(  (p_dds->wrk.limbo == PARADISE)
     &&              /* we are closed OR a close is pending */
     ( ( (p_dds->wrk.adap_state == CLOSE_PENDING) ||
         (p_dds->wrk.adap_state == CLOSED_STATE) ) 
       || 		/* we are in the middle of downloading microcode */
       ( (p_dds->wrk.adap_state == DOWNLOADING) )
       ||                          /* we are in middle of activation */
       ( (p_dds->wrk.adap_state != DEAD_STATE) &&
         (p_dds->wrk.adap_state != NULL_STATE) &&
         (p_dds->wrk.adap_state != OPEN_STATE) ) ) )
{
  /*
   *   Squeeezzze Play!
   *   We have a Limbo entry condition,
   *   BUT we have the last close pending OR 
   *   we are in the middle of downloading microcode OR
   *   we are in the middle of activating the adapter for the first time.
   *   Do nothing.
   *
   *   If the adapter close command never
   *   comes, the close failsafe timer will pop and let us
   *   exit gracefully.
   *
   *   If in the middle of activating the adapter for the 1st time,
   *   our timers will catch us and let us contiue/exit gracefully.
   */
	/* Squeeezzze Play! */
   TRACE3("lSQZ", (unsigned long) p_dds->wrk.adap_state, 
	          (unsigned long) p_dds->wrk.limbo );
}  /* end default else */
else if ( p_dds->wrk.limbo == PARADISE )
{
   /*
   *   It's safe to go into limbo
   *   But first, check the Soft & Hard Chaos counters
   *   to see if we've passed our limbo entry thresholds
   */
	/* Safe to go into Limbo */
   TRACE3("lSAF", (unsigned long) p_dds->wrk.adap_state, 
	          (unsigned long) p_dds->wrk.limbo );
   switch (reason)
   {   /* Soft or Hard entry condition */
       case TOK_CMD_FAIL:
       {   /*
           *   Soft entry reason...
           *       RCV Suspended condition
           *       TX failure condition
           *       Command Reject condition
           */
		
		/*
		 * FUTURE FIX:
		 *	The following logic will need to be
		 * 	uncommented when support for decrementing
		 *	the soft and hard chaos counters is implemented.
		 *	Until then, no limbo thresholding will be done.
		 */	
	  /*
           * if (p_dds->wrk.soft_chaos > SOFT_LIMBO_ABORT)
           * { 
	   *
	   * show's over 
	   *
           *    i_enable(sil);
           *    logerr(p_dds, ERRID_TOK_RCVRY_TERM);
	   *	
           *       Build the asynchronous status block
	   *
           *    p_dds->wrk.limbo_blk.code = CIO_ASYNC_STATUS;
           *    p_dds->wrk.limbo_blk.option[0] = CIO_HARD_FAIL;
           *    p_dds->wrk.limbo_blk.option[1] = reason;
           *    p_dds->wrk.limbo_blk.option[2] = ac;
           *    p_dds->wrk.limbo_blk.option[3] = NULL;
           *    async_status(p_dds, &p_dds->wrk.limbo_blk);
	   *
           *     * Exit...do not initiate limbo
           *     rc = bug_out(p_dds, reason, ac);
           *     return(0);
           * }
           * else
	   */
               /* increment limbo soft entry counter */
               ++p_dds->wrk.soft_chaos;
           break;
       }   /* end case TOK_CMD_FAIL */
       default :
       {   /* hard entry condition */
		/*
		 * FUTURE FIX:
		 *	The following logic will need to be
		 * 	uncommented when support for decrementing
		 *	the soft and hard chaos counters is implemented.
		 *	Until then, no limbo thresholding will be done.
		 */	
	/*
         *  if( p_dds->wrk.hard_chaos > HARD_LIMBO_ABORT)
         *  {   
	 * show is over 
         *
         *   Exceeded limbo hard error threshold
         *       Ring status ??
         *       Lobe wire fault
         *       Auto removal error
         *       Adapter check
         *       IMPL Force Mac Frame
         *
         *     i_enable(sil);
         *     logerr(p_dds, ERRID_TOK_RCVRY_TERM);
         *
         *       Build the asynchronous status block
         *
         *     p_dds->wrk.limbo_blk.code = CIO_ASYNC_STATUS;
         *     p_dds->wrk.limbo_blk.option[0] = CIO_HARD_FAIL;
         *     p_dds->wrk.limbo_blk.option[1] = reason;
         *     if ( reason == TOK_ADAP_CHECK )
         *         bcopy( &(p_dds->wrk.ac_blk), 
         *		  &(p_dds->wrk.limbo_blk.option[2]),
         *                sizeof(adap_check_blk_t) );
         *     else
         *     {
         *         p_dds->wrk.limbo_blk.option[2] = ac;
         *         p_dds->wrk.limbo_blk.option[3] = NULL;
         *     } 
	 * end if !TOK_ADAP_CHECK 
	 *
	 *
      	 *      async_status(p_dds, &p_dds->wrk.limbo_blk);
	 *
         * Exit...do not initiate limbo 
         *     rc = bug_out(p_dds, reason, ac);
         *     return(0);
         * }
         * else
	 */
               /* increment limbo hard entry counter */
               ++p_dds->wrk.hard_chaos;
           break;
       }   /* end default case */
   }   /* end switch */

      /*
       *   NOTE:
       *       At this point we have a go for
       *       Limbo Mode entry.  Set the limbo
       *       state machine for chaos.
       */
       TRACE3("lGO!", (unsigned long) p_dds->wrk.adap_state, 
		      (unsigned long) p_dds->wrk.limbo );

       p_dds->wrk.limbo = CHAOS;      /* We are now in Limbo! */
       p_dds->wrk.rr_entry = reason;   /* save entry reason */
       p_dds->wrk.limcycle = 0;           /* limbo cycle counter to 0 */


        /*
        *       Build the asynchronous status block
        */
        p_dds->wrk.limbo_blk.code = CIO_ASYNC_STATUS;
        p_dds->wrk.limbo_blk.option[0] = CIO_NET_RCVRY_ENTER;
        p_dds->wrk.limbo_blk.option[1] = reason;
        if ( reason == TOK_ADAP_CHECK )
            bcopy( &(p_dds->wrk.ac_blk), 
		   &(p_dds->wrk.limbo_blk.option[2]),
                   sizeof(adap_check_blk_t) );
        else
        {
            p_dds->wrk.limbo_blk.option[2] = ac;
            p_dds->wrk.limbo_blk.option[3] = NULL;
        } /* end if !TOK_ADAP_CHECK */


       /*
        *       Notify all users that we are in Limbo mode.
        */
        async_status(p_dds, &p_dds->wrk.limbo_blk);

       /*
        *       Log the error.
        */
        logerr(p_dds, ERRID_TOK_RCVRY_ENTER);

       /*
        *       clean out the TX chain.
        *       also flush the TX Queue.
        */
        rc = clean_tx(p_dds);

       /*
        *       Passup all receive data to
        *       appropriate users.  Return all currently
        *       unused mbufs to the system.
        */
        p_dds->wrk.recv_mode = FALSE;
        clear_recv_chain( p_dds );

      /*
       *   FUTURE FIX:
       *       Add logic to flush all pending
       *       OFLV work elements.
       *
       *       Add logic to stop any timers that
       *       may be running AND wakeup the
       *       appropriate process threads that
       *       may be sleeping.
       */

	/*
	 *	If any user processes were waiting for adapter responses
	 *	- cancel their failsafe timers
	 *	- wake them up
	 */

	/*
	 *	Handle waiting on ioctl TOK_FUNC_ADDR
	 */
	if (p_dds->wrk.functional_td.run)
	{
		tstop( p_dds->wrk.p_func_timer );
		p_dds->wrk.functional_td.run = FALSE;
	}
	p_dds->wrk.funct_wait = FALSE;
	p_dds->wrk.funct_status = CIO_NET_RCVRY_ENTER;
	if (p_dds->wrk.funct_event != EVENT_NULL)
		e_wakeup( &p_dds->wrk.funct_event );

	/*
	 *	Handle waiting on ioctl TOK_GRP_ADDR
	 */
	if (p_dds->wrk.group_td.run)
	{
		tstop( p_dds->wrk.p_group_timer );
		p_dds->wrk.group_td.run = FALSE;
	}
	p_dds->wrk.group_wait = FALSE;
	p_dds->wrk.group_status = CIO_NET_RCVRY_ENTER;
	if (p_dds->wrk.group_event != EVENT_NULL)
		e_wakeup( &p_dds->wrk.group_event );

	/*
	 *	Handle waiting on ioctl TOK_RING_INFO
	 */
	if (p_dds->wrk.ring_info_td.run)
	{
		tstop( p_dds->wrk.p_ring_info_timer );
		p_dds->wrk.ring_info_td.run = FALSE;
	}
	p_dds->wrk.ring_info_wait = FALSE;
	p_dds->wrk.ring_info_status = CIO_NET_RCVRY_ENTER;
	if (p_dds->wrk.ring_info_event != EVENT_NULL)
		e_wakeup( &p_dds->wrk.ring_info_event );

      /*
       *  Set all the adapter activation variables to 0.
       *  This way we can start the activation all over
       *  with a clean slate.
       */
       p_dds->wrk.reset_retry = 0;
       p_dds->wrk.reset_spin = 0;
       p_dds->wrk.adap_init_retry = 0;
       p_dds->wrk.adap_init_spin = 0;
       p_dds->wrk.adap_open_retry = 0;

       /*
        *       Change the adapter state to 1st phase
        *       of adapter reset, reset the adapter
        *       and we're now in Limbo.
        */

        p_dds->wrk.adap_state = RESET_PHASE0;
        rc = reset_adap(p_dds, &owe);

   TRACE4 ("elmE", (ulong)p_dds, (ulong)p_dds->wrk.soft_chaos,
      (ulong)p_dds->wrk.hard_chaos); /* enter_limbo end */


}  /* end if Limbo Mode entered */
else
{
	/* 
	 * Daily Double! 
	 * This should NEVER happen. 
	 */
   TRACE5("lDD!", (ulong) p_dds->wrk.adap_state, 
	          (ulong) p_dds->wrk.limbo,
		  (ulong)p_dds->wrk.bugout,
		  (ulong)p_dds->wrk.adap_doa );
}
   
i_enable(sil);  
return(rc);
}      /* end enter_limbo() */

/*
 * NAME: kill_limbo
 *
 * FUNCTION:
 *      This function terminates the Token-Ring Ring Recovery Scheme.
 *
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine only executes on a process thread.
 *
 * NOTES:
 *     This rountine can only be if we are in some form of
 *     limbo mode AND we have received the very last close.
 *
 * (RECOVERY OPERATION:) Information describing both hardware and
 *      software error recovery.
 *
 * DATA STRUCTURES: 
 *	Changes the limbo state to LIMBO_KILL_PEND to indicate that 
 * 	we are in the process of killing limbo.
 *
 * RETURNS: What this code returns (NONE, if it has no return value)
 */

int
kill_limbo( dds_t *p_dds )

{ /* begin function exit_limbo() */

        struct status_block sb;
        int rc = 0, sil, pio_attachment, limbo;

   TRACE4 ("klmB", (ulong)p_dds, (ulong)p_dds->wrk.adap_state,
      (ulong)p_dds->wrk.limbo); /* kill_limbo begin */

   /*
   *   Log the Termination of Ring
   *   Recovery Mode.
   */
   logerr(p_dds, ERRID_TOK_RCVRY_TERM);
sil = i_disable(INTCLASS2);

	/* save the current state of limbo.
	 * set the limbo state to indicate that we
	 * are in the process of killing limbo.
	 */

	limbo = p_dds->wrk.limbo;
	p_dds->wrk.limbo = LIMBO_KILL_PEND;

   switch (limbo)
   {   /* what phase of limbo are we in? */
       case CHAOS:
       {   /* begin case CHAOS */
           i_enable(sil);
           rc = kill_act(p_dds);
   		TRACE2("FooT", LMB_KILL_0);
           break;
       }   /* end case CHAOS */
       case PROBATION:
           /*
           *   We are still in the penalty box.
           *   Stop the probation timer, then close the adapter
           */
           i_enable(sil);
           tstop( p_dds->wrk.p_probate_timer );
   		TRACE2("FooT", LMB_KILL_1);
           rc = close_adap( p_dds );
           break;
       case LIMBO_FUNCTIONAL:
           /*
           *   We have a functional address command pending
           *   with the adapter. Stop the functional timer, then
           *   close the adapter.
           *
           *   FUTURE FIX:
           *       Look into flushing the OFLV work queue
           */
           i_enable(sil);
           tstop( p_dds->wrk.p_func_timer );
   		TRACE2("FooT", LMB_KILL_2);
           rc = close_adap( p_dds );
           break;
       default:
           /*
           *   We have a Group address command pending
           *   with the adapter. Stop the Group timer, then
           *   close the adapter.
           *
           *   FUTURE FIX:
           *       Look into flushing the OFLV work queue
           */
           i_enable(sil);
           tstop( p_dds->wrk.p_group_timer );
   		TRACE2("FooT", LMB_KILL_3);
           rc = close_adap( p_dds );
           break;


   } /* endswitch */

   TRACE4 ("klmE", (ulong)p_dds, (ulong)p_dds->wrk.adap_state,
      (ulong)p_dds->wrk.limbo); /* kill_limbo end */
} /* end kill_limbo() */

/*
 * NAME: cycle_limbo
 *
 * FUNCTION:
 *      This function continues the Limbo recovery sequence.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine executes on the OFLV.
 *
 * NOTES:
 *     This routine may be called for one of the following reasons.
 *         1. During the cycling of limbo, the activation
 *            sequence failed.
 *         2. Double Jeopardy!  A limbo entry condition occurred
 *            during the activation sequence during the cycling
 *            of limbo.
 *         3. Red Zone.  A limbo entry condition occurred during
 *            the Egress of Limbo.
 *
 * (RECOVERY OPERATION:) Information describing both hardware and
 *      software error recovery.
 *
 * DATA STRUCTURES:
 *
 *
 * RETURNS: What this code returns (NONE, if it has no return value)
 */
int
cycle_limbo( dds_t *p_dds)
{
   int rc = 0, sil;
   offl_elem_t     owe;

   TRACE4 ("clmB", (ulong)p_dds, (ulong)p_dds->wrk.adap_state,
      (ulong)p_dds->wrk.limbo); /* cycle_limbo begin */
   sil = i_disable(INTCLASS2);
   if (p_dds->wrk.limbo == CHAOS) 
   {   /* died during activation of adapter */
       /* cancel the bringup timer */

	if ( p_dds->wrk.time_data.run == TRUE )
	{
	    tstop( p_dds->wrk.p_bringup_timer );
	    p_dds->wrk.time_data.run = FALSE;
	}

   	TRACE2("FooT", LMB_CYCLE_0);
   }
   else if ( (p_dds->wrk.limbo != PARADISE) &&
		(p_dds->wrk.limbo != NO_OP_STATE) )
   {   /*
       *   We have a failure during the Red Zone.
       *   Stop whatever timer may be running.
       *   Possible timers running are:
       *       Probation
       *       Receive Limbo SCB available
       *       Functional Address
       *       Group Address
       *       Adapter Trace
       *       CHAOS - adapter bringup timer 
       *
       */
       switch( p_dds->wrk.limbo )
       {
           case PROBATION:
               tstop( p_dds->wrk.p_probate_timer);
               p_dds->wrk.probate_td.run = FALSE;
   		TRACE2("FooT", LMB_CYCLE_1);
               break;
           case LIMBO_RECV:
               /*
                * Undo what we've done in regard to
                * the receive processing setup.
                */
                tstop(p_dds->wrk.p_recv_limbo_timer);
                p_dds->wrk.recv_limbo_td.run = FALSE;
                p_dds->wrk.recv_mode = FALSE;
                clear_recv_chain( p_dds );
   		TRACE2("FooT", LMB_CYCLE_2);
                break;
           case LIMBO_FUNCTIONAL:
               tstop( p_dds->wrk.p_func_timer);
               p_dds->wrk.functional_td.run = FALSE;
   		TRACE2("FooT", LMB_CYCLE_3);
               break;
           case LIMBO_KILL_PEND:
		/*
		 * We got a limbo entry condition while already in limbo AND
		 * we are attempting to shutdown the adapter in response
		 * to a close.  So don't reset the adapter just return 
		 * so that the kill limbo code will finish correctly.
		 */
	       return(0);
            default :
               tstop( p_dds->wrk.p_group_timer);
               p_dds->wrk.group_td.run = FALSE;
   		TRACE2("FooT", LMB_CYCLE_4);
               break;
       }   /* end switch limbo */

       p_dds->wrk.limbo = CHAOS;

   } /* endif */
   else
   {   
	/* NB:
	 * we have either bugged out while we were in 
	 * limbo and this call to cycle_limbo() is a left
	 * over one (i.e. a timer pop) OR
	 * we've gotten so confused as to call cycle_limbo()
	 * while we were not in limbo to begin with.
	 * 
	 * This latter case should not happen.
	 */
   		TRACE4("FooT", LMB_CYCLE_5,
			(ulong)p_dds->wrk.bugout,
			(ulong)p_dds->wrk.adap_doa );
	i_enable(sil);
	return(EPERM);
   }   /* end NULL else */


        /*
        *  count the limbo cycles
        */
   p_dds->wrk.limcycle++;

        /*
        *  change the adap_state to the RESET_PHASE0...this
        *  is the 1st phase in bringing up the adpater.
        *
        *  Set all the adapter activation variable to 0.
        *  This way we can start the activation all over
        *  with a clean slate.
        *
        *  Call reset_adap to kick the cycle off again.
        */
   p_dds->wrk.reset_retry = 0;
   p_dds->wrk.reset_spin = 0;
   p_dds->wrk.adap_init_retry = 0;
   p_dds->wrk.adap_init_spin = 0;
   p_dds->wrk.adap_open_retry = 0;
   p_dds->wrk.adap_state = RESET_PHASE0;
   rc = reset_adap(p_dds, &owe);

   TRACE4 ("clmE", (ulong)p_dds->wrk.limcycle, (ulong)p_dds->wrk.soft_chaos,
      (ulong)p_dds->wrk.hard_chaos); /* cycle_limbo end */
   i_enable(sil);
   return(rc);
} /* end cycle_limbo */

/*
 * NAME: egress_limbo
 *
 * FUNCTION:
 *      This function brings the device driver out of limbo.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine executes on the OFLV.
 *
 * NOTES:
 *     This routine will walk the device driver out of limbo mode.
 *     The path taken is:
 *         - Through the probation period
 *         - kicking off the receiving of data.
 *         - setting the functional address
 *         - then setting the group address
 *
 * (RECOVERY OPERATION:) Information describing both hardware and
 *      software error recovery.
 *
 * DATA STRUCTURES:
 *
 *
 * RETURNS: What this code returns (NONE, if it has no return value)
 */

int 
egress_limbo( dds_t *p_dds)

{  /* begin function egress_limbo() */

   /* local vars */
   int sil, rc;
   struct status_block sb;
   unsigned int    pio_attachment;


   sil = i_disable(INTCLASS2);
   TRACE4 ("eglB", (ulong)p_dds, (ulong)p_dds->wrk.adap_state,
      (ulong)p_dds->wrk.limbo); /* egress_limbo begin */

   switch( p_dds->wrk.limbo )
   { /* begin switch */
       case PARADISE:
       case NO_OP_STATE:
       {
	/* NB:
	 * we have either bugged out while we were in 
	 * limbo and this call to egress_limbo() is a left
	 * over one (i.e. a timer pop) OR
	 * we've gotten so confused as to call egress_limbo()
	 * while we were not in limbo to begin with.
	 * 
	 * This latter case should not happen.
	 */
   		TRACE4("FooT", LMB_EGRESS_0,
			(ulong)p_dds->wrk.bugout,
			(ulong)p_dds->wrk.adap_doa );
		break;
       } /* end case PARADISE and NO_OP_STATE */
       case CHAOS:
       {   /* begin case CHAOS */
           p_dds->wrk.probate_td.owe.cmd = OFLV_PROBATION;
           p_dds->wrk.probate_td.owe.who_queued = OFFL_ADAP_BRINGUP;
           p_dds->wrk.p_probate_timer->timeout.it_value.tv_sec
                   = PENALTY_BOX_TIME / 1000;

           p_dds->wrk.p_probate_timer->timeout.it_value.tv_nsec
                   = (PENALTY_BOX_TIME % 1000) * 1000000;
           p_dds->wrk.probate_td.run = TRUE;
           p_dds->wrk.limbo = PROBATION;

           tstart( p_dds->wrk.p_probate_timer );
           break;
       }   /* end case CHAOS */
       case PROBATION:
       {   /* begin case PROBATION */

           /*
            *  Our probation timer has expired.
            *  Start the backing out of Limbo Mode.
            *
            *  We must issue the Receive SCB command
            *  immediately after the OPEN SCB comand completes
            */
		tstop( p_dds->wrk.p_probate_timer);

                p_dds->wrk.recv_mode = TRUE;

               load_recv_chain( p_dds );

               tok_recv_start( p_dds );

           p_dds->wrk.recv_limbo_td.owe.cmd = OFLV_RECV_LIMBO_TIMEOUT;
           p_dds->wrk.recv_limbo_td.owe.who_queued = OFFL_ADAP_BRINGUP;
           p_dds->wrk.p_recv_limbo_timer->timeout.it_value.tv_sec
                   = RECV_LIMBO_TIMEOUT / 1000;

           p_dds->wrk.p_recv_limbo_timer->timeout.it_value.tv_nsec
                   = (RECV_LIMBO_TIMEOUT % 1000) * 1000000;

           p_dds->wrk.limbo = LIMBO_RECV;
               /*
                *  NOTE:
                *       Tell adapter to interrupt us
                *       when the SCB is free to use.
                *
                *       It is key for us to ask for notification
                *       when the SCB is available for use since
                *       we DO NOT get an interrupt for the
                *       completion of the Receive SCB command.
                *
                *       The SCB must be available so we can continue
                *       with the egress of limbo (i.e. issue the
                *       Group and Functional address SCB commands).
                */
               pio_attachment = attach_bus( p_dds );
               rc = pio_write( p_dds, COMMAND_REG, SCBINT );
               detach_bus( p_dds, pio_attachment );

           p_dds->wrk.recv_limbo_td.run = TRUE;

           tstart( p_dds->wrk.p_recv_limbo_timer );

           break;
       }   /* end case PROBATION */
       case LIMBO_RECV:
       {   /* begin case LIMBO_RECV */
          /*
           *   SCB is available.
           *   Stop the SCB available timeout timer.
           *   Set the functional address.
           */
           tstop( p_dds->wrk.p_recv_limbo_timer);
           p_dds->wrk.recv_limbo_td.run = FALSE;

           p_dds->wrk.functional_td.owe.cmd = OFLV_FUNC_TIMEOUT;
           p_dds->wrk.functional_td.owe.who_queued = OFFL_ADAP_BRINGUP;
           p_dds->wrk.p_func_timer->timeout.it_value.tv_sec
                   = FUNCTIONAL_ADDR_TIMEOUT / 1000;

           p_dds->wrk.p_func_timer->timeout.it_value.tv_nsec
                   = (FUNCTIONAL_ADDR_TIMEOUT % 1000) * 1000000;

           p_dds->wrk.functional_td.run = TRUE;
           p_dds->wrk.limbo = LIMBO_FUNCTIONAL;

           issue_scb_command(p_dds, ADAP_FUNCT_CMD,
                                   functional_address(p_dds) );

           tstart( p_dds->wrk.p_func_timer );

           break;
       }   /* end case LIMBO_RECV */
       case LIMBO_FUNCTIONAL:
       {   /* begin case LIMBO_FUNCTIONAL */
          /*
           *   The setting of the functional address
           *   has completed.  Set the group address next.
           */
           tstop( p_dds->wrk.p_func_timer);
           p_dds->wrk.functional_td.run = FALSE;

           p_dds->wrk.group_td.owe.cmd = OFLV_GROUP_TIMEOUT;
           p_dds->wrk.group_td.owe.who_queued = OFFL_ADAP_BRINGUP;
           p_dds->wrk.p_group_timer->timeout.it_value.tv_sec
                   = GROUP_ADDR_TIMEOUT / 1000;

           p_dds->wrk.p_group_timer->timeout.it_value.tv_nsec
                   = (GROUP_ADDR_TIMEOUT % 1000) * 1000000;

           p_dds->wrk.group_td.run = TRUE;
           p_dds->wrk.limbo = LIMBO_GROUP;

           issue_scb_command(p_dds, ADAP_GROUP_CMD,
                                   p_dds->wrk.group_address);

           tstart( p_dds->wrk.p_group_timer );

           break;
       }   /* end case LIMBO_FUNCTIONAL */
       default:
       {   /* begin default case LIMBO_GROUP */

          /*
           *   The setting of the Group address has
           *   completed successfully.
           */
           tstop( p_dds->wrk.p_group_timer);
           p_dds->wrk.group_td.run =FALSE;
	   
	   if ( p_dds->wrk.adap_doa == DEAD_ON_ARRIVAL )
	   {	/* adapter was killed underneath us in SLIH */
		/*
		 * reset adap_doa to put us back on recovery corse.
		 * start up the limbo recovery process.
		 */
		p_dds->wrk.adap_doa = ALIVE_N_WELL;

		TRACE2("FooT", (ulong)LMB_CYCLE_6);
		rc = cycle_limbo( p_dds );
		
	   } /* end if dead on arrival */
	   else
	   {

                /*
                 * Get Transmit started up again.
                 */
               rc = tx_limbo_startup( p_dds );

              /*
               *   We're out of Limbo! We've made it to PARADISE
               */
               p_dds->wrk.limbo = PARADISE;

               logerr( p_dds, ERRID_TOK_RCVRY_EXIT );

               sb.code = CIO_ASYNC_STATUS;
               sb.option[0] = CIO_NET_RCVRY_EXIT;
               sb.option[1] = NULL;
               sb.option[2] = NULL;
               sb.option[3] = NULL;
               async_status( p_dds, &sb);
	    } /* we're out of limbo */


       break;
       }   /* end default LIMBO_GROUP */
   } /* end switch */

   TRACE4 ("eglE", (ulong)p_dds->wrk.limcycle, (ulong)p_dds->wrk.soft_chaos,
      (ulong)p_dds->wrk.hard_chaos); /* egress_limbo end */
   i_enable(sil);
   return(0);
} /* end egress_limbo() */


/*
 * NAME: bug_out
 *
 * FUNCTION:
 *     This function moves the device handler into the
 *     dead state.  A fatal error has just been detected.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine executes on the interrupt level or process thread.
 *
 * NOTES:
 *
 * (RECOVERY OPERATION:) Information describing both hardware and
 *      software error recovery.
 *
 * DATA STRUCTURES:
 *
 *
 * RETURNS: What this code returns (NONE, if it has no return value)
 */
int
bug_out(dds_t  *p_dds,
        unsigned int reason,
        unsigned short ac)
{  /* begin function bug_out() */

	int rc, sil;
	struct status_block sb;
	int pio_attachment;
	uchar pos2;

	sil = i_disable(INTCLASS2);

	TRACE5 ("bugB", (ulong)p_dds, (ulong)p_dds->wrk.adap_state,
		(ulong)p_dds->wrk.limbo, (ulong)reason); /* bug_out begin */

	/*
	 *  FUTURE FIX:
	 *      Add logic to move the device handler from ANY state...
	 *          - download of microcode
	 *          - configuration
	 *          - activation
	 *          - up and running
	 *          - Limbo mode
	 *          - last close
	 *          - kill_limbo()
	 *          - kill_act()
	 *          - any others?
	 *      to the dead state, that is:
	 *          adap_state = NULL_STATE (or DEAD_STATE) &&
	 *          limbo = NO_OP_STATE
	 */

	/*
	 *       Build the asynchronous status block
	 */
	sb.code = CIO_ASYNC_STATUS;
	sb.option[0] = CIO_HARD_FAIL;
	sb.option[1] = reason;
	if ( reason == TOK_ADAP_CHECK )
		bcopy( &(p_dds->wrk.ac_blk), &(sb.option[2]),
				sizeof(adap_check_blk_t) );
	else
	{
		sb.option[2] = ac;
		sb.option[3] = NULL;
	} /* end if !TOK_ADAP_CHECK */


	/* Notify all users that the 
	 * device is dead.
	 */
	async_status(p_dds, &sb);

	/* Log the error
	 */
	logerr(p_dds, ERRID_TOK_ERR10);


	if ( (p_dds->wrk.limbo == PARADISE) &&
	     (p_dds->wrk.adap_state == OPEN_STATE) )
	{
		/* we were up and running
		 * when Fatal error occured
		 */
		TRACE4 ("FooT", (ulong)LMB_BUG_0, 
			(ulong)p_dds->cio.device_state,
			(ulong)p_dds->wrk.adap_doa); 

		/* save bug out action that was
		 * taken.  This will tell what has to be
		 * cleaned up in the bug_out_cleanup() routine.
		 */

		p_dds->wrk.bugout = LMB_BUG_0;

		/* clean out the TX chain.
		 * also flush the TX Queue.
		 */
		rc = clean_tx(p_dds);

		/* Passup all receive data to
		 * appropriate users.  Return all currently
		 * unused mbufs to the system.
		 *
		 */
		clear_recv_chain( p_dds );

		/*
		 * NB:
		 * 	d_complete the ACA.  At this point the
		 * 	adapter state is now the NULL_STATE.  The
		 * 	ACA is now no longer accessible via d_kmove()
		 *	by the device handler or via DMA access by the
		 * 	adapter.
		 */

		p_dds->wrk.adap_state = NULL_STATE;
		p_dds->wrk.limbo = NO_OP_STATE;

		rc = d_complete(p_dds->wrk.dma_chnl_id, DMA_READ,
				p_dds->wrk.p_mem_block, PAGESIZE,
				&(p_dds->wrk.mem_block_xmd),
				p_dds->wrk.p_d_mem_block);

	}
	else if ( (p_dds->wrk.limbo == PARADISE) &&
		  (p_dds->wrk.adap_state == DOWNLOADING) )
	{
		/* In the process of downloading
		 * adapter microcode 
		 */
		TRACE4 ("FooT", (ulong)LMB_BUG_1, 
			(ulong)p_dds->cio.device_state,
			(ulong)p_dds->wrk.adap_doa); 

		/* save bug out action that was
		 * taken.  This will tell what has to be
		 * cleaned up in the bug_out_cleanup() routine.
		 */

		p_dds->wrk.bugout = LMB_BUG_1;

	}
	else if ( (p_dds->wrk.limbo == PARADISE) &&
		  (p_dds->wrk.adap_state == CLOSE_PENDING) )
	{
		/* we are in the process of shutting 
		 * down the adapter.
		 */
		TRACE4 ("FooT", (ulong)LMB_BUG_2, 
			(ulong)p_dds->cio.device_state,
			(ulong)p_dds->wrk.adap_doa); 

		/* save bug out action that was
		 * taken.  This will tell what has to be
		 * cleaned up in the bug_out_cleanup() routine.
		 */

		p_dds->wrk.bugout = LMB_BUG_2;

	}
	else if ( (p_dds->wrk.limbo == PARADISE) &&
		  (p_dds->wrk.adap_state == ADAP_KILL_PENDING) )
	{
		/* We are in the process of killing the
		 * adapter activation sequence. 
		 */
		TRACE4 ("FooT", (ulong)LMB_BUG_3, 
			(ulong)p_dds->cio.device_state,
			(ulong)p_dds->wrk.adap_doa); 

		/* save bug out action that was
		 * taken.  This will tell what has to be
		 * cleaned up in the bug_out_cleanup() routine.
		 */

		p_dds->wrk.bugout = LMB_BUG_3;

	}
	else if ( (p_dds->wrk.limbo == LIMBO_KILL_PEND) &&
		  (p_dds->wrk.adap_state == ADAP_KILL_PENDING) )
	{
		/* We are in the process of killing the limbo
		 * recovery process and we are killing the
		 * adapter activation sequence. 
		 */
		TRACE4 ("FooT", (ulong)LMB_BUG_4, 
			(ulong)p_dds->cio.device_state,
			(ulong)p_dds->wrk.adap_doa); 

		/* save bug out action that was
		 * taken.  This will tell what has to be
		 * cleaned up in the bug_out_cleanup() routine.
		 */

		p_dds->wrk.bugout = LMB_BUG_4;

	}
	else if ( (p_dds->wrk.limbo == LIMBO_KILL_PEND) &&
		  ( (p_dds->wrk.adap_state == OPEN_STATE) ||
		    (p_dds->wrk.adap_state == CLOSE_PENDING) ) )
	{
		/* We are in the process of killing the limbo
		 * recovery process and we are closing the 
		 * adapter.
		 */
		TRACE4 ("FooT", (ulong)LMB_BUG_5, 
			(ulong)p_dds->cio.device_state,
			(ulong)p_dds->wrk.adap_doa); 

		/* save bug out action that was
		 * taken.  This will tell what has to be
		 * cleaned up in the bug_out_cleanup() routine.
		 */

		p_dds->wrk.bugout = LMB_BUG_5;

	}
	else if ( (p_dds->wrk.limbo == NO_OP_STATE) &&
		  (p_dds->wrk.adap_state == NULL_STATE ) )
	{
		/* We have had a fatal error already.
		 * bug_out() has already been called.
		 */
		TRACE4 ("FooT", (ulong)LMB_BUG_6, 
			(ulong)p_dds->cio.device_state,
			(ulong)p_dds->wrk.adap_doa); 

		/* NB:
		 * in the other bug out cases we would
		 * normally save the bug out action that was
		 * taken.  We do not do this here because this
		 * would wipe out the original action that was
		 * taken by the bug_out() routine.
		 */


	}
	else if ( (p_dds->wrk.limbo == PARADISE) &&
		  ( (p_dds->wrk.adap_state != DEAD_STATE) &&
		    (p_dds->wrk.adap_state != DOWNLOADING) &&
		    (p_dds->wrk.adap_state != NULL_STATE) ) )
	{
		/* We were in the middle of adapter
		 * activation sequence when the fatal error occured
		 */
		TRACE4 ("FooT", (ulong)LMB_BUG_7, 
			(ulong)p_dds->cio.device_state,
			(ulong)p_dds->wrk.adap_doa); 

		/* save bug out action that was
		 * taken.  This will tell what has to be
		 * cleaned up in the bug_out_cleanup() routine.
		 */

		p_dds->wrk.bugout = LMB_BUG_7;

	}
	else if ( (p_dds->wrk.limbo != PARADISE) &&
		  (p_dds->wrk.limbo != NO_OP_STATE) )
	{
		/* we are in some phase of limbo
		 */
		TRACE4 ("FooT", (ulong)LMB_BUG_8, 
			(ulong)p_dds->cio.device_state,
			(ulong)p_dds->wrk.adap_doa); 

		/* save bug out action that was
		 * taken.  This will tell what has to be
		 * cleaned up in the bug_out_cleanup() routine.
		 */

		p_dds->wrk.bugout = LMB_BUG_8;

		/* clean out the TX chain.
		 * also flush the TX Queue.
		 */
		rc = clean_tx(p_dds);

		/* Passup all receive data to
		 * appropriate users.  Return all currently
		 * unused mbufs to the system.
		 *
		 */
		clear_recv_chain( p_dds );

		/*
		 * NB:
		 * 	d_complete the ACA.  At this point the
		 * 	adapter state is now the NULL_STATE.  The
		 * 	ACA is now no longer accessible via d_move()
		 *	by the device handler or via DMA access by the
		 * 	adapter.
		 */

		p_dds->wrk.adap_state = NULL_STATE;
		p_dds->wrk.limbo = NO_OP_STATE;

		rc = d_complete(p_dds->wrk.dma_chnl_id, DMA_READ,
				p_dds->wrk.p_mem_block, PAGESIZE,
				&(p_dds->wrk.mem_block_xmd),
				p_dds->wrk.p_d_mem_block);


	}


	/* Disable the card.  We don't
	 * want to give the card the chance to 
	 * misbehave anymore.
	 *
	 * Get access to pos registers.
	 * Get the current setting of pos reg 2.
	 * Turn off the card enable bit in pos.
	 *
	 */

	pio_attachment = attach_iocc( p_dds );

	pos2 = pio_read(p_dds, POS_REG_2);

	(void)pio_write(p_dds, POS_REG_2, (pos2 & ~(CARD_ENABLE) ) );

	detach_iocc(p_dds, pio_attachment);


	TRACE5 ("bugE", (ulong)p_dds->wrk.limcycle, 
		(ulong)p_dds->wrk.soft_chaos,
		(ulong)p_dds->wrk.hard_chaos,
		(ulong)p_dds->wrk.bugout); /* bug_out end */

       i_enable(sil);
   return(0);

}  /* end function bug_out() */
