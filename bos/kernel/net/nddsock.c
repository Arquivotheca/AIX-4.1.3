static char sccsid[] = "@(#)13	1.9  src/bos/kernel/net/nddsock.c, sysnet, bos411, 9433A411a 8/12/94 11:00:19";
/*
 *   COMPONENT_NAME: SYSNET
 *
 *   FUNCTIONS: ndd_control
 *		ndd_output
 *		ndd_usrreq
 *		nddintr
 *		nddisr
 *		nddsock_init
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/device.h>
#include <sys/domain.h>		/* for struct domain */
#include <sys/socket.h>
#include <sys/protosw.h>
#include <sys/errno.h>
#include <sys/mbuf.h>
#include <sys/socketvar.h>
#include <sys/syspest.h>
#include <sys/lockl.h>

#include <sys/malloc.h>

#include <net/raw_cb.h>
#include <sys/cdli.h>
#include <sys/ndd.h>
#include <net/nd_lan.h>
#include <sys/ndd_var.h>
#include <net/netisr.h>

int	ndd_output();
void 	nddintr();
int 	ndd_usrreq();
extern struct domain ndddomain;

/*
 * nddintr() schedules nddisr() via the netisr queue code.
 */
struct ifqueue nddisrq;
int nddqmaxlen=50;

void nddisr();

struct protosw nddsw[] = {
{ SOCK_DGRAM,	&ndddomain,	0,		PR_ATOMIC,
  0,	ndd_output,	0,		0,
  ndd_usrreq,		0,		0,
  0,	0,		0,		0,
}
};

struct domain ndddomain =
    { AF_NDD, "nddnet", 0, 0, 0, 
      nddsw, &nddsw[sizeof(nddsw)/sizeof(nddsw[0])],
      0, 0, 0, 0 };

nddsock_init()
{
	domain_add(&ndddomain);
	IFQ_LOCKINIT(&nddisrq);
	nddisrq.ifq_maxlen = nddqmaxlen;
	(void) netisr_add(NETISR_NDD, nddisr, &nddisrq, &ndddomain);
}

void
ndd_clean_isrq(struct socket *so)
{
	IFQ_LOCK_DECL()
	struct mbuf *m;
	struct ifqueue tmpq;
	struct socket *tmpso;

	bzero((caddr_t)&tmpq, sizeof(tmpq));
	tmpq.ifq_maxlen = nddqmaxlen;

	IFQ_LOCK(&nddisrq);
	do {
		IF_DEQUEUE_NOLOCK(&nddisrq, m);
		if (m) 
			IF_ENQUEUE_NOLOCK(&tmpq, m);
	} while (m);
	do {
		IF_DEQUEUE_NOLOCK(&tmpq, m);	
		if (m) {
			tmpso = (struct socket *)m->m_pkthdr.rcvif;
			
			if (tmpso == so)
				m_freem(m);
			else
				IF_ENQUEUE_NOLOCK(&nddisrq, m);
		}
	
	} while (m);
	IFQ_UNLOCK(&nddisrq);
}


/*ARGSUSED*/
ndd_usrreq(so, req, m, nam, control)
	struct socket *so;
	int req;
	struct mbuf *m, *nam, *control;
{
	register int error = 0;
	register struct raw_nddpcb *rp = sotorawnddpcb(so);

	LOCK_ASSERT("ndd_usrreq", SOCKET_ISLOCKED(so));

	switch (req) {
	case	PRU_ATTACH:
		assert(rp == NULL);

		NET_MALLOC(rp, struct raw_nddpcb *, sizeof *rp, M_PCB, 
			M_WAITOK);
		bzero((caddr_t)rp, sizeof *rp);
		so->so_pcb = (caddr_t)rp;
		break;

	case	PRU_DETACH:
		{
		struct sockaddr_ndd *addr = &rp->rndd_laddr;
		
		assert(rp != NULL);

		if (rp->rndd_recvnddp != NULL) {
			if (rp->rndd_rcb.rcb_laddr != NULL) {
				assert(ns_del_filter(rp->rndd_recvnddp, 
					addr->sndd_data, 
					addr->sndd_filterlen) == 0);
				ndd_clean_isrq(so);
			}
			ns_free(rp->rndd_recvnddp);
		}
		break;
		}

	case	PRU_BIND:
		{
		struct sockaddr_ndd *addr = mtod(nam, struct sockaddr_ndd *);
		struct ns_user ns_user;

		if (addr->sndd_nddname == NULL)
			return(EINVAL);
		if (addr->sndd_filterlen < 0 || 
		    addr->sndd_filterlen > SNDD_MAXFILTER)
			return(EINVAL);

		if (rp->rndd_rcb.rcb_laddr)
			return(EEXIST);

		if (rp->rndd_recvnddp == NULL)
			error = ns_alloc(addr->sndd_nddname, &rp->rndd_recvnddp);
	
		if (error)
			return(error);

		/* XXX - wait for status block for open done? */

		ns_user.isr = nddintr;
		ns_user.protoq = (struct ifqueue *) NULL;
		ns_user.netisr = 0;
		ns_user.pkt_format = NS_INCLUDE_MAC;
		ns_user.isr_data = (caddr_t) so;
		ns_user.ifp = (struct ifnet *) NULL;
		if (error = ns_add_filter(rp->rndd_recvnddp, addr->sndd_data, 
		      addr->sndd_filterlen, &ns_user)) {
			ns_free(rp->rndd_recvnddp);
			rp->rndd_recvnddp = NULL;
			return(error);
		}

		rp->rndd_rcb.rcb_laddr = (struct sockaddr *)&rp->rndd_laddr;
		rp->rndd_laddr = *addr;
		return(0);
		}
	case	PRU_CONNECT:
		{
		struct sockaddr_ndd *addr = mtod(nam, struct sockaddr_ndd *);

		if (rp->rndd_rcb.rcb_faddr)
			return(EISCONN);

		if (rp->rndd_recvnddp == NULL)
			error = ns_alloc(addr->sndd_nddname, &rp->rndd_recvnddp);
	
		if (error)
			return(error);

		/* XXX - wait for status block for open done? */

		rp->rndd_rcb.rcb_faddr = (struct sockaddr*)&rp->rndd_faddr;
		rp->rndd_faddr = *addr;
		soisconnected(so);
		return(0);
		}
	case	PRU_CONTROL:
		return(ndd_control(so, (int)m, (caddr_t)nam, rp->rndd_recvnddp
));
	default:
		break;
	}
	error =  raw_usrreq(so, req, m, nam, control);

	if (error && (req == PRU_ATTACH) && so->so_pcb)
		NET_FREE(so->so_pcb, M_PCB);
	return(error);
}

/*
 * Generic control operations (ioctl's).
 */
/* ARGSUSED */
ndd_control(so, cmd, data, nddp)
	struct socket *so;
	int cmd;
	caddr_t data;
	register struct ndd *nddp;
{
	struct nddctl *nddctl = (struct nddctl *)data;
	int length;
	caddr_t	bufaddr;
	caddr_t buf;
	int error = 0;

	if (data == NULL) {
		if (nddp != (struct ndd *)NULL)
			return((*nddp->ndd_ctl)(nddp, cmd, NULL, 0));
		else
			return(ENOTCONN);
	}
	

	if ( copyin((int *)&nddctl->nddctl_buflen, &length, sizeof(length)) )
		return EFAULT;

	if (length < 0)
		return(EINVAL);
	
	if ( copyin((int *)&nddctl->nddctl_buf, &bufaddr, sizeof(bufaddr)) )
		return EFAULT;

	NET_MALLOC(buf, caddr_t, length, M_TEMP, M_WAITOK);

	if ( copyin((int *)bufaddr, buf, length) ) {
		NET_FREE(buf, M_TEMP);
		return EFAULT;
	}

	if (nddp != (struct ndd *)NULL) {
		error = (*nddp->ndd_ctl)(nddp, cmd, buf, length);
		if ( copyout(buf, (int *)bufaddr, length) )
			error = EFAULT;
	} else
		error = ENOTCONN;

	NET_FREE(buf, M_TEMP);
	return(error);
}

/*
 * nddintr -	NDD interrupt handler
 *
 */
void
nddintr(nddp, m, macp, so)
	register struct ndd	*nddp;
	register struct mbuf	*m;
	caddr_t			macp;
	struct socket		*so;
{
	struct raw_nddpcb 		*rp;
	struct sockaddr_ndd_8022 	*sa;
	struct mbuf			*n;

	IFQ_LOCK_DECL()

	/*
	 * For TAP users, we must copy the mbuf.  If this fails, we
	 * bail...
	 */
	rp = (struct raw_nddpcb *)(so->so_pcb);
	sa = (struct sockaddr_ndd_8022 *)(&rp->rndd_laddr);
	if (sa->sndd_8022_filtertype == NS_TAP) {
		n = m_copym(m, 0, M_COPYALL, M_DONTWAIT);
		if (n == NULL)
			return;
		m = n;
	}

	/* 
	 * Save the socket ptr in the rcvif field of the mbuf so the
	 * ISR handler can use it.
	 */
	m->m_pkthdr.rcvif = (struct ifnet *)so;

	IFQ_LOCK(&nddisrq);
	if (IF_QFULL(&nddisrq)) {
		IF_DROP(&nddisrq);
		m_freem(m);
	} else {
		IF_ENQUEUE_NOLOCK(&nddisrq, m);
	}
	IFQ_UNLOCK(&nddisrq);
	schednetisr(NETISR_NDD);
}

void
nddisr()
{
	IFQ_LOCK_DECL()
	struct mbuf *m;
	struct socket *so;

	do {
		IF_DEQUEUE(&nddisrq, m);
		if (m) {	
			so = (struct socket *)m->m_pkthdr.rcvif;
			SOCKET_LOCK(so);
			SOCKBUF_LOCK(&so->so_rcv);
			if (m->m_pkthdr.len > sbspace(&so->so_rcv))
			       m_freem(m);
			else {
			       sbappendrecord(&so->so_rcv, m);
			       sorwakeup(so);
			}
			SOCKBUF_LOCK(&so->so_rcv);
			SOCKET_UNLOCK(so);
		}
	} while (m);
}

/*
 * 	RAW NDD output
 */
ndd_output(m0, so)
	struct mbuf *m0;
	struct socket *so;
{
	int error=0;
	struct mbuf *m;
	struct ndd *nddp;
	struct raw_nddpcb *rp = sotorawnddpcb(so);

	LOCK_ASSERT("ndd_output", SOCKET_ISLOCKED(so));

	nddp = rp->rndd_recvnddp;
	if (nddp == (struct ndd *) NULL) {
		error = ENOTCONN;
		goto bad;
	}

	for (m = m0; m; m = m0) {
		m0 = m->m_nextpkt;
		m->m_nextpkt = 0;
		if (error == 0) {
			if ( (m->m_pkthdr.len < nddp->ndd_mintu) &&
			     (m->m_pkthdr.len > nddp->ndd_mtu) )
				error = EMSGSIZE;
			else
				error = (*nddp->ndd_output)(nddp, m);
			if (error)
				m_freem(m);
		} else
			m_freem(m);
	}
	return (error);
bad:
	m_freem(m0);
	return (error);
}
