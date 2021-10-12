#ifndef lint
static char sccsid[] = "@(#)59	1.100  src/bos/kernext/tty/ldtty.c, sysxldterm, bos41J, 9525F_all 6/21/95 15:47:13";
#endif
/*
 * COMPONENT_NAME: (sysxtty) Line discipline streams module
 *
 * FUNCTIONS: ldtty_config, ldtty_close, ldtty_rput, ldtty_rsrv,
 *            ldtty_wput, ldtty_wsrv, ldtty_readdata, ldtty_ioctl,
 *            ldtty_ioctl_ack, ldtty_need_flush, ldtioc_sizeof,
 *            ldtty_pend, ldtty_input, ldtty_flush_shead, ldtty_flush,
 *            ldtty_flush_mp, ldtty_echo, ldtty_rub, ldtty_rubo,
 *            ldtty_putc, ldtty_output, ldtty_start, ldtty_retype,
 *            ldtty_wakeup, ldtty_intimeout, ldtty_scanc, ldtty_canputnext
 *            ldtty_write, ldtty_tblock, ldtty_sendcanon, ldtty_sendraw,
 *            ldtty_mnotify, ldtty_info, ldtty_putstr, ldtty_putint,
 *            ldtty_break, ldtty_mhangup, ldtty_mctl, ldtty_sendctl,
 *            ldtty_b_to_m, ldtty_stuffc, ldtty_unstuffc, ldtty_post_input,
 *            ldtty_swinsz, ldtty_sti, ldtty_copymsg, ldtty_getoutbuf,
 *            ldtty_ioctl_bad, ldtty_error, ldtty_gwinsz, ldtty_fastinput
 *
 * ORIGINS: 40, 71, 83
 *
 */
/************************************************************************
 *									*
 *			Copyright (c) 1991 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any	other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or	reliability	*
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
#include <sys/malloc.h>
#include <sys/conf.h>
#include <sys/errno.h>
#include <sys/errids.h>		/* errsave() and ERRID_TTY_x definitions */
#include <sys/user.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/file.h>
#include <sys/sleep.h>	/* for EVENT_SIG and EVENT_SIGRET definitions	*/
#include <sys/li.h>	/* for LI_GETTBC and LI_SETTBC definitions	*/
#include <sys/uio.h>
#include <sys/sysinfo.h>        /* tty sysinfo fields for sar and iostat */


#include <sys/stropts.h>
#include <sys/device.h>
#include <sys/devinfo.h>	/* struct devinfo for IOCINFO ioctl */
#include <sys/str_tty.h>
#include <sys/ttydefaults.h>
#include "ldtty.h"

#undef time

/* Additional define for posix line discipline 	*/
/* TXGETLD ioctl is used in aix ksh source	*/
#define	POSIX_LINE_DISCIPLINE	"posix"

#ifdef LDTTY_DEBUG
#define	ldtty_printf	printf
#else
#define	ldtty_printf
#endif

#define MODULE_ID	7701
#define MODULE_NAME	"ldterm"
#define M_HIWAT		512
#define M_LOWAT		128
 int ldtty_rput(), ldtty_rsrv(), ldtty_open(), ldtty_close();
 int ldtty_wput(), ldtty_wsrv();

#define	staticf 

static struct module_info minfo = {
	MODULE_ID, MODULE_NAME, 0, OBUFSIZ, M_HIWAT, M_LOWAT
};

static struct qinit rinit = {
	ldtty_rput, ldtty_rsrv, ldtty_open, ldtty_close, 0, &minfo, 0
};

static struct qinit winit = {
	ldtty_wput, ldtty_wsrv, 0, 0, 0, &minfo, 0
};

struct streamtab ldttyinfo = { &rinit, &winit };

#include <sys/sysconfig.h>
#include <sys/strconf.h>

#define	CCEQ(val, c)	(c == val ? val != _POSIX_VDISABLE : 0)

#define	flag_null	0x00000000

static	int	ldterm_count = 0;	/* config method loads count	*/

#define	TRC_LDTERM(w)	((HKWD_STTY_LDTERM)|w)

lock_t	ldterm_conf_lock = LOCK_AVAIL;	/* module ldterm configuration lock */

/*
 * Declarations for debugging, lldb and crash.
 */
extern	int	ldtty_print();
#ifdef	TTYDBG
/*
 * ldterm_dbg is passed, at ldterm configuration time, to the ttydbg extension.
 * That is to register the module as a new tty module loaded.
 * The same structure is passed at unconfiguration time, to the ttydbg extension
 * to unregister the module from the ttydbg module table.
 */
struct	str_module_conf ldterm_dbg = {
    MODULE_NAME, 'l', (int (*)())LDTERM_PDPF
};

#endif	/* TTYDBG */

int
ldtty_config(cmd, uiop)
	int	cmd;
	struct	uio	*uiop;
{
	int	error, locked;
	struct	ldterm_dds	init_ldterm_dds;

	static	strconf_t	conf = {
		MODULE_NAME, &ldttyinfo, (STR_NEW_OPEN|STR_MPSAFE),
	};
        Enter(TRC_LDTERM(TTY_CONFIG), 0, 0, cmd, 0, 0);

	locked = lockl(&ldterm_conf_lock, LOCK_SHORT);
	conf.sc_sqlevel = SQLVL_QUEUEPAIR;
	error = 0;
	switch (cmd) {
	case CFG_INIT: 
		if (uiop) {	/* called by configuration routine	*/
			if (uiomove((char *)&init_ldterm_dds,
				    sizeof(struct ldterm_dds), UIO_WRITE, uiop) ||
			   (init_ldterm_dds.which_dds != LDTERM_DDS))
				break;
			else {
				if (ldterm_count == 0) {
#ifdef	TTYDBG
					tty_db_register(&ldterm_dbg);
#endif	/* TTYDBG */
					error =str_install(STR_LOAD_MOD, &conf);
				}
				if (!error)
					ldterm_count++;
			}
		} 
		else {
#ifdef	TTYDBG
			tty_db_register(&ldterm_dbg);
#endif	/* TTYDBG */
			error = str_install(STR_LOAD_MOD, &conf);	
		}
		break;
	case CFG_TERM: 
		if (uiop) {
			if (uiomove((char *)&init_ldterm_dds,
				    sizeof(struct ldterm_dds), UIO_WRITE, uiop) ||
			   (init_ldterm_dds.which_dds != LDTERM_DDS))
				break;
			else {
				if (ldterm_count == 1) {
#ifdef	TTYDBG
					tty_db_unregister(&ldterm_dbg);
#endif	/* TTYDBG */
					error = str_install(STR_UNLOAD_MOD, 
								&conf);
				}
				if (!error)
					ldterm_count--;
			}
		}
		else {
#ifdef	TTYDBG
			tty_db_unregister(&ldterm_dbg);
#endif	/* TTYDBG */
			error = str_install(STR_UNLOAD_MOD, &conf);
		}
		break;
	default: 
		error = EINVAL;
		break;
	}
	if (locked != LOCK_NEST)
		unlockl(&ldterm_conf_lock);
	Return(error);
}

/*
 * module specific structures/definitions
 */
/*
 * functions in this file
 */
int ldtty_ioctl();
int ldtty_ioctl_ack();
int ldtty_ioctl_bad();
int ldtty_need_flush();
void ldtty_flush_mp();
int ldtty_flush();
int ldtty_flush_shead();
int ldtty_input();
void ldtty_echo(), ldtty_retype();
void ldtty_wakeup(), ldtty_info();
void ldtty_rub(), ldtty_rubo();
int ldtty_output();
void ldtty_pend();
int ldtty_start();
int ldtty_write(), ldtty_putc(), ldtty_canputnext();
int ldtty_mctl();
mblk_t * ldtty_sendioctl();
int ldtty_sendctl();
void ldtty_tblock(), ldtty_sendraw(), ldtty_post_input();
int ldtty_b_to_m();
int ldtty_stuffc(), ldtty_unstuffc();
mblk_t * ldtty_readdata();
void ldtty_break(), ldtty_sendcanon();
int ldtty_swinsz();
void ldtty_sti();
void ldtty_copymsg(), ldtty_mnotify();
int ldtty_getoutbuf(), ldtty_mhangup(), ldtty_scanc();

void	ldtty_error();

/*
 * these functions are in ldtty_compat.c
 */
/* COMPAT_BSD_4.3	*/
extern void	ldtty_bsd43_ioctl();
/* COMPAT_BSD_4.3	*/
extern void	ldtty_svid_ioctl();
extern int	ldtty_speedtab();
extern int	ldtty_compatgetflags();
extern void	ldtty_compatsetflags();
extern void	ldtty_compatsetlflags();
extern staticf int	ldtty_compat_to_termios();

/*
 * these functions are in ldtty_euc.c
 */
extern	int	euctty_state();
extern	int	euctty_rocount();
extern	int	euctty_scrwidth();
extern	void	euctty_echo();
extern	void	euctty_rub();
extern	void	euctty_erase();
extern	void	euctty_kill();
extern	void	euctty_werase();
extern	int	euctty_write();
extern	int	euctty_writechar();

/*
 * open
 */
 int
ldtty_open(q, devp, flag, sflag, credp)
	register queue_t *q;
	register dev_t *devp;
	register int flag;
	register int sflag;
        cred_t *credp;
{
	register struct ldtty *tp;
	register mblk_t *mp;
	register int	i;
	register struct stroptions *sop;
	ulong	open_flag = 0;
#ifdef	TTYDBG
	static	struct tty_to_reg ldtty_db; 
#endif	/* TTYDBG */
        Enter(TRC_LDTERM(TTY_OPEN), *devp, (int)q->q_ptr, flag, sflag, 0);

	if (q->q_ptr) {
		tp = (struct ldtty *)q->q_ptr;
		if (tp->t_state & TS_ISOPEN) {	/* this is a reopen	*/
			Return(0);
		}
	}
	else {
		/*
		 * first open
		 */
		tp = (struct ldtty *)he_alloc(sizeof(struct ldtty), BPRI_MED);
		if (!tp) 
			Return(ENOMEM);
		bzero(tp, sizeof(struct ldtty));
		tp->t_queue = q;

#ifdef	TTYDBG
		ldtty_db.dev = *devp;
		ldtty_db.ttyname[0] = '\0';
		bcopy(MODULE_NAME, &ldtty_db.name, sizeof(MODULE_NAME));
		ldtty_db.private_data = tp;
		tty_db_open(&ldtty_db);
#endif	/* TTYDBG */

		ldtty_bufclr(tp);

		/*
	 	 * Set values for input and output buffers limits.
	 	 */
		if (tp->t_ihog < LDTTYMAX * 2)
			tp->t_ihog = LDTTYMAX * 2;
		if (tp->t_ohog < LDTTYMAX * 2)
			tp->t_ohog = LDTTYMAX * 2;
		tp->t_outbuf = allocb(tp->t_ohog, BPRI_MED);
		if (!tp->t_outbuf) {
			he_free(tp);
			Return(ENOMEM);
		}
		mp = allocb(sizeof(struct stroptions), BPRI_MED);
		if (!mp) {
			freemsg(tp->t_outbuf);
			he_free(tp);
			Return(ENOMEM);
		}
		/*
		 * send the M_SETOPS to the stream head
		 */
		mp->b_datap->db_type = M_SETOPTS;
		sop = (struct stroptions *)mp->b_rptr;
		mp->b_wptr =  mp->b_rptr + sizeof(struct stroptions);
		sop->so_flags = SO_READOPT | SO_MREADOFF | SO_NDELON | 
								SO_ISTTY;
		sop->so_readopt = RMSGN;
		putnext(q, mp);
	}
	/*
	 * default values for contents of ldtty
	 */
	tp->t_dev = *devp;
	if ((tp->t_state & TS_ISOPEN) == 0) {
		tp->t_state |= TS_ISOPEN;
		bzero(&tp->t_winsize, sizeof(struct winsize));
	}

	/*
	 * Record the value of O_NDELAY or O_NONBLOCK in tp->t_state.
	 */
	if (flag & (O_NDELAY|O_NONBLOCK))
		tp->t_state |= TS_ONDELAY;
	else
		tp->t_state &= ~TS_ONDELAY;

	/*
	 * Defect number 148029. Locks are replaced by atomic operations.
	 */
	tp->t_intimeout_posted = 0;
	/*
	 * initialization for flags in termios that are treated by the drivers.
	 * oflag, iflag and lflag are initialized to NULL, they could change
	 * in a case of an M_CTL, MC_PART_CANON message from the driver.
	 */
	tp->d_termios.c_oflag = flag_null;
	tp->d_termios.c_iflag = flag_null;
	tp->d_termios.c_lflag = flag_null;
	tp->d_termios.c_cflag = flag_null;

	tp->t_iflag = tp->u_termios.c_iflag = TTYDEF_IFLAG;
	tp->t_oflag = tp->u_termios.c_oflag = TTYDEF_OFLAG;
	tp->t_lflag = tp->u_termios.c_lflag = TTYDEF_LFLAG;
	tp->t_cflag = tp->u_termios.c_cflag = TTYDEF_CFLAG;
	tp->t_control.x_hflag = 0;
	tp->t_control.x_sflag = 0;
	tp->t_cswidth.eucw[0] = 1;
	tp->t_cswidth.eucw[1] = 1;
	tp->t_cswidth.eucw[2] = 0;
	tp->t_cswidth.eucw[3] = 0;
	tp->t_cswidth.scrw[0] = 1;
	tp->t_cswidth.scrw[1] = 1;
	tp->t_cswidth.scrw[2] = 0;
	tp->t_cswidth.scrw[3] = 0;
	tp->t_eucleft = 0;
	tp->t_out_eucleft = 0;
	bcopy((caddr_t)ttydefchars, (caddr_t)tp->t_cc, sizeof(tp->t_cc));
	bcopy((caddr_t)ttydefchars, (caddr_t)tp->u_termios.c_cc, 
		sizeof(tp->u_termios.c_cc));
	/*
	 * defect number 128266, t_vmin and t_vtime added for compatibility 
	 * problems beetween TIOCSETP or TIOCSETN and TCSETS ot TCSETSF or
	 * TCSETSW. Patch part 1 BEGIN.
	 */
	tp->t_vmin = tp->t_cc[VMIN];
	tp->t_vtime = tp->t_cc[VTIME];
	/*
	 * 128266: patch part 1 END.
	 */

	tp->t_shad_time = tp->t_cc[VTIME] * hz / 10;
	tp->t_ioctl_cmd = 0;
	tp->t_ioctl_data = 0;
	tp->t_event = EVENT_NULL;

	/*
	 * set up private queue data
	 */
	q->q_ptr = (char *)tp;
	WR(q)->q_ptr = (char *)tp;

	/*
	 * send M_CTL to driver to ask for termios
	 * send M_CTL MC_CANONQUERY to ask to intelligent hardware what are
	 * there responsible for.
	 * send M_CTL to driver to ask for tty_control
	 * send M_CTL to driver to ask for the tty name.
	 * Defect number 151862.
	 */
	mp = 0;
	tp->t_on_open = 1;
	if (ldtty_sendctl(WR(q), TIOCGETA, sizeof(struct termios)) ||
	    ldtty_sendctl(WR(q), MC_CANONQUERY, sizeof(struct termios)) ||
	    ldtty_sendctl(WR(q), TCGETX, sizeof(struct termiox)) ||
	    ldtty_sendctl(WR(q), TXTTYNAME, TTNAMEMAX) ||
	    !(mp = ldtty_sendioctl(TIOCSETA, sizeof(struct termios))) ||
	    e_sleep(&tp->t_event, EVENT_SIGRET) == EVENT_SIG) {
		/* One of the ldtty_sendctl() calls failed.  We fail
		 * with ENOMEM, after cleaning up any other data we
		 * allocated.
		 */
		q->q_ptr = WR(q)->q_ptr = 0;
		freemsg(tp->t_outbuf);
		if (mp)
		    freemsg(mp);
#ifdef	TTYDBG
		ldtty_db.dev = tp->t_dev;
		ldtty_db.ttyname[0] = '\0';
		bcopy(MODULE_NAME, &ldtty_db.name, sizeof(MODULE_NAME));
		ldtty_db.private_data = tp;
		tty_db_close(&ldtty_db);
#endif	/* TTYDBG */
		he_free(tp);
		Return(ENOMEM);
	}

	/*
	 * We allocated stuff to send a TIOCSETA down and so we now do
	 * this and send the user's view down to make sure that they
	 * are set everywhere and everything like readmode and other
	 * things are totally set up properly.
	 */
	bcopy(&tp->u_termios, mp->b_cont->b_rptr, sizeof(struct termios));
	mp->b_cont->b_wptr = mp->b_cont->b_rptr + sizeof(struct termios);
	putnext(WR(q), mp);
	Return(0);
}

/*
 * close
 */
int
ldtty_close(q, flag, credp)
     register queue_t *q;
     int flag;
     cred_t *credp;
{
	register struct ldtty *tp = (struct ldtty *)q->q_ptr;
	int error = 0;
        mblk_t *mp;
        mblk_t *mp_ioctl;
#ifdef	TTYDBG
	static	struct	tty_to_reg	ldtty_db;
#endif	/* TTYDBG */
        Enter(TRC_LDTERM(TTY_CLOSE), tp->t_dev, (int)tp, flag, 0, 0);

	tp->t_state |= TS_CLOSING;
	ldtty_flush(tp, FREAD, 0);

        /*
         * Undo stream head changes we did when first opened
         */
        mp = allocb(sizeof(struct stroptions), BPRI_MED);
        if (mp) {
                struct stroptions *sop;

		/*
		 * send the M_SETOPS to the stream head
		 */
		mp->b_datap->db_type = M_SETOPTS;
		sop = (struct stroptions *)mp->b_rptr;
		mp->b_wptr =  mp->b_rptr + sizeof(struct stroptions);
		sop->so_flags = SO_READOPT | SO_TONSTOP |
				SO_MREADOFF | SO_NDELOFF | SO_ISNTTY;
		sop->so_readopt = RNORM;
		putnext(q, mp);
	}

	/*
	 * one more chance to drain output if needed
	 */
        if (tp->t_state & TS_TTSTOP) {
                tp->t_state &= ~TS_TTSTOP;
                (void) putctl(WR(tp->t_queue)->q_next, M_START);
        }

        while (ldtty_msgdsize(tp->t_outbuf) || WR(tp->t_queue)->q_first) {
                /* One last chance for output to drain */
                tp->t_state &= ~TS_WAITOUTPUT;
                if (tp->t_state & TS_WAITOUTBUF) {
                        /*
                         * Some previous call to allocb() for
                         * t_outmsg failed.
                         */
                        tp->t_state &= ~TS_WAITOUTBUF;
			if (tp->t_outbid) {
                        	unbufcall(tp->t_outbid);
				tp->t_outbid = 0;
			}

                        if (!ldtty_getoutbuf(tp))
                                goto slp;
                }

                if (ldtty_start(tp) || WR(tp->t_queue)->q_first) {
slp:
                        if (flag & (FNDELAY|FNONBLOCK)) {
                                break;
			}
			
			tp->t_state |= TS_ASLEEP;
			error = e_sleep(&tp->t_event, EVENT_SIGRET);
			tp->t_state &= ~TS_ASLEEP;
			if (error == EVENT_SIG) {
				error = EINTR;
				break;
			}
                }
        }

	ldtty_flush(tp, FREAD|FWRITE, 0);
	/* XXX - some more processing might be required here */
	/* XXX - unbufcall any pending bufcalls */
	/*
	 * free struct ldtty and stuff inside it
	 */
	if (tp->t_intimeout)
		ldtty_unset_intimeout(tp);
	if (tp->t_outbid)
		unbufcall(tp->t_outbid);
	if (tp->t_ackbid)
		unbufcall(tp->t_ackbid);
	if (tp->t_hupbid)
		unbufcall(tp->t_hupbid);
	if (tp->t_rawbuf)
		freemsg(tp->t_rawbuf);
	if (tp->t_outbuf)
		freemsg(tp->t_outbuf);
	if (tp->t_sparebuf) {
		if (tp->t_sparebuf->b_datap)
			freemsg(tp->t_sparebuf);
		else
			tp->t_sparebuf = 0;
	}
#ifdef	TTYDBG
	ldtty_db.dev = tp->t_dev;
	ldtty_db.ttyname[0] = '\0';
	bcopy(MODULE_NAME, &ldtty_db.name, sizeof(MODULE_NAME));
	ldtty_db.private_data = tp;
	tty_db_close(&ldtty_db);
#endif	/* TTYDBG */
	tp->t_dev = 0;
	he_free(tp);
	q->q_ptr = 0;

	Return(error);
}


/*
 * Handle M_IOCNAKs.  Main purpose is to intercept M_IOCNAK messages
 * for TIOCGWINSZ and make them M_IOCACK's with tp->t_winsize in
 * mp->b_cont.
 */
void
ldtty_gwinsz(register struct ldtty *tp, register queue_t *q,
	     register mblk_t *mp) 
{
	register struct iocblk *iocp = (struct iocblk *) mp->b_rptr;

	/* Driver didn't fill in the information, so we will. */
	if (!mp->b_cont) {

		if (!(mp->b_cont = allocb(sizeof(struct winsize), BPRI_MED))) {

			iocp->ioc_error = ENOMEM;
			putnext(q, mp);
			return;
		}
	}
					
	mp->b_cont->b_datap->db_type = M_DATA; /* just to be safe */
	*(struct winsize *) mp->b_cont->b_rptr = tp->t_winsize;
	mp->b_cont->b_wptr = mp->b_cont->b_rptr + sizeof(struct winsize);
	ldtty_iocack_msg(iocp, sizeof(struct winsize), mp);
	putnext(q, mp);
}


/* Those that don't have anything from FASTIMASK and FASTLMASK set can
 * run in interrupt level.  If they also have none of the items in
 * FASTERIMASK and FASTERLMASK set, we can go really fast.  Otherwise
 * we just run them through ldtty_readdata() in interrupt level.
 */
#define FASTIMASK   (ISTRIP|IGNCR|ICRNL|INLCR|IUCLC|PARMRK)
#define FASTLMASK   (ICANON|ECHO|ECHONL)
#define FASTERIMASK (IXOFF|IXON|IXANY)
#define FASTERLMASK (ISIG)

#define FASTINPUT_SUCCESS 0
#define FASTINPUT_RUN     1
#define FASTINPUT_QUEUE   2
/*
 * Fast input path.  If certain conditions are met, we can just put
 * the message we are given onto the end of tp->t_rawtail and wake
 * folks up appropriately.
 *
 * Return values:
 *
 * FASTINPUT_SUCCESS:  Successful
 * FASTINPUT_RUN:      Can't do the fast path, but can run at interrupt
 *                     level.  Caller should call ldtty_readdata().
 * FASTINPUT_QUEUE:    Can't run at interrupt level.  Caller should put mp on
 *                     tp->t_queue.
 *
 */
int
ldtty_fastinput(struct ldtty *tp, mblk_t *mp)
{
	register mblk_t	*rawtail_mp;
	register mblk_t *mp1;
	int msize;

	ASSERT(mp != mp->b_cont);
	if (tp->t_rawtail) {
		ASSERT(tp->t_rawtail->b_cont == 0);
		ASSERT(tp->t_rawtail->b_cont != tp->t_rawtail);
	}
	ASSERT(mp != tp->t_rawtail);
	/* Check immediately to see if we can do the fast path.  If
	 * not, return 1.
	 */
	if ((tp->t_iflag & FASTIMASK) || (tp->t_lflag & FASTLMASK))
		return FASTINPUT_QUEUE;

	if (tp->t_rawcc >= tp->t_ihog) {

		tp->t_state |= TS_RAWBACKUP;

		/* Should we check IMAXBEL here??? */

		return FASTINPUT_QUEUE;
	}

	if ((tp->t_iflag & FASTERIMASK) || (tp->t_lflag & FASTERLMASK)) {

		/* Must run through ldtty_readdata. */
		return FASTINPUT_RUN;
	}


	/* Okay, we can do the fast path, just append the message to
	 * tp->t_rawtail and update tp->t_rawtail, tp->t_rawcc, and
	 * statistics.
	 */

	if (!tp->t_rawtail) {

		/* No "raw" message blocks yet, so just make mp the
		 * new raw buffer.
		 */
		tp->t_rawbuf = tp->t_unsent = mp;
		tp->t_unsent_ndx = mp->b_rptr - mp->b_datap->db_base;
		tp->t_rawcc = 0;

	} else {

		/* We already have a raw buffer, so just tack our
		 * message on the end.
		 */
		tp->t_rawtail->b_cont = mp;
	}

	/* update tp->t_rawtail */
	for (rawtail_mp = mp;
	     rawtail_mp->b_cont;
	     rawtail_mp = rawtail_mp->b_cont) 
		ASSERT(rawtail_mp != rawtail_mp->b_cont);
	tp->t_rawtail = rawtail_mp;
	ASSERT(tp->t_rawtail->b_cont == 0);

	/* Now update counters. */
	tp->t_rawcc += (msize = msgdsize(mp));
	sysinfo_add(sysinfo.rawch, msize);

	if (tp->t_rawcc >= tp->t_vmin) {

		ldtty_wakeup(tp);

	} else {

                if ((tp->t_shad_time > 0) &&         /* VTIME > 0 */
                    (tp->t_state & TS_VTIME_FLAG)) { /* and read issued */
			ldtty_set_intimeout(tp, tp->t_shad_time);
                }
	}

	return FASTINPUT_SUCCESS;
}


/*
 * read side should process the characters coming in from the terminal
 * and other miscellaneous messages
 */
 int
ldtty_rput(q, mp)
	register queue_t *q;
	register mblk_t *mp;
{
	register struct ldtty *tp = (struct ldtty *)q->q_ptr;
        Enter(TRC_LDTERM(TTY_RPUT), tp->t_dev, (int)tp, mp,
	      mp->b_datap->db_type, 0);

	switch (mp->b_datap->db_type) {
	case M_FLUSH:
		if (*mp->b_rptr & FLUSHR)
			ldtty_flush(tp, FREAD, 0);
		putnext(q, mp);
		break;
        case M_DATA:
		if (tp->t_state & TS_CLOSING) {
		    freemsg(mp);
		    break;
		}
                if (tp->t_state & TS_NOCANON) {
			if (canput(q->q_next))
                       		putnext(q, mp);
			else
				putq(q, mp);
		} else {
			mp->b_flag |= MSGCOMPRESS;
			if (canput(q->q_next)) {
				if ((tp->t_state & TS_RAWBACKUP) ||
				    q->q_first) {
				/*
				 * mp may have advanced from where it pointed
				 * a few lines above, so set MSGCOMPRESS bit
				 * again.
				 */
					mp->b_flag |= MSGCOMPRESS;
					putq(q, mp);

				} else switch (ldtty_fastinput(tp, mp)) {

				case FASTINPUT_RUN:
					if (!(mp = ldtty_readdata(tp, mp)))
						break;

					/* FALLSTHROUGH */

				case FASTINPUT_QUEUE:
					mp->b_flag |= MSGCOMPRESS;
					putq(q, mp);
					break;

				case FASTINPUT_SUCCESS:
				default:
					break;
				}
			} else
				putq(q, mp);
		}
                break;
        case M_IOCACK:
                if (!ldtty_ioctl_ack(tp, q, mp)) {
			/* Out of memory -- we have to change this
			 * to a normal priority message to kepp
			 * q from getting instantly enabled. Service
			 * routine knows to undo this. M_IOCTL is
			 * suitable as the temporary type since
			 * M_IOCTL's never come from downstream
			 * (per stream spec).
			 */
			mp->b_datap->db_type = M_IOCTL;
			putbq(q, mp);
		}
                break;
        case M_CTL:
                if (!ldtty_mctl(tp, q, mp))
			putbq(q, mp);
                break;
        case M_BREAK:
                ldtty_break(tp, mp);
                break;
	case M_HANGUP:
		if (!ldtty_mhangup(tp, q, mp)) {
			/* Hi-pri, so putq would qenable immediately.
			 * We'll just send the message along without
			 * the M_ERROR that ldtty_hangup was to insert.
			 * The effect at the stream head will be that
			 * write(2)'s get ENXIO instead of ENIO - seems
			 * preferable to delaying the hangup condition
			 * indefinitely.
			 */
			putnext(tp->t_queue, mp);
		}
		break;

	case M_IOCNAK:
		if (((struct iocblk *)mp->b_rptr)->ioc_cmd != TIOCGWINSZ) {
			putnext(q, mp);
		} else {
			ldtty_gwinsz(tp, q, mp);
		}
		break;

	case M_LETSPLAY:
		++(*(int *)mp->b_rptr);
		putnext(q, mp);
		break;
        default:
		if ((mp->b_datap->db_type >= QPCTL) || canput(q->q_next))
                	putnext(q, mp);
		else
			putq(q, mp);
                break;
        }
	Return(0);
}

int
ldtty_rsrv(q)
	register queue_t *q;
{
	register mblk_t *mp;
        struct ldtty    *tp = (struct ldtty *) q->q_ptr;
	int		need_wakeup = 0;
        Enter(TRC_LDTERM(TTY_RSRV), tp->t_dev, (int)tp, q->q_count, 0, 0);

	ldtty_check_intimeout(tp, &need_wakeup);
	if (need_wakeup)
		ldtty_wakeup(tp);
	if ((tp->t_state & TS_TBLOCK) && (tp->t_lflag & ICANON) &&
	    (tp->t_iflag & IXOFF))
		ldtty_tblock(tp);

        while (mp = getq(q)) {
                switch(mp->b_datap->db_type) {
                case M_DATA:
			if (tp->t_state & TS_CLOSING) {
				freemsg(mp);
				break;
			}
			if (tp->t_state & TS_NOCANON) {

				if (canput(q->q_next)) {

					putnext(q, mp);
					break;

				} else {

					putbq(q, mp);
					Return(0);
				}
			}
					
			if (canput(q->q_next)) {
                        	if (mp = ldtty_readdata(tp, mp)) {
                                	tp->t_state |= TS_RAWBACKUP;
                                	putbq(q, mp);
					Return(0);
				}
			} else {
				putbq(q, mp);
				Return(0);
                        }
                        break;
                case M_IOCTL:
                        /* This is actually an M_IOCACK -- see note
                         * in ldtty_rput().
                         */
                        mp->b_datap->db_type = M_IOCACK;
                        if (!ldtty_ioctl_ack(tp, q, mp)) {
                                mp->b_datap->db_type = M_IOCTL;
                                putbq(q, mp);
				Return(0);
                        }
                        break;
                case M_CTL:
                        if (!ldtty_mctl(tp, q, mp)) {
                                putbq(q, mp);
				Return(0);
                        }
                        break;

		case M_IOCNAK:
			if (((struct iocblk *)mp->b_rptr)->ioc_cmd !=
			    TIOCGWINSZ) {
				putnext(q, mp);
			} else {
				ldtty_gwinsz(tp, q, mp);
			}
			break;
			
                default:
                        ASSERT(mp->b_datap->db_type < QPCTL);
                        if (canput(q->q_next))
                                putnext(q, mp);
                        else {
                                putbq(q, mp);
				Return(0);
                        }
                }
        }
	Return(0);
}

/*
 * Passed the write queue
 */
static int ldtty_backdone(queue_t *q, mblk_t *mp)
{
	queue_t *rq = OTHERQ(q);
	mblk_t *m2;
	mblk_t *m_list = 0;		/* possible IOCTL message to hold */
	struct ldtty *tp = (struct ldtty *)q->q_ptr;

	/* First, deal with the raw buffer */
	if (m2 = tp->t_rawbuf) {
		/* Trim the part that was at the stream head */
		assert(adjmsg(m2, tp->t_shcc));

		/* Clear raw buffer */
		ldtty_bufclr(tp);

		/* Send down as a BACKWASH message */
		m2->b_datap->db_type = M_BACKWASH;
		putnext(q, m2);
	}

	/* Now empty the read queue assuming we have only M_DATA messages */
	while (m2 = getq(rq)) {
		if (m2->b_datap->db_type == M_IOCTL) {
			assert(!m_list);
			m_list = m2;
			continue;
		}

		/* Just a sanity check for now */
		assert(m2->b_datap->db_type == M_DATA);
		m2->b_datap->db_type = M_BACKWASH;
		putnext(q, m2);
	}

	/*
	 * m_list is a message that needs to be put back on the read
	 * queue.
	 */
	if (m_list)
		putq(rq, m_list);

	/* Finally send down the M_BACKDONE message */
	putnext(q, mp);
	return 0;
}

/*
 * write side should process characters coming from the application
 * and ioctl's and other miscellaneous messages.
 */
int
ldtty_wput(q, mp)
	register queue_t *q;
	register mblk_t *mp;
{
	register struct ldtty *tp = (struct ldtty *)q->q_ptr;
        Enter(TRC_LDTERM(TTY_WPUT), tp->t_dev, (int)tp, mp,
	      mp->b_datap->db_type, 0);

	switch (mp->b_datap->db_type) {
	case M_BACKDONE:
		Return(ldtty_backdone(q, mp));

	case M_DATA:
		mp->b_flag |= MSGCOMPRESS;
		if (q->q_first)
			putq(q, mp);
		else
			(void)ldtty_write(tp, mp);
		break;
	case M_FLUSH:
		if (*mp->b_rptr & FLUSHW)
			ldtty_flush(tp, FWRITE, 0);
		putnext(q, mp);
		break;
	case M_READ:
		/*
		 * Cancel previous read timeout
		 */
		ldtty_unset_intimeout(tp);

		/*
		 * Check to see if we need to send up a setopts message.
		 */
		if (tp->t_state & TS_SETNEEDED) {
			mblk_t *mp;
			struct stroptions *sop;

			if (!(mp = allocb(sizeof(struct stroptions),
					  BPRI_MED))) {
				/* #### what do we do now? */
			} else {
				mp->b_datap->db_type = M_SETOPTS;
				sop = (struct stroptions *)mp->b_rptr;
				mp->b_wptr =  (mp->b_rptr +
					       sizeof(struct stroptions));

				if (tp->t_lflag & ICANON) {
					sop->so_readopt = RMSGN | RPROTNORM;
					sop->so_flags = SO_MREADOFF|SO_READOPT;
				} else {
					sop->so_readopt = RNORM ;
#ifdef RPROTCOMPRESS
					sop->so_readopt |= RPROTCOMPRESS;
#endif
					sop->so_flags = SO_MREADON | SO_READOPT;
				}
				putnext(RD(q), mp);
			}
			tp->t_state &= ~TS_SETNEEDED;
		}

		/*
		 * if input is pending take it first
		 */
		if (tp->t_lflag & PENDIN)
			ldtty_pend(tp);
		/*
		 * Rig up timeout or send raw data as appropriate if !ICANON.
		 * If the t_shcc is non-zero, then the M_READ must have been
		 * sent while data was in flight from the line discipline
		 * to the stream head. Just ignore the M_READ in this case:
		 * if the stream head needs more data after the in-flight data
		 * is consumed, it will ask again (with a new M_READ); if it
		 * doesn't, and if we don't ignore this M_READ, then we're at
		 * risk of sending a zero length M_DATA upstream when VMIN is 
		 * greater than zero.
		 */
		if (!(tp->t_lflag & ICANON) && (tp->t_shcc == 0)) {
			if (tp->t_shad_time > 0) {
				/*
				 * We're using VTIME
				 * Flag that a read is pending
				 */
				tp->t_state |= TS_VTIME_FLAG;
				/*
				 * 128266 cc[VMIN] replaced by t_vmin.
				 * Patch part 2 BEGIN.
				 */
				if (tp->t_rawcc || (tp->t_vmin == 0))
					/*
					 * Start the timer now
					 */
					ldtty_set_intimeout(tp, tp->t_shad_time);
			} else if ((tp->t_vmin == 0) &&
				(*(long *)mp->b_rptr != 0))
				ldtty_wakeup(tp);
				/*
				 * 128266: patch part 2 END.
				 */
		}
		/*
		 * Performance decision: don't send downstream the M_READ,
		 * because it is freed anyway by the driver. So, must be
		 * freed here.
		 * Performance improvement, track number 139885.
		 */
		freemsg(mp);
		break;
	case M_NOTIFY:
		if (tp->t_lflag & ICANON)
			freemsg(mp);
		else
			ldtty_mnotify(tp, mp);
		break;
	case M_IOCTL: {
		register struct iocblk *iocp;

		if (tp->t_qioctl) {
			rmvq(q, tp->t_qioctl);
			freemsg(tp->t_qioctl);
			tp->t_qioctl = (mblk_t *)NULL;
		}
		/*
		 * see if it is one that we recognize
		 * if not, pass the message along
		 * see if the size of the data is correct
		 * if not, NAK it
		 * if all ok, process it
		 */
		iocp = (struct iocblk *)mp->b_rptr;
		tp->t_ioctl_cmd = 0;
		iocp->ioc_error = 0;
		switch (iocp->ioc_cmd) {
/* Special AIX starts	*/
		case TXGETLD:
		case TXGETCD:
		case TCVPD:
		case TXSETIHOG:
		case TXSETOHOG:
/* AIX end */
/* specials for open and hard control flow */
		case TCGETX:
		case TCSETX:
		case TCSETXW:
		case TCSETXF:
/* End for specials			*/
		case IOCINFO:
		case TIOCSETD:
		case TIOCFLUSH:
		case TIOCSETA:
		case TIOCSETAW:
		case TIOCSETAF:
		case TIOCSWINSZ:
		case TIOCGETD:
		case TIOCOUTQ:
		case TIOCGETA:
		case TIOCSTI:
		case EUC_WSET:
		case EUC_WGET:
/* COMPAT_BSD_4.3	*/
		case TIOCGETP:
		case TIOCSETP:
		case TIOCSETN:
		case TIOCGETC:
		case TIOCSETC:
		case TIOCSLTC:
		case TIOCGLTC:
		case TIOCLBIS:
		case TIOCLBIC:
		case TIOCLSET:
		case TIOCLGET:
/* COMPAT_BSD_4.3	*/
/* SVID.4 starts */
		case TIOCMBIC:
		case TIOCMBIS:
		case TIOCMSET:
		case TIOCMGET:
/* SVID.4 ends */
/* SVID start */
		case TCXONC:
		case TCFLSH:
		case TCSETA:
		case TCSETAW:
		case TCSETAF:
		case TCGETA:
/* SVID end */	
			if (ldtty_ioctl_bad(mp)) {
				qreply(q,mp);
				Return(0);
			}
			break;
		case TIOCEXCL:
		case TIOCNXCL:
		case TIOCHPCL:
		case TIOCSTOP:
		case TIOCSTART:
		case TIOCSBRK:
		case TIOCCBRK:
		case TXSETLD:
		case TXADDCD:
		case TXDELCD:
			/*
			 * These may come down as TRANSPARENT ioctls
			 * Make sure ioc_count is zeroed for return
			 */
			iocp->ioc_count = 0;
			if (mp->b_cont) {
				freemsg(mp->b_cont);
				mp->b_cont = 0;
			}
			break;
		case TCSBRK:
		case TCSBREAK:
			break;
		default:
			putnext(q, mp);
			Return(0);
		}
		/*
		 * A few ioctls must be queued behind data messages so
		 * data will drain before the ioctls are executed.
		 */
		iocp->ioc_error = 0;
		switch (iocp->ioc_cmd) {
		case TCSETXW:
		case TCSETXF:
		case TIOCSETAW:
		case TIOCSETAF:
/* SVID start */
		case TCSETAW:
		case TCSETAF:
		case TCSBRK:
/* SVID end */
		case TCSBREAK:
		    	if (q->q_first) {
				tp->t_qioctl = mp;
				putq(q, mp);
				break;
			}
		/* else fall through */
		default:
			ldtty_ioctl(tp, q, mp);
		}
	}
		break;
	case M_CTL: {
		register struct iocblk *iocp;

		iocp = (struct iocblk *)mp->b_rptr;
		if (iocp->ioc_cmd == TIOCGETMODEM)
			putnext(q, mp);
		else
			ldtty_mctl(tp, q, mp);
		}
		break;
/*
 * Added cases about M_STOP, M_START, M_STOPI, M_STARTI.
 * Because M_IOCTL is not a high priority message type, some ioctls are
 * handled in the sreamhead for the tty, to avoid some blockage on the write
 * queue of some tty ioctls (transparent), that would be handled as priority
 * messages.
 * Those ioctls are:
 *
 *	TCXONC TCOOF -> M_STOP,
 *	TCXONC TCOON -> M_START,
 *	TCXONC TCIOF -> M_STOPI,
 *	TCXONC TCION -> M_STARTI,
 *	TIOCSTOP -> M_STOP,
 *	TIOCSTART -> M_START,
 *	TCFLSH and TIOCFLUSH are handled by the M_FLUSH case in that routine.
 */
	case M_STOP:
		if (tp->t_iflag & (IXON | IXANY)) {
			/* ldterm responsible for flow control */
			if (tp->t_state & TS_TTSTOP) {
				freemsg(mp);
				break;
			}
		}
		tp->t_state |= TS_TTSTOP; /* so we do the right thing
					   * in ldtty_close()
					   */
		putnext(q, mp);
		break;

	case M_START:
		if (tp->t_iflag & (IXON | IXANY)) {
			/* ldterm responsible for flow control */
                  	if (!(tp->t_state & TS_TTSTOP)) {
				freemsg(mp);
				break;
			}
		}
		tp->t_state &= ~TS_TTSTOP;
                putnext(q, mp);
                if (!(tp->d_termios.c_lflag & FLUSHO)) {
			tp->t_lflag &= ~FLUSHO;
			tp->u_termios.c_lflag &= ~FLUSHO;
                }
                (void)ldtty_start(tp);
                break;

	case M_STOPI:
		/*
		 * if ldterm is not responsible for input flow control,
		 * just send the message downstream.
		 */
		if (!(tp->t_iflag & IXOFF)) {
			putnext(q, mp);
			break;
		}
		if (!(tp->t_state & TS_TBLOCK) &&
		    tp->t_cc[VSTOP] != _POSIX_VDISABLE) {
			putnext(q, mp);
			tp->t_state |= TS_TBLOCK;
		} else
			freemsg(mp);
		break;

	case M_STARTI:
		/*
		 * if ldterm is not responsible for input flow control,
		 * just send the message downstream.
		 */
		if (!(tp->t_iflag & IXOFF)) {
			putnext(q, mp);
			break;
		}
		if ((tp->t_state & TS_TBLOCK) &&
		    tp->t_cc[VSTART] != _POSIX_VDISABLE) {
			putnext(q, mp);
			tp->t_state &= ~TS_TBLOCK;
		} else
			freemsg(mp);
		break;
			
	default:
		if ((mp->b_datap->db_type >= QPCTL) || canput(q->q_next))
			putnext(q, mp);
		else
			putq(q, mp);
		break;
	}
	Return(0);
}

/*
 * write service routine
 * data may encounter flow control
 */
 int
ldtty_wsrv(q)
	register queue_t *q;	/* write q */
{
	register struct ldtty *tp = (struct ldtty *)q->q_ptr;
	register mblk_t *mp;
        Enter(TRC_LDTERM(TTY_WSRV), tp->t_dev, (int)tp, q->q_count, 0, 0);
	
	if (tp->t_state & TS_WAITOUTBUF) {
		/*
		 * some previous call to ldtty_start() failed
		 * try starting output now
		 */
		tp->t_state &= ~TS_WAITOUTBUF;
		
		if (!ldtty_getoutbuf(tp))
			Return(0);
	}

	if (tp->t_state & TS_WAITOUTPUT) {
		tp->t_state &= ~TS_WAITOUTPUT;
		if (ldtty_start(tp) != 0) {
			/* still flow controled */
			Return(0);
		}
	}

	if (tp->t_state & TS_WAITEUC) {
		tp->t_state &= ~TS_WAITEUC;
		if (euctty_out(tp) != 0) {
			/* still can copy multibyte char to outbuf */
			Return(0);
		}
	}

	while (mp = getq(q)) {
		switch (mp->b_datap->db_type) {
		case M_DATA:
			if (ldtty_write(tp, mp))
				Return(0);
			/* can't do more output now */
			break;
		case M_IOCTL:
			ASSERT(mp == tp->t_qioctl);
/*
 * Defect number 125810: in the next line "-" is replaced by "=".
 */
			tp->t_qioctl = (mblk_t *)NULL;
			ldtty_ioctl(tp, q, mp);
			break;
		default:
			if (!canput(q->q_next)) {
				putbq(q, mp);
				Return(0);
			}
			putnext(q, mp);
			break;
		} /* switch */
	} /* while */

	/*
	 * Don't forget to awake the sleep in the close routine as needed.
	 */
	if (tp->t_state & TS_ASLEEP)
		e_wakeup(&tp->t_event);

	Return(0);
}

mblk_t *
ldtty_readdata(tp, mp)
        struct ldtty *tp;
        mblk_t *mp;
{
        register int post_wakeup = 0;
	register int chars_processed = 0;

        while (mp) {
        	mblk_t *mp1;

		mp1 = mp->b_cont;

                while (mp->b_rptr < mp->b_wptr) {
                        post_wakeup |= ldtty_input(tp, *mp->b_rptr);
			chars_processed++;
			if (post_wakeup & T_POST_FLUSH)
				post_wakeup = 0;
			if (post_wakeup & T_POST_BACKUP) {
				tp->t_state |= TS_RAWBACKUP;
				goto out;
			}
			++mp->b_rptr;
		}
		/* Attempt to recycle memory that we'll need again soon
		 * This will save a typical vi session one allocb() per
 		 * character typed.  If data reference count > 1, the
 		 * data block is being shared, so we can't reuse it.  In
 		 * this case, an mp will be allocated as needed, on demand.
  		 */
 		if ((tp->t_sparebuf) || (mp->b_datap->db_ref > 1))
                	freeb(mp);
		else {
			mp->b_cont = 0;
			tp->t_sparebuf = mp;
		}
		mp = mp1;
        }
out:
	sysinfo_add(sysinfo.rawch, chars_processed);
        ldtty_post_input(tp, post_wakeup);
	if (post_wakeup & T_POST_BACKUP)
		return(mp);
	else
		return((mblk_t *) 0);
}

/*
 * tty ioctls
 *
 * some ioctls are forwarded, some are acked
 */
int
ldtty_ioctl(tp, q, mp)
	register struct ldtty *tp;
	register queue_t *q;		/* write q */
	register mblk_t *mp;
{
	register struct iocblk *iocp = (struct iocblk *)mp->b_rptr;
        Enter(TRC_LDTERM(TTY_IOCTL), tp->t_dev, (int)tp, iocp->ioc_cmd, 0, 0);

	switch (iocp->ioc_cmd) {
	case TIOCSETD: {
		register int	arg =*(int *)mp->b_cont->b_rptr;
		/* these ioctls doesn't have much meaning in STREAMS */
		switch (arg) {
		case OTTYDISC:
			tp->t_lflag &= ~ISIG;
			break;
		case NTTYDISC:
			tp->t_lflag |= ISIG;
			break;
		default:
			ldtty_iocnak_msg(iocp, ENXIO, mp);
			qreply(q, mp);
			Return(-1);
		}
		ldtty_iocack_msg(iocp, 0, mp);
		qreply(q, mp);
		break;
	}
	case TCVPD:
		/*
		 * Nothing to do, just ack-it
		 */
	case TCGSAK:
	case TCGLEN:
	case TCSLEN:
	case TXSETLD:
	case TXADDCD:
	case TXDELCD:
                /* these ioctls doesn't have much meaning in STREAMS */
		ldtty_iocack_msg(iocp, 0, mp);
		qreply(q, mp);
		break;
	case TXGETLD:	/* just return "posix" */
	case TXGETCD:
		 bcopy(POSIX_LINE_DISCIPLINE, mp->b_cont->b_rptr, 
			sizeof("posix") + 1);
		mp->b_cont->b_wptr = mp->b_cont->b_rptr + sizeof("posix") + 1;
		ldtty_iocack_msg(iocp, sizeof("posix") + 1, mp);
		qreply(q, mp);
		break;
		
	case TIOCGETD:
		/* 
		 * This ioctl doesn't have much meaning in STREAMS.
		 * It's apparently only used by things wanting to know
		 * if job control is supported, and they view "2" as a yes
		 * answer, so we return 2 for binary compatibility, since
		 * we do support job control.
		 */
		if (tp->t_lflag & ISIG)
			*(int *)mp->b_cont->b_rptr = NTTYDISC;
		else
			*(int *)mp->b_cont->b_rptr = OTTYDISC;

		mp->b_cont->b_wptr = mp->b_cont->b_rptr + sizeof(int);
		ldtty_iocack_msg(iocp, sizeof(int), mp);
		qreply(q, mp);
		break;
	case TIOCFLUSH: {
		register int flags = *(int *)mp->b_cont->b_rptr;

		if (flags)
			flags &= FREAD|FWRITE;
		else
			flags = FREAD|FWRITE;
		ldtty_flush(tp, flags, 1);
		ldtty_iocack_msg(iocp, 0, mp);
		qreply(q, mp);
	}
		break;
	case TIOCOUTQ:
		putnext(q, mp);	/* some drivers need to receive it */
		break;
	case IOCINFO: {
		struct	devinfo	devinfo_ldtty;

		devinfo_ldtty.devtype = DD_TTY;	/* tty device */
		devinfo_ldtty.flags = 0;	/* not applicable */
		*(struct devinfo *)mp->b_cont->b_rptr = devinfo_ldtty;
		ldtty_iocack_msg(iocp, sizeof(struct devinfo), mp);
		qreply(q, mp);
	}
		break;
	case TCGETX:
		*(struct termiox *)mp->b_cont->b_rptr = tp->t_control;
		mp->b_cont->b_wptr = mp->b_cont->b_rptr + 
					sizeof(struct termiox);
		ldtty_iocack_msg(iocp, sizeof(struct termiox), mp);
		qreply(q, mp);
		break;
	case TIOCSETA:
	case TIOCSETAW:
	case TIOCSETAF:
	case TCSETX:
	case TCSETXW:
	case TCSETXF:
/* SVID.4 starts */
	case TIOCMBIC:
	case TIOCMBIS:
	case TIOCMSET:
/* SVID.4 ends */
		/*
		 * these will be handled on the way up, M_IOCACK
		 */
		putnext(q, mp);
		break;
	case TIOCGETA:
/* SVID.4 starts */
	case TIOCMGET:
/* SVID.4 ends */
		/*
		 * we fill in the info on the way up, M_IOCACK
		 */
		putnext(q, mp);
		break;
	case TIOCSWINSZ:
		if (!ldtty_swinsz(tp, mp)) {

			ldtty_iocnak_msg(iocp, ENOMEM, mp);
			qreply(q, mp);

		} else {

			putnext(q, mp);

		}
		break;
	case TXSETIHOG:
	case TXSETOHOG:
		if (iocp->ioc_cmd == TXSETIHOG)
			tp->t_ihog = *(int *)mp->b_cont->b_rptr;
		else
			tp->t_ohog = *(int *)mp->b_cont->b_rptr;
		ldtty_iocack_msg(iocp, 0, mp);
		qreply(q, mp);
		break;
	case TIOCHPCL: {
		/*
		 * convert this one into a TIOCSETA for the driver
		 * then handle it on the way up
		 */
		struct termios term;

		term = tp->u_termios;
		term.c_cflag |= HUPCL;
		if (ldtty_compat_to_termios(tp, mp, TIOCSETA, &term)) {
			ldtty_iocnak_msg(iocp, ENOMEM, mp);
			qreply(q, mp);
			Return(-1);
		}
		putnext(q, mp);
	}
		break;
	case TIOCSTOP:
		if (tp->t_iflag & (IXON | IXANY)) {
			/* ldterm responsible for flow control */
			if ((tp->t_state & TS_TTSTOP) == 0) {

				if (!putctl(q->q_next, M_STOP)) {
					ldtty_iocnak_msg(iocp, ENOMEM, mp);
					qreply(q, mp);
					break;
				}

				tp->t_state |= TS_TTSTOP;
			}
		} else	/* driver responsible for flow control */

			if (!putctl(q->q_next, M_STOP)) {
				ldtty_iocnak_msg(iocp, ENOMEM, mp);
				qreply(q, mp);
				break;
			}

		ldtty_iocack_msg(iocp, 0, mp);
		qreply(q, mp);
		break;
	case TIOCSTART:
		if (tp->t_iflag & (IXON | IXANY)) {
			/* ldterm responsible for flow control */
			if ((tp->t_state & TS_TTSTOP) ||
			    (tp->t_lflag & FLUSHO))     {

				if (!putctl(q->q_next, M_START)) {
					ldtty_iocnak_msg(iocp, ENOMEM, mp);
					qreply(q, mp);
					break;
				}

				tp->t_state &= ~TS_TTSTOP;
				if (!(tp->d_termios.c_lflag & FLUSHO)) {
					tp->t_lflag &= ~FLUSHO;
					tp->u_termios.c_lflag &= ~FLUSHO;
				}
				(void)ldtty_start(tp);
			}
		} else {	/* driver responsible for flow control */

			if (!putctl(q->q_next, M_START)) {
				ldtty_iocnak_msg(iocp, ENOMEM, mp);
				qreply(q, mp);
				break;
			}

			if (!(tp->d_termios.c_lflag & FLUSHO)) {
				tp->t_lflag &= ~FLUSHO;
				tp->u_termios.c_lflag &= ~FLUSHO;
			}
			(void)ldtty_start(tp);
		}
		ldtty_iocack_msg(iocp, 0, mp);
		qreply(q, mp);
		break;
	case EUC_WSET: {
		eucioc_t	*eu = (eucioc_t *)mp->b_cont->b_rptr;
		if ((eu->eucw[1] > 8) || 
	    	    (eu->eucw[2] > 8) || 
		    (eu->eucw[3] > 8)) {
			ldtty_iocnak_msg(iocp, EINVAL, mp);
			qreply(q, mp);
			Return(-1);
		}

		tp->t_cswidth = *eu;
		/* Not clear from spec. whether we should NAK attempt
		 * to set class 0 to something other than 1,1. We ignore
		 * it and just take values for classes 1, 2, and 3.
		 */
		tp->t_cswidth.eucw[0] = 1;
		tp->t_cswidth.scrw[0] = 1;
                if ((tp->t_cswidth.eucw[1] > 1) ||
                    (tp->t_cswidth.scrw[1] > 1) ||
                    (tp->t_cswidth.eucw[2] > 1) ||
                    (tp->t_cswidth.scrw[2] > 1) ||
                    (tp->t_cswidth.eucw[3] > 1) ||
                    (tp->t_cswidth.scrw[3] > 1))
                        tp->t_state |= TS_MBENABLED;
                else
                        tp->t_state &= ~TS_MBENABLED;
		/*
		 * putnext(q, mp);
		 * line replaced by an M_IOCACK message sent upstream.
		 * Defect number 125048.
		 */
		ldtty_iocack_msg(iocp, 0, mp);
                qreply(q, mp);
	}
		break;
	case EUC_WGET:
		*(eucioc_t *)mp->b_cont->b_rptr = tp->t_cswidth;
		mp->b_cont->b_wptr = mp->b_cont->b_rptr + sizeof(eucioc_t);
		ldtty_iocack_msg(iocp, sizeof(eucioc_t), mp);
		qreply(q, mp);
		break;
	case TIOCCBRK:
	case TIOCSBRK: {
		/*
		 * allocate M_BREAK with one integer.
		 */
		register mblk_t *mp1;

		mp1 = allocb(sizeof(int), BPRI_MED);
		if (!mp1) {
			ldtty_iocnak_msg(iocp, ENOMEM, mp);
			qreply(q, mp);
			Return(-1);
		}
		mp1->b_datap->db_type = M_BREAK;
		/*
		 * integer values depends on the ioctl command to be treated
		 * and on the argument value if any.
		 */
		switch (iocp->ioc_cmd) {
		case TIOCCBRK:
			*(int *)mp1->b_wptr = 0;
			mp1->b_wptr += sizeof(int);
			break;
		case TIOCSBRK:
			*(int *)mp1->b_wptr = 1;
			mp1->b_wptr += sizeof(int);
			break;
		}
			
		putnext(q, mp1);
		/*
		 * Posix interface: always returns 0.
		 */
		ldtty_iocack_msg(iocp, 0, mp);
		qreply(q, mp);
	}
		break;
	case TIOCSTI: {
		/*
		 * Simulate typed input character
		 * XXX should do suser check here
		 */
		ldtty_sti(tp, mp);
		ldtty_iocack_msg(iocp, 0, mp);
		qreply(q, mp);
	}
		break;
/* COMPAT_BSD_4.3	*/
	case TIOCGETP:
	case TIOCSETP:
	case TIOCSETN:
	case TIOCGETC:
	case TIOCSETC:
	case TIOCSLTC:
	case TIOCGLTC:
	case TIOCLBIS:
	case TIOCLBIC:
	case TIOCLSET:
	case TIOCLGET:
		ldtty_bsd43_ioctl(tp, q, mp);
		break;
/* COMPAT_BSD_4.3 */
/* SVID starts */
	case TCXONC:
	case TCFLSH:
	case TCSETA:
	case TCSETAW:
	case TCSETAF:
	case TCGETA:
		ldtty_svid_ioctl(tp, q, mp);
		break;
/* SVID ends */
	default:
		putnext(q, mp);
	}
	Return(0);
}

/*
 * tty ioctl's
 *
 * for handling acks on the way up
 */
int
ldtty_ioctl_ack(tp, q, mp)
	register struct ldtty *tp;
	register queue_t *q;		/* read q */
	register mblk_t *mp;
{
	register struct iocblk *iocp;
	register mblk_t *mp1;

	iocp = (struct iocblk *)mp->b_rptr;
	mp1  = mp->b_cont;
	if (iocp->ioc_error == 0) switch (iocp->ioc_cmd) {
	case TIOCGETA: {
		register struct termios *termiosp;
		speed_t	ospeed, ispeed;

		/*
		 * copy the speeds and cflag from the driver
		 */
		termiosp = (struct termios *)mp1->b_rptr;
		tp->t_cflag = termiosp->c_cflag;
		if ((int)cfgetispeed(&tp->t_termios) == 0) {
			ospeed = cfgetospeed(&tp->t_termios);
			cfsetispeed(&tp->t_termios, ospeed);
		}
		tp->u_termios.c_cflag = tp->t_cflag;

		bcopy((caddr_t)tp->t_cc, (caddr_t)tp->u_termios.c_cc, 
						sizeof(tp->u_termios.c_cc));
		bcopy(&tp->u_termios, (struct termios *)mp1->b_rptr,
			sizeof(struct termios));
		mp1->b_wptr = mp1->b_rptr + sizeof(struct termios);
		iocp->ioc_count = sizeof(struct termios);
		putnext(q, mp);
	}
		break;
	case TIOCSETA:
	case TIOCSETAW:
	case TIOCSETAF: {
		register int cmd = iocp->ioc_cmd;
		register struct termios *t;
                register mblk_t *flush_mp, *new_mp;
                register struct stroptions *sop;
		speed_t	ospeed;

		if (!mp1)
			if (mp->b_datap->db_type == M_CTL)
				return(1);
			else
				break;	
		t = (struct termios *)mp1->b_rptr;

		/*
		 * Need to set here new t_vmin and new t_vtime if we are
		 * switching from icanon to not icanon because need_flush
		 * needs the new value.
		 */
                if (!(t->c_lflag & ICANON)) {
                                /*
                                 * If we are switching to no canonical mode
                                 * and the original ioctl is not a posix one
                                 * then set t_vmin to 1 and t_vtime to 0.
                                 */
                                switch (tp->t_ioctl_cmd) {
                                        case TIOCSETP:
                                        case TIOCSETN:
                                                tp->t_vmin = 1;
                                                tp->t_vtime = 0;
                                                break;
                                        case TCSETA:
                                        case TCSETAW:
                                        case TCSETAF:
						tp->t_vmin = t->c_cc[VMIN];
						tp->t_vtime = t->c_cc[VTIME];
						break;
                                        default:
                                                break;
                                }
		}
		/* Attempt to allocate an M_FLUSH if we'll need one. */
		if (ldtty_need_flush(cmd, tp, t)) {
			flush_mp = allocb(1, BPRI_HI);
			if (flush_mp == 0) {
				if (tp->t_ackbid)
					unbufcall(tp->t_ackbid);
				tp->t_ackbid = bufcall(1, BPRI_HI, qenable,
						tp->t_queue);
				return(0);
			}
		} else
			flush_mp = 0;

		iocp->ioc_count = 0;
		if (ldtty_need_setopts(&tp->t_termios, t)) {
			/*
                 	 * Send an M_SETOPTS message to reflect the new
                 	 * settings.
		 	 */

                	new_mp = allocb(sizeof (struct stroptions),BPRI_MED);
			if (new_mp == 0) {
				/*
			 	 * No memory. Delay this ack until later.
			 	 * Process issuing the ioctl will stall.
			 	 */
				if (tp->t_ackbid)
					unbufcall(tp->t_ackbid);
				tp->t_ackbid =
					bufcall(sizeof (struct stroptions),
						BPRI_MED, qenable, tp->t_queue);
				if (flush_mp)
					freemsg(flush_mp);
				return(0);
			}
			/*
		 	 * send the M_SETOPTS to the stream head
		 	 */
			new_mp->b_datap->db_type = M_SETOPTS;
			sop = (struct stroptions *)new_mp->b_rptr;
			new_mp->b_wptr = new_mp->b_rptr +
				sizeof(struct stroptions);

			if (t->c_lflag & TOSTOP)
				sop->so_flags = SO_TOSTOP;
			else
				sop->so_flags = SO_TONSTOP;
			if (!(t->c_lflag & ICANON)) {
				sop->so_readopt = RNORM ;
#ifdef RPROTCOMPRESS
				sop->so_readopt |= RPROTCOMPRESS;
#endif
				sop->so_flags |= SO_MREADON | SO_READOPT;
				tp->t_state &= ~TS_SETNEEDED;
			}
			else {
				sop->so_readopt = RMSGN | RPROTNORM;
				sop->so_flags |=  SO_READOPT;
		    		tp->t_state |= TS_SETNEEDED;
			}
			putnext(q, new_mp);
		}

                /*
                 * for TIOCSETAW and TIOCSETAF,
                 * the output has already been drained by the driver
                 */
                if (cmd == TIOCSETAF) {
                        ldtty_flush_mp(tp, FREAD, flush_mp, 0);
			flush_mp = 0;
		}
		
		/* Check if we're switching into ICANON -- we'll
		 * need to cook buffered data in this case.
		 */
		if ((!(tp->t_lflag & ICANON) && (t->c_lflag & ICANON)) ||
		    t->c_lflag & PENDIN) {
			ldtty_unset_intimeout(tp);
			t->c_lflag |= PENDIN;
			tp->t_state &= ~TS_RAWBACKUP;
			if (tp->t_shcc) {
				ASSERT(flush_mp != (mblk_t *) 0);
				ldtty_flush_shead(tp, flush_mp);
				flush_mp = 0;
			}
			if (tp->t_iflag & IXOFF) 
				ldtty_tblock(tp);
		}

		if (!(t->c_lflag & ICANON)) {
			if (tp->t_lflag & ICANON) {
				/* Switching out of ICANON -- start keeping
				 * track of bytes count as stream head.
				 */
				tp->t_shcc = 0;
			} else {
				if (tp->t_shcc && (tp->t_vmin > tp->t_shcc)){
				/* VMIN no longer satisfied -- remove
				 * unread data from stream head.
				 */
					ASSERT(flush_mp != (mblk_t *) 0);
					(void)ldtty_flush_shead(tp, flush_mp);
					flush_mp = 0;
				}
				if ((tp->t_vtime == 0)
				|| (tp->t_vmin > 0 && tp->t_rawcc == 0))
					ldtty_unset_intimeout(tp);
			}
		}
		if ((tp->t_state & TS_TTSTOP) &&                    
		    (tp->t_iflag & IXON) && !(t->c_iflag & IXON)) {
                        tp->t_state &= ~TS_TTSTOP;
                        putctl(WR(tp->t_queue)->q_next, M_START);
                        (void)ldtty_start(tp);
                }
/*
 * Not needed for osf1.1 point of view
 *		ASSERT(flush_mp == (mblk_t *) 0);
 */
 		ASSERT(flush_mp == (mblk_t *) 0);
		if (flush_mp)
			freemsg(flush_mp);

                /*
                 * set device information
		 */
		tp->t_oflag = t->c_oflag & ~tp->d_termios.c_oflag;
		tp->t_iflag = t->c_iflag & ~tp->d_termios.c_iflag;
		tp->t_lflag = t->c_lflag & ~tp->d_termios.c_lflag;
		tp->t_cflag = t->c_cflag;
		bcopy((caddr_t)t->c_cc, (caddr_t)tp->t_cc, sizeof(tp->t_cc));
		if ((int)cfgetispeed(&tp->t_termios) == 0) {
			ospeed = cfgetospeed(&tp->t_termios);
			cfsetispeed(&tp->t_termios, ospeed);
		}
		tp->u_termios.c_iflag = t->c_iflag ;
		tp->u_termios.c_oflag = t->c_oflag ;
		tp->u_termios.c_lflag = t->c_lflag ;
		tp->u_termios.c_cflag = tp->t_cflag;
		bcopy((caddr_t)tp->t_cc, (caddr_t)tp->u_termios.c_cc, 
							sizeof(tp->t_cc));
		/*
		 * 128266 patch part 10 BEGIN
		 * tp->t_shad_time = t->c_cc[VTIME] * hz / 10;
		 */
		if (tp->t_ioctl_cmd == 0) {
			tp->t_vmin = tp->t_cc[VMIN];
			tp->t_vtime = tp->t_cc[VTIME];
		}
		tp->t_shad_time = tp->t_vtime * hz / 10;
		/*
		 * 128266 patch part 10 END.
		 */
		/* This may have been an M_CTL resulting from an ioctl
		 * down the master side (i.e., the other side) of a pty.
		 * If so, we're done.
		 */
		if (mp->b_datap->db_type == M_CTL)
			return(1);
		/*
		 * check to see if this really was a BSD or SYSV ioctl
		 */
		if (tp->t_ioctl_cmd) {
			iocp->ioc_cmd = tp->t_ioctl_cmd;
			if (tp->t_ioctl_data)
				mp->b_cont = tp->t_ioctl_data;
			tp->t_ioctl_cmd = 0;
			tp->t_ioctl_data = 0;
		}
 
		putnext(q, mp);

	}
		break;
	case TCSETX:
	case TCSETXF:
	case TCSETXW: {
		register int command = iocp->ioc_cmd;
		register struct termiox *tt_ctrl;

		iocp->ioc_count = 0;
		tt_ctrl = (struct termiox *)mp1->b_rptr;
		/* 
		 * for TCSETXW and TCSETXF, the ouput has already been
		 * drained by the driver.
		 */
		if (command == TCSETXF)
			ldtty_flush(tp, FREAD, 1);
		/*
		 * set device information
		 */
		if (tt_ctrl->x_hflag & (RTSXOFF | DTRXOFF)) {
			tp->t_control = *tt_ctrl;
		} else {
			tp->t_control = *tt_ctrl;
		}
		putnext(q, mp);
	}
		break;
				
	case TIOCSWINSZ:
	case TIOCHPCL:
	case EUC_WSET:
		iocp->ioc_count = 0;
		putnext(q, mp);
		break;
	case TIOCOUTQ: {
		mblk_t	*mp1;
		int	outsize = 0;

		outsize = *(int *)mp->b_cont->b_rptr;
		for (mp1 = q->q_first; mp1; mp1 = mp1->b_next) {
			if (mp1->b_datap->db_type == M_DATA)
				outsize += mp1->b_wptr - mp1->b_rptr;
		}
		outsize += (tp->t_outbuf ? msgdsize(tp->t_outbuf) : 0);
		*(int *)mp->b_cont->b_rptr = outsize;
		iocp->ioc_count = sizeof(int);
		mp->b_cont->b_wptr = mp->b_cont->b_rptr + sizeof(int);
	}
	default:
		putnext(q, mp);
		break;
	}
	return(1);
}

int
ldtty_need_flush(cmd, tp, new)
	int	cmd;
	struct ldtty	*tp;
	struct termios	*new;
{
        if (cmd == TIOCSETAF)
	    return(1);

	/*
	 * If going from raw to cooked and there is data at the stream
	 * head then flush.
	 */
	if (!(tp->t_lflag & ICANON) && new->c_lflag & ICANON && tp->t_shcc)
	    return(1);

	/*
	 * If in raw mode and staying in raw we check to see if vmin
	 * has been raised above what is at the stream head.  If it
	 * has, then we need to flush and start fresh.
	 */
	if (!(tp->t_lflag & ICANON) && !(new->c_lflag & ICANON) &&
	    tp->t_shcc && (tp->t_vmin > tp->t_shcc))
	    return(1);

	/*
	 * If PENDIN is set in the new flags, then we need to flush.
	 */
	if (new->c_lflag & PENDIN)
	    return(1);

	return(0);
}

/*
 * return the size of the structure to copy
 */
 int
ldtioc_sizeof(cmd)
	register int cmd;
{

	switch (cmd) {
	case TIOCSETD:
	case TIOCFLUSH:
	case TIOCGETD:
	case TIOCOUTQ:
	case TIOCCONS:
/* COMPAT_BSD_4.3	*/
	case TIOCLBIS:
	case TIOCLBIC:
	case TIOCLSET:
	case TIOCLGET:
/* COMPAT_BSD_4.3	*/
/* SVID start */
	case TCXONC:
	case TCFLSH:
	case TCSBRK:
/* SVID end */
	case TCSBREAK:
/* SVID.4 starts */
	case TIOCMBIC:
	case TIOCMBIS:
	case TIOCMSET:
	case TIOCMGET:
/* SVID.4 ends */
		return(sizeof(int));
	case EUC_WSET:
	case EUC_WGET:
		return(sizeof(eucioc_t));
	case TIOCSETA:
	case TIOCSETAW:
	case TIOCSETAF:
	case TIOCGETA:
		return(sizeof(struct termios));
	case TIOCSTI:
		return(sizeof(char));
	case TIOCSWINSZ:
	case TIOCGWINSZ:
		return(sizeof(struct winsize));
/* COMPAT_BSD_4.3	*/
	case TIOCSETP:
	case TIOCSETN:
	case TIOCGETP:
		return(sizeof(struct sgttyb));
	case TIOCGETC:
	case TIOCSETC:
		return(sizeof(struct tchars));
	case TIOCSLTC:
	case TIOCGLTC:
		return(sizeof(struct ltchars));
/* COMPAT_BSD_4.3	*/
/* SVID start */
	case TCSETA:
	case TCSETAW:
	case TCSETAF:
	case TCGETA:
		return(sizeof(struct termio));
/* SVID end */
	default:
		return(0);
	}
}


/*
 * process pending characters
 */
void
ldtty_pend(tp)
	struct ldtty *tp;
{
	mblk_t *mp;

	tp->t_lflag &= ~PENDIN;
	tp->u_termios.c_lflag = tp->t_lflag | tp->d_termios.c_lflag;
	tp->t_state |= TS_TYPEN;
	mp = tp->t_rawbuf;      /* get current message */
	ldtty_bufclr(tp); /* clear input buffer */
        if (mp)
                ldtty_readdata(tp, mp);     /* reprocess input */
	tp->t_state &= ~TS_TYPEN;
}

/*
 * process input characters
 * returns flag bits describing postprocessing
 * T_POST_WAKEUP set if ldtty_wakeup() is required
 * T_POST_START set if ldtty_start() is required
 * returns 0 if no postprocessing is required
 */
 int
ldtty_input(tp, c)
	register struct ldtty *tp;
	register int c;
{
	register int iflag = tp->t_iflag;
	register int lflag = tp->t_lflag;
	register u_char *cc = tp->t_cc;
	register int i;
	register int post_wakeup = 0;

	/*
	 * if input is pending take it first
	 */
	if (lflag & PENDIN)
		ldtty_pend(tp);
	/*
	 * in tandem mode, check high water mark
	 */
	if (iflag & IXOFF)
		ldtty_tblock(tp);
	/*
	 * Strip the character if we have not already processed it
	 * once. (Characters coming in while TS_TYPEN is set have
	 * already been through here once.
	 */
	if (((tp->t_state & TS_TYPEN) == 0) && ((c & TTY_QUOTE) == 0)) {
		if (iflag & ISTRIP)
			c &= 0x7f;
		else if ((iflag & PARMRK) && (c == 0377)) {
			/*
		 	 * POSIX: If ISTRIP is not set (just checked above),
		  	 * and PARMRK is set, deliver a legitimate \0377 
			 * character as \0377, \0377.  Do the first by sending 
			 * a quoted \0377 through ldtty_input(). Do the second 
			 * by just allowing the \0377 that we already have to 
			 * fall through the rest of this function.
		 	 */
			ldtty_input(tp,(TTY_QUOTE | 0377));
			sysinfo_add(sysinfo.rawch, 1);
		}
	}
	/*
	 * check for literal next
	 */
	if (tp->t_state & TS_LNCH) {
		c |= TTY_QUOTE;
		tp->t_state &= ~TS_LNCH;
	}

	/*
	 * signals
	 */
	if (lflag & ISIG) {
		if (CCEQ(cc[VINTR], c) || CCEQ(cc[VQUIT],c )) {
			if ((lflag & NOFLSH) == 0) {
				post_wakeup |= T_POST_FLUSH;
				ldtty_flush(tp, FREAD|FWRITE, 1);
			}
			ldtty_echo(tp, c);
			if (CCEQ(cc[VINTR], c))
				putctl1(tp->t_queue->q_next, M_PCSIG, SIGINT);
			else
				putctl1(tp->t_queue->q_next, M_PCSIG, SIGQUIT);
			goto endcase;
		}
		if (CCEQ(cc[VSUSP], c)) {
			if ((lflag & NOFLSH) == 0) {
				post_wakeup |= T_POST_FLUSH;
				ldtty_flush(tp, FREAD|FWRITE, 1);
			}
			ldtty_echo(tp, c);
			putctl1(tp->t_queue->q_next, M_PCSIG, SIGTSTP);
			goto endcase;
		}

		if ((lflag & IEXTEN) && CCEQ(cc[VDSUSP], c)) {

			/* We just mark this here.  We take care of it
			 * before we send it upstream.
			 */
			tp->t_state |= TS_DSUSP;
		}
	}
	/*
	 * start/stop chars
	 */
	if (iflag & IXON) {
		if (CCEQ(cc[VSTOP], c)) {
			if ((tp->t_state & TS_TTSTOP) == 0) {
				tp->t_state |= TS_TTSTOP;
				putctl(WR(tp->t_queue)->q_next, M_STOP);
				return(post_wakeup);
			}
			if (!CCEQ(cc[VSTART], c))
				return(post_wakeup);
			/*
			 * if VSTART == VSTOP then toggle
			 */
			goto endcase;
		}
		if (CCEQ(cc[VSTART], c))
			goto restartoutput;
	}
	/*
	 * IGNCR, ICRNL, INLCR
	 */
	if (c == '\r') {
		if (iflag & IGNCR)
			goto endcase;
		else if (iflag & ICRNL)
			c = '\n';
	}
	else if ((c == '\n') && (iflag & INLCR))
		c = '\r';
	/*
	 * map upper case to lower case
	 */
	if ((iflag & IUCLC) && ('A' <= c) && (c <= 'Z'))
		c += 'a' - 'A';

	/*
	 * non canonical mode
	 */
	if ((lflag & ICANON) == 0) {
		if (tp->t_rawcc >= tp->t_ihog) {
			/*
			 * Defect 151624.
			 */
			if (iflag & IXOFF)
				ldtty_error(tp->t_devname, ERRID_TTY_TTYHOG, 0);
			post_wakeup |= T_POST_BACKUP;
		}
		else if (ldtty_stuffc(c & TTY_CHARMASK, tp) >= 0) {
			if ((tp->t_state & TS_CNTTB) == 0) {
                		if (!(tp->d_termios.c_lflag & FLUSHO)) {
                        		tp->t_lflag &= ~FLUSHO;
                        		tp->u_termios.c_lflag &= ~FLUSHO;
                		}
        		}
			/*
			 * defect number 148029, this is to avoid the call to
			 * echo if it is not necessary. ldtty_echo() does the
			 * same test.
			 */
			if ((lflag & ECHO) || ((lflag & ECHONL) && (c == '\n')))
				ldtty_echo(tp, c);
			/*
			 * 128266 patch part 7 BEGIN
			 */
			if (tp->t_rawcc >= tp->t_vmin) {
			/*
			 * 128266 patch part 7 END
			 */
					/* 
					 * Set flag but wit until caller is
					 * done processing the whole M_DATA
					 * message before sending.
					 */
					post_wakeup |= T_POST_WAKEUP;
			}
			else
                                /*
                                 * Caller may need to (re)start timer
                                 */
                                post_wakeup |= T_POST_TIMER;
		}
		goto endcase;
	}
	/*
	 * canonical mode
	 */

	/*
	 * discard
	 */
#ifndef VDISCARD
#define VDISCARD VDISCRD
#endif /* VDISCARD */
	if ((lflag & IEXTEN) && CCEQ(cc[VDISCARD], c)) {
		if ((lflag & FLUSHO) &&
		    (!(tp->d_termios.c_lflag & FLUSHO))) {
			tp->t_lflag &= ~FLUSHO;
			tp->u_termios.c_lflag &= ~FLUSHO;
		}
		else {
			ldtty_flush(tp, FWRITE, 1);
			ldtty_echo(tp, c);
			if (tp->t_rawcc)
				ldtty_retype(tp);
			if (!(tp->d_termios.c_lflag & FLUSHO)) {
				tp->t_lflag |= FLUSHO;
				tp->u_termios.c_lflag |= FLUSHO;
			}
		}
		goto startoutput;
	}

	if (tp->t_quot && (CCEQ(cc[VERASE], c) || (CCEQ(cc[VKILL], c)))) {
		ldtty_rub(tp, ldtty_unstuffc(tp));
	} else {
		/*
		 * erase
		 */
		if (CCEQ(cc[VERASE], c)) {
			if (ldtty_mbenabled(tp)) {
				euctty_erase(tp);
				goto endcase;
			}
			/*
			 * single byte erase
			 */
			if (tp->t_rawcc) {
				unsigned char rub_c;

				rub_c = ldtty_unstuffc(tp);
				ldtty_rub(tp, rub_c);
			}
			goto endcase;
		}
		/*
		 * kill
		 */
		if (CCEQ(cc[VKILL], c)) {
			if (ldtty_mbenabled(tp)) {
				euctty_kill(tp);
				goto endcase;
			}
			/*
			 * single byte kill
			 */
			if (((lflag & (ECHOKE|IEXTEN)) == (ECHOKE|IEXTEN)) &&
			    (tp->t_rawcc == tp->t_rocount) &&
			    !(lflag & ECHOPRT)) {
				while (tp->t_rawcc) {
					unsigned char rub_c;

					rub_c = ldtty_unstuffc(tp);
					ldtty_rub(tp, rub_c);
				}
			}
			else {
				ldtty_echo(tp, c);
				if (lflag & ECHOK)
					ldtty_echo(tp, '\n');
				if (tp->t_rawbuf)
					ldtty_bufreset(tp);
				tp->t_rocount = 0;
			}
			tp->t_state &= ~TS_LOCAL;
			goto endcase;
		}
	}
	/*
	 * word erase
	 */
	if (CCEQ(cc[VWERSE], c) && (lflag & IEXTEN)) {
		if (ldtty_mbenabled(tp)) {
			euctty_werase(tp);
			goto endcase;
		}
		/*
		 * single byte word erase
		 */
		{
		register int ctype;
		int rub_c;

		/*
		 * erase white space
		 */
		while ((rub_c = ldtty_unstuffc(tp)) == ' ' ||
		       rub_c == '\t')
			ldtty_rub(tp, rub_c);
		if (rub_c == -1)
			goto endcase;
		/*
		 * special case last char of token
		 */
		ldtty_rub(tp, rub_c);
		rub_c = ldtty_unstuffc(tp);
		if ((rub_c == -1) || (rub_c == ' ') || (rub_c == '\t')) {
			if (rub_c != -1)
				ldtty_stuffc(rub_c, tp);
			goto endcase;
		}
		/*
		 * erase rest of token
		 */
#define CTYPE(c) ((lflag&ALTWERASE) ? (partab[(c)&TTY_CHARMASK]&0100) : 0)
		ctype = CTYPE(rub_c);
		do {
			ldtty_rub(tp, rub_c);
			rub_c = ldtty_unstuffc(tp);
			if (rub_c == -1)
				goto endcase;
		} while ((rub_c != ' ') && (rub_c != '\t') &&
			 (CTYPE(rub_c) == ctype));
		ldtty_stuffc(rub_c, tp);
		goto endcase;
#undef CTYPE
		} /* single byte word erase */
	}
	/*
	 * reprint
	 */
	if (CCEQ(cc[VREPRINT], c) && (lflag & IEXTEN)) {
		ldtty_retype(tp);
		goto endcase;
	}


	/*
	 * literal next
	 */
	if (CCEQ(cc[VLNEXT], c) && (lflag & IEXTEN)) {
		if (lflag & ECHO) {
			if (lflag & ECHOE) {
				ldtty_output(tp, '^');
				ldtty_output(tp, '\b');
			}
			else {
				ldtty_echo(tp, c);
			}
		}
		tp->t_state |= TS_LNCH;
		goto endcase;
	}

	if (lflag & XCASE && c <= 0177) {

		if (tp->t_state & TS_BKSL) {

			char maptab[] = {
				000,000,000,000,000,000,000,000,
				000,000,000,000,000,000,000,000,
				000,000,000,000,000,000,000,000,
				000,000,000,000,000,000,000,000,
				000,'|',000,000,000,000,000,'`',
				'{','}',000,000,000,000,000,000,
				000,000,000,000,000,000,000,000,
				000,000,000,000,000,000,000,000,
				000,000,000,000,000,000,000,000,
				000,000,000,000,000,000,000,000,
				000,000,000,000,000,000,000,000,
				000,000,000,000,000,000,'~',000,
				000,'A','B','C','D','E','F','G',
				'H','I','J','K','L','M','N','O',
				'P','Q','R','S','T','U','V','W',
				'X','Y','Z',000,000,000,000,000,
			};

			ldtty_rub(tp, ldtty_unstuffc(tp));
			if (maptab[c])
				c = maptab[c];
			tp->t_state &= ~TS_BKSL;
			tp->t_quot = 0;

		} else if (c == '\\') {

			tp->t_state |= TS_BKSL;
		}
	}

	/*
	 * check for input overflow
	 */
	if (tp->t_rawcc >= tp->t_ihog) {
		if (!(ldtty_breakc(c, lflag))) {
	/* can't complete read, so gotta dump chars */
			if ((lflag & IEXTEN) && (iflag & IMAXBEL)) {
	/* if imaxbel, beep and dump (implicitly) from the end */
				if (ldtty_msgdsize(tp->t_outbuf) < tp->t_ohog)
					ldtty_output(tp, CTRL(g));
			}
			else
			{
	/* dump'em all and log it */
				ldtty_error(tp->t_devname, ERRID_TTY_TTYHOG, 0);
				ldtty_bufreset(tp);
			}
			goto endcase;
		}
	}
	/*
	 * put data in tp->t_rawbuf
	 * wakeup if it is a line delimiter
	 */
	if (ldtty_stuffc( c & TTY_CHARMASK, tp) >= 0) {
                if (ldtty_breakc(c, lflag)) {
                        tp->t_rocount = 0;
                        /*
                         * At this point, we just want to send the rawbuf 
			 * upstream
                         */
                        ldtty_sendcanon(tp);
                }
                else if (tp->t_rocount++ == 0)
                        tp->t_rocol = tp->t_col;
                if (tp->t_state & TS_ERASE) {
                        /*
                         * end of prterase \.../
                         */
                        tp->t_state &= ~TS_ERASE;
                        ldtty_output(tp, '/');
                }
                i = tp->t_col;
                ldtty_echo(tp, c);
                if (CCEQ(cc[VEOF], c) && (lflag & ECHO)) {
                        /*
                         * place the cursor over the '^' of the ^D
                         */
                        i = MIN(2, tp->t_col - i);
                        while (i > 0) {
                                ldtty_output(tp, '\b');
                                i--;
                        }
                }
        }
endcase:
	tp->t_quot = c == '\\';
	/*
	 * IXANY means allow any character to restart output
	 */
	if ((tp->t_state & TS_TTSTOP) && !(iflag & IXANY) &&
	    (cc[VSTART] != cc[VSTOP]))
		return(post_wakeup);
restartoutput:
        /*
         * Only send an M_START message if we need to
         */
        if ((tp->t_state & TS_TTSTOP) && (tp->t_iflag & (IXON|IXANY))) {
                tp->t_state &= ~TS_TTSTOP;
                putctl(WR(tp->t_queue)->q_next, M_START);
        }
	if (!(tp->d_termios.c_lflag & FLUSHO)) {
		tp->t_lflag &= ~FLUSHO;
		tp->u_termios.c_lflag &= ~FLUSHO;
	}
startoutput:
        /*
         * Caller needs to start output
         */
        post_wakeup |= T_POST_START;
	return(post_wakeup);
}

int
ldtty_flush_shead(tp, mp)
	struct	ldtty	*tp;
	register	mblk_t	*mp;
{
	
        if (!mp) {
                mp = allocb(1, BPRI_HI);
                if (!mp)
			return(0);
        }
        if (mp->b_datap->db_size < 1) {
#if MACH_ASSERT
                panic("ldtty_flush_shead: zero length message");
#endif
		return(0);
        }
        mp->b_rptr = mp->b_wptr = mp->b_datap->db_base;
        mp->b_flag |= MSGNOTIFY;
        mp->b_datap->db_type = M_FLUSH;
        *mp->b_wptr++ = FLUSHR;
        tp->t_shcc = 0;
        if (tp->t_unsent = tp->t_rawbuf) /* ASSIGN */
                tp->t_unsent_ndx =
                        tp->t_rawbuf->b_rptr - tp->t_rawbuf->b_datap->db_base;
        else
                tp->t_unsent_ndx = 0;
        putnext(tp->t_queue, mp);

	return(1);
}

/*
 * flush the tty queues
 *
 * this is called when a M_FLUSH is received and also
 * from various places in the line discipline
 * if generate, then generate M_FLUSH messages
 *
 * Notice that a READ flush sends a FLUSHR message downstream.
 * The driver at the bottom will reflect it back upstream, causing
 * all modules to flush their read queues in their rput procedures.
 * The stream head will eat the FLUSHR message when it gets to the top.
 *
 * Conversely, a WRITE flush sends a FLUSHW message upstream.
 * The stream head will reflect this message downstream so all modules
 * will flush their write queues in their wput routines.
 * The driver will eat the FLUSHW message at the bottom.
 */
int
ldtty_flush(tp, flags, generate)
	register struct ldtty *tp;
	register int flags;
	register int generate;
{
	mblk_t	*rmp, *wmp;

	rmp = wmp = (mblk_t *) 0;
	if (generate) {
		/* We will punt if operation would be half baked */
		if (flags & FREAD) {
			rmp = allocb(1, BPRI_HI);
			if (!rmp)
				return(0);
		}
		if (flags & FWRITE) {
			if (wmp = tp->t_outbuf) {
				tp->t_outbuf = 0;
				if (wmp->b_cont) {
					freemsg(wmp->b_cont);
					wmp->b_cont = 0;
				}
				wmp->b_rptr = wmp->b_wptr =
					wmp->b_datap->db_base;
			} else {
				wmp = allocb(1, BPRI_HI);
				if (!wmp) {
					if (rmp)
						freeb(rmp);
					return(0);
				}
			}
		}
	}
	ldtty_flush_mp(tp, flags, rmp, wmp);

	return(flags);
}

void
ldtty_flush_mp(tp, flags, rmp, wmp)
	struct	ldtty	*tp;
	int	flags;
	mblk_t	*rmp, *wmp;
{
        if (flags & FREAD) {
                flushq(tp->t_queue, FLUSHDATA);
                tp->t_state &= ~TS_RAWBACKUP;
		/*
		 * Verify that there is really a message allocated.
		 */
                if (rmp && (rmp->b_datap)) {
                        rmp->b_datap->db_type = M_FLUSH;
                        *rmp->b_wptr++ = FLUSHR;
                        putnext(WR(tp->t_queue), rmp);
                }
                if (tp->t_rawbuf)
                        ldtty_bufreset(tp);
		/*
		 * 128266 patch part 8 BEGIN
		 */
		if (tp->t_vmin > 0)
			ldtty_unset_intimeout(tp);
		/*
		 * 128266 patch part 8 END.
		 */
                tp->t_shcc = 0;
                tp->t_rocount = 0;
                tp->t_rocol = 0;
		tp->t_quot = 0;
                tp->t_state &= ~TS_LOCAL;
		if (tp->t_iflag & IXOFF)
                	ldtty_tblock(tp);
        }
        if (flags & FWRITE) {
                flushq(WR(tp->t_queue), FLUSHDATA);
		/*
		 * Verify that the t_outbuf was not really freed ealier.
		 */
		if (!tp->t_outbuf->b_datap)
			tp->t_outbuf = 0;
                if (tp->t_outbuf) {
			freemsg(tp->t_outbuf);
			tp->t_outbuf = 0;
		}
		/*
		 * Verify that there is really a message allocated.
		 * Verification involved by the defect number 124348.
		 */
                if (wmp && (wmp->b_datap)) {
                        wmp->b_datap->db_type = M_FLUSH;
                        *wmp->b_wptr++ = FLUSHW;
                        putnext(tp->t_queue, wmp);
                }
        }
}



/*
 * echo the character
 */
void
ldtty_echo(tp, c)
	register struct ldtty *tp;
	register int c;
{
	if ((tp->t_state & TS_CNTTB) == 0) {
		if (!(tp->d_termios.c_lflag & FLUSHO)) {
			tp->t_lflag &= ~FLUSHO;
			tp->u_termios.c_lflag &= ~FLUSHO;
		}
	}
	if (((tp->t_lflag & ECHO) == 0) &&
	    !((tp->t_lflag & ECHONL) && (c == '\n')))
		return;
	/*
	 * multi-byte EUC echo processing
	 */
	if (ldtty_mbenabled(tp)) {
		euctty_echo(tp, c);
		return;
	}
	if ((tp->t_lflag & ECHOCTL) && (tp->t_lflag & IEXTEN)) {
		/*
		 * echo C0
		 */
		if (((c & TTY_CHARMASK) < 0x20) && (c != '\t') && (c != '\n') ||
		    (c == 0x7f)) {
			(void)ldtty_output(tp, '^');
			c &= TTY_CHARMASK;
			if (c == 0x7f)
				c = '?';
			else
				c += 'A' - 1;
		}
		/*
		 * we won't echo C1 characters as M-^something.
		 * a lower converter module should have converted
		 * the C1 to its C0 equivalent.
		 */
	}
	if ((tp->t_lflag & XCASE) && (c == '\\')) {
		(void)ldtty_output(tp, c | TTY_QUOTE);
		return;
	}
	(void)ldtty_output(tp, c);
}

/*
 * rubout one character as cleanly as possible
 *
 * the character has already been taken out of the buffer
 */
 void
ldtty_rub(tp, c)
	register struct ldtty *tp;
	register int c;
{
	if ((tp->t_lflag & ECHO) == 0)
		return;
	if (!(tp->d_termios.c_lflag & FLUSHO)) {
		tp->t_lflag &= ~FLUSHO;
		tp->u_termios.c_lflag &= ~FLUSHO;
	}
	if (tp->t_lflag & ECHOE) {
		if (tp->t_rocount == 0) {
			/*
			 * screwed by write, retype the line
			 */
			ldtty_retype(tp);
			return;
		}
		/* XXX - can we use partab? */
		switch (partab[c &= TTY_CHARMASK] & 077) {
		case ORDINARY:
			ldtty_rubo(tp, 1);
			break;
		case VTAB:
		case BACKSPACE:
		case CONTROL:
		case RETURN:
		case NEWLINE:
		case FF:
			if ((tp->t_lflag & ECHOCTL) && (tp->t_lflag & IEXTEN))
				ldtty_rubo(tp, 2);
			break;
		case TAB: {
			register int savecol;
			register unsigned char *cp;
			register mblk_t *mp;
			/*
			 * if the column position got screwed, retype
			 */
			if (ldtty_mbenabled(tp)) {
				if (tp->t_rocount < euctty_rocount(tp)) {
					ldtty_retype(tp);
					return;
				}
			}
			else if (tp->t_rocount < tp->t_rawcc) {
				ldtty_retype(tp);
				return;
			}
			savecol = tp->t_col;
			tp->t_state |= TS_CNTTB;
			if (!(tp->d_termios.c_lflag & FLUSHO)) {
				tp->t_lflag |= FLUSHO;
				tp->u_termios.c_lflag |= FLUSHO;
			}
			tp->t_col = tp->t_rocol;
			mp = tp->t_rawbuf;
			while (mp) {
			    cp = mp->b_rptr;
			    while (cp < mp->b_wptr)
				ldtty_echo(tp, *cp++);
			    mp = mp->b_cont;
			}
			if (!(tp->d_termios.c_lflag & FLUSHO)) {
				tp->t_lflag &= ~FLUSHO;
				tp->u_termios.c_lflag &= ~FLUSHO;
			}
			tp->t_state &= ~TS_CNTTB;
			savecol -= tp->t_col;
			tp->t_col += savecol;
			if (savecol > 8)
				savecol = 8;
			while (--savecol >= 0)
				ldtty_output(tp, '\b');
		}
			break;
		break;
			ldtty_retype(tp);
			return;
		}
	}
	else if (tp->t_lflag & ECHOPRT) {
		if ((tp->t_state & TS_ERASE) == 0) {
			ldtty_output(tp, '\\');
			tp->t_state |= TS_ERASE;
		}
		ldtty_echo(tp, c);
	}
	else
		ldtty_echo(tp, tp->t_cc[VERASE]);
	tp->t_rocount--;
}

 void
ldtty_rubo(tp, ct)
	register struct ldtty *tp;
	register int ct;
{
	while (--ct >= 0) {
		(void)ldtty_output(tp, '\b');
		(void)ldtty_output(tp, ' ');
		(void)ldtty_output(tp, '\b');
	}
}

int
ldtty_putc(c, tp)
	register	int	c;
	struct	ldtty	*tp;
{
	register	mblk_t	*mp;

	/*
	 * Sometimes the t_outbuf message is freed but the address is not set
	 * to NULL=>panic when we are trying to put a character in it.
	 * To avoid this problem, a solution is to verify if that t_outbuf is
	 * really a message block. If not them  set the t_outbuf to NULL.
	 * That will prevent the panic problem and we will get a new buffer
	 * allocated .
	 */
	if (!tp->t_outbuf->b_datap)
		tp->t_outbuf = 0;
	if ((!tp->t_outbuf) && (!ldtty_getoutbuf(tp)))
		return(-1);
	mp = tp->t_outbuf;
	if (mp && (mp->b_wptr < mp->b_datap->db_lim)) {
		*mp->b_wptr++ = (unsigned char)c;
		return(0);
	}
	return(-1);
}


/*
 * process output to the terminal
 * adding delays, expanding tabs, CR/NL, etc..
 * returns < 0 if successful
 * must be recursive
 */
 int
ldtty_output(tp, c)
	register struct ldtty *tp;
	register int c;
{
	register char	*colp;
	register int ctype;
	register long oflag = tp->t_oflag;
	register int flush = tp->t_lflag & FLUSHO;

	if (!(oflag & OPOST) || (c & TTY_QUOTE)) {
		if (flush)
			return(-1);
		if (ldtty_putc(c & TTY_CHARMASK, tp))
			return(c);
		return(-1);
	}
	c &= TTY_CHARMASK;
	/*
	 * turn tabs to spaces as required
	 */
	if ((c == '\t') && ((oflag & OXTABS) || ((oflag & TABDLY) == TAB3))) {
		c = 8 - (tp->t_col & 7);
		if (!flush) {
			register int i = 0;

			while (i < c) {
				if (ldtty_putc(' ', tp))
					break;
				i++;
			}
			c = i;
		}
		tp->t_col += c;
		return(tp->t_col & 7 ? '\t' : -1);
	}
	if ((c == CEOF) && (oflag & ONOEOT))
		return(-1);
	/*
	 * generate escapes for upper-case-only terminals
	 */
	if (tp->t_lflag & XCASE) {
		colp = "({)}!|^~'`\\\\";
		while (*colp++)
			if (c == *colp++) {
				(void)ldtty_output(tp, '\\' | TTY_QUOTE);
				c = colp[-2];
				break;
			}
		if (('A' <= c) && (c <= 'Z')) {
			(void)ldtty_output(tp, '\\' | TTY_QUOTE);
		}
	}
	if ((oflag & OLCUC) && ('a' <= c) && (c <= 'z'))
		c += 'A' - 'a';
	/*
	 * turn <nl> to <cr><lf> if desired
	 */
	if ((c == '\n') && (oflag & ONLCR) && (ldtty_output(tp, '\r') >= 0))
		return(c);
	if ((c == '\r') && (oflag & ONOCR) && (tp->t_col == 0))
		return(-1);
	if ((c == '\r') && (oflag & OCRNL)) {
		c = '\n';
		if (!flush && ldtty_putc(c, tp))
			return('\r');
	}
	else {
		if (!flush && ldtty_putc(c, tp))
			return(c);
		if ((c == '\n') && (oflag & ONLRET))
			c = '\r';
	}

	/*
	 * calculate delays (taken from BSD tty.c)
	 *
	 * should not support any terminals which needs this stuff!
	 */
	colp = (char *)&tp->t_col;
	ctype = partab[c];
	c = 0;
	switch (ctype&077) {
	case ORDINARY:
		(*colp)++;
	case CONTROL:
		break;
#define mstohz(ms)	((((ms) * hz) >> 10) & 0x0ff)
	case BACKSPACE:
		if (oflag & BSDLY)
			if (oflag & OFILL)
				c = 1;
			else
				c = mstohz(100);
		if (*colp)
			(*colp)--;
		break;
	case NEWLINE:
		ctype = oflag & NLDLY;
		if (ctype == NL2) {
			if (*colp > 0) {
				c = (((unsigned int)*colp) >> 4) + 3;
				if ((unsigned int)c > 6)
					c = mstohz(60);
				else
					c = mstohz(c * 10);
			}
		}
		else if (ctype == NL1)
			if (oflag & OFILL)
				c = 2;
			else
				c = mstohz(100);
		break;
	case TAB:
		ctype = oflag & TABDLY;
		if (ctype == TAB1) {
			c = 1 - (*colp | ~07);
			if (c < 5)
				c = 0;
			else
				c = mstohz(10 * c);
		}
		else if (ctype == TAB2) {
			c = mstohz(200);
		}
		else
			c = 0;
		if (ctype && (oflag & OFILL))
			c = 2;
		*colp |= 07;
		(*colp)++;
		break;
	case VTAB:
		if (oflag & VTDELAY)
			c = 177;
		break;
	case RETURN:
		if ((oflag & ONOCR) && (*colp == 0))
			return(-1);
		ctype = oflag & CRDLY;
		if (ctype == CR2)
			if (oflag & OFILL)
				c = 2;
			else
				c = mstohz(100);
		else if (ctype == CR3)
			c = mstohz(166);
		else if (ctype == CR1) {
			if (oflag & OFILL)
				c = 4;
			else {
				c = (*colp >> 4) + 3;
				c = c < 6 ? 6 : c;
				c = mstohz(c * 10);
			}
		}
		*colp = 0;
		break;
	case FF:
		if (oflag & FFDLY)
			c = 0177;
		break;
	}

	if (c && !flush) {		/* do delay */
		
		if (oflag & OFILL) {

			if (c < 32) {

				ctype = oflag & OFDEL ? 0x7f : 0;
				(void) ldtty_putc(ctype, tp);
				if (c > 3)
					(void) ldtty_putc(ctype, tp);

			} else {

#define DCHAR 0xff
				(void) ldtty_putc(DCHAR, tp);
				(void) ldtty_putc(c, tp);
			}

		} else {

			/*
			 * send an M_DELAY to the driver
			 * after sending any pending output
			 */
			register mblk_t *mp;

			if (!ldtty_start(tp)) {
				mp = allocb(sizeof(int), BPRI_MED);
				if (mp) {
					mp->b_datap->db_type = M_DELAY;
					*(int *)mp->b_wptr++ = c;
					putnext(WR(tp->t_queue), mp);
				}
			}
		}
	}
	return(-1);
}

/*
 * start output
 * returns 0 on succes, -1 if couldn't allocate new output buffer
 * uses bufcall to recover from allocb failure.
 * -1 is returned, also, in case of stream flow-control.
 */
 int
ldtty_start(tp)
	register struct ldtty *tp;
{
	queue_t *q = WR(tp->t_queue);
	register mblk_t *mp;
	register mblk_t *mp1;
	int	size;

	/*
	 * don't do anything here if we'll be called by wsrv later
	 */
	if (tp->t_state & (TS_WAITOUTPUT | TS_WAITOUTBUF | TS_TTSTOP))
		return(-1);
	/*
	 * don't send an empty message: but awake sleepers if any.
	 */
	if (!(size = ldtty_msgdsize(tp->t_outbuf)))
		goto awake;
	/*
	 * Stop for STREAMS flow control
	 */
	if (!canput(q->q_next)) {
		tp->t_state |= TS_WAITOUTPUT;
		return(-1);
	}
	/*
	 * try to allocate the next t_outbuf
	 * use bufcall on failure
	 */
	mp1 = allocb(tp->t_ohog, BPRI_MED);
	if (!mp1) {
		tp->t_state |= TS_WAITOUTBUF;
		bufcall(tp->t_ohog, BPRI_MED, qenable, WR(tp->t_queue));
		return(-1);
	}
	/*
	 * send chars in outbuf down to the driver
	 */
	mp = tp->t_outbuf;
	tp->t_outbuf = mp1;
	mp->b_datap->db_type = M_DATA;
	/*
	 * Count output bytes
	 */
	sysinfo_add(sysinfo.outch, size);
	putnext(q, mp);

/*
 * Awake sleepers when drained on close.
 */
awake:
        if (q->q_first)
                qenable(q);
	return(0);
}

/*
 * retype the characters in the raw buffer
 */
 void
ldtty_retype(tp)
	register struct ldtty *tp;
{
	register unsigned char *cp;
	register mblk_t *mp;

	if ((tp->t_cc[VREPRINT] != _POSIX_VDISABLE) && (tp->t_lflag & IEXTEN))
		ldtty_echo(tp, tp->t_cc[VREPRINT]);
	ldtty_output(tp, '\n');
	mp = tp->t_rawbuf;
	while (mp) {
	    	cp = mp->b_rptr;
	    	while (cp < mp->b_wptr)
			ldtty_echo(tp, *cp++);
	    	mp = mp->b_cont;
	}
	tp->t_state &= ~TS_ERASE;
	if (ldtty_mbenabled(tp))
		tp->t_rocount = euctty_rocount(tp);
	else
		tp->t_rocount = tp->t_rawcc;
	tp->t_rocol = 0;
}

/*
 * Conditionally block further input
 */
void
ldtty_tblock(tp)

	register struct ldtty *tp;
{
	queue_t	*q = tp->t_queue;
	int	will_restart = (tp->t_lflag & ICANON) ?
				!canput(q->q_next) : (tp->t_shcc > 0);

	/* 
	 * Block input if current buffer is getting full. Special
	 * treatment for ICANON is for a bug (#5894) in wich IXOFF under
	 * ICANON may hung, since we don't know whether there is
	 * any data at all at the stream head, that is, whether the
	 * opportunity exists for the rawcc to return to the low water
	 * mark without further keyboard input (such as ^C or ^U).
	 * The mechanism for handling IXOFF correctly in raw mode
	 * happens to exist "by chance" (it exists for an unrelated
	 * reason), so this is just an ICANON bug.
	 */
	if (tp->t_state & TS_TBLOCK) {
		if ((tp->t_rawcc < LDTTYLOWAT(tp) || !will_restart) &&
		    putctl(WR(q)->q_next, M_STARTI))
			tp->t_state &= ~TS_TBLOCK;
	} else {
		if ((tp->t_rawcc >= LDTTYHIWAT(tp)) && will_restart &&
		    putctl(WR(q)->q_next, M_STOPI))
			tp->t_state |= TS_TBLOCK;
	}
}

/*
 *
 * Function description: ldtty_wakeup
 *
 *      Called when we want to send non-canonical input upstream
 *	Never called in the case of canonical input
 * 
 * Arguments:
 *
 *      tp - pointer to ldtty structure
 *
 * return value:
 *
 *      None
 *
 * Side effects:
 *
 *	pending read timeout cancelled
 *      current rawbuf sent upstream.
 * 	new buffer allocated.
 *
 */
void
ldtty_wakeup(tp)
	register struct ldtty *tp;
{
	ldtty_unset_intimeout(tp);
        tp->t_state &= ~TS_VTIME_FLAG;
	/*
	 * We're not buffering input characters
	 * Just send current buffer upstream
	 */
	ldtty_sendraw(tp);
}

/*
 * called by timeout
 */
void
ldtty_intimeout(timeout_arg_t arg)
{
	register struct ldtty *tp = (struct ldtty *)arg;

	/* The timeout state (consisting of a non-NULL tp->t_intimeout fiels
	 * and a set TS_INTIMEOUT bit in tp->t_state) will be cleared
	 * when the service procedure calls ldtty_check_intimeout(). We
	 * can't do it here since we are not protected by streams locking.
	 */
	ldtty_post_intimeout(tp);
	qenable(tp->t_queue);
}

int 
ldtty_scanc(cc, cp, tab, mask)
        int     cc;
        char    *cp;
        char    *tab;
        char    mask;
{
        if (cc > 0)
                while (((tab[*(unsigned char *)cp++] & mask) == 0) && 
                        (--cc > 0));
        return cc;
}

/*
 * NAME:	ldtty_canputnext
 *
 * FUNCTION:	Only called by ldtty_write(), when we are on the last message
 *		data block of the total message to send downstream after any
 *		output character conversion to do. This is for performance to
 *		avoid the freeing of a t_outbuf and the allocation of a new
 *		one for the next output.
 *		So we copy the t_outbuf that was the result of the characters
 *		checked to be output into the last message received if there
 *		is enough space to hold the post-process of data in the last
 *		message buffer.
 *		We are assuming that the t_outbuf exists because we are called
 *		only by ldtty_write() after the characters have been sent into
 *		the t_outbuf.
 *		This is done for the track number 139884.
 *
 * RETURNS:	TRUE, if the copying is done,
 *		FALSE, if there is no possibility to do that copy.
 *		The return value must be checked in ldtty_write()
 */
int
ldtty_canputnext(tp, mp, q)
	register struct ldtty *tp;
	register mblk_t	*mp;
	register queue_t *q;
{
	register mblk_t	*message;
	int size, size_outbuf, size_mp;

	/*
	 * if we are not authorized to send the outputs now, returned FALSE,
	 * we must postponed those outputs.
	 */
	if (tp->t_state & (TS_WAITOUTPUT|TS_WAITOUTBUF|TS_TTSTOP))
		return(FALSE);

	/*
	 * if we are not able to send directly the output to the next module, 
	 * now, then returned FALSE, ldtty_start() will be call back latter,
	 * infortunately for the performances point of view.
	 */
	if (!canput(q->q_next)) {
		tp->t_state |= TS_WAITOUTPUT;
		return(FALSE);
	}
	size_outbuf = msgdsize(tp->t_outbuf);
	/*
	 * defect number 147556, size_mp is beeing initialized once.
	 */
	/*
	 * if we have the capicity to copy directly the t_outbuf in the last
	 * message data block, then do it and then send it downstream directly.
	 * Then returns TRUE to indicate that ldtty_write has nothing else 
	 * to do.
	 */
	/*
	 * defect number 147556, db_base is best than b_rptr for the
	 * begenning of the data.
	 */
	if (size_outbuf <= (mp->b_datap->db_lim - mp->b_datap->db_base)){
		message = tp->t_outbuf;

		/* initialize the b_wptr for copy. */
		mp->b_wptr = mp->b_rptr = mp->b_datap->db_base;
		size_mp = 0;

		/* copy all characters from the t_outbuf to the same message */
		while (message) {
			size = message->b_wptr - message->b_rptr;
			bcopy(message->b_rptr, mp->b_wptr, size);
			mp->b_wptr += size;
			message->b_wptr = message->b_rptr;
			message = message->b_cont;
			size_mp += size;
		}
		sysinfo_add(sysinfo.outch, size_mp);
		putnext(q, mp);
		return(TRUE);
	}
	return(FALSE);
}

/*
 * write user data
 * if we completely process the message, free it and return 0
 * if we cannot completely process the message because of flow control,
 * put the unprocessed part of message back on the queue and return -1
 */
 int
ldtty_write(tp, mp)
	register struct ldtty *tp;
	register mblk_t *mp;
{
	register mblk_t *mp1;
	register queue_t *q = WR(tp->t_queue);
	int	error, size;

	if (tp->t_lflag & FLUSHO) {
		freemsg(mp);
		return(0);
	}
	/*
	 * if no output processing required, just forward the message
	 */
	if (!(tp->t_oflag & OPOST)) {
		if (!canput(q->q_next)) {
			putbq(q, mp);
			return(-1);
		}
		/*
		 * send the accumulated message in t_outbuf first
		 */
		if (ldtty_start(tp)) {
			putbq(q, mp);
			return(-1);
		}
		tp->t_rocount = 0;
		/*
		 * Count output bytes
		 */
		size = msgdsize(mp);
		sysinfo_add(sysinfo.outch, size);
		putnext(q, mp);
		return(0);
	}
	/*
	 * multi-byte EUC write processing
	 */
	if (ldtty_mbenabled(tp)) {
		error = euctty_write(tp, mp);
		return(error);
	}
	/*
	 * some output processing required
	 */
	while (mp) {
		int c;
                int cc, ce, i;

		cc = mp->b_wptr - mp->b_rptr;
		while (cc > 0) {
                        if ((tp->t_oflag & OLCUC) ||
                            (tp->t_lflag & XCASE))
                                /* 
                                 * Process all the characters 
                                 * one by one 
                                 */
                                ce = 0;
                        else
                                ce = cc - ldtty_scanc(cc, mp->b_rptr, partab, 
						      (char) 077);
                        tp->t_rocount = 0;
                        if (ce == 0) {
                                c = (unsigned char)*mp->b_rptr;
				if ((c = ldtty_output(tp, c)) >= 0) {
					/* If output buffer is full, send
					 * it down stream and try this char
					 * again, but punt if there's still
					 * some holdup (could be allocb()
					 * failure or flow control).
					 */
                                        if (ldtty_start(tp)
					|| ((c = ldtty_output(tp, c)) >= 0)) {
                                                *mp->b_rptr = (unsigned char)c;
                                                putbq(q, mp);
						return(-1);
                                        }
                                }
                                mp->b_rptr++;
                                cc--;
                        }
                        else {
                                /*
                                 * A bunch of normal characters have
                                 * been found, transfer them en masse
                                 * to the output buffer and continue
                                 * processing at the top of the loop.
                                 * If there are any further characters
                                 * in this message block, the first
                                 * should be a character requiring
                                 * special handling by ldtty_output.
                                 */
                                i = ldtty_b_to_m(mp->b_rptr, ce, tp);
				if ((i == ce) && (tp->t_state&TS_WAITOUTBUF)) {
					/* No mem. for output, return */
					putbq(q, mp);
					return(-1);
				}
                                ce -= i;
                                tp->t_col += ce;
                                mp->b_rptr += ce;
                                cc -= ce;
                                if (i > 0) {
                                        /*
                                         * Output buffer full, send it
                                         */
                                        if (ldtty_start(tp)) {
                                                putbq(q, mp);
						return(-1);
                                        }
                                }
                        }
		}

		mp1 = mp;
		mp = mp->b_cont;
		/*
		 * if that message is the last processed, then try to copy
		 * the t_outbuf in that message to be able to send the
		 * message directly downstream. Else in all other cases we
		 * have to free that message, because we can't reuse it.
		 * Performance improvement, track number 139884.
		 */
		if (mp)
			freeb(mp1);
		else {
			if (ldtty_canputnext(tp, mp1, q))
				return(0);
			else
				freeb(mp1);
		}
	}
	if (ldtty_start(tp) == 0)
		return(0);
	return(-1);
}

/* void
 * ldtty_tunblock(tp)
 *       register struct ldtty *tp;
 * {
 */
	/*
	 * unblock input now that the input queue has gone down
	 * The check for ICANON is to minimize the effect of bug
	 * #5894 until it can be properly fixed (see comment in
	 * ldtty_block()).
	 */
/*	if ((tp->t_state&TS_TBLOCK) &&
 *	    ((tp->t_rawcc < LDTTYLOWAT(tp)) || (tp->t_lflag & ICANON)) &&
 *	    (tp->t_cc[VSTART] != ((cc_t)_POSIX_VDISABLE))) {
 *		if (putctl(WR(tp->t_queue)->q_next, M_STARTI))
 *			tp->t_state &= ~TS_TBLOCK;
 *	}
 * }
 */

/*
 *
 * Function description: ldtty_sendcanon
 *
 *      Send a completed line upstream.
 *      Strip the trailing EOF character if there is one.
 *
 * Arguments:
 *
 *      tp - pointer to current ldtty struct
 *
 * return value:
 *
 *      None
 *
 * Side effects:
 *
 *	Input blockage may be cleared
 *
 */
void
ldtty_sendcanon(tp)
	register struct ldtty *tp;
{
	register mblk_t *mp, *mp1, *mp2, *mp3;
	register u_char c, *cp;
	int cc, found_dsusp, done;

	mp = tp->t_rawbuf;
	mp1 = tp->t_rawtail;
	cc = tp->t_rawcc;

	ldtty_bufclr(tp);
	
	if (cc) {
		c = *(mp1->b_wptr - 1);      /* Look at last character */
		if (CCEQ(tp->t_cc[VEOF], c)) /* If it's EOF */
			mp1->b_wptr--;       /* Remove it */

		/* Search for DSUSP character.  We could use "if"
		 * instead of "while", but then we couldn't use
		 * "break" to jump to the end of the "loop".
		 */
		while (tp->t_state & TS_DSUSP) {

			mp2 = mp;
			found_dsusp = 0;
			while (mp2) {
			
				for (cp = mp2->b_rptr; cp <
				     mp2->b_wptr; cp++) {

					if (CCEQ(tp->t_cc[VDSUSP],
						 *cp)) {
						
						found_dsusp = 1;
						break;
					}
				}
				
				if (found_dsusp) {
					break;
				}
				
				mp2 = mp2->b_cont;
			}

			if (!found_dsusp) {
				tp->t_state &= ~TS_DSUSP;
				break;
			}
				
			/* copy mp2 */
			mp3 = dupmsg(mp2);
			if (!mp3) {
				tp->t_state &= ~TS_DSUSP;
				break;
			}

			/* adjust mp2 to point to everything
			 * up to and not including the DSUSP
			 * character.
			 */
			if (!adjmsg(mp2, cp - mp2->b_wptr)) {
					
				/* In this case, give up and
				 * just send mp upstream.
				 *
				 * THIS SHOULDN'T HAPPEN.
				 */
				freemsg(mp3);
				tp->t_state &= ~TS_DSUSP;
				break;
			}

			/* adjust mp3 to point to everything
			 * after the DSUSP character.
			 */
			if (!adjmsg(mp3, cp - mp2->b_rptr + 1)) {

				/* In this case, give up and just send
				 * the adjusted mp upstream.
				 *
				 * THIS SHOULDN'T HAPPEN.
				 */
				freemsg(mp3);
				tp->t_state &= ~TS_DSUSP;
				break;
			}
					
			/* Now send mp. */
			if (mp->b_rptr != mp->b_wptr) {
				mp->b_flag &= ~MSGCOMPRESS;
				putnext(tp->t_queue, mp);
			} else {
				freemsg(mp);
				mp = 0;
			}
					
			/* Next send the M_SIG SIGTSTP. */
			putctl1(tp->t_queue->q_next, M_SIG, SIGTSTP);
					
			/* Turn off TS_DSUSP now. */
			tp->t_state &= ~TS_DSUSP;
				
			/* Now finish processing mp3. */
			if (mp3->b_rptr != mp3->b_wptr) {
				mp = mp3;
			} else {
				freemsg(mp3);
				mp = 0;
			}
		}

		if (mp) {
			mp->b_flag &= ~MSGCOMPRESS;
			putnext(tp->t_queue, mp);
		}
	}

	if (tp->t_iflag & IXOFF)
        	ldtty_tblock(tp);
}

/*
 *
 * Function description: ldtty_sendraw
 *
 *      Send the raw buffer upstream.
 * 	If there's no current buffer,
 *	allocate an empty one to send.
 *
 * Send the raw buffer upstream.
 *
 * In order to perform IXOFF processing correctly, and in order to cook
 * unread raw data if an application switches into canon mode, the line
 * discipline needs to keep its hands on unread raw data, even once VMIN
 * has been satisfied.  In order to perform select, read, and getmsg
 * processing correctly, the stream head also needs to have its hands on
 * unread raw data, but only once VMIN is satisfied.  In other words, both
 * the line discipline and the stream head would like to "own" the raw
 * input buffer.
 *
 * The solution to this problem is to hold the buffer in the line
 * discipline until VMIN is satisfied, and then maintain the buffer in
 * both places (i.e., in duplicate) until a read or flush brings the
 * buffer size below VMIN, at which time the line discipline reclaims
 * sole ownership.
 *
 * To accomplish this, we always retain unread data, but send a copy to
 * the stream head (note that we won't be called at all until VMIN is
 * satisfied or VTIME has expired).  The copy is marked as special via the
 * MSGNOTIFY bit in the M_DATA message.  When the stream head delivers
 * marked data to a user (e.g., in read(2) processing), it sends an
 * M_NOTIFY message downstream with a count of how many marked characters
 * were read.  This tells the line discipline (ldtty_mnotify()) to chop
 * off the beginning portion of the local copy of the raw buffer
 * corresponding to the data that was just read.  Note that M_NOTIFY
 * is not a standard V.4 message type.
 *
 * Upon each entry to the ldtty_sendraw() function, the raw buffer
 * consists of some number of bytes (possibly 0) of which we've already
 * sent a copy to the stream head, followed by some number of "newly
 * arrived" bytes (also possibly 0).  The t_unsent and t_unsent_ndx
 * fields in the ldtty structure together identify the start of the newly
 * arrived region, which is the portion that ldtty_sendraw() must send.
 *
 *
 * Arguments:
 *
 *      tp - pointer to current ldtty struct
 *
 * return value:
 *
 *	None
 *
 * Side effects:
 *
 *	Input blockage may be cleared.
 *
 */
void
ldtty_sendraw(tp)
        register struct ldtty *tp;
{
        register mblk_t *mp1;
        register mblk_t *mp;
	register mblk_t *mp2;
        register int wanted = tp->t_rawcc - tp->t_shcc;
	register int found_dsusp;
	register u_char *cp;

	mp = tp->t_sparebuf;
        if (tp->t_rawbuf && (wanted > 0)) {
                if (mp && (mp->b_datap->db_size >= wanted)) {
#if MACH_ASSERT
                        tp->t_sparehit++;
                        tp->t_rawtrace = 1;
#endif
                        tp->t_sparebuf = 0;
                } else {
#if MACH_ASSERT
                        tp->t_sparemiss++;
                        tp->t_rawtrace = 2;
#endif
			if (!(mp = allocb(wanted, BPRI_MED)))
                                return;
                }
		assert(mp1 = tp->t_unsent);
		assert(tp->t_unsent_ndx >= (mp1->b_rptr - mp1->b_datap->db_base));
		ldtty_copymsg(tp->t_unsent, tp->t_unsent_ndx, wanted, mp);
                mp1 = tp->t_unsent = tp->t_rawtail;
                tp->t_unsent_ndx = (mp1->b_wptr - mp1->b_datap->db_base);
        } else {
#if MACH_ASSERT
                tp->t_rawtrace = 3;
#endif
		if (tp->t_shcc)
			return;
                /* We need to send a zero length M_DATA in this case */
		if (mp) {
			tp->t_sparebuf = 0;
			mp->b_rptr = mp->b_wptr = mp->b_datap->db_base;
		} else if (!(mp = allocb(0, BPRI_MED)))
			return;
        }

	tp->t_shcc = tp->t_rawcc;
	ASSERT(msgdsize(mp) == wanted);

	while (tp->t_state & TS_DSUSP) { /* handle delayed suspend */

		mp1 = mp;
		found_dsusp = 0;
		while (mp1) {

			for (cp = mp1->b_rptr; cp < mp1->b_wptr; cp++) {
				
				if (CCEQ(tp->t_cc[VDSUSP], *cp)) {

					found_dsusp = 1;
					break;
				}
			}

			if (found_dsusp) {
				break;
			}

			mp1 = mp1->b_cont;
		}

		if (!found_dsusp) {
			tp->t_state &= ~TS_DSUSP;
			break;
		}

		/* copy mp1 */
		mp2 = dupmsg(mp1);
		if (!mp2) {
			tp->t_state &= ~TS_DSUSP;
			break;
		}
			
		/* adjust mp1 to point to everyting up to and not
		 * including the DSUSP character.
		 */
		if (!adjmsg(mp1, cp - mp1->b_wptr)) {

			/* THIS SHOULDN'T HAPPEN.  But if it does,
			 * just send mp upstream.
			 */
			freemsg(mp2);
			tp->t_state &= ~TS_DSUSP;
			break;
		}

		/* adjust mp2 to point to everything after the DSUSP
		 * character.
		 */
		if (!adjmsg(mp2, cp - mp1->b_rptr + 1)) {

			/* THIS SHOULDN'T HAPPEN!  But if it does,
			 * just send mp upstream.
			 */
			freemsg(mp2);
			tp->t_state &= ~TS_DSUSP;
			break;
		}

		/* 165735 : As t_cc[VDSUSP] is not sent to the stream head
		 * update the counters.
		 */
		if (tp->t_shcc) {
			tp->t_shcc--;
			tp->t_rawcc--;
		}

		/* Now send mp. */
		if (mp->b_rptr != mp->b_wptr) {
			putnext(tp->t_queue, mp);
		} else {
			freemsg(mp);
			mp = 0;
		}
		
		/* Now send M_SIG SIGTSTP upstream. */
		putctl1(tp->t_queue->q_next, M_SIG, SIGTSTP);

		/* Turn off TS_DSUSP now. */
		tp->t_state &= ~TS_DSUSP;
		
		/* Now finish processing mp2. */
		if (mp2->b_rptr != mp2->b_wptr) {
			mp = mp2;
		} else {
			freemsg(mp2);
			mp = 0;
		}
	}
	if (mp)
		putnext(tp->t_queue, mp);
        return;
}

/*
 * ldtty_mnotify:
 *
 *      Remove front portion of raw input buffer to sync up w/ stream
 *      head.  See comment above ldtty_sendraw().
 */

void
ldtty_mnotify(struct ldtty *tp, mblk_t *mp)
{
        int     count = *((int *)mp->b_rptr);

	if ((tp->t_rawbuf->b_cont == 0) &&
	    (tp->t_rawbuf->b_wptr == tp->t_rawbuf->b_rptr))
	{
		freemsg(mp);
		return;
	}

/*	assert(count <= tp->t_rawcc); */
/* if count is greater than rawcc or shcc, then assume ldterm has been */
/* flushed while the stream head was reading.... */
	if ((count > tp->t_rawcc) || (count > tp->t_shcc))
	{
		freemsg(mp);
		return;
	}
        if (count == tp->t_rawcc) {
                /* Delete entire raw buffer */
                ldtty_bufreset(tp);
        } else {
		assert(adjmsg(tp->t_rawbuf, count));

                /* Trim off empty messages that resulted from the adjmsg().
                 * Since we've determined count to be less than t_rawcc,
                 * we're "guaranteed" to exit the following while loop
                 * before tp->t_rawbuf becomes NULL.
                 */
                while (tp->t_rawbuf->b_wptr == tp->t_rawbuf->b_rptr) {
			mblk_t *mp1 = tp->t_rawbuf;
			mblk_t *mp2 = tp->t_rawbuf = tp->t_rawbuf->b_cont;

			/*
			 * If we just removed the mblk that unsent points to,
			 * that means that it needs to move up to the next block.
			 * This will actually happen frequently because we tend
			 * to send up entire mblks and so the adjmsg will zap
			 * all those blocks to 0 length.
			 */
			if (tp->t_unsent == mp1) {
				tp->t_unsent = mp2;
				tp->t_unsent_ndx = (mp2->b_rptr -
						    mp2->b_datap->db_base);
			}
                        freeb(mp1);
                }
                tp->t_rawcc -= count;
        }
/*        assert((tp->t_shcc -= count) >= 0); */
	tp->t_shcc -= count;
	freemsg(mp);

        if ((tp->t_state & TS_RAWBACKUP) && (tp->t_shcc < LDTTYLOWAT(tp))) {
                tp->t_state &= ~TS_RAWBACKUP;
                qenable(tp->t_queue);
        }
	if (tp->t_iflag & IXOFF)
        	ldtty_tblock(tp);
}

#ifdef	LDTTY_DEBUG
void ldtty_putstr();
void ldtty_putint();
/*
 * print tty info
 */
 void
ldtty_info(tp)
	register struct ldtty *tp;
{
	ldtty_putstr(tp, "ldtty status\n");
	ldtty_putstr(tp, "t_termios");
	ldtty_putstr(tp, " c_iflag=0x");  ldtty_putint(tp, tp->t_iflag, 16);
	ldtty_putstr(tp, " c_oflag=0x");  ldtty_putint(tp, tp->t_oflag, 16);
	ldtty_putstr(tp, " c_cflag=0x");  ldtty_putint(tp, tp->t_cflag, 16);
	ldtty_putstr(tp, " c_lflag=0x");  ldtty_putint(tp, tp->t_lflag, 16);
	ldtty_putstr(tp, "\n");

	ldtty_putstr(tp, "t_cswidth=");
	ldtty_putint(tp, tp->t_cswidth.eucw[1], 10);  ldtty_putstr(tp, ":");
	ldtty_putint(tp, tp->t_cswidth.scrw[1], 10);  ldtty_putstr(tp, ",");
	ldtty_putint(tp, tp->t_cswidth.eucw[2], 10);  ldtty_putstr(tp, ":");
	ldtty_putint(tp, tp->t_cswidth.scrw[2], 10);  ldtty_putstr(tp, ",");
	ldtty_putint(tp, tp->t_cswidth.eucw[3], 10);  ldtty_putstr(tp, ":");
	ldtty_putint(tp, tp->t_cswidth.scrw[3], 10);  ldtty_putstr(tp, "\n");

	ldtty_putstr(tp, "t_state=0x");
	ldtty_putint(tp, tp->t_state, 16);
	ldtty_putstr(tp, " t_shad_time=");
	ldtty_putint(tp, tp->t_shad_time, 10);
	ldtty_putstr(tp, "\n");
}

 void
ldtty_putstr(tp, s)
	register struct ldtty *tp;
	register char *s;
{
	while (*s)
		ldtty_output(tp, *s++);
}

void
ldtty_putint(tp, x, base)
	register struct ldtty *tp;
	register unsigned int x; /* the int to print */
	register int base;	/* the base of the displayed value */
{
	char info[16];
	register char *p = info;

	if (!x) {
		ldtty_output(tp, '0');
		return;
	}
	while (x) {
		*p++ = "0123456789abcdef"[x % base];
		x /= base;
	}
	while (p > info)
		ldtty_output(tp, *--p);
}
#endif	/* LDTTY_DEBUG	*/

/*
 * Process M_BREAK message from read service routine
 */
 void
ldtty_break(tp, mp)
        struct ldtty *tp;
        mblk_t *mp;
{
        register int c;
        int err;
        int post_wakeup = 0;
        int iflag = tp->t_iflag;
	int chars_processed = 0;

        /*
         * This is either parity error or BREAK character
         * If message doesn't contain an int,
         * assume it's just a plain BREAK event
         * Can't use msgdsize() because it doesn't work
         * on M_BREAK message blocks.
	 * Changes done, for defect number 144999, about assertions 16 and 21
	 * on NIST-PCTS tcsetattr.
         */
	switch(*(enum status *)mp->b_rptr) {
	case break_interrupt:
		c = 0;
		err = TTY_FE;
		break;
	case parity_error:
/*
 * for vsx4 test number 10 for c_iflag.
 */
		if ((iflag&INPCK) == 0) {
			char to_send;

			to_send = *(char *)(mp->b_wptr - 1);
			if (to_send == 0x7f)
				c = to_send|TTY_QUOTE;
			else
			   c = to_send;
			err = 0;
			break;
		}
/*
 * Corrected on 01/20/94 to answer to vsx4 assertions about c_iflag
 * tests number 6...
 * To be improved by both NIST-PCTS and VSX4.
 */
		if ((mp->b_wptr - mp->b_rptr) < sizeof(int)) {
			c = *(char *)(mp->b_rptr);
			err = 0;
			break;
		}
		c = *(char *)(mp->b_wptr - 1);
		err = TTY_PE;
		break;
	case framing_error:
		c = *(int *)mp->b_rptr;
		err = TTY_FE;
		break;
	case overrun:
		ldtty_error(tp->t_devname, ERRID_TTY_OVERRUN, 0);
        	freemsg(mp);
		return;
	default:
		c = *(int *)mp->b_rptr;
		err = TTY_FE;
		break;
	}
        freemsg(mp);

        if (err) {
                if ((err&TTY_FE) && !c) {	/* break */
                        if (iflag&IGNBRK)
                                return;
                        else if (iflag&BRKINT) {
                                ldtty_flush(tp, FREAD|FWRITE, 1);
                                putctl1(tp->t_queue->q_next, M_PCSIG, SIGINT);
                                return;
                        } else
                                goto parmrk;
                } else if (((err&TTY_PE) && (iflag&INPCK)) || (err&TTY_FE)) {
                        if (iflag&IGNPAR)
                                return;
                        else if ((iflag&PARMRK) == 0) {
				ldtty_error(tp->t_devname, ERRID_TTY_PARERR, 0);
                                c = 0;
			}
                        else
                                c |= TTY_QUOTE;
parmrk:                 if (iflag&PARMRK) {
                        	post_wakeup |= ldtty_input(tp, 0377|TTY_QUOTE);
                                post_wakeup |= ldtty_input(tp, 000|TTY_QUOTE);
				chars_processed += 2;
                        }
                }
        }
        post_wakeup |= ldtty_input(tp, c);
	sysinfo_add(sysinfo.rawch, chars_processed + 1);
	if ((post_wakeup & T_POST_FLUSH) == 0)
        	ldtty_post_input(tp, post_wakeup);
}

int
ldtty_mhangup(tp, q, mp)
	register struct	ldtty	*tp;
	register queue_t	*q;
	register mblk_t		*mp;
{
	mblk_t	*new_mp = allocb(2, BPRI_HI);

	if (!new_mp) {
		if (tp->t_hupbid)
			unbufcall(tp->t_hupbid);
		tp->t_hupbid = bufcall(2, BPRI_HI, qenable, q);
		return(0);
	}
	new_mp->b_datap->db_type = M_ERROR;
	*new_mp->b_wptr++ = 0;
	*new_mp->b_wptr++ = EIO;
	putnext(q, new_mp);
	putnext(q, mp);
	return(1);
}


/*
 * process the M_CTL messages from the read put routine
 * or from write put routine (in case of TIOCSTI). The
 * calling put routine has already checked canput(q->q_next).
 * return 1 if message processed, 0 otherwise. The zero return
 * would come from a memory request failing below us.
 */
int
ldtty_mctl(tp, q, mp)
	register struct ldtty *tp;
	register queue_t *q;
	register mblk_t *mp;
{
	register struct iocblk *iocp;
	register int	*ctl;
	int	i;
	speed_t	ospeed, ispeed;

	if ((mp->b_wptr - mp->b_rptr) < sizeof(struct iocblk)) {
		ctl = (int *)mp->b_rptr;
		switch (*ctl) {
		case cts_on:
		case cts_off:
		case dsr_on:
		case dsr_off:
		case ri_on:
		case ri_off:
			freemsg(mp);	/* added for defect number 139905 */
			break;
		case cd_on:
			/*
			 * Defect number 126770: changing & by |.
			 * defect number 138623: send an M_CTL with
			 * M_ERROR NOERROR is the cd is changed.
			 */
			if ((tp->t_state & TS_CARR_ON) == 0) {
				tp->t_state |= TS_CARR_ON;
				putctl2(tp->t_queue, M_ERROR, 0, 0);
			}
			freemsg(mp);
			break;
		case cd_off:
			if (tp->t_state & TS_CARR_ON) {
				tp->t_state &= ~TS_CARR_ON;
				if (tp->t_state & TS_ISOPEN &&
				   ((tp->t_cflag & CLOCAL) == 0)) {
					ldtty_flush(tp, FREAD|FWRITE, 1);
					putctl(tp->t_queue, M_HANGUP);
				}
			}
			freemsg(mp);
			break;
		default:
			putnext(q, mp);
			break;
		}
		return(1);
	}
	iocp = (struct iocblk *)mp->b_rptr;
	switch (iocp->ioc_cmd) {
	case TIOCGETA:
		/*
		 * get the termios structure from the driver
		 */
		if (msgdsize(mp->b_cont) >= sizeof(struct termios)) {
			struct termios *termiosp;

			/*
			 * copy the speeds and cflag from the driver
			 */
			termiosp = (struct termios *)mp->b_cont->b_rptr;
			tp->t_cflag = termiosp->c_cflag;
			tp->u_termios = *termiosp;
			bcopy((caddr_t)tp->u_termios.c_cc,
			      (caddr_t)tp->t_cc, sizeof(tp->t_cc));
		}

		freemsg(mp);
		if (tp->t_on_open) {
		    tp->t_on_open = 0;
		    e_wakeup(&tp->t_event);
		}
		return(1);
	case TIOCGETMODEM:
		/*
		 * get the modem state from the driver
		 */
		if (msgdsize(mp->b_cont) >= sizeof(char)) {

                        char	newstate = *mp->b_cont->b_rptr;

			if (newstate) {		/* carr is on	*/
				/*
				 * Defect 138623.
				 */
				if ((tp->t_state & TS_CARR_ON) == 0)
					tp->t_state |= TS_CARR_ON;
				putctl2(tp->t_queue,M_ERROR, 0, 0);
			}
			else {
				if (tp->t_state & TS_CARR_ON)
					tp->t_state &= ~TS_CARR_ON;
				if (((tp->t_cflag & CLOCAL) == 0) &&
				    ((tp->t_state & TS_ONDELAY) == 0))
					putctl2(tp->t_queue, M_ERROR, 0, EIO);
			}
		}
		freemsg(mp);
		return(1);
	case TCGETX:
		/*
		 * get the tty_control state from the driver
		 */
		tp->t_control = *(struct termiox *)mp->b_cont->b_rptr;
		freemsg(mp);
		return(1);
	case TXTTYNAME:
		/*
		 * get the device name from the driver
		 */
		bcopy(mp->b_cont->b_rptr, tp->t_devname, TTNAMEMAX);
		freemsg(mp);
                return(1);
	case MC_DO_CANON: {
		/*
		 * the pty will be out of remote mode
		 */
                tp->t_state &= ~TS_NOCANON;
		putnext(q, mp);
	}
		return(1);
	case MC_NO_CANON: {
		/*
		 *  the pty will be in remote mode
		 */
                tp->t_state |= TS_NOCANON;
		putnext(q, mp);
	}
		return(1);
	case MC_PART_CANON: {
		/*
		 * get the termios structure from the driver
		 */
		if (msgdsize(mp->b_cont) >= sizeof(struct termios)) {
			struct termios *termiosp;

			termiosp = (struct termios *)mp->b_cont->b_rptr;
			/*
			 * put oflag, iflag and lflag from the termios
			 * structure received from the driver in the internal
			 * termios d_termios structure which represents the
			 * flags which are now supposed to be process by the
			 * driver.
			 */
			tp->d_termios.c_iflag = termiosp->c_iflag;
			tp->d_termios.c_oflag = termiosp->c_oflag;
			tp->d_termios.c_lflag = termiosp->c_lflag;

			/*
			 * oflag, iflag and lflag to be used by ldterm are
			 * old ones from the t_termios structure, with the
			 * received ones whose bits are reseted from the old
			 * one.
			 */
			tp->t_iflag = tp->t_iflag & ~termiosp->c_iflag;
			tp->t_oflag = tp->t_oflag & ~termiosp->c_oflag;
			tp->t_lflag = tp->t_lflag & ~termiosp->c_lflag;
		}
	}
		freemsg(mp);
		return(1);
	case TIOCSETA:
		/*
		 * Pty master want us to change termios settings.
		 */
		if (ldtty_ioctl_ack(tp, q, mp)) {
			freemsg(mp);
			return(1);
		} else
			return(0);
        case TIOCSWINSZ:
		/*
		 * Driver wants us to change window size
		 */
		if (ldtty_swinsz(tp, mp)) {
			freemsg(mp);
			return(1);
		} else
			return(0);
	case TIOCSTI: {
		/*
		 * Simulate typein (sent by upstream character conversion
		 * module)
		 */
		ldtty_sti(tp, mp);
		freemsg(mp);
	}
		return(1);
	case TIOC_REQUEST:
	default:
		putnext(q, mp);
		return(1);
	}
}

/*
 * Allocate an M_IOCTL message from ldterm.
 * Specifically done to resolve a possible problem on close with possible
 * hangup.
 * RETURNS: 0 in case of error in allocation,
 *          mp pointer allocated.
 */
mblk_t *
ldtty_sendioctl(command, size)
        register int command;
        register int size;
{
        register mblk_t *mp;

        mp = allocb(sizeof(struct iocblk), BPRI_MED);
        if (!mp)
                return(0);
        else {
                mblk_t *mp1;
                struct iocblk *iocp;

                mp1 = allocb(size, BPRI_MED);
                if (!mp1) {
                        freemsg(mp);
                        return(0);
                }
                mp->b_datap->db_type = M_IOCTL;
                mp->b_cont = mp1;
                mp->b_wptr = mp->b_rptr + sizeof(struct iocblk);
                iocp = (struct iocblk *)mp->b_rptr;
                iocp->ioc_cmd = command;
                iocp->ioc_count = size;
        }
        return(mp);
}

/*
 * Send an "inquiry" M_CTL message with attached M_DATA block
 * Normally, we will expect some module to reply by filling info
 * into the M_DATA block and sending the message back sometime later.
 */
 int
ldtty_sendctl(q, command, size)
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

/*
 * Attempt to copy cc characters from cp to tp->t_outbuf and return
 * the residual count.  A residual may exist due either to flow control
 * or failed ldtty_getoutbuf().  If it's the latter, our caller
 * will know by the TS_WAITOUTBUF bit in the tp structure.
 */
int
ldtty_b_to_m(cp, cc, tp)
        register unsigned char *cp;
        register int cc;
	struct	ldtty	*tp;
{
        register int nc;
        mblk_t *mp;

	if ((!tp->t_outbuf) && (!ldtty_getoutbuf(tp)))
		return(cc);

	mp = tp->t_outbuf;
        if (cc <= 0)
		return(0);
        nc = MIN(cc, (mp->b_datap->db_lim - mp->b_wptr));
        if (nc > 0) {
                bcopy(cp, mp->b_wptr, nc);
                mp->b_wptr += nc;
                cc -= nc;
        }
	return(cc);
}

int
ldtty_stuffc(c, tp)
register int c;
struct	ldtty	*tp;
{
	mblk_t *mp;

	if (mp = tp->t_rawtail)
		assert(mp->b_datap && mp->b_datap->db_base &&
		       mp->b_wptr && mp->b_datap->db_lim &&
		       mp->b_datap->db_base <= mp->b_wptr &&
		       mp->b_wptr <= mp->b_datap->db_lim);

	if (!mp || mp->b_wptr >= mp->b_datap->db_lim) {
                if (tp->t_sparebuf) {
			assert(tp->t_sparebuf->b_datap &&
			       tp->t_sparebuf->b_datap->db_base);
                        mp = tp->t_sparebuf;
                        tp->t_sparebuf = 0;
                        mp->b_rptr = mp->b_wptr = mp->b_datap->db_base;
                } else {
                        mp = allocb(LDTTYCHUNKSIZE, BPRI_MED);
                        if (!mp)
				return(-1);
                }
		if (tp->t_rawtail)
			tp->t_rawtail->b_cont = mp;
		else
			ldtty_newrawbuf(tp, mp);
                tp->t_rawtail = mp;
        }
	/*
	 * Defect number 161405, the b_wptr was NULL.
	 */
        *mp->b_wptr++ = (unsigned char)c;
        tp->t_rawcc++;
	return(0);
}

int
ldtty_unstuffc(tp)
struct ldtty *tp;
{
	register int c;
	mblk_t *mp = tp->t_rawtail;

	if (tp->t_rawcc == 0)
		c = -1;
	else {
		c = *--mp->b_wptr;
		if (--tp->t_rawcc == 0) {
			/*
       			 * No characters left.
       			 */
			ldtty_bufreset(tp);
		}
		else if (mp->b_wptr <= mp->b_rptr) {
			/*
       			 * Took last character out of this mblk,
       			 * Deallocate it.  Have to walk forward
       			 * through the chain starting with cf
       			 */
			mblk_t *mp1 = tp->t_rawbuf;

			freeb(mp);
			while (mp1->b_cont != mp)
				mp1 = mp1->b_cont;
			mp1->b_cont = NULL;
			tp->t_rawtail = mp1;
		}
	}
	return (c);
}

/*
 *
 * Function description: ldtty_post_input
 *
 *	Perform common operations after looping through ldtty_input.
 * 	Operations are:
 * 		Start output
 * 		Send input upstream (ldtty_wakeup)
 * 		(re)start inter-character timer
 *
 * Arguments:
 *
 *      tp - pointer to current ldtty structure
 * 	post_wakeup - flags that indicate what to do
 *
 * return value:
 *
 *	None
 *
 * Side effects:
 *
 *	Listed under "operations" above
 *
 */
void
ldtty_post_input(tp, post_wakeup)
        struct ldtty *tp;
        int post_wakeup;
{
        if (post_wakeup & T_POST_START)
                /*
                 * Start output
                 */
                (void)ldtty_start(tp);

        if (post_wakeup & T_POST_WAKEUP)
                /*
                 * Send input upstream
                 * No need to set timer in this case
                 */
                ldtty_wakeup(tp);
        else if (post_wakeup & T_POST_TIMER) {
                /*
                 * If we're in non-canonical mode,
                 * and VTIME > 0 and a read() has been issued
                 * then (re)start intercharacter timer
                 */
                if (!(tp->t_lflag & ICANON) &&       /* non-canonical */
                    (tp->t_shad_time > 0) &&         /* and VTIME > 0 */
                    (tp->t_state & TS_VTIME_FLAG)) { /* and read issued */
			ldtty_set_intimeout(tp, tp->t_shad_time);
                }
        }
}

staticf int
ldtty_swinsz(tp, mp)
	struct ldtty *tp;
	mblk_t *mp;
{

	if (bcmp(&tp->t_winsize, (struct winsize *)mp->b_cont->b_rptr,
		 sizeof(struct winsize)))
		if (putctl1(tp->t_queue->q_next, M_PCSIG, SIGWINCH)) {
			tp->t_winsize = *(struct winsize *)mp->b_cont->b_rptr;
			return(1);
		} else
			return(0);
	return(1);
}

staticf void
ldtty_sti(tp, mp)
	struct ldtty *tp;
	mblk_t *mp;
{
	mblk_t *mp1;

	mp1 = unlinkb(mp);
	if (mp1)
		puthere(tp->t_queue, mp1);
}

/*
 * ldtty_copymsg:
 *
 * A version of copymsg() that uses a supplied target mblk rather than
 * allocating a new one, and possibly omits some beginning portion of the
 * source mblk in the copy.  This routine is tailored to the needs of
 * ldtty_sendraw().
 *
 * The caller is responsible for verifying that the supplied target mblk
 * is big enough.  The target's b_cont pointer should be NULL on entry to
 * this routine; we don't do b_cont processing for the target.
 */
void
ldtty_copymsg(from, from_index, from_len, to)
	register mblk_t	*from;
	int	from_index;
	int	from_len;
	register mblk_t	*to;
{
	register int	count;
	int		x;

	ASSERT(to->b_cont == 0);
	to->b_rptr = to->b_wptr = to->b_datap->db_base;
	to->b_flag |= MSGNOTIFY;
	/* Copy the appropriate portion of the first mblk.
	 */
	do {
		count = from->b_wptr - (from->b_datap->db_base + from_index);
		ASSERT(count >= 0);
		ASSERT((to->b_wptr + count) <= to->b_datap->db_lim);
		bcopy(from->b_datap->db_base + from_index, to->b_wptr, count);
		to->b_wptr += count;
		from_len -= count;
		from_index = 0;
	} while (from = from->b_cont);
	ASSERT(from_len == 0);
}


int
ldtty_getoutbuf(tp)
	struct	ldtty	*tp;
{

        if (tp->t_outbuf)
		return(1);

        if (tp->t_state & TS_WAITOUTBUF)
		return(0);

        if (tp->t_outbid)
                unbufcall(tp->t_outbid);

        if (tp->t_outbuf = allocb(tp->t_ohog, BPRI_MED)) {
                tp->t_outbid = 0;
		return(1);
        } else {
                tp->t_state |= TS_WAITOUTBUF;
                tp->t_outbid =
                        bufcall(tp->t_ohog, BPRI_MED, qenable, WR(tp->t_queue));
		return(0);
        }
}

/*
 * include the files to process
 *  BSD4.3 and SVID ioctl's
 *  multi-byte EUC
 */
/*
 *
 * Function description: ldtty_ioctl_bad
 *
 *	Check validity of M_IOCTL message
 *	Allocate mblk for answer if needed
 *
 * Arguments:
 *
 *	mp - pointer to M_IOCTL message
 *
 * return value:
 *
 *	1 - message is bad, error filled in ioc_error
 *	0 - message is ok
 *
 * Side effects:
 *
 *	Error value filled in message
 *	mblk allocated for answer in case of IOC_OUT
 *
 */
staticf int
ldtty_ioctl_bad(mblk_t *mp)
{
	struct iocblk *iocp;
	int cmd, len;

	iocp = (struct iocblk *)mp->b_rptr;
	/*
	 * TRANSPARENT ioctls don't want to do a copyout
	 */
	if (iocp->ioc_count == TRANSPARENT)
		iocp->ioc_count = 0;
	cmd = iocp->ioc_cmd;
	len = (((cmd) >> 16) & IOCPARM_MASK);
	if (cmd & IOC_IN) {
		/*
		 * IOC_IN commands must have an mblk of the proper size
		 * chained to mp->b_cont
		 */
		if ((iocp->ioc_count != len) || (!mp->b_cont)) {
			iocp->ioc_error = EINVAL;
			goto out;
		}
	}
	if (cmd & IOC_OUT) {
		/*
		 * if an IOC_OUT command comes down without
		 * an attached mblk, allocate one for the answer
		 */
		if (!mp->b_cont)
			mp->b_cont = allocb(len, BPRI_MED);
		if (mp->b_cont)
			iocp->ioc_count = len;
		else {
			iocp->ioc_error = ENOMEM;
		}
	}
      out:
	if (iocp->ioc_error) {
		mp->b_datap->db_type = M_IOCNAK;
		iocp->ioc_count = 0;
		iocp->ioc_rval = 0;
		return (1);
	} 
	return (0);
}

void
ldtty_error(name, code, err) 
	char	*name;
	int	code, err;
{
	ERR_REC(sizeof(int))	ldtty_err;
	
	ldtty_err.error_id = code;
	bcopy(name, ldtty_err.resource_name, sizeof(ldtty_err.resource_name));
	*(int *)ldtty_err.detail_data = err;
	errsave(&ldtty_err, sizeof(ldtty_err));
}
