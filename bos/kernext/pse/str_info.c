static char sccsid[] = "@(#)20        1.7  src/bos/kernext/pse/str_info.c, sysxpse, bos411, 9428A410j 6/6/94 04:56:10";
/*
 * COMPONENT_NAME: SYSXPSE - STREAMS framework
 * 
 * FUNCTIONS:      heap_report
 *                 dump_module_names
 *                 streams_info_runq
 *                 strinfo
 *
 * ORIGINS: 27, 83
 * 
 */
/*
 *   (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#include <sys/errno.h>
#include <pse/str_stream.h>
#include <sys/stropts.h>
#include <sys/strinfo.h>

extern struct modsw *  dmodsw;
extern struct modsw *  fmodsw;
decl_simple_lock_data(extern,modsw_lock)
decl_simple_lock_data(extern,streams_open_lock)

extern SQH	streams_runq;

/*
 * streams initialization state
 * init_state == 0:	  loaded, in memory, uninitialized
 * init_state >  0:	  in various stages of initialization
 * init_state | INITDONE: initialization complete
 */

extern int init_state;

#define   SQ_EMPTY(sq)            (((SQP)(sq))->sq_next == (SQP)(sq))

/*
 * heap_report can be called for debugging or administrative purposes to print
 * a summary of the current state of the heap on the "console".
 */

#define	MSG_LEN	4096
static char msg[MSG_LEN];

void
heap_report (len) 
	int	*len;
{
	ENTER_FUNC(heap_report, len, 0, 0, 0, 0, 0);
	*len = 0;
	LEAVE_FUNC(heap_report, 0);
} 

/*
 * dump_module_names - strinfo debugging request
 */

void
dump_module_names (lbuf)
	int	*lbuf;
{
	struct modsw *dmp;
	struct modsw *fmp;
	char *pbuf = msg;
	char ptmp[80];
	int len;

	ENTER_FUNC(dump_module_names, lbuf, 0, 0, 0, 0, 0);
	len = 0;
	SIMPLE_LOCK(&modsw_lock);

        for (dmp = dmodsw; dmp; ) {
            if (dmp->d_str) {
                if (((len = mi_sprintf(ptmp,
		    "Device: '%s', dcookie 0x%x, flags: 0x%x, str 0x%x\n",
                    dmp->d_name, dmp->d_major, dmp->d_flags, dmp->d_str))
		    != -1) && (pbuf + len < msg + MSG_LEN)) {
			bcopy(ptmp, pbuf, len);
			pbuf += len;
		}
	    }
	    if ((dmp = dmp->d_next) == dmodsw)
		break;
	}
	for (fmp = fmodsw; fmp; ) {
            if (fmp->d_str) {
		if (((len = mi_sprintf(ptmp,
		    "Module: '%s', flags: 0x%x, str 0x%x\n",
		    fmp->d_name, fmp->d_flags, fmp->d_str)) != -1)
		    && (pbuf + len < msg + MSG_LEN)) {
			bcopy(ptmp, pbuf, len);
			pbuf += len;
		}
	    }
	    if ((fmp = fmp->d_next) == fmodsw)
		break;
        } 

	SIMPLE_UNLOCK(&modsw_lock);
	*lbuf = pbuf - msg;

	LEAVE_FUNC(dump_module_names, 0);
}

/*
 * streams_info_runq - provide information on run queues
 */

staticf int
streams_info_runq(lbuf)
	int	*lbuf;
{
	SQP	sq;
	queue_t	* q;
	SQHP	sqh = &streams_runq;

	STHP sth;
	char *pbuf = msg;
	int len = 0;
	int i;
extern STHP    sth_open_streams[];
	char ptmp[80];

	DISABLE_LOCK_DECL

	ENTER_FUNC(streams_info_runq, lbuf, 0, 0, 0, 0, 0);

	if (((len = mi_sprintf(ptmp, "\nActive Stream Heads\n")) != -1)
		&& (pbuf + len < msg + MSG_LEN)) {
		bcopy(ptmp, pbuf, len);
		pbuf += len;
	}
	if (((len = mi_sprintf(ptmp,
	    "sth       sth_dev   sth_rq    sth_wq    sth_flag  rq->q_first\n"))
	    != -1) && (pbuf + len < msg + MSG_LEN)) {
		bcopy(ptmp, pbuf, len);
		pbuf += len;
	}

	DISABLE_LOCK(&streams_open_lock);
	for (i = 0; i < STH_HASH_TBL_SIZE; i++) {
	    for (sth = sth_open_streams[i]; sth; sth = sth->sth_next) {
		if (((len = mi_sprintf(ptmp,
		    "%08x  %08x  %08x  %08x  %08x  %08x\n",
		    sth, sth->sth_dev, sth->sth_rq,
		    sth->sth_wq, sth->sth_flags,
		    sth->sth_rq->q_first)) != -1)
		    && (pbuf + len < msg + MSG_LEN)) {
			bcopy(ptmp, pbuf, len);
			pbuf += len;
		}
	    }
	}
	DISABLE_UNLOCK(&streams_open_lock);
 
	if ( SQ_EMPTY(sqh) ) {
		*lbuf = pbuf - msg;
		LEAVE_FUNC(streams_info_runq, -1);
		return;
	}
	if (((len = mi_sprintf(ptmp, "\nSTREAMS Service Queue\n")) != -1)
	    && (pbuf + len < msg + MSG_LEN)) {
		bcopy(ptmp, pbuf, len);
		pbuf += len;
	}
	LOCK_QUEUE(sqh);
	for (sq = sqh->sqh_next; (SQHP)sq != sqh; sq = sq->sq_next) {
		q = (queue_t *)sq->sq_arg0;
		if (((len = mi_sprintf(ptmp,
			"Queue 0x%x\tFlags 0x%x\n", q, q->q_flag)) != -1)
		   && (pbuf + len < msg + MSG_LEN)) {
			bcopy(ptmp, pbuf, len);
			pbuf += len;
		}
	}
	UNLOCK_QUEUE(sqh);
	*lbuf = pbuf - msg;
	LEAVE_FUNC(streams_info_runq, 0);
	return;
}

/*
 * ENTRYPOINT
 * strinfo - provide administrative info on pse activity
 *
 * syscall
 */

int
strinfo(cmd, buf, lbuf)
	int	cmd;
	char	*buf;
	int	*lbuf;
{
	int error = 0;
	int len;

	ENTER_FUNC(strinfo, cmd, buf, lbuf, 0, 0, 0);

	if (!(init_state & INITDONE)) {
		setuerror( ENOTREADY);
		LEAVE_FUNC(strinfo, -1);
		return -1;
	}

	switch (cmd) {
	case STR_INFO_HEAP:
		heap_report(&len);
		break;

	case STR_INFO_MODS:
		dump_module_names(&len);
		break;

	case STR_INFO_RUNQ:
		streams_info_runq(&len);
		break;

	default:
		setuerror(EINVAL);
		LEAVE_FUNC(strinfo, -1);
		return -1;
	}

	if (len) copyout(msg, buf, len);
	copyout(&len, lbuf, sizeof(int));
	LEAVE_FUNC(strinfo, error);
	return error;
}
