static char sccsid[] = "@(#)11  1.5  src/bos/usr/ccs/lib/libc/svc_simple.c, libcrpc, bos411, 9428A410j 10/25/93 20:42:50";
/*
 *   COMPONENT_NAME: LIBCRPC
 *
 *   FUNCTIONS: registerrpc
 *		universal
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
static char sccsid[] = 	"@(#)svc_simple.c	1.6 90/07/19 4.1NFSSRC Copyr 1990 Sun Micro";
#endif
 *
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * 1.19 88/02/08
 */


/* 
 * svc_simple.c
 * Simplified front end to rpc.
 */

#include <stdio.h>
#include <rpc/rpc.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/syslog.h>
#include <libc_msg.h>

static struct proglst {
	char *(*p_progname)();
	int  p_prognum;
	int  p_procnum;
	xdrproc_t p_inproc, p_outproc;
	struct proglst *p_nxt;
} *proglst;
static void universal();
static SVCXPRT *transp;
struct proglst *pl;

registerrpc(prognum, versnum, procnum, progname, inproc, outproc)
	char *(*progname)();
	xdrproc_t inproc, outproc;
{
	
	if (procnum == NULLPROC) {
		(void) syslog(LOG_ERR,(char *)oncmsg(LIBCRPC,RPC34,
		    "can't reassign procedure number %d"), NULLPROC);
		return (-1);
	}
	if (transp == 0) {
		transp = svcudp_create(RPC_ANYSOCK);
		if (transp == NULL) {
			(void) syslog(LOG_ERR, (char *)oncmsg(LIBCRPC,RPC35,
				 "couldn't create an rpc server"));
			return (-1);
		}
	}
	(void) pmap_unset((u_long)prognum, (u_long)versnum);
	if (!svc_register(transp, (u_long)prognum, (u_long)versnum, 
	    universal, IPPROTO_UDP)) {
		(void) syslog(LOG_ERR, (char *)oncmsg(LIBCRPC,RPC36,
                           "couldn't register prog %d vers %d"),
			   prognum, versnum);
		return (-1);
	}
	pl = (struct proglst *)malloc(sizeof(struct proglst));
	if (pl == NULL) {
		(void) syslog(LOG_ERR, (char *)oncmsg(LIBCRPC,RPC37,
                           "registerrpc: out of memory"));
		return (-1);
	}
	pl->p_progname = progname;
	pl->p_prognum = prognum;
	pl->p_procnum = procnum;
	pl->p_inproc = inproc;
	pl->p_outproc = outproc;
	pl->p_nxt = proglst;
	proglst = pl;
	return (0);
}

static void
universal(rqstp, transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{
	int prog, proc;
	char *outdata;
	char xdrbuf[UDPMSGSIZE];
	struct proglst *pl;

	/* 
	 * enforce "procnum 0 is echo" convention
	 */
	if (rqstp->rq_proc == NULLPROC) {
		if (svc_sendreply(transp, xdr_void, (char *)NULL) == FALSE) {
			(void) syslog(LOG_ERR, (char *)oncmsg(LIBCRPC,RPC37,
                                    "svc_sendreply failed"));
		}
		return;
	}
	prog = rqstp->rq_prog;
	proc = rqstp->rq_proc;
	for (pl = proglst; pl != NULL; pl = pl->p_nxt)
		if (pl->p_prognum == prog && pl->p_procnum == proc) {
			/* decode arguments into a CLEAN buffer */
			bzero(xdrbuf, sizeof(xdrbuf)); /* required ! */
			if (!svc_getargs(transp, pl->p_inproc, xdrbuf)) {
				svcerr_decode(transp);
				return;
			}
			outdata = (*(pl->p_progname))(xdrbuf);
			if (outdata == NULL && pl->p_outproc != xdr_void)
				/* there was an error */
				return;
			if (!svc_sendreply(transp, pl->p_outproc, outdata)) {
				(void) syslog(LOG_ERR, 
					      (char *)oncmsg(LIBCRPC,RPC38,
					      "trouble replying to prog %d"),
				              pl->p_prognum);
				return;
			}
			/* free the decoded arguments */
			(void)svc_freeargs(transp, pl->p_inproc, xdrbuf);
			return;
		}
	(void) syslog(LOG_ERR, 
		      (char *)oncmsg(LIBCRPC,RPC39,"never registered prog %d"),
		      prog);
	exit(1);
}
