static char sccsid[] = "@(#)80  1.8.1.5  src/bos/kernext/mps_tok/mps_wdt.c, sysxmps, bos41J, 9520B_all 5/18/95 11:20:56";
/*
 *   COMPONENT_NAME: sysxmps
 *
 *   FUNCTIONS: enter_limbo
 *		mps_bug_out
 *		mps_ctl_timeout
 *		mps_lan_connected
 *		mps_re_act
 *		mps_start_recover
 *		mps_start_timeout
 *		mps_tx1_timeout
 *		mps_tx2_timeout
 *		xmit_buf_resetup
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stddef.h>
#include <sys/types.h>
#include <sys/lock_def.h>
#include <sys/errno.h>
#include <sys/device.h>
#include <sys/intr.h>
#include <sys/timer.h>
#include <sys/watchdog.h>
#include <sys/dma.h>
#include <sys/malloc.h>
#include <sys/intr.h>
#include <sys/adspace.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/param.h>
#include <sys/poll.h>
#include <sys/sleep.h>
#include <sys/trchkid.h>
#include <sys/err_rec.h>
#include <sys/mbuf.h>
#include <sys/dump.h>
#include <sys/ndd.h>
#include <sys/cdli.h>

#include <sys/cdli_tokuser.h>
#include <sys/generic_mibs.h>
#include <sys/tokenring_mibs.h>

#include "mps_dslo.h"
#include "mps_mac.h"
#include "mps_dds.h"
#include "mps_dd.h"
#include "tr_mps_errids.h"
#ifdef KTD_DEBUG
#include "intercept_functions.h"
#endif

extern mps_dd_ctl_t mps_dd_ctl;

#define CHK_DIS_R	0x8880
#define CHK_DIS		0x4440
#define TX_DIS		0x44A0

/****************************************************************************/
/*
 * NAME: mps_bug_out
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
/****************************************************************************/
mps_bug_out( mps_dev_ctl_t  *p_dev_ctl,
    		uint         option0,
    		uint         option1,
    		uint         option2,
		uint	     slih_lock,
		uint	     tx_lock,
		uint	     cmd_lock)
{
   
  int rc;
  int ipri, ipri_slih, txpri;
  ndd_t           *p_ndd = &(NDD);
  ndd_statblk_t    stat_blk;   /* status block */
  int iocc, ioa, pio_rc;
  int	device_state = p_dev_ctl->device_state;
  uchar pos2;
  ushort        bmctl = CHK_DIS_R, i;
  ulong         dbas, dba;

 
  TRACE_SYS(MPS_OTHER, "bubB", (int)p_dev_ctl, 0, 0);

  /* Gets locks */
  if (slih_lock == FALSE) {
  	ipri_slih = disable_lock(PL_IMP, &SLIH_LOCK);
  }
  if (tx_lock == FALSE) {
   	txpri = disable_lock(PL_IMP, &TX_LOCK);
  }
  if (cmd_lock == FALSE) {
  	disable_lock(PL_IMP, &CMD_LOCK);
  }
   
  /*
   * Stops the watchdog timers
   */
  w_stop(&(CTLWDT));
  w_stop(&(TX1WDT));
  w_stop(&(TX2WDT));
  w_stop (&(LIMWDT));
  w_stop (&(LANWDT));
  w_stop (&(HWEWDT));

  reset_adapter(p_dev_ctl);

  /*
   * update NDD frags 
   */
  p_dev_ctl->device_state = DEAD;
  NDD.ndd_flags &= ~(NDD_RUNNING | NDD_LIMBO);
  NDD.ndd_flags |= NDD_DEAD;

  /* wakeup outstanding ioctl or open event */
  if (p_dev_ctl->ctl_pending | p_dev_ctl->open_pending) { 
        e_wakeup((int *)&WRK.ctl_event);
	p_dev_ctl->ctl_pending = FALSE;
	p_dev_ctl->open_pending = FALSE;
  }

  TRACE_SYS(MPS_OTHER, "bugE", p_dev_ctl, 0, 0); 
  if (cmd_lock == FALSE) {
  	unlock_enable(PL_IMP, &CMD_LOCK);
  }
  if (tx_lock == FALSE) {
  	unlock_enable(txpri, &TX_LOCK);
  }
  if (slih_lock == FALSE) {
        unlock_enable(ipri_slih, &SLIH_LOCK);
  }

}  /* end function mps_bug_out() */

/*****************************************************************************/
/*
 * NAME:     mps_start_timeout
 *
 * FUNCTION: Wildwood driver error recover timeout
 *
 * EXECUTION ENVIRONMENT: interrupt only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      Timer 
 *
 * INPUT:
 *      p_wdt  - pointer to the watchdog structure in device control area
 *
 * RETURNS:  
 *      0 if successful
 *      errno value on failure
 */
/*****************************************************************************/
mps_start_timeout(
struct watchdog *p_wdt)   /* pointer to watchdog timer structure */

{
  mps_dev_ctl_t *p_dev_ctl = (mps_dev_ctl_t *)
                        ((ulong)p_wdt - offsetof(mps_dev_ctl_t, hwe_wdt));
  ndd_statblk_t stat_blk;   /* status block */
  ndd_t         *p_ndd = &(NDD);


  TRACE_SYS(MPS_OTHER, "tstB", (ulong)p_dev_ctl, 0, 0);

  if ((p_dev_ctl->device_state != DEAD) &&
      (p_dev_ctl->device_state != OPEN_PENDING)) {

  	mps_bug_out(p_dev_ctl, NDD_ADAP_CHECK, 0, 0, FALSE, FALSE, FALSE);
  	mps_logerr(p_dev_ctl, ERRID_MPS_ADAP_CHECK, __LINE__,
                   __FILE__, NDD_ADAP_CHECK, NDD_LIMBO_EXIT, 0, FALSE);
  }
  TRACE_SYS(MPS_OTHER, "tstE", (ulong)p_dev_ctl, 0, 0);

}

/*****************************************************************************/
/*
 * NAME:     mps_lan_connected
 *
 * FUNCTION: Wildwood driver lan connected timer
 *
 * EXECUTION ENVIRONMENT: interrupt only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      Timer 
 *
 * INPUT:
 *      p_wdt  - pointer to the watchdog structure in device control area
 *
 * RETURNS:  
 *      0 if successful
 *      errno value on failure
 */
/*****************************************************************************/
mps_lan_connected(
struct watchdog *p_wdt)   /* pointer to watchdog timer structure */

{
  mps_dev_ctl_t *p_dev_ctl = (mps_dev_ctl_t *)
                        ((ulong)p_wdt - offsetof(mps_dev_ctl_t, lan_wdt));
  ndd_statblk_t stat_blk;   /* status block */
  ndd_t         *p_ndd = &(NDD);
  int           ipri_slih;
  int           ipri;

  TRACE_SYS(MPS_OTHER, "tlnB", (ulong)p_dev_ctl, 0, 0);

  if ((p_dev_ctl->device_state != DEAD) &&
      (p_dev_ctl->device_state != OPEN_PENDING)) {
 	/* Gets locks */
  	ipri_slih = disable_lock(PL_IMP, &SLIH_LOCK);
 	ipri = disable_lock(PL_IMP, &TX_LOCK);
  	disable_lock(PL_IMP, &CMD_LOCK);

  	WRK.ndd_limbo = FALSE;
  	if (p_dev_ctl->device_state != DEAD) {
        	/*
         	 * Updates the adapter device state
        	 */
        	WRK.dev_stime = lbolt;
        	p_dev_ctl->device_state = OPENED;

        	NDD.ndd_flags |= NDD_RUNNING;
        	mps_logerr(p_dev_ctl, ERRID_MPS_RCVRY_EXIT, __LINE__, __FILE__,
				0, 0, 0);

        	/*
        	 * Pass the status block to demuxer
        	 */
        	NDD.ndd_flags &= ~NDD_LIMBO;
        	stat_blk.code = NDD_LIMBO_EXIT;
        	stat_blk.option[0] = NDD_CONNECTED;
        	(*(NDD.nd_status))(p_ndd, &stat_blk);
  	}

  	unlock_enable(PL_IMP, &CMD_LOCK);
  	unlock_enable(ipri, &TX_LOCK);
  	unlock_enable(ipri_slih, &SLIH_LOCK);

  	TRACE_SYS(MPS_OTHER, "tlnE", (ulong)p_dev_ctl, 0, 0);
  }

}

/*****************************************************************************/
/*
 * NAME:     mps_start_recover
 *
 * FUNCTION: Wildwood driver recover timer
 *
 * EXECUTION ENVIRONMENT: interrupt only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      Timer 
 *
 * INPUT:
 *      p_wdt  - pointer to the watchdog structure in device control area
 *
 * RETURNS:  
 *      0 if successful
 *      errno value on failure
 */
/*****************************************************************************/
mps_start_recover(
struct watchdog *p_wdt)   /* pointer to watchdog timer structure */

{
  mps_dev_ctl_t *p_dev_ctl = (mps_dev_ctl_t *)
                        ((ulong)p_wdt - offsetof(mps_dev_ctl_t, lim_wdt));
  ndd_statblk_t stat_blk;   /* status block */
  ndd_t         *p_ndd = &(NDD);


  TRACE_SYS(MPS_OTHER, "tsrB", (ulong)p_dev_ctl, 0, 0);

  if ((p_dev_ctl->device_state != DEAD) &&
      (p_dev_ctl->device_state != OPEN_PENDING)) {
  	/* start recover process */
  	if (mps_re_act(p_dev_ctl, TRUE, FALSE)) {
  		TRACE_BOTH(MPS_ERR, "tsr1", (ulong)p_dev_ctl, 0, 0);
 		mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_ADAP_CHECK, 0, FALSE, 
						FALSE, FALSE);
        	mps_logerr(p_dev_ctl, ERRID_MPS_ADAP_CHECK, __LINE__,
               	      __FILE__, NDD_HARD_FAIL, NDD_DEAD, OPENED);
  	}
  }
  TRACE_SYS(MPS_OTHER, "tsrE", (ulong)p_dev_ctl, 0, 0);

}

/*****************************************************************************/
/*
 * NAME:     mps_tx1_timeout
 *
 * FUNCTION: Wildwood driver transmit channel 1 watchdog timer timeout routine.
 *       Call hardware error handler.
 *
 * EXECUTION ENVIRONMENT: interrupt only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      Timer 
 *
 * INPUT:
 *      p_wdt  - pointer to the watchdog structure in device control area
 *
 * RETURNS: 
 *      0 if successful
 *      errno value on failure
 */
/*****************************************************************************/
mps_tx1_timeout(
struct watchdog *p_wdt)   /* pointer to watchdog timer structure */

{
 mps_dev_ctl_t *p_dev_ctl = (mps_dev_ctl_t *)
                     ((ulong)p_wdt - offsetof(mps_dev_ctl_t, tx1_wdt));
 ndd_statblk_t stat_blk;   /* status block */
 ndd_t         *p_ndd = &(NDD);
 ushort	       bmctl;
 int	       ioa;

  TRACE_SYS(MPS_OTHER, "tt1B", (ulong)p_dev_ctl, bmctl, 0);
  TOKSTATS.tx_timeouts++;
  WRK.tx1wdt_setter = NULL;

  mps_logerr(p_dev_ctl,ERRID_MPS_TX_TIMEOUT, __LINE__, __FILE__, 
			1, WRK.tx1_frame_pending, bmctl);

  if ((p_dev_ctl->device_state != DEAD) &&
      (p_dev_ctl->device_state != OPEN_PENDING)) {
  	/* 
   	 * Gets access to the I/O bus to access I/O registers                 
  	 */
  	ioa = (int)BUSIO_ATT( DDS.bus_id, DDS.io_base_addr);
  	PIO_GETSRX(ioa + BMCtl_sum, &bmctl);
  	BUSIO_DET(ioa);                      /* restore I/O Bus              */

  	if (WRK.pio_rc) {
  		TRACE_BOTH(MPS_ERR, "tt11", (int)p_dev_ctl, WRK.pio_rc, 0);
		mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL,
				0, FALSE, FALSE, FALSE);
  	}
  	enter_limbo(p_dev_ctl, TRUE, FALSE, 0, NDD_TX_TIMEOUT, 1, 0);
  } else {
        WRK.tx1_frame_pending = 0;

  }
  TRACE_SYS(MPS_OTHER, "tt1E", (ulong)p_dev_ctl, 0, 0);

}

/*****************************************************************************/
/*
 * NAME:     mps_tx2_timeout
 *
 * FUNCTION: Wildwood driver transmit channel 2 watchdog timer timeout routine.
 *       Call hardware error handler.
 *
 * EXECUTION ENVIRONMENT: interrupt only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      Timer 
 *
 * INPUT:
 *      p_wdt  - pointer to the watchdog structure in device control area
 *
 * RETURNS: 
 *      0 if successful
 *      errno value on failure
 */
/*****************************************************************************/
mps_tx2_timeout(
struct watchdog *p_wdt)   /* pointer to watchdog timer structure */

{

  mps_dev_ctl_t   *p_dev_ctl = (mps_dev_ctl_t *)
                        ((ulong)p_wdt - offsetof(mps_dev_ctl_t, tx2_wdt));
  ndd_statblk_t  stat_blk;   /* status block */
  ndd_t         *p_ndd = &(NDD);
  ushort	 bmctl;
  int	         ioa;

  /* 
   * Gets access to the I/O bus to access I/O registers                 
   */
  ioa = (int)BUSIO_ATT( DDS.bus_id, DDS.io_base_addr);
  PIO_GETSRX(ioa + BMCtl_sum, &bmctl);

  BUSIO_DET(ioa);                      /* restore I/O Bus              */

  TRACE_SYS(MPS_OTHER, "tt2B", (ulong)p_dev_ctl, bmctl, 0);
  TOKSTATS.tx_timeouts++;
  WRK.tx2wdt_setter = NULL;

  mps_logerr(p_dev_ctl,ERRID_MPS_TX_TIMEOUT, __LINE__, __FILE__, 
			2, WRK.tx2_frame_pending, bmctl);

  if ((p_dev_ctl->device_state != DEAD) &&
      (p_dev_ctl->device_state != CLOSE_PENDING)) {
        if (WRK.pio_rc) {
                TRACE_BOTH(MPS_ERR, "tt21", (int)p_dev_ctl, WRK.pio_rc, 0);
                mps_bug_out(p_dev_ctl, NDD_HARD_FAIL,NDD_PIO_FAIL,0,
                                FALSE,FALSE,FALSE);
        } else {
                enter_limbo(p_dev_ctl, TRUE, FALSE, 0, NDD_TX_TIMEOUT, 2, 0);
        }
  } else {
        WRK.tx2_frame_pending = 0;
  } 
  TRACE_SYS(MPS_OTHER, "tt2E", (ulong)p_dev_ctl, 0, 0);

}

/*****************************************************************************/
/*
 * NAME:     mps_ctl_timeout
 *
 * FUNCTION: Wildwood driver ioctl watchdog timer timeout routine.
 *       Set control operation status and wake up the ioctl routine.
 *
 * EXECUTION ENVIRONMENT: interrupt only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      Timer 
 *
 * INPUT:
 *      p_wdt  - pointer to the watchdog structure in device control area
 *
 * RETURNS: 
 *      0 if successful
 *      errno value on failure
 */
/*****************************************************************************/
mps_ctl_timeout(
struct watchdog *p_wdt)   /* pointer to watchdog timer structure */

{
  mps_dev_ctl_t *p_dev_ctl = (mps_dev_ctl_t *)
                        ((ulong)p_wdt - offsetof(mps_dev_ctl_t, ctl_wdt));
  ndd_statblk_t stat_blk;   /* status block */
  ndd_t         *p_ndd = &(NDD);

  TRACE_SYS(MPS_OTHER, "tctB", (ulong)p_dev_ctl, 0, 0);
  mps_logerr(p_dev_ctl,ERRID_MPS_CTL_ERR,__LINE__,__FILE__,NDD_CMD_FAIL,0,0);

  /* 
   * Wakeup the ioctl event 
   */
  if (p_dev_ctl->ctl_pending) {
        e_wakeup((int *)&WRK.ctl_event);
	p_dev_ctl->ctl_pending = FALSE;
  }

 if ((p_dev_ctl->device_state != DEAD) &&
      (p_dev_ctl->device_state != CLOSE_PENDING)) {

  	enter_limbo(p_dev_ctl, TRUE, FALSE, 0, NDD_CMD_FAIL, 0, 0);
  }
  TRACE_SYS(MPS_OTHER, "tctE", (ulong)p_dev_ctl, 0, 0);

}

/*****************************************************************************/
/*
 * NAME:     enter_limbo
 *
 * FUNCTION: Wildwood driver hardware error handler.
 *
 * EXECUTION ENVIRONMENT: process and interrupt 
 *
 * NOTES:
 *
 * CALLED FROM:
 *      mps_intr 
 *
 * INPUT:
 *      p_dev_ctl       - pointer to the device control area
 *  	at_int_lvl	- running at interrupt level code or not
 *  	from_slih	- flag for enterring from process or interrupt level
 *      delay           - time to delay befor restart the recover
 * 	option0 	- status block option 0 
 * 	option1 	- status block option 1
 * 	option2 	- status block option 2 
 *
 * RETURNS:  
 *       none.
 */
/*****************************************************************************/
enter_limbo(
	mps_dev_ctl_t  *p_dev_ctl,	/* pointer to device control area */
	int  		at_int_lvl,	/* flag for running at interrupt/proc */
	int  		from_slih, 	/* flag for process or interrupt */
	int		delay,		/* delay time			 */
	int		option0,	/* status block option 0 */
	int		option1,	/* status block option 1 */
	int		option2)	/* status block option 2 */

{
  ndd_statblk_t  stat_blk;   /* status block */
  ndd_t		*p_ndd = &(NDD);
  int 		ipri_slih;
  int 		ipri;
  int 		ioa, iocc, pio_rc, rc;
  uchar         pos2;
  int		device_state = p_dev_ctl->device_state;
  ushort	bmctl = CHK_DIS_R, i;
  ulong         dbas, dba;

  TRACE_SYS(MPS_OTHER, "NheB",(ulong)p_dev_ctl, from_slih, at_int_lvl);

  /*
   * No status block will be send if there is no reason code (option 0)
   */
  if (option0) {
  	/*
   	* Pass the status block to demuxer
   	*/
  	stat_blk.code = NDD_LIMBO_ENTER;
  	stat_blk.option[0] = option0;
  	stat_blk.option[1] = option1;
  	stat_blk.option[2] = option2;
  	(*(NDD.nd_status))(p_ndd, &stat_blk);
  }

  /*
   * Stops the watchdog timers
   */
  w_stop(&(CTLWDT));
  w_stop(&(TX1WDT));
  w_stop(&(TX2WDT));
  w_stop (&(LIMWDT));
  w_stop (&(LANWDT));
  w_stop (&(HWEWDT));

  /* Gets locks */
  if (!from_slih) {
  	ipri_slih = disable_lock(PL_IMP, &SLIH_LOCK);
  }

  ipri = disable_lock(PL_IMP, &TX_LOCK);
  disable_lock(PL_IMP, &CMD_LOCK);

  /*
   * update NDD frags * device state
   */
  NDD.ndd_flags &=  ~NDD_RUNNING;
  NDD.ndd_flags |=  NDD_LIMBO;
  p_dev_ctl->device_state = LIMBO;

  /* wakeup outstanding ioctl or open event */
  if (p_dev_ctl->ctl_pending | p_dev_ctl->open_pending) { 
        e_wakeup((int *)&WRK.ctl_event);
	p_dev_ctl->ctl_pending = FALSE;
	p_dev_ctl->open_pending = FALSE;
  }

  if (reset_adapter(p_dev_ctl)) {
 	TRACE_BOTH(MPS_ERR, "Nhe1", (int)p_dev_ctl, WRK.pio_rc, 0);
 	mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, 0, 0, TRUE, TRUE, TRUE);
  } else { 
  	if (delay) {
  		LIMWDT.restart     = delay;
  		w_start (&(LIMWDT));        /* starts watchdog timer  */
  	} else {
 		if (rc = mps_re_act(p_dev_ctl, TRUE, FALSE)) {
  			TRACE_BOTH(MPS_ERR, "Nhe2", (ulong)p_dev_ctl, rc, 0);
 			mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, 0, 0,
                                               TRUE, TRUE, TRUE);
       			mps_logerr(p_dev_ctl, ERRID_MPS_ADAP_CHECK, __LINE__,
                     		__FILE__, NDD_HARD_FAIL, rc, OPENED);
  		}
  	}
  }

  unlock_enable(PL_IMP, &CMD_LOCK);
  unlock_enable(ipri, &TX_LOCK);

  if (!from_slih) {
        unlock_enable(ipri_slih, &SLIH_LOCK);
  }

  TRACE_SYS(MPS_OTHER, "NheE", p_dev_ctl, 0, 0);
}

/*****************************************************************************/
/*
* NAME: mps_re_act
*
* FUNCTION: Start the initalization of the adapter: load POS registers and
*           start the sequence for open the adapter.
*
* EXECUTION ENVIRONMENT:
*      process thread.
*
* Output: 
*  An adapter open.
*
* Called From: 
*  mps_restart
*
* INPUT:
*       p_dev_ctl       - pointer to the device control area
*  	at_int_lvl	- flag for running at interrupt or process level
*  	from_slih	- flag for enterring from process or interrupt level
*
*
* RETURN:  
*      0 = OK
*      ENOCONNECT - unable to activate and setup the adapter
*      ENOMEM - unable to allocate required memory
*
*/
/*****************************************************************************/
int mps_re_act (
mps_dev_ctl_t  *p_dev_ctl,	/* pointer to device control area */
int  		at_int_lvl,	/* flag for running at interrupt/proc */
int  		from_slih)	/* flag for process or interrupt */

{
  ndd_t         *p_ndd = &(NDD);
  int    	ioa, iocc, pio_rc;
  uchar         pos2;
  int    	cntl_reg;
  ushort 	data;
  int 		ipri_slih;
  ndd_statblk_t stat_blk;   		 /* status block */
  int    	i;                       /* loop counter                      */
  int    	rc;                      /* Local return code                 */

  TRACE_SYS(MPS_OTHER, "NraB", p_dev_ctl, at_int_lvl, from_slih);

  /*
   * Sets the adapter's POS registers so driver can access the adapter  
   */
  if (cfg_pos_regs(p_dev_ctl)) {
        TRACE_BOTH(MPS_ERR, "Nra2", p_dev_ctl, ENOCONNECT, 0);
        return(ENOCONNECT);
  }

  /*
   * Frees the Rx buffer queues
   */
  if (WRK.mhead) {
        m_freem(WRK.mhead);
        WRK.mhead = NULL;
  }

  /*********************************************************************/
  /* Reset up the receive memory area                                  */
  /*********************************************************************/
  if (recv_buf_setup(p_dev_ctl, TRUE)) {  /* Something went wrong      */
        TRACE_BOTH(MPS_ERR, "Nra3", p_dev_ctl, ENOMEM, 0);
        return(ENOMEM);
  }

  /*********************************************************************/
  /* Reset up the transmit memory area                                 */
  /*********************************************************************/
  if (xmit_buf_resetup(p_dev_ctl)) {  /* Something went wrong          */
        TRACE_BOTH(MPS_ERR, "Nra4", p_dev_ctl, ENOMEM, 0);
        return(ENOMEM);
  }

  /*
   * Gets access to the I/O bus to access I/O registers                 
   */
  ioa = (int)BUSIO_ATT( DDS.bus_id, DDS.io_base_addr);

  PIO_PUTSRX( ioa + MISRMask, MISR_MSK); /* setup the MISR mask register*/
  if (WRK.pio_rc) {
	mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL, 0, 
			at_int_lvl, FALSE, FALSE);
        TRACE_BOTH(MPS_ERR, "Nra4", p_dev_ctl, WRK.pio_rc, 0);
  	BUSIO_DET(ioa);                /* restore I/O Bus              */
	return (ENOCONNECT);
  }


  /*********************************************************************/
  /* 
   * Starts the adapter init sequence - SISR_Mask is edge triggered    
   * The response of this will be in INITIALIZATION_COMPLETE in       
   * srb_response function.
   */
  /*********************************************************************/
  PIO_PUTSRX( ioa + SISRMask, SISR_MSK);
  if (WRK.pio_rc)
  {
	mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL, 0, 
			at_int_lvl, FALSE, FALSE);
        TRACE_BOTH(MPS_ERR, "Nra5", p_dev_ctl, WRK.pio_rc, 0);
  	BUSIO_DET(ioa);                /* restore I/O Bus              */
	return (ENOCONNECT);
  }

  BUSIO_DET(ioa);                      /* restore I/O Bus              */

  w_start (&(HWEWDT));                 /* starts watchdog timer        */

  return(0);
} /* end mps_re_act                                                          */

/****************************************************************************/
/*
* NAME: xmit_buf_resetup
*
* FUNCTION: 
*	Cleanup transmit list and Re_initialize the transmit list variables
*
* EXECUTION ENVIRONMENT:
*
*      This routine runs only under the process thread.
*
* NOTES:
*
*    Input:
*       p_dev_ctl - pointer to device contril structure
*
*    Output:
*       Transmit list set up, transmit list variables initialized.
*
*    Called From:
*       mps_re_act
*
* RETURN:  0 - Successful completion
*          ENOBUFS - No Bus Address space available
*/
/****************************************************************************/

int xmit_buf_resetup(mps_dev_ctl_t  *p_dev_ctl)
{
  ndd_t         	*p_ndd = &(NDD);
  int     		rc, ioa;
  uchar   		*addr_p;
  int     		i;             /* Loop Counter                       */
  volatile    tx_list_t	xmitlist;
  xmit_elem_t   	*xelm;         /* transmit element structure */
  struct      mbuf    	*mbufp, *mfreep;

  TRACE_SYS(MPS_OTHER, "NxrB", p_dev_ctl, 0, 0);

  /* 
   * Cleanup the transmit 1 chain 
   */
  while (WRK.tx1_frame_pending) {
          xelm = &WRK.tx1_queue[WRK.tx1_elem_next_out];
          m_freem(xelm->mbufp);
          XMITQ_INC(WRK.tx1_elem_next_out);
          WRK.tx1_frame_pending--;
          NDD.ndd_genstats.ndd_opackets_drop++; 
  }

  if (p_dev_ctl->txq1_first) {
        m_freem(p_dev_ctl->txq1_first);
        p_dev_ctl->txq1_first = NULL;
        NDD.ndd_genstats.ndd_opackets_drop += p_dev_ctl->txq1_len; 
  }

  /* 
   * Cleanup the transmit 2 chain 
   */
  while (WRK.tx2_frame_pending) {
        xelm = &WRK.tx2_queue[WRK.tx2_elem_next_out];
        m_freem(xelm->mbufp);
        XMITQ_INC(WRK.tx2_elem_next_out);
        WRK.tx2_frame_pending--;
        NDD.ndd_genstats.ndd_opackets_drop++; 
  }

  if (p_dev_ctl->txq2_first) {
        m_freem(p_dev_ctl->txq2_first);
        p_dev_ctl->txq2_first = NULL;
        NDD.ndd_genstats.ndd_opackets_drop += p_dev_ctl->txq2_len; 
  }

  /*
   * Set up variable for Transmit list 
   */
  WRK.tx1_elem_next_out = 0;
  WRK.tx1_elem_next_in  = 0;
  WRK.tx1_frame_pending = 0;
  WRK.tx1_buf_next_out  = 0;
  WRK.tx1_buf_next_in   = 0;
  WRK.tx1_buf_use_count = 0;
  WRK.tx1_dma_next_out  = 0;
  WRK.tx1_dma_next_in   = 0;
  WRK.tx1_dma_use_count = 0;
  WRK.tx1_retry         = MAX_TX_LIST;
  p_dev_ctl->txq1_len   = 0;
  p_dev_ctl->txq1_first = NULL;
  p_dev_ctl->txq1_last  = NULL;

  WRK.tx2_elem_next_out = 0;
  WRK.tx2_elem_next_in  = 0;
  WRK.tx2_frame_pending = 0;
  WRK.tx2_buf_next_out  = 0;
  WRK.tx2_buf_next_in   = 0;
  WRK.tx2_buf_use_count = 0;
  WRK.tx2_dma_next_out  = 0;
  WRK.tx2_dma_next_in   = 0;
  WRK.tx2_dma_use_count = 0;
  WRK.tx2_retry         = MAX_TX_LIST;
  p_dev_ctl->txq2_len   = 0;
  p_dev_ctl->txq2_first = NULL;
  p_dev_ctl->txq2_last  = NULL;

  WRK.t_len = 0;

  TRACE_SYS(MPS_OTHER, "NxrE", p_dev_ctl, 0, 0);
  return(0);
}  /* end function xmit_buf_resetup */


/****************************************************************************/
/*
 * NAME: reset_adapter
 *
 * FUNCTION:
 *     This function send a adapter reset request to the adapter
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine executes on the interrupt level or process thread.
 *
 */
/****************************************************************************/

int reset_adapter(
mps_dev_ctl_t  *p_dev_ctl,
int		at_int_lvl) {
 
  ushort        bmctl = CHK_DIS_R, i;
  ulong         dbas, dba;
  ulong         halt = 0, ioa;

 TRACE_SYS(MPS_OTHER, "bubB", (int)p_dev_ctl, 0, 0);

  /* requests Tx & Rx channel disable */
  ioa = (int)BUSIO_ATT( DDS.bus_id, DDS.io_base_addr);

  PIO_PUTSRX(ioa + BMCtl_sum, bmctl);

  /* make sure DBA of Tx1 is not moving */
  i = 0;
  do {
          if (i++ > 20) {
                   TRACE_BOTH(MPS_ERR, "res1", p_dev_ctl, dba, dbas);
                   halt = 1;
                   break;
           }
           dbas = dba;
           io_delay(1000);
           PIO_GETLRX(ioa + Tx1DBA_L, &dba);
  } while (dbas != dba);

  /* make sure DBA of Tx2 is not moving */
  i = 0;
  do {
          if (i++ > 20) {
                   TRACE_BOTH(MPS_ERR, "res2", p_dev_ctl, dba, dbas);
                   halt = 1;
                   break;
           }
           dbas = dba;
           io_delay(1000);
           PIO_GETLRX(ioa + Tx2DBA_L, &dba);
  } while (dbas != dba);

  /* check for DMA disable */
  i = 0;
  do {
        if (i++ > 20) {
                TRACE_BOTH(MPS_ERR, "res3", p_dev_ctl, bmctl, 0);
                halt = 1;
                break;
        }

        io_delay(1);
        PIO_GETSRX(ioa + BMCtl_sum, &bmctl);
  } while ((bmctl != CHK_DIS) & (bmctl != TX_DIS));

  if (halt) {
        return (halt);
  }

  /*
   * Turn on soft reset
   */
  PIO_PUTSRX( ioa + BCtl, SOFT_RST_MSK);
  io_delay(1000);
  PIO_PUTSRX( ioa + BCtl, 0x0000);
  
  PIO_PUTSRX( ioa + BCtl, CHCK_DISABLE);
  BUSIO_DET( ioa );            /* restore I/O Bus          */

  if (WRK.pio_rc) {
        TRACE_BOTH(MPS_ERR, "res4", (int)p_dev_ctl, WRK.pio_rc, 0);
  }
  TRACE_SYS(MPS_OTHER, "bubE", (int)p_dev_ctl, 0, 0);

  return (WRK.pio_rc);

}

