static char sccsid[] = "@(#)09  1.4  src/bos/usr/ccs/lib/libtli/tgetinfo.c, libtli, bos411, 9428A410j 3/8/94 19:12:29";
/*
 *   COMPONENT_NAME: LIBTLI
 *
 *   FUNCTIONS: t_getinfo, tli_ioctl
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
 ** tgetinfo.c 1.2, last change 11/8/89
 **/

#include "common.h"
#include <sys/stropts.h>
#include <sys/tihdr.h>
#include <sys/timod.h>

int
t_getinfo (fd, info)
	int	fd;
	struct t_info	* info;
{
	struct T_info_ack	tinfoack;
        int    			code;

	if (info == NULL) {
		code = 0;
		goto rtn;
	}

        code = -1;
	tinfoack.PRIM_type = T_INFO_REQ;
	if (tli_ioctl(fd,TI_GETINFO, (char *)&tinfoack, sizeof(tinfoack)) == -1)
		goto rtn;

	info->addr = tinfoack.ADDR_size;
	info->options = tinfoack.OPT_size;
	info->tsdu = tinfoack.TSDU_size;
	info->etsdu = tinfoack.ETSDU_size;
	info->connect = tinfoack.CDATA_size;
	info->discon = tinfoack.DDATA_size;
	info->servtype = tinfoack.SERV_type;
#ifdef XTI
	info->flags = tinfoack.PROVIDER_flag;
#endif
        code = 0;
rtn:
#ifdef XTIDBG
	tr_getinfo (fd, info, code);
#endif
	return code;
}

int
tli_ioctl (fd, cmd, dp, len)
	int	fd;
	int	cmd;
	char	* dp;
	int	len;
{
	int	ret;
	struct strioctl	stri;

	stri.ic_cmd = cmd;
	stri.ic_timout = -1;
	stri.ic_len = len;
	stri.ic_dp = dp;
	ret = ioctl(fd, I_STR, &stri);
	if (ret == -1) {
		t_unix_to_tli_error();
		return -1;
	}
	if (ret != 0) {
		t_errno = ret & 0xff;
		errno = ret >> 8;
		return -1;
	}
	return 0;
}
