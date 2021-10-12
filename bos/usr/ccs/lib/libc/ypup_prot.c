static char sccsid[] = "@(#)34	1.5  src/bos/usr/ccs/lib/libc/ypup_prot.c, libcyp, bos411, 9428A410j 6/16/90 02:44:22";
/* 
 * COMPONENT_NAME: (LIBCYP) Yellow Pages Library 
 * 
 * FUNCTIONS: xdr_yp_buf, xdr_ypdelete_args, xdr_ypupdate_args 
 *
 * ORIGINS: 24 
 *
 *                  SOURCE MATERIALS
 *
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 */

#include <rpc/rpc.h>
#include <rpcsvc/ypup_prot.h>

bool_t
xdr_yp_buf(xdrs, objp)
	XDR *xdrs;
	yp_buf *objp;
{
	if (!xdr_bytes(xdrs, (char **)&objp->yp_buf_val, (u_int *)&objp->yp_buf_len, MAXYPDATALEN)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_ypupdate_args(xdrs, objp)
	XDR *xdrs;
	ypupdate_args *objp;
{
	if (!xdr_string(xdrs, &objp->mapname, MAXMAPNAMELEN)) {
		return (FALSE);
	}
	if (!xdr_yp_buf(xdrs, &objp->key)) {
		return (FALSE);
	}
	if (!xdr_yp_buf(xdrs, &objp->datum)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_ypdelete_args(xdrs, objp)
	XDR *xdrs;
	ypdelete_args *objp;
{
	if (!xdr_string(xdrs, &objp->mapname, MAXMAPNAMELEN)) {
		return (FALSE);
	}
	if (!xdr_yp_buf(xdrs, &objp->key)) {
		return (FALSE);
	}
	return (TRUE);
}


