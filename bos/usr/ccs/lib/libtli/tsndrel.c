static char sccsid[] = "@(#)25  1.4  src/bos/usr/ccs/lib/libtli/tsndrel.c, libtli, bos411, 9428A410j 3/8/94 19:14:02";
/*
 *   COMPONENT_NAME: LIBTLI
 *
 *   FUNCTIONS: t_sndrel
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
 ** tsndrel.c 1.2, last change 1/29/90
 **/

#include "common.h"
#include <sys/stropts.h>
#include <sys/tihdr.h>

int
t_sndrel (fd)
	int	fd;
{
	struct strbuf		ctlbuf;
	struct T_ordrel_req	tor;
	struct tli_st		* tli;
	int    			code;

        code = -1;
	if (!(tli = iostate_lookup(fd, IOSTATE_VERIFY))) 
		return code;

	if (tli->tlis_servtype != T_COTS_ORD) {
		t_errno = TNOTSUPPORT;
		goto rtn;
	}
	if (tli->tlis_state != T_DATAXFER  &&  tli->tlis_state != T_INREL) {
		t_errno = TOUTSTATE;
		goto rtn;
	}
	if (TLI_LOOK(fd, tli) == T_DISCONNECT) {
		t_errno = TLOOK;
		goto rtn;
	}
	tor.PRIM_type = T_ORDREL_REQ;
	ctlbuf.buf = (char *)&tor;
	ctlbuf.len = sizeof(tor);
	if (putmsg(fd, &ctlbuf, nilp(struct strbuf), 0) == -1) {
		t_unix_to_tli_error();
		if ( t_errno == TNODATA )
			t_errno = TFLOW;
		goto rtn;
	}
	TLI_NEXTSTATE(tli, TLI_SNDREL);
	code = 0;
rtn:
	TLI_UNLOCK(tli);

#ifdef XTIDBG
	tr_sndrel (fd, code);
#endif
	return code;
}
