static char sccsid[] = "@(#)81  1.34.1.18  src/bos/usr/ccs/lib/libc/clnt_kudp.c, libcrpc, bos41J, 9513A_all 3/23/95 14:13:00";
/*
 *   COMPONENT_NAME: LIBCRPC
 *
 *   FUNCTIONS: backoff
 *		bindresvport
 *		buffree
 *		ckuwakeup
 *		clntkudp_abort
 *		clntkudp_callit
 *		clntkudp_callit_addr
 *		clntkudp_control
 *		clntkudp_create
 *		clntkudp_destroy
 *		clntkudp_error
 *		clntkudp_freeres
 *		clntkudp_init
 *		clntkudp_settimers
 *		htop
 *		noop
 *		ptoh
 *		init_clntxid
 *		init_rcstat
 *		nfs_sobind
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

#ifdef	_KERNEL
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = 	"@(#)clnt_kudp.c	1.8 90/07/19 4.1NFSSRC Copyr 1990 Sun Micro";
#endif

/* 
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * 1.59 88/02/16
 */


/*
 * clnt_kudp.c
 * Implements a kernel UPD/IP based, client side RPC.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/rtc.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/protosw.h>
#include <sys/mbuf.h>
#include <sys/lockname.h>
#include <sys/sleep.h>
#include <sys/errno.h>
#include <sys/atomic_op.h>
#include <net/if.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/in_var.h>
#include <netinet/ip_var.h>
#include <netinet/in_systm.h>
#include <netinet/in_pcb.h>
#include <rpc/types.h>
#include <rpc/xdr.h>
#include <rpc/auth.h>
#include <rpc/clnt.h>
#include <rpc/rpc_msg.h>
#include <nfs/nfs_trc.h>
#include <nfs/nfs_fscntl.h>

extern void	tstart(struct trb *);
extern int	tstop(struct trb *);
extern void	tfree(struct trb *);


struct mbuf	*ku_recvfrom();
int		ckuwakeup();

enum clnt_stat	clntkudp_callit();
void		clntkudp_abort();
void		clntkudp_error();
bool_t		clntkudp_freeres();
bool_t		clntkudp_control();
void		clntkudp_destroy();

void		xdrmbuf_init();

/*
 * Operations vector for UDP/IP based RPC
 */
/* static */ struct clnt_ops udp_ops = {
	clntkudp_callit,	/* do rpc call */
	clntkudp_abort,		/* abort call */
	clntkudp_error,		/* return error status */
	clntkudp_freeres,	/* free results */
	clntkudp_destroy,	/* destroy rpc handle */
	clntkudp_control	/* the ioctl() of rpc */
};

/*
 * Private data per rpc handle.  This structure is allocated by
 * clntkudp_create, and freed by cku_destroy.
 */
struct cku_private {
	u_int			 cku_flags;	/* see below */
	CLIENT			 cku_client;	/* client handle */
	int			 cku_retrys;	/* request retrys */
	struct socket		*cku_sock;	/* open udp socket */
	struct sockaddr_in	 cku_addr;	/* remote address */
	struct rpc_err		 cku_err;	/* error status */
	XDR			 cku_outxdr;	/* xdr routine for output */
	XDR			 cku_inxdr;	/* xdr routine for input */
	u_int			 cku_outpos;	/* position of in output mbuf */
	char			*cku_outbuf;	/* output buffer */
	char			*cku_inbuf;	/* input buffer */
	struct mbuf		*cku_inmbuf;	/* input mbuf */
	struct ucred		*cku_cred;	/* credentials */
	struct rpc_timers	*cku_timers;	/* for estimating RTT */
	struct rpc_timers	*cku_timeall;	/* for estimating RTT */
	void			(*cku_feedback)();
	caddr_t			cku_feedarg;	/* argument for feedback func */
	u_long			cku_xid;	/* current XID */
	struct	trb		*cku_trb;	/* timer block for cku */
	Simple_lock		cku_buflock;	/* lock for buffer management */
	int			cku_bufevent;	/* buf event word */
};

#define	KRPC_CLNTBUFF_LOCK(p)	disable_lock(PL_IMP, &(p->cku_buflock))
#define KRPC_CLNTBUFF_UNLOCK(p, ipri)	unlock_enable(ipri, &(p->cku_buflock))

t_rcstat rcstat;	/* rpc client statistics */

#define	ptoh(p)		(&((p)->cku_client))
#define	htop(h)		((struct cku_private *)((h)->cl_private))

/* cku_flags */
#define	CKU_TIMEDOUT	0x001
#define	CKU_BUFBUSY	0x008
#define	CKU_BUFWANTED	0x010
#define CKU_INTR	0x020

/* Times to retry */
#define	RECVTRIES	2
#define	SNDTRIES	4

u_long	clntxid;	/* transaction id used by all clients */

/*
 * init_clntxid() - initialize the clntxid for the sysstem.
 *	Should only be done once.
 */
void
init_clntxid()
{
	struct	timeval	time;

	curtime(&time);
	fetch_and_add((atomic_p)&clntxid, (time.tv_usec ^ time.tv_sec));
}

void
init_rcstat()
{
	bzero(&rcstat, sizeof(rcstat));
}

/* alloc_xid() - Provides a unique XID to the caller.
 */
ulong
alloc_xid()
{
	u_long	current_xid = 0;
	u_long	new_xid = 1;

	while (!compare_and_swap((atomic_p)&clntxid, 
				 (int)&current_xid,
				 (int)new_xid))
		new_xid += current_xid;

	return(new_xid);
}

static
noop()
{
}

/* Wakeup a thread that is waiting on the cku buffer
 */
static void
cku_bufwakeup(struct trb *t)
{
	e_clear_wait((tid_t)t->func_data, THREAD_TIMED_OUT);
}	

/* Wakeup a thread that is waiting on the cku buffer
 */
static void
cku_delay_end(register struct trb *t)
{
	e_wakeup_w_result((int *)t->func_data, THREAD_TIMED_OUT);
}	

/* cku delay function using the trb from the cku private
 * avoiding the page fault possibility from the real delay()
 */
static void
cku_delay(struct cku_private *p, int delay_hz)
{
	TICKS_TO_TIME(p->cku_trb->timeout.it_value, delay_hz);
	p->cku_trb->flags = T_INCINTERVAL;
	p->cku_trb->eventlist = EVENT_NULL;
	p->cku_trb->id = thread_self();
	p->cku_trb->func = cku_delay_end;
	p->cku_trb->func_data = (uint)(&(p->cku_trb->eventlist));
	p->cku_trb->ipri = PL_IMP;	/* serializing with network */

	/* Put on event list so that the wakeup will not be lost. */
	e_assert_wait(&p->cku_trb->eventlist, FALSE); /*not intrptable*/
	tstart(p->cku_trb);

	e_block_thread();
	while (tstop(p->cku_trb)); /* loop until stopped */
}

/* Called from the m_free().  If the process is waiting, get
 * it going again.
 */
static
buffree(buf, size, p)
	caddr_t buf;
	int	size;
	struct cku_private *p;
{
	int	ipri;

	ipri = KRPC_CLNTBUFF_LOCK(p);
	p->cku_flags &= ~CKU_BUFBUSY;
	if (p->cku_flags & CKU_BUFWANTED) {
		p->cku_flags &= ~CKU_BUFWANTED;
		e_wakeup(&(p->cku_bufevent));
	}
	KRPC_CLNTBUFF_UNLOCK(p, ipri);
}

/*
 * Create an rpc handle for a udp rpc connection.
 * Allocates space for the handle structure and the private data, and
 * opens a socket.  Note sockets and handles are one to one.
 */
CLIENT *
clntkudp_create(addr, pgm, vers, retrys, cred)
	struct sockaddr_in *addr;
	u_long pgm;
	u_long vers;
	int retrys;
	struct ucred *cred;
{
	register CLIENT *h;
	register struct cku_private *p;
	int error = 0;
	struct rpc_msg call_msg;
	struct mbuf *m;
	struct	timeval	time;

#ifdef RPCDEBUG
	rpc_debug(4, "clntkudp_create(%X, %d, %d, %d\n",
	    addr->sin_addr.s_addr, pgm, vers, retrys);
#endif
	p = (struct cku_private *)kmem_alloc(sizeof *p);
	pin(p, sizeof *p);
	bzero(p, sizeof *p);

	p->cku_bufevent = EVENT_NULL;

	h = ptoh(p);

	/* handle */
	h->cl_ops = &udp_ops;
	h->cl_private = (caddr_t) p;
	h->cl_auth = authkern_create();

	/* call message, just used to pre-serialize below */
	call_msg.rm_xid = 0;
	call_msg.rm_direction = CALL;
	call_msg.rm_call.cb_rpcvers = RPC_MSG_VERSION;
	call_msg.rm_call.cb_prog = pgm;
	call_msg.rm_call.cb_vers = vers;

	/* Generate the needed locks
	 */
	lock_alloc(&(p->cku_buflock),
		       LOCK_ALLOC_PIN, KRPC_CLNTBUFFER_LOCK, 0);
	simple_lock_init(&(p->cku_buflock));

	/* private */
	clntkudp_init(h, addr, retrys, cred);
	p->cku_outbuf = (char *)kmem_alloc((u_int)UDPMSGSIZE);
	pin(p->cku_outbuf, UDPMSGSIZE);
	m = m_clattach(p->cku_outbuf, noop, UDPMSGSIZE, 0, M_WAIT);
	if (m == NULL)
		goto bad;
	xdrmbuf_init(&p->cku_outxdr, m, XDR_ENCODE);

	/* pre-serialize call message header */
	if (! xdr_callhdr(&(p->cku_outxdr), &call_msg)) {
		TRCHKL0(HKWD_NFS_RPC_DEBUG|hkwd_KRPC_UDPCREATE_FAIL_1);
		(void) m_freem(m);
		goto bad;
	}
	p->cku_outpos = XDR_GETPOS(&(p->cku_outxdr));
	(void) m_free(m);

	/* open udp socket */
	error = socreate(AF_INET, &p->cku_sock, SOCK_DGRAM, IPPROTO_UDP);
	if (error) {
		TRCHKL1(HKWD_NFS_RPC_DEBUG|hkwd_KRPC_UDPCREATE_FAIL_2, error);
		goto bad;
	}
	if (error = bindresvport(p->cku_sock)) {
		TRCHKL1(HKWD_NFS_RPC_DEBUG|hkwd_KRPC_UDPCREATE_FAIL_3, error);
		goto bad;
	}

	/* Get this here so that we do not page fault later on a send.
	 */
	if (!(p->cku_trb = talloc())) {
		goto bad;
	}


	return (h);

bad:
	lock_free(&(p->cku_buflock));
	unpin(p->cku_outbuf, UDPMSGSIZE);
	kmem_free((caddr_t)p->cku_outbuf, (u_int)UDPMSGSIZE);
	unpin(p, sizeof *p);
	kmem_free((caddr_t)p, (u_int)sizeof (struct cku_private));
#ifdef RPCDEBUG
	rpc_debug(4, "create failed\n");
#endif
	return ((CLIENT *)NULL);
}

clntkudp_init(h, addr, retrys, cred)
	CLIENT *h;
	struct sockaddr_in *addr;
	int retrys;
	struct ucred *cred;
{
	struct cku_private *p = htop(h);

	p->cku_retrys = retrys;
	p->cku_addr = *addr;
	p->cku_cred = cred;
	p->cku_xid = 0;
	p->cku_flags &= (CKU_BUFBUSY | CKU_BUFWANTED);
	clntkudp_intr(h, FALSE);
}

/*
 * mark a client as interruptable
 */
clntkudp_intr(cl, flag)
	CLIENT  *cl;
{
	struct cku_private *p = htop(cl);

	if (flag != FALSE) {
		p->cku_flags |= CKU_INTR;
	} else {
		p->cku_flags &= ~CKU_INTR;
	}
}

/*
 * set the timers.  Return current retransmission timeout.
 */
clntkudp_settimers(h, t, all, minimum, feedback, arg, xid)
	CLIENT *h;
	struct rpc_timers *t, *all;
	unsigned int minimum;
	void (*feedback)();
	caddr_t arg;
	u_long xid;
{
	struct cku_private *p = htop(h);
	int value;

	p->cku_feedback = feedback;
	p->cku_feedarg = arg;
	p->cku_timers = t;
	p->cku_timeall = all;
	if (xid)
		p->cku_xid = xid;
	else
		p->cku_xid = alloc_xid();
	value = all->rt_rtxcur;
	value += t->rt_rtxcur;
	if (value < minimum)
		return (minimum);
	(void)fetch_and_add((atomic_p)&rcstat.rctimers, 1);
	return (value);
}

/*
 * Time out back off function. tim is in hz
 */
#define	MAXTIMO	(20 * hz)
#define	backoff(tim)	((((tim) << 1) > MAXTIMO) ? MAXTIMO : ((tim) << 1))

/*
 * Call remote procedure.
 * Most of the work of rpc is done here.  We serialize what is left
 * of the header (some was pre-serialized in the handle), serialize
 * the arguments, and send it off.  We wait for a reply or a time out.
 * Timeout causes an immediate return, other packet problems may cause
 * a retry on the receive.  When a good packet is received we deserialize
 * it, and check verification.  A bad reply code will cause one retry
 * with full (longhand) credentials.
 * If "ignorebad" is true, rpc replies with remote errors are ignored.
 */
enum clnt_stat
clntkudp_callit_addr(h, procnum, xdr_args, argsp, xdr_results, resultsp, wait,
		sin, ignorebad)
	register CLIENT	*h;
	u_long		procnum;
	xdrproc_t	xdr_args;
	caddr_t		argsp;
	xdrproc_t	xdr_results;
	caddr_t		resultsp;
	struct timeval	wait;
	struct sockaddr_in *sin;
	int	ignorebad;
{
	register struct cku_private *p = htop(h);
	register XDR		   *xdrs;
	register struct socket	   *so;
	int			   rtries;
	int			   stries;
	struct mbuf		   *m;
	int timohz;
	u_long xid;
	u_int rempos = 0;
	int refreshes = 2;	/* number of times to refresh credential */
	int round_trip;		/* time the RPC */
	sigset_t smask;		/* saved signal mask */
	sigset_t nmask;
	struct	timeval		t;
	int			error;
	int			ipri;
	struct sockaddr_in	from;


	so = p->cku_sock;
	stries = p->cku_retrys;

/* tv_usec is really nano-seconds in the kernel and we are spoofing the
 * structure for porting reasons.
 */
#define time_in_hz (curtime(&(t)) ? \
		     (t.tv_sec*hz + t.tv_usec/NS_PER_TICK) : \
		     (t.tv_sec*hz + t.tv_usec/NS_PER_TICK))

#ifdef RPCDEBUG
	rpc_debug(4, "cku_callit\n");
#endif

	(void)fetch_and_add((atomic_p)&rcstat.rccalls, 1);

	if (p->cku_xid == 0)
		xid = alloc_xid();
	else
		xid = p->cku_xid;

	/*
	 * This is dumb but easy: keep the time out in units of hz
	 * so it is easy to call timeout and modify the value.
	 */
	timohz = wait.tv_sec * hz + (wait.tv_usec * hz) / 1000000;

	TRCHKL2T(HKWD_NFS_CALL|hkwd_NFS_CALL_ENTRY, xid,
		p->cku_addr.sin_addr.s_addr);

call_again:

	p->cku_err.re_errno = RPC_SUCCESS; /* reset in case of error on last */

	/*
	 * Wait til buffer gets freed then make a type 2 mbuf point at it
	 * The buffree routine clears CKU_BUFBUSY and does a wakeup when
	 * the mbuf gets freed.
	 */
	ipri = KRPC_CLNTBUFF_LOCK(p);
	while (p->cku_flags & CKU_BUFBUSY) {
		p->cku_flags |= CKU_BUFWANTED;
		/*
		 * This is a kludge to avoid deadlock in the case of a
		 * loop-back call.  The client can block wainting for
		 * the server to free the mbuf while the server is blocked
		 * waiting for the client to free the reply mbuf.  Avoid this
		 * by flushing the input queue every once in a while while
		 * we are waiting.
		 */
		p->cku_trb->flags = T_INCINTERVAL;
		p->cku_trb->timeout.it_value.tv_sec = 1;
		p->cku_trb->timeout.it_value.tv_nsec = 0;
		p->cku_trb->id = thread_self();
		p->cku_trb->func = cku_bufwakeup;
		p->cku_trb->ipri = PL_IMP;	/* serializing with network */
		p->cku_trb->func_data = (ulong)thread_self();

		/* Put on event list so that the wakeup will not be lost. */
		e_assert_wait(&p->cku_bufevent, FALSE); /*not intrptable*/
		tstart(p->cku_trb);
		KRPC_CLNTBUFF_UNLOCK(p, ipri);

		if (e_block_thread() != THREAD_TIMED_OUT) {
	        	while (tstop(p->cku_trb));	/* loop til stopped */
		}

		SOCKET_LOCK(so);
		sbflush(&so->so_rcv);
		SOCKET_UNLOCK(so);

		/* make sure we are not at an interrupt level when 
		 * taking the socket lock
		 */
		ipri = KRPC_CLNTBUFF_LOCK(p);
	}
	p->cku_flags |= CKU_BUFBUSY;
	KRPC_CLNTBUFF_UNLOCK(p, ipri);


	m = m_clattach(p->cku_outbuf, buffree, UDPMSGSIZE, (int)p, M_WAIT);
	if (m == NULL) {
		p->cku_err.re_status = RPC_SYSTEMERROR;
		p->cku_err.re_errno = ENOBUFS;
		buffree(p->cku_outbuf, UDPMSGSIZE, p);
		goto done;
	}

	xdrs = &p->cku_outxdr;
	/*
	 * The transaction id is the first thing in the
	 * preserialized output buffer.
	 */
	(*(u_long *)(p->cku_outbuf)) = xid;

	xdrmbuf_init(xdrs, m, XDR_ENCODE);

	if (rempos != 0) {
		XDR_SETPOS(xdrs, rempos);
	} else {
		/*
		 * Serialize dynamic stuff into the output buffer.
		 */
		XDR_SETPOS(xdrs, p->cku_outpos);
		if ((! XDR_PUTLONG(xdrs, (long *)&procnum)) ||
		    (! AUTH_MARSHALL(h->cl_auth, xdrs)) ||
		    (! (*xdr_args)(xdrs, argsp))) {
			p->cku_err.re_status = RPC_CANTENCODEARGS;
			p->cku_err.re_errno = EIO;
			(void) m_freem(m);
			goto done;
		}
		rempos = XDR_GETPOS(xdrs);
	}
	m->m_len = rempos;

	round_trip = time_in_hz;
	if ((p->cku_err.re_errno = 
		ku_sendto_mbuf(so, m, &p->cku_addr, (struct socket *)NULL)) != 0) {
		p->cku_err.re_status = RPC_CANTSEND;
		p->cku_err.re_errno = EIO;
		goto done;
	}

recv_again:
	for (rtries = RECVTRIES; rtries; rtries--) {
		SOCKET_LOCK(so);

		so->so_error = 0;

		if (error = sosblock(&so->so_rcv, so)) {
			SOCKET_UNLOCK(so);
			if (error == EINTR) {
				p->cku_err.re_status = RPC_INTR;
				p->cku_err.re_errno = EINTR;
			}else{
				p->cku_err.re_status = RPC_CANTRECV;
				p->cku_err.re_errno = EIO;
			}
			goto done;
		}

		so->so_rcv.sb_flags |= SB_WAIT;

		/*
		 * Set timeout then wait for input, timeout
		 * or interrupt.
		 */

		/* set the timeout for the wait on incoming data */
		so->so_rcv.sb_timeo = timohz;

		SIGINITSET(nmask);
		SIGADDSET(nmask, SIGHUP);
		SIGADDSET(nmask, SIGINT);
		SIGADDSET(nmask, SIGQUIT);
		SIGADDSET(nmask, SIGTERM);
		SIGADDSET(nmask, SIGKILL);
		limit_sigs(&nmask, &smask);

#ifdef RPCDEBUG
			rpc_debug(3, "callit: waiting %d\n", timohz);
#endif

		error = 0;
		/* check to see if we should ask for an interruptable wait
		   in the socket code.
		   */
		if ((p->cku_flags & CKU_INTR) != 0) {
			so->so_rcv.sb_flags &= ~SB_NOINTR;
		}else{
			so->so_rcv.sb_flags |= SB_NOINTR;
		}

		/* use wakeone with the use of the timeout value */

		so->so_rcv.sb_flags |= SB_WAKEONE | SB_WAIT;

		so->so_rcv.sb_timer = p->cku_trb;

		while (so->so_rcv.sb_cc == 0 && !error)
			error = sosbwait(&so->so_rcv, so);

		so->so_rcv.sb_timer = NULL;
		so->so_rcv.sb_flags &= ~SB_WAKEONE;

		/* fix up signal state */
		sigsetmask(&smask);

		if (error == EINTR) {
			p->cku_err.re_status = RPC_INTR;
			p->cku_err.re_errno = EINTR;
			sbunlock(&so->so_rcv);
			SOCKET_UNLOCK(so);
			goto done;
		}
		if (error == ETIMEDOUT) {
			p->cku_err.re_status = RPC_TIMEDOUT;
			p->cku_err.re_errno = ETIMEDOUT;
			sbunlock(&so->so_rcv);
			SOCKET_UNLOCK(so);
			(void)fetch_and_add((atomic_p)&rcstat.rctimeouts, 1);
			goto done;
		}
			
		if (so->so_error) {
			so->so_error = 0;
			continue;
		}

		/* unlock before our call to ku_recvfrom() */
		sbunlock(&so->so_rcv);
		SOCKET_UNLOCK(so);

		p->cku_inmbuf = ku_recvfrom(so, &from);
		if (sin) {
			*sin = from;
		}

		if (p->cku_inmbuf == NULL) {
			continue;
		}
		p->cku_inbuf = mtod(p->cku_inmbuf, char *);

		if (p->cku_inmbuf->m_len < sizeof (u_long)) {
			m_freem(p->cku_inmbuf);
			continue;
		}
		/*
		 * If reply transaction id matches id sent
		 * we have a good packet.
		 */
		if (*((u_long *)(p->cku_inbuf))
		    != *((u_long *)(p->cku_outbuf))) {
			(void)fetch_and_add((atomic_p)&rcstat.rcbadxids, 1);
			m_freem(p->cku_inmbuf);
			continue;
		}

		break;
	}

	if (rtries == 0) {
		p->cku_err.re_status = RPC_CANTRECV;
		p->cku_err.re_errno = EIO;
		goto done;
	}

	round_trip = time_in_hz - round_trip;
	/*
	 * Van Jacobson timer algorithm here, only if NOT a retransmission.
	 */
	if (p->cku_timers != (struct rpc_timers *)0 &&
	    (stries == p->cku_retrys) && !ignorebad) {
		register int rt;

		rt = round_trip;
		rt -= (p->cku_timers->rt_srtt >> 3);
		p->cku_timers->rt_srtt += rt;
		if (rt < 0)
			rt = - rt;
		rt -= (p->cku_timers->rt_deviate >> 2);
		p->cku_timers->rt_deviate += rt;
		p->cku_timers->rt_rtxcur =
			((p->cku_timers->rt_srtt >> 2) +
			    p->cku_timers->rt_deviate) >> 1;

		rt = round_trip;
		rt -= (p->cku_timeall->rt_srtt >> 3);
		p->cku_timeall->rt_srtt += rt;
		if (rt < 0)
			rt = - rt;
		rt -= (p->cku_timeall->rt_deviate >> 2);
		p->cku_timeall->rt_deviate += rt;
		p->cku_timeall->rt_rtxcur =
			((p->cku_timeall->rt_srtt >> 2) +
			    p->cku_timeall->rt_deviate) >> 1;
		if (p->cku_feedback != (void (*)()) 0)
		    (*p->cku_feedback)(FEEDBACK_OK, procnum, p->cku_feedarg);
	}

	/*
	 * Process reply
	 */

	xdrs = &(p->cku_inxdr);
	xdrmbuf_init(xdrs, p->cku_inmbuf, XDR_DECODE);

	{
		/*
		 * Declare this variable here to have smaller
		 * demand for stack space in this procedure.
		 */
		struct rpc_msg		   reply_msg;

		reply_msg.acpted_rply.ar_verf = _null_auth;
		reply_msg.acpted_rply.ar_results.where = resultsp;
		reply_msg.acpted_rply.ar_results.proc = xdr_results;

		/*
		 * Decode and validate the response.
		 */
		if (xdr_replymsg(xdrs, &reply_msg)) {
			_seterr_reply(&reply_msg, &(p->cku_err));

			if (p->cku_err.re_status == RPC_SUCCESS) {
				/*
				 * Reply is good, check auth.
				 */
				if (! AUTH_VALIDATE(h->cl_auth,
				    &reply_msg.acpted_rply.ar_verf)) {
					p->cku_err.re_status = RPC_AUTHERROR;
					p->cku_err.re_why = AUTH_INVALIDRESP;
					(void)fetch_and_add((atomic_p)&rcstat.rcbadverfs, 1);
				}
				if (reply_msg.acpted_rply.ar_verf.oa_base !=
				    NULL) {
					/* free auth handle */
					xdrs->x_op = XDR_FREE;
					(void) xdr_opaque_auth(xdrs,
					    &(reply_msg.acpted_rply.ar_verf));
				}
			} else {
				p->cku_err.re_errno = EIO;
				if(p->cku_err.re_why == AUTH_ERROR) {
					bcopy(&(p->cku_err.re_why), resultsp, sizeof(p->cku_err.re_why));
				}
				/*
				 * Maybe our credential needs refreshed
				 */
				if (refreshes > 0 && AUTH_REFRESH(h->cl_auth)) {
					refreshes--;
					(void)fetch_and_add((atomic_p)&rcstat.rcnewcreds, 1);
					rempos = 0;
				}
			}
		} else {
			p->cku_err.re_status = RPC_CANTDECODERES;
			p->cku_err.re_errno = EIO;
		}
	}

	m_freem(p->cku_inmbuf);

#ifdef RPCDEBUG
	rpc_debug(4, "cku_callit done\n");
#endif
done:
	/*
	 * Weed out remote rpc error replies.
	 */
	if (ignorebad &&
		((p->cku_err.re_status == RPC_VERSMISMATCH) ||
		    (p->cku_err.re_status == RPC_AUTHERROR) ||
		    (p->cku_err.re_status == RPC_PROGUNAVAIL) ||
		    (p->cku_err.re_status == RPC_PROGVERSMISMATCH) ||
		    (p->cku_err.re_status == RPC_PROCUNAVAIL) ||
		    (p->cku_err.re_status == RPC_CANTDECODEARGS) ||
		    (p->cku_err.re_status == RPC_SYSTEMERROR)))
		goto recv_again;

	/*
	 * Flush the rest of the stuff on the input queue
	 * for the socket.
	 */
	SOCKET_LOCK(so);
	sbflush(&so->so_rcv);
	SOCKET_UNLOCK(so);

	if ((p->cku_err.re_status != RPC_SUCCESS) &&
	    (p->cku_err.re_status != RPC_INTR) &&
	    (p->cku_err.re_status != RPC_CANTENCODEARGS)) {
		if (p->cku_feedback != (void (*)()) 0 &&
		    stries == p->cku_retrys)
			(*p->cku_feedback)(FEEDBACK_REXMIT1,
				procnum, p->cku_feedarg);
		timohz = backoff(timohz);
		if (p->cku_timeall != (struct rpc_timers *)0)
			p->cku_timeall->rt_rtxcur = timohz;
		if (p->cku_err.re_status == RPC_SYSTEMERROR ||
		    p->cku_err.re_status == RPC_CANTSEND) {
			/*
			 * Errors due to lack o resources, wait a bit
			 * and try again.
			 */
			(void) cku_delay(p, hz);
		}
		if (--stries > 0) {
			(void)fetch_and_add((atomic_p)&rcstat.rcretrans, 1);
			goto call_again;
		}
	}

	/*
	 * Insure that buffer is not busy prior to releasing client handle.
	 */
	ipri = KRPC_CLNTBUFF_LOCK(p);
	while (p->cku_flags & CKU_BUFBUSY) {
		p->cku_flags |= CKU_BUFWANTED;

		p->cku_trb->flags = T_INCINTERVAL;
		p->cku_trb->timeout.it_value.tv_sec = 1;
		p->cku_trb->timeout.it_value.tv_nsec = 0;
		p->cku_trb->id = thread_self();
		p->cku_trb->func = cku_bufwakeup;
		p->cku_trb->ipri = PL_IMP;	/* serializing with network */
		p->cku_trb->func_data = (ulong)thread_self();

		/* Put on event list so that the wakeup will not be lost. */
		e_assert_wait(&p->cku_bufevent, FALSE); /*not intrptable*/
		tstart(p->cku_trb);
		KRPC_CLNTBUFF_UNLOCK(p, ipri);

		if (e_block_thread() != THREAD_TIMED_OUT) {
        		while (tstop(p->cku_trb)); /* loop til stopped */
		}

		SOCKET_LOCK(so);
		sbflush(&so->so_rcv);
		SOCKET_UNLOCK(so);

		/* make sure we are not at an interrupt level when 
		 * taking the socket lock
		 */
		ipri = KRPC_CLNTBUFF_LOCK(p);
	}
	KRPC_CLNTBUFF_UNLOCK(p, ipri);

	if ((error = p->cku_err.re_status) != RPC_SUCCESS) {
		(void)fetch_and_add((atomic_p)&rcstat.rcbadcalls, 1);
	}

        TRCHKL2T(HKWD_NFS_CALL|hkwd_NFS_CALL_EXIT, xid,
		p->cku_addr.sin_addr.s_addr);

	return (error);
}

enum clnt_stat
clntkudp_callit(h, procnum, xdr_args, argsp, xdr_results, resultsp, wait)
	register CLIENT *h;
	u_long		procnum;
	xdrproc_t	xdr_args;
	caddr_t		argsp;
	xdrproc_t	xdr_results;
	caddr_t		resultsp;
	struct timeval	wait;
{
	return (clntkudp_callit_addr(h, procnum, xdr_args, argsp, xdr_results,
		resultsp, wait, (struct sockaddr_in *)0, 0));
}


/*
 * Return error info on this handle.
 */
void
clntkudp_error(h, err)
	CLIENT *h;
	struct rpc_err *err;
{
	register struct cku_private *p = htop(h);

	*err = p->cku_err;
}

/* static */ bool_t
clntkudp_freeres(cl, xdr_res, res_ptr)
	CLIENT *cl;
	xdrproc_t xdr_res;
	caddr_t res_ptr;
{
	register struct cku_private *p = (struct cku_private *)cl->cl_private;
	register XDR *xdrs;

	xdrs = &(p->cku_outxdr);

	xdrs->x_op = XDR_FREE;
	return ((*xdr_res)(xdrs, res_ptr));
}

void
clntkudp_abort()
{
}

bool_t
clntkudp_control()
{
	return (FALSE);
}


/*
 * Destroy rpc handle.
 * Frees the space used for output buffer, private data, and handle
 * structure, and closes the socket for this handle.
 */
void
clntkudp_destroy(h)
	CLIENT *h;
{
	register struct cku_private *p = htop(h);

#ifdef RPCDEBUG
	rpc_debug(4, "cku_destroy %x\n", h);
#endif
        while (tstop(p->cku_trb)); /* loop until stopped */
	tfree(p->cku_trb);

	lock_free(&(p->cku_buflock));

	(void) soclose(p->cku_sock);

	unpin(p->cku_outbuf, UDPMSGSIZE);
	kmem_free((caddr_t)p->cku_outbuf, (u_int)UDPMSGSIZE);
	unpin(p, sizeof *p);
	kmem_free((caddr_t)p, sizeof (*p));
}

/*
 * try to bind to a reserved port
 */
bindresvport(so)
	struct socket *so;
{
	struct sockaddr_in *sin;
	struct mbuf *m;
	u_short i;
	int error;
	struct ucred *savecred;

#	define MAX_PRIV	(IPPORT_RESERVED-1)
#	define MIN_PRIV	(IPPORT_RESERVED/2)

	m = m_get(M_WAIT, MT_SONAME);
	if (m == NULL) {
		printf("bindresvport: couldn't alloc mbuf");
		TRCHKL0(HKWD_NFS_RPC_DEBUG|hkwd_KRPC_MBUF_GET_FAIL);
		return (ENOBUFS);
	}

	sin = mtod(m, struct sockaddr_in *);
	sin->sin_family = AF_INET;
	sin->sin_addr.s_addr = INADDR_ANY;
	m->m_len = sizeof (struct sockaddr_in);

	error = EADDRINUSE;
	for (i = MAX_PRIV; error == EADDRINUSE && i >= MIN_PRIV; i--) {
		sin->sin_port = htons(i);
		error = nfs_sobind(so, m);
	}
	(void) m_freem(m);
	return (error);
}

/*
 * This can be a privledged port instead of sobind we have nfs_sobind.
 * - same as uipc socket sobind except we can get around the suser check for 
 * U.U_cred = 0.
 */
int
nfs_sobind(struct socket *so, struct mbuf *nam)
{

	int error;

        SOCKET_LOCK(so);
	so->so_state |= SS_PRIV;
        error =
            (*so->so_proto->pr_usrreq)(so, PRU_BIND,
                (struct mbuf *)0, nam, (struct mbuf *)0);
        SOCKET_UNLOCK(so);
        return (error);

}
#endif /*	_KERNEL*/
