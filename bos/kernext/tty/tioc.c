#ifndef lint
static char sccsid[] = "@(#)63 1.8 src/bos/kernext/tty/tioc.c, sysxtty, bos412, 9447A 11/11/94 14:27:36";
#endif
/*
 * COMPONENT_NAME: (sysxtty) tioc streams module
 *
 * FUNCTIONS: tioc_config, tioc_open, tioc_close, tioc_rput,
 *            tioc_wput, tioc_listadd, tioc_find
 *
 * ORIGINS: 40, 71, 83
 *
 */
/************************************************************************
 *									*
 *			Copyright (c) 1990 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/
/*
 * OSF/1 1.2
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#include <sys/types.h>
#include <sys/signal.h>
#include <sys/conf.h>
#include <sys/malloc.h>
#include <sys/errno.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/uio.h>

#include <sys/stropts.h>
#include <sys/device.h>
#include <sys/ioctl.h>
#include <sys/li.h>
#include <sys/devinfo.h>
#include <sys/str_tty.h>
#include <sys/termiox.h>
#include "stream_tioc.h"
#include <sys/stream.h>

#ifdef	TIOC_DEBUG
#define	tioc_printf	printf
#else
#define	tioc_printf
#endif

/*
 * module for transparent to I_STR ioctl conversion
 */
#define MODULE_ID	7101
#define MODULE_NAME	"tioc"
#define	TIOC_OBUFSIZ	INFPSZ /* track number 140133 */
#define M_HIWAT		2048
#define M_LOWAT		128
static int tioc_rput(), tioc_open(), tioc_close();
static int tioc_wput();

/*
 * declare filter functions: track number 140355.
 */
static int tioc_is_r_interesting(), tioc_is_w_interesting();

static struct module_info minfo = {
	MODULE_ID, MODULE_NAME, 0, TIOC_OBUFSIZ, M_HIWAT, M_LOWAT
};

static struct qinit rinit = {
	tioc_rput, 0, tioc_open, tioc_close, 0, &minfo, 0
};

static struct qinit winit = {
	tioc_wput, 0, 0, 0, 0, &minfo, 0
};

struct streamtab tiocinfo = { &rinit, &winit };

static	int	tioc_dds_count = 0;	/* config methods loads count	*/

#include <sys/sysconfig.h>
#include <sys/strconf.h>

lock_t	tioc_conf_lock = LOCK_AVAIL; /* tioc module configuration lock	*/

int
tioc_config(cmd, uiop)
	int	cmd;
	struct	uio	*uiop;
{
	int 	error = 0, locked;
	struct	tioc_dds	init_tioc_dds;

	static	strconf_t	conf = {
		"tioc",	&tiocinfo, (STR_NEW_OPEN|STR_MPSAFE),
	};

	locked = lockl(&tioc_conf_lock, LOCK_SHORT);
	conf.sc_sqlevel = SQLVL_QUEUEPAIR;
	
	switch (cmd) {
	case CFG_INIT: 
		if (uiop) {
			if (uiomove((char *)&init_tioc_dds,
				    sizeof(struct tioc_dds), UIO_WRITE, uiop) ||
			    (init_tioc_dds.which_dds != TIOC_DDS))
				break;
			else {
				if (tioc_dds_count == 0)
					error=str_install(STR_LOAD_MOD, &conf);
				if (!error)
					tioc_dds_count++;
			}
		}
		else
			error = str_install(STR_LOAD_MOD, &conf);
		break;
	case CFG_TERM: 
		if (uiop) {
			if (uiomove((char *)&init_tioc_dds,
				    sizeof(struct tioc_dds), UIO_WRITE, uiop) ||
			    (init_tioc_dds.which_dds != TIOC_DDS))
				break;
			else {
				if (tioc_dds_count == 1)
					error = str_install(STR_UNLOAD_MOD,
								&conf);
				if (!error)
					tioc_dds_count--;
			}
		}
		else
			error = str_install(STR_UNLOAD_MOD, &conf);
		break;
	default: 
		error = EINVAL;
		break;
	}
	if (locked != LOCK_NEST)
		unlockl(&tioc_conf_lock);
	return(error);
}
/*
 * module specific structures/definitions
 */
#define TIOC_HASH	0xf
#define TIOC_HASHCT	16

struct tioc_s {
	struct tioc_reply *now;	/* ioc_cmd that is being processed now */
	caddr_t uaddr;		/* user address of structure to copy */
	int proc_type;		/* type of command being processed now */
	int openstate;		/* state of the open */
	struct tioc_list *lists[TIOC_HASHCT];	/* hashed list of ioctl's */
};

struct tioc_list {
	struct tioc_list *next;	/* next element in the linked list */
	struct tioc_reply data;	/* information about the ioctl */
};

#define TS_NEEDREPLY	0x0001

struct tioc_reply *tioc_find();
void tioc_listadd();

static int
tioc_open(q, dev, flag, sflag)
	register queue_t *q;
	int dev;
	int flag;
	int sflag;
{
	register struct tioc_s *tip;
	register int i, j, num;
	register mblk_t *mp;
	register struct iocblk *iocp;

	if (q->q_ptr)
		return(0);
	/*
	 * allocate struct tioc_s
	 */
	tip = (struct tioc_s *)he_alloc(sizeof(struct tioc_s), BPRI_MED);
	if (!tip)
		return(OPENFAIL);
	/*
	 * default values for contents of tioc_s
	 */
	tip->now = 0;
	tip->uaddr = 0;
	for (i = 0; i < TIOC_HASHCT; i++)
		tip->lists[i] = 0;
	q->q_ptr = (char *)tip;
	WR(q)->q_ptr = (char *)tip;
	/*
	 * Added the initialization of the q_is_interesting field on the
	 * read and write queues, for streamhead to be able to skip the
	 * M_DATA messages from tioc module. Performance track number 140355.
	 */
	wantmsg(q, tioc_is_r_interesting);
	wantmsg(WR(q), tioc_is_w_interesting);

/*
 * registered the tioc_s structure.
 */
	num = sizeof(ldtty_tioc_reply) / sizeof(struct tioc_reply);
	for (j=0; j<num; j++)
		tioc_listadd(tip, &ldtty_tioc_reply[j]);
	mp = allocb(sizeof(struct iocblk), BPRI_MED);
        if (!mp) {
                he_free(q->q_ptr);
                q->q_ptr = 0;
                return(OPENFAIL);
        }
        mp->b_datap->db_type = M_CTL;
        iocp = (struct iocblk *)mp->b_rptr;
        mp->b_wptr = mp->b_rptr + sizeof(struct iocblk);
        iocp->ioc_cmd = TIOC_REQUEST;
        iocp->ioc_count = 0;
        tip->openstate = TS_NEEDREPLY;
        putnext(WR(q), mp);

	/*
         * send M_CTL to driver to get state info
         */
        (void)tioc_sendctl(WR(q), TIOCGETMODEM, sizeof(int));

	return(0);
}

static int
tioc_close(q)
	register queue_t *q;
{
	register struct tioc_s *tip;
	register int i;

	/*
	 * free struct tioc_s and all stuff inside it
	 */
	if (q->q_ptr) {
		tip = (struct tioc_s *)q->q_ptr;
		/*
		 * free the hashed list of ioctl's
		 */
		for (i = 0; i < TIOC_HASHCT; i++) {
			register struct tioc_list *tlp = tip->lists[i];
			register struct tioc_list *tmp;

			while (tlp) {
				tmp = tlp->next;
				he_free(tlp);
				tlp = tmp;
			}
		}
		he_free(q->q_ptr);
	}
	q->q_ptr = 0;
	return(0);
}

/*
 * some macros for converting messages
 */
#define NAK_IT(mp, err_num) \
	{ \
	register struct iocblk *_iocp; \
	mp->b_datap->db_type = M_IOCNAK; \
	_iocp = (struct iocblk *)mp->b_rptr; \
	_iocp->ioc_error = err_num; \
	_iocp->ioc_rval = 0; \
	_iocp->ioc_count = 0; \
	if (mp->b_cont) { \
		freemsg(mp->b_cont); \
		mp->b_cont = 0; \
	} \
	}

/*
 * read side should process M_IOCACK and M_IOCNAK
 * make sure the answer is what the stream head expects
 * also process the M_CTL to add to the list of ioctl's to process
 */
static int
tioc_rput(q, mp)
	register queue_t *q;
	register mblk_t *mp;
{
	register struct tioc_s *tip;

	tip = (struct tioc_s *)q->q_ptr;
	switch (mp->b_datap->db_type) {
	case M_LETSPLAY:
		++(*(int *)mp->b_rptr);
		putnext(q, mp);
		break;
	case M_CTL: {
		register struct iocblk *iocp;
                register struct tioc_reply *trp;
                register int i, num;
                register mblk_t *wmp;
		
		iocp = (struct iocblk *)mp->b_rptr;
		if (iocp->ioc_cmd != TIOC_REPLY) {
			putnext(q, mp);
			return(0);
		}
                if (mp->b_cont) {
                        /*
                         * add to the hashed list of ioctl's
                         */
                        wmp = mp->b_cont;
                        while (wmp) {
                                num = (wmp->b_wptr - wmp->b_rptr) /
                                        sizeof(struct tioc_reply);
                                trp = (struct tioc_reply *)wmp->b_rptr;
                                for (i = 0; i < num; i++) {
                                        tioc_listadd(tip, trp);
                                        trp++;
                                }
                                wmp = wmp->b_cont;
                        }
                }
                freemsg(mp);
                tip->openstate &= ~TS_NEEDREPLY;
	}
		break;
	case M_IOCACK: {
		register struct iocblk *iocp;

		/*
		 * if we didn't do anything to it, leave it alone
		 */
		if (!tip->now) {
			putnext(q, mp);
			return(0);
		}
		iocp = (struct iocblk *)mp->b_rptr;
		/*
		 * if it's not ours, leave it alone
		 */
		if (tip->now->tioc_cmd != iocp->ioc_cmd) {
			putnext(q, mp);
			return(0);
		}
		switch (tip->proc_type) {
		case TTYPE_COPYIN:
			/*
			 * don't have to do anything, we're done
			 */
			tip->now = 0;
			tip->uaddr = 0;
			break;
		case TTYPE_COPYOUT:
		case TTYPE_COPYINOUT:
		{
			/*
			 * these ioctl's require a COPYOUT
			 */
			register struct copyreq *_cqp;
			register mblk_t *mp1;

			if (!mp->b_cont) {
				NAK_IT(mp, ENOMEM);
				tip->now = 0;
				putnext(q, mp);
				return(0);
			}
			mp->b_datap->db_type = M_COPYOUT;
			_cqp = (struct copyreq *)mp->b_rptr;
			_cqp->cq_addr = tip->uaddr;
			_cqp->cq_size = tip->now->tioc_size;
			_cqp->cq_flag = 0;
			_cqp->cq_private = 0;
			mp->b_wptr = (unsigned char *)(_cqp + 1);
			mp1 = mp->b_cont;
			mp1->b_datap->db_type = M_DATA;
			/*
			 * fix up the size of the data to copy out
			 */
			if (mp1->b_wptr - mp1->b_rptr != _cqp->cq_size) {
				mp1->b_wptr = mp1->b_rptr + _cqp->cq_size;
				tioc_printf("%s fixed up data size for 0x%x\n",
					"tioc: M_COPYOUT:",
					tip->now->tioc_cmd);
			}
			/*
			 * change the type so that we don't do another
			 * M_COPYIN in tioc_wput, M_IOCDATA
			 */
			if (tip->proc_type == TTYPE_COPYINOUT)
				tip->proc_type = TTYPE_COPYOUT;
		}
			break;
		default:
			break;
		}
		putnext(q, mp);
	}
		break;
	case M_IOCNAK:
		tip->now = 0;
		putnext(q, mp);
		break;
	default:
		putnext(q, mp);
		break;
	}
	return(0);
}

/*
 * write side should process M_IOCTL (TRANSPARENT) and M_IOCDATA
 * make sure the M_IOCTL is what the modules below expect
 */
static int
tioc_wput(q, mp)
	register queue_t *q;
	register mblk_t *mp;
{
	register struct tioc_s *tip;

	tip = (struct tioc_s *)q->q_ptr;
	switch (mp->b_datap->db_type) {
	case M_IOCTL: {
		register struct iocblk *iocp;

		/*
		 * if it's not a TRANSPARENT ioctl just send it along
		 */
		iocp = (struct iocblk *)mp->b_rptr;
		if (iocp->ioc_count != TRANSPARENT) {
			putnext(q, mp);
			return(0);
		}
		if (!mp->b_cont) {
			NAK_IT(mp, EINVAL);
			tip->now = 0;
			qreply(q, mp);
			return(0);
		}
		/*
		 * save info about this ioctl, everything starts here
		 */
		tip->now = tioc_find(tip, iocp->ioc_cmd);
		if (!tip->now) {
			putnext(q, mp);
			return(0);
		}
		tip->proc_type = tip->now->tioc_type;
		tip->uaddr = *(caddr_t *)mp->b_cont->b_rptr;
		/*
		 * process it
		 */
		switch (tip->proc_type) {
		case TTYPE_COPYIN:
		case TTYPE_COPYINOUT:
		{
			/*
			 * these ioctl's require a COPYIN
			 */
			register struct copyreq *_cqp;

			mp->b_datap->db_type = M_COPYIN;
			_cqp = (struct copyreq *)mp->b_rptr;
			_cqp->cq_size = tip->now->tioc_size;
			_cqp->cq_addr = tip->uaddr;
			_cqp->cq_flag = 0;
			_cqp->cq_private = 0;
			mp->b_wptr = (unsigned char *)(_cqp + 1);
			freemsg(mp->b_cont);
			mp->b_cont = 0;
			qreply(q, mp);
		}
			break;
		case TTYPE_COPYOUT:
			/*
			 * these ioctl's will require a COPYOUT
			 * (to be done later)
			 * but we have to create the space for the module
			 * to put it's data in
			 */
			/*
			 * throw away second message only if it is not big
			 * enough to accomodate data. Track number 140355.
			 */
			if (mp->b_cont &&
			    (mp->b_cont->b_datap->db_lim -
			     mp->b_cont->b_datap->db_base - 1)
			    < tip->now->tioc_size) {
				freemsg(mp->b_cont);
				mp->b_cont = 0;
			}
			/*
			 * allocate a new message only if we did not find one
			 * big enough previously. Track number 140355.
			 */
			if (!mp->b_cont)
				mp->b_cont = allocb(tip->now->tioc_size,
						    BPRI_MED);
			if (!mp->b_cont) {
				NAK_IT(mp, ENOMEM);
				tip->now = 0;
				qreply(q, mp);
				return(0);
			}
			iocp->ioc_count = tip->now->tioc_size;
			mp->b_cont->b_wptr = mp->b_cont->b_rptr 
						+ iocp->ioc_count;
			putnext(q, mp);
			break;
		case TTYPE_NOCOPY:
			/*
			 * these ioctl's don't require any copies of data,
			 * but the modules and drivers below don't like
			 * to see the size as TRANSPARENT
			 */
			tip->now = 0;	/* we can forget about it */
			freemsg(mp->b_cont);
			mp->b_cont = 0;
			iocp->ioc_count = 0;
			putnext(q, mp);
			break;
		case TTYPE_IMMEDIATE:
			/*
			 * thest ioctl's use the immediate value of the
			 * third argument
			 */
			tip->now = 0;	/* we can forget about it */
			iocp->ioc_count = sizeof(int);
			putnext(q, mp);
			break;
		default:
			/*
			 * it's not one that we know
			 */
			tip->now = 0;
			putnext(q, mp);
			break;
		}
	}
		break;
	case M_IOCDATA: {
		register struct copyresp *csp;
		register struct iocblk *iocp;

		/*
		 * if we didn't do anything to it, leave it alone
		 */
		if (!tip->now) {
			putnext(q, mp);
			return(0);
		}
		csp = (struct copyresp *)mp->b_rptr;
		/*
		 * if it's not ours, leave it alone
		 */
		if (csp->cp_cmd != tip->now->tioc_cmd) {
			putnext(q, mp);
			return(0);
		}
		/*
		 * copy wasn't successful
		 */
		if (csp->cp_rval) {
			NAK_IT(mp, csp->cp_rval);
			tip->now = 0;
			qreply(q, mp);
			return(0);
		}
		/*
		 * we want to avoid blocking an ioctl that is no acked now
		 * by a module downstream if that module need to do an
		 * another copy withthe same ioctl before acking it, see
		 * ioctl's TCGMAP and TCSMAP in nls module for more
		 * explanation. For that we used the cp_private field in the
		 * the copyresp structure, because that field is not used by
		 * anybody.
		 */
		if (!(csp->cp_private))
			csp->cp_private = mp;
		else {
			csp->cp_private = mp;
			putnext(q, mp);
			return(0);
		}
		switch (tip->proc_type) {
		case TTYPE_COPYIN:
		case TTYPE_COPYINOUT:
			/*
			 * data was copied in
			 * we can now send it to the module as a M_IOCTL
			 */
			mp->b_datap->db_type = M_IOCTL;
			iocp = (struct iocblk *)mp->b_rptr;
			iocp->ioc_count = tip->now->tioc_size;
			putnext(q, mp);
			break;
		case TTYPE_COPYOUT:
			/*
			 * data was copied out
			 */
			mp->b_datap->db_type = M_IOCACK;
			iocp = (struct iocblk *)mp->b_rptr;
			iocp->ioc_error = 0;
			iocp->ioc_rval = 0;
			iocp->ioc_count = 0;
			tip->now = 0;
			qreply(q, mp);
			break;
		default:
			/*
			 * something is severly wrong!
			 */
			NAK_IT(mp, EINVAL);
			tip->now = 0;
			qreply(q, mp);
			break;
		}
	}
		break;
	default:
		putnext(q, mp);
		break;
	}
	return(0);
}

/*
 * add a ioctl command to the hashed list of ioctl's
 */
static void
tioc_listadd(tip, trp)
	register struct tioc_s *tip;
	register struct tioc_reply *trp;
{
	register struct tioc_list *tlp;
	
	if (tioc_find(tip, trp->tioc_cmd))
		return;
	tlp = (struct tioc_list *)he_alloc(sizeof(struct tioc_list), BPRI_MED);
	if (!tlp) {
		tioc_printf("tioc_listadd: not enough memory! (%x)\n",
			trp->tioc_cmd);
		return;
	}
	tlp->next = tip->lists[trp->tioc_cmd & TIOC_HASH];
	tip->lists[trp->tioc_cmd & TIOC_HASH] = tlp;
	tlp->data = *trp;
}

/*
 * return a pointer to the tioc_reply structure for the cmd
 */
static struct tioc_reply *
tioc_find(tip, cmd)
	register struct tioc_s *tip;
	register int cmd;
{
	register struct tioc_list *tlp;

	tlp = tip->lists[cmd & TIOC_HASH];
	while (tlp) {
		if (tlp->data.tioc_cmd == cmd)
			return(&tlp->data);
		tlp = tlp->next;
	}
	return(0);
}

/*
 * Next part for the perf track number 140355.
 */
/*
 * read queue filter function.
 * queue is only interested in IOCNAK, IOCACK, and
 * CTL messages.
 */

static int
tioc_is_r_interesting(mblk_t *mp)
{
        if (mp->b_datap->db_type == M_DATA)
                /* fast path for data messages */
                return 0;
        else if (mp->b_datap->db_type == M_IOCNAK ||
                 mp->b_datap->db_type == M_IOCACK ||
                 mp->b_datap->db_type == M_LETSPLAY ||
                 mp->b_datap->db_type == M_CTL)
                return 1;
        else
                return 0;
}

/*
 * write queue filter function.
 * queue is only interested in IOCTL and IOCDATA
 * messages.
 */

static int
tioc_is_w_interesting(mblk_t *mp)
{
        if (mp->b_datap->db_type == M_DATA)
                /* fast path for data messages */
                return 0;
        else if (mp->b_datap->db_type == M_IOCTL ||
                 mp->b_datap->db_type == M_IOCDATA)
                return 1;
        else
                return 0;
}

int
tioc_sendctl(q, command, size)
        register queue_t *q;
        register int command;
        register int size;
{
        register mblk_t *mp;

        mp = allocb(sizeof(struct iocblk), BPRI_MED);
        if (!mp)
                return(1);
        else {
                mblk_t *mp1;
                struct iocblk *iocp;

                mp1 = allocb(size, BPRI_MED);
                if (!mp1) {
                        freemsg(mp);
                        return(1);
                }
                mp->b_datap->db_type = M_CTL;
                mp->b_cont = mp1;
                mp->b_wptr = mp->b_rptr + sizeof(struct iocblk);
                iocp = (struct iocblk *)mp->b_rptr;
                iocp->ioc_cmd = command;
                iocp->ioc_count = size;
                putnext(q, mp);
        }
        return(0);
}
