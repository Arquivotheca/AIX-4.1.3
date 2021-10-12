static char sccsid[] = "@(#)16        1.15  src/bos/kernext/pse/str_funnel.c, sysxpse, bos41J, 9523B_all 6/6/95 16:07:15";
/*
 * COMPONENT_NAME: SYSXPSE Streams Framework
 * 
 * FUNCTIONS:
 *		funnelq_run
 *		funnelq_init
 *		funnelq_term
 *		funnel_open_V3
 *		funnel_open_V4
 *		funnel_close_V3
 *		funnel_close_V4
 *		funnel_putp
 *		funnel_sq_wrapper
 *		funnel_runq_remove
 *		init_funnel
 *		funnel_init
 *		funnel_term
 * 
 * ORIGINS: 83 
 * 
 */

/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#include <pse/str_stream.h>
#include <pse/str_config.h>
#include <pse/str_funnel.h>
#include <net/netisr.h>

#define THREAD_SHOULD_TERMINATE   4097

SQH	funnel_runq;
int	funnel_active = 0;
int	funnel_count  = 0;

/*
 * NAME: funnelq_run
 *
 * FUNCTION:
 *       funnel queue scheduler.
 *
 * NOTES:
 *	This routine runs on MP_MASTER processor.
 *
 * RETURN VALUE:
 *       ignored.
 */

void 
funnelq_run()
{
	SQP     sq;
	SQHP    sqh = &funnel_runq;
	extern void sq_wrapper();
	int     wasfunneled = 0;
	DISABLE_LOCK_DECL

	DISABLE_LOCK(&sqh->sqh_lock);
	funnel_active = 1;
	for (;;) {
		void *arg1;
		queue_t * q;
		void (*pfunc)();

		sq = sqh->sqh_next;
		if (sq == (SQP) sqh)
			break;
		remque(sq);
		/*
		 * Reset sq_sq_entry for next qenable
		 */
		pfunc = sq->sq_entry;
		if (pfunc == sq_wrapper)
			sq->sq_entry = (sq_entry_t) funnel_sq_wrapper;
		DISABLE_UNLOCK(&sqh->sqh_lock);
		if ((q = sq->sq_queue) && (q->q_flag & QUSE)) {
			DISABLE_LOCK(&q->q_qlock);
			arg1 = sq->sq_arg1;
			sq->sq_arg1 = 0;
			DISABLE_UNLOCK(&q->q_qlock);
                        csq_acquire(&q->q_sqh, sq);
			wasfunneled = switch_cpu(MP_MASTER, SET_PROCESSOR_ID);
			(void)(*pfunc)(sq->sq_arg0, arg1);
 			csq_release(&q->q_sqh);
			if (!wasfunneled)
				(void) switch_cpu(PROCESSOR_CLASS_ANY, RESET_PROCESSOR_ID);
		} else {
			sq->sq_flags &= ~(SQ_INUSE|SQ_QUEUED);
		}
		DISABLE_LOCK(&sqh->sqh_lock);
	}
	funnel_active = 0;
	DISABLE_UNLOCK(&sqh->sqh_lock);
}

void funnelq_init()
{

	sqh_init(&funnel_runq);
	if (netisr_add(NETISR_STRFUNNEL, funnelq_run,
				(struct ifqueue *)0, (struct domain *)0))
		panic("funnelq_init");
}

void funnelq_term()
{

	if (funnel_runq.sqh_next != (SQP)&funnel_runq)
		panic("funnelq_term");
	if (netisr_del(NETISR_STRFUNNEL))
		panic("funnelq_term");
}

/*
 * NAME: funnel_open_V3, funnel_open_V4
 *
 * FUNCTION:
 *        open routines for non MPSAFE drivers for SVR3 and SVR4
 *        open style.
 *
 * NOTES:
 *        These procedures have user context and can sleep.
 *
 * RETURN VALUE:
 *        These open routines return 0 for success, or the
 *        appropriate error number. 
 */

int
funnel_open_V3(q, devp, flags, sflag)
queue_t *q;
dev_t *devp;
int flags;
int sflag;
{
	int error;
	struct funnel_qinit *fqp = (struct funnel_qinit *)q->q_qinfo;
	int     wasfunneled = 0;
	
	wasfunneled = switch_cpu(MP_MASTER, SET_PROCESSOR_ID);
	error = (*fqp->fqi_qinit->qi_qopen) (q, devp, flags, sflag);
	if (!wasfunneled)
		(void) switch_cpu(MP_MASTER, RESET_PROCESSOR_ID);
	return error;
}

int
funnel_open_V4(q, devp, flags, sflag, credp)
queue_t *q;
dev_t *devp;
int flags;
int sflag;
cred_t *credp;
{
	int error;
	struct funnel_qinit *fqp = (struct funnel_qinit *)q->q_qinfo;
	int wasfunneled = 0;
	
	wasfunneled = switch_cpu(MP_MASTER, SET_PROCESSOR_ID);
	error = (*fqp->fqi_qinit->qi_qopen) (q, devp, flags, sflag, credp);
	if (!wasfunneled)
		(void) switch_cpu(MP_MASTER, RESET_PROCESSOR_ID);
	return error;
}

/*
 * NAME: funnel_close_V3, funnel_close_V4
 *
 * FUNCTION:
 *        close routines for non MPSAFE drivers for SVR3 and SVR4
 *        close style.
 *
 * NOTES:
 *        These procedures have user context and can sleep.
 *
 * RETURN VALUE:
 *        These open routines return 0 for success, or the
 *        appropriate error number. 
 */

int
funnel_close_V3(q)
queue_t *q;
{
	int error;
	struct funnel_qinit *fqp = (struct funnel_qinit *)q->q_qinfo;
	int wasfunneled = 0;

	wasfunneled = switch_cpu(MP_MASTER, SET_PROCESSOR_ID);
	error = (*fqp->fqi_qinit->qi_qclose)(q);
	if (!wasfunneled)
		(void) switch_cpu(MP_MASTER, RESET_PROCESSOR_ID);
	return error;
}

int
funnel_close_V4(q, credp)
queue_t *q;
cred_t *credp;
{
	int error;
	struct funnel_qinit *fqp = (struct funnel_qinit *)q->q_qinfo;
	int wasfunneled = 0;

	wasfunneled = switch_cpu(MP_MASTER, SET_PROCESSOR_ID);
	error = (*fqp->fqi_qinit->qi_qclose)(q, credp);
	if (!wasfunneled)
		(void) switch_cpu(MP_MASTER, RESET_PROCESSOR_ID);
	return error;
}

/*
 * NAME: funnel_putp
 *
 * FUNCTION:
 *       put routine for non MPSAFE drivers
 *
 * NOTES:
 *       put routine could not have user context and so may not
 *       call any function that sleeps.
 *
 * RETURN VALUE:
 *       ignored.
 */


int
funnel_putp(q, mp)
queue_t *q;
mblk_t  *mp;
{
	struct funnel_qinit *fqp = (struct funnel_qinit *)q->q_qinfo;
	SQP	sq = &(mp->b_sq);
	int     gotit;
	int     error = 0;
	int     wasfunneled = 0;
	DISABLE_LOCK_DECL

	_ssavpri = i_disable(INTBASE);
	if ((mycpu() == MP_MASTER) && (_ssavpri != INTBASE)) {
		return (*fqp->fqi_qinit->qi_putp)(q, mp);
	}
	if ((getpid() != -1) && (_ssavpri == INTBASE)) {
                wasfunneled = switch_cpu(MP_MASTER, SET_PROCESSOR_ID);
                error = (*fqp->fqi_qinit->qi_putp)(q, mp);
		if (!wasfunneled)
                	(void) switch_cpu(MP_MASTER, RESET_PROCESSOR_ID);
                return error;
        }

	DISABLE_LOCK(&q->q_qlock);
	if ( gotit = !(sq->sq_flags & SQ_INUSE)) {
		sq->sq_entry = (sq_entry_t)fqp->fqi_qinit->qi_putp;
		sq->sq_arg1  = mp;
		DISABLE_UNLOCK(&q->q_qlock);
		DISABLE_LOCK(&funnel_runq.sqh_lock);
		insque(sq, funnel_runq.sqh_prev);
		DISABLE_UNLOCK(&funnel_runq.sqh_lock);
		if (!funnel_active) {
			/* Wake up the funnel STREAMS thread */
			schednetisr(NETISR_STRFUNNEL);
		}
	} else {
		DISABLE_UNLOCK(&q->q_qlock);
	}
	return error;
}

/*
 * NAME: funnel_sq_wrapper
 *
 * FUNCTION:
 *       Wrapper function for the q's service procedure for non MPSAFE drivers
 *
 * RETURN VALUE:
 *       ignored.
 */


void
funnel_sq_wrapper(q)
queue_t *q;
{
	SQP	sq = q->q_runq_sq;
	DISABLE_LOCK_DECL

	DISABLE_LOCK(&q->q_qlock);
	if (q->q_flag & QUSE) {
		sq->sq_entry = sq_wrapper;
		DISABLE_UNLOCK(&q->q_qlock);
		DISABLE_LOCK(&funnel_runq.sqh_lock);
		insque(sq, funnel_runq.sqh_prev);
		DISABLE_UNLOCK(&funnel_runq.sqh_lock);
		if (!funnel_active) {
			/* Wake up the funnel STREAMS thread */
			schednetisr(NETISR_STRFUNNEL);
		}
	} else {
		q->q_runq_sq->sq_flags &= ~(SQ_INUSE|SQ_QUEUED);
		DISABLE_UNLOCK(&q->q_qlock);
	}
}

void
funnel_runq_remove (q)
	queue_t * q;
{
	SQP	sq = q->q_runq_sq;
	SQHP	sqh = &funnel_runq;
reg	SQHP	psqh;
	DISABLE_LOCK_DECL

	DISABLE_LOCK(&q->q_qlock);
	if (sq->sq_flags & SQ_INUSE) {
		int savpri = disable_lock(INTMAX, &sqh->sqh_lock);
		for (psqh = (SQHP) sqh->sqh_next; psqh != (SQHP) sqh; psqh = (SQHP) psqh->sqh_next) {
			if (psqh == (SQHP) sq) {
				remque(sq);
				sq->sq_flags &= ~(SQ_INUSE|SQ_QUEUED);
				break;
			}
		}
		unlock_enable(savpri, &sqh->sqh_lock);
	}
	DISABLE_UNLOCK(&q->q_qlock);
}

/*
 * NAME: init_funnel
 *
 * FUNCTION:
 *       routine for funnel qinit structure construction.
 *
 * RETURN VALUE:
 *       return 0 for success, or the appropriate error number.
 *     
 */

static int
init_funnel(fsqi, sqi, adm)
struct qinit **fsqi;
struct qinit *sqi;
struct streamadm *adm;
{
	struct funnel_qinit *qi;

	NET_MALLOC(qi, struct funnel_qinit *, sizeof *qi, M_STREAMS, M_WAITOK);
	if (!qi) return ENOMEM;
	bzero(qi, sizeof *qi);
	qi->fqi_qinit = sqi;
	if (adm->sa_flags & STR_SYSV4_OPEN) {
	    if ((sqi)->qi_qopen) qi->fqi_qopen = funnel_open_V4;
	    if ((sqi)->qi_qclose) qi->fqi_qclose = funnel_close_V4;
	}
	else {
	    if ((sqi)->qi_qopen) qi->fqi_qopen = funnel_open_V3;
	    if ((sqi)->qi_qclose) qi->fqi_qclose = funnel_close_V3;
	}
	if ((sqi)->qi_putp) qi->fqi_putp = funnel_putp;
	if ((sqi)->qi_srvp) qi->fqi_srvp = (void (*) ())(sqi)->qi_srvp;
	if ((sqi)->qi_qadmin) qi->fqi_qadmin = (sqi)->qi_qadmin;
	if ((sqi)->qi_minfo) qi->fqi_minfo = (sqi)->qi_minfo;
	if ((sqi)->qi_mstat) qi->fqi_mstat = (sqi)->qi_mstat;
	*fsqi = (struct qinit *)qi;
	return 0;
}

/*
 * NAME: funnel_init
 *
 * FUNCTION:
 *       initialization of the funnel qinit structure.
 *
 * RETURN VALUE:
 *       return 0 for success, or the appropriate error number.
 *     
 */

int
funnel_init(str, adm)
struct streamtab **str;
struct streamadm *adm;
{
	struct streamtab *fst;
	struct funnel_qinit *qi;
	int error;
	extern void bufcall_funnel_init();

	NET_MALLOC(fst, struct streamtab *, sizeof *fst, M_STREAMS, M_WAITOK);
	if (!fst) return ENOMEM;
	bzero(fst, sizeof *fst);
	if ((*str)->st_rdinit)
		if (error = init_funnel(&fst->st_rdinit, (*str)->st_rdinit, adm)) {
			NET_FREE(fst, M_STREAMS);
			return error;
		}
	if ((*str)->st_wrinit)
		if (error = init_funnel(&fst->st_wrinit, (*str)->st_wrinit, adm)) {
			if (fst->st_rdinit)
				NET_FREE(fst->st_rdinit, M_STREAMS);
			NET_FREE(fst, M_STREAMS);
			return error;
		}
	*str = fst;

	SIMPLE_LOCK(&funnel_runq.sqh_lock);
	if (!funnel_count) bufcall_funnel_init();
	funnel_count ++;
	SIMPLE_UNLOCK(&funnel_runq.sqh_lock);

	return 0;
}

/*
 * NAME: funnel_term
 *
 * RETURN VALUE:
 *       return 0 for success, or the appropriate error number.
 *     
 */

void
funnel_term(str)
struct streamtab *str;
{
	extern void bufcall_funnel_term();

	SIMPLE_LOCK(&funnel_runq.sqh_lock);
	if (!(--funnel_count)) bufcall_funnel_term();
	SIMPLE_UNLOCK(&funnel_runq.sqh_lock);

	if (str->st_rdinit) {
		NET_FREE(str->st_rdinit, M_STREAMS);
	}
	if (str->st_wrinit) {
		NET_FREE(str->st_wrinit, M_STREAMS);
	}
	NET_FREE(str, M_STREAMS);
}
