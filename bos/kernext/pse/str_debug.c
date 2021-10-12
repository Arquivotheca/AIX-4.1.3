static char sccsid[] = "@(#)13        1.6  src/bos/kernext/pse/str_debug.c, sysxpse, bos411, 9428A410j 6/25/94 12:39:15";
/*
 * COMPONENT_NAME: SYSXPSE - STREAMS framework
 * 
 * FUNCTIONS:      DB_init
 *                 DB_is_on
 *                 DB0
 *                 DB1
 *                 DB2
 *                 DB3
 *                 DB4
 *                 DB5
 *                 DB6
 *                 ft_lookup
 *                 STREAMS_ENTER_FUNC
 *                 STREAMS_LEAVE_FUNC
 *                 int_to_str
 *                 REPORT_FUNC
 *                 DB_isopen
 *                 DB_isclosed
 *                 DB_check_streams
 *                 DB_break
 *                 DB_sth_check
 *                 DB_showstream
 *                 
 * 
 * ORIGINS: 63, 71, 83
 * 
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.1
 */
/** Copyright (c) 1988  Mentat Inc.
 **/


#include <pse/str_stream.h>
#include <pse/str_debug.h>
#include <sys/strconf.h>
#include <sys/stropts.h>
#include <pse/mi.h>
#include <pse/nd.h>
#include <pse/str_config.h>
#include <pse/str_proto.h>

#if	STREAMS_DEBUG

extern  long            mi_itimer(caddr_t, long, pfi_t);
extern  void            mi_itimer_fire(caddr_t);
extern  caddr_t *       mi_itimer_init(caddr_t);
extern  int             mi_itimer_rsrv(queue_t *);
extern  void            mi_ibc_qenable(long);
extern  void            mi_ibc_timer(caddr_t);
extern  void            mi_ibc_timer_add(caddr_t);
extern  void            mi_ibc_timer_del(caddr_t);
extern  int             mi_sprintf_noop(char *, int);
extern int              mi_timer_fire(caddr_t);
extern  int             mi_nd_comm(queue_t *, char *, int, uint, int);
extern  int     	nd_get_names();
extern  int     	nd_set_default(queue_t *, mblk_t *, char *, caddr_t);
extern  void            heap_report(int *);
extern  void            dump_module_names(int *);
extern  void            streams_info_runq(int *);
extern  SQHP            sqe_add(caddr_t);
extern  void            sqe_del(caddr_t);
extern  int             modsw_next(struct modsw **, void **, char **);
extern  dev_t           add_device();
extern  int             del_device();
extern  dev_t           add_module();
extern  int           	del_module();
extern  int             osr_getmsg_subr(OSRP, struct strbuf *, struct strbuf *);
extern  int             osr_putmsg_subr(OSRP, struct strbuf *, struct strbuf *);extern  void            sq_wrapper(queue_t *);
extern  int             sth_wsrv(queue_t *);
extern  void            sth_sigpoll_wakeup(STHP, int, int);
extern  void            sth_err_wakeup(STHP, int, int);
extern  void            sth_m_setopts(STHP, MBLKP);
extern  void            sth_set_queue(queue_t *, struct qinit *, struct qinit *);
extern  void            osr_bufcall_wakeup(OSRP);
extern	void		ext_freeb();

/*
 *	Initialization hook.
 *
 *	In case any of these debug packages need intialization.
 */
void
DB_init()
{
}

/*
 *	Configurable debug printf's
 *
 *	The idea is that messages introduced into the code should
 *	disappear dynamically, and perhaps on a per-area base.
 *	Therefor we have a number of global variables (patchable
 *	with the debugger) which control the debug output globally
 *	of for a certain area.
 *
 *	Each debug printf in the code (using the calls DB0() - DB5(),
 *	depending of the number of "%" arguments) has an area and
 *	a level assigned to it. The message gets printed only if the
 *	level <= min(global level, area level). That is, the lower
 *	the number the more urgent the message, and the
 *	global level overrides local levels (see DB_is_on).
 *
 *	The reason for the six (instead of one) DB?() procedures is
 *	the fact that we want to make even the calls themselves
 *	disappear from the code, if STREAMS_DEBUG is not defined.
 *	And the C-Preprocessor doesn't know about varargs.
 */

int	DB_level	= 0;

int
DB_is_on(area)
	int	area;
{

	if (area & DB_level)	{
		return 1;
	}
	return 0;

}

void
DB0(area, fmt)
	int	area;
	char	*fmt;
{
	if ( DB_is_on(area) ) printf(fmt);

}

void
DB1(area, fmt, a)
	int	area;
	char	*fmt;
	caddr_t	a;
{

	if ( DB_is_on(area) ) printf(fmt, a);

}

void
DB2(area, fmt, a, b)
	int	area;
	char	*fmt;
	caddr_t	a, b;
{

	if ( DB_is_on(area) ) printf(fmt, a, b);

}

void
DB3(area, fmt, a, b, c)
	int	area;
	char	*fmt;
	caddr_t	a, b, c;
{

	if ( DB_is_on(area) ) printf(fmt, a, b, c);

}

void
DB4(area, fmt, a, b, c, d)
	int	area;
	char	*fmt;
	caddr_t	a, b, c, d;
{

	if ( DB_is_on(area) ) printf(fmt, a, b, c, d);

}

void
DB5(area, fmt, a, b, c, d, e)
	int	area;
	char	*fmt;
	caddr_t	a, b, c, d, e;
{

	if ( DB_is_on(area) ) printf(fmt, a, b, c, d, e);

}

void
DB6(area, fmt, a, b, c, d, e, f)
	int	area;
	char	*fmt;
	caddr_t	a, b, c, d, e, f;
{

	if ( DB_is_on(area) ) printf(fmt, a, b, c, d, e, f);

}

/*
 * Function entry / exit monitoring.
 *
 * Very high performance impact, but nice potential for monitoring,
 * debugging and tracing.
 *
 * Note that tracing can be selected on the fly by patching the level
 * entry in the func_tab entry for the function under investigation.
 */

struct func_tab {
	pfi_t	ft_func;
	int	ft_level;
	int	ft_count;
	char	ft_name[25];
};

#ifdef	bufcall
#undef	bufcall
extern	int	bufcall();
#endif

extern  int     drv_open();
extern  int     weldq_comm();
extern  int     weldq_main();

struct func_tab func_tab[] = {

/*
 * mi.c
 */
	{ (pfi_t)mi_addr_scanf,		0, 0,	"mi_addr_scanf" },
	{ (pfi_t)mi_allocq,		DB_ALLOC, 0,	"mi_allocq" },
	{ (pfi_t)mi_bufcall,		DB_ALLOC, 0,	"mi_bufcall" },
	{ (pfi_t)mi_close_comm,		DB_CLOSE, 0,	"mi_close_comm" },
	{ (pfi_t)mi_close_detached,	DB_CLOSE, 0,	"mi_close_detached" },
	{ (pfi_t)mi_copyb,		DB_UTIL, 0,	"mi_copyb" },
	{ (pfi_t)mi_copyin,		DB_UTIL, 0,	"mi_copyin" },
	{ (pfi_t)mi_copyout,		DB_UTIL, 0,	"mi_copyout" },
	{ (pfi_t)mi_copyout_alloc,	DB_ALLOC, 0,	"mi_copyout_alloc" },
	{ (pfi_t)mi_copy_done,		DB_UTIL, 0,	"mi_copy_done" },
	{ (pfi_t)mi_copy_state,		DB_UTIL, 0,	"mi_copy_state" },
	{ (pfi_t)mi_detach,		0, 0,	"mi_detach" },
	{ (pfi_t)mi_gq_head,		0, 0,	"mi_gq_head" },
	{ (pfi_t)mi_gq_in,		0, 0,	"mi_gq_in" },
	{ (pfi_t)mi_gq_init,		0, 0,	"mi_gq_init" },
	{ (pfi_t)mi_gq_out,		0, 0,	"mi_gq_out" },
	{ (pfi_t)mi_iprintf,		DB_UTIL, 0,	"mi_iprintf" },
	{ (pfi_t)mi_itimer,		DB_TIMER, 0,	"mi_itimer" },
	{ (pfi_t)mi_itimer_fire,	DB_TIMER, 0,	"mi_itimer_fire" },
	{ (pfi_t)mi_itimer_init,	DB_TIMER, 0,	"mi_itimer_init" },
	{ (pfi_t)mi_itimer_rsrv,	DB_TIMER, 0,	"mi_itimer_rsrv" },
	{ (pfi_t)mi_ibc_timer,		DB_TIMER, 0,	"mi_ibc_timer" },
	{ (pfi_t)mi_ibc_timer_add,	DB_TIMER, 0,	"mi_ibc_timer_add" },
	{ (pfi_t)mi_ibc_timer_del,	DB_TIMER, 0,	"mi_ibc_timer_del" },
	{ (pfi_t)mi_mpprintf,		DB_UTIL, 0,	"mi_mpprintf" },
	{ (pfi_t)mi_mpprintf_nr,	DB_UTIL, 0,	"mi_mpprintf_nr" },
	{ (pfi_t)mi_mpprintf_putc,	DB_UTIL, 0,	"mi_mpprintf_putc" },
	{ (pfi_t)mi_next_ptr,		DB_UTIL, 0,	"mi_next_ptr" },
	{ (pfi_t)mi_open_comm,		DB_OPEN, 0,	"mi_open_comm" },
	{ (pfi_t)mi_offset_param,	0, 0,	"mi_offset_param" },
	{ (pfi_t)mi_offset_paramc,	0, 0,	"mi_offset_paramc" },
	{ (pfi_t)mi_offset_param_mblk,	0, 0,	"mi_offset_param_mblk" },
	{ (pfi_t)mi_panic,		DB_UTIL, 0,	"mi_panic" },
	{ (pfi_t)mi_set_sth_wroff,	0, 0,	"mi_set_sth_wroff" },
	{ (pfi_t)mi_sprintf,		DB_UTIL, 0,	"mi_sprintf" },
	{ (pfi_t)mi_sprintf_noop,	DB_UTIL, 0,	"mi_sprintf_noop" },
	{ (pfi_t)mi_sprintf_putc,	DB_UTIL, 0,	"mi_sprintf_putc" },
	{ (pfi_t)mi_strtol,		DB_UTIL, 0,	"mi_strtol" },
	{ (pfi_t)mi_timer,		DB_TIMER, 0,	"mi_timer" },
	{ (pfi_t)mi_timer_alloc,	DB_ALLOC, 0,	"mi_timer_alloc" },
	{ (pfi_t)mi_timer_fire,		DB_TIMER, 0,	"mi_timer_fire" },
	{ (pfi_t)mi_timer_free,		DB_TIMER, 0,	"mi_timer_free" },
	{ (pfi_t)mi_timer_valid,	DB_TIMER, 0,	"mi_timer_valid" },
	{ (pfi_t)mi_nd_comm,		DB_OPEN, 0,	"mi_nd_comm" },
	{ (pfi_t)mi_nd_get,		0, 0,	"mi_nd_get" },
	{ (pfi_t)mi_nd_set,		0, 0,	"mi_nd_set" },
	{ (pfi_t)mi_reallocb,		DB_ALLOC, 0,	"mi_reallocb" },
	{ (pfi_t)mi_tpi_ack_alloc,	DB_ALLOC, 0,	"mi_tpi_ack_alloc" },
	{ (pfi_t)mi_tpi_addr_and_opt,	0, 0,	"mi_tpi_addr_and_opt" },
	{ (pfi_t)mi_tpi_conn_con,	DB_OPEN, 0,	"mi_tpi_conn_con" },
	{ (pfi_t)mi_tpi_conn_ind,	DB_OPEN, 0,	"mi_tpi_conn_ind" },
	{ (pfi_t)mi_tpi_conn_req,	DB_OPEN, 0,	"mi_tpi_conn_req" },
	{ (pfi_t)mi_tpi_data_ind,	0, 0,	"mi_tpi_data_ind" },
	{ (pfi_t)mi_tpi_data_req,	0, 0,	"mi_tpi_data_req" },
	{ (pfi_t)mi_tpi_discon_ind,	DB_OPEN, 0,	"mi_tpi_discon_ind" },
	{ (pfi_t)mi_tpi_discon_req,	DB_OPEN, 0,	"mi_tpi_discon_req" },
	{ (pfi_t)mi_tpi_err_ack_alloc,	DB_ALLOC, 0,	"mi_tpi_err_ack_alloc" },
	{ (pfi_t)mi_tpi_exdata_ind,	0, 0,	"mi_tpi_exdata_ind" },
	{ (pfi_t)mi_tpi_exdata_req,	0, 0,	"mi_tpi_exdata_req" },
	{ (pfi_t)mi_tpi_info_req,	0, 0,	"mi_tpi_info_req" },
	{ (pfi_t)mi_tpi_ioctl_info_req,	DB_IOCTL, 0,	"mi_tpi_ioctl_info_req" },
	{ (pfi_t)mi_tpi_ok_ack_alloc,	DB_ALLOC, 0,	"mi_tpi_ok_ack_alloc" },
	{ (pfi_t)mi_tpi_ordrel_ind,	0, 0,	"mi_tpi_ordrel_ind" },
	{ (pfi_t)mi_tpi_ordrel_req,	0, 0,	"mi_tpi_ordrel_req" },
	{ (pfi_t)mi_tpi_trailer_alloc,	DB_ALLOC, 0,	"mi_tpi_trailer_alloc" },
	{ (pfi_t)mi_tpi_uderror_ind,	0, 0,	"mi_tpi_uderror_ind" },
	{ (pfi_t)mi_tpi_unitdata_ind,	0, 0,	"mi_tpi_unitdata_ind" },
	{ (pfi_t)mi_tpi_unitdata_req,	0, 0,	"mi_tpi_unitdata_req" },
	{ (pfi_t)mi_zalloc,		DB_ALLOC, 0,	"mi_zalloc" },
	{ (pfi_t)mi_strcmp,		DB_UTIL, 0,	"mi_strcmp" },

/*
 * nd.c
 */
	{ (pfi_t)nd_free,		DB_ALLOC, 0,	"nd_free" },
	{ (pfi_t)nd_getset,		0, 0,	"nd_getset" },
	{ (pfi_t)nd_get_default,	0, 0,	"nd_get_default" },
	{ (pfi_t)nd_get_long,		0, 0,	"nd_get_long" },  
	{ (pfi_t)nd_get_names,		0, 0, "nd_get_names"},
      	{ (pfi_t)nd_load,		0, 0,	"nd_load" },
	{ (pfi_t)nd_set_default,	0, 0,	"nd_set_default" },
	{ (pfi_t)nd_set_long,		0, 0,	"nd_set_long" },

/*
 * str_clone.c
 */

        { (pfi_t)clone_configure,	DB_CONF,0,	"clone_configure " },
        { (pfi_t)pse_clone_open,	DB_OPEN,0,	"pse_clone_open" },

/*
 * str_config.c
 */

	{ (pfi_t)str_config,           	DB_CONF,0,    "str_config" },
	{ (pfi_t)str_init,             	DB_CONF,0,    "str_init" },
	{ (pfi_t)str_term,             	DB_CONF,0,    "str_term" },

/*
 * str_env.c
 */

	{ (pfi_t)str_to,           	DB_TIMER,0,    "str_to" },
	{ (pfi_t)str_to_init,          	DB_CONF,0,    "str_to_init" },
	{ (pfi_t)str_to_term,          	DB_CONF,0,    "str_to_term" },
	{ (pfi_t)str_timeout,          	DB_TIMER,0,    "str_timeout" },
	{ (pfi_t)find_to,          	0,0,    "find_to" },
	{ (pfi_t)pse_untimeout,        	DB_TIMER,0,    "pse_untimeout" },
	{ (pfi_t)pse_timeout,          	DB_TIMER,0,    "pse_timeout" },
	{ (pfi_t)pse_sleepx,          	0,0,    "pse_sleepx" },
	{ (pfi_t)pse_mpsleep,          	0,0,    "pse_mpsleep" },

/*
 * str_filesys.c
 */
	{ (pfi_t)fd_to_cookie,          	0,0,    "fd_to_cookie" },
	{ (pfi_t)fd_alloc,          	DB_ALLOC,0,    "fd_alloc" },
	{ (pfi_t)cookie_destroy,       		0,0,    "cookie_destroy" },
	{ (pfi_t)sth_fd_to_sth,       		0,0,    "sth_fd_to_sth" },
	{ (pfi_t)sth_fattach,       		0,0,    "sth_fattach" },
	{ (pfi_t)sth_update_times,       	0,0,    "sth_update_times" },
/*
 * str_gpmsg.c
 */

	{ (pfi_t)getmsg,          	DB_GETMSG,0,    "getmsg" },
	{ (pfi_t)getpmsg,          	DB_GETPMSG,0,    "getpmsg" },
	{ (pfi_t)putmsg,          	DB_PUTMSG,0,    "putmsg" },
	{ (pfi_t)putpmsg,          	DB_PUTPMSG,0,    "putpmsg" },
/*
 * str_info.c
 */
	
	{ (pfi_t)heap_report,          	13,0,    "heap_report" },
	{ (pfi_t)dump_module_names,     13,0,    "dump_module_names" },
	{ (pfi_t)streams_info_runq,     13,0,    "streams_info_runq" },
	{ (pfi_t)strinfo,        	13,0,    "strinfo" },
/*
 * str_init.c
 */

        { (pfi_t)pse_init,        	DB_CONF,0,    "pse_init     " },
        { (pfi_t)pse_term,        	DB_CONF,0,    "pse_term     " },
        { (pfi_t)str_install,     	DB_CONF,0,    "str_install  " },
        { (pfi_t)strmod_add,      	DB_CONF,0,    "strmod_add   " },
        { (pfi_t)strmod_del,      	DB_CONF,0,    "strmod_del   " },
        { (pfi_t)add_device,      	DB_CONF,0,    "add_device   " },
	{ (pfi_t)del_device,      	DB_CONF,0,    "del_device   " },

/*
 * str_lock.c
 */

/*
 * str_memory.c
 */
	{ (pfi_t)he_alloc,	      	DB_ALLOC,0,	"he_alloc     " },
	{ (pfi_t)he_free,	      	DB_ALLOC,0,	"he_free      " },
	{ (pfi_t)he_realloc,	      	DB_ALLOC,0,	"he_realloc   " },
	{ (pfi_t)bufcall_configure,   	DB_ALLOC,0,	"bufcall_confi" },
	{ (pfi_t)bufcall_rsrv,	      	DB_ALLOC,0,	"bufcall_rsrv " },
	{ (pfi_t)bufcall,	      	DB_ALLOC,0,	"bufcall      " },
	{ (pfi_t)unbufcall,	      	DB_ALLOC,0,	"unbufcall    " },
	{ (pfi_t)bufcall_init,	      	DB_ALLOC,0,	"bufcall_init " },
	{ (pfi_t)allocb,	      	DB_ALLOC,0,	"allocb       " },
	{ (pfi_t)freeb,		      	DB_ALLOC,0,	"freeb        " },
	{ (pfi_t)ext_freeb,		DB_ALLOC,0,	"ext_freeb    " },

/*
 * str_modsw.c
 */

	{ (pfi_t)str_modsw_init,    	DB_CONF,0,	"str_modsw_init" },
	{ (pfi_t)sqe_add,		13,0,	"sqe_add" },
	{ (pfi_t)sqe_del,		13,0,	"sqe_del" },
        { (pfi_t)modsw_ref,            	DB_CONF,0,    "modsw_ref    " },
        { (pfi_t)dcookie_to_dindex,    	DB_CONF,0,    "dcookie_to_index " },
        { (pfi_t)dindex_to_str,        	DB_CONF,0,    "dindex_to_str " },
        { (pfi_t)dname_to_dcookie,	DB_CONF,0,    "dname_to_dcookie " },
        { (pfi_t)dname_to_dindex,	DB_CONF,0,    "dname_to_dindex " },
        { (pfi_t)dname_to_str,		DB_CONF,0,    "dname_to_str " },
        { (pfi_t)dqinfo_to_str,		DB_CONF,0,    "dqinfo_to_str " },
        { (pfi_t)fname_to_str,		DB_CONF,0,    "fname_to_str " },
        { (pfi_t)dmodsw_install,   	DB_CONF,0,    "dmodsw_install " },
        { (pfi_t)dmodsw_search,       	13,0,    "dmodsw_search " },
        { (pfi_t)dmodsw_remove,        	13,0,    "dmodsw_remove" },
        { (pfi_t)fmodsw_install,   	DB_CONF,0,    "fmodsw_install " },
        { (pfi_t)fmodsw_remove,        	13,0,    "fmodsw_remove" },
        { (pfi_t)qinfo_to_name,        	13,0,    "qinfo_to_name" },
        { (pfi_t)mid_to_str,		13,0,    "mid_to_str" },
        { (pfi_t)sqh_set_parent,	13,0,    "sqh_set_parent" },
        { (pfi_t)modsw_next,		13,0,    "modsw_next" },
/*
 * str_osr.c
 */
	{ (pfi_t)discard_passfp,	0,0,	"discard_passfp" },
	{ (pfi_t)osr_getmsg_subr,	DB_GETMSG,0,	"osr_getmsg_subr" },
	{ (pfi_t)osr_putmsg_subr,	DB_PUTMSG,0,	"osr_putmsg_subr" },
	{ (pfi_t)osr_atmark,		DB_IOCTL,0,	"osr_atmark" },
	{ (pfi_t)osr_canput,		DB_CANPUT,0,	"osr_canput" },
	{ (pfi_t)osr_ckband,		DB_IOCTL,0,	"osr_ckband" },
	{ (pfi_t)osr_fattach,		DB_IOCTL,0,	"osr_fattach" },
	{ (pfi_t)osr_fdinsert,		DB_IOCTL,0,	"osr_fdinsert" },
	{ (pfi_t)osr_fifo,		DB_IOCTL,0,	"osr_fifo" },
	{ (pfi_t)osr_find,		DB_IOCTL,0,	"osr_find" },
	{ (pfi_t)osr_fionread,		DB_IOCTL,0,	"osr_fionread" },
	{ (pfi_t)osr_flush,		DB_IOCTL,0,	"osr_flush" },
	{ (pfi_t)osr_flushband,		DB_IOCTL,0,	"osr_flushband" },
	{ (pfi_t)osr_getband,		DB_IOCTL,0,	"osr_getband" },
	{ (pfi_t)osr_getcltime,		DB_IOCTL,0,	"osr_getcltime" },
	{ (pfi_t)osr_getmsg,		DB_GETMSG,0,	"osr_getmsg" },
	{ (pfi_t)osr_getpmsg,		DB_GETPMSG,0,	"osr_getpmsg" },
	{ (pfi_t)osr_getsig,		DB_IOCTL|DB_SIG,0,	"osr_getsig" },
	{ (pfi_t)osr_grdopt,		DB_IOCTL,0,	"osr_grdopt" },
	{ (pfi_t)osr_gwropt,		DB_IOCTL,0,	"osr_gwropt" },
	{ (pfi_t)osr_isastream,		DB_IOCTL,0,	"osr_isastream" },
	{ (pfi_t)osr_link,		DB_IOCTL,0,	"osr_link" },
	{ (pfi_t)osr_list,		DB_IOCTL,0,	"osr_list" },
	{ (pfi_t)osr_look,		DB_IOCTL,0,	"osr_look" },
	{ (pfi_t)osr_nread,		DB_IOCTL,0,	"osr_nread" },
	{ (pfi_t)osr_peek,		DB_IOCTL,0,	"osr_peek" },
	{ (pfi_t)osr_pipe,		DB_IOCTL,0,	"osr_pipe" },
	{ (pfi_t)osr_pipestat,		DB_IOCTL,0,	"osr_pipestat" },
	{ (pfi_t)osr_pop,		DB_IOCTL,0,	"osr_pop" },
	{ (pfi_t)osr_pop_subr,		DB_IOCTL,0,	"osr_pop_subr" },
	{ (pfi_t)osr_push,		DB_IOCTL,0,	"osr_push" },
	{ (pfi_t)osr_putmsg,		DB_PUTMSG,0,	"osr_putmsg" },
	{ (pfi_t)osr_putpmsg,		DB_PUTPMSG,0,	"osr_putpmsg" },
	{ (pfi_t)osr_read,		DB_READ,0,	"osr_read" },
	{ (pfi_t)osr_recvfd,		DB_IOCTL,0,	"osr_recvfd" },
	{ (pfi_t)osr_sendfd,		DB_IOCTL,0,	"osr_sendfd" },
	{ (pfi_t)osr_setcltime,		DB_IOCTL,0,	"osr_setcltime" },
	{ (pfi_t)osr_setsig,		DB_IOCTL|DB_SIG,0,	"osr_setsig" },
	{ (pfi_t)osr_srdopt,		DB_IOCTL,0,	"osr_srdopt" },
	{ (pfi_t)osr_str,		DB_IOCTL,0,	"osr_str" },
	{ (pfi_t)osr_swropt,		DB_IOCTL,0,	"osr_swropt" },
	{ (pfi_t)osr_unlink,		DB_IOCTL,0,	"osr_unlink" },
	{ (pfi_t)osr_unlink_subr,	DB_IOCTL,0,	"osr_unlink_subr" },
	{ (pfi_t)osr_write,		DB_WRITE,0,	"osr_write" },

/*
 * str_runq.c
 */
	{ (pfi_t)runq_init,	       DB_SCHED,0,	"runq_init" },
        { (pfi_t)runq_term,            DB_SCHED,0,	"runq_term" },
	{ (pfi_t)runq_run,	       DB_SCHED,0,	"runq_run" },
	{ (pfi_t)sq_wrapper,	       DB_SCHED,0,	"sq_wrapper" },
	{ (pfi_t)qenable,	       DB_SCHED,0,	"qenable" },
	{ (pfi_t)runq_sq_init,	       DB_SCHED,0,	"runq_sq_init" },
	{ (pfi_t)runq_remove,	       DB_SCHED,0,	"runq_remove" },
	
/*
 * str_scalls.c
 */
	{ (pfi_t)osr_alloc,	      DB_ALLOC,0,	"osr_alloc     " },
	{ (pfi_t)osr_free,	      DB_ALLOC,0,	"osr_free     " },
	{ (pfi_t)pse_open,	      DB_OPEN,0,	"pse_open     " },
	{ (pfi_t)osr_open,	      DB_OPEN,0,	"osr_open     " },
	{ (pfi_t)osr_reopen,	      DB_OPEN,0,	"osr_reopen   " },
	{ (pfi_t)pse_close,	      DB_CLOSE,0,	"pse_close    " },
	{ (pfi_t)osr_close_subr,      DB_CLOSE,0,	"osr_close_subr" },
	{ (pfi_t)pse_read,	      DB_READ,0,	"pse_read     " },
	{ (pfi_t)pse_write,	      DB_WRITE,0,	"pse_write    " },
	{ (pfi_t)pse_ioctl,	      DB_IOCTL,0,	"pse_ioctl    " },
	{ (pfi_t)pse_select,	      DB_SIG,0,		"pse_select   " },
	{ (pfi_t)pse_revoke,	      0,0,		"pse_revoke   " },
/*
 * str_select.c
 */

	{ (pfi_t)select_enqueue,	DB_SIG,0,	"select_enqueue   " },
	{ (pfi_t)select_dequeue_all,	DB_SIG,0,	"select_dequeue_all" },
	{ (pfi_t)select_wakeup,		DB_SIG,0,	"select_wakeup" },

/*
 * str_shead.c
 */

	{ (pfi_t)sth_rput,		0,0,		"sth_rput" },
	{ (pfi_t)sth_getq,		0,0,		"sth_getq" },
	{ (pfi_t)sth_iocdata,		0,0,		"sth_iocdata" },
	{ (pfi_t)sth_canput,		DB_CANPUT,0,	"sth_canput" },
	{ (pfi_t)sth_wsrv,		0,0,		"sth_wsrv" },
	{ (pfi_t)sth_sigpoll_wakeup,	DB_SIG,0,	"sth_sigpoll_wakeup" },
	{ (pfi_t)sth_err_wakeup,	0,0,		"sth_err_wakeup" },
	{ (pfi_t)sth_m_setopts,		0,0,		"sth_m_setopts" },
/*
 * str_subr.c
 */
        { (pfi_t)sth_alloc,        	DB_ALLOC,0,    	"sth_alloc " },
        { (pfi_t)sth_free,        	DB_ALLOC,0,    	"sth_free " },
        { (pfi_t)q_alloc,        	DB_ALLOC,0,    	"q_alloc " },
        { (pfi_t)q_free,        	DB_ALLOC,0,    	"q_free " },
        { (pfi_t)sth_set_queue,         0,0,		"sth_set_queue " },
        { (pfi_t)sth_muxid_lookup,      0,0,		"sth_muxid_lookup " },
        { (pfi_t)sth_iocblk_init,       DB_IOCTL,0,    	"sth_iocblk_init " },
        { (pfi_t)sth_iocblk,        	DB_IOCTL,0,    	"sth_iocblk " },
        { (pfi_t)sth_link_alloc,    	DB_IOCTL|DB_ALLOC,0,    "sth_link_alloc " },
        { (pfi_t)sth_read_reset,        0,0,    	"sth_read_reset " },
        { (pfi_t)sth_read_seek,         0,0,    	"sth_read_seek " },
        { (pfi_t)close_wrapper,         0,0,    	"close_wrapper " },
        { (pfi_t)open_wrapper,         	0,0,   		"open_wrapper " },
        { (pfi_t)sth_uiomove,         	0,0,    	"sth_uiomove " },
        { (pfi_t)sth_uiodone,         	0,0,    	"sth_uiodone " },

/*
 * str_synch.c
 */
	{ (pfi_t)osr_run,	      	DB_SYNC,0,	"osr_run      " },
	{ (pfi_t)osrq_init,	      	DB_CONF,0,	"osrq_init    " },
	{ (pfi_t)osrq_insert,	      	DB_SYNC,0,	"osrq_insert  " },
	{ (pfi_t)osrq_remove,	      	0,0,		"osrq_remove  " },
	{ (pfi_t)osrq_wakeup,	      	DB_SYNC,0,	"osrq_wakeup  " },
        { (pfi_t)osrq_cancel,         	0,0,    	"osrq_cancel  " },
	{ (pfi_t)osr_sleep,	      	DB_SYNC,0,	"osr_sleep    " },
	{ (pfi_t)osr_bufcall_wakeup,  	0,0,		"osr_bufcall_wakeup" },
	{ (pfi_t)osr_bufcall,	      	0,0,		"osr_bufcall" },
	{ (pfi_t)act_q_init,	      	DB_CONF,0,	"act_q_init   " },
        { (pfi_t)act_q_term,          	DB_CONF,0,    	"act_q_term   " },
	{ (pfi_t)csq_run,	     	DB_SYNC,0,	"csq_run      " },
	{ (pfi_t)csq_protect,	      	DB_SYNC,0,	"csq_protect  " },
	{ (pfi_t)csq_which_q,	      	DB_SYNC,0,	"csq_which_q  " },
	{ (pfi_t)_csq_acquire,	      	DB_SYNC,0,	"csq_acquire  " },
	{ (pfi_t)_csq_release,	      	DB_SYNC,0,	"csq_release  " },
	{ (pfi_t)csq_turnover,	      	DB_SYNC,0,	"csq_turnover " },
	{ (pfi_t)csq_lateral,	      	DB_SYNC,0,	"csq_lateral  " },
	{ (pfi_t)mult_sqh_acquire,    	DB_SYNC,0,	"mult_sqh_acqu" },
	{ (pfi_t)mult_sqh_release,    	DB_SYNC,0,	"mult_sqh_rele" },
	{ (pfi_t)sqh_insert,	      	DB_SYNC,0,	"sqh_insert   " },
	{ (pfi_t)sqh_remove,	      	DB_SYNC,0,	"sqh_remove   " },
	{ (pfi_t)csq_newparent,	      	DB_SYNC,0,	"csq_newparent" },
	{ (pfi_t)csq_cleanup,	      	DB_SYNC,0,	"csq_cleanup  " },
/*
 * str_util.c
 */
        { (pfi_t)adjmsg,           	DB_UTIL,0,    "adjmsg   " },
        { (pfi_t)allocbi,          	DB_ALLOC,0,    "allocbi   " },
        { (pfi_t)alloc_qband,          	DB_ALLOC,0,    "alloc_qband   " },
        { (pfi_t)backq,           	DB_UTIL,0,    "backq   " },
        { (pfi_t)bcanput,           	DB_CANPUT,0,    "bcanput   " },
        { (pfi_t)canput,           	DB_CANPUT,0,    "canput   " },
        { (pfi_t)copyb,           	DB_UTIL,0,    "copyb   " },
        { (pfi_t)copymsg,           	DB_UTIL,0,    "copymsg   " },
        { (pfi_t)dupb,           	DB_UTIL,0,    "dupb   " },
        { (pfi_t)dupmsg,           	DB_UTIL,0,    "dupmsg   " },
        { (pfi_t)esballoc,        	DB_ALLOC,0,    "esballoc   " },
        { (pfi_t)flushband,           	DB_UTIL,0,    "flushband   " },
        { (pfi_t)flushq,           	DB_UTIL,0,    "flushq   " },
        { (pfi_t)freemsg,        	DB_ALLOC,0,    "freemsg   " },
        { (pfi_t)getadmin,           	0,0,    "getadmin   " },
        { (pfi_t)getmid,           	0,0,    "getmid   " },
        { (pfi_t)getq,           	DB_UTIL,0,    "getq   " },
        { (pfi_t)insq,           	DB_UTIL,0,    "insq   " },
        { (pfi_t)insqband,           	DB_UTIL,0,    "insqband   " },
        { (pfi_t)linkb,           	DB_UTIL,0,    "linkb   " },
        { (pfi_t)msgdsize,           	DB_UTIL,0,    "msgdsize   " },
        { (pfi_t)pullupmsg,           	DB_UTIL,0,    "pullupmsg   " },
        { (pfi_t)putbq,           	DB_UTIL,0,    "putbq   " },
        { (pfi_t)putbqband,           	DB_UTIL,0,    "putbqband   " },
        { (pfi_t)putctl_comm,           0,0,    "putctl_comm" },
        { (pfi_t)putctl,           	0,0,    "putctl" },
        { (pfi_t)putctl1,           	0,0,    "putctl1" },
        { (pfi_t)putctl2,           	0,0,    "putctl2" },
        { (pfi_t)puthere,           	DB_UTIL,0,    "puthere" },
        { (pfi_t)putnext,           	DB_UTIL,0,    "putnext" },
        { (pfi_t)putq,           	DB_UTIL,0,    "putq" },
        { (pfi_t)putqband,           	DB_UTIL,0,    "putqband" },
        { (pfi_t)putq_deferred,         DB_UTIL,0,    "putq_deferred" },
        { (pfi_t)putq_owned,            DB_UTIL,0,    "putq_owned" },
        { (pfi_t)qreply,                DB_UTIL,0,    "qreply" },
        { (pfi_t)qsize,           	DB_UTIL,0,    "qsize" },
        { (pfi_t)rmvb,          	DB_ALLOC,0,    "rmvb" },
        { (pfi_t)rmvq,          	DB_ALLOC,0,    "rmvq" },
        { (pfi_t)strqget,           	DB_UTIL,0,    "strqget" },
        { (pfi_t)strqset,           	DB_UTIL,0,    "strqset" },
        { (pfi_t)testb,           	DB_UTIL,0,    "testb" },
        { (pfi_t)unlinkb,           	DB_UTIL,0,    "unlinkb" },
        { (pfi_t)canenable,           	0,0,    "canenable" },
        { (pfi_t)datamsg,           	DB_UTIL,0,    "datamsg" },
        { (pfi_t)enableok,           	DB_UTIL,0,    "enableok" },
        { (pfi_t)noenable,           	DB_UTIL,0,    "noenable" },
        { (pfi_t)OTHERQ,           	DB_UTIL,0,    "OTHERQ" },
        { (pfi_t)RD,           		DB_UTIL,0,    "RD" },
        { (pfi_t)WR,           		DB_UTIL,0,    "WR" },

/*
 * str_weld.c
 */
        { (pfi_t)weldq,			DB_WELD,0,	"weldq" },
        { (pfi_t)unweldq,		DB_WELD,0,	"unweldq" },
        { (pfi_t)weldq_comm,		DB_WELD,0,	"weldq_comm" },
        { (pfi_t)weldq_init,		DB_WELD | DB_CONF,0,	"weldq_init" },
        { (pfi_t)weldq_term,		DB_WELD | DB_CONF,0,	"weldq_term" },
        { (pfi_t)weldq_main,		DB_WELD,0,	"weldq_main" },
        { (pfi_t)weldq_exec,		DB_WELD,0,	"weldq_exec" },
        { (pfi_t)unweldq_exec,		DB_WELD,0,	"unweldq_exec" },

	{(pfi_t) 0,		       	DB_LAST,0,	"" }
};

struct func_tab unknown_func =
	{ (pfi_t) 0,		       DB_LAST,0,	"UNKNOWN      " };

struct func_tab *
ft_lookup(func)
	pfi_t func;
{
	struct func_tab *	ftp;

	for (ftp = func_tab; ftp->ft_func != (pfi_t) 0; ftp++)
		if (ftp->ft_func == func) {
			return ftp;
		}

	return &unknown_func;
}

void
STREAMS_ENTER_FUNC(func, p1, p2, p3, p4, p5, p6)
	pfi_t	func;
	int	p1, p2, p3, p4, p5, p6;
{
	struct func_tab * ftp = ft_lookup(func);

	DB6(
		DB_FUNC | ftp->ft_level,
		"ENTER %s (%x, %x, %x, %x, %x, %x)\n",
		ftp->ft_name,
		(caddr_t)p1, (caddr_t)p2, (caddr_t)p3,
		(caddr_t)p4, (caddr_t)p5, (caddr_t)p6
	);
	ftp->ft_count++;
}

void
STREAMS_LEAVE_FUNC(func, retval)
	pfi_t	func;
	int	retval;
{
	struct func_tab * ftp = ft_lookup(func);

	DB2(
		DB_FUNC | ftp->ft_level,
		"LEAVE %s (%x)\n",
		ftp->ft_name,
		(caddr_t)retval
	);
}

#define NDIGITS	5

void
int_to_str(number, buffer)
	int	number;
	char *	buffer;
{
	char *p = buffer + NDIGITS + 1;

	*--p = 0;

	while (p > buffer) {
		*--p = (number % 10) + '0';
		number /= 10;
	}
}

void
REPORT_FUNC()
{
	struct func_tab * ftp;
	int		count;
	char		count_buf[NDIGITS + 1];

	for (	ftp = func_tab, count = 1;
		ftp->ft_func != (pfi_t) 0;
		ftp++, count++ ) {

		int_to_str(ftp->ft_count, count_buf);
		DB2(
			DB_FUNC | DB_CLOSE,
			"%s(%s) ",
			ftp->ft_name,
			count_buf
		);
		if ( count % 5 == 0 )
			DB0(DB_FUNC, 5, "\n");
	}
}

#ifdef	DB_CHECK_LOCK
/*
 *	Consistency check for stream heads
 *	(to be extended to other data structures...)
 *
 *	Currently used:	only for STREAMS_DEBUG version (not MP-safe).
 *
 *	Maintains a list of open streams, and checks whether all of them
 *	are released at strategic points (end of system calls). Does
 *	work only if there is only one process active using streams.
 *
 *	STRlist is also nice for debugging, to find out which stream
 *	heads are currently around.
 *
 *	Known problem: when flushing a M_PASSFP message, an implicit
 *	close happens which will report one stream as locked.
 */

#define MAXSTREAMS 100

STHP	STRlist[MAXSTREAMS];
STHP *	STRlast = STRlist;
int	DB_verbose = 0;


void
DB_isopen(sth)
	caddr_t	sth;
{
	STHP	*sth_p;

	for (sth_p = STRlist; sth_p < STRlast; sth_p++)
	{
		if (*sth_p == nil(STHP))
			break;
	}
	if (sth_p == STRlast)
		sth_p = STRlast++;

	if (sth_p >= STRlist + MAXSTREAMS)
	{
		printf("DB_isopen: STRlist full\n");
		return;
	}

	*sth_p = (STHP)sth;

}

void
DB_isclosed(sth)
	caddr_t	sth;
{
	STHP	*sth_p;

	for (sth_p = STRlist; sth_p < STRlast; sth_p++)
	{
		if (*sth_p == (STHP)sth)
			break;
	}
	if (sth_p == STRlast)
	{
		printf("DB_isclosed: couldn't find pointer to closed stream\n");
		return;
	}
	*sth_p = nil(STHP);
	if (sth_p + 1 == STRlast)
		STRlast--;

}

void
DB_check_streams(caller)
	char *caller;
{
	STHP	*sth_p;
	void	DB_sth_check();

	for (sth_p = STRlist; sth_p < STRlast; sth_p++)
	{
		DB_sth_check(caller, *sth_p);
	}

}
	

void
DB_break()
{
}

void
DB_sth_check(caller, sth)
	char	*caller;
	STHP	sth;
{
	queue_t	*rq;
	queue_t	*wq;
	SQH	*rsqh;
	SQH	*wsqh;
	SQH	*rsqhp;
	SQH	*wsqhp;

	if (sth == nilp(STH)) {
		return;
	}

	rq = sth->sth_rq;
	wq = sth->sth_wq;

	rsqh = &rq->q_sqh;
	wsqh = &wq->q_sqh;

	rsqhp = rsqh->sqh_parent;
	wsqhp = wsqh->sqh_parent;

	if (rsqhp->sqh_refcnt != 0 || DB_verbose) {
		printf(
"*** %s *** STH %x, DEV %x, RQ %x, RSQH %x, RSQHP %x, refcnt %d, owner %x\n",
			caller,
			sth,
			sth->sth_dev,
			rq,
			rsqh,
			rsqhp,
			rsqhp->sqh_refcnt,
			rsqhp->sqh_owner
		);
		DB_break();
	}

	if (wsqhp != rsqhp) {
		printf(
"*** %s *** STH %x, DEV %x, RQ %x WQ %x RSQHP %x != WSQHP %s !!!\n",
			caller,
			sth,
			sth->sth_dev,
			rq,
			wq,
			rsqhp,
			wsqhp
		);
		printf(
"*** %s *** STH %x, DEV %x, WQ %x, WSQH %x, WSQHP %x, refcnt %d, owner %x\n",
			caller,
			sth,
			sth->sth_dev,
			wq,
			wsqh,
			wsqhp,
			wsqhp->sqh_refcnt,
			wsqhp->sqh_owner
		);
	}

}
#endif	/* DB_CHECK_LOCK */

void
DB_showstream (sth)
	STHP	sth;
{
	queue_t	* q;

	for (q = sth->sth_wq; q; q = q->q_next)
	DB5(DB_PUSHPOP, "name %s q 0x%x q_next 0x%x wrq 0x%x wrq_next 0x%x\n",
				q->q_qinfo->qi_minfo->mi_idname,
				(caddr_t)RD(q),
			   	(caddr_t)RD(q)->q_next,
				(caddr_t)q,
				(caddr_t)q->q_next);

}

#endif	/* STREAMS_DEBUG */
