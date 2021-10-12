static char sccsid[] = "@(#)12        1.6  src/bos/usr/ccs/lib/libtli/tlisten.c, libtli, bos411, 9428A410j 7/12/94 12:25:15";
/*
 *   COMPONENT_NAME: LIBTLI
 *
 *   FUNCTIONS: t_listen
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
 ** tlisten.c 1.5, last change 1/29/90
 **/

#include "common.h"
#include <sys/stropts.h>
#include <sys/tihdr.h>
#include <sys/timod.h>

int
t_listen (fd, call)
	int		fd;
reg	struct t_call	* call;
{
	struct T_conn_ind * tci;
	struct tli_st	* tli;
	XTIS	xtis;
        int	code;
	int	event;

        code = -1;
	if (!(tli = iostate_lookup(fd, IOSTATE_VERIFY))) 
		return code;

	if ( tli->tlis_servtype == T_CLTS ) {
		t_errno = TNOTSUPPORT;
		goto rtn;
	}
	if (tli->tlis_state != T_IDLE &&  tli->tlis_state != T_INCON) {
		t_errno = TOUTSTATE;
		goto rtn;
	}
	event = TLI_LOOK(fd, tli);
	if (event && event != T_LISTEN) {
		t_errno = TLOOK;
		goto rtn;
	}
	if (tli_ioctl(fd, TI_XTI_GET_STATE, (char *)&xtis, sizeof(xtis)) == -1
	||  xtis.xtis_qlen <= 0) {
		t_errno = TBADQLEN;
		goto rtn;
	}

#ifdef XTI
	if (tli->tlis_sequence > xtis.xtis_qlen) {
		t_errno = TQFULL;
		goto rtn;
	}
#endif

	switch (t_get_primitive(fd, tli, T_CONN_IND,
		(int)sizeof(struct T_conn_ind), &call->udata)) {
		case -1:
		case TBUFOVFLW:
			goto rtn;
		case 0:
		default:
			break;
	}

	tci = (struct T_conn_ind *)tli->tlis_proto_buf;
	call->sequence = tci->SEQ_number;

	if (tci->SRC_length > 0  &&  call->addr.maxlen > 0) {
		if (call->addr.maxlen < tci->SRC_length) {
			t_errno = TBUFOVFLW;
			goto rtn;
		}
		call->addr.len = tci->SRC_length;
		memcpy(call->addr.buf, &tli->tlis_proto_buf[tci->SRC_offset], call->addr.len);
	} else 
		call->addr.len = 0;

	if (tci->OPT_length > 0  &&  call->opt.maxlen > 0) {
		if (call->opt.maxlen < tci->OPT_length) {
			t_errno = TBUFOVFLW;
			goto rtn;
		}
		call->opt.len = tci->OPT_length;
		memcpy(call->opt.buf, &tli->tlis_proto_buf[tci->OPT_offset], call->opt.len);
	} else
		call->opt.len = 0;
        code = 0;

#ifdef XTI
	tli->tlis_sequence++;
#endif
rtn:
	if (code == 0 || ((code == -1) && (t_errno == TBUFOVFLW)))
		TLI_NEXTSTATE(tli, TLI_LISTEN);
	TLI_UNLOCK(tli);

#ifdef XTIDBG
	tr_listen (fd, call, code);
#endif
	return code;
}
