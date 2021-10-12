static char sccsid[] = "@(#)19  1.2.1.4  src/bos/usr/ccs/lib/libtli/trcvdis.c, libtli, bos411, 9428A410j 7/12/94 12:26:54";
/*
 *   COMPONENT_NAME: LIBTLI
 *
 *   FUNCTIONS: t_rcvdis
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
 ** trcvdis.c 1.4, last change 1/29/90
 **/

#include "common.h"
#include <sys/stropts.h>
#include <sys/tihdr.h>
#include <sys/timod.h>

int
t_rcvdis (fd, discon)
	int	fd;
reg	struct t_discon * discon;
{
	struct T_discon_ind	* tdi;
	struct netbuf 		* netbufp;
	struct tli_st		* tli;
        int			code;
	int			event;

        code = -1;
	if (!(tli = iostate_lookup(fd, IOSTATE_VERIFY))) 
		return code;

	if (tli->tlis_servtype == T_CLTS) {
		t_errno = TNOTSUPPORT;
		goto rtn;
	}
	if ((event = TLI_ILOOK(fd, tli)) != T_DISCONNECT) {
		if (event != -1)
			t_errno = TNODIS;
		goto rtn;
	}

	if ((tli->tlis_state == T_IDLE && event != T_DISCONNECT)
		|| tli->tlis_state == T_UNBND) {
		t_errno = TOUTSTATE;
		goto rtn;
	}

	netbufp = discon ? &discon->udata : nilp(struct netbuf);
	switch (t_get_primitive(fd, tli, T_DISCON_IND,
		(int)sizeof(struct T_discon_ind), netbufp)) {
	case -1:
		if (t_errno == TNODATA  ||  t_errno == TLOOK)
			t_errno = TNODIS;
		goto rtn;
	case TBUFOVFLW:
		TLI_TSYNC(tli, fd);
		goto rtn;
	case 0:
	default:
		break;
	}

	if ( discon ) {
		tdi = (struct T_discon_ind *)tli->tlis_proto_buf;
		discon->reason = tdi->DISCON_reason;
		discon->sequence = tdi->SEQ_number;
	}

	tli_ioctl(fd, TI_XTI_CLEAR_EVENT, nilp(char), 0);
	TLI_TSYNC(tli, fd);
	code = 0;
#ifdef XTI
	if (tli->tlis_sequence)
		tli->tlis_sequence--;
#endif

rtn:
	TLI_UNLOCK(tli);

#ifdef XTIDBG
	tr_rcvdis (fd, discon, code);
#endif
	return code;
}
