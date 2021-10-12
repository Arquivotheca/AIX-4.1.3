static char sccsid[] = "@(#)20  1.1.1.3  src/bos/usr/ccs/lib/libtli/trcvrel.c, libtli, bos411, 9428A410j 3/8/94 19:13:27";
/*
 *   COMPONENT_NAME: LIBTLI
 *
 *   FUNCTIONS: t_rcvrel
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
 ** trcvrel.c 1.3, last change 1/29/90
 **/

#include "common.h"
#include <sys/stropts.h>
#include <sys/tihdr.h>
#include <sys/timod.h>

int
t_rcvrel (fd)
	int	fd;
{
	int			event;
        int     		code;
	struct tli_st		* tli;

        code = -1;
	if (!(tli = iostate_lookup(fd, IOSTATE_VERIFY))) 
		return code;

	if (tli->tlis_servtype != T_COTS_ORD) {
		t_errno = TNOTSUPPORT;
		goto rtn;
	}

	if ((tli->tlis_state != T_DATAXFER) && (tli->tlis_state != T_OUTREL)) {
		t_errno = TOUTSTATE;
		goto rtn;
	}

	event = TLI_ILOOK(fd, tli);
	if (event != T_ORDREL) {
		switch (event) {
		case 0:
			t_errno = TNOREL;
			break;
		case -1:
			break;
		default:
			t_errno = TLOOK;
			break;
		}
		goto rtn;
	}

	switch (t_get_primitive(fd, tli, T_ORDREL_IND,
		(int)sizeof(struct T_ordrel_ind), nilp(struct netbuf))) {
	case -1:
		goto rtn;
	case TBUFOVFLW:		/* not possible ! */
		TLI_NEXTSTATE(tli, TLI_RCVREL);
		goto rtn;
	case 0:
	default:
		break;
	}

	tli_ioctl(fd, TI_XTI_CLEAR_EVENT, nilp(char), 0);
	TLI_NEXTSTATE(tli, TLI_RCVREL);
	code = 0;
rtn:
	TLI_UNLOCK(tli);

#ifdef XTIDBG
	tr_rcvrel (fd, code);
#endif
	return code;
}
