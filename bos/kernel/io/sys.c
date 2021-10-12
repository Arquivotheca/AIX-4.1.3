#ifndef lint
static char sccsid[] = "@(#)00	1.14  src/bos/kernel/io/sys.c, sysio, bos411, 9428A410j 2/16/94 10:38:47";
#endif
/*
 * COMPONENT_NAME: (SYSIO) Machine Device Driver
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

/*
 *	indirect driver for controlling tty.
 */
#include <sys/param.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/dir.h>
#include <sys/signal.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/device.h>
#include <sys/proc.h>
#include <sys/ioctl.h>

#define syscheck(cmd) \
	(((U.U_ttyp == NULL) || (U.U_procp->p_sid != *U.U_ttysid)) ? ENXIO : (cmd))

int syopen(dev_t dev, int rwflag, chan_t chan, int ext)
{
    return syscheck(0);
}

int syread(dev_t dev, struct uio *uio, chan_t chan, int ext)
{
    int rc;

    rc = syscheck(0);
    if (!rc) {
	DD_ENT(rc =, (*devsw[major(U.U_ttyd)].d_read)
	    (U.U_ttyd, uio, U.U_ttympx, ext), IPRI_BASE, major(U.U_ttyd));
    }
    return rc;
}

int sywrite(dev_t dev, struct uio *uio, chan_t chan, int ext)
{
    int rc;

    rc = syscheck(0);
    if (!rc) {
	DD_ENT(rc =, (*devsw[major(U.U_ttyd)].d_write)
	    (U.U_ttyd, uio, U.U_ttympx, ext), IPRI_BASE, major(U.U_ttyd));
    }
    return rc;
}

int syioctl(dev_t dev, int cmd, caddr_t arg, int mode, chan_t chan, int ext,
	long *ioctlrv)
{
    int rc;

    if (cmd == TIOCNOTTY) {
	struct proc *p = U.U_procp;

	/*
	 * Only certain processes will succeed with this BSD ioctl:
	 *     a process that is neither a session leader nor pgrp leader or
	 *     a session leader with no other process in the session or
	 *     a process group leader with no other process in the group.
	 *
	 * Do these actions in the spirit of the BSD ioctl:
	 *         1. release terminal
	 *	   2. be able to acquire a controlling terminal
	 *	   3. get out of process group (by setting new process group)
	 */

	if (!p->p_ganchor ||		/* regular process */
	    (p->p_pid == p->p_sid && p->p_ttyl == p->p_pgrpl) ||
	    (p->p_pid != p->p_sid  && (p == p->p_ganchor && !p->p_pgrpl))) {
	    if (p->p_pid == p->p_sid) {
		if (U.U_ttyp) {
		    *U.U_ttyp = 0;
		    U.U_ttyp = 0;
		}
		if (U.U_ttysid) {
		    *U.U_ttysid = 0;
		    U.U_ttysid = 0;
		}
		U.U_ttyid = 0;
		U.U_ttyf = 0;
	    } else
		setsid2();
	    return(0);
	}
	return (EPERM);
    }

    rc = syscheck(0);
    if (!rc) {
	DD_ENT(rc =, (*devsw[major(U.U_ttyd)].d_ioctl)
	    (U.U_ttyd, cmd, arg, mode, U.U_ttympx, ext, ioctlrv),
	    IPRI_BASE, major(U.U_ttyd));
    }
    return rc;
}


int syselect(dev_t dev, short events, short *reventp, chan_t chan)
{
    return syscheck(devselect(U.U_ttyd,U.U_ttympx,events,reventp, NULL));
}
