/* static char sccsid[] = "@(#)37  1.5  src/bos/usr/include/rpc/pmap_clnt.h, libcrpc, bos411, 9428A410j 10/25/93 20:47:26"; */
/*
 *   COMPONENT_NAME: LIBCRPC
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 24
 *
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 *	@(#)pmap_clnt.h 1.11 88/02/08 SMI
 */

/*	@(#)pmap_clnt.h	1.2 90/07/17 4.1NFSSRC SMI	*/


/*
 * pmap_clnt.h
 * Supplies C routines to get to portmap services.
 */

/*
 * Usage:
 *	success = pmap_set(program, version, protocol, port);
 *	success = pmap_unset(program, version);
 *	port = pmap_getport(address, program, version, protocol);
 *	head = pmap_getmaps(address);
 *	clnt_stat = pmap_rmtcall(address, program, version, procedure,
 *		xdrargs, argsp, xdrres, resp, tout, port_ptr)
 *		(works for udp only.) 
 * 	clnt_stat = clnt_broadcast(program, version, procedure,
 *		xdrargs, argsp,	xdrres, resp, eachresult)
 *		(like pmap_rmtcall, except the call is broadcasted to all
 *		locally connected nets.  For each valid response received,
 *		the procedure eachresult is called.  Its form is:
 *	done = eachresult(resp, raddr)
 *		bool_t done;
 *		caddr_t resp;
 *		struct sockaddr_in raddr;
 *		where resp points to the results of the call and raddr is the
 *		address if the responder to the broadcast.
 */

#ifndef _RPC_PMAP_CLNT_H
#define	_RPC_PMAP_CLNT_H

extern bool_t		pmap_set();
extern bool_t		pmap_unset();
extern struct pmaplist	*pmap_getmaps();
enum clnt_stat		pmap_rmtcall();
enum clnt_stat		clnt_broadcast();
extern u_short		pmap_getport();

#endif /*!_RPC_PMAP_CLNT_H*/
