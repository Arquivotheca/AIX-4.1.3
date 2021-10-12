static char sccsid[] = "@(#)97	1.36  src/bos/kernel/net/net_if.c, sysnet, bos41J, 9513A_all 3/22/95 10:37:18";
/*
 *   COMPONENT_NAME: SYSNET
 *
 *   FUNCTIONS: net_attach
 *		net_detach
 *		net_error
 *		net_queued_write
 *		net_start
 *		net_start_done
 *		net_xmit
 *		net_xmit_trace
 *		net_xmit_trace_cdli
 *		net_recv_trace_cdli
 *		
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/************************************************************************
*
* net_if.c - utilities for BSD network interfaces on comio net devices
*
************************************************************************/
#include	<sys/types.h>
#include 	<sys/param.h>
#include 	<sys/user.h>
#include        <sys/errno.h>
#include        <sys/ioctl.h>
#include	<sys/mbuf.h>
#include	<sys/file.h>
#include	<sys/uio.h>
#include	<sys/device.h>
#include	<sys/sleep.h>
#include	<sys/socket.h>
#include	<sys/nettrace.h>
#include	<sys/comio.h>
#include	<sys/ciouser.h>
#include 	<sys/lock_alloc.h>

#include	<net/if.h>
#include	<net/netisr.h>

#include        <sys/cdli.h>
#include        <sys/ndd.h>

#include	<aixif/net_if.h>

#include	<sys/devinfo.h>		/* define last, so IFF_UP not redef'd */

struct ifqueue writeq = {NULL, NULL, 0, IFQ_MAXLEN, 0};

extern int wildcard_in_table;

void
net_start_timeout(struct trb *trb)
{
        e_clear_wait((tid_t)trb->func_data, THREAD_TIMED_OUT);
};

/*
 * NAME:  net_start
 *
 * FUNCTION:  starts netids on an AIX comio style device handler.
 *
 * RETURNS:
 *	0          - ok
 *	ETIMEDOUT  - netid start(s) timed out
 *	returns from device handler fp_ioctl(CIO_START)
 */
net_start(netfpp, netid)
	struct file **netfpp;
	struct netid_list *netid;
{
	int			rc = 0;
	struct session_blk	session_blk;	/* data for CIO_START	*/
	int			i;
	int			x;
	struct trb 		*trb;
	struct thread 		*t = curthread;

	trb = talloc();
	if (trb == NULL)
		return(ENOBUFS);

        trb->timeout.it_value.tv_sec = 60;
        trb->timeout.it_value.tv_nsec = 0;
	trb->func       =  net_start_timeout;
	trb->eventlist  =  EVENT_NULL;
	trb->func_data  =  t->t_tid;
	trb->ipri       =  INTCLASS2;

	/*
	 *	loop and issue CIO_START for all requested netids.
	 */
	netid->start_complete = ETIMEDOUT;
	netid->complete_count = 0;
	netid->attach_snooze = EVENT_NULL;
	e_assert_wait((int *)&netid->attach_snooze, 0);
	for ( i = 0 ; i < netid->id_count ; i++ ) {
		session_blk.netid = netid->id[i];
		session_blk.length = netid->id_length;
		if (rc = fp_ioctl(*netfpp, CIO_START, &session_blk, NULL)) {
			e_clear_wait(t->t_tid, THREAD_OTHER);
			net_error(0, session_blk.status, *netfpp);
			tfree(trb);
			return(rc);
		}
	}

	tstart(trb);
	(void) e_block_thread();

#ifndef _POWER_MP
	tstop(trb);
#else
	while (tstop(trb));
#endif
	tfree(trb);
	if (netid->start_complete && netid->id_count)
		return(netid->start_complete);

	return(rc);
}

/******************************************************************
 *
 * net_attach() -   open a aix comio style device driver
 *		    This routine will fp_opendev() the specified
 *		    device and then start it. The file pointer
 *		    returned by fp_opendev() will be saved in softc
 *		    for later use.
 *
 *****************************************************************/
net_attach(kopen_ext, device, netid, netfpp)
register struct kopen_ext	*kopen_ext;
register struct	device_req	*device;
register struct netid_list	*netid;
struct	file			**netfpp;
{
	int			rc;

	/* 
	 *	First open the device driver. major, minor is passed in.
	 */
	rc = fp_opendev(device->devno, DREAD|DWRITE|FNDELAY, NULL, 
		        kopen_ext, netfpp);
	if ( rc != 0)
		return(rc);

	rc = net_start(netfpp, netid);
	if (rc != 0) {
		fp_close(*netfpp);
		*netfpp = (struct file *) NULL;
	}

	return(rc);
}

/************************************************************************
*	net_start_done() - handle start done notification for comio 	*
*			   devices. If completions for all starts	*
*			   have been received or if an error occurred, 	*	
*			   wakeup sleeper.				*
************************************************************************/
net_start_done(netid, sbp)
register struct netid_list	*netid;
register struct status_block 	*sbp;
{
	int	x;

	if (sbp->option[0] != CIO_OK) {		/* start failed		*/
		netid->start_complete = EIO;
		e_wakeup((int *)&(netid->attach_snooze)); /* no sense waiting*/
		net_error(0, sbp->option[0], 0);
	} else {
		if (++netid->complete_count >= netid->id_count) {
			/* all the cows came home. tell sleeper of success */
			netid->start_complete = 0;
			e_wakeup((int *)&(netid->attach_snooze));
		}
	}
	return(0);
}

/************************************************************************
*	net_xmit() - transmit a packet on interface			*
*		     device driver will free mbuf			*
************************************************************************/
net_xmit(ifp, m, netfp, lngth, m_extp)
struct	ifnet	*ifp;
struct	mbuf	*m;
struct	file	*netfp;
int	lngth;		/* SNMP BASE */
struct	mbuf	*m_extp;
{
	struct uio			uio;
	struct iovec			iovec;
	int				rc = 0;
	struct ifqueue			*inq;
	extern	struct ifqueue		writeq;	/* delayed write queue	*/
	struct	mbuf			*m0;
	struct 	qwrite			*qp;	/* mbuf data pointer	*/
	int				ext;
	IFQ_LOCK_DECL()
	LOCK_ASSERTL_DECL

	/* First check to see if we're a process. If we're not we can't
	 * do a devwrite. In this case we enqueue the mbuf and wakeup
	 * our kproc so that it will issue the devwrite
	 */
	if ( (getpid() == -1) || ifp->devno == (dev_t)NULL ) {
		M_PREPEND(m,sizeof(struct qwrite),M_DONTWAIT);
		if (m == NULL)	
			goto out;

		qp = mtod(m, struct qwrite *);
		qp->ifp = ifp;
		qp->netfp = netfp;
		qp->length = lngth;
		qp->ext = m_extp;
		if (m_extp != (struct mbuf *)NULL)
			m_extp->m_len = 1;
		inq = &writeq;			/* delayed write queue	*/
		IFQ_LOCK(inq);
	  	if (IF_QFULL(inq)){
			IF_DROP(inq);
			m_freem(m);
        		if (m_extp != (struct mbuf *)NULL)
                		m_freem(m_extp);
		} else {
			IF_ENQUEUE_NOLOCK(inq, m);
			schednetisr(NETISR_WRITE);
		}
		IFQ_UNLOCK(inq);
		return(0);
	}

	if (wildcard_in_table)
		net_xmit_trace(ifp, m);

	uio.uio_iov = &iovec;
	uio.uio_iovcnt = 1;		/* no gather			*/
	uio.uio_offset = 0;
	uio.uio_segflg = UIO_SYSSPACE;	/* from kernel			*/
	uio.uio_resid = sizeof(m);
	uio.uio_fmode = FNDELAY;
	iovec.iov_base = (caddr_t)m;
	iovec.iov_len = sizeof(m);
	if (m_extp != (struct mbuf *)NULL)
		ext = mtod(m_extp, int);
	else
		ext = '\0';

	if ((rc = devwrite(ifp->devno, &uio, ifp->chan, ext)) != 0) {
		net_error(ifp, NET_XMIT_FAIL, rc);	
		/* driver does not free mbuf on errors so free it */
		m_freem(m);
	}

out:
	if (m_extp != (struct mbuf *)NULL)
		m_freem(m_extp);	

	return(rc);
}


/************************************************************************
 * net_detach() -  detach a device driver	
 *
 * Input:
 *	  netfp	- network file pointer	
 ***********************************************************************/
net_detach(netfp)
register struct	file		*netfp;
{
	return(fp_close(netfp));
}

/************************************************************************
 * net_queued_write() -  issue a write after dequeuing a request 	
 *
 *	devwrite's are only allowed from processes. Because of this
 *	net_xmit must check to see if caller is a process. If so
 *	the write request is queued and the network kproc scheduled.
 *	The kproc wakes up and calls this function so that it can call
 *	net_xmit again (this time as a process). 
 *	Hard to believe, but true.
 *
 * Input:
 *	  
 ***********************************************************************/
net_queued_write() {
	struct mbuf	*m;		/* mbuf chain to transmit	*/
	struct 	qwrite	*qp;		/* mbuf data pointer		*/
	struct	ifnet	*ifp;
	struct	file	*netfp;
	int		lngth;		/* SNMP BASE */
	struct	mbuf	*m_extp;
	LOCK_ASSERTL_DECL
	IFQ_LOCK_DECL()

	IF_DEQUEUE(&writeq, m);
	while (m) {
		qp = mtod(m, struct qwrite *);
		ifp = qp->ifp;
		netfp = qp->netfp;
		lngth = qp->length;
		m_extp = qp->ext;
		m_adj(m, sizeof(struct qwrite));
		if (m->m_len == 0)	/* get rid of zero length mbuf	*/
			m = m_free(m);
		if (ifp->devno == (dev_t)NULL)
			fp_getdevno(qp->netfp, &ifp->devno, &ifp->chan);
		net_xmit(ifp, m, netfp, lngth, m_extp);
		IF_DEQUEUE(&writeq, m);
	}
	return(0);
}

net_xmit_trace(ifp, m)
struct ifnet		*ifp;
struct mbuf		*m;
{
	if (!wildcard_in_table)
		return;

	wild_type_dispatch(0, m, (struct arpcom *)ifp, NULL, 1);
}

net_xmit_trace_cdli(nddp, m, hp, arg)
struct	ndd	*nddp;
struct	mbuf	*m;
caddr_t		hp;
caddr_t		arg;
{
	if (!wildcard_in_table)
		return;

	wild_type_dispatch(0, m, (struct arpcom *)arg, NULL, 1);
}

net_recv_trace_cdli(nddp, m, hp, arg)
struct	ndd	*nddp;
struct	mbuf	*m;
caddr_t		hp;
caddr_t		arg;
{
	if (!wildcard_in_table)
		return;

	wild_type_dispatch(0, m, (struct arpcom *)arg, NULL, 0);
}

/************************************************************************
*	net_error() -  generic error handling for network       	*
*		       interface drivers.                       	*
************************************************************************/
net_error(ifp, error_code, netfp)
struct	ifnet	*ifp;		/* interface ptr		 */
int error_code;			/* error code			 */
struct file 	*netfp;		/* file pointer of device driver */
{
	/* trace the error */
	TRCHKL1(HKWD_NETERR|error_code, ifp);
	bsdlog(0, "net_error on %s%d : error = x%x\n", ifp->if_name, 
		ifp->if_unit, error_code);
}
