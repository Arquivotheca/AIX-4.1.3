static char sccsid[] = "@(#)39  1.9  src/bos/kernext/ent/en3com_close.c, sysxent, bos411, 9428A410j 5/16/94 13:43:45";
/*
 * COMPONENT_NAME: sysxent -- High Performance Ethernet Device Driver
 *
 * FUNCTIONS: 
 *		en3com_close
 *		en3com_stop
 *		en3com_cleanup
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
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
#include <sys/iocc.h>
#include <sys/sleep.h>
#include <sys/err_rec.h>
#include <sys/mbuf.h>
#include <sys/dump.h>
#include <sys/ndd.h>
#include <sys/cdli.h>
#include <sys/ethernet_mibs.h>
#include <sys/cdli_entuser.h>
#include <net/spl.h>


#include "en3com_dds.h"
#include "en3com_mac.h"
#include "en3com_hw.h"
#include "en3com_pio.h"
#include "en3com.h"


extern int en3com_open();
extern struct cdt *en3com_cdt_func();
extern en3com_dd_ctl_t en3com_dd_ctl;	

void en3com_stop();
void en3com_cleanup();


/*****************************************************************************/
/*
 * NAME:     en3com_close
 *
 * FUNCTION: Ethernet driver close routine.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      NS user by using the ns_free() service
 *
 * INPUT:
 *      p_ndd           - pointer to the ndd.
 *
 * RETURNS:  
 *	0 - successful
 */
/*****************************************************************************/
en3com_close(
  ndd_t		*p_ndd)		/* pointer to the ndd in the dev_ctl area */

{
  en3com_dev_ctl_t   *p_dev_ctl = (en3com_dev_ctl_t *)(p_ndd->ndd_correlator);
  int ipri;



  TRACE_SYS(HKWD_EN3COM_OTHER, "ClsB", (ulong)p_ndd, 0, 0);
  
  ipri = disable_lock(PL_IMP, &SLIH_LOCK);

  if (p_dev_ctl->device_state == OPENED) {
  	p_dev_ctl->device_state = CLOSE_PENDING;

	/* release the lock */
  	unlock_enable(ipri, &SLIH_LOCK);

  	/* wait for the transmit queue to drain */
  	while (p_dev_ctl->device_state == CLOSE_PENDING &&
		(p_dev_ctl->tx_pending || p_dev_ctl->txq_len)) {
		DELAYMS(1000);		/* delay 1 second */
  	}
	
	/* get the lock again */
  	ipri = disable_lock(PL_IMP, &SLIH_LOCK);
  }
	
  /*
   * de-activate the adapter 
   */
  en3com_stop(p_dev_ctl);
  p_dev_ctl->device_state = CLOSED;
  p_ndd->ndd_flags &= ~(NDD_RUNNING | NDD_UP | NDD_LIMBO | NDD_DEAD);

  /*
   * Stop the system timer
   */
  while(tstop(p_dev_ctl->systimer)) {
	unlock_enable(PL_IMP, &SLIH_LOCK);
	disable_lock(PL_IMP, &SLIH_LOCK);
  }

  unlock_enable(ipri, &SLIH_LOCK);	

  /* cleanup all the resources allocated for open */
  en3com_cleanup(p_dev_ctl);

  lock_write(&DD_LOCK);
  en3com_cdt_del("en3com_dev_ctl", (char *)p_dev_ctl, 
	sizeof(en3com_dev_ctl_t));

  en3com_dd_ctl.open_count--;
  if(!en3com_dd_ctl.open_count) {
	en3com_cdt_del("en3com_dd_ctl", (char *)&en3com_dd_ctl,
		  sizeof(en3com_dd_ctl_t));
	dmp_del(en3com_cdt_func);
  }
  lock_done(&DD_LOCK);

  TRACE_SYS(HKWD_EN3COM_OTHER, "ClsE", 0, 0, 0);
  unpincode(en3com_open);
  return(0);


}
  

/*****************************************************************************/
/*
 * NAME:     en3com_stop
 *
 * FUNCTION: De-activate the adapter.
 *
 * EXECUTION ENVIRONMENT: process or interrupt
 *
 * NOTES:
 *
 * CALLED FROM:
 *      en3com_fixvpd
 *      en3com_close
 *	en3com_hard_fail
 *
 * INPUT:
 *      p_dev_ctl       - pointer to the dev_ctl area.
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
void
en3com_stop(
  en3com_dev_ctl_t	*p_dev_ctl)	/* pointer to the dev_ctl area */
{

  int    ioa, iocc;
  uchar tmp_pos2;
  int	pio_rc = 0;	/* pio exception code */


  TRACE_SYS(HKWD_EN3COM_OTHER, "CstB", (ulong)p_dev_ctl, 0, 0);

  ioa = (int)BUSIO_ATT((ulong)DDS.bus_id, DDS.io_port);

  iocc = (int)IOCC_ATT((ulong)(DDS.bus_id),
                   (ulong)(IO_IOCC + (DDS.slot << 16)));

  /*
   * Clear any possible pending interrupt,
   * such as parity error.
   */
  ENT_PUTCX(ioa + STATUS_REG, 0);

  /* Hard Reset the adapter to force a known state                          */
  ENT_PUTCX(ioa + CONTROL_REG, HARD_RST_MSK);

  /* Read POS register 2 */
  ENT_GETPOS(iocc + POS_REG_2, &tmp_pos2);

  /* Disable parity and the card */
  ENT_PUTPOS(iocc + POS_REG_2, tmp_pos2 & (~PR2_PEN_MSK & ~PR2_CDEN_MSK));

  IOCC_DET(iocc);
  BUSIO_DET(ioa);

  TRACE_SYS(HKWD_EN3COM_OTHER, "CstE", 0, 0, 0);
   
	
}

/*****************************************************************************/
/*
 * NAME:     en3com_cleanup
 *
 * FUNCTION: Cleanup all the resources used by opening the adapter.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      en3com_close
 *
 * INPUT:
 *      p_dev_ctl       - pointer to the dev_ctl area.
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
void
en3com_cleanup(
	en3com_dev_ctl_t	*p_dev_ctl) /* pointer to the dev_ctl area */
{


  en3com_multi_t *p_multi, *p_temp;


  TRACE_SYS(HKWD_EN3COM_OTHER, "CclB", (ulong)p_dev_ctl, 0, 0);

  /* cleanup the transmit/receive buffers */
  if (WRK.rv_buf_alocd)
  	en3com_rv_free(p_dev_ctl);

  if (WRK.tx_buf_alocd)
  	en3com_tx_free(p_dev_ctl);

  /* free the multicast table extensions */
  p_multi = WRK.multi_table.next;
  while (p_multi) {
	p_temp = p_multi->next;
	xmfree(p_multi, pinned_heap);
	p_multi = p_temp;
  }
  WRK.multi_table.next = NULL;
  

  /* Free the DMA clannel */
  if (WRK.channel_alocd) {
	d_clear(WRK.dma_channel);
	WRK.channel_alocd = FALSE;

  } 

  /* Clear the transmit watchdog timer */
  if (WRK.tx_wdt_inited) {
	while (w_clear(&(TXWDT)));
	WRK.tx_wdt_inited = FALSE;
  }
  /* Clear the control operation watchdog timer */
  if (WRK.ctl_wdt_inited) {
	while (w_clear(&(CTLWDT)));
	WRK.ctl_wdt_inited = FALSE;
  }

  /* De-register the interrupt handler */
  if (WRK.intr_inited) {
	i_clear(&(IHS));
	WRK.intr_inited = FALSE;
  }

  TRACE_SYS(HKWD_EN3COM_OTHER, "CclE", 0, 0, 0);


} 

