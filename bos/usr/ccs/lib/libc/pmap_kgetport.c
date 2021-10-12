static char sccsid[] = "@(#)16  1.2  src/bos/usr/ccs/lib/libc/pmap_kgetport.c, libcrpc, bos411, 9428A410j 2/24/94 17:47:54";
/*
 *   COMPONENT_NAME: LIBCRPC
 *
 *   FUNCTIONS: getport_loop
 *		pmap_kgetport
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
static char sccsid[] = 	"@(#)pmap_kgetport.c	1.6 90/07/19 4.1NFSSRC Copyr 1990 Sun Micro";
#endif
 *
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * 1.10 88/02/08 
 */

#ifdef _KERNEL

/*
 * pmap_kgetport.c
 * Kernel interface to pmap rpc service.
 */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/uprintf.h>
#include <cmdnfs_msg.h>
#include <rpc/types.h>
#include <netinet/in.h>
#include <rpc/xdr.h>
#include <rpc/auth.h>
#include <rpc/clnt.h>
#include <rpc/rpc_msg.h>
#include <rpc/pmap_prot.h>

#include <sys/socket.h>
#include <net/if.h>

static struct ucred cred;
#define retries 4
static struct timeval tottimeout = { 1, 0 };


/*
 * Find the mapped port for program, version.
 * Calls the pmap service remotely to do the lookup.
 *
 * The 'address' argument is used to locate the portmapper, then
 * modified to contain the port number, if one was found.  If no
 * port number was found, 'address'->sin_port returns unchanged.
 *
 * Returns:	 0  if port number successfully found for 'program'
 *		-1  (<0) if 'program' was not registered
 *		 1  (>0) if there was an error contacting the portmapper
 */
int
pmap_kgetport(address, program, version, protocol)
	struct sockaddr_in *address;
	u_long program;
	u_long version;
	u_long protocol;
{
	u_short port = 0;
	register CLIENT *client;
	struct pmap parms;
	int error = 0;
	struct sockaddr_in tmpaddr;
	 
	/* ensure that the cred struct is initialized correctly */
	cred.cr_uid = (uid_t)0;
	cred.cr_gid = (gid_t)0;
	cred.cr_ngrps = 1;
	cred.cr_groups[0] = (gid_t)0;

	/* copy 'address' so that it doesn't get trashed */
	tmpaddr = *address;

	tmpaddr.sin_port = htons(PMAPPORT);
	client = clntkudp_create(&tmpaddr, PMAPPROG, PMAPVERS, retries, &cred);

	if (client != (CLIENT *)NULL) {
		/* for use by clntkudp_callit */
		client->cl_auth->ah_private = (caddr_t)&cred;

		parms.pm_prog = program;
		parms.pm_vers = version;
		parms.pm_prot = protocol;
		parms.pm_port = 0;  /* not needed or used */
		if (CLNT_CALL(client, PMAPPROC_GETPORT, xdr_pmap, &parms,
		    xdr_u_short, &port, tottimeout) != RPC_SUCCESS){
			error = 1;	/* error contacting portmapper */
		} else if (port == 0) {
			error = -1;	/* program not registered */
		} else {
			address->sin_port = htons(port);	/* save the port # */
		}
		client->cl_auth->ah_private = (caddr_t)NULL;
		AUTH_DESTROY(client->cl_auth);
		CLNT_DESTROY(client);
	}else{
		error = -1;	/* Can't even try... */
	}

	return (error);
}

/*
 * getport_loop -- kernel interface to pmap_kgetport()
 *
 * Talks to the portmapper using the sockaddr_in supplied by 'address',
 * to lookup the specified 'program'.
 *
 * Modifies 'address'->sin_port by rewriting the port number, if one
 * was found.  If a port number was not found (ie, return value != 0),
 * then 'address'->sin_port is left unchanged.
 *
 * If the portmapper does not respond, prints console message (once).
 * Retries forever, unless a signal is received.
 *
 * Returns:	 0  the port number was successfully put into 'address'
 *		-1  (<0) the requested process is not registered.
 *		 1  (>0) the portmapper did not respond and a signal occurred.
 */
getport_loop(address, program, version, protocol)
	struct sockaddr_in *address;
	u_long program;
	u_long version;
	u_long protocol;
{
	struct	uprintf	up;
	register int pe = 0;
	register int i = 0;

	/* sit in a tight loop until the portmapper responds */
	while ((i = pmap_kgetport(address, program, version, protocol)) > 0) {

		/* test to see if a signal has come in */
		if (sig_chk()) {
			up.upf_defmsg = 
				"Portmapper not responding: giving up\n";
			up.upf_NLcatname = MF_CMDNFS;
			up.upf_NLsetno = KRPCXDR;
			up.upf_NLmsgno = KRPCXDR01;
			NLuprintf(&up);
			goto out;		/* got a signal */
		}
		/* print this message only once */
		if (pe++ == 0) {
			up.upf_defmsg = 
				"Portmapper not responding: still trying\n";
			up.upf_NLcatname = MF_CMDNFS;
			up.upf_NLsetno = KRPCXDR;
			up.upf_NLmsgno = KRPCXDR02;
			NLuprintf(&up);
		}
	}				/* go try the portmapper again */

	/* got a response...print message if there was a delay */
	if (pe != 0) {
		up.upf_defmsg = 
			"Portmapper ok\n";
		up.upf_NLcatname = MF_CMDNFS;
		up.upf_NLsetno = KRPCXDR;
		up.upf_NLmsgno = KRPCXDR03;
		NLuprintf(&up);
	}
out:
	return(i);	/* may return <0 if program not registered */
}
#endif /* _KERNEL */
