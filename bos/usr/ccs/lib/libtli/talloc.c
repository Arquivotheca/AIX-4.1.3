static char sccsid[] = "@(#)99  1.3  src/bos/usr/ccs/lib/libtli/talloc.c, libtli, bos411, 9428A410j 12/20/93 17:10:25";
/*
 *   COMPONENT_NAME: LIBTLI
 *
 *   FUNCTIONS: netbuf_alloc, t_alloc
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
 ** talloc.c 1.4, last change 12/20/89
 **/

#include "common.h"

extern	void	* calloc(   int elem_cnt, int elem_size   );

static int
netbuf_alloc (nb, len)
	struct netbuf	* nb;
	long		len;
{
reg	uint ulen = (uint)len;

	if (len < 0L) {
		nb->buf = nilp(char);
		nb->len = 0;
		nb->maxlen = 0;
		errno = EINVAL;
		return 1;
	}
	if ((long)ulen != len) {
		errno = EINVAL;
		return 0;
	}
	if (ulen > 0 && !(nb->buf = (char *)calloc(ulen, (uint)sizeof(char)))) {
		errno = ENOMEM;
		return 0;
	}
	nb->len = 0;
	nb->maxlen = ulen;
	return 1;
}

char *
t_alloc (fd, struct_type, fields)
	int	fd;
	int	struct_type;
	int	fields;
{
	struct netbuf	* addrp = nilp(struct netbuf);
	struct netbuf	* datap = nilp(struct netbuf);
	struct netbuf	* optp = nilp(struct netbuf);
	struct t_info	tinfo;
	struct tli_st	* tli;
	union {
		struct t_bind	* t_b;
		struct t_call	* t_c;
		struct t_info	* t_i;
		struct t_optmgmt * t_o;
		struct t_discon	* t_d;
		struct t_unitdata * t_u;
		struct t_uderr	* t_ud;
	} up;
	char *  code = nilp(char);
	long		len;

	if (!(tli = iostate_lookup(fd, IOSTATE_VERIFY)))
		return code;

	up.t_b = nilp(struct t_bind);
	switch (struct_type) {
	case T_BIND:
		up.t_b = newa(struct t_bind, 1);
		addrp = &up.t_b->addr;
		break;
	case T_CALL:
		up.t_c = newa(struct t_call, 1);
		datap = &up.t_c->udata;
		optp = &up.t_c->opt;
		addrp = &up.t_c->addr;
		break;
	case T_DIS:
		up.t_d = newa(struct t_discon, 1);
		datap = &up.t_d->udata;
		break;
	case T_INFO:
		up.t_i = newa(struct t_info, 1);
		break;
	case T_OPTMGMT:
		up.t_o = newa(struct t_optmgmt, 1);
		optp = &up.t_o->opt;
		break;
	case T_UDERROR:
		up.t_ud = newa(struct t_uderr, 1);
		optp = &up.t_ud->opt;
		addrp = &up.t_ud->addr;
		break;
	case T_UNITDATA:
		up.t_u = newa(struct t_unitdata, 1);
		datap = &up.t_u->udata;
		optp = &up.t_u->opt;
		addrp = &up.t_u->addr;
		break;
	default:
		t_errno = TNOSTRUCTYPE;
		code =  nilp(char);
		goto rtn;
	}
	if (!up.t_b) {
		errno = ENOMEM;
		t_errno = TSYSERR;
		goto rtn;
	}
	if (!(fields & (T_ALL | T_ADDR | T_OPT | T_UDATA))) {
		code =  (char *)up.t_b;
		goto rtn;
	}

	if (t_getinfo(fd, &tinfo) != 0) {
		(void)t_free((char *)up.t_b, struct_type);
		code =  nilp(char);
		goto rtn;
	}

	if (!(fields & T_ADDR))
		addrp = 0;
	if (!(fields & T_OPT))
		optp = 0;
	if (!(fields & T_UDATA))
		datap = 0;

	if (((fields & T_ALL) || (fields & T_ADDR))  &&  addrp) {
		if (!netbuf_alloc(addrp, tinfo.addr))
			goto alloc_err;
	}
	if (((fields & T_ALL) || (fields & T_OPT))  &&  optp) {
		if (!netbuf_alloc(optp, tinfo.options))
			goto alloc_err;
	}
	if (((fields & T_ALL) || (fields & T_UDATA))  &&  datap) {
		switch (struct_type) {
		case T_UNITDATA:
			len = tinfo.tsdu;
			break;
		case T_CALL:
			len = tinfo.connect;
			break;
		case T_DIS:
			len = tinfo.discon;
			break;
		default:
			len = -1;
			break;
		}
		if (!netbuf_alloc(datap, len))
			goto alloc_err;
	}
	code = (char *)up.t_b;
	goto rtn;
alloc_err:
	(void)t_free((char *)up.t_b, struct_type);
	t_errno = TSYSERR;
	code = nilp(char);
rtn:
	TLI_UNLOCK(tli);
#ifdef XTIDBG
        tr_allocate (fd, struct_type, fields, code);
#endif
	return code;

}
