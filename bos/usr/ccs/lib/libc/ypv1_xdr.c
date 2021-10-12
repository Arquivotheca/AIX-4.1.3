static char sccsid[] = "@(#)35	1.4  src/bos/usr/ccs/lib/libc/ypv1_xdr.c, libcyp, bos411, 9428A410j 6/16/90 02:44:29";
/* 
 * COMPONENT_NAME: (LIBCYP) Yellow Pages Library
 * 
 * FUNCTIONS: _xdr_ypbind_oldsetdom, _xdr_yprequest, _xdr_ypresponse 
 *
 * ORIGINS: 24 
 *
 *                  SOURCE MATERIALS
 *
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * 1.5 87/08/12
 * 
 */


/* #if defined(LIBC_SCCS) && !defined(lint)
 * static char sccsid[] = 	"(#)ypv1_xdr.c	1.2 88/07/27 4.0NFSSRC Copyr 1988 Sun Micro";
 * #endif
 */



/*
 * This contains the xdr functions needed by ypserv and the yp
 * administrative tools to support the previous version of the yp protocol.
 * Note that many "old" xdr functions are called, with the assumption that
 * they have not changed between the v1 protocol (which this module exists
 * to support) and the current v2 protocol.  
 */

#define NULL 0
#include <rpc/rpc.h>
#include <rpcsvc/yp_prot.h>
#include <rpcsvc/ypv1_prot.h>
#include <rpcsvc/ypclnt.h>
typedef struct xdr_discrim XDR_DISCRIM;

extern bool xdr_ypreq_key();
extern bool xdr_ypreq_nokey();
extern bool xdr_ypresp_val();
extern bool xdr_ypresp_key_val();
extern bool xdr_ypmap_parms();

/*
 * xdr discriminant/xdr_routine vector for yp requests.
 */
XDR_DISCRIM _yprequest_arms[] = {
	{(int) YPREQ_KEY, (xdrproc_t) xdr_ypreq_key},
	{(int) YPREQ_NOKEY, (xdrproc_t) xdr_ypreq_nokey},
	{(int) YPREQ_MAP_PARMS, (xdrproc_t) xdr_ypmap_parms},
	{__dontcare__, (xdrproc_t) NULL}
};

/*
 * Serializes/deserializes a yprequest structure.
 */
bool
_xdr_yprequest (xdrs, ps)
	XDR * xdrs;
	struct yprequest *ps;

{
	return(xdr_union(xdrs, &ps->yp_reqtype, &ps->yp_reqbody,
	    _yprequest_arms, NULL) );
}


/*
 * xdr discriminant/xdr_routine vector for yp responses
 */
XDR_DISCRIM _ypresponse_arms[] = {
	{(int) YPRESP_VAL, (xdrproc_t) xdr_ypresp_val},
	{(int) YPRESP_KEY_VAL, (xdrproc_t) xdr_ypresp_key_val},
	{(int) YPRESP_MAP_PARMS, (xdrproc_t) xdr_ypmap_parms},
	{__dontcare__, (xdrproc_t) NULL}
};

/*
 * Serializes/deserializes a ypresponse structure.
 */
bool
_xdr_ypresponse (xdrs, ps)
	XDR * xdrs;
	struct ypresponse *ps;

{
	return(xdr_union(xdrs, &ps->yp_resptype, &ps->yp_respbody,
	    _ypresponse_arms, NULL) );
}

/*
 * Serializes/deserializes a ypbind_oldsetdom structure.
 */
bool
_xdr_ypbind_oldsetdom(xdrs, ps)
	XDR *xdrs;
	struct ypbind_setdom *ps;
{
	char *domain = ps->ypsetdom_domain;
	
	return(xdr_ypdomain_wrap_string(xdrs, &domain) &&
	    xdr_yp_binding(xdrs, &ps->ypsetdom_binding) );
}
