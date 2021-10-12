static char sccsid[] = "@(#)07  1.5  src/bos/usr/ccs/lib/libtli/tfixup.c, libtli, bos411, 9428A410j 3/8/94 19:12:22";
/*
 *   COMPONENT_NAME: LIBTLI
 *
 *   FUNCTIONS: t_unix_to_tli_error, t_chk_ack, t_get_primitive
 *
 *   ORIGINS: 18 27 63
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */

/** Copyright (c) 1989  Mentat Inc.
 ** tfixup.c 1.3, last change 1/29/90
 **/

#include "common.h"
#include <sys/stropts.h>
#include <sys/poll.h>
#include <sys/tihdr.h>

void
t_unix_to_tli_error () {
	switch (errno) {
	case EBADF:
	case ENOSTR:
		t_errno = TBADF;
		break;
	case EACCES:
		t_errno = TACCES;
		break;
	case EAGAIN:
#if	ENODATA != EAGAIN
	case ENODATA:
#endif
		t_errno = TNODATA;
		break;
#ifdef XTI
	case EPROTO:
		t_errno = TPROTO;
		break;
#endif
	default:
		t_errno = TSYSERR;
		break;
	}
}

int
t_chk_ack (fd, tli, prim_type)
	int		fd;
	struct	tli_st	* tli;
	int		prim_type;
{
	struct strbuf		ctlbuf;
	int			iflags;
	int			ret;
	struct T_error_ack	* tea;
	struct T_ok_ack		* toa;
	struct pollfd		pfd;
	
	/*
	 * NB: acks must _always_ be synchronous
	 *     so wait indefinitely (even if O_NDELAY!) in poll().
	 *     when poll returns, either a system error has occured
	 *     or the message we want has arrived.
	 */
	pfd.fd = fd;
	pfd.events = POLLIN | POLLPRI;
	if (poll(&pfd, 1, -1) < 0) {
		t_errno = TSYSERR;
		return -1;
	}

	/* Check for accept acknowledgment */
	ctlbuf.buf = (char *)tli->tlis_proto_buf;
	ctlbuf.maxlen = _DEFAULT_STRCTLSZ;
	ctlbuf.len = 0;
	iflags = RS_HIPRI;
	
	/*
	 * Since we're reading a maximum size control buffer and no data,
	 * this getmsg should return 0.  MORECTL would mean that the
	 * transport provider created a M_PROTO block larger than the
	 * maximum allowed for writing.  MOREDATA can't happen.
	 * getmsg may return -1 if the stream is in non-blocking mode
	 * and there is no message available.
	 */
	toa = (struct T_ok_ack *)tli->tlis_proto_buf;
	if (( ret = getmsg(fd, &ctlbuf, nilp(struct strbuf), &iflags)) != 0) {
		t_unix_to_tli_error();
		return -1;
	}
	if ( ctlbuf.len < (int)sizeof(toa->PRIM_type) ) {
		/* Transport provider error -- illegal TPI message */
		t_errno = TSYSERR;
		errno = EPROTO;
		return -1;
	}
	toa = (struct T_ok_ack *)tli->tlis_proto_buf;
	if ( toa->PRIM_type == T_ERROR_ACK ) {
		struct T_error_ack * tea = (struct T_error_ack *)tli->tlis_proto_buf;
		if ( ctlbuf.len < (int)sizeof(struct T_error_ack)
		||  tea->ERROR_prim != prim_type ) {
			/* NOTE: how can this happen?? */
			tli->tlis_flags |= TLIS_SAVED_PROTO;
			t_errno = TLOOK;
			return -1;
		}
		t_errno = tea->TLI_error;
		errno = tea->UNIX_error;
		return -1;
	}
	if ( ctlbuf.len < sizeof(struct T_ok_ack)
	||  toa->PRIM_type != T_OK_ACK
	||  toa->CORRECT_prim != prim_type ) {
		tli->tlis_flags |= TLIS_SAVED_PROTO;
		t_errno = TLOOK;
		return -1;
	}
	return 0;
}

/*
 * t_get_primitive()
 *              return -1:
 *                      fatal error
 *              return TBUFOVFLW:
 *                      caller should return -1 and t_errno = TBUFOVFLW
 *                      this will allow state to transition to next state
 *              return 0:
 *                      no error
 */
int
t_get_primitive (fd, tli, expected_primitive, expected_len, netbufp)
	int                     fd;
	struct tli_st *         tli;
	long                    expected_primitive;
	int                     expected_len;
	struct netbuf *         netbufp;
{
	char                    buf1[64];
	char                    buf2[64];
	struct strbuf           ctlbuf;
	struct strbuf           databuf;
	struct strpeek          strp;
	int                     iflags, len;
	int                     ret = 0;
	union T_primitives *    prim;


	prim = (union T_primitives *)tli->tlis_proto_buf;
	if (!(tli->tlis_flags & TLIS_SAVED_PROTO)) {

		/*
		 * If there is no primitive already waiting in the library,
		 * then call getmsg to retrieve just the M_PROTO part of the
		 * next message.
		 */
		ctlbuf.buf = tli->tlis_proto_buf;
		ctlbuf.len = 0;
		ctlbuf.maxlen = _DEFAULT_STRCTLSZ;

		iflags = 0;
		ret = getmsg(fd, &ctlbuf, nilp(struct strbuf), &iflags);
		switch (ret) {
		case MORECTL:
		case MORECTL|MOREDATA:
			/* This should *never* happen...
			 * MORECTL means that the transport provider
			 * created a M_PROTO block larger than the
			 * maximum allowed for writing.
			 */
			t_errno = TSYSERR;
			errno = EPROTO;
			return -1;
		case MOREDATA:
		case 0:
			if (ctlbuf.len != -1 && ctlbuf.len < expected_len) {
				if (prim->type != expected_primitive)
					goto tlook_error;
				t_errno = TSYSERR;
				errno = EPROTO;
				return -1;
			}
			break;
		case -1:
		default:
			t_unix_to_tli_error();
			return -1;
		}
	}

	if (prim->type != expected_primitive)
		goto tlook_error;

	/* Check for data associated with this primitive. */
	strp.ctlbuf.buf = buf1;
	strp.ctlbuf.maxlen = sizeof(buf1);
	strp.databuf.buf = buf2;
	strp.databuf.maxlen = sizeof(buf2);
	strp.flags = 0;
	switch (ioctl(fd, I_PEEK, (char *)&strp)) {
	case -1:
		t_unix_to_tli_error();
		return -1;
	case 1:
		if ( strp.ctlbuf.len <= 0 && strp.databuf.len > 0 ) 
			break;
		fallthru;
	case 0:
		/* No data, we're all done. */
		if (ret == MOREDATA)
			break;
		if (netbufp)
			netbufp->len = 0;
		tli->tlis_flags &= ~TLIS_SAVED_PROTO;
		return 0;
	}
	/* There's data, so do a second getmsg to retrieve all data. */

	tli->tlis_flags &= ~TLIS_SAVED_PROTO;

	databuf.buf = (char *)malloc(len = _DEFAULT_STRMSGSZ);
	databuf.maxlen = len;
	iflags = 0;
	ret = getmsg(fd, nilp(struct strbuf), &databuf, &iflags);
	if (netbufp  &&  netbufp->maxlen > 0) {
		memmove(netbufp->buf, databuf.buf,
		(size_t)(netbufp->len = databuf.len > 0 ? databuf.len : 0));
		if (databuf.len > netbufp->maxlen) {
			t_errno = TBUFOVFLW;
			return TBUFOVFLW;
		}
	}
	/* XXX Not sure about these ! 
	else {
		t_errno = TBUFOVFLW;
		return TBUFOVFLW;
	}
	*/

	free(databuf.buf);	/* memory leak in OSF */
	tli->tlis_flags &= ~TLIS_SAVED_PROTO;
	return 0;

tlook_error:
	tli->tlis_flags |= TLIS_SAVED_PROTO;
	t_errno = TLOOK;
	return -1;
}

