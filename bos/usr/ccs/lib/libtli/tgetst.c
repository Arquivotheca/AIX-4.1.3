static char sccsid[] = "@(#)10        1.2.1.2  src/bos/usr/ccs/lib/libtli/tgetst.c, libtli, bos411, 9428A410j 12/20/93 17:25:12";
/*
 *   COMPONENT_NAME: LIBTLI
 *
 *   FUNCTIONS: t_getstate
 *
 *   ORIGINS: 18 27 63
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991, 1993
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
 ** tgetst.c 1.2, last change 1/29/90
 **/


#include "common.h"

int
t_getstate (fd)
	int	fd;
{
	struct	tli_st	*tli;
        int	code;

        code = -1;
	if (!(tli = iostate_lookup(fd, IOSTATE_VERIFY)))
		return code;

	if (tli->tlis_state == -1 || tli->tlis_state == TLI_TSTATECHNG) {
		if (TLI_TSYNC(tli, fd) < 0)
			goto rtn;
	}
	code = tli->tlis_state;
rtn:
	TLI_UNLOCK(tli);
#ifdef XTIDBG
	tr_getstate (fd, code);
#endif
	return code;
}
