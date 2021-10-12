static char sccsid[] = "@(#)16  1.5  src/bos/usr/ccs/lib/libtli/toptmgmt.c, libtli, bos411, 9428A410j 3/8/94 19:12:59";
/*
 *   COMPONENT_NAME: LIBTLI
 *
 *   FUNCTIONS: t_optmgmt
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
 ** toptmgmt.c 1.3, last change 10/16/89
 **/

#include "common.h"
#include <sys/tihdr.h>
#include <sys/timod.h>

int
t_optmgmt (fd, req, ret)
	int	fd;
	struct t_optmgmt * req;
	struct t_optmgmt * ret;
{
	char			* buf = 0;
	char			stack_buf[TLI_STACK_BUF_SIZE];
	struct T_optmgmt_req	* toptmgmtr;
	struct T_optmgmt_ack	* toptmgmta;
	int			total_len;
        int     		code;
	struct	tli_st		* tli;

        code = -1;
	if (!(tli = iostate_lookup(fd, IOSTATE_VERIFY))) 
		return code;

#ifndef XTI
	if (tli->tlis_state == T_UNBND) {
		t_errno = TOUTSTATE;
		goto rtn;
	}
#endif
#ifdef XTI
	if (req->flags != T_NEGOTIATE && req->flags != T_DEFAULT &&
		req->flags != T_CURRENT && req->flags != T_CHECK) {
#else
	if (req->flags != T_NEGOTIATE && req->flags != T_DEFAULT && 
		req->flags != T_CHECK) {
#endif
		t_errno = TBADFLAG;
		goto rtn;
	}

	total_len = req ? req->opt.len : 0;
	if (ret  &&  ret->opt.maxlen > total_len)
		total_len = ret->opt.maxlen;
	total_len += sizeof(struct T_optmgmt_req);
	if (total_len > sizeof(stack_buf)) {
		if (!(buf = (char *)malloc(total_len))) {
			t_errno = TSYSERR;
			errno = ENOMEM;
			goto rtn;
		}
	} else
		buf = stack_buf;

	toptmgmtr = (struct T_optmgmt_req *)&buf[0];
	toptmgmtr->PRIM_type = T_OPTMGMT_REQ;
	if (req) {
		toptmgmtr->MGMT_flags = req->flags;
		toptmgmtr->OPT_length = req->opt.len;
		toptmgmtr->OPT_offset = (char *)&toptmgmtr[1] - (char *)toptmgmtr;
		memcpy((char *)&toptmgmtr[1], req->opt.buf, req->opt.len);
	} else {
		toptmgmtr->MGMT_flags = 0;
		toptmgmtr->OPT_length = 0;
		toptmgmtr->OPT_offset = 0;
	}
	if (tli_ioctl(fd, TI_OPTMGMT, toptmgmtr, total_len) == -1)
		goto rtn;

	if (ret) {
		toptmgmta = (struct T_optmgmt_ack *)&buf[0];
		ret->flags = toptmgmta->MGMT_flags;
		if (toptmgmta->OPT_length > 0  &&  ret->opt.maxlen > 0) {
			if (ret->opt.maxlen < toptmgmta->OPT_length) {
				t_errno = TBUFOVFLW;
				goto rtn;
			}
			ret->opt.len = toptmgmta->OPT_length;
			memcpy(ret->opt.buf, &buf[toptmgmta->OPT_offset], ret->opt.len);
		} else
			ret->opt.len = 0;
	}
	TLI_NEXTSTATE(tli, TLI_OPTMGMT);
        code = 0;
rtn:
	if (buf && buf != stack_buf)
		free(buf);
	TLI_UNLOCK(tli);

#ifdef XTIDBG
	tr_optmgmt (fd, req, ret, code);
#endif
	return code;
}
