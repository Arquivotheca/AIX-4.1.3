static char sccsid[] = "@(#)26  1.4  src/bos/usr/ccs/lib/libtli/tsudata.c, libtli, bos411, 9428A410j 3/8/94 19:14:08";
/*
 *   COMPONENT_NAME: LIBTLI
 *
 *   FUNCTIONS: t_sndudata
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
 ** tsudata.c 1.2, last change 1/29/90
 **/

#include "common.h"
#include <sys/stropts.h>
#include <sys/tihdr.h>

int
t_sndudata (fd, ud)
	int	fd;
	struct t_unitdata * ud;
{
	char			* buf;
	char			stack_buf[TLI_STACK_BUF_SIZE];
	struct strbuf		ctlbuf;
	struct strbuf		databuf;
	struct T_unitdata_req	* tudr;
	struct tli_st		* tli;
	int			total_len;
        int     		code;
        int     		ret;

        code = -1;
	if (!(tli = iostate_lookup(fd, IOSTATE_VERIFY)))
		return code;

	if ( tli->tlis_servtype != T_CLTS ) {
		t_errno = TNOTSUPPORT;
		goto rtn;
	}

	if ( tli->tlis_state != T_IDLE ) {
		int event = TLI_ILOOK(fd, tli);

		if (event) {
			t_errno = TLOOK;
			goto rtn;
		} else {
			t_errno = TOUTSTATE;
			goto rtn;
		}
	}

	if ( ud->udata.len == 0 ) {
		t_errno = TBADDATA;
		goto rtn;
	}

	/* NOTE: this check may need to be against tsdu for XTI */
	if ( ud->udata.len > tli->tlis_tsdu_size ) {
#ifdef XTI
		t_errno = TBADDATA;
#else
		t_errno = TSYSERR;
		errno = EPROTO;
#endif
		goto rtn;
	}
	total_len = (ud->addr.len >= 0 ? ud->addr.len : 0);
	total_len += (ud->opt.len >= 0 ? ud->opt.len : 0);
	total_len += sizeof(struct T_unitdata_req);
	if (total_len > (int)sizeof(stack_buf)) {
		buf = (char *)malloc(total_len);
		if (!buf) {
			t_errno = TSYSERR;
			errno = ENOMEM;
			goto rtn;
		}
	} else
		buf = stack_buf;

	tudr = (struct T_unitdata_req *)&buf[0];
	tudr->PRIM_type = T_UNITDATA_REQ;
	if (ud->addr.len > 0) {
		tudr->DEST_length = ud->addr.len;
		tudr->DEST_offset = sizeof(struct T_unitdata_req);
		memcpy(&buf[tudr->DEST_offset], ud->addr.buf, tudr->DEST_length);
	} else {
		tudr->DEST_length = 0;
		tudr->DEST_offset = 0;
	}
	if (ud->opt.len > 0) {
		tudr->OPT_length = ud->opt.len;
		tudr->OPT_offset = tudr->DEST_offset + tudr->DEST_length;
		memcpy(&buf[tudr->OPT_offset], ud->opt.buf, tudr->OPT_length);
	} else {
		tudr->OPT_length = 0;
		tudr->OPT_offset = 0;
	}
	ctlbuf.buf = (char *)tudr;
	ctlbuf.len = total_len;
	databuf.buf = ud->udata.buf;
	databuf.len = ud->udata.len;
	ret = putmsg(fd, &ctlbuf, &databuf, 0);

	if (buf != stack_buf)
		free(buf);

	if (ret == -1) {
		if ( errno == ERANGE ) {
			/* Secret message from TIMOD!  T_UDERR waiting. */
			TLI_TSYNC(tli, fd);
			t_errno = TLOOK;
			goto rtn;
		}
		t_unix_to_tli_error();
		if ( t_errno == TNODATA ) {
#ifdef XTI
			tli->tlis_flags |= TLIS_DATA_STOPPED;
#endif
			t_errno = TFLOW;
			goto rtn;
		}
		goto rtn;
	}

#ifdef XTI
	tli->tlis_flags &= ~TLIS_DATA_STOPPED;
#endif
	code = 0;

rtn:
	TLI_UNLOCK(tli);

#ifdef XTIDBG
	tr_sndudata (fd, ud, code); 
#endif
	return code;
}

