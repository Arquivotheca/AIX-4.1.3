static char sccsid[] = "@(#)60  1.8  src/bos/kernext/mps_tok/mps_close.c, sysxmps, bos41J, 9511A_all 3/7/95 15:24:12";
/*
 *   COMPONENT_NAME: sysxmps
 *
 *   FUNCTIONS: close_cleanup
 *		mps_close
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

extern int mps_config();
extern struct cdt *cdt_func();
extern mps_dd_ctl_t mps_dd_ctl;

/*****************************************************************************/
/*
 * NAME:     mps_close
 *
 * FUNCTION: Wildwood driver close routine.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      NS user by using the ndd_close field in the NDD on the NDD chain.
 *
 * INPUT:
 *      p_ndd           - pointer to the ndd.
 *
 * RETURNS:  
 *      0 if successful
 *      errno value on failure
 */
/*****************************************************************************/
mps_close(
ndd_t   *p_ndd)              /* pointer to the ndd in the dev_ctl area */

{
  mps_dev_ctl_t   *p_dev_ctl = (mps_dev_ctl_t *)(p_ndd->ndd_correlator);
  int rc, ioa;
  ndd_statblk_t  stat_blk;   /* status block */
  int ipri;

  TRACE_SYS(MPS_OTHER, "ClsB", (ulong)p_dev_ctl, 0, 0);

  ipri = disable_lock(PL_IMP, &SLIH_LOCK);

  assert(!((p_dev_ctl->device_state == CLOSED) | 
	   (p_dev_ctl->device_state == CLOSE_PENDING)));

  if (p_dev_ctl->device_state == OPENED) {
        p_dev_ctl->device_state = CLOSE_PENDING;

        /* release the lock */
        unlock_enable(ipri, &SLIH_LOCK);

  	/*
  	 * Waits for the transmit queue to drain 
  	 */
 	 while (p_dev_ctl->device_state == CLOSE_PENDING &&
        	(WRK.tx1_frame_pending || WRK.tx2_frame_pending)) {
		io_delay(1000);
  	}

        /* get the lock again */
        ipri = disable_lock(PL_IMP, &SLIH_LOCK);
  }


  /*
   * De-activate the adapter 
   * Disables the adapter interrupts and turn on soft reset 
   */
  reset_adapter(p_dev_ctl);
  TRACE_SYS(MPS_OTHER, "Cls1", (ulong)p_dev_ctl, 0, 0);

  p_dev_ctl->device_state = CLOSED;
  p_ndd->ndd_flags = NDD_BROADCAST;

  unlock_enable(ipri, &SLIH_LOCK);

  /*
   * cleanup all the resources allocated for open 
   */
  close_cleanup(p_dev_ctl);

  lock_write(&DD_LOCK);
  cdt_del("dev_ctl", (char *)p_dev_ctl, sizeof(mps_dev_ctl_t));
  mps_dd_ctl.open_count--;
  if(!mps_dd_ctl.open_count) {
        cdt_del("dd_ctl", (char *)&mps_dd_ctl,sizeof(mps_dd_ctl_t));
        dmp_del(cdt_func);
  }
  lock_done(&DD_LOCK);

  TRACE_SYS(MPS_OTHER, "ClsE", (ulong)p_dev_ctl, 0, 0);
  unpincode(mps_config);

  return(0);
}

/*****************************************************************************/
/*
 * NAME:     close_cleanup
 *
 * FUNCTION: Wildwood driver close routine.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      mps_close
 *
 * INPUT:
 *      p_ndd           - pointer to the ndd.
 *
 * RETURNS:
 *   none
 */
/*****************************************************************************/
close_cleanup (
mps_dev_ctl_t  *p_dev_ctl)
{
  xmit_elem_t   *xelm;         /* transmit element structure */
  mps_multi_t 	*p_multi, *p_temp;
  ndd_t         *p_ndd = (ndd_t *)&(NDD);
  int		rc;

  TRACE_SYS(MPS_OTHER, "CclB", (ulong)p_dev_ctl, 0, 0);
  /*
   * Clears the watchdog timer 
   */
  if (WRK.wdt_inited) {
        w_stop (&(TX1WDT));
        w_clear (&(TX1WDT));
        w_stop (&(TX2WDT));
        w_clear (&(TX2WDT));
        w_stop (&(LANWDT));
        w_clear (&(LANWDT));
        w_stop (&(LIMWDT));
        w_clear (&(LIMWDT));
        w_stop (&(HWEWDT));
        w_clear (&(HWEWDT));
        w_stop (&(CTLWDT));
        w_clear (&(CTLWDT));
        WRK.wdt_inited = FALSE;
  }

  /*
   * De-register the interrupt handler 
   */
  if (WRK.intr_inited) {
        i_clear(&(IHS));
        WRK.intr_inited = FALSE;
  }
  /*
   * Frees the multicast table extensions 
   */
  p_multi = WRK.multi_table.next;
  while (p_multi) {
        p_temp = p_multi->next;
        xmfree(p_multi, pinned_heap);
        p_multi = p_temp;
  }

  if (WRK.new_multi) {
        xmfree(WRK.new_multi, pinned_heap);
	WRK.new_multi = NULL;
  }
  if (WRK.free_multi) {
	xmfree(WRK.free_multi, pinned_heap);
	WRK.free_multi = NULL;
  }
  WRK.multi_count  = 0;

  /*
   * Frees any resources that were allocated for the rx & tx 
   */
  if (WRK.setup) {
	TRACE_DBG(MPS_OTHER, "Ccl9", MAX_RX_LIST, 0, 0);
        recv_cleanup(p_dev_ctl, MAX_RX_LIST);
        mps_tx1_undo(p_dev_ctl);
        mps_tx2_undo(p_dev_ctl);
        WRK.setup = FALSE;
  }

  /*
   * Frees the Rx buffer queues
   */
  if (WRK.mhead) {
        m_freem(WRK.mhead);
        WRK.mhead = NULL;
  }

  /*
   * Frees the tx1 and tx2 software queues 
   */
  if (p_dev_ctl->txq1_first) {
        m_freem(p_dev_ctl->txq1_first);
        p_dev_ctl->txq1_first = NULL;
  }

  if (p_dev_ctl->txq2_first) {
        m_freem(p_dev_ctl->txq2_first);
        p_dev_ctl->txq2_first = NULL;
  }

  /*
   * Frees the DMA clannel 
   */
  if (WRK.channel_alocd) {
        d_clear (WRK.dma_channel);
        WRK.channel_alocd = FALSE;
  }

  TRACE_SYS(MPS_OTHER, "CclE", p_dev_ctl, 0, 0);

} /* end of close_cleanup */
