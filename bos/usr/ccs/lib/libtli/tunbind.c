static char sccsid[] = "@(#)28  1.4  src/bos/usr/ccs/lib/libtli/tunbind.c, libtli, bos411, 9428A410j 3/8/94 19:14:19";
/*
 *   COMPONENT_NAME: LIBTLI
 *
 *   FUNCTIONS: t_unbind
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
 ** tunbind.c 1.2, last change 11/8/89
 **/

#include "common.h"
#include <sys/tihdr.h>
#include <sys/timod.h>

int
t_unbind (fd)
	int	fd;
{
	union T_primitives	tunbindr;
        int   			code;
	struct	tli_st		* tli;

        code = -1;
	if (!(tli = iostate_lookup(fd, IOSTATE_VERIFY)))
		return code;

	if (tli->tlis_state != T_IDLE) {
		t_errno = TOUTSTATE;
		goto rtn;
	}
	if (TLI_LOOK(fd, tli)) {
		t_errno = TLOOK;
		goto rtn;
	}
	tunbindr.type = T_UNBIND_REQ;
	code = tli_ioctl(fd, TI_UNBIND, &tunbindr, sizeof(struct T_unbind_req));
	if (code < 0)
		goto rtn;

	TLI_NEXTSTATE(tli, TLI_UNBIND);
rtn:
	TLI_UNLOCK(tli);

#ifdef XTIDBG
	tr_unbind (fd, code);
#endif
	return code;
}
