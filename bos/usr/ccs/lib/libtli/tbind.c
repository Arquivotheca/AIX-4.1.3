static char sccsid[] = "@(#)00  1.4  src/bos/usr/ccs/lib/libtli/tbind.c, libtli, bos411, 9428A410j 3/8/94 19:12:03";
/*
 *   COMPONENT_NAME: LIBTLI
 *
 *   FUNCTIONS: t_bind
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
 ** tbind.c 1.3, last change 10/16/89
 **/

#include "common.h"
#include <sys/tihdr.h>
#include <sys/timod.h>

int
t_bind (fd, req, ret)
	int	fd;
	struct t_bind	* req;
	struct t_bind	* ret;
{
	char			* buf = 0;
	char			stack_buf[TLI_STACK_BUF_SIZE];
	struct T_bind_req	* tbindr;
	struct T_bind_ack	* tbinda;
	int			total_len;
	int			code;
	struct tli_st		* tli;

	code = -1;
	if (!(tli = iostate_lookup(fd, IOSTATE_VERIFY)))
		return code;

	if (tli->tlis_state != T_UNBND) {
		t_errno = TOUTSTATE;
		goto rtn;
	}

	total_len = req ? req->addr.len : 0;
	if (ret  &&  ret->addr.maxlen > total_len)
		total_len = ret->addr.maxlen;
	total_len += sizeof(struct T_bind_req);
	if (total_len > sizeof(stack_buf)) {
		if (!(buf = (char *)malloc(total_len))) {
			t_errno = TSYSERR;
			errno = ENOMEM;
			goto rtn;
		}
	} else
		buf = stack_buf;

	tbindr = (struct T_bind_req *)&buf[0];
	tbindr->PRIM_type = T_BIND_REQ;
	if (req) {
		tbindr->CONIND_number = req->qlen;
		tbindr->ADDR_length = req->addr.len;
		tbindr->ADDR_offset = (char *)&tbindr[1] - (char *)tbindr;
		memcpy((char *)&tbindr[1], req->addr.buf, req->addr.len);
	} else {
		tbindr->CONIND_number = 0;
		tbindr->ADDR_length = 0;
		tbindr->ADDR_offset = 0;
	}
	if (tli_ioctl(fd, TI_BIND, (char *)tbindr, total_len) == -1)
		goto rtn;

	if (ret) {
		tbinda = (struct T_bind_ack *)&buf[0];
		ret->qlen = tbinda->CONIND_number;
		if (tbinda->ADDR_length > 0  &&  ret->addr.maxlen > 0) {
			if (ret->addr.maxlen < tbinda->ADDR_length) {
				t_errno = TBUFOVFLW;
				goto rtn1;
			}
			ret->addr.len = tbinda->ADDR_length;
			memcpy(ret->addr.buf, &buf[tbinda->ADDR_offset], ret->addr.len);
		} else 
			ret->addr.len = 0;
	}
	code = 0;
rtn1:
	TLI_NEXTSTATE(tli, TLI_BIND);
rtn:
	if (buf && buf != stack_buf)
		free(buf);
	TLI_UNLOCK(tli);

#ifdef XTIDBG
	tr_bind (fd, req, ret, code);
#endif
	return code;
}
