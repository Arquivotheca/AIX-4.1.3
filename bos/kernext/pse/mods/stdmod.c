static char sccsid[] = "@(#)09        1.1  src/bos/kernext/pse/mods/stdmod.c, sysxpse, bos411, 9428A410j 8/27/93 09:21:09";
/*
 * COMPONENT_NAME: SYSXPSE - STREAMS framework
 * 
 * FUNCTIONS:      stdmod_config
 *                 mod_open
 *                 null_rput
 *                 null_wput
 *                 pass_put
 *                 spass_put
 *                 spass_srv
 *                 pmod_put
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

/** Copyright (c) 1988-1991  Mentat Inc.
 **/

#include <pse/str_system.h>
#include <sys/stream.h>
#include <sys/stropts.h>
#include <pse/str_debug.h>
#include <sys/device.h>
#include <sys/uio.h>
#include <sys/strconf.h>

extern dev_t clonedev;

static int      mod_open(queue_t *, dev_t *, int, int, cred_t *);
static int	null_rput(queue_t *, mblk_t *);
static int	null_wput(queue_t *, mblk_t *);
static int	pass_put(queue_t *, mblk_t *);
static int	spass_put(queue_t *, mblk_t *);
static int	spass_srv(queue_t *);
static int	pmod_put(queue_t *, mblk_t *);

/*
 * Null is a discard driver, and also functions as a mux or module.
 */

static struct module_info nulinfo =  {
	5003, "null", 0, INFPSZ, 512, 128
};
static struct qinit nulmrinit = {
	null_rput, NULL, mod_open, NULL, NULL, &nulinfo
};
static struct qinit nulmwinit = {
	null_wput, NULL, NULL, NULL, NULL, &nulinfo
};
struct streamtab nulminfo ={ &nulmrinit, &nulmwinit };

/*
 * Pass is a passthrough module with an "errm" alterego.
 */
static struct module_info passminfo =  {
	5003, "pass", 0, INFPSZ, 2048, 128
};
static struct qinit passrinit = {
	pass_put, NULL, mod_open, NULL, NULL, &passminfo
};
static struct qinit passeinit = {
	NULL, NULL, NULL, NULL, NULL, &passminfo
};
struct streamtab passinfo = { &passrinit, &passrinit };
struct streamtab modeinfo = { &passrinit, &passeinit };

/*
 * Spass is a passthrough via a service procedure.
 */
static struct module_info spassminfo =  {
	5007, "spass", 0, INFPSZ, 2048, 128
};
static struct qinit spassrinit = {
	spass_put, spass_srv, mod_open, NULL, NULL, &spassminfo
};
struct streamtab spassinfo = { &spassrinit, &spassrinit };

/*
 * Rspass is a read side half-spass handy for pushing on
 * drivers which call putnext in interrupt context.
 */
static struct module_info rspassminfo =  {
	5008, "rspass", 0, INFPSZ, 2048, 128
};
static struct qinit rspassrinit = {
	spass_put, spass_srv, mod_open, NULL, NULL, &rspassminfo
};
static struct qinit rspasswinit = {
	pass_put, NULL, NULL, NULL, NULL, &rspassminfo
};
struct streamtab rspassinfo = { &rspassrinit, &rspasswinit };

/*
 * Pipemod implement pipes and fifo's.
 */
static struct module_info pipminfo =  {
	5303, "pipemod", 0, INFPSZ, PIPSIZ, PIPSIZ-1
};
static struct qinit pipmrinit = {
	pmod_put, NULL, mod_open, NULL, NULL, &pipminfo
};
struct streamtab pmodinfo = { &pipmrinit, &pipmrinit };

strconf_t std_entries[] = {
	{ "null", &nulminfo, STR_NEW_OPEN, 0, SQLVL_QUEUE, 0 },
	{ "pass", &passinfo, STR_NEW_OPEN, 0, SQLVL_QUEUE, 0 },
	{ "errm", &modeinfo, STR_NEW_OPEN, 0, SQLVL_QUEUE, 0 },
	{ "spass", &spassinfo, STR_NEW_OPEN, 0,SQLVL_QUEUE, 0 },
	{ "rspass", &rspassinfo, STR_NEW_OPEN, 0,SQLVL_QUEUE, 0},
	{ "pipemod", &pmodinfo, STR_NEW_OPEN, 0, SQLVL_QUEUE, 0},
};

#define NSTD   (sizeof std_entries / sizeof std_entries[0])

int
stdmod_config (cmd, uiop)
	int cmd;
	struct uio *uiop;
{
	char buf[FMNAMESZ+1];
	int i;
	int error = 0;

	if (error = uiomove((char *)buf, sizeof(buf), UIO_WRITE, uiop))
	    return EFAULT;
	buf[FMNAMESZ] = '\0';

	for (i = 0; i < NSTD; i++) {
	    if (!strcmp(buf, std_entries[i].sc_name)) break;
	}

	if (i == NSTD) return EPROTONOSUPPORT;

	switch (cmd) {
	case CFG_INIT:
		return str_install(STR_LOAD_MOD, &std_entries[i]);
	case CFG_TERM:
		return str_install(STR_UNLOAD_MOD, &std_entries[i]);
	default:
		return EINVAL;
	}
}

static int
mod_open(q, devp, flag, sflag, credp)
        queue_t * q;
        dev_t   * devp;
        int     flag;
        int     sflag;
        cred_t  * credp;
{
	int (*putp)() = WR(q)->q_qinfo->qi_putp;
	if (putp == NULL)	/* errm */
		return ENXIO;
	return 0;
}


/* For use when configured as a module (as opposed to a device) */
static int
null_rput (q, mp)
	queue_t	* q;
	mblk_t * mp;
{
	switch (mp->b_datap->db_type) {
	case M_FLUSH:
		break;
	case M_DATA:
		if (msgdsize(mp) == 0)
			break;
		/* fall through */
	default:
		freemsg(mp);
		return 1;
	}
	putnext(q, mp);
	return 1;
}

static int
null_wput (q, mp)
	queue_t	* q;
	mblk_t * mp;
{
	switch (mp->b_datap->db_type) {
	case M_IOCTL:
		switch (((struct iocblk *)mp->b_rptr)->ioc_cmd) {
		case I_PLINK:
		case I_PUNLINK:
		case I_LINK:
		case I_UNLINK:
			mp->b_datap->db_type = M_IOCACK;
			((struct iocblk *)mp->b_rptr)->ioc_count = 0;
			((struct iocblk *)mp->b_rptr)->ioc_error = 0;
			break;
		default:
			mp->b_datap->db_type = M_IOCNAK;
			break;
		}
		if (mp->b_cont) {
			freemsg(mp->b_cont);
			mp->b_cont = 0;
		}
		qreply(q, mp);
		return 1;
	case M_FLUSH:
		if (*mp->b_rptr & FLUSHR) {
			*mp->b_rptr &= ~FLUSHW;
			qreply(q, mp);
			return 1;
		}
		break;
	}
	freemsg(mp);
	return 1;
}

static int
pass_put (q, mp)
	queue_t	* q;
	mblk_t * mp;
{
	putnext(q, mp);
	return 1;
}

static int
spass_put (q, mp)
	queue_t	* q;
	mblk_t * mp;
{
	if (mp->b_datap->db_type == M_FLUSH) {
		if (*mp->b_rptr & ((q->q_flag & QREADR) ? FLUSHR : FLUSHW)) {
			if ((*mp->b_rptr & FLUSHBAND)
			&&  mp->b_wptr - mp->b_rptr == 2)
				flushband(q, mp->b_rptr[1], FLUSHALL);
			else
				flushq(q, FLUSHALL);
		}
		putnext(q, mp);
		return 1;
	}
	if (putq(q, mp))
		return 1;
	freemsg(mp);
	return 0;
}

static int
spass_srv (q)
	queue_t	* q;
{
	mblk_t * mp;

	while (mp = getq(q)) {
		if (mp->b_datap->db_type < QPCTL
		&&  !bcanput(q->q_next, mp->b_band)) {
			(void) putbq(q, mp);
			break;
		}
		putnext(q, mp);
	}
	return 0;
}

static int
pmod_put (q, mp)
	queue_t	* q;
	mblk_t	* mp;
{
	switch (mp->b_datap->db_type) {
	case M_IOCTL:
		mp->b_datap->db_type = M_IOCNAK;
		if (mp->b_cont) {
			freemsg(mp->b_cont);
			mp->b_cont = 0;
		}
		qreply(q, mp);
		break;
	case M_FLUSH:
		if (q->q_flag & QREADR) {
			if (mp->b_rptr[0] == FLUSHR)
				mp->b_rptr[0] = FLUSHW;
		} else {
			if (mp->b_rptr[0] == FLUSHW)
				mp->b_rptr[0] = FLUSHR;
		}
		/* fall through */
	default:
		putnext(q, mp);
		break;
	}
	return 1;
}
