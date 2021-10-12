static char sccsid[] = "@(#)24  1.5  src/bos/usr/ccs/lib/libtli/tsnddis.c, libtli, bos411, 9428A410j 7/12/94 12:27:42";
/*
 *   COMPONENT_NAME: LIBTLI
 *
 *   FUNCTIONS: t_snddis
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
 ** tsnddis.c 1.4, last change 1/29/90
 **/

#include "common.h"
#include <sys/stropts.h>
#include <sys/tihdr.h>

int
t_snddis (fd, call)
	int	fd;
	struct t_call * call;
{
	struct strbuf		ctlbuf;
	struct strbuf		databuf;
	struct T_discon_req	tdr;
	struct tli_st 		* tli;
        int    			code;

        code = -1;
	if (!(tli = iostate_lookup(fd, IOSTATE_VERIFY)))
		return code;

	if ( tli->tlis_servtype == T_CLTS ) {
		t_errno = TNOTSUPPORT;
		goto rtn;
	}

	if (TLI_ILOOK(fd, tli) == T_DISCONNECT) {
		t_errno = TLOOK;
		goto rtn;
	}
	if (tli->tlis_state == T_IDLE || tli->tlis_state == T_UNBND) {
		t_errno = TOUTSTATE;
		goto rtn;
	}

	tdr.PRIM_type = T_DISCON_REQ;
	ctlbuf.buf = (char *)&tdr;
	ctlbuf.len = sizeof(struct T_discon_req);
	if ( call ) {
		tdr.SEQ_number = call->sequence;
		databuf.buf = call->udata.buf;
		databuf.len = call->udata.len ? call->udata.len : -1;
	} else {
		tdr.SEQ_number = -1;
		databuf.len = -1;
	}
	if (putmsg(fd, &ctlbuf, &databuf, 0) == -1) {
		t_unix_to_tli_error();
		goto rtn;
	}
	code = t_chk_ack(fd, tli, T_DISCON_REQ);

	TLI_TSYNC(tli, fd);
#ifdef XTI
	if (call)
		tli->tlis_sequence--;
#endif
rtn:
	TLI_UNLOCK(tli);

#ifdef XTIDBG
	tr_snddis (fd, call, code);
#endif
	return code;
}

