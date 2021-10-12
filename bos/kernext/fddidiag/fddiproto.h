/* @(#)89       1.2  src/bos/kernext/fddidiag/fddiproto.h, diagddfddi, bos411, 9428A410j 3/7/94 10:26:47 */
/*
 *   COMPONENT_NAME: DIAGDDFDDI
 *
 *   FUNCTIONS: ASSERT
 *		fddi_ioctl
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1990,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_FDDIPROTO
#define _H_FDDIPROTO

/* --------------------------------------------------------------------	*/
/*			Include files for whole driver			*/
/* --------------------------------------------------------------------	*/
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/err_rec.h>
#include <sys/lock_def.h>
#include <sys/watchdog.h>
#include <sys/device.h>
#include <sys/intr.h>
#include <sys/comio.h>
#include "diagddfddiuser.h"
#include "diagddfddidds.h"
#include "fddibits.h"
#include "fdditypes.h"
#include "fddimacro.h"
/*
 * Asserts defined with debug
 */
#ifdef FDDI_DEBUG
#	define ASSERT(c)	if (!(c)) brkpoint();
#else
#	define ASSERT(c)
#endif

/* --------------------------------------------------------------------	*/
/*			Function Prototypes				*/
/* --------------------------------------------------------------------	*/

#ifndef _NO_PROTO
/* config entry point */
extern int fddidiag_config(	dev_t	devno,
			int	cmd,
			struct uio	*p_uio);

extern int fddi_config_init( 	dev_t		devno,
				struct uio	*p_uio,
				int		rc);

extern int fddi_config_term( 	int	adap );

/* config query vital product data */
extern int fddi_config_qvpd(	int		adap,
				struct uio	*p_uio);

/* configure pos registers */
extern int cfg_pos_regs(fddi_acs_t	*p_acs);

/* get vital product data */
extern int get_vpd(fddi_acs_t	*p_acs);

/* generate VPD crc */
extern ushort fddi_gen_crc (uchar *p_buf, int l_buf);

/* mpx entry point */
extern int fddi_mpx(	dev_t	devno,
			int	*p_chan,
			char	*p_channame);

/* open entry point */
extern int fddi_open(	dev_t		devno,
			ulong		devflag,
			chan_t		chan,
			cio_kopen_ext_t	*p_ext);

extern int fddi_1st_dd_open();
extern int fddi_undo_1st_dd_open();

extern int fddi_1st_acs_open (fddi_acs_t	*p_acs, dev_t	devno);
extern int fddi_undo_1st_acs_open (fddi_acs_t	*p_acs);

/* initialize an open element */
extern void fddi_init_open_elem(	fddi_acs_t	*p_acs,
					fddi_open_t	*p_open,
					ulong		devflag,
					cio_kopen_ext_t	*p_ext);
/*
 * The timeout function for transmits
 */
extern void	fddi_tx_to (struct watchdog *p_wdt);
extern void	fddi_cmd_to (struct watchdog *p_wdt);
extern void	fddi_dnld_to (struct watchdog *p_wdt);

/* close entry point */
extern int fddi_close(	dev_t	devno,
			chan_t	chan);

extern int fddi_remove_netids(	fddi_acs_t	*p_acs,		/* ACS ptr */
				fddi_open_t	*p_open);	/* open ptr */


extern void fddi_free_rcvq(	fddi_acs_t	*p_acs,		/* ACS ptr */
				fddi_open_t	*p_open);	/* open ptr */
/* ioctl entry point */
extern int fddi_ioctl( dev_t devno,		/* major and minor number */
		int cmd,	/* ioctl operation */
		int arg,	/* arg for this cmd, (usually struc ptr) */
		ulong devflag,/* flags */
		chan_t chan,	/* mpx channel num */
		int ext);	/* optional additional arg */

/* write entry point */
extern int fddi_write(	dev_t		devno,
			struct uio	*p_uio,
			chan_t		chan,
			cio_write_ext_t	*p_ext);

/* read entry point */
extern int fddi_read(	dev_t		devno,
			struct uio	*p_uio,
			chan_t		chan,
			cio_read_ext_t	*p_ext);

/* select entry point */
extern int fddi_select(	dev_t	devno,
			ushort	events,
			ushort	*p_revent,
			chan_t	chan);

/* second level interrupt handler and offlevel handler */
extern int fddi_slih(	fddi_acs_t	*p_acs);
extern int fddi_oflv(	struct	ihs	*p_ihs);


/* report status */
extern void fddi_report_status(	fddi_acs_t	*p_acs,
				fddi_open_t	*p_open,
				cio_stat_blk_t	*p_stat);

/* report asynchronous status */
extern void fddi_async_status(	fddi_acs_t	*p_acs,
				cio_stat_blk_t	*p_stat);

/* report connnection done */
extern void fddi_conn_done(	fddi_acs_t	*p_acs,
				int		status);

/* retry PIO operation */
extern int fddi_pio_retry(	fddi_acs_t	*p_acs,
			int		excpt_code,
			enum pio_func	iofunc,
			void		*p_ioaddr,
			long		ioparam,
			int		cnt);

extern void fddi_close_wait (struct watchdog *p_wdt);
extern void fddi_limbo_act (struct watchdog *p_wdt);

/* internal trace routine */
extern void fddi_trace (register uchar	str[],	/* trace data Footprint */
			register ulong	arg2,	/* trace data */
			register ulong	arg3,	/* trace data */
			register ulong	arg4);	/* trace data */

extern int fddi_mem_acc(fddi_acs_t 	*p_acs,	/* ACS ptr */
			fddi_open_t	*p_open,/* Open element ptr */
			fddi_mem_acc_t	*p_arg,	/* Memory access struct */
			dev_t		devflag);	/* devflag */

extern int fddi_dma_acc(fddi_acs_t	*p_acs,	/* ACS ptr */
			fddi_open_t	*p_open,/* Open element ptr */
			fddi_mem_acc_t	*p_mem);/* Mem acces structure */

extern int fddi_do_dma(	fddi_acs_t	*p_acs,	/* ACS ptr */
			ushort		icr,	/* icr command */
			uint		cnt,	/* # of transfer locations */
			uint		offset);	/* RAM offset */

extern int fddi_get_trace(fddi_acs_t	*p_acs,	/* ACS ptr */
			fddi_open_t	*p_open,/* Open element ptr */
			fddi_get_trace_t *p_arg,/* Memory access struct */
			dev_t		devflag);	/* devflag */

extern int fddi_act_cmplt(fddi_acs_t	*p_acs,	/* ACS ptr */
			fddi_cmd_t	*p_cmd,	/* CMD blk ptr */
			int		bus);	/* bus attach value */

		
extern void fddi_logerr (fddi_acs_t	*p_acs,	/* ACS ptr */
			ulong		errid,	/* Error ID to log */
			int		line,	/* line number */
			char		*p_file); /* file name */

extern int fddi_get_stat(
	fddi_acs_t	*p_acs, 
	fddi_open_t	*p_open, 
	cio_stat_blk_t	*p_arg,
	ulong		devflag);

extern void fddi_cdt_add (
	char	*p_name,
	char	*ptr,
	int	len);

extern int fddi_get_query(
	fddi_acs_t		*p_acs, 
	struct query_parms	*p_query,
	int			len, 
	ulong			devflag);

extern void fddi_start_netid(
	fddi_acs_t	*p_acs, 
	fddi_open_t	*p_open, 
	netid_t		netid);

extern void fddi_halt_netid(
	fddi_acs_t	*p_acs, 
	fddi_open_t	*p_open, 
	netid_t		netid);

extern void fddi_clean_swque(
	fddi_acs_t 	*p_acs,
	fddi_open_t	*p_open);


extern void fddi_cdt_del (char	*ptr);
extern int fddi_cdt_undo_init ();
extern struct cdt * fddi_cdt_func();

/* #ifdef WHY_BROKEN */
extern void fddi_enter_limbo(
			fddi_acs_t	*p_acs,	/* ACS pointer */
			ulong		rc1,	/* primary reason code */
			ulong		ac);	/* specific adapter code */
extern void fddi_run_stest(fddi_acs_t	*p_acs,
			ulong 	rc1,
			ulong	ac);
extern void fddi_bugout (
			fddi_acs_t	*p_acs,	/* ACS ptr */
			ulong		rc1,	/* primary reason */
			ulong		rc2,	/* secondary reason */
			ulong		ac);	/* specific adapter code */
extern void fddi_cycle_limbo(
			fddi_acs_t	*p_acs,	/* ACS pointer */
			ulong		rc1,	/* primary reason code */
			ulong		ac);	/* specific adapter code */
extern void fddi_limbo_to(struct watchdog	*p_wdt);
/* #endif */
extern int fddi_limbo_cmplt(
			fddi_acs_t	*p_acs,	/* ACS ptr */
			fddi_cmd_t	*p_cmd,	/* cmd blk ptr */
			uint		bus);	/* bus access value */

extern void fddi_wakeup_users(
			fddi_acs_t	*p_acs);	/* ACS ptr */

extern int cmplt_fn();
extern void fddi_cmd_handler(
			fddi_acs_t	*p_acs,		/* ACS ptr */
			uint		bus);		/* access to bus */
			
extern int
fddi_uls_handler (
	fddi_acs_t	*p_acs,
	fddi_cmd_t	*p_cmd,
	int		bus);

void
fddi_query_stats_cmplt (
	fddi_acs_t	*p_acs,
	fddi_cmd_t	*p_cmd,
	int		bus);
#else
extern struct cdt * fddi_cdt_func();
extern void fddi_cdt_add ();
extern void fddi_cdt_del ();
extern int fddi_cdt_undo_init ();
extern int fddi_get_stat();
extern int fddidiag_config();
extern int fddi_config_init();
extern int fddi_config_term();
extern int fddi_config_qvpd();
extern int cfg_pos_regs();
extern int get_vpd();
extern ushort fddi_gen_crc();

extern int fddi_mpx();
extern int fddi_open();
extern int fddi_1st_dd_open();
extern int fddi_1st_acs_open();
extern int fddi_undo_1st_dd_open();
extern int fddi_undo_1st_acs_open();
extern void fddi_link_open_hash();
extern void fddi_relink_open_hash();
extern void fddi_init_open_elem();

extern int fddi_get_query();
extern void fddi_start_netid();
extern void fddi_halt_netid();
/*
extern int fddi_query_addr();
extern void fddi_enter_limbo();
*/

extern int fddi_close();
extern int fddi_remove_netids();
extern void fddi_free_rcvq();

extern int fddi_ioctl();

extern int fddi_write();

extern int fddi_read();

extern int fddi_select();
extern int fddi_slih();
extern int fddislih();

extern void fddi_report_status();
extern void fddi_async_status();
extern void fddi_conn_done();
extern int fddi_pio_retry();
extern void fddi_close_wait ();
extern void fddi_limbo_act ();
extern void fddi_trace();
extern int fddi_mem_acc();
extern int fddi_dma_acc();
extern int fddi_do_dma();
extern int fddi_get_trace();

extern int fddi_act_cmplt();
extern void fddi_logerr();

#endif /* end if ! _NO_PROTO */

#endif /* end if ! _H_FDDIPROTO */
