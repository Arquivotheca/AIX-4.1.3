static char sccsid[] = "@(#)18        1.5  src/bos/usr/ccs/lib/libtli/trcvcon.c, libtli, bos411, 9428A410j 3/8/94 19:13:12";
/*
 *   COMPONENT_NAME: LIBTLI
 *
 *   FUNCTIONS: t_rcvconnect
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
 ** trcvcon.c 1.3, last change 10/16/89
 **/

#include "common.h"
#include <sys/stropts.h>
#include <sys/tihdr.h>

int
t_rcvconnect (fd, call)
	int	fd;
reg	struct t_call * call;
{
	struct tli_st	* tli;
        int             code;
	int		event;

        code = -1;
	if (!(tli = (struct tli_st *)iostate_lookup(fd, IOSTATE_VERIFY)))
	        return code;

	if (tli->tlis_servtype == T_CLTS) {
		t_errno = TNOTSUPPORT;
	        goto rtn;
	}

	if (tli->tlis_state != T_OUTCON) {
		t_errno = TOUTSTATE;
	        goto rtn;
	}
	event = TLI_LOOK(fd, tli);
	if (event) {
		if (event != T_CONNECT) {
			t_errno = TLOOK;
			goto rtn;
		}
	}

	switch (t_ircvconnect(fd, tli, call)) {
	case 0:
		code = 0;
		break;
	case TBUFOVFLW:
		t_errno = TBUFOVFLW;
		break;
	case -1:
	default:
		break;
	}
rtn:
	if (code == 0 || ((code == -1) && (t_errno == TBUFOVFLW)))
		TLI_NEXTSTATE(tli, TLI_RCVCONN);

	TLI_UNLOCK(tli);
#ifdef XTIDBG
	tr_rcvcon (fd, call, code);
#endif
	return code;
}
