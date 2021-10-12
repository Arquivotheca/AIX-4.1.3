static char sccsid[] = "@(#)05  1.11.1.8  src/bos/usr/ccs/lib/libc/svc.c, libcrpc, bos41J, 9511A_all 3/3/95 09:52:15";
/*
 *   COMPONENT_NAME: LIBCRPC
 *
 *   FUNCTIONS: svc_find
 *		svc_getreq
 *		svc_getreqset
 *		svc_register
 *		svc_run
 *		svc_sendreply
 *		svc_unregister
 *		svc_versquiet
 *		svcerr_auth
 *		svcerr_decode
 *		svcerr_noproc
 *		svcerr_noprog
 *		svcerr_progvers
 *		svcerr_systemerr
 *		svcerr_weakauth
 *		version_keepquiet
 *		xprt_register
 *		xprt_unregister
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
#ifndef lint
static char sccsid[] = 	"@(#)svc.c	1.5 90/07/19 4.1NFSSRC Copyr 1990 Sun Micro";
#endif
 *
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * 1.44 88/02/08 
 */


/*
 * svc.c, Server-side remote procedure call interface.
 *
 * There are two sets of procedures here.  The xprt routines are
 * for handling transport handles.  The svc routines handle the
 * list of service routines.
 */

#ifdef _KERNEL
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/time.h>
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
#include <netinet/in.h>
#include <rpc/types.h>
#include <rpc/xdr.h>
#include <rpc/auth.h>
#include <rpc/clnt.h>
#include <rpc/rpc_msg.h>
#include <rpc/svc.h>
#include <rpc/svc_auth.h>
#else
#include <sys/errno.h>
#include <rpc/rpc.h>
#include <rpc/pmap_clnt.h>

extern int errno;
static SVCXPRT **xports;
#endif

#define NULL_SVC ((struct svc_callout *)0)
#define	RQCRED_SIZE	400		/* this size is excessive */

#define	SVC_VERSQUIET 0x0001		/* keept quiet about vers mismatch */
#define	version_keepquiet(xp)  ((u_long)(xp)->xp_p3 & SVC_VERSQUIET)

/*
 * The services list
 * Each entry represents a set of procedures (an rpc program).
 * The dispatch routine takes request structs and runs the
 * apropriate procedure.
 */
static struct svc_callout {
	struct svc_callout *sc_next;
	u_long		    sc_prog;
	u_long		    sc_vers;
	void		    (*sc_dispatch)();
} *svc_head;

static struct svc_callout *svc_find();

#ifdef _KERNEL
/* Simple lock for protecting the services list
   Initialized in the krpc_config routine.
   */
Simple_lock	krpc_services_list_lock;
#define KRPC_SERVICES_LIST_LOCK()	simple_lock(&krpc_services_list_lock)
#define KRPC_SERVICES_LIST_UNLOCK()	simple_unlock(&krpc_services_list_lock)

int
init_svclock()
{
	lock_alloc(&krpc_services_list_lock, LOCK_ALLOC_PIN, 
		       KRPC_SVCS_LIST_LOCK, 0);
	simple_lock_init(&krpc_services_list_lock);
	return(0);
}

#endif

/* ***************  SVCXPRT related stuff **************** */

/*
 * Activate a transport handle.
 */
#ifdef _KERNEL
/*ARGSUSED*/
#endif
void
xprt_register(xprt)
	SVCXPRT *xprt;
{
#ifndef _KERNEL
	register int sock = xprt->xp_sock;

	if (xports == NULL) {
		xports = (SVCXPRT **)
			mem_alloc(FD_SETSIZE * sizeof(SVCXPRT *));
	}
	if (sock < _rpc_dtablesize()) {
		xports[sock] = xprt;
		FD_SET(sock, &svc_fdset);
	}
#endif
}

/*
 * De-activate a transport handle. 
 */
#ifndef _KERNEL
void
xprt_unregister(xprt) 
	SVCXPRT *xprt;
{ 
	register int sock = xprt->xp_sock;

	if ((sock < _rpc_dtablesize()) && (xports[sock] == xprt)) {
		xports[sock] = (SVCXPRT *)0;
		FD_CLR(sock, &svc_fdset);
	}
} 
#endif


/* ********************** CALLOUT list related stuff ************* */

/*
 * Add a service program to the callout list.
 * The dispatch routine will be called when a rpc request for this
 * program number comes in.
 */
#ifdef _KERNEL 
/*ARGSUSED*/ 
#endif
bool_t
svc_register(xprt, prog, vers, dispatch, protocol)
	SVCXPRT *xprt;
	u_long prog;
	u_long vers;
	void (*dispatch)();
	int protocol;
{
	struct svc_callout *prev;
	register struct svc_callout *s;

#ifdef _KERNEL
	KRPC_SERVICES_LIST_LOCK();
#endif
	if ((s = svc_find(prog, vers, &prev)) != NULL_SVC) {
		if (s->sc_dispatch == dispatch)
			goto pmap_it;  /* he is registering another xptr */
#ifdef _KERNEL
		KRPC_SERVICES_LIST_UNLOCK();
#endif
		return (FALSE);
	}
	s = (struct svc_callout *)mem_alloc(sizeof(struct svc_callout));

	if (s == (struct svc_callout *)0) {
		return (FALSE);
	}

	s->sc_prog = prog;
	s->sc_vers = vers;
	s->sc_dispatch = dispatch;
	s->sc_next = svc_head;
	svc_head = s;
pmap_it:
#ifndef _KERNEL
	/* now register the information with the local binder service */
	if (protocol) {
		return (pmap_set(prog, vers, protocol, xprt->xp_port));
	}
#endif
#ifdef _KERNEL
	KRPC_SERVICES_LIST_UNLOCK();
#endif
	return (TRUE);
}

/*
 * Remove a service program from the callout list.
 */
void
svc_unregister(prog, vers)
	u_long prog;
	u_long vers;
{
	struct svc_callout *prev;
	register struct svc_callout *s;

#ifdef _KERNEL
	KRPC_SERVICES_LIST_LOCK();
#endif
	if ((s = svc_find(prog, vers, &prev)) == NULL_SVC) {
#ifdef _KERNEL
		KRPC_SERVICES_LIST_UNLOCK();
#endif
		return;
	}
	if (prev == NULL_SVC) {
		svc_head = s->sc_next;
	} else {
		prev->sc_next = s->sc_next;
	}
	s->sc_next = NULL_SVC;
	mem_free((char *) s, (u_int) sizeof(struct svc_callout));
#ifndef _KERNEL
	/* now unregister the information with the local binder service */
	(void)pmap_unset(prog, vers);
#endif
#ifdef _KERNEL
	KRPC_SERVICES_LIST_UNLOCK();
#endif
}

/*
 * Search the callout list for a program number, return the callout
 * struct.
 */
static struct svc_callout *
svc_find(prog, vers, prev)
	u_long prog;
	u_long vers;
	struct svc_callout **prev;
{
	register struct svc_callout *s, *p;

	p = NULL_SVC;
	for (s = svc_head; s != NULL_SVC; s = s->sc_next) {
		if ((s->sc_prog == prog) && (s->sc_vers == vers))
			goto done;
		p = s;
	}
done:
	*prev = p;
	return (s);
}

/* ******************* REPLY GENERATION ROUTINES  ************ */

/*
 * Send a reply to an rpc request
 */
bool_t
svc_sendreply(xprt, xdr_results, xdr_location)
	register SVCXPRT *xprt;
	xdrproc_t xdr_results;
	caddr_t xdr_location;
{
	struct rpc_msg rply; 

	rply.rm_direction = REPLY;  
	rply.rm_reply.rp_stat = MSG_ACCEPTED; 
	rply.acpted_rply.ar_verf = xprt->xp_verf; 
	rply.acpted_rply.ar_stat = SUCCESS;
	rply.acpted_rply.ar_results.where = xdr_location;
	rply.acpted_rply.ar_results.proc = xdr_results;
	return (SVC_REPLY(xprt, &rply)); 
}

#ifdef _KERNEL
/*
 * Send a reply to an rpc request for the kernel only.
 * This takes an additional parameter of a socket that will allow
 * the lower level routines the opportunity to use this to retrieve
 * routing info....
 */
bool_t
svc_sendreply_othersock(xprt, xdr_results, xdr_location, othsocket)
	register SVCXPRT *xprt;
	xdrproc_t xdr_results;
	caddr_t xdr_location;
	struct socket *othsocket;
{
	struct rpc_msg rply; 

	rply.rm_direction = REPLY;  
	rply.rm_reply.rp_stat = MSG_ACCEPTED; 
	rply.acpted_rply.ar_verf = xprt->xp_verf; 
	rply.acpted_rply.ar_stat = SUCCESS;
	rply.acpted_rply.ar_results.where = xdr_location;
	rply.acpted_rply.ar_results.proc = xdr_results;

	/* make this call directly without use of the svc macros
	 * to avoid making changes in header files, etc.
	 */
	return (svckudp_send_othersock(xprt, &rply, othsocket)); 
}
#endif

/*
 * No procedure error reply
 */
void
svcerr_noproc(xprt)
	register SVCXPRT *xprt;
{
	struct rpc_msg rply;

	rply.rm_direction = REPLY;
	rply.rm_reply.rp_stat = MSG_ACCEPTED;
	rply.acpted_rply.ar_verf = xprt->xp_verf;
	rply.acpted_rply.ar_stat = PROC_UNAVAIL;
	SVC_REPLY(xprt, &rply);
}

/*
 * Can't decode args error reply
 */
void
svcerr_decode(xprt)
	register SVCXPRT *xprt;
{
	struct rpc_msg rply; 

	rply.rm_direction = REPLY; 
	rply.rm_reply.rp_stat = MSG_ACCEPTED; 
	rply.acpted_rply.ar_verf = xprt->xp_verf;
	rply.acpted_rply.ar_stat = GARBAGE_ARGS;
	SVC_REPLY(xprt, &rply); 
}

#ifndef _KERNEL
/*
 * Some system error
 */
void
svcerr_systemerr(xprt)
	register SVCXPRT *xprt;
{
	struct rpc_msg rply; 

	rply.rm_direction = REPLY; 
	rply.rm_reply.rp_stat = MSG_ACCEPTED; 
	rply.acpted_rply.ar_verf = xprt->xp_verf;
	rply.acpted_rply.ar_stat = SYSTEM_ERR;
	SVC_REPLY(xprt, &rply); 
}

/*
 * Tell RPC package to not complain about version errors to the client.  This
 * is useful when revving broadcast protocols that sit on a fixed address.
 * There is really one (or should be only one) example of this kind of
 * protocol: the portmapper (or rpc binder).
 */
void
svc_versquiet(xprt)
	register SVCXPRT *xprt;
{
	xprt->xp_p3 = (caddr_t)(SVC_VERSQUIET|(u_long)xprt->xp_p3);
}

#endif /* !_KERNEL*/

/*
 * Authentication error reply
 */
void
svcerr_auth(xprt, why)
	SVCXPRT *xprt;
	enum auth_stat why;
{
	struct rpc_msg rply;

	rply.rm_direction = REPLY;
	rply.rm_reply.rp_stat = MSG_DENIED;
	rply.rjcted_rply.rj_stat = AUTH_ERROR;
	rply.rjcted_rply.rj_why = why;
	SVC_REPLY(xprt, &rply);
}

/*
 * Auth too weak error reply
 */
void
svcerr_weakauth(xprt)
	SVCXPRT *xprt;
{

	svcerr_auth(xprt, AUTH_TOOWEAK);
}

/*
 * Program unavailable error reply
 */
void 
svcerr_noprog(xprt)
	register SVCXPRT *xprt;
{
	struct rpc_msg rply;  

	rply.rm_direction = REPLY;   
	rply.rm_reply.rp_stat = MSG_ACCEPTED;  
	rply.acpted_rply.ar_verf = xprt->xp_verf;  
	rply.acpted_rply.ar_stat = PROG_UNAVAIL;
	SVC_REPLY(xprt, &rply);
}

/*
 * Program version mismatch error reply
 */
void  
svcerr_progvers(xprt, low_vers, high_vers)
	register SVCXPRT *xprt; 
	u_long low_vers;
	u_long high_vers;
{
	struct rpc_msg rply;

	rply.rm_direction = REPLY;
	rply.rm_reply.rp_stat = MSG_ACCEPTED;
	rply.acpted_rply.ar_verf = xprt->xp_verf;
	rply.acpted_rply.ar_stat = PROG_MISMATCH;
	rply.acpted_rply.ar_vers.low = low_vers;
	rply.acpted_rply.ar_vers.high = high_vers;
	SVC_REPLY(xprt, &rply);
}

/* ******************* SERVER INPUT STUFF ******************* */

/*
 * Get server side input from some transport.
 *
 * Statement of authentication parameters management:
 * This function owns and manages all authentication parameters, specifically
 * the "raw" parameters (msg.rm_call.cb_cred and msg.rm_call.cb_verf) and
 * the "cooked" credentials (rqst->rq_clntcred).
 * However, this function does not know the structure of the cooked 
 * credentials, so it make the following assumptions: 
 *   a) the structure is contiguous (no pointers), and
 *   b) the cred structure size does not exceed RQCRED_SIZE bytes. 
 * In all events, all three parameters are freed upon exit from this routine.
 * The storage is trivially management on the call stack in user land, but
 * is mallocated in kernel land.
 */

#ifndef _KERNEL
void
svc_getreq(rdfds)
	int rdfds;
{
	fd_set readfds;

	FD_ZERO(&readfds);
	readfds.fds_bits[0] = rdfds;
	svc_getreqset(&readfds);
}
#endif

#ifdef _KERNEL

/* This value is the base level priority for the NFS server daemons.  If it is
 * set to 0 (zero), then priority setting is disabled for the NFS server daemons.
 * This is the default. 
 */
int nfs_server_base_priority = 0;

/* This array represent the delta of priority change that will be made for the
 * various NFS operations.  When an operation is received, the priority of the
 * NFS daemon will be set to the base priority minus the deltas of this array.
 * This allows more 'important' NFS operations to have higher priority over other
 * not-so-important NFS operations.
 */
int nfs_priority_array[] =
{
 0,	/* NULL 			*/
 1,	/* RFS_GETATTR			*/
 3,	/* RFS_SETATTR			*/
 0,	/* RFS_ROOT - not used		*/
 1,	/* RFS_LOOKUP			*/
 2,	/* RFS_READLINK			*/
 4,	/* RFS_READ			*/
 0,	/* RFS_WRITECACHE - not used	*/
 5,	/* RFS_WRITE 			*/
 3,	/* RFS_CREATE			*/ 
 3,	/* RFS_REMOVE			*/
 2,	/* RFS_RENAME			*/
 2,	/* RFS_LINK			*/
 2,	/* RFS_SYMLINK			*/
 3,	/* RFS_MKDIR			*/
 3,	/* RFS_RMDIR			*/
 4,	/* RFS_READDIR			*/
 1,	/* RFS_STATFS			*/
 0,	/* -padding-			*/
 0	/* -padding-			*/
};

#endif /* _KERNEL */

void
#ifdef _KERNEL
svc_getreq(xprt)
	register SVCXPRT *xprt;
#else
svc_getreqset(readfds)
	fd_set *readfds;
#endif
{
	enum xprt_stat stat;
	struct rpc_msg msg;
	int prog_found;
	u_long low_vers;
	u_long high_vers;
	struct svc_req r;
	char cred_area[2*MAX_AUTH_BYTES + RQCRED_SIZE];
	int	base_prio;
#ifndef _KERNEL
	register SVCXPRT *xprt;
	register u_long mask;
	register int bit;
	register u_long *maskp;
	register int setsize;
	register int sock;
#endif

	msg.rm_call.cb_cred.oa_base = cred_area;
	msg.rm_call.cb_verf.oa_base = &(cred_area[MAX_AUTH_BYTES]);
	r.rq_clntcred = &(cred_area[2*MAX_AUTH_BYTES]);

#ifndef _KERNEL

	setsize = _rpc_dtablesize();	
	maskp = (u_long *)readfds->fds_bits;
	for (sock = 0; sock < setsize; sock += NFDBITS) {
	    for (mask = *maskp++; bit = ffs(mask); mask ^= (1 << (bit - 1))) {
		/* sock has input waiting */
		xprt = xports[sock + bit - 1];
#endif
		/* now receive msgs from xprtprt (support batch calls) */
		do {
			if (SVC_RECV(xprt, &msg)) {

				/* now find the exported program and call it */
				register struct svc_callout *s;
				enum auth_stat why;

				r.rq_xprt = xprt;
				r.rq_prog = msg.rm_call.cb_prog;
				r.rq_vers = msg.rm_call.cb_vers;
				r.rq_proc = msg.rm_call.cb_proc;
				r.rq_cred = msg.rm_call.cb_cred;
				/* first authenticate the message */
				if ((why= _authenticate(&r, &msg)) != AUTH_OK) {
					svcerr_auth(xprt, why);
#ifdef _KERNEL
					/*
					 * Free the arguments.
					 */
					(void) SVC_FREEARGS(xprt, (xdrproc_t)0,
						(caddr_t)0);
#endif
					goto call_done;
				}
				/* now match message with a registered service*/
				prog_found = FALSE;
				low_vers = 0 - 1;
				high_vers = 0;
				for (s = svc_head; s != NULL_SVC; s = s->sc_next) {
					if (s->sc_prog == r.rq_prog) {
						if (s->sc_vers == r.rq_vers) {
#ifdef _KERNEL
#define NFS_PROGRAM	((u_long)100003)
if ( r.rq_prog == NFS_PROGRAM &&
    (base_prio = nfs_server_base_priority) != 0) {
	setpri(0, base_prio - nfs_priority_array[r.rq_proc]);
	(*s->sc_dispatch)(&r, xprt);
	setpri(0, base_prio);
}else{
	(*s->sc_dispatch)(&r, xprt);
}
#else /* _KERNEL */
							(*s->sc_dispatch)(&r, xprt);
#endif /* _KERNEL */
							goto call_done;
						}  /* found correct version */
						prog_found = TRUE;
						if (s->sc_vers < low_vers)
							low_vers = s->sc_vers;
						if (s->sc_vers > high_vers)
							high_vers = s->sc_vers;
					}   /* found correct program */
				}

				/*
				 * if we got here, the program or version
				 * is not served ...
				 */
				if (prog_found && !version_keepquiet(xprt))
					svcerr_progvers(xprt,
					low_vers, high_vers);
				else
					 svcerr_noprog(xprt);
#ifdef _KERNEL
				/*
				 * Free the argument storage.
				 * This is done in the dispatch routine
				 * for successful calls.
				 */
				(void) SVC_FREEARGS(xprt,(xdrproc_t)0,
					(caddr_t)0);
#endif
				/* Fall through to ... */
			}
		call_done:
			if ((stat = SVC_STAT(xprt)) == XPRT_DIED){
				SVC_DESTROY(xprt);
				break;
			}
		} while (stat == XPRT_MOREREQS);
#ifndef _KERNEL
	    }
	}
#endif
}


/*
 * This is the rpc server side idle loop
 * Wait for input, call server program.
 */
#ifdef _KERNEL

int	Rpccnt = 0;

svc_run(xprt)
	SVCXPRT *xprt;
{
	int	error;
	int	sigchk_count = 0;
	static	int	nfsdcount = 0;

	while (TRUE) {
		SOCKET_LOCK(xprt->xp_sock);
		error = 0;
		if (error = sosblock(&(xprt->xp_sock->so_rcv),
				     xprt->xp_sock)) {
			SOCKET_UNLOCK(xprt->xp_sock);
			return (error);
		}

		nfsdcount++;

		xprt->xp_sock->so_rcv.sb_flags |= SB_WAIT | SB_WAKEONE;
		xprt->xp_sock->so_rcv.sb_flags &= ~SB_NOINTR;

		/* We are asking for ku_recvfrom() to do the checksum work
		 * to avoid holding the socket lock while running through
		 * the data.
		 */
		xprt->xp_sock->so_options |= SO_CKSUMRECV;

		while (xprt->xp_sock->so_rcv.sb_cc == 0 && !error) {
			error = sosbwait(&xprt->xp_sock->so_rcv, xprt->xp_sock);
		}

		/* if we are the last nfsd that is waiting, then clear the
		 * SB_WAKEONE so that the lower level UDP/IP layers won't
		 * waste their time attempting to deliver packets.
		 */
		nfsdcount--;
		if (nfsdcount == 0)
			xprt->xp_sock->so_rcv.sb_flags &= ~SB_WAKEONE;
			
		sbunlock(&(xprt->xp_sock->so_rcv));
		SOCKET_UNLOCK(xprt->xp_sock);

		/* EINTR is the only error expected from waiting on socket
		   buffer. */
		if (error)
			return(error);

		svc_getreq(xprt);
		Rpccnt++;

		/* In the off chance that the server is so busy that it does
		 * not wait in sosbwait(), we have to check to see if a signal
		 * has been posted to the nfsd daemon.  We will only do this
		 * once every 1000 NFS requests so that it does not bother
		 * the performance of the nfsd daemon.  If we are not that
		 * busy then the nfsd will actually sleep in sosbwait()
		 */

		if (sigchk_count < 1000) {
			sigchk_count++;
		}else{
			sigchk_count = 0;
			if (sig_chk()) {
				return(EINTR);
			}
		}
	}
}
#endif
