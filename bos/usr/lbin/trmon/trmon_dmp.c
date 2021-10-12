static char sccsid[] = "@(#)74	1.3  src/bos/usr/lbin/trmon/trmon_dmp.c, sysxtok, bos411, 9428A410j 5/28/94 12:23:49";
/*
 *   COMPONENT_NAME: SYSXTOK
 *
 *   FUNCTIONS: dmp_entry
 *		fmt_aca
 *		fmt_dd_ctrl
 *		fmt_ddi
 *		fmt_dds
 *		fmt_ndd
 *		fmt_wrk
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

#include <sys/types.h>
#include <sys/lock_def.h>
#include <sys/intr.h>
#include <sys/watchdog.h>
#include <sys/err_rec.h>
#include <sys/param.h>
#include <sys/dump.h>
#include <sys/xmem.h>
#include <sys/ndd.h>
#include <sys/cdli.h>
#include <sys/tokenring_mibs.h>
#include <sys/cdli_tokuser.h>

#include "tr_mon_dds.h"
#include "tokbits.h"
#include "tokmacros.h"
#include "toktypes.h"

extern char *malloc();

#define DMPNDD dmp_dds->ndd
#define NSTATS dmp_dds->ndd.ndd_genstats
#define DMPDDI dmp_dds->ddi
#define DMPTOK dmp_dds->tokstats
#define DMPDEV dmp_dds->devstats
#define DMPWRK dmp_dds->wrk



/*****************************************************************************/
/*
 * NAME:     dmp_entry
 *
 * FUNCTION: Monterey Token-Ring driver dump formatter.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      main 
 *
 * INPUT:
 *      kmem		- the fd of the dump file. 
 *	name		- the character string of the dump entry name
 *	len		- length for this dump entry
 *
 * RETURNS:  
 *	0 - OK
 *	-1 - Can't read the dump file
 */
/*****************************************************************************/
dmp_entry(kmem, p_cdt_entry)
  int		   kmem;
  struct cdt_entry *p_cdt_entry;
{

  char		*d_ptr;
  char		*name;
  int		len;
  dd_ctrl_t	*dmp_dd_ctrl;
  char		*dmp_aca;
  dds_t		*dmp_dds;

  /*
   * get values from the cdt_entry
   */
  name  = p_cdt_entry->d_name;
  len   = p_cdt_entry->d_len;
  d_ptr = p_cdt_entry->d_ptr;

  /*
   * dump entry name is "dd_ctrl"
   * the dd_ctrl length varies because the trace table has varying lengths
   */
  if (!strcmp(name, "dd_ctrl")) {
	if ((dmp_dd_ctrl = (dd_ctrl_t *)malloc(len)) == NULL) {
		return(-1);
	}
	if (read(kmem, dmp_dd_ctrl, len) != len) {
		return(-1);
	}
	fmt_dd_ctrl(dmp_dd_ctrl, d_ptr, len);
	free(dmp_dd_ctrl);
	return(0);
  }

  /*
   * dump entry name is "DDS"
   */
  if (!strcmp(name, "DDS")) {
	if ((dmp_dds = (dds_t *)malloc(len)) == NULL) {
		return(-1);
	}
	if (read(kmem, dmp_dds, len) != len) { 
		return(-1);
	}
	fmt_dds(dmp_dds, d_ptr, len);
	return(0);
  }

  /*
   * dump entry name is "ACA"
   */
  if (!strcmp(name, "ACA")) {
	if ((dmp_aca = (char *)malloc(len)) == NULL) {
		return(-1);
	}
	if (read(kmem, dmp_aca, len) != len) {
		return(-1);
	}
	fmt_aca(dmp_aca, d_ptr);
	free(dmp_aca);
	return(0);
  }

  return(0);
}
/*****************************************************************************/
/*
 * NAME:     fmt_dd_ctrl
 *
 * FUNCTION: The format routine for the dd_ctrl structure.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * CALLED FROM:
 *      dmp_entry 
 *
 * INPUT:
 *	dmp_dd_ctrl - dd_ctrl structure
 *	d_ptr	    - address of dd_ctrl in dumped machine
 *	len	    - length of dd_ctrl in this dump
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
fmt_dd_ctrl(dmp_dd_ctrl, d_ptr, len)
  dd_ctrl_t *dmp_dd_ctrl;
  char	    *d_ptr;
  int       len;
{
  int   trace_len;
	
  printf("\n<< Dump of dd_ctrl (%08x) >> :\n", d_ptr);

  printf("mon_cfg_lock: %08x    dd_slock: %08x\n",
	dmp_dd_ctrl->mon_cfg_lock, dmp_dd_ctrl->dd_slock);
  printf("p_dds_head: %08x      num_devs: %08x     num_opens: %08x\n\n", 
	dmp_dd_ctrl->p_dds_head, dmp_dd_ctrl->num_devs,
	dmp_dd_ctrl->num_opens);

  printf("cdt - (struct trmon_cdt):");
  hex_dmp("  ", &dmp_dd_ctrl->cdt, sizeof(trmon_cdt_t));
  
  /*
   * The size of the trace table will vary.  It is assumed that this routine
   * was compiled with the minimum size trace.  So take the size of the
   * dd_ctrl_t structure (which will normally be the length of this area
   * being dumped) and subtract the size of the trace table.  That is the
   * size of everything else.  Subtract that from the length of the area
   * being dumped and that is the length of the trace table this time.
   * (also subtract 8 for the other two fields in trmon_trace_t) 
   */
  trace_len = (len - (sizeof(dd_ctrl_t) - sizeof(trmon_trace_t)) -8);

  printf("\n");
  printf("trace.table - (struct trmon_trace):\n");
  printf("  trace_slock: %08x   next_entry: %08x\n",
	dmp_dd_ctrl->trace.trace_slock, dmp_dd_ctrl->trace.next_entry);
  printf("  trace.table (NEXT is next entry):");
  hex_dmp("    ", &dmp_dd_ctrl->trace.table, trace_len);

}

/*****************************************************************************/
/*
 * NAME:     fmt_dds
 *
 * FUNCTION: The format routine for the dds_t structure.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      dmp_entry 
 *
 * INPUT:
 *	none
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
fmt_dds(dmp_dds, d_ptr, len)
  dds_t *dmp_dds;
  char  *d_ptr;
  int   len;
{
	
  printf("\n<< Dump of device control area (DDS) (%08x) >> :\n", d_ptr);

  printf("Interrupt handler - (struct intr):");
  hex_dmp("  ", &dmp_dds->ihs, sizeof(struct intr));

  printf("\n");
  printf("next: %08x          seq_number: %08x\n", 
	dmp_dds->next, dmp_dds->seq_number);
  printf("dds_correlator: %08x\n", dmp_dds->dds_correlator);

  fmt_ndd(dmp_dds);

  fmt_ddi(dmp_dds);

  printf("\n");
  printf("VPD - (struct tok_vpd):");
  hex_dmp("  ", &dmp_dds->vpd, sizeof(tok_vpd_t));

  printf("\n");
  printf("bu_wdt - (struct watchdog):");
  hex_dmp("  ", &dmp_dds->bu_wdt, sizeof(struct watchdog));

  printf("\n");
  printf("xmit_wdt - (struct watchdog):");
  hex_dmp("  ", &dmp_dds->xmit_wdt, sizeof(struct watchdog));

  printf("\n");
  printf("tokstats - (struct tok_genstats):\n");
  printf("  device_type: %08x      dev_elapsed_time: %08x\n",
    DMPTOK.device_type, DMPTOK.dev_elapsed_time);
  printf("  ndd_flags: %08x\n", DMPTOK.ndd_flags);
  printf("  tok_nadr:");
  hex_dmp("    ", DMPTOK.tok_nadr, CTOK_NADR_LENGTH);
  printf("  mcast_xmit: %08x       bcast_xmit: %08x\n",
    DMPTOK.mcast_xmit, DMPTOK.bcast_xmit);
  printf("  mcast_recv: %08x       bcast_recv: %08x\n",
    DMPTOK.mcast_recv, DMPTOK.bcast_recv);
  printf("  line_errs: %08x        burst_errs: %08x\n",
    DMPTOK.line_errs, DMPTOK.burst_errs);
  printf("  ac_errs: %08x          abort_errs: %08x\n",
    DMPTOK.ac_errs, DMPTOK.abort_errs);
  printf("  int_errs: %08x         lostframes: %08x\n",
    DMPTOK.int_errs, DMPTOK.lostframes);
  printf("  rx_congestion: %08x    framecopies: %08x\n",
    DMPTOK.rx_congestion, DMPTOK.framecopies);
  printf("  token_errs: %08x       soft_errs: %08x\n",
    DMPTOK.token_errs, DMPTOK.soft_errs);
  printf("  hard_errs: %08x        signal_loss: %08x\n",
    DMPTOK.hard_errs, DMPTOK.signal_loss);
  printf("  tx_beacons: %08x       recoverys: %08x\n",
    DMPTOK.tx_beacons, DMPTOK.recoverys);
  printf("  lobewires: %08x        removes: %08x\n",
    DMPTOK.lobewires, DMPTOK.removes);
  printf("  singles: %08x          freq_errs: %08x\n",
    DMPTOK.singles, DMPTOK.freq_errs);
  printf("  tx_timeouts: %08x      sw_txq_len: %08x\n",
    DMPTOK.tx_timeouts, DMPTOK.sw_txq_len);
  printf("  hw_txq_len: %08x       reserve1: %08x\n",
    DMPTOK.hw_txq_len, DMPTOK.reserve1);
  printf("  reserve2: %08x         reserve3: %08x\n",
    DMPTOK.reserve2, DMPTOK.reserve3);
  printf("  reserve4: %08x\n", DMPTOK.reserve4);

  printf("\n");
  printf("devstats - (struct tr_mon_stats):\n");
  printf("  ARI_FCI_errors: %08x   DMA_bus_errors: %08x\n",
    DMPDEV.ARI_FCI_errors, DMPDEV.DMA_bus_errors);
  printf("  DMA_parity_errors: %08x\n", DMPDEV.DMA_parity_errors);

  fmt_wrk(dmp_dds, len);

}

/*****************************************************************************/
/*
 * NAME:     fmt_ndd
 *
 * FUNCTION: The format routine for the ndd structure.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      fmt_dds 
 *
 * INPUT:
 *	none
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
fmt_ndd(dmp_dds)
  dds_t *dmp_dds;
{

  printf("\n");
  printf("ndd - (struct ndd):\n");
  printf("  ndd_next: %08x    ndd_refcnt: %08x\n", 
    DMPNDD.ndd_next, DMPNDD.ndd_refcnt);
  printf("  ndd_name: %08x    ndd_alias: %08x\n",
    DMPNDD.ndd_name, DMPNDD.ndd_alias);

  printf("  ndd_flags: %08x  (", DMPNDD.ndd_flags);
  if (DMPNDD.ndd_flags & NDD_UP) 
	printf(" UP");
  if (DMPNDD.ndd_flags & NDD_BROADCAST) 
	printf(" BROADCAST");
  if (DMPNDD.ndd_flags & NDD_DEBUG) 
	printf(" DEBUG");
  if (DMPNDD.ndd_flags & NDD_RUNNING) 
	printf(" RUNNING");
  if (DMPNDD.ndd_flags & NDD_SIMPLEX) 
	printf(" SIMPLEX");
  if (DMPNDD.ndd_flags & NDD_DEAD) 
	printf(" DEAD");
  if (DMPNDD.ndd_flags & NDD_LIMBO) 
	printf(" LIMBO");
  if (DMPNDD.ndd_flags & NDD_PROMISC) 
	printf(" PROMISC");
  if (DMPNDD.ndd_flags & NDD_ALTADDRS) 
	printf(" ALTADDRS");
  if (DMPNDD.ndd_flags & NDD_MULTICAST) 
	printf(" MULTICAST");
  if (DMPNDD.ndd_flags & NDD_DETACHED) 
	printf(" DETACHED");
  if (DMPNDD.ndd_flags & TOK_ATTENTION_MAC) 
	printf(" TOK_ATTENTION_MAC");
  if (DMPNDD.ndd_flags & TOK_BEACON_MAC) 
	printf(" TOK_BEACON_MAC");
  if (DMPNDD.ndd_flags & TOK_RECEIVE_FUNC) 
	printf(" TOK_RECEIVE_FUNC");
  if (DMPNDD.ndd_flags & TOK_RECEIVE_GROUP) 
	printf(" TOK_RECEIVE_GROUP");
  if (DMPNDD.ndd_flags & TOK_RING_SPEED_4) 
	printf(" TOK_RING_SPEED_4");
  if (DMPNDD.ndd_flags & TOK_RING_SPEED_16) 
	printf(" TOK_RING_SPEED_16");
  printf(")\n");

  printf("  ndd_correlator: %08x\n", DMPNDD.ndd_correlator);
  printf("  ndd_open: %08x       ndd_close: %08x    ndd_output: %08x\n",
    DMPNDD.ndd_open, DMPNDD.ndd_close, DMPNDD.ndd_output);
  printf("  ndd_ctl: %08x        nd_receive: %08x   nd_status: %08x\n", 
    DMPNDD.ndd_ctl, DMPNDD.nd_receive, DMPNDD.nd_status);
  printf("  ndd_mtu: %08x        ndd_mintu: %08x\n", DMPNDD.ndd_mtu, 
    DMPNDD.ndd_mintu);
  printf("  ndd_type: %08x       ndd_addrlen: %08x\n",
    DMPNDD.ndd_type, DMPNDD.ndd_addrlen);
  printf("  ndd_hdrlen: %08x     ndd_physaddr: %08x\n",
    DMPNDD.ndd_hdrlen, DMPNDD.ndd_physaddr);
  printf("  ndd_demuxer: %08x    ndd_nsdemux: %08x\n",
    DMPNDD.ndd_demuxer, DMPNDD.ndd_nsdemux);
  printf("  ndd_specdemux: %08x  ndd_demuxsource: %08x  ndd_demux_lock: %08x\n",
    DMPNDD.ndd_specdemux, DMPNDD.ndd_demuxsource, DMPNDD.ndd_demux_lock);
  printf("  ndd_trace: %08x      ndd_trace_arg: %08x    ndd_lock: %08x\n",
    DMPNDD.ndd_trace, DMPNDD.ndd_trace_arg, DMPNDD.ndd_lock);
  printf("  ndd_reserved:");
  hex_dmp("    ", &DMPNDD.ndd_reserved, sizeof(DMPNDD.ndd_reserved));
  printf("  ndd_specstats: %08x    ndd_speclen: %08x\n",
    DMPNDD.ndd_specstats, DMPNDD.ndd_speclen);

  printf("  ndd_genstats - (struct ndd_genstats):\n");
  printf("    elapsed_time: %08x\n", NSTATS.ndd_elapsed_time);
  printf("    ipackets_msw: %08x    ipackets_lsw: %08x\n",
    NSTATS.ndd_ipackets_msw, NSTATS.ndd_ipackets_lsw);
  printf("    ibytes_msw: %08x      ibytes_lsw: %08x\n",
    NSTATS.ndd_ibytes_msw, NSTATS.ndd_ibytes_lsw);
  printf("    recvintr_msw: %08x    recvintr_lsw: %08x    ierrors: %08x\n",
    NSTATS.ndd_recvintr_msw, NSTATS.ndd_recvintr_lsw, NSTATS.ndd_ierrors);
  printf("    opackets_msw: %08x    opackets_lsw: %08x\n",
    NSTATS.ndd_opackets_msw, NSTATS.ndd_opackets_lsw);
  printf("    obytes_msw: %08x      obytes_lsw: %08x\n",
    NSTATS.ndd_obytes_msw, NSTATS.ndd_obytes_lsw);
  printf("    xmitintr_msw: %08x    xmitintr_lsw: %08x    oerrors: %08x\n",
    NSTATS.ndd_xmitintr_msw, NSTATS.ndd_xmitintr_lsw, NSTATS.ndd_oerrors);
  printf("    nobufs: %08x          xmitque_max: %08x     xmitque_ovf: %08x\n",
    NSTATS.ndd_nobufs, NSTATS.ndd_xmitque_max, NSTATS.ndd_xmitque_ovf);
  printf("    ipackets_drop: %08x   ibadpackets: %08x\n",
    NSTATS.ndd_ipackets_drop, NSTATS.ndd_ibadpackets);
  printf("    opackets_drop: %08x   xmitque_cur: %08x\n",
    NSTATS.ndd_opackets_drop, NSTATS.ndd_xmitque_cur);
  printf("    stat_reserved:");
  hex_dmp("      ", &NSTATS.ndd_stat_reserved,
	sizeof(NSTATS.ndd_stat_reserved));

}

/*****************************************************************************/
/*
 * NAME:     fmt_ddi
 *
 * FUNCTION: The format routine for the tr_mon_dds structure.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      fmt_dds
 *
 * INPUT:
 *	none
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
fmt_ddi(dmp_dds)
  dds_t *dmp_dds;
{

  printf("\n");
  printf("config to device driver structure (dds) - (struct tr_mon_dds):\n");
  printf("  lname: %-16s       alias: %-16s\n", DMPDDI.lname, DMPDDI.alias);
  printf("  bus_type: %08x            bus_id: %08x\n",
	DMPDDI.bus_type, DMPDDI.bus_id);
  printf("  intr_level: %08x          intr_priority: %08x\n",
	DMPDDI.intr_level, DMPDDI.intr_priority);
  printf("  tcw_bus_mem_addr: %08x    dma_arbit_lvl: %08x\n",
	DMPDDI.tcw_bus_mem_addr, DMPDDI.dma_arbit_lvl);
  printf("  io_port: %08x             slot: %08x\n",
	DMPDDI.io_port, DMPDDI.slot);
  printf("  xmt_que_size: %08x        ring_speed: %08x\n",
	DMPDDI.xmt_que_size, DMPDDI.ring_speed);
  printf("  use_alt_addr: %08x        attn_mac: %08x\n",
     DMPDDI.use_alt_addr, DMPDDI.attn_mac);
  printf("  beacon_mac: %08x\n", DMPDDI.beacon_mac);
  printf("  alt_addr:");
  hex_dmp("    ", &DMPDDI.alt_addr, sizeof(DMPDDI.alt_addr));

}

/*****************************************************************************/
/*
 * NAME:     fmt_wrk
 *
 * FUNCTION: The format routine for the dds_wrk structure.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      fmt_dds 
 *
 * INPUT:
 *	none
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
fmt_wrk(dmp_dds, len)
  dds_t *dmp_dds;
  int	len;
{

  int	queue_len;

  printf("\n");
  printf("wrk - (struct dds_wrk):\n");
  printf("  tx_slock: %08x         slih_slock: %08x\n",
     DMPWRK.tx_slock, DMPWRK.slih_slock);
  printf("  adap_state: %08x", DMPWRK.adap_state);
  if (DMPWRK.adap_state == DEAD_STATE) 
	printf(" (DEAD_STATE)\n");
  if (DMPWRK.adap_state == NULL_STATE) 
	printf(" (NULL_STATE)\n");
  if (DMPWRK.adap_state == OPEN_PENDING) 
	printf(" (OPEN_PENDING)\n");
  if (DMPWRK.adap_state == OPEN_STATE) 
	printf(" (OPEN_STATE)\n");
  if (DMPWRK.adap_state == CLOSED_STATE) 
	printf(" (CLOSED_STATE)\n");
  if (DMPWRK.adap_state == CLOSE_PENDING) 
	printf(" (CLOSE_PENDING)\n");
  if (DMPWRK.adap_state == LIMBO_STATE) 
	printf(" (LIMBO_STATE)\n");

  printf("  limbo: %08x", DMPWRK.limbo);
  if (DMPWRK.limbo == PARADISE) 
	printf(" (PARADISE)\n");
  if (DMPWRK.limbo == CHAOS) 
	printf(" (CHAOS)\n");
  if (DMPWRK.limbo == PROBATION) 
	printf(" (PROBATION)\n");
  if (DMPWRK.limbo == LIMBO_KILL_PEND) 
	printf(" (LIMBO_KILL_PEND)\n");
  if (DMPWRK.limbo == NO_OP_STATE) 
	printf(" (NO_OP_STATE)\n");

  printf("  bringup: %08x", DMPWRK.bringup);
  if (DMPWRK.bringup == RESET_PHASE0) 
	printf(" (RESET_PHASE0)\n");
  if (DMPWRK.bringup == RESET_PHASE1) 
	printf(" (RESET_PHASE1)\n");
  if (DMPWRK.bringup == ADAP_INIT_PHASE0) 
	printf(" (ADAP_INIT_PHASE0)\n");
  if (DMPWRK.bringup == ADAP_INIT_PHASE1) 
	printf(" (ADAP_INIT_PHASE1)\n");
  if (DMPWRK.bringup == OPEN_PHASE0) 
	printf(" (OPEN_PHASE0)\n");

  printf("  open_status: %08x\n", DMPWRK.open_status);
  printf("  ndd_stime: %08x        dev_stime: %08x\n",
     DMPWRK.ndd_stime, DMPWRK.dev_stime);
  printf("  do_dkmove %08x", DMPWRK.do_dkmove);
  if (DMPWRK.do_dkmove) {
	printf(" (Do d_kmove)\n");
  } else {
	printf(" (Do bcopy)\n");
  }
  printf("  connect_sb_sent: %08x  bcon_sb_sent: %08x\n",
     DMPWRK.connect_sb_sent, DMPWRK.bcon_sb_sent);
  printf("  rr_entry: %08x         rr_errid: %08x\n",
     DMPWRK.rr_entry, DMPWRK.rr_errid);
  printf("  limcycle: %08x         pio_errors: %08x\n",
     DMPWRK.limcycle, DMPWRK.pio_errors);
  printf("  piox: %08x\n", DMPWRK.piox);
  printf("  pio_rc: %08x           pio_addr: %08x\n",
     DMPWRK.pio_rc, DMPWRK.pio_addr);
  printf("  mcerr: %08x            footprint: %08x\n",
     DMPWRK.mcerr, DMPWRK.footprint);
  printf("  bugout: %08x           afoot: %08x\n",
     DMPWRK.bugout, DMPWRK.afoot);
  printf("  open_fail_code: %08x   ring_status: %08x\n",
     DMPWRK.open_fail_code, DMPWRK.ring_status);
  printf("  dma_chnl_id: %08x      alloc_size: %08x\n",
     DMPWRK.dma_chnl_id, DMPWRK.alloc_size);

  printf("  ac_blk - (struct adap_check_blk_t):");
  hex_dmp("    ", &DMPWRK.ac_blk, sizeof(adap_check_blk_t));
  printf("  limbo_iwe - (struct intr_elem_t):");
  hex_dmp("    ", &DMPWRK.limbo_iwe, sizeof(intr_elem_t));
  printf("  adap_err_log - (struct tok_adap_error_log_t):");
  hex_dmp("    ", &DMPWRK.adap_err_log, sizeof(tok_adap_error_log_t));
  printf("  ring_info - (struct tok_ring_info_t):");
  hex_dmp("    ", &DMPWRK.ring_info, sizeof(tok_ring_info_t));
  printf("  adap_addr - struct (tok_adap_addr_t):");
  hex_dmp("    ", &DMPWRK.adap_addr, sizeof(tok_adap_addr_t));
  printf("  func_addr_ref:");
  hex_dmp("    ", &DMPWRK.func_addr_ref, sizeof(DMPWRK.func_addr_ref));

  printf("  p_mem_block: %08x      p_d_mem_block: %08x\n",
     DMPWRK.p_mem_block, DMPWRK.p_d_mem_block);
  printf("  mem_block_xmd - (struct xmem):");
  hex_dmp("    ", &DMPWRK.mem_block_xmd, sizeof(DMPWRK.mem_block_xmd));

  printf("  p_scb: %08x            p_d_scb: %08x\n",
     DMPWRK.p_scb, DMPWRK.p_d_scb);
  printf("  p_ssb: %08x            p_d_ssb: %08x\n",
     DMPWRK.p_ssb, DMPWRK.p_d_ssb);
  printf("  p_prod_id: %08x        p_d_prod_id: %08x\n",
     DMPWRK.p_prod_id, DMPWRK.p_d_prod_id);
  printf("  p_errlog: %08x         p_d_errlog: %08x\n",
     DMPWRK.p_errlog, DMPWRK.p_d_errlog);
  printf("  p_ring_info: %08x      p_d_ring_info: %08x\n",
     DMPWRK.p_ring_info, DMPWRK.p_d_ring_info);
  printf("  p_adap_addr: %08x      p_d_adap_addr: %08x\n",
     DMPWRK.p_adap_addr, DMPWRK.p_d_adap_addr);

  printf("\n");
  printf("  xmit_buf: %08x\n", DMPWRK.xmit_buf);
  printf("  xbuf_xd - (struct xmem):");
  hex_dmp("    ", &DMPWRK.xbuf_xd, sizeof(DMPWRK.xbuf_xd));
  printf("  tx_buf_size: %08x      tx_buf_count: %08x\n",
     DMPWRK.tx_buf_size, DMPWRK.tx_buf_count);
  printf("  tx_buf_use_count: %08x tx_buf_next_in: %08x\n",
     DMPWRK.tx_buf_use_count, DMPWRK.tx_buf_next_in);

  printf("  tx_buf_des - (struct xmit_dest_t):");
  hex_dmp("    ", &DMPWRK.tx_buf_des, sizeof(DMPWRK.tx_buf_des));
  printf("  tx_list_use_count: %08x tx_list_next_in: %08x\n",
     DMPWRK.tx_list_use_count, DMPWRK.tx_list_next_in);

  printf("  p_d_tx_fwds:");
  hex_dmp("    ", &DMPWRK.p_d_tx_fwds, sizeof(DMPWRK.p_d_tx_fwds));
  printf("  p_tx_head: %08x        p_d_tx_head: %08x\n",
     DMPWRK.p_tx_head, DMPWRK.p_d_tx_head);
  printf("  p_tx_tail: %08x        p_d_tx_tail: %08x\n",
     DMPWRK.p_tx_tail, DMPWRK.p_d_tx_tail);
  printf("  p_tx_1st_update: %08x  p_d_tx_1st_update: %08x\n",
     DMPWRK.p_tx_1st_update, DMPWRK.p_d_tx_1st_update);
  printf("  p_tx_next_avail: %08x  p_d_tx_next_avail: %08x\n",
     DMPWRK.p_tx_next_avail, DMPWRK.p_d_tx_next_avail);
  printf("  xmit_queue: %08x       tx_que_next_buf: %08x\n",
     DMPWRK.xmit_queue, DMPWRK.tx_que_next_buf);
  printf("  tx_que_next_in: %08x   tx_que_next_out: %08x\n",
     DMPWRK.tx_que_next_in, DMPWRK.tx_que_next_out);
  printf("  xmits_queued: %08x     xmits_adapter: %08x\n",
     DMPWRK.xmits_queued, DMPWRK.xmits_adapter);
  printf("  tx_iwe - (struct intr_elem_t):");
  hex_dmp("    ", &DMPWRK.tx_iwe, sizeof(intr_elem_t));
  printf("  issue_tx_cmd: %08x\n", DMPWRK.issue_tx_cmd);

  printf("\n");
  printf("  recv_iwe - (struct intr_elem_t):");
  hex_dmp("    ", &DMPWRK.recv_iwe, sizeof(intr_elem_t));
  printf("  recv_mode: %08x        read_index: %08x\n",
     DMPWRK.recv_mode, DMPWRK.read_index);
  printf("  recv_list:");
  hex_dmp("    ", &DMPWRK.recv_list, sizeof(DMPWRK.recv_list));
  printf("  recv_vadr:");
  hex_dmp("    ", &DMPWRK.recv_vadr, sizeof(DMPWRK.recv_vadr));
  printf("  recv_addr:");
  hex_dmp("    ", &DMPWRK.recv_addr, sizeof(DMPWRK.recv_addr));
  printf("  recv_mbuf:");
  hex_dmp("    ", &DMPWRK.recv_mbuf, sizeof(DMPWRK.recv_mbuf));

  printf("  cfg_pos:");
  hex_dmp("    ", &DMPWRK.cfg_pos, sizeof(DMPWRK.cfg_pos));
  printf("  tok_vpd_addr:");
  hex_dmp("    ", &DMPWRK.tok_vpd_addr, sizeof(DMPWRK.tok_vpd_addr));
  printf("  tok_vpd_rosl:");
  hex_dmp("    ", &DMPWRK.tok_vpd_rosl, sizeof(DMPWRK.tok_vpd_rosl));
  printf("  tok_vpd_mclvl:");
  hex_dmp("    ", &DMPWRK.tok_vpd_mclvl, sizeof(DMPWRK.tok_vpd_mclvl));

  printf("  adap_iparms - (struct tok_adap_i_parms_t):");
  hex_dmp("    ", &DMPWRK.adap_iparms, sizeof(DMPWRK.adap_iparms));
  printf("  adap_open_opts - (struct tok_adap_open_options_t):");
  hex_dmp("    ", &DMPWRK.adap_open_opts, sizeof(DMPWRK.adap_open_opts));
  printf("  p_open_opts: %08x      p_d_open_opts: %08x\n",
     DMPWRK.p_open_opts, DMPWRK.p_d_open_opts);

  printf("  xmit_wdt_inited: %08x  xmit_wdt_active: %08x\n",
     DMPWRK.xmit_wdt_inited, DMPWRK.xmit_wdt_active);
  printf("  xmit_wdt_opackets: %08x bu_wdt_inited: %08x\n",
     DMPWRK.xmit_wdt_opackets, DMPWRK.bu_wdt_inited);
  printf("  bu_wdt_cmd: %08x       event_wait: %08x\n",
     DMPWRK.bu_wdt_cmd, DMPWRK.event_wait);
  printf("  funct_event: %08x      group_event: %08x\n",
     DMPWRK.funct_event, DMPWRK.group_event);
  printf("  elog_event: %08x       read_adap_event: %08x\n",
     DMPWRK.elog_event, DMPWRK.read_adap_event);
  printf("  command_to_do: %08x    reset_spin: %08x\n",
     DMPWRK.command_to_do, DMPWRK.reset_spin);
  printf("  adap_init_spin: %08x   close_event: %08x\n",
     DMPWRK.adap_init_spin, DMPWRK.close_event);
  printf("  p_a_ucode_lvl: %08x    p_a_addrs: %08x\n",
     DMPWRK.p_a_ucode_lvl, DMPWRK.p_a_addrs);
  printf("  p_a_parms: %08x        mask_int: %08x\n",
     DMPWRK.p_a_parms, DMPWRK.mask_int);
  printf("  dump_read_started: %08x dump_pri: %08x\n",
     DMPWRK.dump_read_started, DMPWRK.dump_pri);
  printf("  sav_index: %08x        dump_first_wrt: %08x\n",
     DMPWRK.sav_index, DMPWRK.dump_first_wrt);
  printf("  dump_read_last: %08x   freeze_dump: %08x\n",
     DMPWRK.dump_read_last, DMPWRK.freeze_dump);
  printf("  freeze_data: %08x\n", DMPWRK.freeze_data);

  /*
   * The transmit queue is allocated with the DDS and imediately follows
   * it.  It's length varies based upon the xmt_que_size.
   */
  queue_len = len - sizeof(dds_t);
  printf("  Transmit Queue - (struct xmt_elem_t):");
  hex_dmp("    ", (int)dmp_dds + sizeof(dds_t), queue_len);

}

/*****************************************************************************/
/*
 * NAME:     fmt_aca
 *
 * FUNCTION: The format routine for the ACA structure.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      dmp_entry 
 *
 * INPUT:
 *	dmp_aca - ACA structure
 *	d_ptr	- address of dd_aca in dumped machine
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
fmt_aca(dmp_aca, d_ptr)
  char	*dmp_aca;
  char	*d_ptr;
{


  printf("\n<< Dump of adapter control area (ACA) (%08x) >> :\n", d_ptr);

  printf("SCB - (struct t_scb):");
  hex_dmp("  ", dmp_aca + ACA_SCB_BASE, sizeof(t_scb));

  printf("\n");
  printf("SSB - (struct t_ssb):");
  hex_dmp("  ", dmp_aca + ACA_SSB_BASE, sizeof(t_ssb));

  printf("\n");
  printf("Product ID - (struct tok_prod_id_t):");
  hex_dmp("  ", dmp_aca + ACA_PROD_ID_BASE, sizeof(tok_prod_id_t));

  printf("\n");
  printf("Adapter error log - (struct tok_adap_error_log_t):");
  hex_dmp("  ", dmp_aca + ACA_ADAP_ERR_LOG_BASE, sizeof(tok_adap_error_log_t));

  printf("\n");
  printf("Ring info - (struct tok_ring_info_t):");
  hex_dmp("  ", dmp_aca + ACA_RING_INFO_BASE, sizeof(tok_ring_info_t));

  printf("\n");
  printf("Adapter addresses - (struct tok_adap_addr_t):");
  hex_dmp("  ", dmp_aca + ACA_ADAP_ADDR_BASE, sizeof(tok_adap_addr_t));

  printf("\n");
  printf("Adapter Open Parameters - (struct tok_adap_open_options_t):");
  hex_dmp("  ", dmp_aca + ACA_OPEN_BLOCK_BASE,
	sizeof(tok_adap_open_options_t));

  printf("\n");
  printf("Receive Chain - (struct recv_list_t):");
  hex_dmp("  ", dmp_aca + ACA_RCV_CHAIN_BASE,
	(RCV_CHAIN_SIZE * sizeof(recv_list_t)));

  printf("\n");
  printf("Transmit Chain - (struct t_tx_list):");
  hex_dmp("  ", dmp_aca + ACA_TX_CHAIN_BASE,
	(TX_CHAIN_SIZE * sizeof(t_tx_list)));

}
