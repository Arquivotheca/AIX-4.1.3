static char sccsid[] = "@(#)78  1.3  src/bos/usr/lbin/en3com/en3com_dmp.c, sysxent, bos411, 9428A410j 5/10/94 17:27:26";
/*
 * COMPONENT_NAME: sysxent -- High Performance Ethernet Device Driver
 *
 * FUNCTIONS: 
 *		dmp_entry
 *		fmt_dd_ctl
 *		fmt_dev_ctl
 *		fmt_ndd
 *		fmt_dds
 *		fmt_wrk
 *		hex_dmp
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/lock_def.h>
#include <sys/intr.h>
#include <sys/watchdog.h>
#include <sys/intr.h>
#include <sys/err_rec.h>
#include <sys/mbuf.h>
#include <sys/dump.h>
#include <sys/ndd.h>
#include <sys/cdli.h>
#include <sys/ethernet_mibs.h>
#include <sys/cdli_entuser.h>

#include "en3com_dds.h"
#include "en3com_mac.h"
#include "en3com_hw.h"
#include "en3com.h"

en3com_dd_ctl_t *dmp_dd_ctl;	/* pointer to a dynamic allocated area */
en3com_dev_ctl_t dmp_dev_ctl;	/* global structure for the dev_ctl dump */

#define DMPNDD dmp_dev_ctl.ndd
#define NSTATS dmp_dev_ctl.ndd.ndd_genstats
#define DMPDDS dmp_dev_ctl.dds
#define DMPWRK dmp_dev_ctl.wrk



/*****************************************************************************/
/*
 * NAME:     dmp_entry
 *
 * FUNCTION: 3COM Ethernet driver dump formatter.
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
 *	p_cdt_entry	- point to the cdt entry saved in the global table.
 *
 * RETURNS:  
 *	0 - OK
 *	-1 - Can't read the dump file
 */
/*****************************************************************************/
dmp_entry(kmem, p_cdt_entry)
int kmem;
struct cdt_entry *p_cdt_entry;

{

  char          *d_ptr;
  char          *name;
  int           len;

  /*
   * get values from the cdt_entry
   */
  name  = p_cdt_entry->d_name;
  len   = p_cdt_entry->d_len;
  d_ptr = p_cdt_entry->d_ptr;


  /* If the dump entry name is "dd_ctl", this is a en3com_dd_ctl structure.*/
  /* We dynamically allocated a piece of memory for this area because the  */
  /* trace table in this structure may have different size depends on the  */
  /* driver has the DEBUG flag turned on or not when it was built.	   */
  if (!strcmp(name, "dd_ctl")) {
        if ((dmp_dd_ctl = (en3com_dd_ctl_t *)malloc(len)) == NULL) {
                return(-1);
        }
	if (read(kmem, dmp_dd_ctl, len) != len) {
		return(-1);
	}

	fmt_dd_ctl(len);
        free(dmp_dd_ctl);
	return(0);
  }

  /* If the dump entry name is "dev_ctl", this is a en3com_dev_ctl structure.*/
  if (!strcmp(name, "dev_ctl")) {
	if (read(kmem, &dmp_dev_ctl, sizeof(dmp_dev_ctl)) != 
		sizeof(dmp_dev_ctl)) {
		return(-1);
	}

	fmt_dev_ctl(d_ptr);
	return(0);
  }

  return(0);

}

/*****************************************************************************/
/*
 * NAME:     fmt_dd_ctl
 *
 * FUNCTION: The format routine for the en3com_dd_ctl structure.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      dmp_entry
 *
 * INPUT:
 *	len	- length of the dumped area.
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
fmt_dd_ctl(len)
int len;

{
	
  char *p;


  printf("\n<< Dump of en3com_dd_ctl >> :\n");
  printf("cfg_lock: %08x        dd_lock: %08x\n", dmp_dd_ctl->cfg_lock, 
	dmp_dd_ctl->dd_clock);
  printf("p_dev_list: %08x      num_dev: %08x     open_count: %08x\n", 
	dmp_dd_ctl->p_dev_list, dmp_dd_ctl->num_devs, dmp_dd_ctl->open_count);
  
  /* Calculate the trace table length in case the driver was compiled 	*/
  /* with a DEBUG trace table. This calculation will dump the trace   	*/
  /* table according to the dump length, no matter what the driver    	*/
  /* trace table is DEBUG or non-DEBUG version.				*/
  printf("trace - (struct en3com_trace):");
  len -= sizeof(en3com_dd_ctl_t);
  len += sizeof(en3com_trace_t);
  hex_dmp("", dmp_dd_ctl->trace, len);

  printf("cdt - (struct en3com_cdt):");
  p = (char *)dmp_dd_ctl;
  p += len;
  hex_dmp("", p, sizeof(en3com_cdt_t));

}

/*****************************************************************************/
/*
 * NAME:     fmt_dev_ctl
 *
 * FUNCTION: The format routine for the en3com_dev_ctl structure.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      dmp_entry
 *
 * INPUT:
 *	d_ptr	- real memory address of the dumped entry.
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
fmt_dev_ctl(d_ptr)
char *d_ptr;

{
	
  printf("\n<< Dump of en3com_dev_ctl [%08x] >> :\n", d_ptr);
  printf("ihs - (struct intr):");
  hex_dmp("", &dmp_dev_ctl.ihs, sizeof(struct intr));

  printf("ctl_correlator: %08x\n", dmp_dev_ctl.ctl_correlator);
  fmt_ndd();

  printf("next: %08x          seq_number: %08x\n", 
	dmp_dev_ctl.next, dmp_dev_ctl.seq_number);
  printf("ctl_clock: %08x     cmd_slock: %08x\n", dmp_dev_ctl.ctl_clock, 
	dmp_dev_ctl.cmd_slock);
  printf("tx_slock: %08x      slih_slock: %08x\n", dmp_dev_ctl.tx_slock,
	dmp_dev_ctl.slih_slock);

  printf("device_state:");
  if (dmp_dev_ctl.device_state == CLOSED)
	printf(" CLOSED");
  if (dmp_dev_ctl.device_state == DEAD)
	printf(" DEAD");
  if (dmp_dev_ctl.device_state == LIMBO)
	printf(" LIMBO");
  if (dmp_dev_ctl.device_state == OPEN_PENDING)
	printf(" OPEN_PENDING");
  if (dmp_dev_ctl.device_state == OPENED)
	printf(" OPENED");
  if (dmp_dev_ctl.device_state == CLOSE_PENDING)
	printf(" CLOSE_PENDING");

  printf("    limbo_state:");
  if (dmp_dev_ctl.limbo_state == NO_LIMBO)
	printf(" NO_LIMBO");
  if (dmp_dev_ctl.limbo_state == LIMBO_RESET)
	printf(" LIMBO_RESET");
  if (dmp_dev_ctl.limbo_state == LIMBO_RESET_DONE)
	printf(" LIMBO_RESET_DONE");
  if (dmp_dev_ctl.limbo_state == LIMBO_GET_MBOX_1)
	printf(" LIMBO_GET_MBOX_1");
  if (dmp_dev_ctl.limbo_state == LIMBO_GET_MBOX_2)
	printf(" LIMBO_GET_MBOX_2");
  if (dmp_dev_ctl.limbo_state == LIMBO_GET_MBOX_3)
	printf(" LIMBO_GET_MBOX_3");
  if (dmp_dev_ctl.limbo_state == LIMBO_GET_MBOX_4)
	printf(" LIMBO_GET_MBOX_4");
  if (dmp_dev_ctl.limbo_state == LIMBO_SET_EN)
	printf(" LIMBO_SET_EN");
  if (dmp_dev_ctl.limbo_state == LIMBO_AL_LOC_OFF)
	printf(" LIMBO_AL_LOC_OFF");
  if (dmp_dev_ctl.limbo_state == LIMBO_CONFIGURE)
	printf(" LIMBO_CONFIGURE");
  if (dmp_dev_ctl.limbo_state == LIMBO_SET_ADDR)
	printf(" LIMBO_SET_ADDR");
  if (dmp_dev_ctl.limbo_state == LIMBO_CONFIG_LIST)
	printf(" LIMBO_CONFIG_LIST");
  if (dmp_dev_ctl.limbo_state == LIMBO_REPORT_CONFIG)
	printf(" LIMBO_REPORT_CONFIG");
  if (dmp_dev_ctl.limbo_state == LIMBO_NO_FILTER)
	printf(" LIMBO_NO_FILTER");

  printf("\nndd_stime: %08x     dev_stime: %08x\n", dmp_dev_ctl.ndd_stime,
	dmp_dev_ctl.dev_stime);
  printf("txq_len: %08x       txq_first: %08x      txq_last: %08x\n",
	dmp_dev_ctl.txq_len, dmp_dev_ctl.txq_first, dmp_dev_ctl.txq_last);
  printf("tx_pending: %08x\n", dmp_dev_ctl.tx_pending);
  printf("ctl_status: %08x    ctl_pending: %08x    ctl_event: %08x\n",
	dmp_dev_ctl.ctl_status, dmp_dev_ctl.ctl_pending, dmp_dev_ctl.ctl_event);

  fmt_dds();

  printf("tx_wdt - (struct watchdog):");
  hex_dmp("", &dmp_dev_ctl.tx_wdt, sizeof(struct watchdog));

  printf("ctl_wdt - (struct watchdog):");
  hex_dmp("", &dmp_dev_ctl.ctl_wdt, sizeof(struct watchdog));

  printf("systimer: %08x\n", dmp_dev_ctl.systimer);

  printf("vpd - (struct en3com_vpd):");
  hex_dmp("", &dmp_dev_ctl.vpd, sizeof(en3com_vpd_t));

  printf("entstats - (struct ent_genstats):");
  hex_dmp("", &dmp_dev_ctl.entstats, sizeof(ent_genstats_t));

  printf("devstats - (struct en3com_stats):");
  hex_dmp("", &dmp_dev_ctl.devstats, sizeof(en3com_stats_t));

  printf("mibs - (struct ethernet_all_mib):");
  hex_dmp("", &dmp_dev_ctl.mibs, sizeof(ethernet_all_mib_t));

  fmt_wrk();




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
 *      fmt_dev_ctl 
 *
 * INPUT:
 *	none
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
fmt_ndd()
{


  printf("ndd - (struct ndd):\n");
  printf("  ndd_next: %08x    ndd_refcnt: %08x\n", 
    DMPNDD.ndd_next, DMPNDD.ndd_refcnt);
  printf("  ndd_name: %08x    ndd_alias: %08x\n",
    DMPNDD.ndd_name, DMPNDD.ndd_alias);

  printf("  ndd_flags: %08x  (");
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
  if (DMPNDD.ndd_flags & ENT_RCV_BAD_FRAME) 
	printf(" ENT_RCV_BAD_FRAME");
  printf(")\n");

  printf("  ndd_correlator: %08x\n", DMPNDD.ndd_correlator);
  printf("  ndd_open: %08x    ndd_close: %08x    ndd_output: %08x\n",
    DMPNDD.ndd_open, DMPNDD.ndd_close, DMPNDD.ndd_output);
  printf("  ndd_ctl: %08x     nd_receive: %08x   nd_status: %08x\n", 
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
  hex_dmp("  ", &DMPNDD.ndd_reserved, sizeof(DMPNDD.ndd_reserved));
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
  hex_dmp("    ", &NSTATS.ndd_stat_reserved, sizeof(NSTATS.ndd_stat_reserved));

}

/*****************************************************************************/
/*
 * NAME:     fmt_dds
 *
 * FUNCTION: The format routine for the en3com_dds structure.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      fmt_dev_ctl 
 *
 * INPUT:
 *	none
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
fmt_dds()
{


  printf("dds - (struct en3com_dds):\n");
  printf("  bus_type: %08x            bus_id: %08x\n", DMPDDS.bus_type, 
	DMPDDS.bus_id);
  printf("  intr_level: %08x          intr_priority: %08x\n", DMPDDS.intr_level, 
	DMPDDS.intr_priority);
  printf("  xmt_que_size: %08x\n", DMPDDS.xmt_que_size);
  printf("  lname: %-16s       alias: %-16s\n", DMPDDS.lname, DMPDDS.alias);
  printf("  bus_mem_addr: %08x        bus_mem_size: %08x\n",
    DMPDDS.bus_mem_addr, DMPDDS.bus_mem_size);
  printf("  tcw_bus_mem_addr: %08x    tcw_bus_mem_size: %08x\n",
     DMPDDS.tcw_bus_mem_addr, DMPDDS.tcw_bus_mem_size);
  printf("  io_port: %08x             slot: %08x\n",
     DMPDDS.io_port, DMPDDS.slot);
  printf("  dma_arbit_lvl: %08x       use_alt_addr: %08x\n",
     DMPDDS.dma_arbit_lvl, DMPDDS.use_alt_addr);
  printf("  alt_addr:");
  hex_dmp("  ", &DMPDDS.alt_addr, sizeof(DMPDDS.alt_addr));
  printf("  rv_pool_size: %08x        bnc_select: %08x\n",
     DMPDDS.rv_pool_size, DMPDDS.bnc_select);




}

/*****************************************************************************/
/*
 * NAME:     fmt_wrk
 *
 * FUNCTION: The format routine for the en3com_wrk structure.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      fmt_dev_ctl 
 *
 * INPUT:
 *	none
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
fmt_wrk()
{


  printf("wrk - (struct en3com_wrk):\n");
  printf("  net_addr:");
  hex_dmp("  ", &DMPWRK.net_addr, sizeof(DMPWRK.net_addr));
  printf("  vpd_na:");
  hex_dmp("  ", &DMPWRK.vpd_na, sizeof(DMPWRK.vpd_na));
  printf("  vpd_ros:");
  hex_dmp("  ", &DMPWRK.vpd_rosl, sizeof(DMPWRK.vpd_rosl));
  printf("  vpd_pn:");
  hex_dmp("  ", &DMPWRK.vpd_pn, sizeof(DMPWRK.vpd_pn));
  printf("  vpd_ec:");
  hex_dmp("  ", &DMPWRK.vpd_ec, sizeof(DMPWRK.vpd_ec));
  printf("  vpd_dd:");
  hex_dmp("  ", &DMPWRK.vpd_dd, sizeof(DMPWRK.vpd_dd));
  printf("  vpd_ros_length: %04x     vpd_hex_rosl: %04x     vpd_hex_dd: %04x\n",
    DMPWRK.vpd_ros_length, DMPWRK.vpd_hex_rosl, DMPWRK.vpd_hex_dd);

  printf("  main_offset: %08x    rv_mail_box: %08x  tx_mail_box: %08x\n",
    DMPWRK.main_offset, DMPWRK.rv_mail_box, DMPWRK.tx_mail_box);
  printf("  exec_mail_box: %08x  stat_count_off: %08x\n",
    DMPWRK.exec_mail_box, DMPWRK.stat_count_off);
  printf("  adpt_ram_size: %04x      buf_des_reg_size: %04x\n",
    DMPWRK.adpt_ram_size, DMPWRK.buf_des_reg_size);
  printf("  tx_list_off: %08x    rv_list_off: %08x\n",
    DMPWRK.tx_list_off, DMPWRK.rv_list_off);
  printf("  tx_list_cnt: %04x        rv_list_cnt: %04x\n",
    DMPWRK.tx_list_cnt, DMPWRK.rv_list_cnt);
  printf("  version_num: %04x\n",
    DMPWRK.version_num);

  printf("  tx_tcw_base: %08x    rv_tcw_base: %08x\n",
    DMPWRK.tx_tcw_base, DMPWRK.rv_tcw_base);
  printf("  txd_cnt: %04x       rvd_cnt: %04x\n",
    DMPWRK.txd_cnt, DMPWRK.rvd_cnt);
  printf("  tx_buf: %08x    rv_buf: %08x\n",
    DMPWRK.tx_buf, DMPWRK.rv_buf);
  printf("  txbuf_xmem - (struct xmem):");
  hex_dmp("  ", &DMPWRK.txbuf_xmem, sizeof(struct xmem));
  printf("  rvbuf_xmem - (struct xmem):");
  hex_dmp("  ", &DMPWRK.rvbuf_xmem, sizeof(struct xmem));

  printf("  pos_reg:");
  hex_dmp("  ", &DMPWRK.pos_reg, sizeof(DMPWRK.pos_reg));
  printf("  dma_channel: %08x    dma_fair: %08x        dma_addr_burst: %08x\n",
    DMPWRK.dma_channel, DMPWRK.dma_fair, DMPWRK.dma_addr_burst);
  printf("  channel_alocd: %08x  tx_buf_alocd: %08x    rv_buf_alocd: %08x\n",
    DMPWRK.channel_alocd, DMPWRK.tx_buf_alocd, DMPWRK.rv_buf_alocd);
  printf("  pos_parity: %08x     fdbk_intr_en: %08x    intr_inited: %08x\n",
    DMPWRK.pos_parity, DMPWRK.fdbk_intr_en, DMPWRK.intr_inited);
  printf("  tx_wdt_inited: %08x  ctl_wdt_inited: %08x  gate_array_fix: %x\n",
    DMPWRK.tx_wdt_inited, DMPWRK.ctl_wdt_inited, DMPWRK.gate_array_fix);
  printf("  card_type: %08x      vpd_chk_flags: %08x   restart_count: %08x\n",
    DMPWRK.card_type, DMPWRK.vpd_chk_flags, DMPWRK.restart_count);
  printf("  promiscuous_count: %08x\n", DMPWRK.promiscuous_count);
  printf("  badframe_count: %08x        otherstatus: %08x\n",
    DMPWRK.badframe_count, DMPWRK.otherstatus);
  printf("  multi_promis_mode: %08x     enable_multi: %08x\n",
    DMPWRK.multi_promis_mode, DMPWRK.enable_multi);
  printf("  filter_count: %08x          multi_count: %08x\n",
    DMPWRK.filter_count, DMPWRK.multi_count);
  printf("  multi_table - (struct en3com_multi):");
  hex_dmp("  ", &DMPWRK.multi_table, sizeof(en3com_multi_t));

  printf("  txd_first: %08x    txd_last: %08x    txd_avail: %08x\n",
    DMPWRK.txd_first, DMPWRK.txd_last, DMPWRK.txd_avail);
  printf("  rvd_first: %08x    rvd_last: %08x\n", DMPWRK.rvd_first, 
    DMPWRK.rvd_last);
  printf("  txd - (list of struct en3com_bdc):");
  hex_dmp("  ", &DMPWRK.txd, sizeof(DMPWRK.txd));
  printf("  rvd - (list of struct en3com_bdc):");
  hex_dmp("  ", &DMPWRK.rvd, sizeof(DMPWRK.rvd));
  



}

/*****************************************************************************/
/*
 * NAME:     hex_dmp
 *
 * FUNCTION: Format a block of data in a hex with ascii format.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      fmt_dd_ctl 
 *	fmt_dev_ctl
 *	fmt_ndd
 *	fmt_dds
 *	fmt_wrk
 *
 * INPUT:
 *	pad	- a string for output indentation 
 *	adr	- starting address of the data area
 *	len	- length of the data area to be formatted
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
hex_dmp(pad, adr, len)
char *pad;
char *adr;
int len;

{
  
  int i, j, offset;
  char *dp;
  ulong *lp;
  char buf[20];



  printf("\n");

  offset = 0;
  dp = adr;
  buf[0] = '|';

  /*
   * Format 16 bytes of data per line with its hex offset in the front.
   * The buf is used as the ascii string buffer for the ascii display 
   * on the right.
   */
  while (len >= 16) {
	j = 1;		/* index for buf */
  	for (i = 1; i <= 16; i++) {
		if (*dp >= 0x20 && *dp <= 0x7e) 
			buf[j++] = *dp;
		else
			buf[j++] = '.';
		dp++;
	}
	buf[j++] = '|';
	buf[j] = '\0';

	lp = (ulong *)adr;
	printf("%s%08x: %08x %08x %08x %08x    %s\n", pad, offset, *lp, 
		*(lp + 1), *(lp + 2), *(lp + 3), buf);
		
	len -= 16;
	offset += 16;
	adr += 16;
  }
  
  /*
   * Format the last line with less than 16 bytes of data.
   */
  if (len) {
  	j = 1;		/* index for buf */
  	lp = (ulong *)adr;
  	printf("%s%08x: ", pad, offset);
  	for (i = 1; i <= len; i++) {
		if (*dp >= 0x20 && *dp <= 0x7e)  
			buf[j++] = *dp;
		else
			buf[j++] = '.';
		printf("%02x", *dp);
		dp++;
		if (i % 4 == 0)
			printf(" ");
  	}

  	for (; i < 16; i++) {
  		printf("  ");
		buf[j++] = ' ';
		if (i % 4 == 0)
			printf(" ");
	}
  	buf[j++] = ' ';
  	buf[j++] = '|';
  	buf[j] = '\0';

  	printf("      ");
  	printf("%s\n", buf);

  }
		
		
}
