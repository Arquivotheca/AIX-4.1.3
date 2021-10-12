static char sccsid[] = "@(#)25	1.6  src/bos/usr/ccs/lib/libc/yp_all.c, libcyp, bos411, 9428A410j 1/19/94 10:15:22";
/* 
 * COMPONENT_NAME: (LIBCYP) Yellow Pages Library
 * 
 * FUNCTIONS: defined, yp_all 
 *
 * ORIGINS: 24 
 *
 *                  SOURCE MATERIALS
 *
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * 
 */


/* #if defined(LIBC_SCCS) && !defined(lint)
 * static char sccsid[] = 	"(#)yp_all.c	1.2 88/07/27 4.0NFSSRC Copyr 1988 Sun Micro";
 * #endif
 */

#define NULL 0
#include <sys/time.h>
#include <rpc/rpc.h>
#include <rpcsvc/yp_prot.h>
#include <rpcsvc/ypv1_prot.h>
#include <rpcsvc/ypclnt.h>
#include <nl_types.h>
#include "libc_msg.h"



static struct timeval tcp_timout = {
	120,				/* 120 seconds */
	0
	};
extern int _yp_dobind();
extern unsigned int _ypsleeptime;
extern char *malloc();

/*
 * This does the "glommed enumeration" stuff.  callback->foreach is the name
 * of a function which gets called per decoded key-value pair:
 * 
 * (*callback->foreach)(status, key, keylen, val, vallen, callback->data);
 *
 * If the server we get back from _yp_dobind speaks the old protocol, this
 * returns YPERR_VERS, and does not attempt to emulate the new functionality
 * by using the old protocol.
 */
int
yp_all (domain, map, callback)
	char *domain;
	char *map;
	struct ypall_callback *callback;
{
	int domlen;
	int maplen;
	struct ypreq_nokey req;
	int reason;
	struct dom_binding *pdomb;
	enum clnt_stat s;
	nl_catd  catd;

	if ( (map == NULL) || (domain == NULL) ) {
		return(YPERR_BADARGS);
	}
	
	domlen = strlen(domain);
	maplen = strlen(map);
	
	if ( (domlen == 0) || (domlen > YPMAXDOMAIN) ||
	    (maplen == 0) || (maplen > YPMAXMAP) ||
	    (callback == (struct ypall_callback *) NULL) ) {
		return(YPERR_BADARGS);
	}

	if (reason = _yp_dobind(domain, &pdomb) ) {
		return(reason);
	}

	if (pdomb->dom_vers == YPOLDVERS) {
		return (YPERR_VERS);
	}
		
	clnt_destroy(pdomb->dom_client);
	(void) close(pdomb->dom_socket);
	pdomb->dom_socket = RPC_ANYSOCK;
	pdomb->dom_server_port = pdomb->dom_server_addr.sin_port = 0;
	
	if ((pdomb->dom_client = clnttcp_create(&(pdomb->dom_server_addr),
	    YPPROG, YPVERS, &(pdomb->dom_socket), 0, 0)) ==
	    (CLIENT *) NULL) {
		    clnt_pcreateerror("yp_all - TCP channel create failure");
		    return(YPERR_RPC);
	}

	req.domain = domain;
	req.map = map;
	
	s = clnt_call(pdomb->dom_client, YPPROC_ALL, xdr_ypreq_nokey, &req,
	    xdr_ypall, callback, tcp_timout);

	if (s != RPC_SUCCESS) {
	        catd = catopen(MF_LIBC,NL_CAT_LOCALE);
		clnt_perror(pdomb->dom_client,catgets(catd,LIBCYP,CYP1,
		    "yp_all - RPC clnt_call (TCP) failure"));
	}

	yp_unbind(domain);
	
	if (s == RPC_SUCCESS) {
		return(0);
	} else {
		return(YPERR_RPC);
	}
}

