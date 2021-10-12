/* @(#)23	1.11  src/bos/kernext/tok/tokpro.h, sysxtok, bos411, 9428A410j 4/18/94 14:49:55 */
/*
 *   COMPONENT_NAME: SYSXTOK
 *
 *   FUNCTIONS: none
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

#ifndef _H_TOKPRO
#define _H_TOKPRO

/*
 *  Following are all of the include files needed by the tokdd source.
 */
#include <sys/types.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/dma.h>
#include <sys/dump.h>
#include <sys/errno.h>
#include <sys/err_rec.h>
#include <sys/file.h>
#include <sys/intr.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/atomic_op.h>
#include <sys/lock_alloc.h>
#include <sys/lockl.h>
#include <sys/lockname.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/param.h>
#include <sys/sleep.h>
#include <sys/timer.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/watchdog.h>
#include <sys/xmem.h>
#include <sys/trchkid.h>
#include <sys/adspace.h>
#include <sys/listmgr.h>
#include <sys/except.h>
#include <sys/cdli.h>
#include <sys/ndd.h>
#include <sys/tokenring_mibs.h>
#include <stddef.h>
#include <net/spl.h>
#include <sys/cdli_tokuser.h>
#include "tr_mon_dds.h"
#include "tr_mon_errids.h"
#include "tokbits.h"
#include "tokmacros.h"
#include "toktypes.h"

extern void close_adap (dds_t *p_dds);

extern void config_term (dds_t	*p_dds);

extern void save_trace( ulong	hook,
	char	*tag,
	ulong	arg1,
	ulong	arg2,
	ulong	arg3);

extern void cmd_pro(dds_t *p_dds, intr_elem_t *p_iwe);

extern void tok_receive(dds_t *p_dds, intr_elem_t *iwe);

extern void tok_recv_init(dds_t	*p_dds);

extern void tok_recv_undo(dds_t	*p_dds);

extern void arm_recv_list (dds_t *p_dds, int i);

extern void read_recv_chain (dds_t *p_dds, recv_list_t *last);

/* Transmit */
extern int tok_output(ndd_t		*p_ndd,
		      struct mbuf	*p_mbuf);

/* generate a 16-bit crc value */
extern ushort tok_gen_crc (uchar *buf,
                           int    len);

/* add an entry to the component dump table */
extern void add_cdt (char *name,
                         char *ptr,
                         int   len);

/* delete an entry from the component dump table */
extern void del_cdt (char *name,
                         char *ptr,
                         int   len);

/* component dump table function */
extern trmon_cdt_t *tok_cdt_func ( );

/*
 *  open entry point from kernel
 */
extern int tokopen(ndd_t *p_ndd);
  
/*
 *  close entry point from kernel
 */
extern int tokclose(ndd_t *p_ndd);

/*
 * Config entry point
 */
extern int tokconfig(int		cmd,
		     struct uio		*p_uio);

/*
 *  ioctl entry point from kernel
 */
extern int tokioctl(ndd_t   *p_ndd,
		    int     cmd,
		    caddr_t arg,
		    int     length);
/*
 *  dump entry point from kernel
 */
extern int tokdump(ndd_t *p_ndd, int cmd, caddr_t arg);

/*
 * Bringup Timer
 */
extern void bringup_timer(struct watchdog *p_wdt);

extern void xmit_timeout(struct watchdog *p_wdt);

/* open cleanup routine */
extern void tokopen_cleanup(dds_t	*p_dds);

/* activate ("open" and/or connect) the adapter */
extern void ds_act (dds_t *p_dds);

extern int get_adap_point(dds_t *p_dds);

extern int tokdnld(dds_t *p_dds,
		   ndd_config_t	*p_ndd_config);

extern int tokdsgetvpd(dds_t *p_dds);


extern int aca_alloc(dds_t *p_dds, int *first_open);

extern int aca_dealloc(dds_t *p_dds);

extern int get_mem(dds_t *p_dds);

extern int get_mem_undo(dds_t *p_dds);

extern void sb_setup(dds_t *p_dds);

extern int sb_undo(dds_t *p_dds);

extern void reset_adap(dds_t *p_dds);


extern int init_adap(dds_t *p_dds);


extern int open_adap(dds_t *p_dds);


extern void open_adap_pend(dds_t       *p_dds,
                          intr_elem_t  *p_iwe);

extern void open_timeout(dds_t *p_dds);

extern void logerr(dds_t *p_dds,
                   ulong errid,
		   int line,
		   char *p_fname);

extern void list_error( dds_t	*p_dds,
			intr_elem_t *p_iwe);

extern void move_tx_list( dds_t	*p_dds,
			t_tx_list *tmp_tx_list,
			t_tx_list *p_tx_list,
			t_tx_list *p_d_tx_list,
			int flag);

extern void tx_done(dds_t *p_dds,
			intr_elem_t *p_iwe);

extern int clean_rcv( dds_t *p_dds);
extern int clean_tx( dds_t *p_dds);
extern int rcv_limbo_startup( dds_t *p_dds);
extern void tx_limbo_startup( dds_t *p_dds);

extern void enter_limbo( dds_t *p_dds,
			uint   reason,
			uint   errid,
			ushort ac,
			int    got_tx_lock );

extern void kill_limbo( dds_t *p_dds );

extern void cycle_limbo( dds_t  *p_dds,
			uint   reason,
			uint   errid);

extern void egress_limbo( dds_t  *p_dds );

extern int tokslih( struct intr *ihsptr );

extern void cfg_adap_parms( dds_t *p_dds);

extern int cfg_pos_regs(dds_t *p_dds);

extern void issue_scb_command( dds_t         *p_dds,
			     uint  command,
			     uint  address);

extern void bug_out( dds_t *p_dds,
		   uint   errid,
		   uint   reason,
		   uint   subcode);

extern void hwreset ( dds_t	*p_dds);

extern void freeze_dump ( dds_t	*p_dds);

extern void get_mibs( dds_t *p_dds, token_ring_all_mib_t *arg);

extern void get_stats ( dds_t	*p_dds);

extern void ring_stat_pro( dds_t *p_dds,
			intr_elem_t *p_iwe);

#endif /* ! _H_TOKPRO */
