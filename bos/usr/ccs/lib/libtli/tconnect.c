static char sccsid[] = "@(#)03  1.6  src/bos/usr/ccs/lib/libtli/tconnect.c, libtli, bos411, 9428A410j 4/20/94 18:21:57";
/*
 *   COMPONENT_NAME: LIBTLI
 *
 *   FUNCTIONS: t_connect, t_ircvconnect
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
 ** tconnect.c 1.4, last change 1/29/90
 **/

#include "common.h"
#include <sys/stropts.h>
#include <sys/tihdr.h>
#ifdef _THREAD_SAFE
#include <pthread.h>	/* WARNING: this header defines _THREAD_SAFE to be on */
#include <lib_lock.h>
#endif

#ifdef _THREAD_SAFE
/*
 *  tli_unlock()
 *	returns nothing.
 *  purpose:
 *	cleanup handler for threads that are cancelled and are still holding
 *	mutex locks.  since TLI_UNLOCK is a macro and pthread_cleanup_push
 *	takes as its first paramter a pointer to a function, this routine
 *	has been created for that task.
 */

void
tli_unlock(struct tli_st *tli)
{
	TLI_UNLOCK(tli);
}
#endif /* _THREAD_SAFE */

int
t_connect (fd, sndcall, rcvcall)
	int	fd;
	struct t_call	* sndcall;
	struct t_call	* rcvcall;
{
	char			* buf;
	char			stack_buf[TLI_STACK_BUF_SIZE];
	struct strbuf		ctlbuf;
	struct strbuf		databuf;
	int			ret;
	struct T_conn_req	* tcr;
	int			total_len;
	int     		code;
	struct tli_st		* tli;
	int			event;
	int			rc;

	code = -1;
	if (!(tli = iostate_lookup(fd, IOSTATE_VERIFY)))
		return code;

	if (tli->tlis_servtype == T_CLTS ) {
		t_errno = TNOTSUPPORT;
		goto rtn;
	}
	event = TLI_ILOOK(fd, tli);
	if (event == T_DISCONNECT || event == T_LISTEN) {
		t_errno = TLOOK;
		goto rtn;
	}
	if (tli->tlis_state != T_IDLE) {
		t_errno = TOUTSTATE;
		goto rtn;
	}
	if (!sndcall) {
		t_errno = TBADADDR;
		goto rtn;
	}

	total_len = sizeof(struct T_conn_req);
	if (sndcall->addr.len > 0)
		total_len += sndcall->addr.len;
	if (sndcall->opt.len > 0)
		total_len += sndcall->opt.len;

	if (total_len > sizeof(stack_buf)) {
		if (!(buf = (char *)malloc(total_len))) {
			t_errno = TSYSERR;
			errno = ENOMEM;
			goto rtn;
		}
	} else
		buf = stack_buf;

	tcr = (struct T_conn_req *)&buf[0];
	tcr->PRIM_type = T_CONN_REQ;
	if (sndcall->addr.len > 0) {
		tcr->DEST_length = sndcall->addr.len;
		tcr->DEST_offset = sizeof(struct T_conn_req);
		memcpy(&buf[tcr->DEST_offset], sndcall->addr.buf, tcr->DEST_length);
	} else {
		tcr->DEST_length = 0;
		tcr->DEST_offset = 0;
	}
	if (sndcall->opt.len > 0) {
		tcr->OPT_length = sndcall->opt.len;
		tcr->OPT_offset = tcr->DEST_offset + tcr->DEST_length;
		memcpy(&buf[tcr->OPT_offset], sndcall->opt.buf, tcr->OPT_length);
	} else {
		tcr->OPT_length = 0;
		tcr->OPT_offset = 0;
	}
	ctlbuf.buf = (char *)tcr;
	ctlbuf.len = total_len;
	/*
	 * Only send data if the length is greater than 0.
	 * You can't send 0 bytes.
	 */
	if (sndcall->udata.len > 0) {
		databuf.buf = sndcall->udata.buf;
		databuf.len = sndcall->udata.len;
	} else
		databuf.len = -1;

	ret = putmsg(fd, &ctlbuf, &databuf, 0);

	if (buf != stack_buf)
		free(buf);

	if (ret == -1) {
		t_unix_to_tli_error();
		goto rtn;
	}

	/* Check for connection request acknowledgment */
	if ( t_chk_ack(fd, tli, T_CONN_REQ) == -1 )
		goto rtn;

	/* for non-block i/o return with TNODATA */
	/*  Since t_is_nonblocking() calls fcntl which is
	 *  a cancellation point, push a cleanup handler 
	 *  unto the thread's cleanup handler stack in
	 *  case the thread gets cancelled, and fcntl
	 *  catches it.
	 */

#ifdef _THREAD_SAFE
	pthread_cleanup_push(tli_unlock, tli);
#endif /* _THREAD_SAFE */
	rc = t_is_nonblocking(fd);

	/* Ok, the thread wasn't cancelled.  Pop the
	 *  cleanup handler off the stack.
	 */
#ifdef _THREAD_SAFE
	pthread_cleanup_pop(0);
#endif /* _THREAD_SAFE */

	if (rc) {
		t_errno = TNODATA;
		TLI_NEXTSTATE(tli, TLI_CONNECT2);
		goto rtn;
	}

	switch (t_ircvconnect(fd, tli, rcvcall)) {
	case 0:
		code = 0;
		TLI_NEXTSTATE(tli, TLI_CONNECT1);
		break;
	case TBUFOVFLW:
		t_errno = TBUFOVFLW;
		TLI_NEXTSTATE(tli, TLI_CONNECT1);
		break;
	case -1:
	default:
		break;
	}
rtn:
	TLI_UNLOCK(tli);

#ifdef XTIDBG
	tr_connect (fd, sndcall, rcvcall, code);
#endif
	return code;
}

/*
 * t_ircvconnect()
 *              return -1:
 *                      fatal error
 *              return TBUFOVFLW:
 *                      caller should return -1 and t_errno = TBUFOVFLW
 *                      this will allow state to transition to next state
 *              return 0:
 *                     no error
 */

int
t_ircvconnect ( fd, tli, rcvcall)
	int		fd;
	struct tli_st	* tli;
	struct t_call	* rcvcall;
{
	int                     ctl_len;
	int                     iflags;
	struct netbuf *         netbufp;
	int                     ret;
	struct T_conn_con *     tcc;

	/* Get the connection confirm */
	netbufp = rcvcall ? &rcvcall->udata : nilp(struct netbuf);
	switch (t_get_primitive(fd, tli, T_CONN_CON, 
		(int)sizeof(struct T_conn_con), netbufp)) {
		case TBUFOVFLW:
			return TBUFOVFLW;
		case -1:
			return -1;
		default:
			break;
	}

	if ( rcvcall ) {
		tcc = (struct T_conn_con *)tli->tlis_proto_buf;
		if (tcc->RES_length > 0  &&  rcvcall->addr.maxlen > 0) {
			if (rcvcall->addr.maxlen < tcc->RES_length) 
				return TBUFOVFLW;
			rcvcall->addr.len = tcc->RES_length;
			memcpy(rcvcall->addr.buf, &tli->tlis_proto_buf[tcc->RES_offset], rcvcall->addr.len);
		} else
			rcvcall->addr.len = 0;
		if (tcc->OPT_length > 0  &&  rcvcall->opt.maxlen > 0) {
			if (rcvcall->opt.maxlen < tcc->OPT_length) 
				return TBUFOVFLW;
			rcvcall->opt.len = tcc->OPT_length;
			memcpy(rcvcall->opt.buf, &tli->tlis_proto_buf[tcc->OPT_offset], rcvcall->opt.len);
		} else
			rcvcall->opt.len = 0;
	}
	return 0;
}
