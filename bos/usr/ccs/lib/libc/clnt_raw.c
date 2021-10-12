static char sccsid[] = "@(#)83  1.7  src/bos/usr/ccs/lib/libc/clnt_raw.c, libcrpc, bos411, 9428A410j 10/25/93 20:40:05";
/*
 *   COMPONENT_NAME: LIBCRPC
 *
 *   FUNCTIONS: clntraw_abort
 *		clntraw_call
 *		clntraw_control
 *		clntraw_create
 *		clntraw_destroy
 *		clntraw_freeres
 *		clntraw_geterr
 *		
 *
 *   ORIGINS: 24,27
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/* 
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = 	"@(#)clnt_raw.c	1.7 90/07/19 4.1NFSSRC Copyr 1990 Sun Micro";
#endif
 *
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * 1.23 88/02/08 
 */


/*
 * clnt_raw.c
 *
 * Memory based rpc for simple testing and timing.
 * Interface to create an rpc client and server in the same process.
 * This lets us similate rpc and get round trip overhead, without
 * any interference from the kernal.
 */

#include <libc_msg.h>
#include <rpc/rpc.h>
#include <rpc/raw.h>
#include <sys/syslog.h>

#define MCALL_MSG_SIZE 24

/*
 * This is the "network" we will be moving stuff over.
 */
static struct clntraw_private {
	CLIENT	client_object;
	XDR	xdr_stream;
	char	*raw_buf;
	char	mashl_callmsg[MCALL_MSG_SIZE];
	u_int	mcnt;
} *clntraw_private;

extern char *_rpcrawcombuf;
static enum clnt_stat	clntraw_call();
static void		clntraw_abort();
static void		clntraw_geterr();
static bool_t		clntraw_freeres();
static bool_t		clntraw_control();
static void		clntraw_destroy();

static struct clnt_ops client_ops = {
	clntraw_call,
	clntraw_abort,
	clntraw_geterr,
	clntraw_freeres,
	clntraw_destroy,
	clntraw_control
};

void	svc_getreq();

/*
 * Create a client handle for memory based rpc.
 */
CLIENT *
clntraw_create(prog, vers)
	u_long prog;
	u_long vers;
{
	register struct clntraw_private *clp = clntraw_private;
	struct rpc_msg call_msg;
	XDR *xdrs;
	CLIENT  *client;

	if (clp == 0) {
		clp = (struct clntraw_private *)calloc(1, sizeof (*clp));
		if (clp == NULL)
			return (NULL);
		clntraw_private = clp;
		if (_rpcrawcombuf == NULL) {
			_rpcrawcombuf = (char *)calloc(UDPMSGSIZE,
							sizeof (char));
			if (_rpcrawcombuf == NULL)
				return (NULL);
		}
		clp->raw_buf = _rpcrawcombuf; /* Share it with the server */
	}
	xdrs = &clp->xdr_stream;
	client = &clp->client_object;

	/*
	 * pre-serialize the staic part of the call msg and stash it away
	 */
	call_msg.rm_direction = CALL;
	call_msg.rm_call.cb_rpcvers = RPC_MSG_VERSION;
	call_msg.rm_call.cb_prog = prog;
	call_msg.rm_call.cb_vers = vers;
	xdrmem_create(xdrs, clp->mashl_callmsg, MCALL_MSG_SIZE, XDR_ENCODE); 
	if (! xdr_callhdr(xdrs, &call_msg)) {
		(void) syslog(LOG_ERR, oncmsg(LIBCRPC,RPC53,
			"clnt_raw.c - Fatal header serialization error."));
		return((CLIENT *)NULL);
	}
	clp->mcnt = XDR_GETPOS(xdrs);
	XDR_DESTROY(xdrs);

	/*
	 * Set xdrmem for client/server shared buffer
	 */
	xdrmem_create(xdrs, clp->raw_buf, UDPMSGSIZE, XDR_FREE);

	/*
	 * create client handle
	 */
	client->cl_ops = &client_ops;
	client->cl_auth = authnone_create();
	return (client);
}

static enum clnt_stat 
clntraw_call(h, proc, xargs, argsp, xresults, resultsp, timeout)
	CLIENT *h;
	u_long proc;
	xdrproc_t xargs;
	caddr_t argsp;
	xdrproc_t xresults;
	caddr_t resultsp;
	struct timeval timeout;
{
	register struct clntraw_private *clp = clntraw_private;
	register XDR *xdrs = &clp->xdr_stream;
	struct rpc_msg msg;
	enum clnt_stat status;
	struct rpc_err error;

	if (clp == 0)
		return (RPC_FAILED);
call_again:
	/*
	 * send request
	 */
	xdrs->x_op = XDR_ENCODE;
	XDR_SETPOS(xdrs, 0);
	((struct rpc_msg *)clp->mashl_callmsg)->rm_xid ++ ;
	if ((! XDR_PUTBYTES(xdrs, clp->mashl_callmsg, clp->mcnt)) ||
	    (! XDR_PUTLONG(xdrs, (long *)&proc)) ||
	    (! AUTH_MARSHALL(h->cl_auth, xdrs)) ||
	    (! (*xargs)(xdrs, argsp))) {
		return (RPC_CANTENCODEARGS);
	}
	(void)XDR_GETPOS(xdrs);  /* called just to cause overhead */

	/*
	 * We have to call server input routine here because this is
	 * all going on in one process. Yuk.
	 */
	svc_getreq(1);

	/*
	 * get results
	 */
	xdrs->x_op = XDR_DECODE;
	XDR_SETPOS(xdrs, 0);
	msg.acpted_rply.ar_verf = _null_auth;
	msg.acpted_rply.ar_results.where = resultsp;
	msg.acpted_rply.ar_results.proc = xresults;
	if (! xdr_replymsg(xdrs, &msg))
		return (RPC_CANTDECODERES);
	_seterr_reply(&msg, &error);
	status = error.re_status;

	if (status == RPC_SUCCESS) {
		if (! AUTH_VALIDATE(h->cl_auth, &msg.acpted_rply.ar_verf)) {
			status = RPC_AUTHERROR;
		}
	}  /* end successful completion */
	else {
		if (AUTH_REFRESH(h->cl_auth))
			goto call_again;
	}  /* end of unsuccessful completion */

	if (status == RPC_SUCCESS) {
		if (! AUTH_VALIDATE(h->cl_auth, &msg.acpted_rply.ar_verf)) {
			status = RPC_AUTHERROR;
		}
		if (msg.acpted_rply.ar_verf.oa_base != NULL) {
			xdrs->x_op = XDR_FREE;
			(void)xdr_opaque_auth(xdrs, &(msg.acpted_rply.ar_verf));
		}
	}

	return (status);
}

static void
clntraw_geterr()
{
}


static bool_t
clntraw_freeres(cl, xdr_res, res_ptr)
	CLIENT *cl;
	xdrproc_t xdr_res;
	caddr_t res_ptr;
{
	register struct clntraw_private *clp = clntraw_private;
	register XDR *xdrs = &clp->xdr_stream;

	if (clp == 0)
		return (FALSE);
	xdrs->x_op = XDR_FREE;
	return ((*xdr_res)(xdrs, res_ptr));
}

static void
clntraw_abort()
{
}

static bool_t
clntraw_control()
{
	return (FALSE);
}

static void
clntraw_destroy()
{
}
