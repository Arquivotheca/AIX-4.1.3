static char sccsid[] = "@(#)37	1.3  src/bos/kernext/pse/mods/echo.c, sysxpse, bos411, 9428A410j 7/16/91 16:04:38";
/*
 *   COMPONENT_NAME: SYSXPSE
 *
 *   ORIGINS: 27 63
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/** Copyright (c) 1990  Mentat Inc.
 ** echo.c 2.6
 **/

#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/signal.h>
#include <pse/common.h>
#include <sys/sysmacros.h>
#include <sys/errno.h>

#define	ECHO_IOCACK		1	/** Do an iocack reply with no data */
#define	ECHO_IOCNAK		2	/** Do an iocnak reply */
#define	ECHO_NOREPLY		3	/** Let the ioctl timeout, no reply */
#define	ECHO_DATA		4	/** Do an iocack with data */
#define	ECHO_BIGDATA		5	/** Do an iocack with > ic_len data */
#define	ECHO_GENMSG		6	/** Gen a msg from the eblks in data */
#define	ECHO_GENMSG_NOREPLY	7	/** Gen a msg, no reply to M_IOCTL */
#define	ECHO_FEED_ME		8	/** Send infinite messages upstream. */
#define	ECHO_RVAL		0x8000	/** Add return value */
#define	ECHO_RERROR		0x4000	/** Error return */

typedef struct echo_s {
	int	echo_flags;
} echo_t;
#define	ECHO_FEEDER	1

typedef struct iecho_s {
	int	ie_error;
	int	ie_rval;
	char	* ie_buf;	/* buffer address for transparent ioctls */
	int	ie_len;		/* buffer length for transparent ioctls */
} iecho_t;

typedef	struct echo_blk {
	int	eb_type;	/* type of genned mblk */
	int	eb_len;		/* len of data to put in mblk; */
				/* data follows immediately (if any) */
	int	eb_flag;	/* flag word to be copied into b_flag */
} eblk_t;

staticf	int	echo_admin(void);
staticf int	echo_close(queue_t * q);
staticf int	echo_open(queue_t *, dev_t *, int, int, cred_t *);
staticf int	echo_rsrv(queue_t * q);
staticf int	echo_wsrv(queue_t * q);
staticf int	echo_wput(queue_t * q, mblk_t * mp);

static struct module_info minfo =  {
	5000, "echo", 0, INFPSZ, 2048, 128
};

static struct qinit rinit = {
	nil(pfi_t), echo_rsrv, echo_open, echo_close, echo_admin, &minfo
};

static struct qinit winit = {
	echo_wput, echo_wsrv, nil(pfi_t), nil(pfi_t), nil(pfi_t), &minfo
};

struct streamtab echoinfo = { &rinit, &winit };

static	IDP	echo_g_head;

staticf int
echo_admin () {
	return 0;
}

staticf int
echo_close (q)
	queue_t	* q;
{
	WR(q)->q_next = nilp(queue_t);
	return mi_close_comm(&echo_g_head, q);
}

staticf int
echo_open (q, devp, flag, sflag, credp)
	queue_t	* q;
	dev_t	* devp;
	int	flag;
	int	sflag;
	cred_t	* credp;
{
	int	err;

	err = mi_open_comm(&echo_g_head, sizeof(echo_t),
						q, devp, flag, sflag, credp);
	if (!err) {
		echo_t	* echo = (echo_t *)q->q_ptr;
		echo->echo_flags = 0;
		/* Create a stream pipe (mostly for testing SENDFD) */
		WR(q)->q_next = q;
	}
	return err;
}

staticf int
echo_rsrv (q)
	queue_t	* q;
{
	echo_t	* echo = (echo_t *)q->q_ptr;
	MBLKP	mp;

	if ( !(echo->echo_flags & ECHO_FEEDER) ) {
		qenable(WR(q));
		return 0;
	}
	/* Produce a steady stream of messages for a reader process. */
	while ( canput(q->q_next) ) {
		mp = allocb(4096, BPRI_LO);
		if ( !mp ) {
			bufcall(4096, BPRI_LO, (pfi_t)qenable, (long)q);
			return 0;
		}
		mp->b_wptr = mp->b_datap->db_lim;
		putnext(q, mp);
	}
	return 0;
}

staticf int
echo_wsrv (q)
	queue_t	* q;
{
	struct iocblk	* iocp;
	mblk_t	* mp, * mp1;

	while (mp = getq(q)) {
		switch (mp->b_datap->db_type) {
		default:
			if (!bcanput(RD(q)->q_next, mp->b_band)) {
				putbq(q, mp);
				return 0;
			}
			qreply(q, mp);
			continue;
		case M_IOCTL:
			iocp = (struct iocblk *)mp->b_rptr;
			if (iocp->ioc_cmd == ECHO_BIGDATA) {
				if (!(mp1 = allocb(100, BPRI_HI))) {
					putbq(q, mp);
					bufcall(100, BPRI_HI, (pfi_t)echo_wsrv, (long)q);
					return 0;
				}
				iocp->ioc_count += 100;
				mp1->b_cont = mp->b_cont;
				mp->b_cont = mp1;
				mp->b_datap->db_type = M_IOCACK;
				qreply(q, mp);
				continue;
			} else {
				freemsg(mp);
				continue;
			}
		}
	}
	return 0;
}

staticf int
echo_wput (q, mp)
	queue_t	* q;
	mblk_t	* mp;
{
	struct copyresp	* cp;
	int	cmd;
	struct copyreq	* cq;
	echo_t	* echo = (echo_t *)q->q_ptr;
	iecho_t	* ie;
	struct iocblk	* iocp;
	mblk_t	* mp1, * mp2, * first_mp;
	eblk_t	* ep;

	switch (mp->b_datap->db_type) {
	case M_FLUSH:
		if (*mp->b_rptr & FLUSHW) {
			if ((*mp->b_rptr & FLUSHBAND) &&
			    mp->b_wptr - mp->b_rptr == 2)
				flushband(q, mp->b_rptr[1], FLUSHALL);
			else
				flushq(q, FLUSHALL);
		}
		if (*mp->b_rptr & FLUSHR) {
			if ((*mp->b_rptr & FLUSHBAND) &&
			    mp->b_wptr - mp->b_rptr == 2)
				flushband(RD(q), mp->b_rptr[1], FLUSHALL);
			else
				flushq(RD(q), FLUSHALL);
			*mp->b_rptr &= ~FLUSHW;
			qreply(q, mp);
			return 0;
		}
		break;
	case M_IOCDATA:
		cp = (struct copyresp *)mp->b_rptr;
		iocp = (struct iocblk *)cp;
		if (cp->cp_rval != 0) {
			iocp->ioc_error = (int)cp->cp_rval;
			goto iocack;
		}
		if (!cp->cp_private) {
			ie = (iecho_t *)mp->b_cont->b_rptr;
			if (ie->ie_len > 0) {
				cp->cp_private = mp->b_cont;
				mp->b_cont = allocb(ie->ie_len, BPRI_MED);
				if (!mp->b_cont) {
					freemsg(cp->cp_private);
					iocp->ioc_error = ENOMEM;
					goto iocack;
				}
				mp->b_cont->b_rptr[0] = 'A';
				mp->b_cont->b_wptr += ie->ie_len;
				mp->b_datap->db_type = M_COPYOUT;
				cq = (struct copyreq *)cp;
				cq->cq_addr = ie->ie_buf;
				cq->cq_size = ie->ie_len;
				cq->cq_flag = 0;
				qreply(q, mp);
				return 0;
			}
			if (iocp->ioc_cmd & ECHO_RVAL) 
				iocp->ioc_rval = ie->ie_rval;
			else
				iocp->ioc_rval = 0;
			if (iocp->ioc_cmd & ECHO_RERROR) 
				iocp->ioc_error = ie->ie_error;
			else
				iocp->ioc_error = 0;
		} else {
			ie = (iecho_t *)cp->cp_private->b_rptr;
			if (iocp->ioc_cmd & ECHO_RVAL)
				iocp->ioc_rval = ie->ie_rval;
			else
				iocp->ioc_rval = 0;
			if (iocp->ioc_cmd & ECHO_RERROR)
				iocp->ioc_error = ie->ie_error;
			else
				iocp->ioc_error = 0;
			freemsg(cp->cp_private);
			cp->cp_private = nilp(mblk_t);
		}
		iocp->ioc_cmd &= ~(ECHO_RVAL | ECHO_RERROR);
		switch (iocp->ioc_cmd) {
		case ECHO_IOCACK:
			fallthru;
		case ECHO_DATA:
			iocp->ioc_count = 0;
iocack:
			mp->b_datap->db_type = M_IOCACK;
			qreply(q, mp);
			return 0;
		default:
			iocp->ioc_error = EINVAL;
			fallthru;
		case ECHO_IOCNAK:
iocnak:
			iocp->ioc_count = 0;
			mp->b_datap->db_type = M_IOCNAK;
			qreply(q, mp);
			return 0;
		case ECHO_NOREPLY:
			break;
		}
		break;
	case M_IOCTL:
		iocp = (struct iocblk *)mp->b_rptr;
		iocp->ioc_error = 0;
		if (iocp->ioc_count == TRANSPARENT) {
			switch (iocp->ioc_cmd & ~(ECHO_RVAL | ECHO_RERROR)) {
			default:
				goto iocnak;
			case ECHO_IOCACK:
			case ECHO_DATA:
			case ECHO_IOCNAK:
				break;
			}
			cq = (struct copyreq *)iocp;
			cq->cq_addr = (caddr_t)*(u32 *)mp->b_cont->b_rptr;
			freemsg(mp->b_cont);
			mp->b_cont = nilp(mblk_t);
			cq->cq_size = sizeof(iecho_t);
			cq->cq_flag = 0;
			cq->cq_private = nilp(mblk_t);
			mp->b_datap->db_type = M_COPYIN;
			qreply(q, mp);
			return 0;
		}
		if (mp->b_cont  &&  iocp->ioc_count >= sizeof(iecho_t)) {
			ie = (iecho_t *)mp->b_cont->b_rptr;
			if (iocp->ioc_cmd & ECHO_RVAL)
				iocp->ioc_rval = ie->ie_rval;
			if (iocp->ioc_cmd & ECHO_RERROR)
				iocp->ioc_error = ie->ie_error;
		}
		cmd = (iocp->ioc_cmd & ~(ECHO_RVAL | ECHO_RERROR));
		switch (cmd) {
		case ECHO_IOCACK:
			iocp->ioc_count = 0;
			fallthru;
		case ECHO_DATA:
			goto iocack;
		default:
			iocp->ioc_error = EINVAL;
			fallthru;
		case ECHO_IOCNAK:
			goto iocnak;
		case ECHO_NOREPLY:
			break;
		case ECHO_BIGDATA:
			if (mp1 = allocb(100, BPRI_HI)) {
				iocp->ioc_count += 100;
				mp1->b_wptr += 100;
				mp1->b_cont = mp->b_cont;
				mp->b_cont = mp1;
				goto iocack;
			} else
				putq(q, mp);
			return 0;
		case ECHO_GENMSG:
		case ECHO_GENMSG_NOREPLY:
			if (!mp->b_cont)
				goto iocnak;
			ep = (eblk_t *)mp->b_cont->b_rptr;
			first_mp = nilp(mblk_t);
			while (&ep[1] <= (eblk_t *)mp->b_cont->b_wptr) {
				eblk_t	* next_ep =
					(eblk_t *)((char *)&ep[1] + ep->eb_len);
				
				if ((unsigned char *)ep + ep->eb_len > mp->b_cont->b_wptr
				|| !(mp2 = allocb(ep->eb_len, BPRI_HI))) {
					if (first_mp)
						freemsg(first_mp);
					goto iocnak;
				}
				mp2->b_datap->db_type = ep->eb_type;
				mp2->b_flag = ep->eb_flag;
				bcopy((char *)&ep[1], (char *)mp2->b_wptr, ep->eb_len);
				mp2->b_wptr += ep->eb_len;
				if (!first_mp)
					first_mp = mp2;
				else
					mp1->b_cont = mp2;
				mp1 = mp2;
				ep = next_ep;
			}
			if (cmd == ECHO_GENMSG_NOREPLY)
				freemsg(mp);
			else {
				iocp->ioc_count = 0;
				mp->b_datap->db_type = M_IOCACK;
				qreply(q, mp);
			}
			qreply(q, first_mp);
			return 0;
		case ECHO_FEED_ME:
			q = RD(q);
			echo->echo_flags |= ECHO_FEEDER;
			iocp->ioc_error = 0;
			iocp->ioc_count = 0;
			mp->b_datap->db_type = M_IOCACK;
			putnext(q, mp);
			echo_rsrv(q);
			return 0;
		}
		break;
	case M_PROTO:
		if (mp->b_datap->db_lim - mp->b_datap->db_base < 64) {
			printf("echo: M_PROTO block contains %d bytes\n",
				mp->b_datap->db_lim - mp->b_datap->db_base);
			/*
			 * Zero the M_PROTO block in an attempt to notify
			 * the application that something was wrong.
			 */
			mp->b_rptr = mp->b_wptr;
		}
		fallthru;
	default:
		if (mp->b_datap->db_type > QPCTL ||
		    bcanput(RD(q)->q_next, mp->b_band))
			qreply(q, mp);
		else
			putq(q, mp);
		return 0;
	}
	freemsg(mp);
	return 0;
}

#include <sys/device.h>
#include <sys/strconf.h>

int
echo_config(dev, cmd, uiop)
	dev_t dev;
	int cmd;
	struct uio *uiop;
{
	static strconf_t conf = {
		"echo", &echoinfo, STR_NEW_OPEN,
	};

	conf.sc_major = major(dev);

	switch (cmd) {
	case CFG_INIT:	return str_install(STR_LOAD_DEV, &conf);
	case CFG_TERM:	return str_install(STR_UNLOAD_DEV, &conf);
	default:	return EINVAL;
	}
}
