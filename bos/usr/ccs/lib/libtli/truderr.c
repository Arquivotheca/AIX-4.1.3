static char sccsid[] = "@(#)22  1.1.1.3  src/bos/usr/ccs/lib/libtli/truderr.c, libtli, bos411, 9428A410j 3/8/94 19:13:42";
/*
 *   COMPONENT_NAME: LIBTLI
 *
 *   FUNCTIONS: t_rcvuderr
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
 ** truderr.c 1.5, last change 1/29/90
 **/

#include "common.h"
#include <sys/stropts.h>
#include <sys/tihdr.h>
#include <sys/timod.h>

int
t_rcvuderr (fd, uderr)
	int	fd;
	struct t_uderr * uderr;
{
	struct T_uderror_ind	* tudei;
        int     		code;
        int     		event;
	struct tli_st		* tli;

        code = -1;
	if (!(tli = iostate_lookup(fd, IOSTATE_VERIFY)))
		return code;

	if (tli->tlis_servtype != T_CLTS ) {
		t_errno = TNOTSUPPORT;
		goto rtn;
	}
	event = TLI_ILOOK(fd, tli);
	if (event != T_UDERR) {
		t_errno = TNOUDERR;
		goto rtn;
	}

	switch (t_get_primitive(fd, tli, T_UDERROR_IND,
		(int)sizeof(struct T_uderror_ind), nilp(struct netbuf))) {
	case -1:
		if ((t_errno == TNODATA)  || (t_errno == TLOOK))
			t_errno = TNOUDERR;
		goto rtn;
	case TBUFOVFLW:
		goto rtn;
	case 0:
	default:
		break;
	}

	tli_ioctl(fd, TI_XTI_CLEAR_EVENT, nilp(char), 0);
	if (uderr) {
                tudei = (struct T_uderror_ind *)tli->tlis_proto_buf;
		uderr->error = tudei->ERROR_type;
		if (tudei->DEST_length > 0  &&  uderr->addr.maxlen > 0) {
			if (uderr->addr.maxlen < tudei->DEST_length) {
				t_errno = TBUFOVFLW;
				goto rtn;
			}
			uderr->addr.len = tudei->DEST_length;
			memcpy(uderr->addr.buf, &tli->tlis_proto_buf[tudei->DEST_offset], uderr->addr.len);
		} else
			uderr->addr.len = 0;
		if (tudei->OPT_length > 0  &&  uderr->opt.maxlen > 0) {
			if (uderr->opt.maxlen < tudei->OPT_length) {
				t_errno = TBUFOVFLW;
				goto rtn;
			}
			uderr->opt.len = tudei->OPT_length;
			memcpy(uderr->opt.buf, &tli->tlis_proto_buf[tudei->OPT_offset], uderr->opt.len);
		} else
			uderr->opt.len = 0;
	}
	code = 0;
rtn:
	TLI_UNLOCK(tli);

#ifdef XTIDBG
	tr_rcvuderr (fd, uderr, code);
#endif
	return code;
}
