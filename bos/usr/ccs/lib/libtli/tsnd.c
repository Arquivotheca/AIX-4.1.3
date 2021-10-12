static char sccsid[] = "@(#)23  1.1.1.3  src/bos/usr/ccs/lib/libtli/tsnd.c, libtli, bos411, 9428A410j 3/8/94 19:13:49";
/*
 *   COMPONENT_NAME: LIBTLI
 *
 *   FUNCTIONS: t_snd
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
 ** tsnd.c 1.2, last change 1/29/90
 **/

#include "common.h"
#include <sys/stropts.h>
#include <sys/tihdr.h>

int
t_snd (fd, buf, nbytes, flags)
	int	fd;
	char	* buf;
	unsigned nbytes;
	int	flags;
{
	struct strbuf		ctlbuf;
	struct strbuf		databuf;
	struct T_data_req	tdr;
	struct tli_st		* tli;
        int			code;
	long			count;
	long			tsdu;	/* tsdu/etsdu: max total message size */
	long			tidu;	/* tidu: max single T_DATA_REQ size */
	int			event;

        code = -1;
	if (!(tli = iostate_lookup(fd, IOSTATE_VERIFY)))
		return code;

	if ( tli->tlis_servtype == T_CLTS ) {
		t_errno = TNOTSUPPORT;
		goto rtn;
	}

	if ( tli->tlis_state != T_DATAXFER  &&  tli->tlis_state != T_INREL ) {
		event = TLI_LOOK(fd, tli);
		if (event && event != T_ORDREL) 
			t_errno = TLOOK;
		else 
			t_errno = TOUTSTATE;
		goto rtn;
	}

	if (flags & ~(T_MORE | T_EXPEDITED)) {
		t_errno = TBADFLAG;
		goto rtn;
	}
	if (nbytes == 0) {
		t_errno = TBADDATA;
		goto rtn;
	}

	/*
	 * tsdu == -1 means no limit;
	 * tsdu ==  0 means no boundries, no limit;
	 * tsdu >   0 means limit of tsdu bytes;
	 * tidu == max message size in T_DATA_REQ (see TPI v1.5, 92/12/10)
	 */

	if (flags & T_EXPEDITED) {
		tdr.PRIM_type = T_EXDATA_REQ;
		tsdu = tli->tlis_etsdu_size;
	} else {
		tdr.PRIM_type = T_DATA_REQ;
		tsdu = tli->tlis_tsdu_size;
	}

	if (tsdu == -2 || (tsdu > 0 && nbytes > tsdu)) {
#ifdef XTI
		t_errno = TBADDATA;
#else
		t_errno = TSYSERR;
		errno = EPROTO;
#endif
		goto rtn;
	}
	tidu = tli->tlis_tidu_size;

	ctlbuf.buf = (char *)&tdr;
	ctlbuf.len = sizeof(tdr);
	databuf.buf = buf;
	for (count = 0; nbytes > 0; ) {
		int len;
		if (tidu > 0 && nbytes > tidu) {
			tdr.MORE_flag |= T_MORE;
			len = databuf.len = tidu;
		} else {
			tdr.MORE_flag = (flags & T_MORE);
			len = databuf.len = nbytes;
		}

		if (putmsg(fd, &ctlbuf, &databuf, 0) == -1) {
			if ( errno == ERANGE ) {
				/* Secret message from TIMOD!
				** T_DISCONNECT or T_ORDREL waiting
				*/
				TLI_TSYNC(tli, fd);
				t_errno = TLOOK;
				goto rtn;
			}
			t_unix_to_tli_error();
			if ( t_errno == TNODATA ) {
				if (databuf.buf != buf) {
					/* we've sent some data */
					code = count;
					goto rtn;
				}
				t_errno = TFLOW;
#ifdef XTI
				if (flags & T_EXPEDITED)
					tli->tlis_flags |= TLIS_EXDATA_STOPPED;
				else
					tli->tlis_flags |= TLIS_DATA_STOPPED;
#endif
			}
			goto rtn;
		}
		databuf.buf += len;
		count += len;
		nbytes -= len;
	} 
#ifdef XTI
	if (flags & T_EXPEDITED)
		tli->tlis_flags &= ~TLIS_EXDATA_STOPPED;
	else
		tli->tlis_flags &= ~TLIS_DATA_STOPPED;
#endif

	code = count;
	TLI_NEXTSTATE(tli, TLI_SND);
rtn:
	TLI_UNLOCK(tli);

#ifdef XTIDBG
	tr_snd (fd, buf, nbytes, flags, code);
#endif
	return code;
}
