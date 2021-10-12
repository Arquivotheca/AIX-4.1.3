static char sccsid[] = "@(#)19        1.11  src/bos/kernext/pse/str_gpmsg.c, sysxpse, bos41J, 9515A_all 4/7/95 01:55:00";
/*
 * COMPONENT_NAME: SYSXPSE - STREAMS framework
 * 
 * FUNCTIONS:      getmsg
 *                 getpmsg
 *                 putmsg
 *                 putpmsg
 * 
 * ORIGINS: 27, 63, 83
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
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.1
 */

#include <sys/errno.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/user.h>
#include <sys/poll.h>
#include <pse/str_stream.h>
#include <pse/str_proto.h>
#include <pse/str_ioctl.h>
#include <sys/sleep.h>
#include <sys/stropts.h>
#include <sys/trchkid.h>
#include <pse/str_debug.h>

#define _Geterrno	getuerror
#define _Seterrno	setuerror

extern const char ttybg[];

static int
get_sth_from_fd(int fd, STHPP sthp, int *flag)
{
	struct file     *fp;

	ufdhold(fd);
	if (ufdgetf(fd, &fp)) {
		ufdrele(fd);
		return ENOSTR;
	}
	ufdrele(fd);
	if (!(*sthp = (STHP)fdtosth(fd))) {
		dev_t   dev;

		if (fp_getdevno(fp, &dev, (chan_t *)NULL))
			return ENOSTR;
		/*
		 * /dev/console = MAJOR 4
		 * /dev/tty     = MAJOR 1
		 */
		if (major(dev) == 4 || major(dev) == 1) {
			struct  ttyinfo get_ttyinfo;

			getctty(&get_ttyinfo);
			dev = get_ttyinfo.ti_ttyd;
		}
		if (!(*sthp = (STHP)devtosth(dev)))
			return ENOSTR;
	}
	if (fp->f_flag & FNDELAY)
		*flag |= F_OSR_NDELAY;
	if (fp->f_flag & FNONBLOCK)
		*flag |= F_OSR_NONBLOCK;
	return 0;
}

int
getmsg (fd, ctlptr, dataptr, flags)
        int     fd;
        struct strbuf   * ctlptr;
        struct strbuf   * dataptr;
        int     * flags;
{
	STHP		sth;
	OSRP		osr;
	int 		error = 0;
	int		retval;
	int		len = 0;
	int		flag = 0;
	struct strpeek	speek;
	struct strpeek  *strp;
	

	TRCHKL4T(HKWD_PSE | hkwd_pse_getmsg_in, fd, ctlptr, dataptr, flags);

	ENTER_FUNC(getmsg, fd, ctlptr, dataptr, flags, 0, 0);

	if (!flags) {
		error = EINVAL;
		goto out;
	}
	if (error = get_sth_from_fd(fd, &sth, &flag)) {
		goto out;
	}
	
	osr = osr_alloc(sth, 0, BPRI_WAITOK);
	osr->osr_creds = crref();
	osr->osr_ioctl_arg0p = (char *)(strp = &speek);
	osr->osr_handler     = osr_getmsg;
	osr->osr_osrq        = &sth->sth_read_osrq;
	osr->osr_flags      |= flag;
	osr->osr_flags      |= F_OSR_AUDIT_READ|F_OSR_RTTY_CHECK;
	osr->osr_closeout    = RL_ERROR_FLAGS;
	len                  = sizeof(struct strpeek);

        if (ctlptr) {
		if (error = copyin((caddr_t)ctlptr, (caddr_t)&strp->ctlbuf,
							sizeof *ctlptr))
			goto done;
        } else {
		strp->ctlbuf.buf = nilp(char);
                strp->ctlbuf.maxlen = -1;
                strp->ctlbuf.len = -1;
	}
        if (dataptr) {
		if (error = copyin((caddr_t)dataptr, (caddr_t)&strp->databuf,
							sizeof *dataptr))
			goto done;
        } else {
		strp->databuf.buf = nilp(char);
                strp->databuf.maxlen = -1;
                strp->databuf.len = -1;
	}
        if (error = copyin((caddr_t)flags, (caddr_t)&strp->flags,
							sizeof(*flags)))
		goto done;

	if (error = osr_run(osr)) goto done;

	retval = osr->osr_ret_val;

	if (ctlptr && (error = copyout((caddr_t)&strp->ctlbuf, (caddr_t)ctlptr,
							sizeof *ctlptr)))
		goto done;
	if (dataptr && (error = copyout((caddr_t)&strp->databuf,
					(caddr_t)dataptr, sizeof *dataptr)))
		goto done;
	error = copyout((caddr_t)&strp->flags, (caddr_t)flags, sizeof(*flags));

done:
	DB_check_streams("GETMSG");
	
	osr_free(osr);

out:
	TRCHKL1T(HKWD_PSE | hkwd_pse_getmsg_out, error);

	LEAVE_FUNC(getmsg, error);
	_Seterrno(error);
        return (error ? -1 : (retval ? retval : 0));
}

int
getpmsg (fd, ctlptr, dataptr, bandp, flags)
        int     fd;
        struct strbuf   * ctlptr;
        struct strbuf   * dataptr;
        int     * bandp;
        int     * flags;
{
	STHP		sth;
	OSRP		osr;
	int 		error = 0;
	int 		retval;
	int		len = 0;
	int		flag = 0;
	struct strpmsg	strpmsg;
        struct strpmsg  *strp;

	TRCHKL5T(HKWD_PSE | hkwd_pse_getpmsg_in,fd,ctlptr,dataptr,bandp,flags);

	ENTER_FUNC(getpmsg, fd, ctlptr, dataptr, bandp, flags, 0);

        if (flags == 0 || bandp == 0) {
                error = EINVAL;
                goto out;
        }
	if (error = get_sth_from_fd(fd, &sth, &flag)) {
		goto out;
	}
	
	osr = osr_alloc(sth, 0, BPRI_WAITOK);
	osr->osr_creds = crref();
	osr->osr_ioctl_arg0p = (char *)(strp = &strpmsg);
	osr->osr_handler     = osr_getpmsg;
	osr->osr_osrq        = &sth->sth_read_osrq;
	osr->osr_flags      |= flag;
	osr->osr_flags      |= F_OSR_AUDIT_READ|F_OSR_RTTY_CHECK;
	osr->osr_closeout    = RL_ERROR_FLAGS;
	len                  = sizeof(struct strpmsg);

        if (ctlptr) {
		if (error = copyin((caddr_t)ctlptr, (caddr_t)&strp->ctlbuf,
							sizeof *ctlptr))
			goto done;
        } else {
		strp->ctlbuf.buf = nilp(char);
                strp->ctlbuf.maxlen = -1;
                strp->ctlbuf.len = -1;
	}
        if (dataptr) {
		if (error = copyin((caddr_t)dataptr, (caddr_t)&strp->databuf,
							sizeof *dataptr))
			goto done;
        } else {
		strp->databuf.buf = nilp(char);
                strp->databuf.maxlen = -1;
                strp->databuf.len = -1;
	}
        if (error = copyin((caddr_t)flags, (caddr_t)&strp->flags,
							sizeof(*flags)))
		goto done;
        if (error = copyin((caddr_t)bandp, (caddr_t)&strp->band,
							sizeof(*bandp)))
		goto done;

	if (error = osr_run(osr)) goto done;

	retval = osr->osr_ret_val;

	if (ctlptr && (error = copyout((caddr_t)&strp->ctlbuf, (caddr_t)ctlptr,
							sizeof *ctlptr)))
		goto done;
	if (dataptr && (error = copyout((caddr_t)&strp->databuf,
					(caddr_t)dataptr, sizeof *dataptr)))
		goto done;

	if (error = copyout((caddr_t)&strp->flags, (caddr_t)flags,
					sizeof(*flags)))
			goto done;
	error = copyout((caddr_t)&strp->band, (caddr_t)bandp, sizeof(*bandp));

done:
	DB_check_streams("GETPMSG");
	
	osr_free(osr);

out:
	TRCHKL1T(HKWD_PSE | hkwd_pse_getpmsg_out, error);

	LEAVE_FUNC (getpmsg, error);
	_Seterrno(error);
        return (error ? -1 : (retval ? retval : 0));
}

int
putmsg (fd, ctlptr, dataptr, flags)
        int     fd;
        struct strbuf   * ctlptr;
        struct strbuf   * dataptr;
        int     flags;
{
	STHP		sth;
	OSRP		osr;
	int 		error = 0;
	int		len = 0;
	int		flag = 0;
	struct strpeek	speek;
        struct strpeek  *strp;

	TRCHKL4T(HKWD_PSE | hkwd_pse_putmsg_in, fd, ctlptr, dataptr, flags);

	ENTER_FUNC(putmsg, fd, ctlptr, dataptr, flags, 0, 0);

	if (error = get_sth_from_fd(fd, &sth, &flag)) {
		goto out;
	}
	
	osr = osr_alloc(sth, 0, BPRI_WAITOK);
	osr->osr_creds = crref();
	osr->osr_ioctl_arg0p = (char *)(strp = &speek);
	osr->osr_handler     = osr_putmsg;
	osr->osr_osrq        = &sth->sth_write_osrq;
	osr->osr_flags      |= flag;
	osr->osr_flags      |= F_OSR_AUDIT_WRITE|F_OSR_WTTY_CHECK;
	osr->osr_closeout    = WHL_ERROR_FLAGS;

        if (ctlptr) {
		if (error = copyin((caddr_t)ctlptr, (caddr_t)&strp->ctlbuf,
							sizeof *ctlptr))
			goto done;
        } else {
		strp->ctlbuf.buf = nilp(char);
                strp->ctlbuf.maxlen = -1;
                strp->ctlbuf.len = -1;
	}
        if (dataptr) {
		if (error = copyin((caddr_t)dataptr, (caddr_t)&strp->databuf,
							sizeof *dataptr))
			goto done;
        } else {
		strp->databuf.buf = nilp(char);
                strp->databuf.maxlen = -1;
                strp->databuf.len = -1;
	}
	strp->flags = flags;

	error = osr_run(osr);

done:
	DB_check_streams("PUTMSG");
	
	osr_free(osr);

out:
	TRCHKL1T(HKWD_PSE | hkwd_pse_putmsg_out, error);
	LEAVE_FUNC (putmsg, error);
	_Seterrno(error);
        return (error ? -1 : 0);
}

int
putpmsg (fd, ctlptr, dataptr, band, flags)
        int     fd;
        struct strbuf   * ctlptr;
        struct strbuf   * dataptr;
        int     band;
        int     flags;
{
	STHP		sth;
	OSRP		osr;
	int 		error = 0;
	int		len = 0;
	int		flag = 0;
	struct strpmsg	strpmsg;
        struct strpmsg  *strp;

	TRCHKL5T(HKWD_PSE | hkwd_pse_putpmsg_in,fd,ctlptr,dataptr,band,flags);

	ENTER_FUNC(putpmsg, fd, ctlptr, dataptr, band, flags, 0);

	if (error = get_sth_from_fd(fd, &sth, &flag)) {
		goto out;
	}
	
	osr = osr_alloc(sth, 0, BPRI_WAITOK);
	osr->osr_creds = crref();
	osr->osr_ioctl_arg0p = (char *)(strp = &strpmsg);
	osr->osr_handler     = osr_putpmsg;
	osr->osr_osrq        = &sth->sth_write_osrq;
	osr->osr_flags      |= flag;
	osr->osr_flags      |= F_OSR_AUDIT_WRITE|F_OSR_WTTY_CHECK;
	osr->osr_closeout    = WHL_ERROR_FLAGS;

        if (ctlptr) {
		if (error = copyin((caddr_t)ctlptr, (caddr_t)&strp->ctlbuf,
							sizeof *ctlptr))
			goto done;
        } else {
		strp->ctlbuf.buf = nilp(char);
                strp->ctlbuf.maxlen = -1;
                strp->ctlbuf.len = -1;
	}
        if (dataptr) {
		if (error = copyin((caddr_t)dataptr, (caddr_t)&strp->databuf,
							sizeof *dataptr))
			goto done;
        } else {
		strp->databuf.buf = nilp(char);
                strp->databuf.maxlen = -1;
                strp->databuf.len = -1;
	}
	strp->flags = flags;
	strp->band = band;

	error = osr_run(osr);

done:
	DB_check_streams("PUTPMSG");
	
	osr_free(osr);

out:
	TRCHKL1T(HKWD_PSE | hkwd_pse_putpmsg_out, error);

	LEAVE_FUNC (putpmsg, error);
	_Seterrno(error);
        return (error ? -1 : 0);
}
