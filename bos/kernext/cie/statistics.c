static char sccsid[]="@(#)32   1.7  src/bos/kernext/cie/statistics.c, sysxcie, bos411, 9428A410j 4/18/94 16:22:08";

/*
 *
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 *
 * FUNCTIONS:
 *
 *   mapTokMonStats
 *   mapFddiStats
 *   mapEnt3ComStats
 *   mapEntIentStats
 *
 * DESCRIPTION:
 *
 *    Device Statistics
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <sys/types.h>
#include <sys/comio.h>
#include <sys/ciouser.h>
#include <sys/err_rec.h>
#include <sys/tokuser.h>
#include <sys/entuser.h>
#include <sys/fddiuser.h>
#include <sys/cdli.h>
#include <sys/cdli_tokuser.h>
#include <sys/cdli_entuser.h>
#include <sys/cdli_fddiuser.h>
#include <sys/tok_demux.h>
#include <sys/eth_demux.h>

#define  fddi_hdr cfddi_dmxhdr
#include <sys/fddi_demux.h>
#undef   fddi_hdr

typedef struct tok_dmx_stats tok_dmx_stats_t;
typedef struct eth_dmx_stats eth_dmx_stats_t;

#include "devstats.h"
#include "statistics.h"
#include "dmalloc.h"

/*---------------------------------------------------------------------------*/
/*               Convert Token Ring Status from CDLI to COMIO                */
/*---------------------------------------------------------------------------*/

void
   mapTokMonStats(
      register       tok_query_stats_t * t3    ,//  O-COMIO TR Statistics
      register const DEVSTATS          * ds    ,// I -CIOEM Device Statistics
      register const mon_all_stats_t   * t4    ,// I -CDLI TR Statistics
      register const nd_dmxstats_t     * dmx   ,// I -CDLI Generic Demux Stats
      register const tok_dmx_stats_t   * dmxTok // I -CDLI TR Demux Stats
   )
{
   memset(t3,0x00,sizeof(*t3));

   t3->cc.tx_byte_mcnt                      = t4->tok_ndd_stats.ndd_obytes_msw;
   t3->cc.tx_byte_lcnt                      = t4->tok_ndd_stats.ndd_obytes_lsw;
   t3->cc.rx_byte_mcnt                      = t4->tok_ndd_stats.ndd_ibytes_msw;
   t3->cc.rx_byte_lcnt                      = t4->tok_ndd_stats.ndd_ibytes_lsw;
   t3->cc.tx_frame_mcnt                     = t4->tok_ndd_stats.ndd_opackets_msw;
   t3->cc.tx_frame_lcnt                     = t4->tok_ndd_stats.ndd_opackets_lsw;
   t3->cc.rx_frame_mcnt                     = t4->tok_ndd_stats.ndd_ipackets_msw;
   t3->cc.rx_frame_lcnt                     = t4->tok_ndd_stats.ndd_ipackets_lsw;
   t3->cc.tx_err_cnt                        = t4->tok_ndd_stats.ndd_oerrors;
   t3->cc.rx_err_cnt                        = t4->tok_ndd_stats.ndd_ierrors;
   t3->cc.nid_tbl_high                      = ds->sesMax;
   t3->cc.xmt_que_high                      = t4->tok_ndd_stats.ndd_xmitque_max;
   t3->cc.rec_que_high                      = ds->rdqMax;
   t3->cc.sta_que_high                      = ds->stqMax;
   t3->ds.intr_lost                         = 0;
   t3->ds.wdt_lost                          = 0;
   t3->ds.timo_lost                         = 0;
   t3->ds.sta_que_overflow                  = ds->stqOvfl;
   t3->ds.rec_que_overflow                  = ds->rdqOvfl;
   t3->ds.rec_no_mbuf                       = t4->tok_ndd_stats.ndd_nobufs;
   t3->ds.rec_no_mbuf_ext                   = 0;
   t3->ds.recv_intr_cnt                     = t4->tok_ndd_stats.ndd_recvintr_lsw;
   t3->ds.xmit_intr_cnt                     = t4->tok_ndd_stats.ndd_xmitintr_lsw;
   t3->ds.ctr_pkt_rej_cnt                   = dmx->nd_nofilter;
   t3->ds.pkt_acc_cnt                       = t4->tok_ndd_stats.ndd_ipackets_lsw-dmx->nd_nofilter;
   t3->ds.rcv_byt_cnt                       = t4->tok_ndd_stats.ndd_ibytes_lsw;
   t3->ds.trx_byt_cnt                       = t4->tok_ndd_stats.ndd_obytes_lsw;
   t3->ds.pkt_trx_cnt                       = t4->tok_ndd_stats.ndd_opackets_lsw;
   t3->ds.ovflo_pkt_cnt                     = 0;
   t3->ds.tx_err_cnt                        = t4->tok_ndd_stats.ndd_oerrors;
   t3->ds.adap_err_log.line_err_count       = t4->tok_gen_stats.line_errs;
   t3->ds.adap_err_log.internal_err_count   = t4->tok_gen_stats.int_errs;
   t3->ds.adap_err_log.burst_err_count      = t4->tok_gen_stats.burst_errs;
   t3->ds.adap_err_log.ari_fci_err_count    = t4->tok_mon_stats.ARI_FCI_errors;
   t3->ds.adap_err_log.abort_del_err_count  = t4->tok_gen_stats.abort_errs;
   t3->ds.adap_err_log.res1                 = 0;
   t3->ds.adap_err_log.lost_frame_err_count = t4->tok_gen_stats.lostframes;
   t3->ds.adap_err_log.rec_cong_err_count   = t4->tok_gen_stats.rx_congestion;
   t3->ds.adap_err_log.frame_cpy_err_count  = t4->tok_gen_stats.framecopies;
   t3->ds.adap_err_log.res2                 = 0;
   t3->ds.adap_err_log.token_err_count      = t4->tok_gen_stats.token_errs;
   t3->ds.adap_err_log.res3                 = 0;
   t3->ds.adap_err_log.dma_bus_err_count    = t4->tok_mon_stats.DMA_bus_errors;
}

/*---------------------------------------------------------------------------*/
/*                Convert FDDI Statistics from CDLI to COMIO                 */
/*---------------------------------------------------------------------------*/

void
   mapFddiStats(
      register       fddi_query_stats_t * f3     ,//  O-COMIO FDDI Statistics
      register const DEVSTATS           * ds     ,// I -CIOEM Device Statistics
      register const fddi_ndd_stats_t   * f4     ,// I -CDLI FDDI Statistics
      register const nd_dmxstats_t      * dmx    ,// I -CDLI Generic Demux Stats
      register const fddi_dmx_stats_t   * dmxFddi // I -CDLI FDDI Demux Stats
   )
{
   memset(f3,0x00,sizeof(*f3));

   f3->cc.tx_byte_mcnt   = f4->genstats.ndd_obytes_msw;
   f3->cc.tx_byte_lcnt   = f4->genstats.ndd_obytes_lsw;
   f3->cc.rx_byte_mcnt   = f4->genstats.ndd_ibytes_msw;
   f3->cc.rx_byte_lcnt   = f4->genstats.ndd_ibytes_lsw;
   f3->cc.tx_frame_mcnt  = f4->genstats.ndd_opackets_msw;
   f3->cc.tx_frame_lcnt  = f4->genstats.ndd_opackets_lsw;
   f3->cc.rx_frame_mcnt  = f4->genstats.ndd_ipackets_msw;
   f3->cc.rx_frame_lcnt  = f4->genstats.ndd_ipackets_lsw;
   f3->cc.tx_err_cnt     = f4->genstats.ndd_oerrors;
   f3->cc.rx_err_cnt     = f4->genstats.ndd_ierrors;
   f3->cc.nid_tbl_high   = ds->sesMax;
   f3->cc.xmt_que_high   = f4->genstats.ndd_xmitque_max;
   f3->cc.rec_que_high   = ds->rdqMax;
   f3->cc.sta_que_high   = ds->stqMax;
   f3->ds.stat_que_ovflw = ds->stqOvfl;
   f3->ds.rcv_que_ovflw  = ds->rdqOvfl;
   f3->ds.tx_que_ovflw   = f4->genstats.ndd_xmitque_ovf;
   f3->ds.rcv_no_mbuf    = f4->genstats.ndd_nobufs;
   f3->ds.rcv_cmd_cmplt  = 0;
   f3->ds.rcv_intr_cnt   = f4->genstats.ndd_recvintr_lsw;
   f3->ds.tx_intr_cnt    = f4->genstats.ndd_xmitintr_lsw;
   f3->ds.pkt_rej_cnt    = dmx->nd_nofilter;
   f3->ds.ovflw_pkt_cnt  = 0;
   f3->ds.adap_err_cnt   = 0;
   f3->ls.smt_error_lo   = f4->fddistats.smt_error_lo;
   f3->ls.smt_error_hi   = f4->fddistats.smt_error_hi;
   f3->ls.smt_event_lo   = f4->fddistats.smt_event_lo;
   f3->ls.smt_event_hi   = f4->fddistats.smt_event_hi;
   f3->ls.cpv            = f4->fddistats.cpv;
   f3->ls.port_event     = f4->fddistats.port_event;
   f3->ls.setcount_lo    = f4->fddistats.setcount_lo;
   f3->ls.setcount_hi    = f4->fddistats.setcount_hi;
   f3->ls.aci_code       = f4->fddistats.aci_code;
   f3->ls.pframe_cnt     = f4->fddistats.pframe_cnt;
   f3->ls.ecm_sm         = f4->fddistats.ecm_sm;
   f3->ls.pcm_a_sm       = f4->fddistats.pcm_a_sm;
   f3->ls.pcm_b_sm       = f4->fddistats.pcm_b_sm;
   f3->ls.cfm_a_sm       = f4->fddistats.cfm_a_sm;
   f3->ls.cfm_b_sm       = f4->fddistats.cfm_b_sm;
   f3->ls.cf_sm          = f4->fddistats.cf_sm;
   f3->ls.mac_cfm_sm     = f4->fddistats.mac_cfm_sm;
   f3->ls.rmt_sm         = f4->fddistats.rmt_sm;
   f3->ls.sba_alloc_lo   = 0;
   f3->ls.sba_alloc_hi   = 0;
   f3->ls.tneg_lo        = 0;
   f3->ls.tneg_hi        = 0;
   f3->ls.payload_lo     = 0;
   f3->ls.payload_hi     = 0;
   f3->ls.overhead_lo    = 0;
   f3->ls.overhead_hi    = 0;
   f3->ls.res1[7]        = 0;
   f3->ls.ucode_ver      = 0x0201;
   f3->ls.res2           = 0;
}

/*---------------------------------------------------------------------------*/
/*            Convert Ethernet 3Com Statistics from CDLI to COMIO            */
/*---------------------------------------------------------------------------*/

void
   mapEnt3ComStats(
      register       ent_query_stats_t  * e3    ,//  O-COMIO 3Com Ent Statistics
      register const DEVSTATS           * ds    ,// I -CIOEM Device Statistics
      register const en3com_all_stats_t * e4    ,// I -CDLI 3Com Statistics
      register const nd_dmxstats_t      * dmx   ,// I -CDLI Generic Demux Stats
      register const eth_dmx_stats_t    * dmxEnt // I -CDLI 3Com Demux Stats
   )
{
   memset(e3,0x00,sizeof(*e3));

   e3->cc.tx_byte_mcnt     = e4->ent_ndd_stats.ndd_obytes_msw;
   e3->cc.tx_byte_lcnt     = e4->ent_ndd_stats.ndd_obytes_lsw;
   e3->cc.rx_byte_mcnt     = e4->ent_ndd_stats.ndd_ibytes_msw;
   e3->cc.rx_byte_lcnt     = e4->ent_ndd_stats.ndd_ibytes_lsw;
   e3->cc.tx_frame_mcnt    = e4->ent_ndd_stats.ndd_opackets_msw;
   e3->cc.tx_frame_lcnt    = e4->ent_ndd_stats.ndd_opackets_lsw;
   e3->cc.rx_frame_mcnt    = e4->ent_ndd_stats.ndd_ipackets_msw;
   e3->cc.rx_frame_lcnt    = e4->ent_ndd_stats.ndd_ipackets_lsw;
   e3->cc.tx_err_cnt       = e4->ent_ndd_stats.ndd_oerrors;
   e3->cc.rx_err_cnt       = e4->ent_ndd_stats.ndd_ierrors;
   e3->cc.nid_tbl_high     = ds->sesMax;
   e3->cc.xmt_que_high     = e4->ent_ndd_stats.ndd_xmitque_max;
   e3->cc.rec_que_high     = ds->rdqMax;
   e3->cc.sta_que_high     = ds->stqMax;
   e3->ds.intr_lost        = 0;
   e3->ds.wdt_lost         = 0;
   e3->ds.sta_que_overflow = ds->stqOvfl;
   e3->ds.rec_que_overflow = ds->rdqOvfl;
   e3->ds.rec_no_mbuf      = e4->ent_ndd_stats.ndd_nobufs;
   e3->ds.rec_no_mbuf_ext  = 0;
   e3->ds.recv_intr_cnt    = e4->ent_ndd_stats.ndd_recvintr_lsw;
   e3->ds.xmit_intr_cnt    = e4->ent_ndd_stats.ndd_xmitintr_lsw;
   e3->ds.crc_error        = e4->ent_gen_stats.fcs_errs;
   e3->ds.align_error      = e4->ent_gen_stats.align_errs;
   e3->ds.overrun          = e4->ent_gen_stats.overrun;
   e3->ds.too_short        = e4->ent_gen_stats.short_frames;
   e3->ds.too_long         = e4->ent_gen_stats.long_frames;
   e3->ds.no_resources     = e4->ent_gen_stats.no_resources;
   e3->ds.pckts_discard    = e4->ent_gen_stats.rx_drop;
   e3->ds.max_collision    = e4->ent_gen_stats.excess_collisions;
   e3->ds.late_collision   = e4->ent_gen_stats.late_collisions;
   e3->ds.carrier_lost     = e4->ent_gen_stats.carrier_sense;
   e3->ds.underrun         = e4->ent_gen_stats.underrun;
   e3->ds.cts_lost         = e4->ent_gen_stats.cts_lost;
   e3->ds.xmit_timeouts    = e4->ent_gen_stats.tx_timeouts;
   e3->ds.par_err_cnt      = 0;
   e3->ds.diag_over_flow   = 0;
   e3->ds.exec_over_flow   = 0;
   e3->ds.exec_cmd_errors  = 0;
   e3->ds.host_rec_eol     = e4->en3com_stats.host_rcv_eol;
   e3->ds.adpt_rec_eol     = e4->en3com_stats.adpt_rcv_eol;
   e3->ds.adpt_rec_pack    = e4->en3com_stats.adpt_rcv_pack;
   e3->ds.host_rec_pack    = 0;
   e3->ds.start_recp_cmd   = e4->ent_gen_stats.start_rx;
   e3->ds.rec_dma_to       = e4->en3com_stats.rcv_dma_to;
   e3->ds.reserved[0]      = e4->en3com_stats.reserved[0];
   e3->ds.reserved[1]      = e4->en3com_stats.reserved[1];
   e3->ds.reserved[2]      = e4->en3com_stats.reserved[2];
   e3->ds.reserved[3]      = e4->en3com_stats.reserved[3];
   e3->ds.reserved[4]      = e4->en3com_stats.reserved[4];
}

/*---------------------------------------------------------------------------*/
/*         Convert Integrated Ethernet Statistics from CDLI to COMIO         */
/*---------------------------------------------------------------------------*/

void
   mapEntIentStats(
      register       ent_query_stats_t * e3    ,//  O-COMIO Ient Statistics
      register const DEVSTATS          * ds    ,// I -Emulator Device Statistics
      register const ient_all_stats_t  * e4    ,// I -CDLI Ient Statistics
      register const nd_dmxstats_t     * dmx   ,// I -CDLI Generic Demux Stats
      register const eth_dmx_stats_t   * dmxEnt // I -CDLI Ient Demux Stats
   )
{
   memset(e3,0x00,sizeof(*e3));

   e3->cc.tx_byte_mcnt     = e4->ent_ndd_stats.ndd_obytes_msw;
   e3->cc.tx_byte_lcnt     = e4->ent_ndd_stats.ndd_obytes_lsw;
   e3->cc.rx_byte_mcnt     = e4->ent_ndd_stats.ndd_ibytes_msw;
   e3->cc.rx_byte_lcnt     = e4->ent_ndd_stats.ndd_ibytes_lsw;
   e3->cc.tx_frame_mcnt    = e4->ent_ndd_stats.ndd_opackets_msw;
   e3->cc.tx_frame_lcnt    = e4->ent_ndd_stats.ndd_opackets_lsw;
   e3->cc.rx_frame_mcnt    = e4->ent_ndd_stats.ndd_ipackets_msw;
   e3->cc.rx_frame_lcnt    = e4->ent_ndd_stats.ndd_ipackets_lsw;
   e3->cc.tx_err_cnt       = e4->ent_ndd_stats.ndd_oerrors;
   e3->cc.rx_err_cnt       = e4->ent_ndd_stats.ndd_ierrors;
   e3->cc.nid_tbl_high     = ds->sesMax;
   e3->cc.xmt_que_high     = e4->ent_ndd_stats.ndd_xmitque_max;
   e3->cc.rec_que_high     = ds->rdqMax;
   e3->cc.sta_que_high     = ds->stqMax;
   e3->ds.intr_lost        = 0;
   e3->ds.wdt_lost         = 0;
   e3->ds.sta_que_overflow = ds->stqOvfl;
   e3->ds.rec_que_overflow = ds->rdqOvfl;
   e3->ds.rec_no_mbuf      = e4->ent_ndd_stats.ndd_nobufs;
   e3->ds.rec_no_mbuf_ext  = 0;
   e3->ds.recv_intr_cnt    = e4->ent_ndd_stats.ndd_recvintr_lsw;
   e3->ds.xmit_intr_cnt    = e4->ent_ndd_stats.ndd_xmitintr_lsw;
   e3->ds.crc_error        = e4->ent_gen_stats.fcs_errs;
   e3->ds.align_error      = e4->ent_gen_stats.align_errs;
   e3->ds.overrun          = e4->ent_gen_stats.overrun;
   e3->ds.too_short        = e4->ent_gen_stats.short_frames;
   e3->ds.too_long         = e4->ent_gen_stats.long_frames;
   e3->ds.no_resources     = e4->ent_gen_stats.no_resources;
   e3->ds.pckts_discard    = e4->ent_gen_stats.rx_drop;
   e3->ds.max_collision    = e4->ent_gen_stats.excess_collisions;
   e3->ds.late_collision   = e4->ent_gen_stats.late_collisions;
   e3->ds.carrier_lost     = e4->ent_gen_stats.carrier_sense;
   e3->ds.underrun         = e4->ent_gen_stats.underrun;
   e3->ds.cts_lost         = e4->ent_gen_stats.cts_lost;
   e3->ds.xmit_timeouts    = e4->ent_gen_stats.tx_timeouts;
   e3->ds.par_err_cnt      = 0;
   e3->ds.diag_over_flow   = 0;
   e3->ds.exec_over_flow   = 0;
   e3->ds.exec_cmd_errors  = 0;
   e3->ds.host_rec_eol     = 0;
   e3->ds.adpt_rec_eol     = 0;
   e3->ds.adpt_rec_pack    = 0;
   e3->ds.host_rec_pack    = 0;
   e3->ds.start_recp_cmd   = e4->ent_gen_stats.start_rx;
   e3->ds.rec_dma_to       = 0;
}
