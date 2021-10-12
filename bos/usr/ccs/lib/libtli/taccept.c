static char sccsid[] = "@(#)98  1.1.1.4  src/bos/usr/ccs/lib/libtli/taccept.c, libtli, bos411, 9428A410j 7/12/94 12:21:43";
/*
 *   COMPONENT_NAME: LIBTLI
 *
 *   FUNCTIONS: t_accept
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
 ** taccept.c 1.2, last change 6/4/90
 **/

#include "common.h"
#include <sys/stropts.h>
#include <sys/tihdr.h>
#ifdef XTI
#include <sys/timod.h>
#endif

int
t_accept (fd, resfd, call)
	int	fd;
	int	resfd;
	struct	t_call * call;
{
	char			* buf;
	int			event;
	char			stack_buf[TLI_STACK_BUF_SIZE];
	struct strfdinsert	strfd;
	struct T_conn_res	* tcr;
	int			total_len;
	int     		code;
	int     		ret;
	struct tli_st		* tli, * tli2;
#ifdef XTI
	XTIS	xtis;
#endif

	code = -1;
	if (!(tli = iostate_lookup(fd, IOSTATE_VERIFY)))
		return code;

	if (fd != resfd) {
		if (!(tli2 = iostate_lookup(resfd, IOSTATE_VERIFY))) {
			TLI_UNLOCK(tli);
			return code;
		}
	}

	if (tli->tlis_servtype == T_CLTS ) {
		t_errno = TNOTSUPPORT;
		goto rtn;
	}
	if (tli->tlis_state != T_INCON) {
		t_errno = TOUTSTATE;
		goto rtn;
	}

	event = TLI_LOOK(fd, tli);
	if (event) {
		if (event == T_LISTEN && fd == resfd) {
#ifdef XTI
			t_errno = TINDOUT;
#else
			t_errno = TBADF;
#endif
		} else
			t_errno = TLOOK;
		goto rtn;
	}
#ifdef XTI
	if ((fd != resfd) && (tli2->tlis_state != T_IDLE && 
				tli2->tlis_state != T_UNBND)) {
#else
	if ((fd != resfd) && (tli2->tlis_state != T_IDLE)) {
#endif
		t_errno = TOUTSTATE;
		goto rtn;
	}
#ifdef XTI
	if ((fd != resfd) && 
	    (tli_ioctl(resfd, TI_XTI_GET_STATE,(char *)&xtis, sizeof(xtis))== -1
			||  xtis.xtis_qlen > 0)) {
		t_errno = TRESQLEN;
		goto rtn;
	}
#endif
	if (!call) {
		t_errno = TBADADDR;
		goto rtn;
	}

	total_len = (call->opt.len >= 0 ? call->opt.len : 0);
	total_len += sizeof(struct T_conn_res);
	if (total_len > sizeof(stack_buf)) {
		if (!(buf = (char *)malloc(total_len))) {
			t_errno = TSYSERR;
			errno = ENOMEM;
			goto rtn;
		}
	} else
		buf = stack_buf;

	tcr = (struct T_conn_res *)&buf[0];
	tcr->PRIM_type = T_CONN_RES;
	if (call->opt.len > 0) {
		tcr->OPT_length = call->opt.len;
		tcr->OPT_offset = sizeof(struct T_conn_res);
		memcpy(&buf[tcr->OPT_offset], call->opt.buf, tcr->OPT_length);
	} else {
		tcr->OPT_length = 0;
		tcr->OPT_offset = 0;
	}
	tcr->SEQ_number = call->sequence;
	strfd.ctlbuf.buf = (char *)tcr;
	strfd.ctlbuf.len = total_len;

	/*
	 * Only send data if the length is greater than 0.
	 * You can't send 0 bytes.
	 */
	if (call->udata.len > 0) {
		strfd.databuf.buf = call->udata.buf;
		strfd.databuf.len = call->udata.len;
	} else
		strfd.databuf.len = -1;

	strfd.flags = 0;
	strfd.fildes = resfd;
	strfd.offset = sizeof(long);
	ret = ioctl(fd, I_FDINSERT, (char *)&strfd);

	if (buf != stack_buf)
		free(buf);

	if (ret == -1) {
		t_unix_to_tli_error();
		goto rtn;
	}
	/* Check for accept acknowledgment */
	if ((code = t_chk_ack(fd, tli, T_CONN_RES)) == -1) {
		if (t_errno == TOUTSTATE) {
			switch (TLI_LOOK(fd, tli)) {
			case T_DISCONNECT:
			case T_LISTEN:
				t_errno = TLOOK;
			}
		}
		goto rtn;
	}

	TLI_TSYNC(tli, fd);
	if (resfd != fd) {
		TLI_NEXTSTATE(tli2, TLI_PASSCON);
	}
rtn:   
#ifdef XTI
	if (code == 0 || (code == -1 && t_errno == TLOOK && 
		(TLI_LOOK(fd, tli) == T_DISCONNECT)))
		tli->tlis_sequence--;
#endif
	TLI_UNLOCK(tli);
	if ( resfd != fd )
		TLI_UNLOCK(tli2);

#ifdef XTIDBG
        tr_accept (fd, resfd, call, code);
#endif
	return code;
}
