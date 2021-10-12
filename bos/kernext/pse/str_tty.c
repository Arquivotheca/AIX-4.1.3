static char sccsid[] = "@(#)32	1.29.1.6  src/bos/kernext/pse/str_tty.c, sysxpse, bos41J, 9516A_all 4/10/95 13:49:23";
/*
 * COMPONENT_NAME: SYSXPSE - STREAMS framework
 *
 * FUNCTIONS:      sth_ttyopen
 *                 sth_ttyclose
 *                 sth_pgsignal
 *                 sth_isctty 
 *                 osr_revoke
 *                 osr_tiocgpgrp
 *                 osr_tiocgsid
 *                 osr_tiocsctty
 *                 osr_tiocknosess
 *                 osr_tiocknopgrp
 *                 osr_bgcheck
 *                 sth_register
 *                 sth_unregister
 *                 sth_query
 *                 osr_tctrust
 *                 osr_tcqtrust
 *		   osr_tcsak
 *		   osr_tcqsak
 *                 osr_tioccons
 *		   osr_tcskep
 *		   osr_kepcheck
 *		   osr_tcxonc
 * 		   sth_tiocspgid
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
 * OSF/1 1.2
 */
/*
 * FILE:	str_tty.c - special stream head activities for TTY support.
 *
 * These activities are all related to the keyword "controlling tty". The
 * System V.3 STREAMS specification exposes the related data structures
 * and their semantics to the module, and lets it deal with it. In OSF/1
 * (and supposedly in V.4, too) we find a slightly changed data structure,
 * in order to implement POSIX standards. Therefore, we implement the
 * V.4 definition instead, PLUS what is needed to support OSF/1 ioctl's.
 *
 * The code which does the actual work here is copied from the traditional
 * TTY driver, and should be kept in sync with it.
 */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/errno.h>
#include <sys/syspest.h>
#undef	T_IDLE
#include <sys/malloc.h>	/* for pinned_heap declaration	*/
#include <sys/sleep.h>
#include <sys/var.h>
#include <sys/signal.h>
#include <sys/stropts.h>

#include <sys/console.h>                /* for TIOCCONS support */
#include <sys/uio.h>
#include <pse/str_stream.h>
#include <pse/str_ioctl.h>
#include <pse/mi.h>
#include <sys/str_tty.h>
#include <pse/str_select.h>

#define	PID_RSVRD	PID_MAX
#define	EVENT_NEVER	0x0627

int	sth_query(int, int);
int	sth_register(STHP);
void	sth_unregister(STHP);

#ifdef TTYDBG
#define SHTTYNAME "shtty"

static  int     shtty_conf_err = 0;  /* global for debug purpose */
static  int     shtty_reg_err = 0;   /* global for debug purpose */

static  struct  str_module_conf shtty_str_module_conf = {
                       SHTTYNAME, 's', (int(*)())STR_TTY_PDPF
};
static  struct  tty_to_reg shtty_tty_to_reg;
#endif

/*
 * ROUTINE:
 *	sth_ttyopen	- stream head activities after opening a tty
 *
 * PARAMETERS:
 *	sth		- the stream head
 *      flag		- O_NOCTTY or not
 *
 * RETURN VALUE:
 *	0 if success.
 *	ENOMEM if sth_register() is not successfull.
 *
 * DESCRIPTION:
 *	This routine is called when the stream head realizes that the
 *	stream acts as a terminal. This is the result of the module
 *	sending an M_SETOPTS message containing the SO_ISTTY flag
 *	during the qi_qopen processing.
 *
 *	The list of special activities needed at this time has currently
 *	only one element: the allocation of this stream as a controlling
 *	TTY. This is basically the same work, as we do it for the explicit
 *	TIOCSCTTY ioctl (see below), but in addition we need to check some
 *	more conditions.
 */

int
sth_ttyopen (sth, flag)
	STHP		sth;
	int		flag;
{

	int	error = 0;
	/*
	 * registered this streamhead in the general table of streamhead
	 * for tty, get the identificator for the tty.
	 */
	if (!sth->shttyp){
		if (error = sth_register(sth))
			return(error);
#ifdef TTYDBG
		/* ttydbg - open - */
		shtty_tty_to_reg.dev = sth->sth_dev;
		shtty_tty_to_reg.ttyname[0] = '\0';
		bcopy(SHTTYNAME, &shtty_tty_to_reg.name, sizeof(SHTTYNAME));
		shtty_tty_to_reg.private_data = sth->shttyp;

		shtty_reg_err = tty_db_open(&shtty_tty_to_reg);
#endif
	}
	if (!(flag & O_NOCTTY) &&!(sth->shttyp->shtty_pgrp)) {
		pid_t	cursid = kgetsid(0);
		pid_t	curpgrp= getpgrp();
		struct	ttyinfo	get_ttyinfo;

		/*
		 * Gets the ttyinfo from the user structure.
		 */
		getctty(&get_ttyinfo);
		if (is_sessl(0) &&
		    cursid	&&
		    !get_ttyinfo.ti_ttyp &&
		    !(sth->shttyp->shtty_sid)) {
			sth->sth_flags |= F_STH_CTTY;
			sth->shttyp->shtty_sid = cursid;
			sth->shttyp->shtty_pgrp = curpgrp;
			get_ttyinfo.ti_ttysid = (int)&sth->shttyp->shtty_sid;
			get_ttyinfo.ti_ttyp = (int)&sth->shttyp->shtty_pgrp;
			get_ttyinfo.ti_ttyid = (int)sth->shttyp;
			get_ttyinfo.ti_ttyd = sth->sth_dev;
			get_ttyinfo.ti_ttyf = sth_query;
			/*
			 * Set new tty informations in the user structure.
			 */
			setctty(&get_ttyinfo);
			/*
			 * Now this is a controlling tty.
			 */
		}
	}

	return(error);
}

/*
 * ROUTINE:
 *	sth_ttyclose	- stream head activities after closing a tty
 *
 * PARAMETERS:
 *	sth	- streamhead structure pointer.
 *
 * RETURN VALUE:
 *	(none)
 *
 * DESCRIPTION:
 *	Deallocation of a controlling TTY. Currently, this only happens
 *	during close. This way, we at least don't lose data structures.
 */

void
sth_ttyclose(sth)
	STHP		sth;
{
	struct	ttyinfo	get_ttyinfo;
	struct      tcons_info tcsav;	/* console tee info save area */
	struct      uio     uio_tmp;	/* copy of uio structure        */
	struct      iovec   iov_tmp;	/* iovec structure for single iovec */

	tcsav.tcons_devno = sth->sth_dev;
	tcsav.tcons_chan = 0;

#ifdef TTYDBG
        /* ttydbg - close - */
        shtty_tty_to_reg.dev = sth->sth_dev;
        shtty_tty_to_reg.ttyname[0] = '\0';
        bcopy(SHTTYNAME, &shtty_tty_to_reg.name, sizeof(SHTTYNAME));
        shtty_tty_to_reg.private_data = sth->shttyp;

        shtty_reg_err = tty_db_close(&shtty_tty_to_reg);
#endif

	if (sth->shttyp->shtty_flags & F_SHTTY_CONS) {
		sth->shttyp->shtty_flags &= ~F_SHTTY_CONS;
						/* console redirection is */
						/* not active.		  */
	        uio_tmp.uio_iov = &iov_tmp;
        	uio_tmp.uio_xmem = (struct xmem *) NULL;
        	uio_tmp.uio_iovcnt = 1;
        	uio_tmp.uio_segflg = UIO_SYSSPACE;
        	uio_tmp.uio_resid = sizeof (struct tcons_info);
        	iov_tmp.iov_base = (char *)&tcsav;
        	iov_tmp.iov_len = sizeof (struct tcons_info);
        	(void) conconfig((dev_t)0,CHG_CONS_TEE, &uio_tmp);
	}

	if ((sth->sth_flags & F_STH_ISATTY) && (sth->sth_flags & F_STH_CTTY)) {
		sth->sth_flags &= ~F_STH_CTTY;
		sth->shttyp->shtty_sid = 0;
		sth->shttyp->shtty_pgrp = 0;
		/*
		 * Gets the ttyinfo from the user structure.
		 */
		getctty(&get_ttyinfo);

		get_ttyinfo.ti_ttysid = 0;
		get_ttyinfo.ti_ttyid = 0;
		get_ttyinfo.ti_ttyp = 0;
		get_ttyinfo.ti_ttyf = 0;
		get_ttyinfo.ti_ttyd = NODEVICE;
		/*
		 * Sets those tty informations in the user structure.
		 */
		setctty(&get_ttyinfo);
	}
	/*
	 * Deleted this streamhead tty entry from the table.
	 */
	sth_unregister(sth);
	return;
		
}

/*
 * ROUTINE:
 *	sth_pgsignal - send a signal to this stream's process group
 *
 * PARAMETERS:
 *	sth	- stream head pointer
 *	sig	- signal number to send
 *
 * RETURN VALUE:
 *	(none)
 *
 * DESCRIPTION:
 *	Sends the indicated signal to the process group which has this
 *	stream as a controlling tty. Since we don't necessarily
 *	get called when we are no longer a CTTY, we have to check our
 *	data first.
 *
 *	For use by stream head only - modules must send M_SIG message.
 *	Caller must alrady have acquired stream head.
 */

void
sth_pgsignal(sth, sig)
        STHP    sth;
        int     sig;
{
        /*
         * The SIGSAK signal must be sent to the registered pid.
         */
        if (sig == SIGSAK)
                if (sth->shttyp->shtty_flags & F_SHTTY_SAK)
                        pidsig(sth->shttyp->shtty_tsm, sig);
                else
                        return;
        else if (sth->shttyp->shtty_pgrp && sth->shttyp->shtty_sid) {
                        if (sig == SIGHUP)
                                pidsig(sth->shttyp->shtty_sid, sig);
                        else
                                pgsignal(sth->shttyp->shtty_pgrp, sig);
                }
}

/*
 * NAME:	sth_isctty()
 *
 * PARAMETERS:
 *	p	- process identificator on which we send signal
 *	sth	- streamhead structure pointer
 *
 * CALL:	called by pse_ioctl in case of TIOCSTI command.
 *
 * FUNCTION:	Search the info about controlling tty.
 *
 * RETURN:	0 if this one is not a controlling tty
 *		1 if this one is a controlling tty.
 */
int
sth_isctty(p, sth)
	pid_t	p;   
	STHP	sth; 
{
	return ((kgetsid(p) == sth->shttyp->shtty_sid) ? 1 : 0);
}

/*
 * NAME:	osr_revoke()
 *
 * PARMETERS:
 *	osr	- OSR structure pointer
 *
 * CALL:	call by the streamhead in case of of revoke() or frevoke()
 *		syscalls.
 *
 * FUNCTION:	
 *		setups the process group correctly.
 *
 * RETURNS:	0 if succedeed.
 *		ENOTTY if not a controlling tty.
 */
int
osr_revoke(osr)
	OSRP	osr;
{
	STHP	sth = osr->osr_sth;
	int	flag = osr->osr_ioctl_arg1;
	void osrq_sigkill();
	struct	ttyinfo	get_ttyinfo;

	if (!sth->shttyp || !(sth->sth_flags & F_STH_CTTY))
		return(ENOTTY);
	/*
	 * Kill all threads sleeping for that tty.
	 */
	osrq_sigkill(&sth->sth_ioctl_osrq);
	osrq_sigkill(&sth->sth_read_osrq);
	osrq_sigkill(&sth->sth_write_osrq);
	/*
	 * get all the informations from the user structure concerning that
	 * tty.
	 */
	getctty(&get_ttyinfo);
	/*
	 * mark sth as the controlling tty for NO session. This will prohibit
	 * attempts to access the terminal through /dev/tty. sysck() checks
	 * that the terminal specified by <u.u_ttyd> is the controlling 
	 * terminal for the process's session id.
	 */
	sth->shttyp->shtty_pgrp = 0;
	sth->shttyp->shtty_sid = 0;
	sth->sth_flags &= ~F_STH_CTTY;

	/*
	 * frevoke() by a process: 1) capable of becoming a process group
	 * leader, 2) with no controlling terminal, or with <sth> as the
	 * controlling terminal, results in the calling process being a
	 * process group leader with <tp> as the controlling terminal.
	 */
	if (flag &&
	    (is_sessl(0)) &&
	    (!get_ttyinfo.ti_ttyp || 
	     (get_ttyinfo.ti_ttyp == (int)&sth->shttyp->shtty_pgrp))) {
		sth->shttyp->shtty_sid = kgetsid(0);
		sth->shttyp->shtty_pgrp = getpgrp();
		sth->sth_flags |= F_STH_CTTY;
		get_ttyinfo.ti_ttysid = (int)&sth->shttyp->shtty_sid;
		get_ttyinfo.ti_ttyp = (int)&sth->shttyp->shtty_pgrp;
		get_ttyinfo.ti_ttyid = (int)sth->shttyp;
		get_ttyinfo.ti_ttyd = sth->sth_dev;
		get_ttyinfo.ti_ttyf = sth_query;
		get_ttyinfo.ti_ttyd = NODEVICE;
		get_ttyinfo.ti_ttympx = 0;
	} else if (!get_ttyinfo.ti_ttyd) 
		get_ttyinfo.ti_ttyp = NULL;
		get_ttyinfo.ti_ttyd = NODEVICE;
	/*
	 * sets the tty informatios in the user structure.
	 */
	setctty(&get_ttyinfo);
	return(0);
}


/*
 * NAME:	osr_tiocgpgrp()
 *
 * PARAMETERS:
 *	osr	- osr structure pointer.
 *
 * FUNCTION:	To get the process group identificator for this tty.
 *		Answered the ioctl TIOCGPGRP and TXGPGRP.
 *
 * RETURNS:	ENOTTY if not a controlling tty.
 *		0 if succedeed.
 */
OSR_STATUS
osr_tiocgpgrp(osr)
	OSRP	osr;
{
	STHP		sth = osr->osr_sth;
	int		cmd = osr->osr_ioctl_arg2;
	struct	ttyinfo	get_ttyinfo;

	/*
	 * Gets ttyinfo from user structure.
	 */
	getctty(&get_ttyinfo);
	if (cmd == TXGPGRP)
		if ((get_ttyinfo.ti_ttyp != (int)&sth->shttyp->shtty_pgrp) ||
		    !sth->shttyp->shtty_sid)
			return(ENOTTY);

	if (cmd == TIOCGPGRP)
		if (!get_ttyinfo.ti_ttyp || !sth->shttyp->shtty_pgrp)
			return(ENOTTY);

 	osr->osr_ioctl_arg0_len = sizeof (int);
	*(int *)osr->osr_ioctl_arg0p = sth->shttyp->shtty_pgrp;
	return(0);
}

/*
 * NAME:	osr_tiocgsid()
 *
 * PARAMETERS:
 *	osr	- osr structure pointer.
 *
 * FUNCTION:	get the session identificator for this tty. Answered the
 *		TIOCGSID ioctl.
 *
 * RETURNS:	ENOTTY if it is not a controlling tty
 *		0 if succedeed.
 */
OSR_STATUS
osr_tiocgsid(osr)
	OSRP	osr;
{
	STHP		sth = osr->osr_sth;

	if (!sth_isctty(kgetsid(0), sth))
		return(ENOTTY);

	osr->osr_ioctl_arg0_len = sizeof (pid_t);
	*(int *)osr->osr_ioctl_arg0p = sth->shttyp->shtty_sid;
	return(0);
}

/*
 * NAME:	osr_tiocsctty()
 *
 * PARAMETERS:
 *	osr	- osr structure pointer.
 *
 * FUNCTION:	Sets the tty as a controlling tty. Answered the TIOCSCTTY ioctl.
 *
 * RETURNS:	ENOTTY if it is not a tty.
 *		EPERM if it is not a session leader or not in the same session.
 *		0 if succedeed.
 */
OSR_STATUS
osr_tiocsctty(osr)
	OSRP	osr;
{
	STHP		sth = osr->osr_sth;
	pid_t		curpid = getpid();
	pid_t		cursid = kgetsid(0);
	struct	ttyinfo	get_ttyinfo;

	if ((sth->sth_flags & F_STH_ISATTY) == 0) {
		return(ENOTTY);
	}

	/*
	 * Gets ttyinfo from user structure.
	 */
	getctty(&get_ttyinfo);
        if (!is_sessl(curpid) ||
            !curpid ||
            (get_ttyinfo.ti_ttysid && 
	    (get_ttyinfo.ti_ttysid != (int)&sth->shttyp->shtty_sid)))
                return(EPERM);

        sth->sth_flags |= F_STH_CTTY;
        sth->shttyp->shtty_sid = cursid;
	sth->shttyp->shtty_pgrp = getpgrp();
        get_ttyinfo.ti_ttysid = (int)&sth->shttyp->shtty_sid;
        get_ttyinfo.ti_ttyp = (int)&sth->shttyp->shtty_pgrp;
        get_ttyinfo.ti_ttyf = sth_query;
	/*
	 * Sets the tty informations in the user structure.
	 */
       	setctty(&get_ttyinfo);
	return(0);
}

/*
 * NAME:	osr_tiocknosess()
 *
 * PARAMETERS:
 *	osr	- osr structure poiter.
 *
 * FUNCTION:	Sets the session to null for that tty. Answered to the
 *		TIOCKNOSESS ioctl.
 *
 * RETURNS:	EINVAL if the user session does not match the tty session.
 *		0 if succedeed.
 */
OSR_STATUS
osr_tiocknosess(osr)
	OSRP	osr;
{
	STHP    sth = osr->osr_sth;
	struct	ttyinfo	get_ttyinfo;

	/*
	 * Gets ttyinfo from user structure.
	 */
	getctty(&get_ttyinfo);
	if (get_ttyinfo.ti_ttysid == (int)&sth->shttyp->shtty_sid)
		sth->shttyp->shtty_sid = 0;
	else
		return(EINVAL);
	return(0);
}

/*
 * NAME:	osr_tiocknopgrp()
 *
 * PARAMETERS:	
 *	osr	- osr structure pointer.
 *
 * FUNCTION:	Sets the tty process group identificator to 0. Answered the
 *		TIOCKNOPGRP ioctl.
 *
 * RETURNS:	EINVAL if the user process group does not match the tty
 *			process group.
 *		0 if succedeed.
 */
OSR_STATUS
osr_tiocknopgrp(osr)
	OSRP	osr;
{
	STHP    sth = osr->osr_sth;
	struct	ttyinfo	get_ttyinfo;
	
	/*
	 * Gets ttyinfo from user structure.
	 */
	getctty(&get_ttyinfo);
	if (get_ttyinfo.ti_ttyp == (int)&sth->shttyp->shtty_pgrp)
		sth->shttyp->shtty_pgrp = 0;
	else
		return(EINVAL);
	return(0);
}


static int do_read_job_control(STHP sth)
{
	pid_t		cursid = kgetsid(0);
	pid_t		curpgrp = getpgrp();
	struct ttyinfo	get_ttyinfo;

	getctty(&get_ttyinfo);
	if ((get_ttyinfo.ti_ttyp == (int)&sth->shttyp->shtty_pgrp)
	    && (curpgrp != sth->shttyp->shtty_pgrp)) {
		if (cursid != sth->shttyp->shtty_sid) {
			pgsignal(curpgrp, SIGHUP);
			return (-2);    /* read denied */
		}
		/*
		 * First check if it is a background process.
		 */
		if (is_blocked(SIGTTIN) ||
		    is_ignored(SIGTTIN) || is_orphan(0))
			return (EIO);
		pgsignal(curpgrp, SIGTTIN);
		return ERESTART;
	}
	return(0);		/* read allowed */
}

static int do_ioctl_job_control(STHP sth)
{
	pid_t		cursid = kgetsid(0);
	pid_t		curpgrp = getpgrp();
	struct ttyinfo	get_ttyinfo;

	getctty(&get_ttyinfo);
	if (get_ttyinfo.ti_ttyp == (int)&sth->shttyp->shtty_pgrp) {
		if (cursid != sth->shttyp->shtty_sid) {
			/*
			 * Session of caller has relinquished its
			 * controlling tty
			 */
			pgsignal(curpgrp, SIGHUP);
			return (ENOTTY); /* access denied; returning
					    ENOTTY as specified by POSIX */
		}
		if ((curpgrp != sth->shttyp->shtty_pgrp) &&
		    !is_ignored(SIGTTOU) && !is_blocked(SIGTTOU)) {
			pgsignal(curpgrp, SIGTTOU);
			return ERESTART;
		}
	}
	return(0);
}

static int do_write_job_control(STHP sth)
{
	pid_t cursid = kgetsid(0);
	pid_t curpgrp = getpgrp();
	struct ttyinfo get_ttyinfo;

	getctty(&get_ttyinfo);
	if ((get_ttyinfo.ti_ttyp == (int)&sth->shttyp->shtty_pgrp) &&
	    (curpgrp != sth->shttyp->shtty_pgrp)) {
		if (cursid != sth->shttyp->shtty_sid) {
			pgsignal(curpgrp, SIGHUP);
			return (EIO);   /* access denied */
		}
		if ((sth->sth_flags & F_STH_TOSTOP) &&
		    !is_blocked(SIGTTOU) &&
		    (is_caught(SIGTTOU)  || !is_ignored(SIGTTOU))) {
			if (is_orphan(0)) 
				return(EIO);
			pgsignal(curpgrp, SIGTTOU);
			return EINTR;
		}
	}
	return (0);			/* allow, access not denied */
}

/*
 * NAME:	osr_bgcheck()
 *
 * PARAMETERS:
 *	osr	- osr structure pointer
 *
 * FUNCTION:	check for attempt by background process to access
 *		its controlling terminal. The call is originated by a pse_read
 *		pse_write or pse_ioctl. The caller's process group may be
 *		signalled in some cases.
 *
 * RETURNS:	EIO if process state prevents signalling that job control
 *			shell needs (I/O should be failed)
 *
 *		for ioctl side :
 *			 0      success, access is allowed
 *			-1      access is denied, SIGTTOU posted to 
 *				process group of caller
 *			EINTR   access denied, a signal has interrupted 
 *				the posting of SIGTTOU
 *		for read side : 
 *			 0      success, read is allowed
 *			-1      read is denied, SIGTTIN posted to 
 *				process group of caller
 *			EIO     read denied, process is in an orphaned 
 *				background pgrp
 *			EINTR   read denied, a signal has interrupted
 *				the posting of SIGTTIN
 *		for write side :
 *			 0      success, write is allowed
 *			-1      write is denied, SIGTTOU posted to process
 *				group of caller
 *			EIO     write denied, process is in an orphaned
 *				tostop is set, and the process is not ignoring
 *				and not blocking SIGTTOU.
 *			EINTR   write denied, a signal has interrupted
 *				the posting of SIGTTOU
 */
int
osr_bgcheck(osr)
	OSRP	osr;
{
	STHP		sth = osr->osr_sth;

	switch (osr->osr_flags & F_OSR_TTYBITS) {
	case F_OSR_RTTY_CHECK:		/* read-style check */
		return do_read_job_control(sth);

	case F_OSR_ITTY_CHECK:		/* ioctl-style check */
		return do_ioctl_job_control(sth);

	case F_OSR_WTTY_CHECK:		/* write-style check */
		return do_write_job_control(sth);

	default:
		/* exactly one of the *TTY_CHECK flags should be set */
		panic("osr_bgcheck: unexpected osr_flags");
	}
}

int
pse_deny_job_ctl_read(queue_t *q)
{
	/* Find read side of queue pair */
	if (!(q->q_flag & QREADR))
		q = OTHERQ(q);
	/* Follow read side up to the top (stream head) */
	while (q->q_next)
		q = q->q_next;
	return do_read_job_control((STHP)q->q_ptr);
}

int
pse_deny_job_ctl_ioctl(queue_t *q)
{
	/* Find read side of queue pair */
	if (!(q->q_flag & QREADR))
		q = OTHERQ(q);
	/* Follow read side up to the top (stream head) */
	while (q->q_next)
		q = q->q_next;
	return do_ioctl_job_control((STHP)q->q_ptr);
}

int
pse_deny_job_ctl_write(queue_t *q)
{
	/* Find read side of queue pair */
	if (!(q->q_flag & QREADR))
		q = OTHERQ(q);
	/* Follow read side up to the top (stream head) */
	while (q->q_next)
		q = q->q_next;
	return do_write_job_control((STHP)q->q_ptr);
}

/*
 * NAME:	sth_register()
 *
 * PARAMETERS:
 *	sth	- streamhead structure pointer
 *
 * FUNCTION:	Called by the streamhead tty open to register the terminal id.
 *
 * RETURNS:
 *	ENOMEM if not enough place in memory to allocate the shtty_s structure.
 *
 *	0 if success.
 */
int
sth_register(sth)
	STHP	sth;
{
	struct  ttyinfo get_ttyinfo;

#ifdef TTYDBG
        /* ttydbg - register - */
        shtty_conf_err = tty_db_register(&shtty_str_module_conf);
#endif

	NET_MALLOC(sth->shttyp, struct shtty_s *, sizeof(struct  shtty_s),
							M_STRHEAD, M_WAITOK);

	if (!sth->shttyp) 
		return(ENOMEM);
	else {
		bzero(sth->shttyp, sizeof(struct  shtty_s));
		return(0);
	}
}

/*
 * NAME:	sth_unregister()
 *
 * PARAMETERS:
 *	sth	- streamhead structure pointer.
 *
 * FUNCTION:	unregisters this sth from the streamhead tty table.
 *
 * RETURNS:	no object.
 */
void
sth_unregister(sth)
	STHP sth;
{
#ifdef TTYDBG
        /* ttydbg - unregister - */
        shtty_conf_err = tty_db_unregister(&shtty_str_module_conf);
#endif

	NET_FREE(sth->shttyp, M_STRHEAD);
	sth->shttyp = NULL;
}

/*
 * NAME:	sth_query()
 *
 * PARAMETERS:
 *	cmd	no object
 *	id	terminal identificator
 *
 * FUNCTION:	Retrieves if that terminal is trusted or not.
 *
 * RETURNS:	0 if that terminal is not trusted..
 *		1 if that terminal is trusted.
 */
int
sth_query(cmd, id)
	int	cmd, id;
{
	struct shtty_s *shttyp = (struct shtty_s *)id;

	if (shttyp)
		return((shttyp->shtty_flags & F_SHTTY_TRUST) ? 1 : 0);
	else return 0;
}

/*
 * NAME:	osr_tctrust()
 *
 * PARAMETERS:
 *	osr	- osr structure pointer.
 *
 * FUNCTION:	To set or unset the trust flag in the stream head for that tty.
 *
 * RETURNS:	EINVAL if no pointer sended or if the flag is no TCTRUSTED or
 *		TCUNTRUSTED. Answered the TCTRUST ioctl.
 *
 *		0 if succedeed.
 */
OSR_STATUS
osr_tctrust(osr)
	OSRP	osr;
{
        int	trust_flag = osr->osr_ioctl_arg1;
        STHP	sth = osr->osr_sth;
	int     error = 0;

	if (sth == NULL)
		return(EINVAL);
	if (!sth->shttyp)
		return(ENOTTY);

	if (error = privcheck(TPATH_CONFIG))
		return error;

	if (trust_flag == TCTRUSTED)
		sth->shttyp->shtty_flags |= F_SHTTY_TRUST;
	else if (trust_flag == TCUNTRUSTED)
		sth->shttyp->shtty_flags &= ~F_SHTTY_TRUST;
	else
		return(EINVAL);

	return(0);
}

/*
 * NAME:	osr_tcqtrust()
 *
 * PARAMETERS:
 *	osr	- osr structure pointer.
 *
 * FUNCTION:	get the state of that terminal, trusted or not. Answered the
 *		TCQTRUST ioctl.
 *
 * RETURNS:	always 0.
 */
OSR_STATUS
osr_tcqtrust(osr)
	OSRP	osr;
{
	STHP		sth = osr->osr_sth;
	int     	error = 0;

	if (error = privcheck(TPATH_CONFIG))
		return error;

        osr->osr_ioctl_arg0_len = sizeof (int);
	*(int *)osr->osr_ioctl_arg0p = 
			((sth->shttyp->shtty_flags & F_SHTTY_TRUST) ? 1 : 0);
	return(0);
}

/*
 * NAME:	osr_tcsak()
 *
 * PARAMETERS:
 *	osr	- osr structure pointer.
 *
 * FUNCTION:	set or unset the state for security attention key in the
 *		streamhead before following the message downstream.
 *		Answered to the TCSAK ioctl.
 *
 * RETURNS:	EINVAL if the flag sent by the user is not a known value.
 *		0 if succeeded.
 */
OSR_STATUS
osr_tcsak(osr)
	OSRP	osr;
{
	STHP	sth = osr->osr_sth;
	int     error = 0;

	if (error = privcheck(TPATH_CONFIG))
		return error;

	switch(osr->osr_ioctl_arg1) {
	case TCSAKON:
		sth->shttyp->shtty_tsm = getpid();
		sth->shttyp->shtty_flags |= F_SHTTY_SAK;
		break;
	case TCSAKOFF:
		sth->shttyp->shtty_tsm = 0;
		sth->shttyp->shtty_flags &= ~F_SHTTY_SAK;
		break;
	default:
		return(EINVAL);
	}
	return(0);
}
	
/*
 * NAME:	osr_tcqsak()
 *
 * PARAMETERS:
 *	osr	- osr structure pointer.
 *
 * FUNCTION:	returns the value setted in shtty_tsm if the sak sequence has
 *		been enabled or 0 if not, to the user.
 *
 * RETURNS:	always 0.
 */
OSR_STATUS
osr_tcqsak(osr)
	OSRP	osr;
{
	STHP	sth = osr->osr_sth;
	int     error = 0;

	if (error = privcheck(TPATH_CONFIG))
		return error;

        osr->osr_ioctl_arg0_len = sizeof (int);
        *(int *)osr->osr_ioctl_arg0p = 
		((sth->shttyp->shtty_flags & F_SHTTY_SAK) ? 
						sth->shttyp->shtty_tsm : 0);
	return(0);
}

/*
 * NAME:	osr_tioccons()
 *
 * PARAMETERS:
 *	osr	- osr structure pointer.
 *
 * FUNCTION:	Set the console redirection active or not, answered the
 *		TIOCCONS ioctl.
 *
 * RETURNS:	EACCESS if not configuration possible.
 *	
 *		all return codes sent by conconfig() call.
 *
 *		0 if succedeed.
 */
OSR_STATUS
osr_tioccons(osr)
        OSRP    osr;
{
        STHP            sth = osr->osr_sth;

	struct	tcons_info	tcsav;	/* console tee info save area */
	struct  uio		uio_tmp;/* copy of uio structure          */
        struct  iovec           iov_tmp;/* iovec structure for single iovec */
	int	err = 0;

	tcsav.tcons_devno = sth->sth_dev;
	tcsav.tcons_chan = 0;
        uio_tmp.uio_iov = &iov_tmp;
        uio_tmp.uio_xmem = (struct xmem *) NULL;
        uio_tmp.uio_iovcnt = 1;
        uio_tmp.uio_segflg = UIO_SYSSPACE;
        uio_tmp.uio_resid = sizeof (struct tcons_info);
        iov_tmp.iov_base = (char *)&tcsav;
        iov_tmp.iov_len = sizeof (struct tcons_info);

	if (osr->osr_ioctl_arg1) {		/* if arg data	*/
                if (privcheck(SYS_CONFIG))
                        err = EACCES;
                else
                        if((err=conconfig(sth->sth_dev,CHG_CONS_TEE, &uio_tmp))
									==0)
                                sth->shttyp->shtty_flags |= F_SHTTY_CONS; 
			/* set console redirection active */
                return(err);
        }
        else {
		/* indicate console tee inactive */
                sth->shttyp->shtty_flags &= ~F_SHTTY_CONS;
                err = conconfig((dev_t)0,CHG_CONS_TEE, &uio_tmp);
		return(err);
        }
}

/*
 * NAME:        osr_tcskep()
 *
 * PARAMETERS:
 *      osr     - osr structure pointer.
 *
 * FUNCTION:    set or unset the state for Keyboard emulator program
 *              hook in the streamhead.
 *
 * RETURNS:     ENOTTY if it is not a tty.
 *              0 if succeeded.
 */
OSR_STATUS
osr_tcskep(osr)
	OSRP	osr;
{
	STHP	sth = osr->osr_sth;

	if (!sth->shttyp) return(ENOTTY);
	if (osr->osr_ioctl_arg1)
		sth->shttyp->shtty_flags |= F_SHTTY_KEP;
	else
		sth->shttyp->shtty_flags &= ~F_SHTTY_KEP;
	return(0);
}

/*
 * NAME:        osr_kepcheck()
 *
 * PARAMETERS:
 *      osr     - osr structure pointer.
 *
 * FUNCTION:    Keyboard Emulation Program flag is set.  Send a ACK out to
 *              tell it that we're about to go to sleep, so we must be all
 *              done.
 *
 * RETURNS:     all error return codes sent by osr_bufcall().
 *              0 if succeeded.
 */
OSR_STATUS
osr_kepcheck(osr)
	OSRP	osr;
{
	STHP	sth = osr->osr_sth;
	MBLKP	mp;
	int	error = 0;

	while ( !(mp = allocb(sizeof(char), BPRI_HI)) ) {
		if (error = osr_bufcall(osr, TRUE, 0, sizeof(char), BPRI_HI))
			return error;
	}
	*mp->b_rptr = '\006';
	mp->b_wptr += sizeof (char);
	mp->b_datap->db_type = M_DATA;
	putnext(sth->sth_wq, mp);
	return error;
}

/*
 * NAME:        osr_tcxonc()
 *
 * PARAMETERS:
 *      osr     - osr structure pointer.
 *
 * FUNCTION:    Send according with the parameter a priority message
 *              downstream.
 *
 * RETURNS:     return EINVAL for an invalid parameter.
 *              0 if succeeded.
 */
OSR_STATUS
osr_tcxonc(osr)
	OSRP	osr;
{
	STHP    sth = osr->osr_sth;

	switch((int)osr->osr_ioctl_arg1) {
	case TCOOFF:
		putctl(sth->sth_wq->q_next, M_STOP);
		break;
	case TCOON:
		putctl(sth->sth_wq->q_next, M_START);
		break;
	case TCIOFF:
		putctl(sth->sth_wq->q_next, M_STOPI);
		break;
	case TCION:
		putctl(sth->sth_wq->q_next, M_STARTI);
		break;
	default:
		return EINVAL;
	}
	return 0;
}

/*
 * NAME:        osr_flushdata()
 *
 * PARAMETERS:
 *      osr     - osr structure pointer.
 *
 * FUNCTION:    Flushdata at the streamhead according with the setting of the
 *              parameter.
 *              Send according with the parameter a priority message
 *              downstream.
 *
 * RETURNS:     return EINVAL for an invalid parameter.
 *              return ENOSR if the priority can't be sent.
 *              0 if succeeded.
 */
OSR_STATUS
osr_flushdata(osr)
	OSRP	osr;
{
	STHP	sth = osr->osr_sth;
	int	flags = osr->osr_ioctl_arg1;

	switch (flags) {
	case FLUSHW:
		flushq(sth->sth_wq, FLUSHDATA);
		break;
	case FLUSHR:
		flushq(sth->sth_rq, FLUSHDATA);
		break;
	case FLUSHRW:
		flushq(sth->sth_rq, FLUSHDATA);
		flushq(sth->sth_wq, FLUSHDATA);
		break;
	default:
		LEAVE_FUNC(osr_flush, EINVAL);
		return EINVAL;
	}
	if (putctl1(sth->sth_wq->q_next, M_FLUSH, flags)) {
		LEAVE_FUNC(osr_flush, 0);
		return 0;
	}
	return ENOSR;
}

/*
 * NAME: sth_tiocspgid()
 *
 * PARAMETERS:
 *      sth     - streams head pointer
 * 	pgid 	- new process group id
 * 	cmd	- which ioctl command
 * 			TIOCSPGID
 *			TXSPGRP
 *
 * FUNCTION:
 * Called by pse_ioctl() directly.  Given the stream head pointer, a new 
 * process group id, and a command (TIOCSPGID or TXSPGRP), set the
 * process group for this tty.
 * 
 * RETURNS:
 *		0 	If all goes well.
 * 		ENOTTY	Posix.
 *      	EPERM   No process whose identificator is a pgrp or the pgrp is
 *	                not in the same session.
 *		EINVAL	If pgid is bogus.
 *		errors from do_ioctl_job_control().
 *		errors marked in the stream head.
 *
 */
int
sth_tiocspgrp(sth, pgid, cmd)
	STH	*sth;
	pid_t	pgid;
	int	cmd;

{
	pid_t	sid;
	pid_t   cursid = kgetsid(0);
	struct  ttyinfo get_ttyinfo;
	int error=0;

	/*
	 * Check for an existing error on the stream head.  If one
	 * exists, AND it's not a write EIO, then return the 
	 * appropriate error.  This logic stolen from osr_run().  I
	 * don't know why we should ignore EIO, but that's the way
	 * osr_run() does it.
	 */
	if ((sth->sth_flags & RWHL_ERROR_FLAGS) && 
		(sth->sth_write_error != EIO)) {

		if (sth->sth_write_error)
			error = sth->sth_write_error;
		else if (sth->sth_read_error)
			error = sth->sth_read_error;
		else
			error = ENXIO;
		return(error);
	} 
	if (sth->sth_flags & F_STH_CTTY) {
		error = do_ioctl_job_control(sth);
		if (error)
			return(error);
	}
	if (pgid < 0)
		return(EINVAL);
	sid = kgetsid(pgid);
	if (!is_pgrp(pgid) || sid != cursid)
		return(EPERM);
	getctty(&get_ttyinfo);
	if (cmd == TXSPGRP && ((!get_ttyinfo.ti_ttyp) || 
		(cursid != sth->shttyp->shtty_sid)))
		return(ENOTTY);
	sth->shttyp->shtty_pgrp = pgid;
	return(0);
}
