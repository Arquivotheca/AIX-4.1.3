/* static char sccsid[] = "@(#)69  1.5  src/bos/usr/include/rpc/pmap_rmt.h, libcrpc, bos411, 9428A410j 10/25/93 20:47:46"; */
/*
 *   COMPONENT_NAME: LIBCRPC
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 24
 *
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 *	1.2 88/02/08 SMI	
 */

/*	@(#)pmap_rmt.h	1.2 90/07/17 4.1NFSSRC SMI	*/


/*
 * Structures and XDR routines for parameters to and replies from
 * the portmapper remote-call-service.
 */

#ifndef _RPC_PMAP_RMT_H
#define	_RPC_PMAP_RMT_H

struct rmtcallargs {
	u_long prog, vers, proc, arglen;
	caddr_t args_ptr;
	xdrproc_t xdr_args;
};

bool_t xdr_rmtcall_args();

struct rmtcallres {
	u_long *port_ptr;
	u_long resultslen;
	caddr_t results_ptr;
	xdrproc_t xdr_results;
};

bool_t xdr_rmtcallres();

#endif /*!_RPC_PMAP_RMT_H*/
