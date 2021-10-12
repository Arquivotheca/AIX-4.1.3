/* @(#)59       1.4  src/bos/usr/include/pse/mi.h, sysxpse, bos411, 9428A410j 11/12/93 10:57:12 */
/*
 * COMPONENT_NAME: LIBCPSE
 * 
 * ORIGINS:  27 63 71 83 
 * 
 */
/*
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * LEVEL 1, 5 Years Bull Confidential Information
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/** Copyright (c) 1988  Mentat Inc. */

#ifndef _MI_H
#define _MI_H

#include <sys/types.h>

#ifndef	nilp
#define	nilp(t)		((t *)0)
#define	nil(t)		((t)0)
#define reg		register
#endif
#define	noop		/**/
#define	fallthru	/**/

typedef char            * IDP;
typedef	int		(*pfi_t)();
typedef	pfi_t		(*pfpfi_t)();
typedef short		i16;
typedef int		i32;
typedef	u_char		u8;
typedef	u_short		u16;
typedef u_long		u32;
#define OK_32PTR(p)	(sizeof (u32) >= sizeof p)

#ifndef	MI_IOC_BASE
#define	MI_IOC_BASE	('M' << 8)
#endif

#define	MI_IOC_LL_INIT		(MI_IOC_BASE + 0)
#define	MI_IOC_LL_INIT_ECHO	(MI_IOC_BASE + 1)

#define GET_TYPE_FROM_FLAGS(flags)	(((flags) >> 8) & 0xFF)
#define	SET_TYPE_IN_FLAGS(flags, type)	(flags = ((flags) & ~0xFF00) | (((type) & 0xFF) << 8))

#define	MI_COPY_IN		1
#define	MI_COPY_OUT		2
#define	MI_COPY_DIRECTION(mp)	(*(int *)&(mp)->b_cont->b_next)
#define	MI_COPY_COUNT(mp)	(*(int *)&(mp)->b_cont->b_prev)
#define	MI_COPY_CASE(dir,cnt)	(((cnt)<<2)|dir)
#define	MI_COPY_STATE(mp)	MI_COPY_CASE(MI_COPY_DIRECTION(mp),MI_COPY_COUNT(mp))
#define	MI_IS_TRANSPARENT(mp)	(mp->b_cont && (mp->b_cont->b_rptr != mp->b_cont->b_wptr))

typedef struct gq_s {
	struct	gq_s *	gq_next;
	struct	gq_s *	gq_prev;
	char *		gq_data;
} GQ, * GQP;

extern	int		mi_addr_scanf(char *, char **, int, char *, int, char *);
extern	queue_t *	mi_allocq(struct streamtab *);
extern	void		mi_bufcall(queue_t *, int, int);
extern	int		mi_close_comm(caddr_t *, queue_t *);
extern	void		mi_copyin(queue_t *, mblk_t *, char *, int);
extern	void		mi_copyout(queue_t *, mblk_t *);
extern	mblk_t *	mi_copyout_alloc(queue_t *, mblk_t *, char *, int);
extern	void		mi_copy_done(queue_t *, mblk_t *, int);
extern	int		mi_copy_state(queue_t *, mblk_t *, mblk_t **);
extern	void		mi_free(caddr_t);
extern	char *		mi_gq_head(GQP);
extern	void		mi_gq_in(GQP, GQP);
extern	void		mi_gq_init(GQP, char *);
extern	void		mi_gq_out(GQP);
extern	int		mi_iprintf(char *, va_list, pfi_t, char *);
extern	int		mi_link_device(queue_t *, char *);
extern	int		mi_mpprintf(mblk_t *, char *, ...);
extern	int		mi_mpprintf_nr(mblk_t *, char *, ...);
extern	int		mi_mpprintf_putc(char *, char);
extern	caddr_t		mi_next_ptr(caddr_t);
extern	u8		* mi_offset_param(mblk_t *, u32, u32);
extern	u8		* mi_offset_paramc(mblk_t *, u32, u32);
extern	mblk_t		* mi_offset_param_mblk(mblk_t *, u32, u32, int *);
extern	int		mi_open_comm(caddr_t *, uint, queue_t *, dev_t *, int, int, cred_t *);
extern	mblk_t *	mi_reallocb(mblk_t *, int);
extern	int		mi_set_sth_wroff(queue_t *, int);
extern	int		mi_sprintf(char *, char *, ...);
extern	int		mi_sprintf_putc(char *, char);
extern	int		mi_strlog(queue_t *, ...);
extern	long		mi_strtol(char *, char **, int);
extern	void		mi_timer(queue_t *, mblk_t *, long);
extern	mblk_t *	mi_timer_alloc(uint);
extern	void		mi_timer_free(mblk_t *);
extern	int		mi_timer_valid(mblk_t *);
extern	int		mi_nd_get(queue_t *, char *, int, uint);
extern	int		mi_nd_set(queue_t *, char *, int, uint);
extern	mblk_t *	mi_tpi_ack_alloc(mblk_t *, uint, uint);
extern	mblk_t *	mi_tpi_conn_con(mblk_t *, char *, int, char *, int);
extern	mblk_t *	mi_tpi_conn_ind(mblk_t *, char *, int, char *, int, int);
extern	mblk_t *	mi_tpi_conn_req(mblk_t *, char *, int, char *, int);
extern	mblk_t *	mi_tpi_data_ind(mblk_t *, int, int);
extern	mblk_t *	mi_tpi_data_req(mblk_t *, int, int);
extern	mblk_t *	mi_tpi_discon_ind(mblk_t *, int, int);
extern	mblk_t *	mi_tpi_discon_req(mblk_t *, int);
extern	mblk_t *	mi_tpi_err_ack_alloc(mblk_t *, int, int);
extern	mblk_t *	mi_tpi_exdata_ind(mblk_t *, int, int);
extern	mblk_t *	mi_tpi_exdata_req(mblk_t *, int, int);
extern	mblk_t *	mi_tpi_info_req(void);
extern	mblk_t *	mi_tpi_ioctl_info_req(uint);
extern	mblk_t *	mi_tpi_ok_ack_alloc(mblk_t *);
extern	mblk_t *	mi_tpi_ordrel_ind(void);
extern	mblk_t *	mi_tpi_ordrel_req(void);
extern	mblk_t *	mi_tpi_uderror_ind(char *, int, char *, int, int);
extern	mblk_t *	mi_tpi_unitdata_ind(mblk_t *, char *, int, char *, int);
extern	mblk_t *	mi_tpi_unitdata_req(mblk_t *, char *, int, char *, int);
extern	caddr_t		mi_zalloc(uint);
extern	void		mi_bzero(char *, int);

#endif /* _MI_H */
