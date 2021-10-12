static char sccsid[] = "@(#)07        1.6  src/bos/kernext/pse/mods/stddev.c, sysxpse, bos411, 9439A411b 9/22/94 04:32:58";
/*
 * COMPONENT_NAME: SYSXPSE - STREAMS framework
 * 
 * FUNCTIONS:      stddev_config
 *                 drv_open
 *                 drv_close
 *                 echo_wput
 *                 null_lrput
 *                 null_wput
 *                 echo_srv
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
/** Copyright (c) 1988  Mentat Inc.
 ** nd.c 1.5, last change 1/2/90
 **/

#include <pse/str_system.h>
#include <sys/sleep.h>
#include <sys/stream.h>
#include <sys/stropts.h>
#include <pse/str_debug.h>
#include <sys/device.h>
#include <sys/uio.h>
#include <sys/strconf.h>
#include <pse/mi.h>


extern dev_t clonedev;
extern int unweldq();

static int	drv_open(queue_t *, dev_t *, int, int, cred_t *);
static int 	drv_close(queue_t *);
static int	echo_wput(queue_t *, mblk_t *);
static int	null_lrput(queue_t *, mblk_t *);
static int	null_wput(queue_t *, mblk_t *);
static int	pmod_put(queue_t *, mblk_t *);
static int	echo_srv(queue_t *);

/*
 * Echo is a simple echo driver.
 */
static struct module_info echominfo =  {
	5000, "echo", 0, INFPSZ, 2048, 128
};
static struct qinit echorinit = {
	NULL, NULL, drv_open, drv_close, NULL, &echominfo
};
static struct qinit echowinit = {
	echo_wput, echo_srv, NULL, NULL, NULL, &echominfo
};
struct streamtab echoinfo = { &echorinit, &echowinit };

/*
 * Null is a discard driver, and also functions as a mux or module.
 */
static struct module_info nullinfo =  {
	5001, "nuls", 0, INFPSZ, 512, 128
};
static struct qinit nulllrinit = {	/* lower read */
	null_lrput, NULL, NULL, NULL, NULL, &nullinfo
};
static struct qinit nulllwinit = {	/* lower write */
	NULL, NULL, NULL, NULL, NULL, &nullinfo
};
static struct qinit nullrinit = {
	NULL, NULL, drv_open, drv_close, NULL, &nullinfo
};
static struct qinit nullwinit = {
	null_wput, NULL, NULL, NULL, NULL, &nullinfo
};
struct streamtab nulsinfo ={ &nullrinit, &nullwinit, &nulllrinit, &nulllwinit };

/*
 * Pipe implements pipes and fifo's.
 */

static struct module_info pipinfo =  {
	5304, "pipe", 0, INFPSZ, PIPSIZ, PIPSIZ-1
};
static struct qinit piperinit = {
	pmod_put, NULL, drv_open, drv_close, NULL, &pipinfo
};
static struct qinit pipewinit = {
	pmod_put, NULL, NULL, NULL, NULL, &pipinfo
};
struct streamtab pipeinfo = { &piperinit, &pipewinit };

typedef struct dev_s {
	dev_t   d_dev;
	int     *d_ev;
} STD, *STDP, **STDPP;

struct dev_entry_t {
	IDP	  g_head;
	strconf_t conf;
} std_entries[] = {
    {NULL, { "nuls", &nulsinfo, STR_NEW_OPEN|STR_MPSAFE, 0, SQLVL_QUEUE, 0}},
    {NULL, { "echo", &echoinfo, STR_NEW_OPEN|STR_MPSAFE, 0, SQLVL_QUEUE, 0}},
    {NULL, { "pipe", &pipeinfo, STR_NEW_OPEN|STR_MPSAFE, 0, SQLVL_QUEUE, 0}},
};

#define NSTD   (sizeof std_entries / sizeof std_entries[0])

int
stddev_config (dev, cmd, uiop)
	dev_t		dev;
	int		cmd;
	struct uio	*uiop;
{
	int i;
	int error = 0;
	char buf[FMNAMESZ+1];

	if (error = uiomove((char *)buf, sizeof(buf), UIO_WRITE, uiop))
	    return EFAULT;
	buf[FMNAMESZ] = '\0';

	for (i = 0; i < NSTD; i++) {
		if (!strcmp(buf, std_entries[i].conf.sc_name)) break;
	}

	if (i == NSTD) return EPROTONOSUPPORT;

	std_entries[i].conf.sc_major = major(dev);

	switch (cmd) {
	case CFG_INIT:  return str_install(STR_LOAD_DEV, &std_entries[i].conf);
        case CFG_TERM:  return str_install(STR_UNLOAD_DEV,&std_entries[i].conf);
        default:        return EINVAL;
        }
	
}

/*
 * Common open routine for these drivers and modules.
 */
static int
drv_open (q, devp, flag, sflag, credp)
	queue_t	* q;
	dev_t	* devp;
	int	flag;
	int	sflag;
	cred_t	* credp;
{
	int	i;
	STDP	dp;
	int	err, (*putp)() = WR(q)->q_qinfo->qi_putp;

	err = 0;
	if (putp == NULL) {	/* errm */
		return ENXIO;
	}
	for (i = 0; i < NSTD; i++) {
		if (std_entries[i].conf.sc_major == major(*devp))
			break;
	}
	if (i == NSTD) return ENXIO;
	if (err = mi_open_comm(&std_entries[i].g_head, sizeof(STD), q, devp,
						flag, sflag, credp))
		return err;
	dp = (STDP)q->q_ptr;
	dp->d_dev = *devp;
	dp->d_ev = EVENT_NULL;
	if (putp == echo_wput &&  sflag != MODOPEN && !WR(q)->q_next) {
		extern void e_wakeup();

		e_assert_wait(&(dp->d_ev), FALSE);
		err = weldq(WR(q), q, (queue_t *)0, (queue_t *)0, e_wakeup,
								&(dp->d_ev), q);
		if (err == 0) {
			e_block_thread();
		} else
			e_clear_wait(thread_self(), 0);
	}
	return err;
}

static int
drv_close (q)
	queue_t * q;
{
	int	i, err;
	STDP	dp;
	int	(*putp)() = WR(q)->q_qinfo->qi_putp;

	for (i = 0; i < NSTD; i++) {
		if (std_entries[i].conf.sc_str->st_rdinit == q->q_qinfo)
			break;
	}
	if (i == NSTD) return ENXIO;

	dp = (STDP)q->q_ptr;

	if (putp == echo_wput && WR(q)->q_next) {
		extern void e_wakeup();

		e_assert_wait(&(dp->d_ev), FALSE);
		err = unweldq(WR(q), q, (queue_t *)0, (queue_t *)0, e_wakeup,
								&(dp->d_ev), q);
		if (err == 0) {
			e_block_thread();
		}
		else {
			e_clear_wait(thread_self(), 0);
			return err;
		}
	}
	return mi_close_comm(&std_entries[i].g_head, q);
}


static int
echo_wput (q, mp)
	queue_t	* 	q;
	mblk_t *	mp;
{
	switch (mp->b_datap->db_type) {
	case M_FLUSH:
		if (*mp->b_rptr & (FLUSHR|FLUSHW)) {
			if ((*mp->b_rptr & FLUSHBAND)
			&&  mp->b_wptr - mp->b_rptr == 2)
				flushband(q, mp->b_rptr[1], FLUSHALL);
			else
				flushq(q, FLUSHALL);
		}
		if (*mp->b_rptr & FLUSHR) {
			*mp->b_rptr &= ~FLUSHW;
			qreply(q, mp);
			return 1;
		}
		break;
	case M_IOCTL:
		mp->b_datap->db_type = M_IOCNAK;
		if (mp->b_cont) {
			freemsg(mp->b_cont);
			mp->b_cont = 0;
		}
		qreply(q, mp);
		return 1;
	default:
		if (mp->b_datap->db_type >= QPCTL
		||  bcanput(RD(q)->q_next, mp->b_band)) {
			qreply(q, mp);
			return 1;
		}
		if (putq(q, mp))
			return 1;
		break;
	}
	freemsg(mp);
	return 0;
}

static int
null_lrput (q, mp)
	queue_t	* q;
	mblk_t * mp;
{
	if (mp->b_datap->db_type == M_FLUSH) {
		/* Tests are reversed because we are the acting "stream head"*/
		if (*mp->b_rptr & FLUSHW) {
			*mp->b_rptr &= ~FLUSHR;
			qreply(q, mp);
			return 1;
		}
	}
	freemsg(mp);
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
echo_srv (q)
	queue_t	* q;
{
	mblk_t * mp;

	while (mp = getq(q)) {
		if (mp->b_datap->db_type < QPCTL
		&&  !bcanput(q->q_next, mp->b_band)) {
			(void) putbq(q, mp);
			break;
		}
		qreply(q, mp);
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
