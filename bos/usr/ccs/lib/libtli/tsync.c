static char sccsid[] = "@(#)27  1.1.2.2  src/bos/usr/ccs/lib/libtli/tsync.c, libtli, bos411, 9428A410j 3/8/94 19:14:13";
/*
 *   COMPONENT_NAME: LIBTLI
 *
 *   FUNCTIONS: t_sync, __t_sync
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
 ** tsync.c 1.1, last change 8/9/89
 **/

#include "common.h"

int
t_sync (fd)
	int	fd;
{
	struct tli_st	* tli;
        int		code;

	code = -1;
	if (!(tli = iostate_lookup(fd, IOSTATE_SYNC)))
		return code;

	code = tli->tlis_state;
	TLI_UNLOCK(tli);
#ifdef XTIDBG
	tr_sync (fd, code);
#endif
	return code;
}

#if defined(_THREAD_SAFE) || defined(_REENTRANT)

#include <sys/tihdr.h>
#include <sys/timod.h>
/*
	This routine get calls when the process already has lock, so no need
	to call iostate_lookup(). The tli and fd both should be valid at this
	point and if ioctl() fails, return.
*/

int
__t_sync (tli, fd)
	struct	tli_st	*tli;
	int	fd;
{
	struct  T_info_ack      tinfo;


	tinfo.PRIM_type = T_INFO_REQ;
	if (tli_ioctl(fd, TI_GETINFO, (char *)&tinfo, sizeof(tinfo)) == -1)
		return;

	tli->tlis_servtype = tinfo.SERV_type;
	tli->tlis_etsdu_size = tinfo.ETSDU_size;
	tli->tlis_tsdu_size = tinfo.TSDU_size;
	tli->tlis_tidu_size = tinfo.TIDU_size;
	tli->tlis_state = _txstate(tinfo.CURRENT_state);
}

#endif
