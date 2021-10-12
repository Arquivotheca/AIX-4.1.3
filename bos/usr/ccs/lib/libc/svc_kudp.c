static char sccsid[] = "@(#)08  1.18.1.10  src/bos/usr/ccs/lib/libc/svc_kudp.c, libcrpc, bos41J, 9511A_all 3/3/95 14:46:57";
/*
 *   COMPONENT_NAME: LIBCRPC
 *
 *   FUNCTIONS: DRHASH
 *		REQTOXID
 *		XIDHASH
 *		buffree
 *		rpc_buffer
 *		svckudp_create
 *		svckudp_destroy
 *		svckudp_dup
 *		svckudp_dupdone
 *		svckudp_dupsave
 *		svckudp_freeargs
 *		svckudp_getargs
 *		svckudp_recv
 *		svckudp_send
 *		svckudp_stat
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
static char sccsid[] = 	"@(#)svc_kudp.c	1.3 90/07/19 4.1NFSSRC Copyr 1990 Sun Micro";
#endif
 *
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * 1.28 88/02/08
 */

#ifdef _KERNEL

/*
 * svc_kudp.c,
 * Server side for UDP/IP based RPC in the kernel.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <rpc/types.h>
#include <netinet/in.h>
#include <rpc/xdr.h>
#include <rpc/auth.h>
#include <rpc/clnt.h>
#include <rpc/rpc_msg.h>
#include <rpc/svc.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/mbuf.h>
#include <sys/vfs.h>
#include <sys/vnode.h>
#include <sys/errno.h>
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
#include <sys/atomic_op.h>
#include <nfs/nfs.h>
#include <nfs/nfs_trc.h>
#include <nfs/nfs_fscntl.h>


#define	rpc_buffer(xprt) ((xprt)->xp_p1)

/*
 * Routines exported through ops vector.
 */
bool_t		svckudp_recv();
bool_t		svckudp_send();
enum xprt_stat	svckudp_stat();
bool_t		svckudp_getargs();
bool_t		svckudp_freeargs();
void		svckudp_destroy();

/*
 * Server transport operations vector.
 */
struct xp_ops svckudp_op = {
	svckudp_recv,		/* Get requests */
	svckudp_stat,		/* Return status */
	svckudp_getargs,	/* Deserialize arguments */
	svckudp_send,		/* Send reply */
	svckudp_freeargs,	/* Free argument data space */
	svckudp_destroy		/* Destroy transport handle */
};


struct mbuf	*ku_recvfrom();
void		xdrmbuf_init();

/*
 * Transport private data.
 * Kept in xprt->xp_p2.
 */
struct udp_data {
	int	ud_flags;			/* flag bits, see below */
	u_long 	ud_xid;				/* id */
	struct	mbuf *ud_inmbuf;		/* input mbuf chain */
	XDR	ud_xdrin;			/* input xdr stream */
	XDR	ud_xdrout;			/* output xdr stream */
	char	ud_verfbody[MAX_AUTH_BYTES];	/* verifier */
	Simple_lock	ud_lock;		/* lock for buffree */
	int	ud_event;			/* event list */
};

#define KRPC_SVCBUFF_LOCK(ud)	disable_lock(PL_IMP, &(ud->ud_lock))
#define KRPC_SVCBUFF_UNLOCK(ud, ipri) unlock_enable(ipri, &(ud->ud_lock))


/*
 * Flags
 */
#define	UD_BUSY		0x001		/* buffer is busy */
#define	UD_WANTED	0x002		/* buffer wanted */

/*
 * Server statistics
 */
t_rsstat rsstat;

#define	INCR_RSSTAT_RSCALLS()    fetch_and_add((atomic_p)&rsstat.rscalls, 1)
#define INCR_RSSTAT_RSBADCALLS() fetch_and_add((atomic_p)&rsstat.rsbadcalls, 1)
#define INCR_RSSTAT_RSNULLRECV() fetch_and_add((atomic_p)&rsstat.rsnullrecv, 1)
#define INCR_RSSTAT_RSBADLEN()   fetch_and_add((atomic_p)&rsstat.rsbadlen, 1)
#define INCR_RSSTAT_RSXDRCALL()  fetch_and_add((atomic_p)&rsstat.rsxdrcall, 1)

void
init_rsstat()
{
	bzero(&rsstat, sizeof(rsstat));
}

/*
 * Create a transport record.
 * The transport record, output buffer, and private data structure
 * are allocated.  The output buffer is serialized into using xdrmem.
 * There is one transport record per user process which implements a
 * set of services.
 */
SVCXPRT *
svckudp_create(sock, port)
	struct socket	*sock;
	u_short		 port;
{
	register SVCXPRT	 *xprt;
	register struct udp_data *ud;

#ifdef RPCDEBUG
	rpc_debug(4, "svckudp_create so = %x, port = %d\n", sock, port);
#endif
	xprt = (SVCXPRT *)kmem_alloc((u_int)sizeof(SVCXPRT));
	pin(xprt, sizeof(SVCXPRT));
	rpc_buffer(xprt) = (caddr_t)kmem_alloc((u_int)UDPMSGSIZE);
	pin(rpc_buffer(xprt), UDPMSGSIZE);
	ud = (struct udp_data *)kmem_alloc((u_int)sizeof(struct udp_data));
	pin(ud, sizeof(struct udp_data));
	bzero((caddr_t)ud, sizeof(*ud));

	/* Get the lock for this service struct.  Make sure to free 
	 * everything before returning.
	 */
	lock_alloc(&(ud->ud_lock), LOCK_ALLOC_PIN, KRPC_SVCBUFFER_LOCK,0);
	simple_lock_init(&(ud->ud_lock));
		      
	ud->ud_event = EVENT_NULL;

	xprt->xp_addrlen = 0;
	xprt->xp_p2 = (caddr_t)ud;
	xprt->xp_p3 = NULL;
	xprt->xp_verf.oa_base = ud->ud_verfbody;
	xprt->xp_ops = &svckudp_op;
	xprt->xp_port = port;
	xprt->xp_sock = sock;
	xprt_register(xprt);
	return (xprt);
}
 
/*
 * Destroy a transport record.
 * Frees the space allocated for a transport record.
 */
void
svckudp_destroy(xprt)
	register SVCXPRT   *xprt;
{
	register struct udp_data *ud = (struct udp_data *)xprt->xp_p2;
	int						  ipri;

#ifdef RPCDEBUG
	rpc_debug(4, "usr_destroy %x\n", xprt);
#endif
	/*
	 * Make sure that the buffer is not in use before freeing it.
	 */
	ipri = KRPC_SVCBUFF_LOCK(ud);
	while (ud->ud_flags & UD_BUSY) {
		ud->ud_flags |= UD_WANTED;
		e_assert_wait(&(ud->ud_event), FALSE);
		KRPC_SVCBUFF_UNLOCK(ud, ipri);
		e_block_thread();
		ipri = KRPC_SVCBUFF_LOCK(ud);
	}
	KRPC_SVCBUFF_UNLOCK(ud, ipri);

	if (ud->ud_inmbuf) {
		m_freem(ud->ud_inmbuf);
	}
	
	lock_free(&(ud->ud_lock));

	unpin(ud, sizeof(struct udp_data));
	kmem_free((caddr_t)ud, (u_int)sizeof(struct udp_data));
	unpin(rpc_buffer(xprt), UDPMSGSIZE);
	kmem_free((caddr_t)rpc_buffer(xprt), (u_int)UDPMSGSIZE);
	unpin(xprt, sizeof(SVCXPRT));
	kmem_free((caddr_t)xprt, (u_int)sizeof(SVCXPRT));
}

/*
 * Receive rpc requests.
 * Pulls a request in off the socket, checks if the packet is intact,
 * and deserializes the call packet.
 */
bool_t
svckudp_recv(xprt, msg)
	register SVCXPRT	 *xprt;
	struct rpc_msg		 *msg;
{
	register struct udp_data *ud = (struct udp_data *)xprt->xp_p2;
	register XDR	 *xdrs = &(ud->ud_xdrin);
	register struct mbuf	 *m;
	int			  s;

#ifdef RPCDEBUG
	rpc_debug(4, "svckudp_recv %x\n", xprt);
#endif
	INCR_RSSTAT_RSCALLS();

	m = ku_recvfrom(xprt->xp_sock, &(xprt->xp_raddr));

	if (m == NULL) {
		INCR_RSSTAT_RSNULLRECV();
		return (FALSE);
	}

	if ((m->m_len < 8 * BYTES_PER_XDR_UNIT) &&
	    (m = m_pullup(m, 8 * BYTES_PER_XDR_UNIT)) == NULL) {
		INCR_RSSTAT_RSBADLEN();
		goto badnofree;
	}

	xdrmbuf_init(&ud->ud_xdrin, m, XDR_DECODE);
	if (! xdr_callmsg(xdrs, msg)) {
		INCR_RSSTAT_RSXDRCALL();
		goto bad;
	}
	ud->ud_xid = msg->rm_xid;
	ud->ud_inmbuf = m;
#ifdef RPCDEBUG
	rpc_debug(5, "svckudp_recv done\n");
#endif
	return (TRUE);

bad:
	m_freem(m);
badnofree:
	ud->ud_inmbuf = NULL;
	INCR_RSSTAT_RSBADCALLS();
	return (FALSE);
}


static
buffree(buf, size, ud)
	caddr_t	buf;
	int	size;
	register struct udp_data *ud;
{
	int	ipri;

	ipri = KRPC_SVCBUFF_LOCK(ud);

	ud->ud_flags &= ~UD_BUSY;
	if (ud->ud_flags & UD_WANTED) {
		ud->ud_flags &= ~UD_WANTED;
		e_wakeup(&(ud->ud_event));
	}

	KRPC_SVCBUFF_UNLOCK(ud, ipri);
}

bool_t
/* ARGSUSED */
svckudp_send(xprt, msg)
	register SVCXPRT *xprt; 
	struct rpc_msg *msg; 
{
	return(svckudp_send_othersock(xprt, msg, (struct socket *)NULL));
}

/*
 * Send rpc reply.
 * Serialize the reply packet into the output buffer then
 * call ku_sendto to make an mbuf out of it and send it.
 */
bool_t
/* ARGSUSED */
svckudp_send_othersock(xprt, msg, othersock)
	register SVCXPRT *xprt; 
	struct rpc_msg *msg; 
	struct socket *othersock;
{
	register struct udp_data *ud = (struct udp_data *)xprt->xp_p2;
	register XDR *xdrs = &(ud->ud_xdrout);
	register int slen;
	register int stat = FALSE;
	int ipri;
	struct mbuf *m, *m_clattach();

#ifdef RPCDEBUG
	rpc_debug(4, "svckudp_send %x\n", xprt);
#endif
	ipri = KRPC_SVCBUFF_LOCK(ud);
	while (ud->ud_flags & UD_BUSY) {
		ud->ud_flags |= UD_WANTED;
		e_assert_wait(&(ud->ud_event), FALSE);
		KRPC_SVCBUFF_UNLOCK(ud, ipri);
		e_block_thread();
		ipri = KRPC_SVCBUFF_LOCK(ud);
	}
	ud->ud_flags |= UD_BUSY;
	KRPC_SVCBUFF_UNLOCK(ud, ipri);

	m = m_clattach(rpc_buffer(xprt), buffree, UDPMSGSIZE, (int)ud, M_WAIT);
	if (m == NULL) {
		buffree(rpc_buffer(xprt), UDPMSGSIZE, ud);
		return (stat);
	}

	xdrmbuf_init(&ud->ud_xdrout, m, XDR_ENCODE);
	msg->rm_xid = ud->ud_xid;
	if (xdr_replymsg(xdrs, msg)) {
		slen = (int)XDR_GETPOS(xdrs);
		if (m->m_next == 0) {		/* XXX */
			m->m_len = slen;
		}
		if (!ku_sendto_mbuf(xprt->xp_sock, m, 
				    &xprt->xp_raddr, othersock))
			stat = TRUE;
	} else {
		TRCHKL0(HKWD_NFS_RPC_DEBUG|hkwd_KRPC_SVCKUDP_SEND);
		m_freem(m);
	}
	/*
	 * This is completely disgusting.  If public is set it is
	 * a pointer to a structure whose first field is the address
	 * of the function to free that structure and any related
	 * stuff.  (see rrokfree in nfs_xdr.c).
	 */
	if (xdrs->x_public) {
		(**((int (**)())xdrs->x_public))(xdrs->x_public);
	}
#ifdef RPCDEBUG
	rpc_debug(5, "svckudp_send done\n");
#endif
	return (stat);
}

/*
 * Return transport status.
 */
/*ARGSUSED*/
enum xprt_stat
svckudp_stat(xprt)
	SVCXPRT *xprt;
{

	return (XPRT_IDLE); 
}

/*
 * Deserialize arguments.
 */
bool_t
svckudp_getargs(xprt, xdr_args, args_ptr)
	SVCXPRT	*xprt;
	xdrproc_t	 xdr_args;
	caddr_t		 args_ptr;
{

	return ((*xdr_args)(&(((struct udp_data *)(xprt->xp_p2))->ud_xdrin), args_ptr));
}

bool_t
svckudp_freeargs(xprt, xdr_args, args_ptr)
	SVCXPRT	*xprt;
	xdrproc_t	 xdr_args;
	caddr_t		 args_ptr;
{
	register XDR *xdrs =
	    &(((struct udp_data *)(xprt->xp_p2))->ud_xdrin);
	register struct udp_data *ud = (struct udp_data *)xprt->xp_p2;

	if (ud->ud_inmbuf) {
		m_freem(ud->ud_inmbuf);
	}
	ud->ud_inmbuf = (struct mbuf *)0;
	if (args_ptr) {
		xdrs->x_op = XDR_FREE;
		return ((*xdr_args)(xdrs, args_ptr));
	} else {
		return (TRUE);
	}
}

/*
 * the dup cacheing routines below provide a cache of non-failure
 * transaction id's.  rpc service routines can use this to detect
 * retransmissions and re-send a non-failure response.
 */

struct dupreq {
	struct dupreq	*forw_dr_chain;
	struct dupreq	*back_dr_chain;
	struct dupreq	*dr_next;
	u_long		dr_xid;
	struct sockaddr_in dr_addr;
	u_long		dr_proc;
	u_long		dr_vers;
	u_long		dr_prog;
	int		dr_status;	/* request status	*/
	char		dr_resp[1];	/* cached response	*/
};

/* 'Inlined' insque(),remque() routines */

#define DRINSQUE(member, queue) \
{ \
struct	dbl_q {struct dbl_q *forw; struct dbl_q *back;}; \
((struct dbl_q *)(member))->forw = ((struct dbl_q *)(queue))->forw; \
((struct dbl_q *)(member))->back = (struct dbl_q *)queue; \
((struct dbl_q *)(queue))->forw->back = (struct dbl_q *)member; \
((struct dbl_q *)(queue))->forw = (struct dbl_q *)member; \
}

#define DRREMQUE(member) \
{ \
struct	dbl_q {struct dbl_q *forw; struct dbl_q *back;}; \
((struct dbl_q *)(member))->back->forw = ((struct dbl_q *)(member))->forw; \
((struct dbl_q *)(member))->forw->back = ((struct dbl_q *)(member))->back; \
}
		 
/* DRUNHASH will remove a dupreq entry from a hash bucket.
 * Assume that if the forw chain pointer points to itself then it is the
 * only entry in the bucket.  This is derived from the fact that the
 * hash bucket is a doubly linked list and the first entry in the bucket
 * will be linked to itself.
 */
#define DRUNHASH(dr) \
{ \
if ((dr)->forw_dr_chain == (dr)) { \
	drhashtbl[DRHASH(dr)] = NULL; \
}else{ \
	if (drhashtbl[DRHASH(dr)] == (dr)) { \
		drhashtbl[DRHASH(dr)] = (dr)->forw_dr_chain; \
                DRREMQUE(dr); \
	}else{ \
		DRREMQUE(dr); \
	} \
} \
}

/*
 * MAXDUPREQS is the number of cached items.  It should be adjusted
 * to the service load so that there is likely to be a response entry
 * when the first retransmission comes in.
 */
#define	MAXDUPREQS	10000

#define	DUPREQSZ	(sizeof(struct dupreq) - 2*sizeof(caddr_t))
#define	DRHASHSZ	64
#define	XIDHASH(xid)	((xid) & (drhashsz-1))
#define	DRHASH(dr)	XIDHASH((dr)->dr_xid)
#define	REQTOXID(req)	((struct udp_data *)((req)->rq_xprt->xp_p2))->ud_xid

/* set this starting size here so that if the user sets the dupcache size 
 * before the NFS server gets its first request, then the allocation will work
 * correctly.  Can not allocate a dup cache free list without the size of
 * the dup cache entries and that is only supplied by the NFS server when
 * it calls the first time to save a dup request.  So the svckudp_dup_chgsize 
 * routine will be able to set this starting size and it will be used on 
 * the first allocation.
 */
static	int	starting_dupcache_size = NFS_DUPCACHE_SIZE_DEFAULT;
uint	krpc_maxdupreqs = 0;
int	drhashsz = DRHASHSZ;
int	ndupreqs = 0;
int	dupreqs = 0;
int	dupchecks = 0;
int	dupreplysize = 0;

struct dupreq **drhashtbl;

struct dupreq *dupreq_freelist = NULL;

struct dupreq *dupreq_allocbuckets = NULL;

/*
 * drmru points to the head of a circular linked list in lru order.
 */
struct dupreq *drmru;

Simple_lock krpc_drcache_lock;
#define KRPC_DR_CACHE_LOCK()	simple_lock(&krpc_drcache_lock)
#define KRPC_DR_CACHE_UNLOCK()	simple_unlock(&krpc_drcache_lock)

int
init_svckudp()
{
	lock_alloc(&krpc_drcache_lock, LOCK_ALLOC_PIN, 
		       KRPC_DRCACHE_LOCK, 0);
	simple_lock_init(&krpc_drcache_lock);

	/* allocate the hash chain based on the dupreq hash size
	   */
	drhashtbl = (struct dupreq **)
		kmem_alloc((u_int)(sizeof(struct dupreq *) * drhashsz));

	bzero((caddr_t)drhashtbl, (int)(sizeof(struct dupreq *) * drhashsz));

	drmru = NULL;

	return(0);
}

int
svckudp_dup_chgsize(u_int newsize)
{
	struct	dupreq	*pdr;
	int	newhashsize;
	int	size_diff;
	int	struct_size;
	int	i;
	caddr_t	ptmp;

	/* lock out all other nfs server operations on the dup cache
	   */
	KRPC_DR_CACHE_LOCK();

	if (newsize <= 0 || newsize > MAXDUPREQS ||
	    krpc_maxdupreqs >= newsize) {
		KRPC_DR_CACHE_UNLOCK();
		return(EINVAL);
	}

	/* check to see if the NFS server has made any dup save requests
	 * if not, then store the size of the dup cache and return.
	 */
	if (dupreplysize == 0) {
		krpc_maxdupreqs = starting_dupcache_size = newsize;
		KRPC_DR_CACHE_UNLOCK();
		return(0);
	}

	/* allocate memory for the new dup cache entries and place them
	   on the free list.
	   */
	size_diff = newsize - krpc_maxdupreqs;
	krpc_maxdupreqs = newsize;
	struct_size = RNDUP(sizeof(struct dupreq) + dupreplysize);

	if ((ptmp = (caddr_t)
	     kmem_alloc((size_diff + 1) * struct_size)) == NULL) {
		KRPC_DR_CACHE_UNLOCK();
		return(EIO);
	}

	/* place the entries on the free list after saving the pointer
	   to the head of the bucket of allocated dup req caches
	   */
	((struct dupreq *)ptmp)->dr_next = dupreq_allocbuckets;
	dupreq_allocbuckets = (struct dupreq *)ptmp;
	ptmp += struct_size;

	for(i = 0; i < size_diff; i++) {
		((struct dupreq *)ptmp)->dr_next = dupreq_freelist;
		dupreq_freelist = (struct dupreq *)ptmp;
		ptmp += struct_size;	/* go to next new entry */
	}

	/* figure out what the new hash bucket size will be based on the 
	   total number of entries in the duplicate cache.  keep it based
	   on a power of two so that the hash algorithm continues to work.
	   */
	#define HASH_DEPTH_TARGET	16

	{
		u_int i = (newsize / HASH_DEPTH_TARGET);
		u_int j;
		u_int k = (1 << ((NBPW * NBPB) - 1));

		/* search for the largest power of two
		   */
		for (j = 0 ; j < (NBPW * NBPB); j++) {
			if (k & i)
				break;
			k >>= 1;
		}
		newhashsize = (k < 64 ? 64 : k);
	}


	/* removed all of the entries in the dup cache from the hash buckets
	   only do this if we have actually allocated any
	   */

	if (drmru != NULL) {
		pdr = drmru;
		DRUNHASH(pdr);
		pdr->forw_dr_chain = pdr;
		pdr->back_dr_chain = pdr;
		pdr = pdr->dr_next;

		while (pdr != drmru) {
			DRUNHASH(pdr);
			pdr->forw_dr_chain = pdr;
			pdr->back_dr_chain = pdr;
			pdr = pdr->dr_next;
		}
	}

	/* clean up the hash table */
	bzero((caddr_t)drhashtbl, (int)(sizeof(struct dupreq *) * drhashsz));

	/* free the current hash bucket and reallocate based on the new size
	   */
	if (drhashsz != newhashsize) {
		caddr_t memtmp = (caddr_t)
			kmem_alloc((u_int)
				   (sizeof(struct dupreq *) * newhashsize));
		/* if the mem allocation did not fail, go ahead and
		   free the hold hash.  if it did fail then leave the old
		   one around with the old size.
		   */
		if (memtmp != NULL) {
			kmem_free(drhashtbl, 0 /*dummy arg*/);

			drhashsz = newhashsize;

			drhashtbl = (struct dupreq **)memtmp;
			bzero((caddr_t)drhashtbl, 
			      (int)(sizeof(struct dupreq *) * drhashsz));
		}
	}

	/* rebuild the hash buckets based on the new hash size, etc.
	   */
	if (drmru != NULL) {
		pdr = drmru;
		if (drhashtbl[DRHASH(pdr)] == NULL) {
			drhashtbl[DRHASH(pdr)] = pdr;
		}else{
			DRINSQUE(pdr, drhashtbl[DRHASH(pdr)]);
		}
		pdr = pdr->dr_next;

		while (pdr != drmru) {
			if (drhashtbl[DRHASH(pdr)] == NULL) {
				drhashtbl[DRHASH(pdr)] = pdr;
			}else{
				DRINSQUE(pdr, drhashtbl[DRHASH(pdr)]);
			}
			pdr = pdr->dr_next;
		}
	}

	KRPC_DR_CACHE_UNLOCK();

	return(0);
}
/*
 * scvkudp_dupsave searches the request cache and stores the
 * request.  If it already exists, then just mark it in progress.
 */
static struct dupreq *
svckudp_dupsave_req(req, size)
	register struct svc_req *req;
	register int size;
{
	register struct dupreq *dr;

	if (dupreq_freelist) {
		dr = dupreq_freelist;
		dupreq_freelist = dr->dr_next;

		dr->dr_status = 0;

		if (drmru) {
			dr->dr_next = drmru->dr_next;
			drmru->dr_next = dr;
		} else {
			dr->dr_next = dr;
		}
		ndupreqs++;
	} else {
		dr = drmru->dr_next;
		DRUNHASH(dr);

		/* check to see if the dup cache should shrink
		   if so, remove one extra and free associated memory
		   */
		if (ndupreqs > krpc_maxdupreqs) {
			drmru->dr_next = dr->dr_next;
			bzero(dr->dr_resp, size);
			kmem_free(dr->dr_resp, size);
			bzero(dr, sizeof(*dr));
			kmem_free(dr, sizeof(*dr));

			dr = drmru->dr_next;
			DRUNHASH(dr);
		}
	}

	drmru = dr;

	/* make sure the dr chain is set up correctly
	   */
	dr->forw_dr_chain = dr;
	dr->back_dr_chain = dr;

	dr->dr_status = DUP_INPROGRESS;
	dr->dr_xid = REQTOXID(req);
	dr->dr_prog = req->rq_prog;
	dr->dr_vers = req->rq_vers;
	dr->dr_proc = req->rq_proc;
	dr->dr_addr = req->rq_xprt->xp_raddr;

	/* get the hash chain set up
	   */
	if (drhashtbl[DRHASH(dr)] == NULL) {
		drhashtbl[DRHASH(dr)] = dr;
	}else{
		DRINSQUE(dr, drhashtbl[DRHASH(dr)]);
	}
	return(dr);
}

/*
 * svckudp_dup searches the request cache and returns 0 if the
 * request is not found in the cache.  If it is found, then it
 * returns the state of the request (in progress or done) and
 * the status or attributes that were part of the original reply.
 */
svckudp_dupsave(req, res, size, maxsize, saved_dupreq)
	register struct svc_req *req;
	caddr_t	res;
	register int size;
	register int maxsize;
	caddr_t	*saved_dupreq;
{
	register struct dupreq *dr;
	u_long xid;
	 
	if (dupreplysize == 0) {
		dupreplysize = maxsize;
		/* reset so allocation works correctly*/
		krpc_maxdupreqs = 0;
		svckudp_dup_chgsize(starting_dupcache_size);
	}

	KRPC_DR_CACHE_LOCK();

	dupchecks++;
	xid = REQTOXID(req);
	dr = drhashtbl[XIDHASH(xid)]; 
	while (dr != NULL) { 
		if (dr->dr_xid != xid ||
		    dr->dr_prog != req->rq_prog ||
		    dr->dr_vers != req->rq_vers ||
		    dr->dr_proc != req->rq_proc ||
		    bcmp((caddr_t)&dr->dr_addr,
		     (caddr_t)&req->rq_xprt->xp_raddr,
		     sizeof(dr->dr_addr)) != 0) {
			/* check to see if we are done with this hash queue
			   */
			if (drhashtbl[XIDHASH(xid)] == dr->forw_dr_chain)
				break;
			else
				dr = dr->forw_dr_chain;
		} else {
			int	status;

			dupreqs++;
			/*
			 * Should probably guard against zeroing
			 * res in case there isn't a saved response
			 * for some reason.
			 */
			if ((dr->dr_resp) && (size))
				bcopy(dr->dr_resp, res, (u_int)size);
			status = dr->dr_status;
			saved_dupreq = (caddr_t *)NULL;
			KRPC_DR_CACHE_UNLOCK();
			return (status);
		}
	}
	/* Save the request for the caller */
	*saved_dupreq = (caddr_t)svckudp_dupsave_req(req, maxsize);
	KRPC_DR_CACHE_UNLOCK();
	return (DUP_ADDED);
}

/*
 * svckudp_dupdone searches the request cache and marks the
 * request done (DUP_SUCCESS or DUP_FAILED) and stores the
 * response.  If the request is not found, then the cache is
 * too small!
 */
svckudp_dupdone(req, res, size, dup)
	register struct svc_req *req;
	register caddr_t res;
	register int	size;
	caddr_t dup;
{
	u_long xid;
	register struct dupreq *dr = (struct dupreq *)dup;

	KRPC_DR_CACHE_LOCK();

	xid = REQTOXID(req);

	if (dr->dr_xid != xid ||
	    dr->dr_prog != req->rq_prog ||
	    dr->dr_vers != req->rq_vers ||
	    dr->dr_proc != req->rq_proc ||
	    bcmp((caddr_t)&dr->dr_addr,
		 (caddr_t)&req->rq_xprt->xp_raddr,
		 sizeof (dr->dr_addr)) != 0) {
		KRPC_DR_CACHE_UNLOCK();
		/*
		 * XXX - if not found, then should warn that maybe there's not
		 * enough dup req cache space !
		 */
		TRCHKL0(HKWD_NFS_RPC_DEBUG|hkwd_KRPC_SVCKUDP_DUPNUMBER);
		return;
	}

	/* dup entry matches - copy res (if applicable) */
	dr->dr_status = DUP_DONE;
	if ((res != (caddr_t)0) && (size != 0)) {
		/*
		 * should test if resp == NULL first
		 */
		bcopy(res, dr->dr_resp, (u_int)size);
	}
	KRPC_DR_CACHE_UNLOCK();
	return;
}
#endif /* _KERNEL */
