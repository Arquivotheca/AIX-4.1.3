static char sccsid[] = "@(#)13  1.2.1.3  src/bos/usr/ccs/lib/libtli/tlook.c, libtli, bos411, 9428A410j 3/8/94 19:12:48";
/*
 *   COMPONENT_NAME: LIBTLI
 *
 *   FUNCTIONS: t_ilook, t_look, __t_ilook, __t_look
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
 ** tlook.c 1.2, last change 1/29/90
 **/

#include "common.h"
#include <sys/stropts.h>
#include <sys/tihdr.h>
#include "tlistate.h"
#ifdef XTI
#include <sys/poll.h> 
#endif

int
t_ilook (fd)
	int	fd;
{
	long		type, data;
	struct strpeek	strp;
        int		code;
	struct tli_st	* tli;

        code = -1;
	if (!(tli = iostate_lookup(fd, IOSTATE_VERIFY)))
		return code;

	if (!(tli->tlis_flags & TLIS_SAVED_PROTO)) {
		strp.ctlbuf.buf = (char *)&type;
		strp.ctlbuf.maxlen = sizeof(type);
		strp.databuf.buf = (char *)&data;
		strp.databuf.maxlen = sizeof(data);
		strp.flags = 0;
		switch (ioctl(fd, I_PEEK, &strp)) {
		case -1:
			t_unix_to_tli_error();
			goto rtn;
		case 0:
			code = 0;
			goto rtn;
		case 1:
			break;
		}
		if (strp.ctlbuf.len <= 0 && strp.databuf.len > 0) {
			if (tli->tlis_flags & TLIS_SAVED_EXDATA)
				return T_EXDATA;
			else
				return T_DATA;
		}
		if (strp.ctlbuf.len != sizeof(type)) {
			code = 0;
			goto rtn;
       		}
	} else
		type = ((union T_primitives *)tli->tlis_proto_buf)->type;

	switch (type) {
	case T_CONN_CON:
		code = T_CONNECT;
		break;
	case T_CONN_IND:
		code = T_LISTEN;
		break;
	case T_DATA_IND:
		code = T_DATA;
		break;
	case T_DISCON_IND:
		code = T_DISCONNECT;
		break;
	case T_EXDATA_IND:
		code = T_EXDATA;
		break;
	case T_ORDREL_IND:
		code = T_ORDREL;
		break;
	case T_UNITDATA_IND:
		code = T_DATA;
		break;
	case T_UDERROR_IND:
		code = T_UDERR;
		break;
	default:
		errno = EPROTO;
		t_errno = TSYSERR;
		break;
	}
rtn:
	TLI_UNLOCK(tli);

#ifndef XTI
#ifdef XTIDBG
	tr_look (fd, code);
#endif /* XTIDBG */
#endif /* XTI */
	return code;
}

#ifdef	XTI
int
t_look (fd)
	int	fd;
{
	int		i1;
	struct tli_st	* tli;
	struct pollfd	fds[1];

	i1 = t_ilook(fd);
	if ( i1 != 0 )
		return i1;
	if (tli = iostate_lookup(fd, IOSTATE_VERIFY)) {
		if (tli->tlis_flags & (TLIS_DATA_STOPPED|TLIS_EXDATA_STOPPED)) {
			fds[0].fd = fd;
			fds[0].events = POLLOUT;
			if (poll(fds, 1L, 0L) == 1
			&&  fds[0].revents == POLLOUT) {
				if (tli->tlis_flags & TLIS_EXDATA_STOPPED) {
					tli->tlis_flags &= ~TLIS_EXDATA_STOPPED;
					i1 = T_GOEXDATA;
				}
				else {
					tli->tlis_flags &= ~TLIS_DATA_STOPPED;
					i1 = T_GODATA;
				}
			}
		}
		TLI_UNLOCK(tli);
	}
#ifdef XTIDBG
	tr_look (fd, i1);
#endif /* XTIDBG */
	return i1;
}
#endif

#if defined(_THREAD_SAFE) || defined(_REENTRANT)

int
__t_ilook (fd, tli)
	int	fd;
	struct	tli_st	*tli;
{
	long		type, data;
	struct strpeek	strp;
        int		code;

        code = -1;
	if (!(tli->tlis_flags & TLIS_SAVED_PROTO)) {
		strp.ctlbuf.buf = (char *)&type;
		strp.ctlbuf.maxlen = sizeof(type);
		strp.databuf.buf = (char *)data;
		strp.databuf.maxlen = sizeof(data);
		strp.flags = 0;
		switch (ioctl(fd, I_PEEK, &strp)) {
		case -1:
			t_unix_to_tli_error();
			goto rtn;
		case 0:
			code = 0;
			goto rtn;
		case 1:
			break;
		}
		if (strp.ctlbuf.len <= 0 && strp.databuf.len > 0) {
			if (tli->tlis_flags & TLIS_SAVED_EXDATA)
				return T_EXDATA;
			else
				return T_DATA;
		}
		if (strp.ctlbuf.len != sizeof(type)) {
			code = 0;
			goto rtn;
       		}
	} else
		type = ((union T_primitives *)tli->tlis_proto_buf)->type;

	switch (type) {
	case T_CONN_CON:
		code = T_CONNECT;
		break;
	case T_CONN_IND:
		code = T_LISTEN;
		break;
	case T_DATA_IND:
		code = T_DATA;
		break;
	case T_DISCON_IND:
		code = T_DISCONNECT;
		break;
	case T_EXDATA_IND:
		code = T_EXDATA;
		break;
	case T_ORDREL_IND:
		code = T_ORDREL;
		break;
	case T_UNITDATA_IND:
		code = T_DATA;
		break;
	case T_UDERROR_IND:
		code = T_UDERR;
		break;
	default:
		errno = EPROTO;
		t_errno = TSYSERR;
		break;
	}
rtn:
#ifdef XTIDBG
	tr_look (fd, code);
#endif /* XTIDBG */
	return code;
}

#ifdef	XTI
int
__t_look (fd, tli)
	int	fd;
	struct	tli_st	*tli;
{
	int		i1;
	struct pollfd	fds[1];

	i1 = __t_ilook(fd);
	if ( i1 != 0 )
		return i1;
	if (tli->tlis_flags & (TLIS_DATA_STOPPED | TLIS_EXDATA_STOPPED)) {
		fds[0].fd = fd;
		fds[0].events = POLLOUT;
		if (poll(fds, 1L, 0L) == 1
		&&  fds[0].revents == POLLOUT) {
			if (tli->tlis_flags & TLIS_EXDATA_STOPPED) {
				tli->tlis_flags &= ~TLIS_EXDATA_STOPPED;
				i1 = T_GOEXDATA;
			}
			else {
				tli->tlis_flags &= ~TLIS_DATA_STOPPED;
				i1 = T_GODATA;
		        }
		}
	}
#ifdef XTIDBG
	tr_look (fd, i1);
#endif /* XTIDBG */
	return i1;
}
#endif

#endif /* _THREAD_SAFE || _REENTRANT */
