/* @(#)02       1.17  src/bos/kernel/include/pse/str_proto.h, sysxpse, bos41J, 9509A_all 2/23/95 14:48:40 */
/*
 * COMPONENT_NAME: SYSXPSE - STREAMS framework
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

#ifndef _STR_PROTO_H
#define _STR_PROTO_H

#include <pse/str_config.h>
#include <pse/str_select.h>

/*
 * mi.c
 */
extern	void		mi_close_detached(caddr_t *, caddr_t);
extern	mblk_t *	mi_copyb(mblk_t *);
extern	void		mi_detach(caddr_t);
extern	int		mi_panic(char *, ...);
extern	void		mi_tpi_addr_and_opt (mblk_t *, char *, int, char *, int);
extern	mblk_t *	mi_tpi_trailer_alloc(mblk_t *, int, int);
extern	int		mi_strcmp(char *, char *);

/*
 * str_modsw.c
 */
extern	void		str_modsw_init(void);
extern	int		modsw_ref(struct qinit *, int);
extern	int		dcookie_to_dindex(int);
extern	struct streamtab * dindex_to_str(int);
extern	int		dname_to_dcookie(char *);
extern	int		dname_to_dindex(char *);
extern	struct streamtab * dname_to_str(char *);
extern	struct streamtab * dqinfo_to_str(struct qinit *, struct streamtab **);
extern	struct streamtab * fname_to_str(char *);
extern	int		dmodsw_install(struct streamadm *, struct streamtab *, int);
extern	int		dmodsw_remove(char *);
extern	int		fmodsw_install(struct streamadm *, struct streamtab *);
extern	int		fmodsw_remove(char *);
extern	char *		qinfo_to_name(struct qinit *);
extern	struct streamtab * mid_to_str(ushort);
extern	SQHP		sqh_set_parent(queue_t *, struct streamtab *);

/*
 * str_clone.c
 */
extern	int		clone_configure(int, str_config_t *, size_t, str_config_t *, size_t);
extern  int             pse_clone_open(dev_t, int, void **, dev_t *);
/*
 * str_config.c
 */
extern	void		str_config(void);
extern  void            str_init(struct uio *);
extern  void            str_term(struct uio *);

/*
 * str_tty.c
 */
extern	int		sth_ttyopen(STHP, int);
extern	void		sth_ttyclose(STHP);
extern	int		sth_tiocsctty(STHP, struct proc *);
extern	void		sth_pgsignal(STHP, int);
extern	int		sth_ttyioctl(struct tty *, dev_t, int, caddr_t, int);

/*
 * str_env.c
 */
extern	void		str_to(void);
extern	void		str_to_init(void);
extern  void            str_to_term(void);
extern	void		streams_lbolt_init(void);
extern	void		str_timeout(caddr_t);
extern	SQP		find_to(int, int);

/*
 * str_filesys.c
 */
extern	int		fd_to_cookie(int, struct file_cookie *);
extern	int		fd_alloc(struct file_cookie *, int *);
extern	void		cookie_destroy(struct file_cookie *);
extern	int		sth_fd_to_sth(int, STHPP);
extern	void		sth_update_times(STHP, int, struct stat *);
extern	int		sth_fattach(STHP, int, void *);

/*
 * str_gpmsg.c
 */
extern	int		getmsg(int, struct strbuf *,  struct strbuf *, int *);
extern	int		getpmsg(int, struct strbuf *,  struct strbuf *, int *, int *);
extern	int		putmsg(int, struct strbuf *,  struct strbuf *, int);
extern	int		putpmsg(int, struct strbuf *,  struct strbuf *, int, int);

/*
 * str_info.c
 */
extern	int		strinfo(int, char *, int *);
extern	void		get_device_names(mblk_t *);
extern	void		get_module_names(mblk_t *);
extern	void		get_mod_and_dev_names(struct msgb *);

/*
 * str_init.c
 */
extern	int		dmodsw_search(void);
extern	int		pse_init(void);
extern  int             pse_term(void);
extern  int             str_install();
extern	void		str_open_init(void);

/*
 * str_memory.c
 */
extern	caddr_t		he_alloc(long, int);
extern	void		he_free(caddr_t);
extern	caddr_t		he_realloc(caddr_t, long, long);
extern	int		bufcall_configure(/* TODO */);
extern	int		bufcall_init(void);
extern	int		bufcall_rsrv(queue_t *);

/*
 * str_osr.c and str_tty.c
 */
extern	void		discard_passfp(MBLKP);
extern	int		osr_atmark(OSRP);
extern	int		osr_canput(OSRP);
extern	int		osr_ckband(OSRP);
extern	int		osr_fattach(OSRP);
extern	int		osr_fdinsert(OSRP);
extern	int		osr_fifo(OSRP);
extern	int		osr_find(OSRP);
extern	int		osr_fionread(OSRP);
extern	int		osr_flush(OSRP);
extern	int		osr_flushband(OSRP);
extern	int		osr_getband(OSRP);
extern	int		osr_getcltime(OSRP);
extern	int		osr_getmsg(OSRP);
extern	int		osr_getpmsg(OSRP);
extern	int		osr_grdopt(OSRP);
extern	int		osr_gwropt(OSRP);
extern	int		osr_getsig(OSRP);
extern	int		osr_isastream(OSRP);
extern	int		osr_link(OSRP);
extern	int		osr_list(OSRP);
extern	int		osr_look(OSRP);
extern	int		osr_nread(OSRP);
extern	int		osr_peek(OSRP);
extern	int		osr_pipe(OSRP);
extern	int		osr_pipestat(OSRP);
extern	int		osr_pop(OSRP);
extern	int		osr_pop_subr(OSRP, queue_t *);
extern	int		osr_push(OSRP);
extern	int		osr_putmsg(OSRP);
extern	int		osr_putpmsg(OSRP);
extern	int		osr_read(OSRP);
extern	int		osr_recvfd(OSRP);
extern	int		osr_sendfd(OSRP);
extern	int		osr_setcltime(OSRP);
extern	int		osr_tiocspgrp(OSRP);
extern	int		osr_setsig(OSRP);
extern	int		osr_srdopt(OSRP);
extern	int		osr_swropt(OSRP);
extern	int		osr_str(OSRP);
extern	int		osr_tioccons(OSRP);
extern	int		osr_tiocgpgrp(OSRP);
extern	int		osr_tiocgsid(OSRP);
extern	int		osr_tiocsctty(OSRP);
extern	int		osr_unlink(OSRP);
extern	int		osr_unlink_subr(OSRP, int, int, OSRQP);
extern	int		osr_write(OSRP);
extern  int             osr_tiockpgsig(OSRP);
extern  int             osr_tiocknosess(OSRP);
extern  int             osr_tiocknopgrp(OSRP);
extern  int             osr_tiocisatty(OSRP);
extern  int             osr_tctrust(OSRP);
extern  int             osr_tcqtrust(OSRP);
extern  int             osr_tcsak(OSRP);
extern  int             osr_tcqsak(OSRP);
extern  int             osr_tcskep(OSRP);
extern  int             osr_kepcheck(OSRP);
extern	int		osr_tcxonc(OSRP);
extern	int		osr_flushdata(OSRP);

/*
 * str_runq.c
 */
extern	void		scheduled_run(void);
extern	void		scheduled_remove(queue_t *);
extern	void		runq_init(void);
extern	void		flip_and_run(void);
extern  void            runq_term(void);
extern	void		runq_run(void);
extern	int		qenable(queue_t *);
extern	void		runq_sq_init(queue_t *);
extern	void		runq_remove(queue_t *);

/*
 * str_scalls.c
 */
extern  int             pse_open(dev_t, int, void **, dev_t *);
extern  int             osr_open(OSRP);
extern  int             osr_reopen(OSRP);
extern  int             clone_open(dev_t, int, int, dev_t *,
                                        struct ucred *, void **);
extern  int             pse_close(dev_t, void *);
extern  int             osr_close_subr(OSRQP);
extern  int             pse_read(dev_t, struct uio *, void *, int);
extern  int             pse_write(dev_t, struct uio *, void *, int);
extern  int             pse_ioctl(dev_t, int, caddr_t, int,
                                        void *, int, int *);
extern  int             pse_select(dev_t, ushort, ushort *, void *);
extern  int             pse_revoke(dev_t, void *, int);

/*
 * str_select.c
 */

extern void select_enqueue( STHP, ushort, chan_t);
extern void select_dequeue_all(POLLQP);
extern void select_wakeup(POLLSP);

/*
 * str_shead.c
 */
extern	int		sth_rput(queue_t *, MBLKP);
extern	void		sth_iocdata(STHP, MBLKP, int);
extern	MBLKP		sth_getq(STHP);
extern	int		sth_canput(STHP, int);
extern	void		sth_sigpoll_wakeup(STHP, int, int);

/*
 * str_subr.c
 */
extern	STHP		sth_alloc(void);
extern	void		sth_free(STHP);
extern	queue_t *	q_alloc(void);
extern	int		q_free(queue_t *);
extern	OSRP		osr_alloc(STHP, int, int);
extern	void		osr_free(OSRP);
extern	STHPP		sth_muxid_lookup(STHP, int, int);
extern	void		sth_iocblk_init(void);
extern	int		sth_iocblk(void);
extern	MBLKP		sth_link_alloc(OSRP, int, int, queue_t *, queue_t *);
extern	int		sth_read_reset(OSRP);
extern	int		sth_read_seek(OSRP, int, long);
extern	int		open_wrapper(struct open_args *);
extern	int		close_wrapper(struct open_args *);
extern	int		sth_uiomove(caddr_t, int, OSRP);
extern	void		sth_uiodone(OSRP);

/*
 * str_synch.c
 */
extern	int		osr_run(OSRP);
extern	void		osrq_init(OSRQP);
extern	void		osrq_insert(OSRQP, OSRP);
extern	OSRP		osrq_remove(OSRQP);
extern	int		osrq_cancel(OSRQP);
extern	int		osr_sleep(OSRP, int, int);
extern	void		osr_bufcall_wakeup(OSRP);
extern	int		osr_bufcall(OSRP, int, int, int, int);
extern	void		osrq_wakeup(OSRQP);

extern	void		act_q_init(void);
extern  void            act_q_term(void);
extern	void		csq_run(SQP);
typedef	void *		csq_protect_arg_t;
typedef	int		(*csq_protect_fcn_t)(csq_protect_arg_t);
extern	int		csq_protect(queue_t *, queue_t *,
				csq_protect_fcn_t, csq_protect_arg_t, SQP, int);
extern	queue_t *	csq_which_q(void);
extern	void		_csq_acquire(SQHP, SQP); /* macro in str_stream.h */
extern	void		_csq_release(SQHP);	 /* macro in str_stream.h */
extern	int		csq_turnover(SQHP);
extern	int		csq_lateral(SQHP, SQP, int);
extern	void		mult_sqh_acquire(OSRP);
extern	void		mult_sqh_release(OSRP);
extern	void		csq_newparent(OSRP, queue_t *, struct streamtab *);
extern	void		csq_cleanup(SQHP);

extern	void		sqh_insert(SQHP, SQP);
extern	SQP		sqh_remove(SQP);

/*
 * str_util.c
 *
 * Implements the STREAMS utility routines. Some routines which
 * are closely related to certain modules are found in other
 * files. The prototypes for this file come from the standard
 * include file stream.h. We list here only non-standard
 * extensions, which are for the use by the stream head, but
 * functionally closely related to standard utilities.
 */
extern	int		putq_owned(queue_t *, mblk_t *);
extern	int		putctl2(queue_t *, int, int, int);
#ifdef STREAMS_DEBUG
extern	int     alloc_qband(queue_t * q, unsigned char band);
extern	int     putctl_comm(queue_t *, int, int, int, int);
extern	int     putqband(queue_t *q, MBLKP mp);
extern	void    putq_deferred(queue_t *, MBLKP);
extern	int     putbqband(queue_t *q, MBLKP mp);
extern	int     insqband(queue_t *q, MBLKP emp, MBLKP nmp);
#undef datamsg
extern	int		datamsg(int);
#undef OTHERQ
extern	queue_t *	OTHERQ(queue_t *);
#undef RD
extern	queue_t *	RD(queue_t *);
#undef WR
extern	queue_t *	WR(queue_t *);

#endif


/*
 *	str_weld.c
 */
extern	int		weldq_init(void);
extern  void            weldq_term(void);
extern	void		weldq_exec(queue_t *, queue_t *, SQP);
extern	void		unweldq_exec(queue_t *, queue_t *, SQP);

#endif /* _STR_PROTO_H */
