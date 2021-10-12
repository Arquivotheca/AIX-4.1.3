/* @(#)88       1.7  src/bos/kernext/fddi/fddiproto.h, sysxfddi, bos411, 9428A410j 3/22/94 15:50:43 */
/*
 *   COMPONENT_NAME: SYSXFDDI
 *
 *   FUNCTIONS: fddi_ctl
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
#ifndef _H_FDDIPROTO
#define _H_FDDIPROTO

/* -------------------------------------------------------------------- */
/*   Include files for whole driver   */
/* -------------------------------------------------------------------- */
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/err_rec.h>
#include <sys/lock_def.h>
#include <sys/watchdog.h>
#include <sys/trchkid.h>
#include <sys/device.h>
#include <sys/intr.h>
#include <sys/comio.h>
#include <sys/errno.h>
#include <sys/iocc.h>
#include <sys/adspace.h>
#include <sys/errno.h>
#include <sys/ioacc.h>
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/generic_mibs.h>
#include <net/spl.h>
#include <sys/cdli_fddiuser.h>
#include <sys/cdli.h>
#include <sys/fddi_demux.h>
#include "fddibits.h"
#include "fdditypes.h"
#include "fddimacro.h"
#include "fddi_errids.h"

extern time_t lbolt;              /* number ticks since last boot */

#ifndef _NO_PROTO
/* -------------------------------------------------------------------- */
/*   Function Prototypes    */
/* -------------------------------------------------------------------- */

/* config entry point */
extern int fddi_config( int cmd,
   struct uio *p_uio);

/* open entry point */
extern int fddi_open( fddi_acs_t *p_ndd);

/* close entry point */
extern int fddi_close( fddi_acs_t *p_ndd);

/* ctl entry point */
extern int fddi_ctl( fddi_acs_t *p_ndd,
  int cmd, /* ctl command */
  caddr_t arg, /* arg for this cmd, (usually struc ptr) */
  int length); /* length of the arg */

/* output entry point */
extern int fddi_output( fddi_acs_t *p_ndd,
   struct mbuf *p_mbuf);

/* retry PIO operation */
extern int fddi_pio_retry( fddi_acs_t *p_acs,
    enum pio_func iofunc,
    void  *p_ioaddr,
    long  ioparam,
    int  cnt);


extern void init_acs(fddi_acs_t *p_acs);

extern void fddi_dnld_to (struct watchdog *p_wdt);
extern void fddi_close_to (struct watchdog *p_wdt);
extern void fddi_reset_to ( struct watchdog *p_wdt);
extern void fddi_cmd_to ( struct watchdog *p_wdt);
extern int init_services( fddi_acs_t  *p_acs);
extern void free_services( fddi_acs_t  *p_acs);
extern ushort fddi_gen_crc( register uchar  *p_buf, register int l_buf);
extern struct cdt * fddi_cdt_func( int     pass );
extern void fddi_cdt_add ( char *p_name, char *ptr, int len);
extern void fddi_cdt_del ( char *ptr);
extern void disable_card ( fddi_acs_t *p_acs);
extern void undo_tx ( fddi_acs_t *p_acs);
extern void hcr_act_cmplt(fddi_acs_t *p_acs,fddi_cmd_t *p_cmd,int bus,int ipri);
extern void hcr_smt_cmplt(fddi_acs_t *p_acs,fddi_cmd_t *p_cmd,int bus,int ipri);
extern void hcr_addr_cmplt(fddi_acs_t *p_acs,fddi_cmd_t *p_cmd,int bus,
  int ipri);
extern void hcr_all_addr_cmplt(fddi_acs_t *p_acs,fddi_cmd_t *p_cmd,int  bus,
  int ipri);
extern void free_addr(fddi_acs_t *p_acs);
extern int fddi_slih (struct intr *ihs);
extern void send_cmd ( fddi_acs_t *p_acs, fddi_cmd_t  *p_cmd);
extern int issue_cmd ( fddi_acs_t *p_acs, fddi_cmd_t *p_cmd, int  bus, int  cmd_flag);
extern void hcr_uls_cmplt ( fddi_acs_t *p_acs, fddi_cmd_t *p_cmd, int  bus,
 int ipri);
extern void hcr_stat_cmplt ( fddi_acs_t  *p_acs, fddi_cmd_t *p_cmd, int  bus,
 int ipri);
extern void fddi_logerr ( fddi_acs_t *p_acs, ulong  errid, int  line, char  *p_file, 
 int  status1, int  status2, int  status3);
extern void issue_pri_cmd(fddi_acs_t *p_acs,int pri_cmd_to_issue,int bus,
  int cmd_flag);
extern void free_first_acs();

extern int fddi_dump( fddi_acs_t *p_acs, int cmd, caddr_t arg);
extern int fddi_dump_output( fddi_acs_t *p_acs, struct mbuf *p_mbuf);
extern void get_vpd( fddi_acs_t *p_acs );
extern void get_fr_xcard_vpd( fddi_acs_t *p_acs );
extern void get_sc_xcard_vpd( fddi_acs_t *p_acs );
extern void enable_multicast ( fddi_acs_t  *p_acs, fddi_cmd_t  *p_cmd);
extern void prom_on ( fddi_acs_t  *p_acs, fddi_cmd_t  *p_cmd);
extern void clear_stats( fddi_acs_t *p_acs);
extern void hcr_limbo_cmplt(fddi_acs_t *p_acs, fddi_cmd_t *p_cmd, uint  bus, int  ipri);
#else
extern int fddi_config();
extern int fddi_open();
extern int fddi_close();
extern int fddi_ctl();
extern int fddi_output();
extern int fddi_slih();
extern int fddi_pio_retry();
extern void init_acs();
extern void fddi_dnld_to ();
extern void fddi_close_to ();
extern void fddi_reset_to ();
extern void fddi_cmd_to ();
extern int init_services();
extern void free_services();
extern ushort fddi_gen_crc();
extern struct cdt * fddi_cdt_func();
extern void fddi_cdt_add ();
extern void fddi_cdt_del ();
extern void disable_card ();
extern void hcr_act_cmplt ();
extern void hcr_smt_cmplt ();
extern void hcr_addr_cmplt ();
extern void hcr_all_addr_cmplt ();
extern void free_addr();
extern void send_cmd ();
extern int issue_cmd ();
extern void hcr_uls_cmplt ();
extern void hcr_stat_cmplt ();
extern void fddi_logerr ();
extern void issue_pri_cmd ();
extern void undo_tx ();
extern void free_first_acs();
extern int fddi_dump();
extern int fddi_dump_output();
extern void get_vpd();
extern void get_fr_xcard_vpd();
extern void get_sc_xcard_vpd();
extern void enable_multicast ( );
extern void prom_on ();
extern void clear_stats();
extern void hcr_limbo_cmplt();
#endif /* end if ! _NO_PROTO */

#endif /* end if ! _H_FDDIPROTO */
