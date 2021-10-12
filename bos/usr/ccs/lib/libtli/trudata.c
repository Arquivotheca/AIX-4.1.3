static char sccsid[] = "@(#)21  1.4  src/bos/usr/ccs/lib/libtli/trudata.c, libtli, bos411, 9428A410j 3/8/94 19:13:35";
/*
 *   COMPONENT_NAME: LIBTLI
 *
 *   FUNCTIONS: t_rcvudata
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
 ** trudata.c 1.3, last change 10/16/89
 **/

#include "common.h"
#include <sys/stropts.h>
#include <sys/tihdr.h>

int
t_rcvudata (fd, ud, flags)
	int	fd;
reg	struct t_unitdata * ud;
	int	* flags;
{
reg	struct T_unitdata_ind	* tudi;
	struct strbuf		ctlbuf;
	struct strbuf		databuf;
	int			iflags;
	int			len;
	int			ret;
        int     		code;
	struct tli_st		* tli;

        code = -1;
	if (!(tli = iostate_lookup(fd, IOSTATE_VERIFY)))
		return code;

	if ( tli->tlis_servtype != T_CLTS ) {
		t_errno = TNOTSUPPORT;
		goto rtn;
	}
	if ( tli->tlis_state != T_IDLE ) {
		int event = TLI_LOOK(fd, tli);

		if (event) {
			if (event != T_DATA) {
				t_errno = TLOOK;
				goto rtn;
			}
		} else {
			t_errno = TOUTSTATE;
			goto rtn;
		}
	}

	if (tli->tlis_flags & TLIS_SAVED_PROTO) {
		t_errno = TLOOK;
		goto rtn;
	}
											ctlbuf.buf = tli->tlis_proto_buf;
	ctlbuf.maxlen = _DEFAULT_STRCTLSZ;

	databuf.buf = ud->udata.buf;
	databuf.maxlen = ud->udata.maxlen;
	
	iflags = 0;
	ret = getmsg(fd, &ctlbuf, &databuf, &iflags);
	switch(ret) {
	case MORECTL:
	case MORECTL|MOREDATA:
		/* This should *never* happen... */
		t_errno = TSYSERR;
		errno = EPROTO;
		goto rtn;
	case MOREDATA:
		if (flags)
			*flags = T_MORE;
		break;
	case 0:
		if (flags)
			*flags = 0;
		break;
	case -1:
	default:
		t_unix_to_tli_error();
		goto rtn;
	}

	ud->udata.len = databuf.len > 0 ? databuf.len : 0;
	ud->addr.len = 0;
	ud->opt.len = 0;

	len = ctlbuf.len;
	if (len >= (int)sizeof(long)) {
		if (((union T_primitives *)tli->tlis_proto_buf)->type != T_UNITDATA_IND) {
			tli->tlis_flags |= TLIS_SAVED_PROTO;
			t_errno = TLOOK;
			goto rtn;
		}
	}
	if (len >= (int)sizeof(struct T_unitdata_ind)) {
		tudi = (struct T_unitdata_ind *)tli->tlis_proto_buf;
		len = tudi->SRC_length;
		if (len > 0  &&  ud->addr.maxlen > 0) {
			if (ud->addr.maxlen < len) {
				t_errno = TBUFOVFLW;
				goto rtn;
			}
			ud->addr.len = len;
			memcpy(ud->addr.buf, &tli->tlis_proto_buf[tudi->SRC_offset], len);
		}
		len = tudi->OPT_length;
		if (len > 0  &&  ud->opt.maxlen > 0) {
			if (ud->opt.maxlen < len) {
				t_errno = TBUFOVFLW;
				goto rtn;
			}
			ud->opt.len = len;
			memcpy(ud->opt.buf, &tli->tlis_proto_buf[tudi->OPT_offset], len);
		}
	} else if (databuf.len == 0) {
		t_errno = TNODATA;
		goto rtn;
	}
	code = 0;
	if ( *flags & T_MORE ) {
		if (! (tli->tlis_flags & TLIS_MORE_RUDATA))
			tli->tlis_flags |= TLIS_MORE_RUDATA;
		else {
			ud->addr.len = 0;
			ud->opt.len = 0;
		}
	} else {
		if (tli->tlis_flags & TLIS_MORE_RUDATA) {
			ud->addr.len = 0;
			ud->opt.len = 0;
		}
		tli->tlis_flags &= ~TLIS_MORE_RUDATA;
	}

rtn:
	TLI_UNLOCK(tli);

#ifdef XTIDBG
	tr_rcvudata (fd, ud, flags, code);
#endif
	return code;
}
