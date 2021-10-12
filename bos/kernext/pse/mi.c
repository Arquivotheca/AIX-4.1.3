static char sccsid[] = "@(#)28        1.7.1.4  src/bos/kernext/pse/mi.c, sysxpse, bos411, 9428A410j 6/28/94 22:07:09";
/*
 * COMPONENT_NAME: SYSXPSE - STREAMS framework
 * 
 * FUNCTIONS:      mi_addr_scanf
 *                 mi_allocq
 *                 mi_bufcall
 *                 mi_close_comm
 *                 mi_close_detached
 *                 mi_copyb
 *                 mi_copyin
 *                 mi_copyout
 *                 mi_copyout_alloc
 *                 mi_copy_done
 *                 mi_copy_state
 *                 mi_detach
 *                 mi_gq_head
 *                 mi_gq_in
 *                 mi_gq_init 
 *                 mi_gq_out
 *                 mi_iprintf
 *                 mi_itimer 
 *                 mi_itimer_fire
 *                 mi_itimer_init
 *                 mi_itimer_rsrv
 *                 mi_ibc_qenable
 *                 mi_ibc_timer
 *                 mi_ibc_timer_add
 *                 mi_ibc_timer_del
 *                 mi_mpprintf
 *                 mi_mpprintf_nr
 *                 mi_mpprintf_putc
 *                 mi_next_ptr
 *                 mi_open_comm
 *                 mi_offset_param
 *                 mi_offset_paramc
 *                 mi_offset_param_mblk
 *                 mi_panic
 *                 mi_set_sth_wroff
 *                 mi_sprintf
 *                 mi_sprintf_noop
 *                 mi_sprintf_putc
 *                 mi_strlog
 *                 mi_strtol
 *                 mi_timer
 *                 mi_timer_alloc
 *                 mi_timer_fire
 *                 mi_timer_free
 *                 mi_timer_valid
 *                 mi_nd_comm
 *                 mi_nd_get
 *                 mi_nd_set
 *                 mi_reallocb
 *                 mi_tpi_ack_alloc
 *                 mi_tpi_addr_and_opt
 *                 mi_tpi_conn_con
 *                 mi_tpi_conn_ind
 *                 mi_tpi_conn_req
 *                 mi_tpi_data_ind
 *                 mi_tpi_data_req
 *                 mi_tpi_discon_ind
 *                 mi_tpi_discon_req
 *                 mi_tpi_err_ack_alloc
 *                 mi_tpi_exdata_ind
 *                 mi_tpi_exdata_req
 *                 mi_tpi_info_req
 *                 mi_tpi_ioctl_info_req
 *                 mi_tpi_ok_ack_alloc
 *                 mi_tpi_ordrel_ind
 *                 mi_tpi_ordrel_req
 *                 mi_tpi_trailer_alloc
 *                 mi_tpi_uderror_ind
 *                 mi_tpi_unitdata_ind
 *                 mi_tpi_unitdata_req
 *                 mi_zalloc
 *                 mi_strcmp
 *                 
 *                 
 * 
 * ORIGINS: 71, 63, 83
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

#include <sys/stropts.h>
#include <sys/strstat.h>

#include <pse/mi.h>
#include <pse/nd.h>
#include <sys/tihdr.h>
#include <sys/tiuser.h>
#include <sys/timod.h>
#include <sys/strlog.h>
#include <pse/str_debug.h>

#ifndef staticf
#define staticf static
#endif

#define A_CNT(arr)      (sizeof(arr)/sizeof(arr[0]))
#define A_END(arr)      (&arr[A_CNT(arr)])
#define A_LAST(arr)     (&arr[A_CNT(arr)-1])
#define MAX_UINT        ((unsigned int)~0)

#define GET_TYPE_FROM_FLAGS(flags)	(((flags) >> 8) & 0xFF)	/**/
#define	SET_TYPE_IN_FLAGS(flags, type)	(flags = ((flags) & ~0xFF00) | (((type) & 0xFF) << 8))	/**/
#define	ISDIGIT(ch)	((ch) >= '0'  &&  (ch) <= '9')
#define	ISUPPER(ch)	((ch) >= 'A'  &&  (ch) <= 'Z')
#define	tolower(ch)	('a' + ((ch) - 'A'))

#ifndef	MI_IOC_BASE					/**/
#define	MI_IOC_BASE	('M' << 8)			/**/
#endif							/**/

#define	MI_IOC_LL_INIT		(MI_IOC_BASE + 0)	/**/
#define	MI_IOC_LL_INIT_ECHO	(MI_IOC_BASE + 1)	/**/

#define GET_TYPE_FROM_FLAGS(flags)	(((flags) >> 8) & 0xFF)	/**/
#define	SET_TYPE_IN_FLAGS(flags, type)	(flags = ((flags) & ~0xFF00) | (((type) & 0xFF) << 8))	/**/

#ifndef	MS_TO_TICKS
#define	MS_TO_TICKS(ms)	((ms) >> 3)
#endif
#define	ISDIGIT(ch)	((ch) >= '0'  &&  (ch) <= '9')
#define	ISUPPER(ch)	((ch) >= 'A'  &&  (ch) <= 'Z')
#define	tolower(ch)	('a' + ((ch) - 'A'))

#define	MI_COPY_IN		1							/**/
#define	MI_COPY_OUT		2							/**/
#define	MI_COPY_DIRECTION(mp)	(*(int *)&(mp)->b_cont->b_next)				/**/
#define	MI_COPY_COUNT(mp)	(*(int *)&(mp)->b_cont->b_prev)				/**/
#define	MI_COPY_CASE(dir,cnt)	(((cnt)<<2)|dir)					/**/
#define	MI_COPY_STATE(mp)	MI_COPY_CASE(MI_COPY_DIRECTION(mp),MI_COPY_COUNT(mp))	/**/
#define	MI_IS_TRANSPARENT(mp)	(mp->b_cont  &&  (mp->b_cont->b_rptr != mp->b_cont->b_wptr))

/* Internal buff call holder */
typedef struct ibc_s {
	struct ibc_s	* ibc_next;
	int	ibc_size;
	int	ibc_pri;
	queue_t	* ibc_q;
} IBC, * IBCP, ** IBCPP;

typedef struct mi_o_s {
	IBC		mi_o_ibcs[2];
	u32		mi_o_pattern;
	struct mi_o_s	* mi_o_next;
	int		mi_o_dev;
} MI_O, * MI_OP;

/** mblk encapsulation */
typedef struct mbe_s {	/**/
	u32	mbe_len;
	u32	mbe_type;
	u32	mbe_hlen;
	u32	mbe_tlen;
} mbe_t;

/** Lower Level Init structure */
typedef struct lli_s {	/**/
	u32	lli_cmd;
	u32	lli_name_offset;
	u32	lli_name_length;
	u32	lli_mac_type;
	u32	lli_sap;
	u32	lli_max_frag;
	u32	lli_multicast_addr_offset;
	u32	lli_multicast_addr_length;
	u32	lli_hw_addr_offset;
	u32	lli_hw_addr_length;
} lli_t;

typedef	struct stroptions * STROPTP;
typedef union T_primitives	* TPRIMP;

/* Timer block states. */
#define	TB_CANCELLED	1
#define	TB_IDLE		2
#define	TB_POSTED	3
#define	TB_TO_BE_FREED	4
#define	TB_PENDING	5

#define	TB_Q_IDLE	8
#define	TB_Q_TICKING	9
#define	TB_Q_FIRED	10

#define	TB_IS_Q_HEAD(tb)	((tb)->tb_state >= TB_Q_IDLE)

typedef struct tb_s {
	struct	tb_s	* tb_next;
	struct	tb_s	* tb_prev;
	long		tb_ticks_remaining;
	int		tb_state;
	union {
		pfi_t		tb_u_pfi;
		long		tb_u_ticks_per_timeout;
	} tb_u;
#define	tb_ticks_per_timeout	tb_u.tb_u_ticks_per_timeout
#define	tb_pfi			tb_u.tb_u_pfi
} TB, * TBP;

typedef struct mtb_s {
	TB	mtb_tb;
	queue_t	* mtb_q;
	mblk_t	* mtb_mp;
} MTB, * MTBP;

void	mi_ibc_qenable(   long long_ibc   );
void	mi_ibc_timer(   caddr_t ibcp   );
void	mi_ibc_timer_add(   IBCP ibc   );
void	mi_ibc_timer_del(   IBCP ibc   );
long	mi_itimer(   TBP tb, long ticks, pfi_t pfi   );
void	mi_itimer_fire(   caddr_t tb_param   );
TBP	mi_itimer_init(   TBP tb   );
int	mi_itimer_rsrv(   queue_t * q   );
int	mi_nd_comm(   queue_t * q, char * name, int len, uint id, int cmd   );
int	mi_timer_fire(   TBP tb   );
void	mi_tpi_addr_and_opt(   mblk_t * mp, char * addr, int addr_length, char * opt, int opt_length   );
mblk_t	* mi_tpi_trailer_alloc(   mblk_t * trailer_mp, int size, int type   );

extern  int	putq_owned(queue_t *, mblk_t *);

#define index strchr
extern	char	* index(   char * str, int ch   );

static struct module_info minfo =  {
	0, "mi_itimer", 0, INFPSZ, 2048, 128
};

static struct qinit rinit = {
	nil(pfi_t), mi_itimer_rsrv, nil(pfi_t), nil(pfi_t), nil(pfi_t), &minfo
};

static struct qinit winit = {
	nil(pfi_t), mi_itimer_rsrv, nil(pfi_t), nil(pfi_t), nil(pfi_t), &minfo
};

struct streamtab mi_itimerinfo = { &rinit, &winit };

static	IBCP	mi_g_ibc_timer_head;
static	IBCP	mi_g_ibc_timer_tail;

/* The third param in each timer queue structure specifies the
** best resolution of that timer in milliseconds.  Timer requests
** are sorted in to the coarsest queue first and migrate to
** finer resolution queues as their time remaining drops below the
** resolution of the queue they are in.  When the last request
** in a queue is cancelled, posted to the requesting client or
** migrates to a finer resolution queue, the system timer associated
** with a queue goes inactive.
*/
static	TB	mi_tb_arr[] = {
	{ nil(TBP), nil(TBP), 10 },
	{ nil(TBP), nil(TBP), 100 },
	{ nil(TBP), nil(TBP), 1000 }
};
	queue_t	* mi_g_timer_q;
#ifndef	time
extern	long	time;
#endif

int
mi_addr_scanf (str, ptr, base, dest, len, to_skip)
	char	* str;
	char	** ptr;
	int	base;
	char	* dest;
	int	len;
	char	* to_skip;
{
	int	bytes_filled = 0;
	int	ch;
	long	l1;
	char	* fstr;

	ENTER_FUNC(mi_addr_scanf, str, ptr, base, dest, len, to_skip);

	if (!str  ||  !dest  ||  len <= 0) {
		if (ptr)
			*ptr = str;
		return bytes_filled;
	}
	while (*str == ' '  ||  *str == '\t')
		str++;
	if ((base == 0  ||  base == 16)
	&&  str[0] == '0'
	&&  (str[1] == 'x'  ||  str[1] == 'X')) {
		str += 2;
		base = 16;
	}
	for (fstr = str; *str  &&  bytes_filled < len; ) {
		if (base == 16  &&  str[1]) {
			ch = str[2];
			str[2] = '\0';
		} else
			ch = '\0';
		l1 = mi_strtol(str, &fstr, base);
		if (ch)
			str[2] = ch;
		if (str == fstr  ||  l1 < 0  ||  l1 > 255)
			break;
		str = fstr;
		*dest++ = (char)l1;
		bytes_filled++;
		if (to_skip) {
			while (*str  &&  index(to_skip, *str))
				str++;
		}
	}
	if (ptr)
		*ptr = str;
	LEAVE_FUNC(mi_addr_scanf, bytes_filled);

	return bytes_filled;
}

queue_t *
mi_allocq (st)
reg	struct streamtab	* st;
{
reg	queue_t	* q;
extern	queue_t	* allocq(   void   );

	ENTER_FUNC(mi_allocq, st, 0, 0, 0, 0, 0);

	if (!st) {
		LEAVE_FUNC(mi_allocq, nilp(queue_t));
		return nilp(queue_t);
	}
	if (q = q_alloc()) {
		q->q_qinfo = st->st_rdinit;
		q->q_minpsz = st->st_rdinit->qi_minfo->mi_minpsz;
		q->q_maxpsz = st->st_rdinit->qi_minfo->mi_maxpsz;
		q->q_hiwat = st->st_rdinit->qi_minfo->mi_hiwat;
		q->q_lowat = st->st_rdinit->qi_minfo->mi_lowat;
		WR(q)->q_qinfo = st->st_wrinit;
		WR(q)->q_minpsz = st->st_wrinit->qi_minfo->mi_minpsz;
		WR(q)->q_maxpsz = st->st_wrinit->qi_minfo->mi_maxpsz;
		WR(q)->q_hiwat = st->st_wrinit->qi_minfo->mi_hiwat;
		WR(q)->q_lowat = st->st_wrinit->qi_minfo->mi_lowat;
	}
	LEAVE_FUNC(mi_allocq, q);
	return q;
}

void
mi_bufcall (q, size, pri)
	queue_t	* q;
	int	size;
	int	pri;
{
reg	MI_OP	mi_o;
reg	IBCP	ibc;

	ENTER_FUNC(mi_bufcall, q, size, pri, 0, 0, 0);

	/* Encode which ibc is used in sign of ibc_size field */
	size++;
	if (!q  ||  !(mi_o = (MI_OP)q->q_ptr) ||  size <= 0)
		return;
	--mi_o;
	if (mi_o->mi_o_ibcs[0].ibc_q == q
	||  mi_o->mi_o_ibcs[1].ibc_q == q)
		return;
	if (!mi_o->mi_o_ibcs[0].ibc_q) {
		ibc = &mi_o->mi_o_ibcs[0];
		ibc->ibc_size = size;
	} else if (!mi_o->mi_o_ibcs[1].ibc_q) {
		ibc = &mi_o->mi_o_ibcs[1];
		ibc->ibc_size = -size;
	} else
		return;
	ibc->ibc_pri = pri;
	ibc->ibc_q = q;
	if (!bufcall(size - 1, pri, mi_ibc_qenable, ibc))
		mi_ibc_timer_add(ibc);
	LEAVE_FUNC(mi_bufcall, 0);
}

int
mi_close_comm (mi_opp_orig, q)
	caddr_t	* mi_opp_orig;
	queue_t	* q;
{
reg	MI_OP	mi_o;
reg	MI_OP	* mi_opp;

	ENTER_FUNC(mi_close_comm, mi_opp_orig, q, 0, 0, 0, 0);

	if ((mi_opp = (MI_OP *)mi_opp_orig)  &&  q  &&  q->q_ptr) {
		for (; mi_o = mi_opp[0]; mi_opp = &mi_o->mi_o_next) {
			mi_o--;
			if ((caddr_t)mi_opp[0] == q->q_ptr) {
				mi_opp[0] = mi_o->mi_o_next;
				q->q_ptr = nil(caddr_t);
				OTHERQ(q)->q_ptr = nil(caddr_t);
				break;
			}
		}
		if (mi_o) {
			mi_o->mi_o_dev = OPENFAIL;
			mi_ibc_timer_del(&mi_o->mi_o_ibcs[0]);
			mi_ibc_timer_del(&mi_o->mi_o_ibcs[1]);
			if (!mi_o->mi_o_ibcs[0].ibc_q
			&&  !mi_o->mi_o_ibcs[1].ibc_q)
				he_free((caddr_t)mi_o);
		}
	}

	LEAVE_FUNC(mi_close_comm, 0);

	return 0;
}

void
mi_close_detached (mi_opp_orig, ptr)
	caddr_t	* mi_opp_orig;
	caddr_t	ptr;
{
reg	MI_OP	mi_o;
reg	MI_OP	* mi_opp;

	ENTER_FUNC(mi_close_detached, mi_opp_orig, ptr, 0, 0, 0, 0);

	if ((mi_opp = (MI_OP *)mi_opp_orig)  &&  ptr) {
		for (; mi_o = mi_opp[0]; mi_opp = &mi_o->mi_o_next) {
			mi_o--;
			if ((caddr_t)mi_opp[0] == ptr) {
				mi_opp[0] = mi_o->mi_o_next;
				break;
			}
		}
		if (mi_o) {
			mi_o->mi_o_dev = OPENFAIL;
			mi_ibc_timer_del(&mi_o->mi_o_ibcs[0]);
			mi_ibc_timer_del(&mi_o->mi_o_ibcs[1]);
			if (!mi_o->mi_o_ibcs[0].ibc_q
			&&  !mi_o->mi_o_ibcs[1].ibc_q)
				he_free((caddr_t)mi_o);
		}
	}

	LEAVE_FUNC(mi_close_detached, 0);
}

/** Copy a message block.  (Used only in environments where the native version is suspect.) */
mblk_t *
mi_copyb (bp)
	mblk_t	* bp;
{
reg	dblk_t	* dp, * new_dp;
reg	mblk_t	* new_bp;

	ENTER_FUNC(mi_copyb, bp, 0, 0, 0, 0, 0);
	dp = bp->b_datap;
	if (!(new_bp = allocb(dp->db_lim - dp->db_base, BPRI_MED)))
	{
		LEAVE_FUNC(mi_copyb, nil(mblk_t *));
		return nil(mblk_t *);
	}
	new_dp = new_bp->b_datap;
	new_bp->b_rptr = new_dp->db_base + (bp->b_rptr - dp->db_base);
	new_bp->b_wptr = new_dp->db_base + (bp->b_wptr - dp->db_base);
	new_bp->b_band = bp->b_band;
	new_bp->b_flag = bp->b_flag;
	new_dp->db_type = dp->db_type;
	bcopy((char *)bp->b_rptr, (char *)new_bp->b_rptr, bp->b_wptr - bp->b_rptr);

	LEAVE_FUNC(mi_copyb, new_bp);
	return new_bp;
}

void
mi_copyin (q, mp, uaddr, len)
	queue_t	* q;
	mblk_t	* mp;
	char	* uaddr;
	int	len;
{
	struct iocblk * iocp = (struct iocblk *)mp->b_rptr;
	struct copyreq * cq;
	int	err;
	mblk_t	* mp1;
	
	if ( mp->b_datap->db_type == M_IOCTL ) {
		if (iocp->ioc_count != TRANSPARENT) {
			mp1 = mp->b_cont;
			if (!uaddr
			&& (!mp1  ||  (mp1->b_wptr - mp1->b_rptr) < len)) {
				err = EINVAL;
				goto err_ret;
			}
			mp1 = allocb(0, BPRI_MED);
			if (!mp1) {
				err = ENOMEM;
				goto err_ret;
			}
			mp1->b_cont = mp->b_cont;
			mp->b_cont = mp1;
		}
		MI_COPY_COUNT(mp) = 0;
		mp->b_datap->db_type = M_IOCDATA;
	} else if (!uaddr) {
		err = EPROTO;
		goto err_ret;
	}
	mp1 = mp->b_cont;
	cq = (struct copyreq *)iocp;
	cq->cq_private = mp1;
	cq->cq_size = len;
	cq->cq_addr = uaddr;
	cq->cq_flag = 0;
	MI_COPY_DIRECTION(mp) = MI_COPY_IN;
	MI_COPY_COUNT(mp)++;
	if (!uaddr) {
		if ( !MI_IS_TRANSPARENT(mp) ) {
			struct copyresp * cp = (struct copyresp *)mp->b_rptr;
			mp->b_datap->db_type = M_IOCDATA;
			mp->b_wptr = mp->b_rptr + sizeof(struct copyresp);
			mp->b_cont = mp1->b_cont;
			cp->cp_private->b_cont = nilp(mblk_t);
			cp->cp_rval = 0;
			puthere(q, mp);
			return;
		}
		bcopy((char *)mp1->b_rptr, (char *)&cq->cq_addr, sizeof(cq->cq_addr));
	}
	mp->b_cont = nilp(mblk_t);
	mp->b_datap->db_type = M_COPYIN;
	mp->b_wptr = mp->b_rptr + sizeof(struct copyreq);
	qreply(q, mp);
	return;
err_ret:
	iocp->ioc_error = err;
	iocp->ioc_count = 0;
	if ( mp->b_cont ) {
		freemsg(mp->b_cont);
		mp->b_cont = nilp(mblk_t);
	}
	mp->b_wptr = mp->b_rptr + sizeof(struct iocblk);
	mp->b_datap->db_type = M_IOCACK;
	qreply(q, mp);
	return;
}

void
mi_copyout (q, mp)
	queue_t	* q;
	mblk_t	* mp;
{
	struct iocblk * iocp = (struct iocblk *)mp->b_rptr;
	struct copyreq * cq = (struct copyreq *)iocp;
	struct copyresp * cp = (struct copyresp *)cq;
	mblk_t	* mp1;
	mblk_t	* mp2;
	
	if ( mp->b_datap->db_type != M_IOCDATA  ||  !mp->b_cont) {
		mi_copy_done(q, mp, EPROTO);
		return;
	}
	/* Check completion of previous copyout operation. */
	mp1 = mp->b_cont;
	if ( (int)cp->cp_rval  ||  !mp1->b_cont) {
		mi_copy_done(q, mp, (int)cp->cp_rval);
		return;
	}
	if (!mp1->b_cont->b_cont  &&  !MI_IS_TRANSPARENT(mp)) {
		mp1->b_next = nilp(mblk_t);
		mp1->b_prev = nilp(mblk_t);
		mp->b_cont = mp1->b_cont;
		freeb(mp1);
		mp1 = mp->b_cont;
		mp1->b_next = nilp(mblk_t);
		mp1->b_prev = nilp(mblk_t);
		iocp->ioc_count = mp1->b_wptr - mp1->b_rptr;
		iocp->ioc_error = 0;
		mp->b_datap->db_type = M_IOCACK;
		mp->b_wptr = mp->b_rptr + sizeof(struct iocblk);
		qreply(q, mp);
		return;
	}
	if (MI_COPY_DIRECTION(mp) == MI_COPY_IN) {
		/* Set up for first copyout. */
		MI_COPY_DIRECTION(mp) = MI_COPY_OUT;
		MI_COPY_COUNT(mp) = 1;
	} else {
		++MI_COPY_COUNT(mp);
	}
	cq->cq_private = mp1;
	/* Find message preceding last. */
	for ( mp2 = mp1; mp2->b_cont->b_cont; mp2 = mp2->b_cont )
		;
	if (mp2 == mp1)
		bcopy((char *)mp1->b_rptr, (char *)&cq->cq_addr, sizeof(cq->cq_addr));
	else
		cq->cq_addr = (char *)mp2->b_cont->b_next;
	mp1 = mp2->b_cont;
	mp->b_datap->db_type = M_COPYOUT;
	mp->b_wptr = mp->b_rptr + sizeof(struct copyreq);
	mp->b_cont = mp1;
	mp2->b_cont = nilp(mblk_t);
	mp1->b_next = nilp(mblk_t);
	cq->cq_size = mp1->b_wptr - mp1->b_rptr;
	cq->cq_flag = 0;
	qreply(q, mp);
	return;
}

mblk_t *
mi_copyout_alloc (q, mp, uaddr, len)
	queue_t	* q;
	mblk_t	* mp;
	char	* uaddr;
	int	len;
{
	struct iocblk * iocp = (struct iocblk *)mp->b_rptr;
	mblk_t	* mp1;

	if ( mp->b_datap->db_type == M_IOCTL ) {
		if (iocp->ioc_count != TRANSPARENT) {
			mp1 = allocb(0, BPRI_MED);
			if (!mp1) {
				iocp->ioc_error = ENOMEM;
				iocp->ioc_count = 0;
				freemsg(mp->b_cont);
				mp->b_cont = nilp(mblk_t);
				mp->b_datap->db_type = M_IOCACK;
				qreply(q, mp);
				return nilp(mblk_t);
			}
			mp1->b_cont = mp->b_cont;
			mp->b_cont = mp1;
		}
		MI_COPY_COUNT(mp) = 0;
		MI_COPY_DIRECTION(mp) = MI_COPY_OUT;
		/* Make sure it looks clean to mi_copyout. */
		mp->b_datap->db_type = M_IOCDATA;
		((struct copyresp *)iocp)->cp_rval = 0;
	}
	mp1 = allocb(len, BPRI_MED);
	if (!mp1) {
		if (q)
			mi_copy_done(q, mp, ENOMEM);
		return nilp(mblk_t);
	}
	linkb(mp, mp1);
	mp1->b_next = (mblk_t *)uaddr;
	return mp1;
}

void
mi_copy_done (q, mp, err)
	queue_t	* q;
	mblk_t	* mp;
	int	err;
{
	struct iocblk * iocp;
	mblk_t	* mp1;
	
	if (!mp)
		return;
	if (!q  ||  (mp->b_wptr - mp->b_rptr) < sizeof(struct iocblk)) {
		freemsg(mp);
		return;
	}
	iocp = (struct iocblk *)mp->b_rptr;
	mp->b_datap->db_type = M_IOCACK;
	mp->b_wptr = mp->b_rptr + sizeof(struct iocblk);
	iocp->ioc_error = err;
	iocp->ioc_count = 0;
	if (mp1 = mp->b_cont) {
		for ( ; mp1; mp1 = mp1->b_cont) {
			mp1->b_prev = nilp(mblk_t);
			mp1->b_next = nilp(mblk_t);
		}
		freemsg(mp->b_cont);
		mp->b_cont = nilp(mblk_t);
	}
	qreply(q, mp);
}

int
mi_copy_state (q, mp, mpp)
	queue_t	* q;
	mblk_t	* mp;
	mblk_t	* * mpp;
{
	struct iocblk * iocp = (struct iocblk *)mp->b_rptr;
	struct copyresp * cp = (struct copyresp *)iocp;
	mblk_t	* mp1;
	
	mp1 = mp->b_cont;
	mp->b_cont = cp->cp_private;
	if (mp1)
		linkb(mp->b_cont, mp1);
	if ( cp->cp_rval ) {
		mi_copy_done(q, mp, (int)cp->cp_rval);
		return -1;
	}
	if ( mpp  &&  MI_COPY_DIRECTION(mp) == MI_COPY_IN )
		*mpp = mp1;
	return MI_COPY_STATE(mp);
}

void
mi_detach (ptr)
	caddr_t	ptr;
{
reg	MI_OP	mi_o = (MI_OP)ptr;

	if (mi_o) {
		mi_o--;
		mi_o->mi_o_dev = OPENFAIL;
	}
}

char *
mi_gq_head (h)
	GQP	h;
{
	GQP	a;

	if (h  &&  (a = h->gq_next)  &&  a != h) {
		mi_gq_out(a);
		return a->gq_data;
	}
	return nilp(char);
}

void
mi_gq_in (h, t)
reg	GQP	h;
reg	GQP	t;
{
	if (h  &&  t) {
		mi_gq_out(t);
		h->gq_prev->gq_next = t;
		t->gq_prev = h->gq_prev;
		h->gq_prev = t;
		t->gq_next = h;
	}
}

void
mi_gq_init (a, d)
reg	GQP	a;
	char	* d;
{
	if (a) {
		a->gq_next = a;
		a->gq_prev = a;
		a->gq_data = d;
	}
}

void
mi_gq_out (a)
reg	GQP	a;
{
	if (a) {
		if (a->gq_next) {
			a->gq_prev->gq_next = a->gq_next;
			a->gq_next->gq_prev = a->gq_prev;
		}
		a->gq_prev = a;
		a->gq_next = a;
	}
}

int
mi_iprintf (fmt, ap, putc_func, cookie)
reg	char	* fmt;
	va_list	ap;
	pfi_t	putc_func;
	char	* cookie;
{
reg	int	base;
	char	buf[(sizeof(long) * 3) + 1];
static	char	hex_val[] = "0123456789abcdef";
	int	ch;
	int	count;
	char	* cp1;
	int	digits;
reg	char	* fcp;
	int	is_long;
	ulong	uval;
	long	val;
	int	zero_filled;

	if (!fmt)
		return -1;
	count = 0;
	while (*fmt) {
		if (*fmt != '%' || *++fmt == '%') {
			count += (*putc_func)(cookie, *fmt++);
			continue;
		}
		if (*fmt == '0') {
			zero_filled = 1;
			fmt++;
			if (!*fmt)
				break;
		} else
			zero_filled = 0;
		base = 0;
		for (digits = 0; ISDIGIT(*fmt); fmt++) {
			digits *= 10;
			digits += (*fmt - '0');
		}
		if (!*fmt)
			break;
		is_long = 0;
		if (*fmt == 'l') {
			is_long = 1;
			fmt++;
		}
		if (!*fmt)
			break;
		ch = *fmt++;
		if (ISUPPER(ch)) {
			ch = tolower(ch);
			is_long = 1;
		}
		switch (ch) {
		case 'c':
			count += (*putc_func)(cookie, va_arg(ap, int *));
			continue;
		case 'd':
			base = 10;
			break;
		case 'm':	/* Print out memory, 2 hex chars per byte */
			if (is_long)
				fcp = va_arg(ap, char *);
			else {
				if (cp1 = va_arg(ap, char *))
					fcp = (char *)cp1;
				else
					fcp = nilp(char);
			}
			if (!fcp) {
				for (fcp = (char *)"(NULL)"; *fcp; fcp++)
					count += (*putc_func)(cookie, *fcp);
			} else {
				while (digits--) {
					int u1 = *fcp++ & 0xFF;
					count += (*putc_func)(cookie,hex_val[(u1>>4)& 0xF]);
					count += (*putc_func)(cookie,hex_val[u1& 0xF]);
				}
			}
			continue;
		case 'o':
			base = 8;
			break;
		case 'x':
			base = 16;
			break;
		case 's':
			if (is_long)
				fcp = va_arg(ap, char *);
			else {
				if (cp1 = va_arg(ap, char *))
					fcp = (char *)cp1;
				else
					fcp = nilp(char);
			}
			if (!fcp)
				fcp = (char *)"(NULL)";
			while (*fcp) {
				count += (*putc_func)(cookie, *fcp++);
				if (digits  &&  --digits == 0)
					break;
			}
			while (digits > 0) {
				count += (*putc_func)(cookie, ' ');
				digits--;
			}
			continue;
		case 'u':
			base = 10;
			break;
		default:
			return count;
		}
		if (is_long)
			val = va_arg(ap, long);
		else
			val = va_arg(ap, int);
		if (base == 10  &&  ch != 'u') {
			if (val < 0) {
				count += (*putc_func)(cookie, '-');
				val = -val;
			}
			uval = val;
		} else {
			if (is_long)
				uval = val;
			else
				uval = (unsigned)val;
		}
		/* Hand overload/restore the register variable 'fmt' */
		cp1 = fmt;
		fmt = A_END(buf);
		*--fmt = '\0';
		do {
			if (fmt > buf)
				*--fmt = hex_val[uval % base];
			if (digits  &&  --digits == 0)
				break;
		} while (uval /= base);
		if (zero_filled) {
			while (digits > 0  &&  fmt > buf) {
				*--fmt = '0';
				digits--;
			}
		}
		while (*fmt)
			count += (*putc_func)(cookie, *fmt++);
		fmt = cp1;
	}
	return count;
}

long
mi_itimer (tb, ticks, pfi)
reg	TBP	tb;
	long	ticks;
	pfi_t	pfi;
{
	int	old_state;
	int	savpri;
reg	TBP	tb1;
reg	TBP	tb2;


	ENTER_FUNC(mi_itimer, tb, ticks, pfi, 0, 0, 0);

	if (!tb) {
		LEAVE_FUNC(mi_itimer, 0L);
		return 0L;
	}
	if (ticks >= 0  &&  pfi) {
		for (tb1 = A_LAST(mi_tb_arr); tb1 != mi_tb_arr; tb1--) {
			if (tb1->tb_ticks_per_timeout <= ticks)
				break;
		}
	}
	savpri = splstr();
	old_state = tb->tb_state;
	if (tb->tb_state == TB_PENDING) {
		if (!TB_IS_Q_HEAD(tb->tb_next))
			tb->tb_next->tb_ticks_remaining += tb->tb_ticks_remaining;
		tb->tb_next->tb_prev = tb->tb_prev;
		tb->tb_prev->tb_next = tb->tb_next;
		tb->tb_next = tb;
		tb->tb_prev = tb;
	}
	if (ticks < 0  ||  !pfi) {
		tb->tb_state = TB_CANCELLED;
		splx(savpri);
		LEAVE_FUNC(mi_itimer, old_state); 
		return old_state;
	}
	if (tb->tb_state == TB_POSTED) {
		tb->tb_ticks_remaining = ticks;
		splx(savpri);
		LEAVE_FUNC(mi_itimer, old_state);
		return old_state;
	}
	if (tb1->tb_state != TB_Q_IDLE)
		ticks += (tb1->tb_ticks_per_timeout - 1);
	for (tb2 = tb1->tb_next; tb2 != tb1; tb2 = tb2->tb_next) {
		if (tb2->tb_ticks_remaining > ticks) {
			tb2->tb_ticks_remaining -= ticks;
			break;
		}
		ticks -= tb2->tb_ticks_remaining;
	}
	tb->tb_pfi = pfi;
	tb->tb_ticks_remaining = ticks;
	tb->tb_state = TB_PENDING;
	tb->tb_prev = tb2->tb_prev;
	tb->tb_next = tb2;
	tb2->tb_prev->tb_next = tb;
	tb2->tb_prev = tb;
	if (tb1->tb_state == TB_Q_IDLE)
		tb1->tb_state = TB_Q_TICKING;
	else
		tb1 = nil(TBP);
	splx(savpri);
	if (tb1)
		timeout(mi_itimer_fire, (caddr_t)tb1, (int)tb1->tb_ticks_per_timeout);
	LEAVE_FUNC(mi_itimer, old_state);
	return old_state;
}

void
mi_itimer_fire (tb_param)
	caddr_t	tb_param;
{
reg	TBP	tb = (TBP)tb_param;

	ENTER_FUNC(mi_itimer_fire, tb_param, 0, 0, 0, 0, 0);
	tb->tb_state = TB_Q_FIRED;
	qenable(mi_g_timer_q);
	LEAVE_FUNC(mi_itimer_fire, 0);
}

TBP
mi_itimer_init (tb)
	TBP	tb;
{
reg	TBP	tb1;

	ENTER_FUNC(mi_itimer_init, tb, 0, 0, 0, 0, 0);
	if (!tb) {
		LEAVE_FUNC(mi_itimer_init, nil(TBP));
		return nil(TBP);
	}
	if (!mi_g_timer_q) {
		if (!(mi_g_timer_q = mi_allocq(&mi_itimerinfo))) {
			LEAVE_FUNC(mi_itimer_init, nil(TBP));
			return nil(TBP);
		}

		mi_g_timer_q->q_sqh.sqh_parent = &mi_g_timer_q->q_sqh;
		WR(mi_g_timer_q)->q_sqh.sqh_parent = &mi_g_timer_q->q_sqh;

		for (tb1 = mi_tb_arr; tb1 < A_END(mi_tb_arr); tb1++) {
			tb1->tb_ticks_per_timeout = MS_TO_TICKS(tb1->tb_ticks_remaining);
			if (tb1->tb_ticks_per_timeout <= 0)
				tb1->tb_ticks_per_timeout = 1;
			tb1->tb_next = tb1;
			tb1->tb_prev = tb1;
			tb1->tb_state = TB_Q_IDLE;
		}
	}
	tb1 = tb;
	tb1->tb_next = tb1;
	tb1->tb_prev = tb1;
	tb1->tb_state = TB_IDLE;
	LEAVE_FUNC(mi_itimer_init, tb1);
	return tb1;
}

int
mi_itimer_rsrv (q)
	queue_t	* q;
{
reg	TBP	tb, tb1;
	TB	posting;
	long	min_ticks;
	long	ticks_remaining;

	ENTER_FUNC(mi_itimer_rsrv, q, 0, 0, 0, 0, 0);
	tb = &posting;
	tb->tb_next = tb;
	tb->tb_prev = tb;
	tb->tb_state = TB_Q_IDLE;
	min_ticks = mi_tb_arr[0].tb_ticks_per_timeout;
	for (tb = mi_tb_arr; tb < A_END(mi_tb_arr); tb++) {
		int savpri = splstr();
		if (tb->tb_state != TB_Q_FIRED) {
			splx(savpri);
			continue;
		}
		for (;;) {
			tb1 = tb->tb_next;
			if (tb1 == tb) {
				tb->tb_state = TB_Q_IDLE;
				splx(savpri);
				break;
			}
			ticks_remaining = tb1->tb_ticks_remaining - tb->tb_ticks_per_timeout;
			if (ticks_remaining >= tb->tb_ticks_per_timeout) {
				tb1->tb_ticks_remaining = ticks_remaining;
				tb->tb_state = TB_Q_TICKING;
				splx(savpri);
				timeout(mi_itimer_fire, (caddr_t)tb, (int)tb->tb_ticks_per_timeout);
				break;
			}
			if (ticks_remaining < min_ticks) {
				if (!TB_IS_Q_HEAD(tb1->tb_next))
					tb1->tb_next->tb_ticks_remaining += tb1->tb_ticks_remaining;
				tb->tb_next = tb1->tb_next;
				tb1->tb_next->tb_prev = tb;
				tb1->tb_next = &posting;
				tb1->tb_prev = posting.tb_prev;
				posting.tb_prev->tb_next = tb1;
				posting.tb_prev = tb1;
			} else {
				splx(savpri);
				(void)mi_itimer(tb1, ticks_remaining, tb1->tb_pfi);
				savpri = splstr();
			}
		}
	}
	tb = &posting;
	for (;;) {
		int savpri = splstr();
		tb1 = tb->tb_next;
		if (tb1 == tb) {
			splx(savpri);
			break;
		}
		tb->tb_next = tb1->tb_next;
		tb1->tb_next->tb_prev = tb;
		tb1->tb_next = tb1;
		tb1->tb_prev = tb1;
		tb1->tb_state = TB_POSTED;
		tb1->tb_ticks_remaining = 0;	/* We're not just trying to be neat here!  (See mi_timer_valid.) */
		splx(savpri);
		(*tb1->tb_pfi)(tb1);
	}
	LEAVE_FUNC(mi_itimer_rsrv, 0);
	return 0;
}

void
mi_ibc_qenable (long_ibc)
	long	long_ibc;
{
reg	IBCP	ibc = (IBCP)long_ibc;
reg	MI_OP	mi_o;

	ENTER_FUNC(mi_ibc_qenable, long_ibc, 0, 0, 0, 0, 0);

	mi_o = (MI_OP)((ibc->ibc_size < 0) ? &ibc[-1] : ibc);
	if (mi_o->mi_o_dev == OPENFAIL) {
		if (!mi_o->mi_o_ibcs[0].ibc_q  &&  !mi_o->mi_o_ibcs[1].ibc_q)
			he_free((caddr_t)mi_o);
	} else if (ibc->ibc_q)
		qenable(ibc->ibc_q);
	ibc->ibc_q = nilp(queue_t);
	
	LEAVE_FUNC(mi_ibc_qenable, 0);
}

void
mi_ibc_timer (ibcp)
	caddr_t	ibcp;
{
reg	IBCP	ibc;
	int	size;
	
	ENTER_FUNC(mi_ibc_timer, ibcp, 0, 0, 0, 0, 0);

	while (ibc = *(IBCPP)ibcp) {
		size = (ibc->ibc_size < 0) ? -ibc->ibc_size : ibc->ibc_size;
		if (!bufcall(size - 1, ibc->ibc_pri, mi_ibc_qenable, ibc))
			break;
		mi_ibc_timer_del(ibc);
	}

	LEAVE_FUNC(mi_ibc_timer, 0);
}

void
mi_ibc_timer_add (ibc)
reg	IBCP	ibc;
{

	ENTER_FUNC(mi_ibc_timer_add, ibc, 0, 0, 0, 0, 0);

	if (!ibc->ibc_next  &&  ibc != mi_g_ibc_timer_tail) {
		if (mi_g_ibc_timer_head)
			mi_g_ibc_timer_tail->ibc_next = ibc;
		else {
			mi_g_ibc_timer_head = ibc;
			timeout(mi_ibc_timer, (caddr_t)&mi_g_ibc_timer_head, MS_TO_TICKS(4000));
		}
		mi_g_ibc_timer_tail = ibc;
	}

	LEAVE_FUNC(mi_ibc_timer_add, 0);
}

void
mi_ibc_timer_del (ibc)
reg	IBCP	ibc;
{
reg	IBCPP	ibcp;
	IBCP	prev_ibc = nil(IBCP);

	ENTER_FUNC(mi_ibc_timer_del, ibc, 0, 0, 0, 0, 0);

	for (ibcp = &mi_g_ibc_timer_head; ibcp[0]; ibcp = &ibcp[0]->ibc_next) {
		if (ibcp[0] == ibc) {
			ibcp[0] = ibc->ibc_next;
			if (ibc == mi_g_ibc_timer_tail)
				mi_g_ibc_timer_tail = prev_ibc;
			break;
		}
		prev_ibc = ibcp[0];
	}

	LEAVE_FUNC(mi_ibc_timer_del, 0);
}

int
mi_mpprintf (
	mblk_t	* mp,
	char	* fmt,
	...)
{
	va_list	ap;
	int	count = -1;

	ENTER_FUNC(mi_mpprintf, mp, fmt, 0, 0, 0, 0);

	va_start(ap, fmt);
	if (mp) {
		count = mi_iprintf(fmt, ap, (pfi_t)mi_mpprintf_putc, (char *)mp);
		if (count != -1)
			mi_mpprintf_putc((char *)mp, '\0');
	}
	va_end(ap);

	LEAVE_FUNC(mi_mpprintf, count);
	return count;
}

int
mi_mpprintf_nr (MBLKP mp, char * fmt, ...)
{
	va_list	ap;
	int	count = -1;
	MBLKP	mp1;

	ENTER_FUNC(mi_mpprintf_nr, mp, fmt, 0, 0, 0, 0);

	va_start(ap, fmt);
	if (mp) {
		adjmsg(mp, -1);
		count = mi_iprintf(fmt, ap, (pfi_t)mi_mpprintf_putc, (char *)mp);
		if (count != -1)
			mi_mpprintf_putc((char *)mp, '\0');
	}
	va_end(ap);

	LEAVE_FUNC(mi_mpprintf_nr, count);
	return count;
}

int
mi_mpprintf_putc (
	char	* cookie,
	char	ch)
{
reg	mblk_t	* mp = (mblk_t *)cookie;

	ENTER_FUNC(mi_mpprintf_putc, cookie, ch, 0, 0, 0, 0);

	while (mp->b_wptr >= mp->b_datap->db_lim) {
		if (!(mp = mp->b_cont)) {
			LEAVE_FUNC(mi_mpprintf_putc, 0);
			return 0;
		}
	}
	*mp->b_wptr++ = (unsigned char)ch;

	LEAVE_FUNC(mi_mpprintf_putc, 1);
	return 1;
}

caddr_t
mi_next_ptr (ptr)
	caddr_t	ptr;
{
reg	MI_OP	mi_op;

	ENTER_FUNC(mi_next_ptr, ptr, 0, 0, 0, 0, 0);

	if (mi_op = (MI_OP)ptr) {
		LEAVE_FUNC(mi_next_ptr, (caddr_t)mi_op[-1].mi_o_next);
		return (caddr_t)mi_op[-1].mi_o_next;
	}
	LEAVE_FUNC(mi_next_ptr, nil(caddr_t)); 
	return nil(caddr_t);
}

/*
 * If sflag == CLONEOPEN, search for the lowest number available.
 * If sflag != CLONEOPEN then attempt to open the 'dev' given.
 */
int
mi_open_comm (mi_opp_orig, size, q, devp, flag, sflag, credp)
	caddr_t	* mi_opp_orig;
	uint	size;
	queue_t	* q;
	dev_t	* devp;
	int	flag;
	int	sflag;
	cred_t	* credp;
{
reg	MI_OP	mi_o;
reg	MI_OP	* mi_opp;
	dev_t	dev;

	ENTER_FUNC(mi_open_comm, mi_opp_orig, size, q, devp, flag, sflag);

	if (!mi_opp_orig ||  !q  ||  size > (MAX_UINT - sizeof(MI_O))) {
		LEAVE_FUNC(mi_open_comm, ENXIO);
		return ENXIO;
	}
	if (q->q_ptr) {
		LEAVE_FUNC(mi_open_comm, 0);
		return 0;
	}
	if (sflag == MODOPEN) {
		sflag = CLONEOPEN;
		devp = nilp(dev_t);
	} else if ( !devp ) {
		LEAVE_FUNC(mi_open_comm, ENXIO);
		return ENXIO;
	}
	if (sflag == CLONEOPEN)
		dev = 5;	/* Leave a few for special opens */
	else
		dev = minor(*devp);
	mi_opp = (MI_OP *)mi_opp_orig;
	if (mi_o = *mi_opp) {
		do {
			mi_o--;
			if (mi_o->mi_o_dev > dev
			&&  mi_o->mi_o_dev != OPENFAIL)
				break;
			if (sflag == CLONEOPEN)
				dev++;
			else if (mi_o->mi_o_dev == dev) {
				LEAVE_FUNC(mi_open_comm, 0);
				return 0;
			}
			mi_opp = &mi_o->mi_o_next;
		} while (mi_o = mi_o->mi_o_next);
	}
	if (dev == OPENFAIL
	|| !(mi_o = (MI_OP)mi_zalloc(size + sizeof(MI_O)))) {
		LEAVE_FUNC(mi_open_comm, EAGAIN);
		return EAGAIN;
	}
	mi_o->mi_o_pattern = (u32)0x03020100L;
	mi_o->mi_o_dev = dev;
	mi_o->mi_o_next = *mi_opp;
	mi_o++;
	*mi_opp = mi_o;
	q->q_ptr = (caddr_t)mi_o;
	OTHERQ(q)->q_ptr = (caddr_t)mi_o;
	if ( devp )
		*devp = makedev(major(*devp), dev);
	LEAVE_FUNC(mi_open_comm, 0);
	return 0;
}

u8 *
mi_offset_param (mp, offset, len)
	mblk_t	* mp;
	u32	offset;
	u32	len;
{
	unsigned int	msg_len;

	ENTER_FUNC(mi_offset_param, mp, offset, len, 0, 0, 0);
	if (!mp) {
		LEAVE_FUNC(mi_offset_param,  nilp(u8));
		return nilp(u8);
	}
	msg_len = mp->b_wptr - mp->b_rptr;
	if ((int)msg_len <= 0
	||  offset > msg_len
	||  len > msg_len
	||  (offset + len) > msg_len
	||  len == 0) {
		LEAVE_FUNC(mi_offset_param,  nilp(u8));
		return nilp(u8);
	}
	LEAVE_FUNC(mi_offset_param, &mp->b_rptr[offset]);
	return &mp->b_rptr[offset];
}

u8 *
mi_offset_paramc (mp, offset, len)
	mblk_t	* mp;
	u32	offset;
	u32	len;
{
	u8	* param;

	ENTER_FUNC(mi_offset_paramc, mp, offset, len, 0, 0, 0);
	for ( ; mp; mp = mp->b_cont) {
		int type = mp->b_datap->db_type;
		if (datamsg(type)) {
			if (param = mi_offset_param(mp, offset, len)) {
				LEAVE_FUNC(mi_offset_paramc, param);
				return param;
			}
			if (offset < (mp->b_wptr - mp->b_rptr))
				break;
			offset -= (mp->b_wptr - mp->b_rptr);
		}
	}
	LEAVE_FUNC(mi_offset_paramc, nilp(u8));
	return nilp(u8);
}

mblk_t *
mi_offset_param_mblk (mp, offset, length, err)
	mblk_t	* mp;
	u32	offset;
	u32	length;
	int	* err;
{
reg	mbe_t	* mbe;
	u8	* ucp = mi_offset_param(mp, offset, length);

	ENTER_FUNC(mi_offset_param_mblk, mp, offset, length, err, 0, 0);
	if (length == 0) {
		if (err)
			*err = 0;
		LEAVE_FUNC(mi_offset_param_mblk, nilp(mblk_t));
		return nilp(mblk_t);
	}
	if (length < sizeof(mbe_t)
	||  !(mbe = (mbe_t *)ucp)
	||  !OK_32PTR(ucp)
	||  (length < mbe->mbe_len + sizeof(mbe_t))
	||  mbe->mbe_hlen > 4096
	||  mbe->mbe_tlen > 4096
	||  mbe->mbe_len > 8192) {
		if (err)
			*err = EINVAL;
		LEAVE_FUNC(mi_offset_param_mblk, nilp(mblk_t));
		return nilp(mblk_t);
	}
	switch (mbe->mbe_type) {
	case M_DATA:
	case M_PROTO:
	case M_PCPROTO:
		break;
	default:
		if (err)
			*err = EINVAL;
		LEAVE_FUNC(mi_offset_param_mblk, nilp(mblk_t));
		return nilp(mblk_t);
	}
	mp = allocb(mbe->mbe_len + mbe->mbe_hlen + mbe->mbe_tlen, BPRI_MED);
	if (!mp) {
		if (err)
			*err = EAGAIN;
		LEAVE_FUNC(mi_offset_param_mblk, nilp(mblk_t));
		return nilp(mblk_t);
	}
	mp->b_rptr = mp->b_datap->db_base + mbe->mbe_hlen;
	mp->b_wptr = mp->b_rptr + mbe->mbe_len;
	if (mbe->mbe_len)
		bcopy((char *)&mbe[1], (char *)mp->b_rptr, mbe->mbe_len);
	mp->b_datap->db_type = mbe->mbe_type;
	if (err)
		*err = 0;
	LEAVE_FUNC(mi_offset_param_mblk, mp);
	return mp;
}

int
mi_panic (
	char	* fmt,
	...)
{
	va_list	ap;
	char	lbuf[256];
	char	* buf = lbuf;

	ENTER_FUNC(mi_panic, fmt, 0, 0, 0, 0, 0);
	va_start(ap, fmt);
	mi_iprintf(fmt, ap, (pfi_t)mi_sprintf_putc, (char *)&buf);
	mi_sprintf_putc((char *)&buf, '\0');
	va_end(ap);
	panic(lbuf);
	/*NOTREACHED*/
	LEAVE_FUNC(mi_panic, 0);
}

int
mi_set_sth_wroff (q, size)
	queue_t	* q;
	int	size;
{
reg	mblk_t	* mp;
	STROPTP stropt;

	ENTER_FUNC(mi_set_sth_wroff, q, size, 0, 0, 0, 0);
	if (!(mp = allocb(sizeof(*stropt), BPRI_LO))) {
		LEAVE_FUNC(mi_set_sth_wroff, 0);
		return 0;
	}
	mp->b_datap->db_type = M_SETOPTS;
	mp->b_wptr += sizeof(*stropt);
	stropt = (STROPTP)mp->b_rptr;
	stropt->so_flags = SO_WROFF;
	stropt->so_wroff = size;
	putnext(q, mp);
	LEAVE_FUNC(mi_set_sth_wroff, 1);
	return 1;
}

int
mi_sprintf (
	char	* buf,
	char	* fmt,
	...)
{
	va_list	ap;
	int	count = -1;
	ENTER_FUNC(mi_sprintf, buf, fmt, 0, 0, 0, 0);
	va_start(ap, fmt);
	if (buf) {
		count = mi_iprintf(fmt, ap, (pfi_t)mi_sprintf_putc, (char *)&buf);
		if (count != -1)
			mi_sprintf_putc((char *)&buf, '\0');
	}
	va_end(ap);
	LEAVE_FUNC(mi_sprintf, count);
	return count;
}

/* Used to count without writing data */
int
mi_sprintf_noop (cookie, ch)
	char	* cookie;
	int	ch;
{
	char	** cpp = (char **)cookie;

	ENTER_FUNC(mi_sprintf_noop, cookie, ch, 0, 0, 0, 0);
	(*cpp)++;
	LEAVE_FUNC(mi_sprintf_noop, 1);
	return 1;
	
}

int
mi_sprintf_putc (
	char	* cookie,
	char	ch)
{
	char	** cpp = (char **)cookie;

	ENTER_FUNC(mi_sprintf_putc, cookie, ch, 0, 0, 0, 0);
	**cpp = (char)ch;
	(*cpp)++;
	LEAVE_FUNC(mi_sprintf_putc, 1);
	return 1;
}

int
mi_strlog (
	queue_t	* q,
	...)
{
	va_list	ap;
	char	buf[200];
	char	* alloc_buf = buf;
	int	count = -1;
	char	* cp;
	ushort	flags;
	char	* fmt;
	char	level;
	short	mid;
	MI_OP	mi_op;
	int	ret;
	short	sid;

	ENTER_FUNC(mi_strlog, q, 0, 0, 0, 0, 0);
	va_start(ap, q);
	sid = 0;
	mid = 0;
	if (q) {
		if ((mi_op = (MI_OP)q->q_ptr)
		&&  mi_op[-1].mi_o_pattern == 0x03020100L)
			sid = mi_op[-1].mi_o_dev;
		mid = q->q_qinfo->qi_minfo->mi_idnum;
	}
	level = (char)va_arg(ap, int);
	flags = (ushort)va_arg(ap, int);
	fmt = va_arg(ap, char *);

	/* Find out how many bytes we need and allocate if necesary */
	cp = buf;
	count = mi_iprintf(fmt, ap, (pfi_t)mi_sprintf_noop, (char *)&cp);
	if (count > sizeof(buf)
	&& !(alloc_buf = he_alloc(count + 2, BPRI_MED))) {
		va_end(ap);
		LEAVE_FUNC(mi_strlog, -1);
		return -1;
	}

	cp = alloc_buf;
	count = mi_iprintf(fmt, ap, (pfi_t)mi_sprintf_putc, (char *)&cp);
	if (count != -1)
		mi_sprintf_putc((char *)&cp, '\0');
	else
		alloc_buf[0] = '\0';
	va_end(ap);
	ret = strlog(mid, sid, level, flags, alloc_buf);
	if (alloc_buf != buf)
		he_free(alloc_buf);
	LEAVE_FUNC(mi_strlog, ret);
	return ret;
}

long
mi_strtol (str, ptr, base)
	char	* str;
	char	** ptr;
	int	base;
{
reg	char	* cp;
	long	digits, value;
	int	is_negative;

	cp = str;
	while (*cp == ' '  ||  *cp == '\t'  ||  *cp == '\n')
		cp++;
	if (is_negative = (*cp == '-'))
		cp++;
	if (base == 0) {
		base = 10;
		if (*cp == '0') {
			base = 8;
			cp++;
			if (*cp == 'x'  ||  *cp == 'X') {
				base = 16;
				cp++;
			}
		}
	}
	value = 0;
	for (; *cp; cp++) {
		if (*cp >= '0'  &&  *cp <= '9')
			digits = *cp - '0';
		else if (*cp >= 'a'  &&  *cp <= 'f')
			digits = *cp - 'a' + 10;
		else if (*cp >= 'A'  &&  *cp <= 'F')
			digits = *cp - 'A' + 10;
		else
			break;
		if (digits >= base)
			break;
		value = (value * base) + digits;
	}
	if (ptr)
		*ptr = cp;
	if (is_negative)
		value = -value;
	return value;
}

void
mi_timer (q, mp, tim)
	queue_t	* q;
	mblk_t	* mp;
	long	tim;
{
	MTBP	mtb;

	ENTER_FUNC(mi_timer, q, mp, tim, 0, 0, 0);
	if (!q
	||  !mp
	||  (mp->b_rptr - mp->b_datap->db_base) != sizeof(MTB)) {
		LEAVE_FUNC(mi_timer, -1);
		return;
	}
	if (tim >= 0)
		tim = MS_TO_TICKS(tim);
	mtb = (MTBP)mp->b_datap->db_base;
	mtb->mtb_q = q;
	mtb->mtb_mp = mp;
	if ( tim != -2 )
		(void)mi_itimer(&mtb->mtb_tb, tim, mi_timer_fire);
	LEAVE_FUNC(mi_timer, 0);
}

mblk_t *
mi_timer_alloc (size)
	uint	size;
{
reg	mblk_t	* mp;
reg	MTBP	mtb;

        ENTER_FUNC(mi_timer_alloc, size, 0, 0, 0, 0, 0);

	if (mp = allocb(size + sizeof(MTB), BPRI_HI)) {
		mp->b_datap->db_type = M_PCSIG;
		mp->b_rptr += sizeof(MTB);
		mp->b_wptr = mp->b_rptr + size;
		mtb = (MTBP)mp->b_datap->db_base;
		if (mi_itimer_init(&mtb->mtb_tb)) {
			mtb->mtb_mp = mp;
			mtb->mtb_q = nil(queue_t *);
			LEAVE_FUNC(mi_timer_alloc, mp);
			return mp;
		}
		freeb(mp);
	}
        LEAVE_FUNC(mi_timer_alloc, nilp(mblk_t));
	return nilp(mblk_t);
}

int
mi_timer_fire (tb)
	TBP	tb;
{
	mblk_t *	mp = ((MTBP)tb)->mtb_mp;
	queue_t	* q = ((MTBP)tb)->mtb_q;
reg	SQP	sq = &mp->b_sq;
	

	ENTER_FUNC(mi_timer_fire, tb, 0, 0, 0, 0, 0);

	sq->sq_entry = (sq_entry_t)putq_owned;
	sq->sq_queue = q;
	sq->sq_arg0 = q;
	sq->sq_arg1 = mp;
	csq_lateral(&q->q_sqh, sq, 0);

	LEAVE_FUNC(mi_timer_fire, 0);
	return 0;
}

void
mi_timer_free (mp)
	mblk_t	* mp;
{
reg	TBP	tb;

        ENTER_FUNC(mi_timer_free, mp, 0, 0, 0, 0, 0);

	if (!mp
	|| (mp->b_rptr - mp->b_datap->db_base) != sizeof(MTB)) {
		LEAVE_FUNC(mi_timer_free, -1);
		return;
	}
	tb = (TBP)mp->b_datap->db_base;
	if ( mi_itimer(tb, -1L, nil(pfi_t)) == TB_POSTED )
		tb->tb_state = TB_TO_BE_FREED;
	else
		freemsg(mp);
        LEAVE_FUNC(mi_timer_free, 0);
}

int
mi_timer_valid (mp)
	mblk_t	* mp;
{
reg	TBP	tb;
	long	ticks;

        ENTER_FUNC(mi_timer_valid, mp, 0, 0, 0, 0, 0);

	if (!mp  ||  (mp->b_rptr - mp->b_datap->db_base) != sizeof(MTB)) {
		LEAVE_FUNC(mi_timer_valid, 0);
		return 0;
	}
	tb = (TBP)mp->b_datap->db_base;
	if ( tb->tb_state == TB_TO_BE_FREED ) {
		freemsg(mp);
		LEAVE_FUNC(mi_timer_valid, 0);
		return 0;
	}
	if (tb->tb_state != TB_POSTED) {
		return 0;
	}
	tb->tb_state = TB_IDLE;
	ticks = tb->tb_ticks_remaining;
	if (ticks > 0) {
		(void)mi_itimer(tb, ticks, mi_timer_fire);
		LEAVE_FUNC(mi_timer_valid, 0);
		return 0;
	}
        LEAVE_FUNC(mi_timer_valid, 1);
	return 1;
}

int
mi_nd_comm (q, name, len, id, cmd)
	queue_t	* q;
	char	* name;
	int	len;
	uint	id;
	int	cmd;
{
	mblk_t	* mp, * mp1;
	struct iocblk	* iocp;

	ENTER_FUNC(mi_nd_comm, q, name, len, id, cmd, 0);

	if (mp1 = allocb(len + 128, BPRI_MED)) {
		bcopy(name, (char *)mp1->b_rptr, len);
		mp1->b_wptr += len;
		if (mp = allocb(sizeof(struct iocblk), BPRI_MED)) {
			mp->b_datap->db_type = M_IOCTL;
			mp->b_wptr += sizeof(struct iocblk);
			iocp = (struct iocblk *)mp->b_rptr;
			iocp->ioc_cmd = cmd;
			iocp->ioc_cr = nilp(cred_t);	/* Sure hope nobody minds */
			iocp->ioc_id = id;
			iocp->ioc_count = len;
			iocp->ioc_error = 0;
			iocp->ioc_rval = 0;
			mp->b_cont = mp1;
			putnext(q, mp);
			LEAVE_FUNC(mi_nd_comm, 1);
			return 1;
		}
		freeb(mp1);
	}
	LEAVE_FUNC(mi_nd_comm, 0);
	return 0;
}

int
mi_nd_get (q, name, len, id)
	queue_t	* q;
	char	* name;
	int	len;
	uint	id;
{
	ENTER_FUNC(mi_nd_get, q, name, len, id, 0, 0);

	return mi_nd_comm(q, name, len, id, ND_GET);

	LEAVE_FUNC(mi_nd_get, 0);
}

int
mi_nd_set (q, name, len, id)
	queue_t	* q;
	char	* name;
	int	len;
	uint	id;
{
	ENTER_FUNC(mi_nd_set, q, name, len, id, 0, 0);
	return mi_nd_comm(q, name, len, id, ND_SET);
	LEAVE_FUNC(mi_nd_set, 0);
}

mblk_t *
mi_reallocb (mp, new_size)
	mblk_t	* mp;
	int	new_size;
{
	mblk_t	* mp1;
	int	our_size;

	ENTER_FUNC(mi_reallocb, mp, new_size, 0, 0, 0, 0);
	if ((mp->b_datap->db_lim - mp->b_rptr) >= new_size) {
		LEAVE_FUNC(mi_reallocb, mp);
		return mp;
	}
	our_size = mp->b_wptr - mp->b_rptr;
	if ((mp->b_datap->db_lim - mp->b_datap->db_base) >= new_size) {
		bcopy((char *)mp->b_rptr, (char *)mp->b_datap->db_base, our_size);
		mp->b_rptr = mp->b_datap->db_base;
		mp->b_wptr = mp->b_rptr + our_size;
		LEAVE_FUNC(mi_reallocb, mp);
		return mp;
	}
	if (mp1 = allocb(new_size, BPRI_MED)) {
		mp1->b_wptr = mp1->b_rptr + our_size;
		mp1->b_datap->db_type = mp->b_datap->db_type;
		mp1->b_cont = mp->b_cont;
		bcopy((char *)mp->b_rptr, (char *)mp1->b_rptr, our_size);
		freeb(mp);
	}
	LEAVE_FUNC(mi_reallocb, mp1);
	return mp1;
}

/* Allocate and return a TPI ack packet, filling in the primitive type
** and bumping 'mp->b_wptr' down by the size indicated.  If 'mp' is non-nil
** blow it off to make room first.
*/
mblk_t *
mi_tpi_ack_alloc (mp, size, type)
reg	mblk_t	* mp;
	uint	size;
	uint	type;
{

	ENTER_FUNC(mi_tpi_ack_alloc, mp, size, type, 0, 0, 0);

	if (mp)
		freemsg(mp);
	if (mp = allocb(size, BPRI_HI)) {
		mp->b_datap->db_type = M_PCPROTO;
		mp->b_wptr += size;
		((TPRIMP)mp->b_rptr)->type = type;
	}

	LEAVE_FUNC(mi_tpi_ack_alloc, mp);
	return mp;
}

void
mi_tpi_addr_and_opt (mp, addr, addr_length, opt, opt_length)
	mblk_t	* mp;
	char	* addr;
	int	addr_length;
	char	* opt;
	int	opt_length;
{
	struct T_unitdata_ind	* tudi;

	ENTER_FUNC(mi_tpi_addr_and_opt, mp, addr, addr_length, opt, opt_length, 0); 
	tudi = (struct T_unitdata_ind *)mp->b_rptr;
	tudi->SRC_offset = mp->b_wptr - mp->b_rptr;
	tudi->SRC_length = addr_length;
	if (addr_length > 0) {
		bcopy(addr, (char *)mp->b_wptr, addr_length);
		mp->b_wptr += addr_length;
	}
	tudi->OPT_offset = mp->b_wptr - mp->b_rptr;
	tudi->OPT_length = opt_length;
	if (opt_length > 0) {
		bcopy(opt, (char *)mp->b_wptr, opt_length);
		mp->b_wptr += opt_length;
	}
	LEAVE_FUNC(mi_tpi_addr_and_opt, 0);
}

mblk_t *
mi_tpi_conn_con (trailer_mp, src, src_length, opt, opt_length)
	mblk_t	* trailer_mp;
	char	* src;
	int	src_length;
	char	* opt;
	int	opt_length;
{
	int	len;
reg	mblk_t	* mp;

	ENTER_FUNC(mi_tpi_conn_con, trailer_mp, src, src_length, opt, opt_length, 0);
	len = sizeof(struct T_conn_con) + src_length + opt_length;
	if (mp = mi_tpi_trailer_alloc(trailer_mp, len, T_CONN_CON)) {
		mp->b_wptr = &mp->b_rptr[sizeof(struct T_conn_con)];
		mi_tpi_addr_and_opt(mp, src, src_length, opt, opt_length);
	}
	LEAVE_FUNC(mi_tpi_conn_con, mp);
	return mp;
}

mblk_t *
mi_tpi_conn_ind (trailer_mp, src, src_length, opt, opt_length, seqnum)
	mblk_t	* trailer_mp;
	char	* src;
	int	src_length;
	char	* opt;
	int	opt_length;
	int	seqnum;
{
	int	len;
reg	mblk_t	* mp;

	ENTER_FUNC(mi_tpi_conn_ind, trailer_mp, src, src_length, opt,opt_length, seqnum);
	
	len = sizeof(struct T_conn_ind) + src_length + opt_length;
	if (mp = mi_tpi_trailer_alloc(trailer_mp, len, T_CONN_IND)) {
		mp->b_wptr = &mp->b_rptr[sizeof(struct T_conn_ind)];
		mi_tpi_addr_and_opt(mp, src, src_length, opt, opt_length);
		((struct T_conn_ind *)mp->b_rptr)->SEQ_number = seqnum;
		mp->b_datap->db_type = M_PROTO;
	}
	LEAVE_FUNC(mi_tpi_conn_ind, mp);
	return mp;
}

mblk_t *
mi_tpi_conn_req (trailer_mp, dest, dest_length, opt, opt_length)
	mblk_t	* trailer_mp;
	char	* dest;
	int	dest_length;
	char	* opt;
	int	opt_length;
{
	int	len;
reg	mblk_t	* mp;

	ENTER_FUNC(mi_tpi_conn_req, trailer_mp, dest, dest_length, opt, opt_length, 0);
	len = sizeof(struct T_conn_req) + dest_length + opt_length;
	if (mp = mi_tpi_trailer_alloc(trailer_mp, len, T_CONN_REQ)) {
		mp->b_wptr = &mp->b_rptr[sizeof(struct T_conn_req)];
		mi_tpi_addr_and_opt(mp, dest, dest_length, opt, opt_length);
		mp->b_datap->db_type = M_PROTO;
	}
	LEAVE_FUNC(mi_tpi_conn_req, mp);
	return mp;
}

mblk_t *
mi_tpi_data_ind (trailer_mp, flags, type)
	mblk_t	* trailer_mp;
	int	flags;
	int	type;
{
	mblk_t	* mp;

	ENTER_FUNC(mi_tpi_data_ind, trailer_mp, flags, type, 0, 0, 0);
	if (mp = mi_tpi_data_req(trailer_mp, flags, type))
		((struct T_data_ind *)mp->b_rptr)->PRIM_type = T_DATA_IND;
	LEAVE_FUNC(mi_tpi_data_ind, mp);
	return mp;
}

mblk_t *
mi_tpi_data_req (trailer_mp, flags, type)
	mblk_t	* trailer_mp;
	int	flags;
	int	type;
{
	mblk_t	* mp;
reg	struct T_data_req	* tdr;

	ENTER_FUNC(mi_tpi_data_req, trailer_mp, flags, type, 0, 0, 0);
	if (mp = mi_tpi_trailer_alloc(trailer_mp, sizeof(struct T_data_req), T_DATA_REQ)) { 
		tdr = (struct T_data_req *)mp->b_rptr;
		tdr->MORE_flag = flags;
		if (type)
			SET_TYPE_IN_FLAGS(tdr->MORE_flag, type);
		if (flags & T_MORE) {
			if (tdr->MORE_flag < 0)
				tdr->MORE_flag &= ~LONG_SIGN_BIT;
			if (tdr->MORE_flag == 0) 
				tdr->MORE_flag++;
		} else if (tdr->MORE_flag > 0)
			tdr->MORE_flag |= LONG_SIGN_BIT;
	}
	LEAVE_FUNC(mi_tpi_data_req, mp);
	return mp;
}

mblk_t *
mi_tpi_discon_ind (trailer_mp, reason, seqnum)
	mblk_t	* trailer_mp;
	int	reason;
	int	seqnum;
{
	mblk_t	* mp;
	struct T_discon_ind	* tdi;

	ENTER_FUNC(mi_tpi_discon_ind, trailer_mp, reason, seqnum, 0, 0, 0);
	if (mp = mi_tpi_trailer_alloc(trailer_mp, sizeof(struct T_discon_ind),T_DISCON_IND)) {
		tdi = (struct T_discon_ind *)mp->b_rptr;
		tdi->DISCON_reason = reason;
		tdi->SEQ_number = seqnum;
	}
	LEAVE_FUNC(mi_tpi_discon_ind, mp);
	return mp;
}

mblk_t *
mi_tpi_discon_req (trailer_mp, seqnum)
	mblk_t	* trailer_mp;
	int	seqnum;
{
	mblk_t	* mp;
	struct T_discon_req	* tdr;

	ENTER_FUNC(mi_tpi_discon_req, trailer_mp, seqnum,  0, 0, 0, 0);
	if (mp = mi_tpi_trailer_alloc(trailer_mp, sizeof(struct T_discon_req),T_DISCON_REQ)) {
		tdr = (struct T_discon_req *)mp->b_rptr;
		tdr->SEQ_number = seqnum;
	}
	LEAVE_FUNC(mi_tpi_discon_req, mp);
	return mp;
}

/* Allocate and fill in a TPI err ack packet using the 'mp' passed in
** for the 'error_prim' context as well as sacrifice.
*/
mblk_t *
mi_tpi_err_ack_alloc (mp, tlierr, unixerr)
reg	mblk_t	* mp;
	int	tlierr;
	int	unixerr;
{
	struct T_error_ack	* teackp;
	long	error_prim;

	ENTER_FUNC(mi_tpi_err_ack_alloc, mp, tlierr, unixerr, 0, 0, 0); 
	if (!mp) {
		LEAVE_FUNC(mi_tpi_err_ack_alloc, nilp(mblk_t));
		return nilp(mblk_t);
	}
	error_prim = ((TPRIMP)mp->b_rptr)->type;
	if (mp = mi_tpi_ack_alloc(mp, sizeof(struct T_error_ack),T_ERROR_ACK)){
		teackp = (struct T_error_ack *)mp->b_rptr;
		teackp->ERROR_prim = error_prim;
		teackp->TLI_error = tlierr;
		teackp->UNIX_error = unixerr;
	}
	LEAVE_FUNC(mi_tpi_err_ack_alloc, mp);
	return mp;
}

mblk_t *
mi_tpi_exdata_ind (trailer_mp, flags, type)
	mblk_t	* trailer_mp;
	int	flags;
	int	type;
{
	mblk_t	* mp;

	ENTER_FUNC(mi_tpi_exdata_ind, trailer_mp, flags, type, 0, 0, 0);
	if (mp = mi_tpi_data_req(trailer_mp, flags, type))
		((struct T_exdata_ind *)mp->b_rptr)->PRIM_type = T_EXDATA_IND;
	LEAVE_FUNC(mi_tpi_exdata_ind, mp);
	return mp;
}

mblk_t *
mi_tpi_exdata_req (trailer_mp, flags, type)
	mblk_t	* trailer_mp;
	int	flags;
	int	type;
{
	mblk_t	* mp;

	ENTER_FUNC(mi_tpi_exdata_req, trailer_mp, flags, type, 0, 0, 0);
	if (mp = mi_tpi_data_req(trailer_mp, flags, type))
		((struct T_exdata_ind *)mp->b_rptr)->PRIM_type = T_EXDATA_REQ;
	LEAVE_FUNC(mi_tpi_exdata_req, mp);
	return mp;
}

mblk_t *
mi_tpi_info_req ()
{
reg	mblk_t	* mp;

        ENTER_FUNC(mi_tpi_info_req, 0, 0, 0, 0, 0, 0);
	if (mp = allocb(sizeof(struct T_info_req), BPRI_MED)) {
		mp->b_datap->db_type = M_PROTO;
		((TPRIMP)mp->b_rptr)->type = T_INFO_REQ;
		mp->b_wptr += sizeof(struct T_info_req);
	}
        LEAVE_FUNC(mi_tpi_info_req, mp);
	return mp;
}

mblk_t *
mi_tpi_ioctl_info_req (id)
	uint	id;
{
reg	mblk_t	* mp;
	struct iocblk	* iocp;

        ENTER_FUNC(mi_tpi_ioctl_info_req, id, 0, 0, 0, 0, 0);

	if (mp = allocb(sizeof(struct iocblk), BPRI_MED)) {
		if (mp->b_cont = allocb(sizeof(struct T_info_req), BPRI_MED)) {
			((struct T_info_req *)mp->b_cont->b_rptr)->PRIM_type = T_INFO_REQ;
			mp->b_cont->b_wptr += sizeof(struct T_info_req);
			mp->b_datap->db_type = M_IOCTL;
			iocp = (struct iocblk *)mp->b_rptr;
			mp->b_wptr += sizeof(struct iocblk);
			iocp->ioc_cmd = TI_GETINFO;
			iocp->ioc_cr = nilp(cred_t);	/* Sure hope nobody minds */
			iocp->ioc_id = id;
			iocp->ioc_count = sizeof(struct T_info_ack);
			iocp->ioc_error = 0;
			iocp->ioc_rval = 0;
		} else
			freeb(mp);
	}
        LEAVE_FUNC(mi_tpi_ioctl_info_req, mp);
	return mp;
}

mblk_t *
mi_tpi_ok_ack_alloc (mp)
	mblk_t	* mp;
{
	long	correct_prim;

	ENTER_FUNC(mi_tpi_ok_ack_alloc, mp, 0, 0, 0, 0, 0);

	if (!mp) {
		LEAVE_FUNC(mi_tpi_ok_ack_alloc, nilp(mblk_t));
		return nilp(mblk_t);
	}
	correct_prim = ((TPRIMP)mp->b_rptr)->type;
	if (mp = mi_tpi_ack_alloc(mp, sizeof(struct T_ok_ack), T_OK_ACK))
		((struct T_ok_ack *)mp->b_rptr)->CORRECT_prim = correct_prim;
	LEAVE_FUNC(mi_tpi_ok_ack_alloc, mp);
	return mp;
}

mblk_t *
mi_tpi_ordrel_ind ()
{
reg	mblk_t	* mp;

        ENTER_FUNC(mi_tpi_ordrel_ind, 0, 0, 0, 0, 0, 0);

	if (mp = allocb(sizeof(struct T_ordrel_ind), BPRI_HI)) {
		mp->b_datap->db_type = M_PROTO;
		((struct T_ordrel_ind *)mp->b_rptr)->PRIM_type = T_ORDREL_IND;
		mp->b_wptr += sizeof(struct T_ordrel_ind);
	}

        LEAVE_FUNC(mi_tpi_ordrel_ind, mp);
	return mp;
}

mblk_t *
mi_tpi_ordrel_req ()
{
reg	mblk_t	* mp;

        ENTER_FUNC(mi_tpi_ordrel_req, 0, 0, 0, 0, 0, 0);

	if (mp = allocb(sizeof(struct T_ordrel_req), BPRI_HI)) {
		mp->b_datap->db_type = M_PROTO;
		((struct T_ordrel_req *)mp->b_rptr)->PRIM_type = T_ORDREL_REQ;
		mp->b_wptr += sizeof(struct T_ordrel_req);
	}

        LEAVE_FUNC(mi_tpi_ordrel_req, mp);
	return mp;
}

mblk_t *
mi_tpi_trailer_alloc (trailer_mp, size, type)
	mblk_t	* trailer_mp;
	int	size;
	int	type;
{
reg	mblk_t	* mp;

        ENTER_FUNC(mi_tpi_trailer_alloc,trailer_mp, size, type, 0, 0, 0);  
	if (mp = allocb(size, BPRI_MED)) {
		mp->b_cont = trailer_mp;
		mp->b_datap->db_type = M_PROTO;
		((union T_primitives *)mp->b_rptr)->type = type;
		mp->b_wptr += size;
	}

        LEAVE_FUNC(mi_tpi_trailer_alloc, mp);
	return mp;
}

mblk_t *
mi_tpi_uderror_ind (dest, dest_length, opt, opt_length, error)
	char	* dest;
	int	dest_length;
	char	* opt;
	int	opt_length;
	int	error;
{
	int	len;
	mblk_t	* mp;
	struct T_uderror_ind	* tudei;

	ENTER_FUNC(mi_tpi_uderror_ind, dest, dest_length, opt, opt_length, error, 0); 
	len = sizeof(struct T_uderror_ind) + dest_length + opt_length;
	if (mp = allocb(len, BPRI_HI)) {
		mp->b_datap->db_type = M_PROTO;
		tudei = (struct T_uderror_ind *)mp->b_rptr;
		tudei->PRIM_type = T_UDERROR_IND;
		tudei->ERROR_type = error;
		mp->b_wptr = &mp->b_rptr[sizeof(struct T_uderror_ind)];
		mi_tpi_addr_and_opt(mp, dest, dest_length, opt, opt_length);
	}

        LEAVE_FUNC(mi_tpi_uderror_ind, mp);
	return mp;
}

mblk_t *
mi_tpi_unitdata_ind (trailer_mp, src, src_length, opt, opt_length)
	mblk_t	* trailer_mp;
	char	* src;
	int	src_length;
	char	* opt;
	int	opt_length;
{
	int	len;
reg	mblk_t	* mp;

	ENTER_FUNC(mi_tpi_unitdata_ind, trailer_mp, src, src_length, opt, opt_length, 0);
	len = sizeof(struct T_unitdata_ind) + src_length + opt_length;
	if (mp = mi_tpi_trailer_alloc(trailer_mp, len, T_UNITDATA_IND)) {
		mp->b_wptr = &mp->b_rptr[sizeof(struct T_unitdata_ind)];
		mi_tpi_addr_and_opt(mp, src, src_length, opt, opt_length);
	}

	LEAVE_FUNC(mi_tpi_unitdata_ind, mp);
	return mp;
}

mblk_t *
mi_tpi_unitdata_req (trailer_mp, dst, dst_length, opt, opt_length)
	mblk_t	* trailer_mp;
	char	* dst;
	int	dst_length;
	char	* opt;
	int	opt_length;
{
	int	len;
reg	mblk_t	* mp;

	ENTER_FUNC(mi_tpi_unitdata_req,  trailer_mp, dst, dst_length, opt, opt_length, 0); 
	len = sizeof(struct T_unitdata_req) + dst_length + opt_length;
	if (mp = mi_tpi_trailer_alloc(trailer_mp, len, T_UNITDATA_REQ)) {
		mp->b_wptr = &mp->b_rptr[sizeof(struct T_unitdata_req)];
		mi_tpi_addr_and_opt(mp, dst, dst_length, opt, opt_length);
	}

	LEAVE_FUNC(mi_tpi_unitdata_req, mp);
	return mp;
}

caddr_t
mi_zalloc (size)
	uint	size;
{
	caddr_t	ptr;

	ENTER_FUNC(mi_zalloc, size, 0, 0, 0, 0, 0);
	if (ptr = he_alloc(size, BPRI_LO))
		bzero(ptr, size);
	LEAVE_FUNC(mi_zalloc, ptr);
	return ptr;
}

int
mi_strcmp (cp1, cp2)
reg     char    * cp1;
reg     char    * cp2;
{

	ENTER_FUNC(mi_strcmp, cp1, cp2, 0, 0, 0, 0);
        while (*cp1++ == *cp2++) {
                if (!cp2[-1]) {
			LEAVE_FUNC(mi_strcmp, 0);
                        return 0;
		}
        }

	LEAVE_FUNC(mi_strcmp, ((uint)cp2[-1]  & 0xFF) - ((uint)cp1[-1] & 0xFF));
        return ((uint)cp2[-1]  & 0xFF) - ((uint)cp1[-1] & 0xFF);
}
