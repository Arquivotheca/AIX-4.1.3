static char sccsid[] = "@(#)17        1.5  src/bos/usr/ccs/lib/libtli/trcv.c, libtli, bos411, 9428A410j 3/8/94 19:13:06";
/*
 *   COMPONENT_NAME: LIBTLI
 *
 *   FUNCTIONS: t_rcv
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
 ** trcv.c 1.1, last change 8/9/89
 **/

#include "common.h"
#include <sys/stropts.h>
#include <sys/tihdr.h>

int
t_rcv (fd, buf, nbytes, flags)
	int	fd;
	char	* buf;
	unsigned nbytes;
	int	* flags;
{
	struct strbuf		ctlbuf;
	struct strbuf		databuf;
	boolean			data_only;
	int			iflags;
	int			ret;
	struct T_data_ind 	* tdi;
	struct tli_st		* tli;
        int     		code;

        code = -1;
	if (!(tli = (struct tli_st *)iostate_lookup(fd, IOSTATE_VERIFY)))
		return code;

	if ( tli->tlis_state != T_DATAXFER  &&  tli->tlis_state != T_OUTREL ) {
		int event = TLI_LOOK(fd, tli);

		if (event) {
			if (event != T_DATA && event != T_EXDATA) {
				t_errno = TLOOK;
				goto rtn;
			}
		} else {
			t_errno = TOUTSTATE;
			goto rtn;
		}
	}

	if ( tli->tlis_servtype == T_CLTS ) {
		t_errno = TNOTSUPPORT;
		goto rtn;
	}
	data_only = true;
	if (tli->tlis_flags & TLIS_SAVED_PROTO) {
		long    type= ((union T_primitives *)tli->tlis_proto_buf)->type;
		if (type != T_DATA_IND  &&  type != T_EXDATA_IND) {
			t_errno = TLOOK;
			goto rtn;
		}
		tdi = (struct T_data_ind *)tli->tlis_proto_buf;
		data_only = false;
	}

	ctlbuf.buf = tli->tlis_proto_buf;
	ctlbuf.maxlen = _DEFAULT_STRCTLSZ;
	databuf.buf = buf;
	databuf.maxlen = nbytes;
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
			if (ctlbuf.len != -1)
				tli->tlis_flags |= TLIS_SAVED_PROTO;
			tli->tlis_flags |= TLIS_MORE_DATA;
			*flags |= T_MORE;
			break;
		case 0:
			*flags = 0;
			tli->tlis_flags &= ~(TLIS_SAVED_PROTO | 
					TLIS_SAVED_EXDATA | TLIS_MORE_DATA);
			break;
		case -1:
		default:
			t_unix_to_tli_error();
			goto rtn;
	}
	if (ctlbuf.len != -1) {
		tdi = (struct T_data_ind *)tli->tlis_proto_buf;
		if (ctlbuf.len != (int)sizeof(struct T_data_ind) || 
		   (tdi->PRIM_type != T_DATA_IND && tdi->PRIM_type != T_EXDATA_IND)) {
			tli->tlis_flags |= TLIS_SAVED_PROTO;
			t_errno = TLOOK;
			goto rtn;
		}
		data_only = false;
	}

	if (!data_only) {
		if (tdi->MORE_flag == T_MORE)
			tli->tlis_flags |= TLIS_SAVED_DATA;
		else
			tli->tlis_flags &= ~TLIS_SAVED_DATA;

		if ( tdi->PRIM_type == T_EXDATA_IND )
			tli->tlis_flags |= TLIS_SAVED_EXDATA;
		else
			tli->tlis_flags &= ~TLIS_SAVED_EXDATA;
	}
	if (tli->tlis_flags & TLIS_SAVED_DATA)
		*flags |= T_MORE;
	if (tli->tlis_flags & TLIS_SAVED_EXDATA)
		*flags |= T_EXPEDITED;

	code = databuf.len;
	TLI_NEXTSTATE(tli, TLI_RCV);
rtn:
	TLI_UNLOCK(tli);

#ifdef XTIDBG
        tr_rcv (fd, buf, nbytes, flags, code);
#endif
	return code;
}
