#ifndef lint
static char sccsid[] = "@(#)60 1.16 src/bos/kernext/tty/ldtty_compat.c, sysxtty, bos411, 9428A410j 7/12/94 13:32:56";
#endif
/*
 * COMPONENT_NAME: (sysxtty) Compatibility for BSD or SVID
 *
 * FUNCTIONS: ldtty_compat_to_termios, ldtty_bsd43_ioctl, ldtty_svid_ioctl
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

/*
 * process BSD4.3 and SVID ioctls
 */

#include <sys/types.h>
#include <sys/signal.h>
#include <sys/conf.h>
#include <sys/errno.h>
#include <sys/user.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/file.h>
#include <sys/sleep.h>

#include <sys/stropts.h>
#include <sys/stream.h> 
#include <sys/device.h>
#include <sys/str_tty.h>
#include "ldtty.h"

#define VNOFLSH (NOFLSH>>16) /* no defined in termios.h */

#ifdef _KERNEL
#define VVINTR	0
#define	VVQUIT	1
#define	VVERASE	2
#define VVKILL	3
#define	VVEOF	4
#define	VVMIN	4
#define	VVEOL	5
#define	VVTIME	5
#define	VVEOL2	6
#define VVSWTCH	7
#endif

extern	int	ldtty_compatgetflags();
extern	int	ldtty_compatsetflags();
extern	int	ldtty_compatsetlflags();
extern	int	ldtty_speedtab();

/* COMPAT_BSD_4.3	*/
void ldtty_bsd43_ioctl();
/* COMPAT_BSD_4.3 */
void ldtty_svid_ioctl();
int ldtty_compat_to_termios();

/*
 * ldtty_compat_to_termios
 *
 * convert a BSD or SYSV M_IOCTL message to a termios M_IOCTL message.
 * this way the drivers will only see the termios ioctl's.
 * on the way up, the ldtty has to make sure that the original ioctl goes up.
 */
int
ldtty_compat_to_termios (tp, mp, cmd, termp)
	register struct ldtty *tp;
	register mblk_t *mp;
	register int cmd;
	register struct termios *termp;
{
	register mblk_t *mp1;
	register struct iocblk *iocp;

	iocp = (struct iocblk *)mp->b_rptr;
	/*
	 * save the original M_IOCTL info
	 */
	tp->t_ioctl_cmd = iocp->ioc_cmd;
	/*
	 * allocate the termios data
	 */
	if (!mp->b_cont ||
		mp->b_cont->b_datap->db_size < sizeof(struct termios)) {
		if ((mp1 = allocb(sizeof(struct termios), BPRI_MED)) == NULL) {
			tp->t_ioctl_cmd = 0;
			tp->t_ioctl_data = 0;
			return(-1);
		}
		if (mp->b_cont) {
			tp->t_ioctl_data = 0;
			freemsg(mp->b_cont);
		}
	} else {
		mp1 = mp->b_cont;
		mp1->b_rptr = mp1->b_datap->db_base;
	}

	mp->b_cont = mp1;
	/*
	 * copy the termios info
	 */
	bcopy(termp, mp1->b_rptr, sizeof(struct termios));
	mp1->b_wptr = mp1->b_rptr + sizeof(struct termios);
	/*
	 * fix up the iocblk
	 */
	iocp->ioc_count = sizeof(struct termios);
	iocp->ioc_cmd = cmd;
	return(0);
}

/* COMPAT_BSD_4.3	*/
/*
 * process BSD4.3 compatibility tty ioctl's
 */
void
ldtty_bsd43_ioctl(tp, q, mp)
	register struct ldtty *tp;
	register queue_t *q;
	register mblk_t *mp;
{
	register struct iocblk *iocp;

	iocp = (struct iocblk *)mp->b_rptr;
	switch (iocp->ioc_cmd) {
	case TIOCGETP: {
		register struct sgttyb *sg;

		sg = (struct sgttyb *)mp->b_cont->b_rptr;
		termios_to_sgttyb(&tp->u_termios, sg);
		mp->b_cont->b_wptr = mp->b_cont->b_rptr + sizeof(struct sgttyb);
		ldtty_iocack_msg(iocp, sizeof(struct sgttyb), mp);
		qreply(q, mp);
	}
		break;
	case TIOCSETP:
	case TIOCSETN: {
		int 		cmd;
		struct termios term;
		int pflag = 0;   /* flag to hold compat EVENP/ODDP flags */

		term = tp->u_termios;
		tp->t_flags = ldtty_compatgetflags(term.c_iflag,
						   term.c_lflag,
						   term.c_oflag,
						   term.c_cflag,
						   &pflag);

		sgttyb_to_termios((struct sgttyb *)mp->b_cont->b_rptr,
					&term, &tp->t_flags, pflag);

		cmd = iocp->ioc_cmd == TIOCSETP ? TIOCSETAF : TIOCSETA;

		if (ldtty_compat_to_termios(tp, mp, cmd, &term)) {
			ldtty_iocnak_msg(iocp, ENOMEM, mp);
			qreply(q, mp);
			return;
		}
		putnext(q, mp);
	}
		break;
	case TIOCGETC:
		termios_to_tchars(&tp->u_termios, 
					(struct tchars *)mp->b_cont->b_rptr);
		mp->b_cont->b_wptr = mp->b_cont->b_rptr + sizeof(struct tchars);
		ldtty_iocack_msg(iocp, sizeof(struct tchars), mp);
		qreply(q, mp);
		break;
	case TIOCSETC: {
		struct termios term;

		term = tp->u_termios;
		tchars_to_termios((struct tchars *)mp->b_cont->b_rptr, &term);

		if (ldtty_compat_to_termios(tp, mp, TIOCSETA, &term)) {
			ldtty_iocnak_msg(iocp, ENOMEM, mp);
			qreply(q, mp);
			return;
		}
		putnext(q, mp);
	}
		break;
	case TIOCGLTC:
		termios_to_ltchars(&tp->u_termios,
					(struct	ltchars *)mp->b_cont->b_rptr);
		mp->b_cont->b_wptr = mp->b_cont->b_rptr + 
					sizeof(struct ltchars);
		ldtty_iocack_msg(iocp, sizeof(struct ltchars), mp);
		qreply(q, mp);
		break;
	case TIOCSLTC: {
		struct termios term;

		term = tp->u_termios;
		ltchars_to_termios((struct ltchars *)mp->b_cont->b_rptr, &term);
		if (ldtty_compat_to_termios(tp, mp, TIOCSETA, &term)) {
			ldtty_iocnak_msg(iocp, ENOMEM, mp);
			qreply(q, mp);
			return;
		}
		putnext(q, mp);
	}
		break;
	case TIOCLGET: {
	        int pflag = -1;
		int ret = ldtty_compatgetflags(tp->u_termios.c_iflag, 
					       tp->u_termios.c_lflag,
					       tp->u_termios.c_oflag, 
					       tp->u_termios.c_cflag,
					       &pflag);
		*(int *)mp->b_cont->b_rptr = ret >> 16;
		mp->b_cont->b_wptr = mp->b_cont->b_rptr + sizeof(int);
		ldtty_iocack_msg(iocp, sizeof(int), mp);
		qreply(q, mp);
		break;
	}
	case TIOCLBIS:
	case TIOCLBIC:
	case TIOCLSET: {
                int pflag = 0;
		register int com = iocp->ioc_cmd;
		register newflags = *(int *)mp->b_cont->b_rptr;
		struct termios term;

		term = tp->u_termios;
		tp->t_flags = ldtty_compatgetflags(term.c_iflag,
						   term.c_lflag,
						   term.c_oflag,
						   term.c_cflag,
						   &pflag);

		flags_to_termios(com, newflags, &term, &tp->t_flags);

		if (ldtty_compat_to_termios(tp, mp, TIOCSETA, &term)) {
			ldtty_iocnak_msg(iocp, ENOMEM, mp);
			qreply(q, mp);
			return;
		}
		putnext(q, mp);
	}
		break;
	default:
		ldtty_iocnak_msg(iocp, EINVAL, mp);
		qreply(q, mp);
	}
}
/* COMPAT_BSD_4.3 */

/* SVID starts */
/*
 * process SVID tty ioctl's
 */
void
ldtty_svid_ioctl(tp, q, mp)
	register struct ldtty *tp;
	register queue_t *q;		/* write q */
	register mblk_t *mp;
{
	register struct iocblk *iocp;
	int	err;

	iocp = (struct iocblk *)mp->b_rptr;
	switch (iocp->ioc_cmd) {
	case TCXONC: {
		switch (*(int *)mp->b_cont->b_rptr) {
		case TCOOFF:
			/*
			 * should be the same as TIOCSTOP
			 */
			if (tp->t_iflag & (IXON | IXANY)) {
			  /* ldterm responsible for flow control */
			  if ((tp->t_state & TS_TTSTOP) == 0) {
			    tp->t_state |= TS_TTSTOP;
			    putctl(WR(tp->t_queue)->q_next, M_STOP);
			  }
			} else  /* driver responsible for flow control */
				putctl(WR(tp->t_queue)->q_next, M_STOP);
			
			break;
		    case TCOON:
			/*
			 * should be the same as TIOCSTART
			 */
			if (tp->t_iflag & (IXON | IXANY)) {
			  /* ldterm responsible for flow control */
			  if (tp->t_state & TS_TTSTOP) {
			    tp->t_state &= ~TS_TTSTOP;
			    putctl(WR(tp->t_queue)->q_next, M_START);
			    if (!(tp->d_termios.c_lflag & FLUSHO)) {
			      tp->t_lflag &= ~FLUSHO;
			      tp->u_termios.c_lflag &= ~FLUSHO;
			    }
			    err = ldtty_start(tp);
			  }
			} else { /* driver responsible for flow control */
			  putctl(WR(tp->t_queue)->q_next, M_START);
			  if (!(tp->d_termios.c_lflag & FLUSHO)) {
			    tp->t_lflag &= ~FLUSHO;
			    tp->u_termios.c_lflag &= ~FLUSHO;
			  }
			  err = ldtty_start(tp);
			}
			break;
		    case TCIOFF:
			/*
			 * if ldterm is not responsible for input flow 
			 * control
			 */
			if (!(tp->t_iflag & IXOFF)) {
			  putctl(q->q_next, M_STOPI);
			  break;
			}
			if (!(tp->t_state & TS_TBLOCK) &&
			    tp->t_cc[VSTOP] != _POSIX_VDISABLE &&
			    putctl(q->q_next, M_STOPI)) {
			    tp->t_state |= TS_TBLOCK;
			}
			break;
		    case TCION:
			/*
			 * if ldterm is not responsible for input flow 
			 * control
			 */
			if (!(tp->t_iflag & IXOFF)) {
			  putctl(q->q_next, M_STARTI);
			  break;
			}
			if ((tp->t_state & TS_TBLOCK) &&
			    tp->t_cc[VSTART] != _POSIX_VDISABLE &&
			    putctl(q->q_next, M_STARTI)) {
			    tp->t_state &= ~TS_TBLOCK;
			}
			break;
		    default:
			ldtty_iocnak_msg(iocp, EINVAL, mp);
			qreply(q, mp);
			return;
		    }
		ldtty_iocack_msg(iocp, 0, mp);
		qreply(q, mp);
		break;
	    }

	case TCFLSH: {
		register int flags;

		switch (*(int *)mp->b_cont->b_rptr) {
		case TCIFLUSH:
			flags = FREAD;
			break;
		case TCOFLUSH:
			flags = FWRITE;
			break;
		case TCIOFLUSH:
			flags = FREAD | FWRITE;
			break;
		default:
			flags = -1;
			ldtty_iocnak_msg(iocp, EINVAL, mp);
			qreply(q, mp);
			return;
		}
		if (flags != -1)
			ldtty_flush(tp, flags, 1);
		ldtty_iocack_msg(iocp, 0, mp);
		qreply(q, mp);
	}
		break;

	case TCGETA: {
		register struct termio *termiop;

		termiop = (struct termio *)mp->b_cont->b_rptr;
		termiop->c_iflag = tp->u_termios.c_iflag & 0xffff;
		termiop->c_oflag = tp->u_termios.c_oflag & 0xffff;
		termiop->c_cflag = tp->u_termios.c_cflag & 0xffff;
		termiop->c_lflag = tp->u_termios.c_lflag & 0xffff;
		if (tp->u_termios.c_lflag & NOFLSH) {
			termiop->c_lflag |= VNOFLSH;
		}
		termiop->c_line = 0;	/* this is STREAMS */
		termiop->c_cc[VVINTR] = tp->t_cc[VINTR];
		termiop->c_cc[VVQUIT] = tp->t_cc[VQUIT];
		termiop->c_cc[VVERASE] = tp->t_cc[VERASE];
		termiop->c_cc[VVKILL] = tp->t_cc[VKILL];
		termiop->c_cc[VVEOL2] = tp->t_cc[VEOL2];
		termiop->c_cc[VVEOL] = tp->t_cc[VEOL];
		termiop->c_cc[VVEOF] = tp->t_cc[VEOF];
		mp->b_cont->b_wptr = mp->b_cont->b_rptr + sizeof(struct termio);
		ldtty_iocack_msg(iocp, sizeof(struct termio), mp);
		qreply(q, mp);
	}
		break;
	case TCSETA:
	case TCSETAW:
	case TCSETAF: {
		register struct termio *termiop;
		struct termios term;
		register int speed, i, cmd;

		/* Note that since the termio structure uses shorts and */
		/* the termios structure uses longs any flag set in the */
		/* upper half of the word will be cleared. However, this */
		/* is the correct behavior. Note that we have to move */
		/* around the NOFLSH flag. */
		termiop = (struct termio *)mp->b_cont->b_rptr;
/*
 * The termios structure term must be initialized to the u_termios structure
 * before to include the changes decided by the user via the TCSETA ioctl.
 */
		bzero(&term, sizeof(struct termios));
		bcopy(&tp->u_termios, &term, sizeof(struct termios));

		term.c_iflag = (term.c_iflag&0xffff0000)|
				(termiop->c_iflag&0x0000ffff);
		term.c_oflag = (term.c_oflag&0xffff0000)|
				(termiop->c_oflag&0x0000ffff);
		term.c_cflag = (term.c_cflag & 0xffff0000 & ~_CIBAUD)|
				(termiop->c_cflag&0x0000ffff);
	
		/*
		 * term.c_cflag = termiop->c_cflag & 0x0000ffff;
		 * ask_for_speed = compatspcodes[termiop->c_cflag & 0xf];
		 * cfsetispeed(&term, ask_for_speed);
		 * cfsetospeed(&term, ask_for_speed);
		 */

		term.c_lflag = (term.c_lflag&0xffff0000)|
				((termiop->c_lflag&0x0000ffff) & ~VNOFLSH);
		/* term.c_lflag = termiop->c_lflag & ~VNOFLSH; */
		 if (termiop->c_lflag & VNOFLSH) {
		 	term.c_lflag |= NOFLSH;
        	 }
		 
		/* Initialize the whole termios cc array since its bigger */
		for (i = 0; i < NCCS; i++) {
			term.c_cc[i] = tp->t_cc[i];
		}
		term.c_cc[VINTR] = termiop->c_cc[VVINTR];
		term.c_cc[VQUIT] = termiop->c_cc[VVQUIT];
		term.c_cc[VERASE] = termiop->c_cc[VVERASE];
		term.c_cc[VKILL] = termiop->c_cc[VVKILL];
		term.c_cc[VEOL2] = termiop->c_cc[VVEOL2];
		if (termiop->c_lflag & ICANON) {
			term.c_cc[VEOL] = termiop->c_cc[VVEOL];
			term.c_cc[VEOF] = termiop->c_cc[VVEOF];
		} else {
			term.c_cc[VMIN] = termiop->c_cc[VMIN];
			term.c_cc[VTIME] = termiop->c_cc[VTIME];
		}
		switch (iocp->ioc_cmd) {
		case TCSETAW:
			cmd = TIOCSETAW;
			break;
		case TCSETAF:
			cmd = TIOCSETAF;
			break;
		case TCSETA:
			cmd = TIOCSETA;
			break;
		}

		if (ldtty_compat_to_termios(tp, mp, cmd, &term)) {
			ldtty_iocnak_msg(iocp, ENOMEM, mp);
			qreply(q, mp);
			return;
		}
		putnext(q, mp);
	}
		break;
	default:
		ldtty_iocnak_msg(iocp, EINVAL, mp);
		qreply(q, mp);
	}
}
/* SVID ends */
