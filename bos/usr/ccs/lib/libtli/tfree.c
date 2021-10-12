static char sccsid[] = "@(#)08  1.3  src/bos/usr/ccs/lib/libtli/tfree.c, libtli, bos411, 9428A410j 12/20/93 17:20:23";
/*
 *   COMPONENT_NAME: LIBTLI
 *
 *   FUNCTIONS: t_free
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

#include "common.h"

#ifdef	_NO_PROTO
extern	void	free();
#else
extern	void	free(char * ptr);
#endif

int
t_free (ptr, struct_type)
	char	* ptr;
	int	struct_type;
{
reg	struct netbuf	* addrp = nilp(struct netbuf);
	struct netbuf	* optp = nilp(struct netbuf);
	struct netbuf	* datap = nilp(struct netbuf);
	union {
		struct t_bind	* t_b;
		struct t_call	* t_c;
		struct t_info	* t_i;
		struct t_optmgmt * t_o;
		struct t_discon	* t_d;
		struct t_unitdata * t_u;
		struct t_uderr	* t_ud;
	} up;
        int    code;

        code = -1;
	if (!(up.t_b = (struct t_bind *)ptr)) {
	        code = 0;
		goto rtn;
	}
	switch (struct_type) {
	case T_BIND:
		addrp = &up.t_b->addr;
		break;
	case T_CALL:
		datap = &up.t_c->udata;
		optp = &up.t_c->opt;
		addrp = &up.t_c->addr;
		break;
	case T_OPTMGMT:
		optp = &up.t_o->opt;
		break;
	case T_DIS:
		datap = &up.t_d->udata;
		break;
	case T_INFO:
		break;
	case T_UNITDATA:
		datap = &up.t_u->udata;
		optp = &up.t_u->opt;
		addrp = &up.t_u->addr;
		break;
	case T_UDERROR:
		optp = &up.t_ud->opt;
		addrp = &up.t_ud->addr;
		break;
	default:
		t_errno = TNOSTRUCTYPE;
		goto rtn;
	}
	if (datap  &&  datap->buf)
		free(datap->buf);
	if (optp  &&  optp->buf)
		free(optp->buf);
	if (addrp  &&  addrp->buf)
		free(addrp->buf);
	free((char *)up.t_b);
        code = 0;
rtn:

#ifdef XTIDBG
	tr_free (struct_type, code);
#endif
	return code;
}
