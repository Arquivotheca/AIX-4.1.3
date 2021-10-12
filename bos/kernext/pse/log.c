static char sccsid[] = "@(#)27        1.8.2.6  src/bos/kernext/pse/log.c, sysxpse, bos411, 9428A410j 4/22/94 08:19:48";
/*
 * COMPONENT_NAME: SYSXPSE - STREAMS framework
 * 
 * FUNCTIONS:      strlog_init
 *                 strlog_term
 *                 log_configure
 *                 strlog
 *                 log_close
 *                 log_deliver
 *                 log_open
 *                 log_should_deliver
 *                 log_wput
 *                 
 * 
 * ORIGINS: 63, 71, 83 
 * 
 */

/*
 * Copyright (c) 1990  Mentat Inc.
 */

/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */

/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#include <pse/str_system.h>
#include <sys/strconf.h>
#include <sys/stropts.h>
#include <sys/strstat.h>
#include <sys/strlog.h>
#include <sys/syslog.h>
#include <sys/lock_alloc.h>

#include <sys/time.h>
#include <pse/str_lock.h>
#include <pse/str_config.h>
#include <sys/stream.h>
#include <pse/mi.h>

#ifndef staticf
#define staticf static
#endif

typedef struct log_ctl  * LOGP;
typedef struct trace_ids * TRACEP;

typedef struct logi_s {
        int     logi_sid;
} LOGI, * LOGIP;

staticf	int		log_close(queue_t *, int, cred_t *);
staticf	int		log_deliver(queue_t *, mblk_t *, int);
static 	int		log_open(queue_t *, dev_t *, int, int, cred_t *);
staticf	queue_t	*	log_should_deliver(short, short, char, short);
staticf	int		log_wput(queue_t *, mblk_t *);

static	queue_t	*	log_error_q;
static	mblk_t *	log_trace_blks;
static	queue_t	*	log_trace_q;
static	int		trace_seq_num;
static	int		error_seq_num;
decl_simple_lock_data(static,log_lock)

static struct module_info minfo =  {
#define	MODULE_ID	44
	MODULE_ID, "LOG", 0, INFPSZ, 2048, 128
};

static struct qinit rinit = {
	NULL, NULL, log_open, log_close, NULL, &minfo
};

static struct qinit winit = {
	log_wput, NULL, NULL, NULL, NULL, &minfo
};

struct streamtab loginfo = { &rinit, &winit };

static  caddr_t     log_g_head;

void
strlog_init()
{
	lock_alloc((&log_lock), LOCK_ALLOC_PIN, PSE_LOG_LOCK, -1);
	simple_lock_init(&log_lock);
}

void
strlog_term()
{
	lock_free(&log_lock);
}

extern dev_t clonedev;

int
log_configure (op, indata, indatalen, outdata, outdatalen)
        uint         op;
        str_config_t *  indata;
        size_t          indatalen;
        str_config_t *  outdata;
        size_t          outdatalen;
{
        struct streamadm        sa;
        static dev_t            devno;
        int                     error = 0;

        sa.sa_flags             = STR_IS_DEVICE | STR_SYSV4_OPEN
				| STR_IS_MPSAFE;
        sa.sa_ttys              = 0;
        sa.sa_sync_level        = SQLVL_MODULE;
        sa.sa_sync_info         = 0;
        strcpy(sa.sa_name,      "slog");

        switch (op) {
        case CFG_INIT:
            strlog_init();

            if (error = strmod_add(indata->sc_devnum, &loginfo, &sa)) {
                return error;
            }
	    devno = indata->sc_devnum;
            break;
        case CFG_TERM:
            if (( error = strmod_del(devno, &loginfo, &sa))) return error;
	    strlog_term();
            break;
        default:
	    return EINVAL;
        }

	if (outdata != NULL && outdatalen == sizeof(str_config_t)) {
	    outdata->sc_devnum = makedev(major(clonedev), major(devno));
	    outdata->sc_sa_flags = sa.sa_flags;
	    strcpy(outdata->sc_sa_name, sa.sa_name);
	}

        return error;

}

/** Submit messages for logging */

int
strlog (
	short mid,
	short sid,
	char  level,
	unsigned short flags,
	char *fmt,
	...)
{
        va_list ap;
	queue_t	* q;
	mblk_t *  mp, *mp1;
	LOGP	lctlp;
	int     i, *ip;
	queue_t		*error_q, *trace_q;
	DISABLE_LOCK_DECL

	DISABLE_LOCK(&log_lock);
	/*
	 * Increment message numbers even if this one doesn't go anywhere,
	 * or we can't get memory.
	 */
	if (flags & SL_TRACE) {
		trace_q = log_trace_q;
		trace_seq_num++;
	}
	if (flags & SL_ERROR) {
		error_q = log_error_q;
		error_seq_num++;
	}

	/* No log or trace programs open, so quit early */
	if (!log_trace_q  &&  !log_error_q  &&  !(flags & SL_CONSOLE))
		goto out;
	/* Find out if we should deliver this message */
	if (!(q = log_should_deliver(mid, sid, level, (short)flags)))
		goto out;
	if (!(mp = allocb(sizeof(struct log_ctl), BPRI_MED)))
		goto out;
	/* NLOGARGS+1 for alignment overhead */
	if (!(mp->b_cont = allocb(LOGMSGSZ+(NLOGARGS+1)*WORDLEN, BPRI_MED))) {  
		freeb(mp);
out:
		DISABLE_UNLOCK(&log_lock);
		return(0);
	}
	if (fmt) {
		va_start(ap, fmt);
		strcpy(mp->b_cont->b_rptr, fmt);
		i = mp->b_cont->b_rptr + strlen(fmt);
		ip = (int *)(i + (WORDLEN - i % WORDLEN));
		for (i = 0; i < NLOGARGS; i++)
			ip[i] = va_arg(ap, int);
		mp->b_cont->b_wptr = (char*)ip + (WORDLEN*NLOGARGS);
		va_end(ap);
	}
	mp->b_datap->db_type = M_PROTO;
	mp->b_wptr += sizeof(struct log_ctl);
	lctlp = (LOGP)mp->b_rptr;
	lctlp->sid = sid;
	lctlp->mid = mid;
	lctlp->level = level;
	lctlp->flags = flags;
	lctlp->seq_no = (flags & SL_ERROR) ? error_seq_num : trace_seq_num;
	lctlp->ltime = lbolt;
	lctlp->ttime = time;
	DISABLE_UNLOCK(&log_lock);
	if (flags & SL_CONSOLE) {
		bsdlog(LOG_CONS | LOG_KERN, "%s\r\n", mp->b_cont->b_rptr);
		if (flags & ~SL_CONSOLE) goto end;
	}
	if ((flags & SL_ERROR)  &&  log_error_q) {
		if (q  &&  q == trace_q)
			mp1 = dupmsg(mp);
		 else
			mp1 = 0;
		putnext(error_q, mp);
		mp = mp1;
	}
	if (q  &&  q == trace_q  &&  mp) {
		putnext(q, mp);
		mp = 0;
	}
end:
	if (mp)
		freemsg(mp);
	return 0;
}

staticf int
log_close (q, flag, credp)
	queue_t	* q;
	int	flag;
	cred_t	* credp;
{
	DISABLE_LOCK_DECL

	DISABLE_LOCK(&log_lock);
	if (q == log_error_q)
		log_error_q = 0;
	if (q == log_trace_q) {
		log_trace_q = 0;
		if (log_trace_blks) {
			freemsg(log_trace_blks);
			log_trace_blks = 0;
		}
	}
	DISABLE_UNLOCK(&log_lock);
	return mi_close_comm((caddr_t *)&log_g_head, q);
}

staticf int
log_open (q, devp, flag, sflag, credp)
	queue_t	* q;
	dev_t	* devp;
	int	flag;
	int	sflag;
	cred_t	* credp;
{
	int	err;

	if (q->q_ptr)
		return ENXIO;
	err = mi_open_comm((caddr_t *)&log_g_head, sizeof(LOGI),
					q, devp, flag, sflag, credp);
	if (!err)
		((LOGIP)q->q_ptr)->logi_sid = minor(*devp);
	return err;
}

staticf queue_t *
log_should_deliver (
	short		mid,
	short		sid,
	char		level,
	short		flags)
{
	queue_t *	q = 0;
	mblk_t *	mp;
	TRACEP		tracep;

	if (flags & SL_TRACE) {
		for (mp = log_trace_blks; mp; mp = mp->b_cont) {
			for (tracep = (TRACEP)mp->b_rptr;
			     tracep < (TRACEP)mp->b_wptr;
			     tracep++) {
				if ((tracep->ti_mid == -1 ||
						tracep->ti_mid == mid)
				&& (tracep->ti_sid == -1 ||
					tracep->ti_sid == sid)
				&& (tracep->ti_level == -1 ||
						tracep->ti_level >= level)) {
					q = log_trace_q;
					goto out;
				}
			}
		}
	}
out:
	if (q == 0 && (flags & SL_ERROR))
		q = log_error_q;
	if (q == 0 && (flags & SL_CONSOLE))
		q = (queue_t *)-1;
	return q;
}

staticf int
log_wput (q, mp)
	queue_t	* 	q;
	mblk_t *	mp;
{
	struct iocblk * iocp;
	LOGP		logp;
	mblk_t *	mp1;
	int		flags;
	queue_t		*error_q,  *trace_q;
	DISABLE_LOCK_DECL

	switch (mp->b_datap->db_type) {
	case M_PCPROTO:
	case M_PROTO:
		if ((mp->b_wptr - mp->b_rptr) < sizeof(struct log_ctl)
		||  !mp->b_cont)
			break;
		logp = (LOGP)mp->b_rptr;
		logp->mid = MODULE_ID;
		logp->sid = ((LOGIP)q->q_ptr)->logi_sid;
		flags = logp->flags;
		DISABLE_LOCK(&log_lock);
		if (flags & SL_TRACE) {
			trace_q = log_trace_q;
			trace_seq_num++;
		}
		if (flags & SL_ERROR) {
			error_q = log_error_q;
			error_seq_num++;
		}
		q = log_should_deliver(logp->mid, logp->sid, logp->level, logp->flags);
		if (q) {
			mblk_t *	mp1;

			logp = (LOGP)mp->b_rptr;
			logp->ltime = lbolt;
			logp->ttime = time;
			logp->seq_no = (flags & SL_ERROR) ?
						error_seq_num : trace_seq_num;
			DISABLE_UNLOCK(&log_lock);
			if (flags & SL_CONSOLE) {
				bsdlog(LOG_CONS | LOG_KERN, "%s\r\n",
							mp->b_cont->b_rptr);
			if (flags & ~SL_CONSOLE) goto end;
			}
			if ((flags & SL_ERROR)  &&  error_q) {
				if (q  &&  q == trace_q)
					mp1 = dupmsg(mp);
				else
					mp1 = 0;
				putnext(error_q, mp);
				mp = mp1;
			}
			if (q  &&  q == trace_q  &&  mp) {
				putnext(q, mp);
				mp = 0;
			}
end:
			if (mp)
				freemsg(mp);
			return 0;
		}
		DISABLE_UNLOCK(&log_lock);
		break;
	case M_DATA:
		break;
	case M_IOCTL:
		iocp = (struct iocblk *)mp->b_rptr;
		mp->b_datap->db_type = M_IOCACK;
		iocp->ioc_error = ENXIO;
		DISABLE_LOCK(&log_lock);
		switch (iocp->ioc_cmd) {
		case I_TRCLOG:
			if (iocp->ioc_count < sizeof(struct trace_ids)
			||  (iocp->ioc_count % sizeof(struct trace_ids)) != 0
			||  !mp->b_cont)
				break;
			if (log_trace_q  &&  log_trace_q != RD(q))
				break;
			if (!log_trace_q)
				log_trace_q = RD(q);
			mp1 = mp->b_cont;
			mp->b_cont = 0;
			mp1->b_cont = log_trace_blks;
			log_trace_blks = mp1;
			iocp->ioc_error = 0;
			break;
		case I_ERRLOG:
			if (mp->b_cont) {
				freemsg(mp->b_cont);
				mp->b_cont = 0;
			}
			if (log_error_q  &&  log_error_q != RD(q))
				break;
			if (!log_error_q)
				log_error_q = RD(q);
			iocp->ioc_error = 0;
			break;
		default:
			mp->b_datap->db_type = M_IOCNAK;
			break;
		}
		DISABLE_UNLOCK(&log_lock);
		iocp->ioc_count = 0;
		qreply(q, mp);
		return 0;
	case M_FLUSH:
		if (*mp->b_rptr & FLUSHR) {
			*mp->b_rptr &= ~FLUSHW;
			qreply(q, mp);
			return 0;
		}
		break;
	}
	freemsg(mp);
	return 0;
}
