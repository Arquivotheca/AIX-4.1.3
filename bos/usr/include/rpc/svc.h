/* static char sccsid[] = "@(#)40  1.17  src/bos/usr/include/rpc/svc.h, libcrpc, bos411, 9428A410j 5/13/94 17:15:27"; */
/*
 *   COMPONENT_NAME: LIBCRPC
 *
 *   FUNCTIONS: SVC_DESTROY
 *		SVC_FREEARGS
 *		SVC_GETARGS
 *		SVC_RECV
 *		SVC_REPLY
 *		SVC_STAT
 *		svc_destroy
 *		svc_freeargs
 *		svc_getargs
 *		svc_getcaller
 *		svc_recv
 *		svc_reply
 *		svc_stat
 *		
 *
 *   ORIGINS: 24
 *
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 *      @(#)svc.h 1.20 88/02/08 SMI      *
 */

/*	@(#)svc.h	1.3 90/07/19 4.1NFSSRC SMI	*/

/*
 * svc.h, Server-side remote procedure call interface.
 */

#ifndef _RPC_SVC_H
#define	_RPC_SVC_H
#include <sys/types.h>
#include <sys/select.h>
#include <rpc/xdr.h>

/*
 * This interface must manage two items concerning remote procedure calling:
 *
 * 1) An arbitrary number of transport connections upon which rpc requests
 * are received.  The two most notable transports are TCP and UDP;  they are
 * created and registered by routines in svc_tcp.c and svc_udp.c, respectively;
 * they in turn call xprt_register and xprt_unregister.
 *
 * 2) An arbitrary number of locally registered services.  Services are
 * described by the following four data: program number, version number,
 * "service dispatch" function, a transport handle, and a boolean that
 * indicates whether or not the exported program should be registered with a
 * local binder service;  if true the program's number and version and the
 * port number from the transport handle are registered with the binder.
 * These data are registered with the rpc svc system via svc_register.
 *
 * A service's dispatch function is called whenever an rpc request comes in
 * on a transport.  The request's program and version numbers must match
 * those of the registered service.  The dispatch function is passed two
 * parameters, struct svc_req * and SVCXPRT *, defined below.
 */

enum xprt_stat {
	XPRT_DIED,
	XPRT_MOREREQS,
	XPRT_IDLE
};

/*
 * Server side transport handle
 */
typedef struct SVCXPRT {
#ifdef _KERNEL
	struct socket	*xp_sock;
#else
	int		xp_sock;
#endif /* _KERNEL */
#undef xp_stat

	u_short		xp_port;	 /* associated port number */
	struct xp_ops {
#ifdef _NO_PROTO
	    bool_t	(*xp_recv)();	 /* receive incomming requests */
	    enum xprt_stat (*xp_stat)(); /* get transport status */
	    bool_t	(*xp_getargs)(); /* get arguments */
	    bool_t	(*xp_reply)();	 /* send reply */
	    bool_t	(*xp_freeargs)();/* free mem allocated for args */
	    void	(*xp_destroy)(); /* destroy this struct */
#else /* _NO_PROTO */
	    bool_t	(*xp_recv)(struct SVCXPRT *, struct rpc_msg *);
	    enum xprt_stat (*xp_stat)(struct SVCXPRT *);
	    bool_t	(*xp_getargs)(struct SVCXPRT *, xdrproc_t, caddr_t);
	    bool_t	(*xp_reply)(struct SVCXPRT *, struct rpc_msg *);
	    bool_t	(*xp_freeargs)(struct SVCXPRT *, xdrproc_t, caddr_t);
	    void	(*xp_destroy)(struct SVCXPRT *);
#endif /* _NO_PROTO */
	} *xp_ops;
	int		xp_addrlen;	 /* length of remote address */
	struct sockaddr_in xp_raddr;	 /* remote address */
	struct opaque_auth xp_verf;	 /* raw response verifier */
	caddr_t		xp_p1;		 /* private: for use by svc ops */
	caddr_t		xp_p2;		 /* private: for use by svc ops */
	caddr_t		xp_p3;		 /* private: for use by svc lib */
} SVCXPRT;

/*
 *  Approved way of getting address of caller
 */
#define svc_getcaller(x) (&(x)->xp_raddr)

/*
 * Operations defined on an SVCXPRT handle
 *
 * SVCXPRT		*xprt;
 * struct rpc_msg	*msg;
 * xdrproc_t		 xargs;
 * caddr_t		 argsp;
 */
#define SVC_RECV(xprt, msg)				\
	(*(xprt)->xp_ops->xp_recv)((xprt), (msg))
#define svc_recv(xprt, msg)				\
	(*(xprt)->xp_ops->xp_recv)((xprt), (msg))

#define SVC_STAT(xprt)					\
	(*(xprt)->xp_ops->xp_stat)(xprt)
#define svc_stat(xprt)					\
	(*(xprt)->xp_ops->xp_stat)(xprt)

#define SVC_GETARGS(xprt, xargs, argsp)			\
	(*(xprt)->xp_ops->xp_getargs)((xprt), (xargs), (argsp))
#define svc_getargs(xprt, xargs, argsp)			\
	(*(xprt)->xp_ops->xp_getargs)((xprt), (xargs), (argsp))

#define SVC_REPLY(xprt, msg)				\
	(*(xprt)->xp_ops->xp_reply) ((xprt), (msg))
#define svc_reply(xprt, msg)				\
	(*(xprt)->xp_ops->xp_reply) ((xprt), (msg))

#define SVC_FREEARGS(xprt, xargs, argsp)		\
	(*(xprt)->xp_ops->xp_freeargs)((xprt), (xargs), (argsp))
#define svc_freeargs(xprt, xargs, argsp)		\
	(*(xprt)->xp_ops->xp_freeargs)((xprt), (xargs), (argsp))

#define SVC_DESTROY(xprt)				\
	(*(xprt)->xp_ops->xp_destroy)(xprt)
#define svc_destroy(xprt)				\
	(*(xprt)->xp_ops->xp_destroy)(xprt)


/*
 * Service request
 */
struct svc_req {
	u_long		rq_prog;	/* service program number */
	u_long		rq_vers;	/* service protocol version */
	u_long		rq_proc;	/* the desired procedure */
	struct opaque_auth rq_cred;	/* raw creds from the wire */
	caddr_t		rq_clntcred;	/* read only cooked cred */
	SVCXPRT	*rq_xprt;		/* associated transport */
};


/*
 * Service registration
 *
 * svc_register(xprt, prog, vers, dispatch, protocol)
 *	SVCXPRT *xprt;
 *	u_long prog;
 *	u_long vers;
 *	void (*dispatch)();
 *	int protocol;  like TCP or UDP, zero means do not register 
 */
#ifdef _NO_PROTO
extern bool_t	svc_register();
#else
extern bool_t	svc_register(SVCXPRT *, u_long, u_long, void (*)(struct svc_req *, SVCXPRT *), int);
#endif
/*
 * Service un-registration
 *
 * svc_unregister(prog, vers)
 *	u_long prog;
 *	u_long vers;
 */
#ifdef _NO_PROTO
extern void	svc_unregister();
#else
extern void	svc_unregister(u_long, u_long);
#endif

/*
 * Transport registration.
 *
 * xprt_register(xprt)
 *	SVCXPRT *xprt;
 */
#ifdef _NO_PROTO
extern void	xprt_register();
#else
extern void	xprt_register(SVCXPRT *);
#endif

#ifndef _KERNEL
/*
 * Transport un-register
 *
 * xprt_unregister(xprt)
 *	SVCXPRT *xprt;
 */
#ifdef _NO_PROTO
extern void	xprt_unregister();
#else
extern void	xprt_unregister(SVCXPRT *);
#endif

#endif /* !_KERNEL*/


/*
 * When the service routine is called, it must first check to see if it
 * knows about the procedure;  if not, it should call svcerr_noproc
 * and return.  If so, it should deserialize its arguments via 
 * SVC_GETARGS (defined above).  If the deserialization does not work,
 * svcerr_decode should be called followed by a return.  Successful
 * decoding of the arguments should be followed the execution of the
 * procedure's code and a call to svc_sendreply.
 *
 * Also, if the service refuses to execute the procedure due to too-
 * weak authentication parameters, svcerr_weakauth should be called.
 * Note: do not confuse access-control failure with weak authentication!
 *
 * NB: In pure implementations of rpc, the caller always waits for a reply
 * msg.  This message is sent when svc_sendreply is called.  
 * Therefore pure service implementations should always call
 * svc_sendreply even if the function logically returns void;  use
 * xdr.h - xdr_void for the xdr routine.  HOWEVER, tcp based rpc allows
 * for the abuse of pure rpc via batched calling or pipelining.  In the
 * case of a batched call, svc_sendreply should NOT be called since
 * this would send a return message, which is what batching tries to avoid.
 * It is the service/protocol writer's responsibility to know which calls are
 * batched and which are not.  Warning: responding to batch calls may
 * deadlock the caller and server processes!
 */

#ifdef _NO_PROTO
extern bool_t	svc_sendreply();
extern void	svcerr_decode();
extern void	svcerr_weakauth();
extern void	svcerr_noproc();
extern void	svcerr_progvers();
extern void	svcerr_auth();
extern void	svcerr_noprog();
#ifndef _KERNEL
extern void	svcerr_systemerr();
#endif /* _KERNEL */
#else /* _NO_PROTO */
extern bool_t	svc_sendreply(SVCXPRT *, xdrproc_t, caddr_t);
extern void	svcerr_decode(SVCXPRT *);
extern void	svcerr_weakauth(SVCXPRT *);
extern void	svcerr_noproc(SVCXPRT *);
extern void	svcerr_progvers(SVCXPRT *, u_long, u_long);
extern void	svcerr_auth(SVCXPRT *, enum auth_stat);
extern void	svcerr_noprog(SVCXPRT *);
#ifndef _KERNEL
extern void	svcerr_systemerr(SVCXPRT *);
#endif /* _KERNEL */
#endif /* _NO_PROTO */    
/*
 * Lowest level dispatching -OR- who owns this process anyway.
 * Somebody has to wait for incoming requests and then call the correct
 * service routine.  The routine svc_run does infinite waiting; i.e.,
 * svc_run never returns.
 * Since another (co-existant) package may wish to selectively wait for
 * incoming calls or other events outside of the rpc architecture, the
 * routine svc_getreq is provided.  It must be passed readfds, the
 * "in-place" results of a select system call (see select, section 2).
 */

#ifndef _KERNEL
/*
 * Global keeper of rpc service descriptors in use
 * dynamic; must be inspected before each call to select 
 */
extern fd_set svc_fdset;
#define svc_fds svc_fdset.fds_bits[0]	/* compatibility */

/*
 * a small program implemented by the svc_rpc implementation itself;
 * also see clnt.h for protocol numbers.
 */
extern void rpctest_service();
#endif /* !_KERNEL*/

#ifdef _NO_PROTO
extern void	svc_getreq();
#ifndef _KERNEL
extern void	svc_getreqset();	/* takes fdset instead of int */
#endif
#ifdef _KERNEL
extern int	svc_run();
#else
extern void	svc_run(); 	 /* never returns */
#endif
#else /* _NO_PROTO */
#ifndef _KERNEL
extern void	svc_getreq(int);
extern void	svc_getreqset(fd_set *);
#endif
#ifdef _KERNEL
extern void	svc_getreq(SVCXPRT *);
extern int	svc_run(SVCXPRT *);
#else
extern void	svc_run();
#endif
#endif /* _NO_PROTO */
/*
 * Socket to use on svcxxx_create call to get default socket
 */
#define	RPC_ANYSOCK	-1

/*
 * These are the existing service side transport implementations
 */

#ifndef _KERNEL
/*
 * Memory based rpc for testing and timing.
 */
#ifdef _NO_PROTO
extern SVCXPRT *svcraw_create();
#else
extern SVCXPRT *svcraw_create(void);
#endif
/*
 * Udp based rpc.
 */
#ifdef _NO_PROTO
extern SVCXPRT *svcudp_create();
extern SVCXPRT *svcudp_bufcreate();
#else
extern SVCXPRT *svcudp_create(int);
extern SVCXPRT *svcudp_bufcreate(int, u_int, u_int);
#endif
/*
 * Tcp based rpc.
 */
#ifdef _NO_PROTO
extern SVCXPRT *svctcp_create();
#else
extern SVCXPRT *svctcp_create(int, u_int, u_int);
#endif
/*
 * Like svtcp_create(), except the routine takes any *open* UNIX file
 * descriptor as its first input.
 */
#ifdef _NO_PROTO
SVCXPRT *svcfd_create();
#else
SVCXPRT *svcfd_create(int, u_int, u_int);
#endif
#else

/*
 * Kernel udp based rpc.
 */
extern SVCXPRT *svckudp_create();
#endif /* !_KERNEL*/

#ifdef _KERNEL
extern int svckudp_dupsave(struct svc_req *, caddr_t, int, int, caddr_t *);
extern int svckudp_dupdone(struct svc_req *, caddr_t, int, caddr_t);

#define	DUP_ADDED		0x00	/* dup entry added to cache */
#define	DUP_INPROGRESS		0x01	/* request in progress */
#define DUP_DONE		0x02	/* request complete */
#endif /* _KERNEL */

/*
 * Used in KERNEL and user space.  These statistics are recorded in the
 * nfs_krpc.ext kernel extension and reported via the cmd nfsstat or the
 * programming interface nfs_cntl() (see below).
 *
 * Server side rpc statistics
 * struct is to be used with
 * NFS_CNTL_GET_SVKRPC_STAT in which the return stats are placed
 * in var of this type, i.e.
 * t_rsstat rsstat;
 * nfs_cntl(NFS_CNTL_GET_SVKRPC_STAT,&rsstat,NFS_CNTL_SVKRPC_SIZE);
 */
typedef struct {
	int     rscalls;
	int     rsbadcalls;
	int     rsnullrecv;
	int     rsbadlen;
	int     rsxdrcall;
} t_rsstat ;
 
#endif /*!_RPC_SVC_H*/
