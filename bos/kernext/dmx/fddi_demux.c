static char sccsid[] = "@(#)22	1.18  src/bos/kernext/dmx/fddi_demux.c, sysxdmx, bos41J, 9514A_all 4/4/95 18:51:19";
/*
 *   COMPONENT_NAME: SYSXDMX
 *
 *   FUNCTIONS: fddi_dmx_add_demuxer
 *		fddi_dmx_add_filter
 *		fddi_dmx_add_mac
 *		fddi_dmx_add_smt
 *		fddi_dmx_add_status
 *		fddi_dmx_add_tap
 *		fddi_dmx_config
 *		fddi_dmx_del_filter
 *		fddi_dmx_del_mac
 *		fddi_dmx_del_smt
 *		fddi_dmx_del_status
 *		fddi_dmx_del_tap
 *		fddi_dmx_init
 *		fddi_dmx_mac_receive
 *		fddi_dmx_receive
 *		fddi_dmx_response
 *		fddi_dmx_smt_receive
 *		fddi_dmx_status
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/syspest.h>
#include <sys/uio.h>
#include <sys/device.h>
#include <sys/mbuf.h>
#include <sys/cdli.h>
#include <sys/time.h>
#include <net/net_globals.h>
#include <net/net_malloc.h>
#include <net/nd_lan.h>
#include <net/netisr.h>
#include <net/if_arp.h>
#include <netinet/if_fddi.h>
#include <sys/ndd.h>
#include <net/spl.h>
#include <sys/fddi_demux.h>

struct	ns_demuxer	demuxer;		/* Global demuxer struct */
int                     fddi_users;		/* Current # of users */

struct	arptab		*fddi_arptabp;		/* Default arp table */
struct	trb		*fddi_arptime = NULL;
int 	fddi_arptabbsiz;
int 	fddi_arptabnb;
#define FDDI_ARPTAB_HASH(a) ((u_long) (a) % fddi_arptabnb)
#if	NETSYNC_LOCK
Simple_lock	fddi_arptab_lock;
#endif

/*
 * The following lock is used to serialize:
 * 	Access to the static variable initialized (in fddi_dmx_config).
 *	Access to fddi_users.
 */
lock_t			cfg_lock = LOCK_AVAIL;

/*
 * Forward declarations.
 */
void fddi_dmx_mac_receive(struct ndd *, struct mbuf *, struct fddi_mac_hdr *);
void fddi_dmx_smt_receive(struct ndd *, struct mbuf *, struct fddi_mac_hdr *);
int fddi_arp_init(void);
void fddi_arptimer(void);

#ifdef DEBUG
#define DBGMSG(m) bsdlog(0, m)
#else
#define DBGMSG(m)
#endif

/*
 * NAME: fddi_dmx_config
 *                                                                    
 * FUNCTION: 
 * 	Kernel extension entry point to configure the fddi demuxer.
 *                                                                    
 * EXECUTION ENVIRONMENT: process
 *                                                                   
 * NOTES: 
 * 	Can handle more than one CFG_INIT call (noops after the first).
 *	Does NOTHING for CFG_TERM.
 *
 * RECOVERY OPERATION: 
 *      None.
 *
 * DATA STRUCTURES: 
 *      NDD struct (ndd_t)
 *      NS status user struct (ns_user_t)
 *      Ethernet demuxer control struct (struct fddi_dmx_ctl) - internal struct
 *
 * ROUTINES CALLED:
 *	pincode
 *	fddi_dmx_add_demuxer
 *
 * RETURNS:  
 *              0 - success
 *		EINVAL - invalid configure command
 *		rc from pincode
 *		rc from fddi_dmx_add_demuxer
 */  
fddi_dmx_config(cmd, uio)
int		cmd;
struct uio	*uio;
{
	int				error=0;
	static int 			initialized=0;
	int				s;

	if ( ((cmd != CFG_INIT) && (cmd != CFG_TERM)) )
		return(EINVAL);

	lockl(&cfg_lock, LOCK_SHORT);
	if (cmd == CFG_INIT) {
		DBGMSG("FDDI_DMX: CFG_INIT called\n");
		if (initialized++ == 0) {
			if (!(fddi_arptime = talloc()))
				error = ENOMEM;
			if (!error)
				error = pincode(fddi_dmx_config);
			if (!error) {
				DBGMSG("FDDI_DMX: pinned code ok\n");
				error = fddi_dmx_add_demuxer();
				if (error) {
					DBGMSG("FDDI_DMX: failed to add demuxer\n");
					unpincode(fddi_dmx_config);
					initialized--;
				}
			} else
				initialized--;
			if (!initialized && fddi_arptime)
				tfree (fddi_arptime);
		}
	} 
	else {
		DBGMSG("FDDI_DMX: CFG_TERM called \n");
		if (initialized == 1) {
			if (fddi_users == 0) {
				DBGMSG("FDDI_DMX: final TERM/no users, removing demuxer\n");
				error = ns_del_demux(NDD_FDDI);
				if (!error) {
					while (tstop(fddi_arptime));
					ARPTAB_LOCK(&fddi_arptab_lock);
					if (fddi_arptabp)
						NET_FREE(fddi_arptabp, 
							M_KTABLE);
					fddi_arptabp = (struct arptab *)NULL;
					ARPTAB_UNLOCK(&fddi_arptab_lock);
					lock_free(&fddi_arptab_lock);
					initialized = 0;
					DBGMSG("unpinned code\n");
					unpincode(fddi_dmx_config);
					if (fddi_arptime)
						tfree(fddi_arptime);
				}
			} else
				error = EBUSY;
		} else if (initialized > 1)
			initialized--;
		else {
			error = EINVAL;
			DBGMSG("FDDI_DMX: Bogus TERM...initialized <=0\n");
		}
	}
	unlockl(&cfg_lock);
	return(error);
}


/*
 * NAME: fddi_dmx_add_filter
 *                                                                    
 * FUNCTION: 
 *      Process user request for adding filters.
 *                                                                    
 * EXECUTION ENVIRONMENT: process
 *                                                                   
 * NOTES: 
 *      The input for this routine is:
 *              struct ndd      *nddp
 *              ns_8022_t       p_filter
 *              int             len
 *              struct ns_user  *p_user
 *
 *      This routine will handle the following types of filters:
 *              NS_TAP
 *              NS_8022_LLC_DSAP
 *              NS_8022_LLC_DSAP_SNAP
 *              FDDI_DEMUX_MAC
 *		FDDI_DEMUX_SMT_NSA
 *		FDDI_DEMUX_SMT
 *
 * RECOVERY OPERATION: 
 *      None.
 *
 * DATA STRUCTURES: 
 *      Common filter struct (ns_8022_t)
 *      NDD struct (ndd_t)
 *      NS user struct (ns_user_t)
 *
 * ROUTINES CALLED:
 *      Common 802.2 add filter (dmx_8022_add_filter )
 *      Common 802.2 delete filter (dmx_8022_del_filter )
 *      NDD ctl function with NDD_ADD_FILTER operation
 * 	fddi_dmx_add_mac
 *	fddi_dmx_add_tap
 *	fddi_dmx_add_smt
 *	fddi_dmx_init
 *
 * RETURNS:  
 *              0       - successful
 *              ENOMEM  - Unable to allocate required memory
 *              EEXIST  - Filter already in use
 *              EINVAL  - Invalid filter type
 *              rc      - return codes from dmx_8022_add_filter()
 *              rc      - return codes from NDD's NDD_ADD_FILTER operation
 *              rc      - return codes from fddi_dmx_add_smt
 *              rc      - return codes from fddi_dmx_add_tap
 */  
fddi_dmx_add_filter(nddp, filter, len, ns_user)
	struct ndd		*nddp;
	struct ns_8022		*filter;
	int			len;
	struct ns_user		*ns_user;
{
	int 	error;
	DEMUXER_LOCK_DECL();

	if (ns_user->isr == NULL && ns_user->netisr == 0)
		return(EINVAL);
	if (ns_user->protoq != NULL && ns_user->netisr == 0)
		return(EINVAL);
	if (len != sizeof(struct ns_8022) && len != sizeof(fddi_dmx_filter_t))
		return(EINVAL);

	DEMUXER_LOCK(&nddp->ndd_demux_lock);

	/*
	 * If this is the first add, then initialize the FDDI control
	 * structure.
	 */
	if (nddp->ndd_specdemux == (caddr_t) NULL) {
		if (fddi_dmx_init(nddp)) {
			DEMUXER_UNLOCK(&nddp->ndd_demux_lock);
			return(ENOMEM);
		}
	}
	switch (filter->filtertype) {
		case NS_8022_LLC_DSAP:
		case NS_8022_LLC_DSAP_SNAP:
			error = dmx_8022_add_filter(nddp, filter, ns_user);
			break;

		case FDDI_DEMUX_SMT_NSA:
		case FDDI_DEMUX_SMT:
			error = fddi_dmx_add_smt(nddp, ns_user, 
				(fddi_dmx_filter_t *)filter);
			break;

		case FDDI_DEMUX_MAC:
			error = fddi_dmx_add_mac(nddp, ns_user);
			break;

		case NS_TAP:
			error = fddi_dmx_add_tap(nddp, ns_user);
			break;

		default:
			error = EINVAL;
			break;
	}

	/*
	 *  If everything worked okay then call the NDD's control function.
	 */
	if ((!error) && (filter->filtertype != NS_TAP)) {
		DEMUXER_UNLOCK(&nddp->ndd_demux_lock);
		error = ((*(nddp->ndd_ctl)) (nddp, NDD_ADD_FILTER, 
			filter, len));
		DEMUXER_LOCK(&nddp->ndd_demux_lock);
                if (error && error != EOPNOTSUPP) {
			switch (filter->filtertype) {
				case NS_8022_LLC_DSAP:
				case NS_8022_LLC_DSAP_SNAP:
					(void)dmx_8022_del_filter(nddp, 
						filter);
					break;

				case FDDI_DEMUX_SMT_NSA:
				case FDDI_DEMUX_SMT:
					(void)fddi_dmx_del_smt(nddp, 
						(fddi_dmx_filter_t *)filter);
					break;

				case FDDI_DEMUX_MAC:
					(void)fddi_dmx_del_mac(nddp);
					break;
				default:
					break;
			}
                } else
                        error = 0;
	}
        if (!error) {
                fddi_users++;
	}
	DEMUXER_UNLOCK(&nddp->ndd_demux_lock);
	return(error);
}

/*
 * NAME: fddi_dmx_del_filter
 *                                                                    
 * FUNCTION: 
 *      Process user request for deleting filters.
 *                                                                    
 * EXECUTION ENVIRONMENT: process
 *                                                                   
 * NOTES: 
 *      The input for this routine is:
 *              struct ndd      *nddp
 *              ns_8022_t       p_filter
 *              int             len
 *
 *      This routine will handle the following types of filters:
 *              NS_TAP
 *              NS_8022_LLC_DSAP
 *              NS_8022_LLC_DSAP_SNAP
 *              FDDI_DEMUX_MAC
 *              FDDI_DEMUX_SMT_NSA
 *              FDDI_DEMUX_SMT
 *
 * RECOVERY OPERATION: 
 *      None.
 *
 * DATA STRUCTURES: 
 *      Common filter struct (ns_8022_t)
 *      NDD struct (ndd_t)
 *
 * ROUTINES CALLED:
 *      Common 802.2 delete filter (dmx_8022_del_filter)
 *      NDD ctl function with NDD_DEL_FILTER operation
 * 	fddi_dmx_del_mac
 * 	fddi_dmx_del_smt
 *	fddi_dmx_del_tap
 *
 * RETURNS:  
 *              0       - successful
 *              EINVAL  - Invalid filter type
 *              rc      - return codes from dmx_8022_del_filter()
 *		rc	- return codes from fddi_dmx_del_smt
 *		rc	- return codes from fddi_dmx_del_tap
 */  
fddi_dmx_del_filter(nddp, filter, len)
	struct ndd		*nddp;
	struct ns_8022		*filter;
	int			len;
{
	int 	error;
	DEMUXER_LOCK_DECL();

	if (len != sizeof(struct ns_8022) && len != sizeof(fddi_dmx_filter_t))
		return(EINVAL);

	DEMUXER_LOCK(&nddp->ndd_demux_lock);

	/* If there is no FDDI control structure in the NDD, then
	 * we've never been initialized (via an add_filter) so bail.
	 */
	if (nddp->ndd_specdemux == (caddr_t) NULL) {
		DEMUXER_UNLOCK(&nddp->ndd_demux_lock);
		return(ENOENT);
	}

	switch (filter->filtertype) {
		case NS_8022_LLC_DSAP:
		case NS_8022_LLC_DSAP_SNAP:
			error = dmx_8022_del_filter(nddp, filter);
			break;

		case FDDI_DEMUX_SMT_NSA:
		case FDDI_DEMUX_SMT:
			error = fddi_dmx_del_smt(nddp, 
				(fddi_dmx_filter_t *)filter);
			break;

		case FDDI_DEMUX_MAC:
			error = fddi_dmx_del_mac(nddp);
			break;

		case NS_TAP:
			error = fddi_dmx_del_tap(nddp);
			break;

		default:
			error = EINVAL;
			break;
	}

	if (!error && filter->filtertype != NS_TAP) {
		DEMUXER_UNLOCK(&nddp->ndd_demux_lock);
		((*(nddp->ndd_ctl)) (nddp, NDD_DEL_FILTER, filter, len));
		DEMUXER_LOCK(&nddp->ndd_demux_lock);
	}
        if (!error) {
                fddi_users--;
		if (fddi_users == 0) {
			NET_FREE(nddp->ndd_specdemux, M_TEMP);
			nddp->ndd_specdemux=(caddr_t)NULL;
		}
	}
	DEMUXER_UNLOCK(&nddp->ndd_demux_lock);
	return(error);
}

/*
 * NAME: fddi_dmx_add_status
 *                                                                    
 * FUNCTION: 
 *      Process user request for adding status filters.
 *                                                                    
 * EXECUTION ENVIRONMENT: process
 *                                                                   
 * NOTES: 
 *      The input for this routine is:
 *              struct ndd      *nddp
 *              ns_com_status_t	p_filter
 *              int             len
 *              ns_statuser_t   *p_user
 *
 *      This routine will handle the following types of status filters:
 *              NS_STATUS_MASK  
 *
 * RECOVERY OPERATION: 
 *      None.
 *
 * DATA STRUCTURES: 
 *      Common status filter struct (ns_com_status_t)
 *      NDD struct (ndd_t)
 *      NS status user struct (ns_statuser_t)
 *
 * ROUTINES CALLED:
 *      Common add status filter (dmx_add_status )
 *
 * RETURNS:  
 *              0       - successful
 *              EINVAL  - Invalid filter type
 *              rc      - return codes from dmx_add_status()
 */  
fddi_dmx_add_status(nddp, filter, len, ns_statuser)
	struct ndd		*nddp;
	struct ns_com_status	*filter;
	int			len;
	struct ns_statuser	*ns_statuser;
{
	int	error;
	DEMUXER_LOCK_DECL();

	DEMUXER_LOCK(&nddp->ndd_demux_lock);

	/*
	 * If this is the first add, then initialize the fddi control
	 * structure.
	 */
	if (nddp->ndd_specdemux == (caddr_t) NULL) {
		if (fddi_dmx_init(nddp)) {
			DEMUXER_UNLOCK(&nddp->ndd_demux_lock);
			return(ENOMEM);
		}
	}
	error = dmx_add_status(nddp, filter, ns_statuser);
	if (!error) {
		DEMUXER_UNLOCK(&nddp->ndd_demux_lock);
		error = ((*(nddp->ndd_ctl)) (nddp, NDD_ADD_STATUS, 
			filter, len));
		DEMUXER_LOCK(&nddp->ndd_demux_lock);
		if (error && error != EOPNOTSUPP)
			(void) dmx_del_status(nddp, filter);
		else 
			error = 0;
	}
	if (!error) {
		fddi_users++;
	}
	DEMUXER_UNLOCK(&nddp->ndd_demux_lock);
	return(error);
}

/*
 * NAME: fddi_dmx_del_status
 *                                                                    
 * FUNCTION: 
 *      Process user request for deleting status filters.
 *                                                                    
 * EXECUTION ENVIRONMENT: process
 *                                                                   
 * NOTES: 
 *      The input for this routine is:
 *              struct ndd      *nddp
 *              ns_com_status_t p_filter
 *              int             len
 *
 *      This routine will handle the following types of status filters:
 *              NS_STATUS_MASK  
 *
 * RECOVERY OPERATION: 
 *      None.
 *
 * DATA STRUCTURES: 
 *      Common status filter struct (ns_com_status_t)
 *      NDD struct (ndd_t)
 *
 * ROUTINES CALLED:
 *      Common delete status filter (dmx_del_status )
 *
 * RETURNS:  
 *              0       - successful
 *              EINVAL  - Invalid filter type
 *              rc      - return codes from dmx_del_status()
 */  
fddi_dmx_del_status(nddp, filter, len)
	struct ndd		*nddp;
	struct ns_com_status	*filter;
	int			len;
{
	int	error;
	DEMUXER_LOCK_DECL();

	DEMUXER_LOCK(&nddp->ndd_demux_lock);

	/* If there is no FDDI control structure in the NDD, then
	 * we've never been initialized (via an add_filter) so bail.
	 */
	if (nddp->ndd_specdemux == (caddr_t) NULL) {
		DEMUXER_UNLOCK(&nddp->ndd_demux_lock);
		return(ENOENT);
	}
	error = dmx_del_status(nddp, filter);
	if (!error) {
		DEMUXER_UNLOCK(&nddp->ndd_demux_lock);
		((*(nddp->ndd_ctl)) (nddp, NDD_DEL_STATUS, filter, len));
		DEMUXER_LOCK(&nddp->ndd_demux_lock);
		fddi_users--;
		if (fddi_users == 0 ) {
			NET_FREE(nddp->ndd_specdemux, M_TEMP);
			nddp->ndd_specdemux=(caddr_t)NULL;
		}
	}
	DEMUXER_UNLOCK(&nddp->ndd_demux_lock);
	return(error);
}

/*
 * NAME: fddi_dmx_receive
 *                                                                    
 * FUNCTION: 
 *      Process incoming packets from a NDD.  
 *      The packets are routed to the appropriate users.
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt
 *                                                                   
 * NOTES: 
 *      This routine is called with a ptr to a NDD struct (nddp) and 
 *      an mbuf ptr (m).  The mbuf chain may contain more than 1
 *      pkt.
 *
 *      This function must demux the following types of FDDI pkts:
 *              LLC frames with or without SNAP
 *              MAC frames
 *		SMT_NSA frames
 *		SMT frames
 *
 * RECOVERY OPERATION: 
 *      None.
 *
 * DATA STRUCTURES: 
 *      FDDI packet header (fddi_hdr_t)
 *      NDD structure (ndd_t)
 *      mbuf structure (struct mbuf)
 *      FDDI demuxer ctl struct (struct fddi_dmx_ctl) - internal struct
 *
 *
 * ROUTINES CALLED:
 *      Common 802.2 demuxer routine (dmx_8022_receive)
 *      m_freem
 *
 * RETURNS:  
 *              none
 */  
void
fddi_dmx_receive(nddp, m)
	struct ndd		*nddp;
	struct mbuf		*m;
{
        struct fddi_mac_hdr    	*macp;
	struct mbuf		*mnextpkt;
	struct fddi_dmx_ctl	*fddi_dmx_ctl;
	struct isr_data_ext	ext, *extp;
	DEMUXER_LOCK_DECL();

	DEMUXER_LOCK(&nddp->ndd_demux_lock);

	/* if we don't have any filters, then free the mbuf chain and
	 * bail.
	 */
	if (nddp->ndd_specdemux == (caddr_t)NULL) {
		while ( m != (struct mbuf *) NULL ) {
			mnextpkt = m->m_nextpkt;
			m->m_nextpkt = (struct mbuf *) NULL;
			m_freem(m);
			m = mnextpkt;
			nddp->ndd_nsdemux->nd_dmxstats.nd_nofilter++;
		}	
		DEMUXER_UNLOCK(&nddp->ndd_demux_lock);
		return;	
	}
	fddi_dmx_ctl = (struct fddi_dmx_ctl *)nddp->ndd_specdemux;
	while ( m != (struct mbuf *) NULL ) {

		mnextpkt = m->m_nextpkt;
		m->m_nextpkt = (struct mbuf *) NULL;
		macp = mtod(m, struct fddi_mac_hdr *);

		/*
		 * Give to tap user if one exists.  Note that the tap user
		 * must copy the packet (we retain ownership of mbuf chain).
	 	 */
		if (fddi_dmx_ctl->tap_user.isr != NULL) {
			if (fddi_dmx_ctl->tap_user.pkt_format & 
				NS_HANDLE_HEADERS) {

				ext.dstp = macp->mac_dst_f;
				ext.srcp = macp->mac_src_f;
				ext.dstlen = ext.srclen = FDDI_ADDRLEN;
				if (has_route_f(macp)) {
					ext.segp = (caddr_t)&macp->mac_rcf_f;
					ext.seglen = route_bytes_f(macp);
				} else {
					ext.segp = 0;	
					ext.seglen = 0;	
				}
				ext.isr_data = fddi_dmx_ctl->tap_user.isr_data;
				ext.llcp = (caddr_t)macp + mac_size_f(macp);
				ext.llclen = 0;
				extp = &ext;
			} else
				extp = (struct isr_data_ext *)
					fddi_dmx_ctl->tap_user.isr_data;
			(*(fddi_dmx_ctl->tap_user.isr)) (nddp, m, macp, extp);
		}

		/* 
		 * Now switch on the MAC type.  
		 */
		switch(macp->mac_fcf_f & FDDI_DEMUX_FC_MASK) {
	
			case FDDI_DEMUX_LLC_FRAME:

				/*
				 * If we're in promiscuous mode AND it's
				 * not a [M|B]CAST AND the dst hwaddr is
				 * not ours, then drop the packet.
				 */
				if (nddp->ndd_flags & NDD_PROMISC) {
				     if ((m->m_flags & 
					(M_BCAST|M_MCAST)) == 0) {

					/*
					 * memcmp() gets inlined...
					 */
					if (memcmp((caddr_t)nddp->ndd_physaddr,
						(caddr_t)macp->mac_dst_f,
						FDDI_ADDRLEN)) {	
						m_freem(m);
						m = mnextpkt;
						continue;
					}
				     }
				}
				ext.dstp = macp->mac_dst_f;
				ext.srcp = macp->mac_src_f;
				ext.dstlen = ext.srclen = FDDI_ADDRLEN;
				if (has_route_f(macp)) {
					ext.segp = (caddr_t)&macp->mac_rcf_f;
					ext.seglen = route_bytes_f(macp);
				} else {
					ext.segp = 0;	
					ext.seglen = 0;	
				}
				dmx_8022_receive(nddp, m, mac_size_f(macp), 
					&ext);
				break;

			case FDDI_DEMUX_MAC_FRAME:
				fddi_dmx_mac_receive(nddp, m, macp);
				break;
				
			case FDDI_DEMUX_SMT_FRAME:
				fddi_dmx_smt_receive(nddp, m, macp);
				break;
				
			default:

				/*
				 * Frame is bogus...
				 */
				m_freem(m);
				break;
			} /* switch */
		m = mnextpkt;
		} /* while */
	DEMUXER_UNLOCK(&nddp->ndd_demux_lock);
	return;
}


/*
 * NAME: fddi_dmx_response
 *                                                                    
 * FUNCTION: 
 *      Swap src and dst addrs and send frame.
 *      Called by 802.2 demuxer services to send a XID or TEST
 *	LLC frame.
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt
 *                                                                   
 * NOTES: 
 *      This routine is called with a ptr to a NDD struct (nddp), 
 *      an mbuf ptr (m), and the llc offset.  
 *	The mbuf chain may NOT contain more than 1 pkt. 
 *
 * RECOVERY OPERATION: 
 *      None.
 *
 * DATA STRUCTURES: 
 *      FDDI packet header (fddi_hdr_t)
 *      NDD structure (ndd_t)
 *      mbuf structure (struct mbuf)
 *
 *
 * ROUTINES CALLED:
 *      NDD output routine
 *
 * RETURNS:  
 *              none
 */  
void
fddi_dmx_response(nddp, m, llcoffset)
	struct ndd		*nddp;
	struct mbuf		*m;
	int			llcoffset;
{
	struct fddi_mac_hdr		*fddimacp;
	caddr_t				llcp;

	fddimacp = mtod(m, struct fddi_mac_hdr *);
	llcp = (caddr_t)fddimacp + llcoffset;
	FDDI_DEMUX_ADDRCOPY(fddimacp->mac_src_f, fddimacp->mac_dst_f);
	FDDI_DEMUX_ADDRCOPY(nddp->ndd_physaddr, fddimacp->mac_src_f);

	/* 
	 * optional information field
	 * preserve the routing info if needed
	 */
	fddimacp->mac_src_f[0] |= fddimacp->mac_dst_f[0] & RI_PRESENT;
	fddimacp->mac_dst_f[0] &= ~RI_PRESENT;
	if (has_route_f(fddimacp)) {
		fddimacp->mac_rcf_f ^=  RCF_DIRECTION;
		fddimacp->mac_rcf_f &= 
			~(RCF_ALL_BROADCAST | RCF_LOCAL_BROADCAST);
	}

	if ((*(nddp->ndd_output))(nddp, m))
		m_freem(m);
	return;
}


/*
 * NAME: fddi_dmx_status
 *                                                                    
 * FUNCTION: 
 *      Called from the NDD.  Pass status up to the user.
 *	Grab the demuxer lock and call the common services.
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt
 *                                                                   
 * NOTES: 
 *      This routine is called with a ptr to a NDD struct (nddp) and 
 *      a pointer to a status filter.
 *
 *
 * RECOVERY OPERATION: 
 *      None.
 *
 * DATA STRUCTURES: 
 *
 * ROUTINES CALLED:
 *      Common 802.2 demuxer status routine
 *
 * RETURNS:  
 *              none
 */  

void
fddi_dmx_status(nddp, statusp)
struct ndd		*nddp;
struct ndd_statblk	*statusp;
{
	DEMUXER_LOCK_DECL();
	DEMUXER_LOCK(&nddp->ndd_demux_lock);
	dmx_status(nddp, statusp);
	DEMUXER_UNLOCK(&nddp->ndd_demux_lock);
}

/*
 * NAME: fddi_dmx_address_input
 *                                                                    
 * FUNCTION: 
 * 	Default ARP input handler
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt
 *                                                                   
 * NOTES: 
 * 	
 *
 * RECOVERY OPERATION: 
 *      None.
 *
 * DATA STRUCTURES: 
 *      arptab struct
 *      fddi_mac_hdr struct
 *
 * ROUTINES CALLED:
 *	fddi_arp_init
 *
 * RETURNS:  
 *              None.
 */  
void
fddi_dmx_address_input(mac, arp)
struct fddi_mac_hdr *mac;
int arp;
{
	register struct arptab *at, *ato;
	u_long src;
	int n, s;
	u_char o_sister[FDDI_ADDRLEN];

	/* do no arping unless requested */
	if (!arp)
		return;

	ARPTAB_LOCK(&fddi_arptab_lock);
	if (!fddi_arptabp)  /* Not initialized. Set it up. One long interrupt.*/
		if (fddi_arp_init()) {/* We'll try again on another packet. */
			ARPTAB_UNLOCK(&fddi_arptab_lock);
			return;
		}

	if (has_route_f(mac)) {
		mac->mac_rcf_f ^=  RCF_DIRECTION;
		mac->mac_rcf_f &= ~(RCF_ALL_BROADCAST | RCF_LOCAL_BROADCAST);
	}
	bcopy(mac->mac_src_f, o_sister, FDDI_ADDRLEN);
	o_sister[0] &= ~RI_PRESENT;
	src = (u_long) o_sister[0] + (u_long) o_sister[FDDI_ADDRLEN-1]; 
	at = ato = &fddi_arptabp[FDDI_ARPTAB_HASH(src) * fddi_arptabbsiz];
	for (n = 0 ; n < fddi_arptabbsiz ; n++, at++) {
		if (!(bcmp(at->hwaddr, o_sister, FDDI_ADDRLEN)))
			break;
		if (!at->at_flags)
			ato = at;
		if (ato->at_flags && at->at_timer > ato->at_timer)
			ato = at;
	}
	if  (n >= fddi_arptabbsiz) {
		
		/*
		 * the old one's a goner
		 */
		at = ato;
		bzero(at, sizeof(*at));
	}
	bcopy(o_sister, (caddr_t)at->at_traddr,FDDI_ADDRLEN);
		/*
		 * snarf any routing info
		 */
	if (has_route_f(mac)
	    && route_bytes_f(mac) > sizeof (mac->mac_rcf_f)) {
		at->at_rcf = mac->mac_rcf_f;
		bcopy(mac->mac_seg_f, at->at_seg, 
			     sizeof (at->at_seg));
	} else
		at->at_rcf = 0;
	/*
	 * Set the flag indicating the entry in use
	 */
	at->at_flags = (ATF_INUSE);
	ARPTAB_UNLOCK(&fddi_arptab_lock);
	mac->mac_src_f[0] &= ~RI_PRESENT;
	return;
}

/*
 * NAME: fddi_dmx_address_resolve
 *                                                                    
 * FUNCTION: 
 * 	Default ARP table resolver
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt, process
 *                                                                   
 * NOTES: 
 * 	
 *
 * RECOVERY OPERATION: 
 *      None.
 *
 * DATA STRUCTURES: 
 *      arptab struct
 *      fddi_mac_hdr struct
 *      ie2_llc_snaphdr struct
 *
 * ROUTINES CALLED:
 *	fddi_arp_init
 *
 * RETURNS:  
 *              EINVAL output_bundle in error.
 *              rc from ndd_output function.
 *		0 success.
*/
int
fddi_dmx_address_resolve(obp, m, nddp)
struct output_bundle	*obp;
struct mbuf 		*m;
struct ndd 		*nddp; 
{
	struct fddi_hdr mac;
	struct fddi_mac_hdr *macp;
	struct ie2_llc_snaphdr *llcp;
	struct arptab *at;
	int llcsize;
	int rc, s, n;
	u_long dest;

	ARPTAB_LOCK(&fddi_arptab_lock);
	if (!fddi_arptabp) 
		if (fddi_arp_init()) {
			ARPTAB_UNLOCK(&fddi_arptab_lock);
			m_freem(m);
			return(ENOBUFS);
		}

	macp = (struct fddi_mac_hdr *) &mac;
	llcp = (struct ie2_llc_snaphdr *)(macp+1);
	switch(obp->helpers.pkt_format & 0xFF) {
		case NS_PROTO:
			llcp->dsap = obp->helpers.sapu.llc.dsap;
			llcp->ssap = obp->helpers.sapu.llc.ssap;
			llcp->ctrl = obp->helpers.sapu.llc.ctrl;
			llcsize = sizeof(struct ie2_llc_hdr);
			break;
				
		case NS_PROTO_SNAP:
			llcp->dsap = obp->helpers.sapu.llc.dsap;
			llcp->ssap = obp->helpers.sapu.llc.ssap;
			llcp->ctrl = obp->helpers.sapu.llc.ctrl;
			bcopy(obp->helpers.sapu.llcsnap.prot_id, 
				llcp->prot_id, 3);
			llcp->type = obp->helpers.sapu.llcsnap.type;
			llcsize = sizeof(struct ie2_llc_snaphdr);
			break;

		case NS_INCLUDE_LLC:
			llcsize = 0;
			break;
		default:
			ARPTAB_UNLOCK(&fddi_arptab_lock);
			m_freem(m);
			return(EINVAL);
	}

	macp->mac_fcf_f     = FCF_FDDI;
	macp->_First.reserved[0] = 0;
	
	bcopy(obp->key_to_find, macp->mac_dst_f, FDDI_ADDRLEN);
	bcopy(nddp->ndd_physaddr, macp->mac_src_f, FDDI_ADDRLEN);

	/* use user-provided route info if present */
	if (obp->helpers.seglen > 0) {
		macp->mac_src_f[0] |= RI_PRESENT;
		bcopy(obp->helpers.segp, &macp->mac_rcf_f, obp->helpers.seglen);
		goto done;
	}

	/*
	 * check for broadcast.
	 */
 	if ((m->m_flags & M_BCAST) || (macp->mac_dst_f[0] & 0x80)) {	
		goto done;
	}
	dest = (u_long) macp->mac_dst_f[0] + 
		(u_long) macp->mac_dst_f[FDDI_ADDRLEN-1]; 
	at = &fddi_arptabp[FDDI_ARPTAB_HASH(dest) * fddi_arptabbsiz];
	for (n = 0 ; n < fddi_arptabbsiz ; n++, at++)
		if (!(bcmp(at->hwaddr, macp->mac_dst_f, FDDI_ADDRLEN)))
			break;
	if  (n >= fddi_arptabbsiz)
		at = 0; 
	if (at == 0) {
		/*
		 * can't ask for it. Set no route and pray for rain.
		 */
		macp->mac_src_f[0] &= ~RI_PRESENT;
	} else {
		/*
		 * routing control info present?
		 */
		if (at->at_rcf) {
			macp->mac_src_f[0] |= RI_PRESENT;
			macp->mac_rcf_f     = at->at_rcf;
			bcopy((caddr_t) at->at_seg
				, macp->mac_seg_f
				, sizeof (macp->mac_seg_f));
		} else  {
			macp->mac_src_f[0] &= ~RI_PRESENT;
		}
		at->at_timer = 0; /* zero out the usage count for timer aging*/
	}
done:
	ARPTAB_UNLOCK(&fddi_arptab_lock);
	M_PREPEND(m, mac_size_f(macp) + llcsize, M_DONTWAIT);
	if (m == 0) {
		nddp->ndd_nsdemux->nd_dmxstats.nd_nobufs++;
		return(ENOBUFS);
	}
	bcopy(macp, m->m_data, mac_size_f(macp));
	if (llcsize)
		bcopy(llcp, m->m_data+mac_size_f(macp), llcsize);
	
	rc = (*(nddp->ndd_output))(nddp, m); 
	if (rc)
		m_freem(m);
	return(rc);
}
/*
 * NAME: fddi_arp_init
 *                                                                    
 * FUNCTION: 
 * 	Inits Default ARP table
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt
 *                                                                   
 * NOTES: 
 * 	
 *
 * RECOVERY OPERATION: 
 *      None.
 *
 * DATA STRUCTURES: 
 *      arptab struct
 *      trb struct
 *
 * ROUTINES CALLED:
 *	tstart
 *
 * RETURNS:  
 *              1 Failure.
 *              0 Success.
 */  
int fddi_arp_init(void)
{
int thesize;

	thesize = arptabsize * sizeof(struct arptab);
	if (thesize > MAXALLOCSAVE) {
		fddi_arptabbsiz = arptab_bsiz_dflt;
		fddi_arptabnb = arptab_nb_dflt;
		thesize = (sizeof(struct arptab)) * fddi_arptabbsiz * 
			fddi_arptabnb;
	} else {
		fddi_arptabbsiz = arptab_bsiz;
		fddi_arptabnb = arptab_nb;
	}
	NET_MALLOC(fddi_arptabp, struct arptab *, thesize, M_KTABLE, M_NOWAIT);
	if (!fddi_arptabp)
		return(1);
	bzero((caddr_t)fddi_arptabp, thesize);
	fddi_arptime->timeout.it_value.tv_sec = 60;
	fddi_arptime->timeout.it_value.tv_nsec = 0;
	fddi_arptime->ipri = PL_IMP;
	fddi_arptime->flags = 0;
	fddi_arptime->func = (void (*)())fddi_arptimer;
	fddi_arptime->func_data = (ulong) 0;
	tstart(fddi_arptime);
	return(0);
}

/*
 * NAME: fddi_arptimer
 *                                                                    
 * FUNCTION: 
 * 	Default ARP table timer
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt
 *                                                                   
 * NOTES: 
 * 	
 *
 * RECOVERY OPERATION: 
 *      None.
 *
 * DATA STRUCTURES: 
 *      arptab struct
 *      trb struct
 *
 * ROUTINES CALLED:
 *	tstart
 *
 * RETURNS:  
 *              None.
*/
void
fddi_arptimer()
{
	register  struct  arptab	*at;
	register  int	i, killer, s;

	killer = ARPT_KILLC;
	ARPTAB_LOCK(&fddi_arptab_lock);
	if (!fddi_arptabp) {
		ARPTAB_UNLOCK(&fddi_arptab_lock);
		return;
	}
	at = fddi_arptabp;
	for (i = 0; i <  (fddi_arptabbsiz * fddi_arptabnb); i++ , at++) {
		if   (at->at_flags == 0 || (at->at_flags & ATF_PERM))
			continue;
		if (++at->at_timer <  killer)
				continue;
		/* timer has expired, clear entry	*/
		bzero(at, sizeof(*at));
	}
	ARPTAB_UNLOCK(&fddi_arptab_lock);
	fddi_arptime->timeout.it_value.tv_sec = 60;
	fddi_arptime->timeout.it_value.tv_nsec = 0;
	fddi_arptime->ipri = PL_IMP;
	tstart(fddi_arptime);
	return;
}
	
/*
 * NAME: fddi_dmx_add_demuxer
 *                                                                    
 * FUNCTION: 
 * 	Adds the demuxer to the global demuxer list.
 *                                                                    
 * EXECUTION ENVIRONMENT: process
 *                                                                   
 * NOTES: 
 *	Set up the global ns_demuxer structure.
 *
 * RECOVERY OPERATION: 
 *      None.
 *
 * DATA STRUCTURES: 
 *	ns_demuxer
 *
 * ROUTINES CALLED:
 *	None.
 *
 * RETURNS:  
 *              0 - success
 *		rc from ns_add_demux service.
 */  
fddi_dmx_add_demuxer()
{
	
	bzero(&demuxer, sizeof(struct ns_demuxer));
	demuxer.nd_add_filter = fddi_dmx_add_filter;
	demuxer.nd_del_filter = fddi_dmx_del_filter;
	demuxer.nd_add_status = fddi_dmx_add_status;
	demuxer.nd_del_status = fddi_dmx_del_status;
	demuxer.nd_receive = fddi_dmx_receive;
	demuxer.nd_status = fddi_dmx_status;
	demuxer.nd_response = fddi_dmx_response;
	demuxer.nd_use_nsdmx = TRUE;
	demuxer.nd_address_resolve = fddi_dmx_address_resolve;
	demuxer.nd_address_input = fddi_dmx_address_input;
	fddi_arptabp = (struct arptab *) 0;
	ARPTAB_LOCKINIT(&fddi_arptab_lock);
	return(ns_add_demux(NDD_FDDI, &demuxer));
}

/*
 * NAME: fddi_dmx_init
 *                                                                    
 * FUNCTION: 
 * 	Initializes the demuxer.
 *                                                                    
 * EXECUTION ENVIRONMENT: process
 *                                                                   
 * NOTES: 
 * 	Assumes demuxer lock is held.
 *
 * RECOVERY OPERATION: 
 *      None.
 *
 * DATA STRUCTURES: 
 *      NDD struct (ndd_t)
 *      Ethernet demuxer control struct (struct fddi_dmx_ctl) - internal struct
 *
 * ROUTINES CALLED:
 *	NET_MALLOC
 *
 * RETURNS:  
 *              0 - success
 *		ENOMEM - no mem
 */  
fddi_dmx_init(nddp) 
	ndd_t		*nddp;
{
	struct fddi_dmx_ctl	*fddi_dmx_ctl;
	int			i;

	NET_MALLOC(fddi_dmx_ctl, struct fddi_dmx_ctl *, 
		   sizeof(struct fddi_dmx_ctl), M_TEMP, M_NOWAIT); 
	if (fddi_dmx_ctl == (struct fddi_dmx_ctl *) NULL)
		return(ENOMEM);
	bzero((caddr_t)fddi_dmx_ctl, sizeof(struct fddi_dmx_ctl));
	nddp->ndd_specdemux = (caddr_t) fddi_dmx_ctl;
	fddi_dmx_ctl->smt_user.next = &fddi_dmx_ctl->smt_user;
	fddi_dmx_ctl->smt_user.prev = &fddi_dmx_ctl->smt_user;
	return(0);
}

/*
 * NAME: fddi_dmx_add_tap
 *                                                                    
 * FUNCTION: 
 * 	Adds a NS_TAP filter.
 *                                                                    
 * EXECUTION ENVIRONMENT: process
 *                                                                   
 * NOTES: 
 * 	Assumes demuxer lock is held.
 *
 * RECOVERY OPERATION: 
 *      None.
 *
 * DATA STRUCTURES: 
 *      NDD struct (ndd_t)
 *      NS status user struct (ns_user_t)
 *      Ethernet demuxer control struct (struct fddi_dmx_ctl) - internal struct
 *
 * ROUTINES CALLED:
 *	None.
 *
 * RETURNS:  
 *              0 - success
 *		EXIST - already exists
 */  
fddi_dmx_add_tap(nddp, ns_user)
	ndd_t		*nddp;
	ns_user_t	*ns_user;
{
	struct fddi_dmx_ctl	*fddi_dmx_ctl;

	fddi_dmx_ctl = (struct fddi_dmx_ctl *) nddp->ndd_specdemux;
	if (fddi_dmx_ctl->tap_user.isr != NULL)
		return(EEXIST);

	fddi_dmx_ctl->tap_user = *ns_user;	
	return(0);
}

/*
 * NAME: fddi_dmx_del_tap
 *                                                                    
 * FUNCTION: 
 * 	Deletes a NS_TAP filter.
 *                                                                    
 * EXECUTION ENVIRONMENT: process
 *                                                                   
 * NOTES: 
 * 	Assumes demuxer lock is held.
 *
 * RECOVERY OPERATION: 
 *      None.
 *
 * DATA STRUCTURES: 
 *      NDD struct (ndd_t)
 *      Ethernet demuxer control struct (struct fddi_dmx_ctl) - internal struct
 *
 * ROUTINES CALLED:
 *	None.
 *
 * RETURNS:  
 *              0 - success
 *		ENOENT - no tap user exists
 */  
fddi_dmx_del_tap(nddp)
	ndd_t		*nddp;
{
	struct fddi_dmx_ctl	*fddi_dmx_ctl;

	fddi_dmx_ctl = (struct fddi_dmx_ctl *) nddp->ndd_specdemux;
	if (fddi_dmx_ctl->tap_user.isr == NULL)
		return(ENOENT);

	bzero(&fddi_dmx_ctl->tap_user, sizeof(fddi_dmx_ctl->tap_user));
	return(0);
}

/*
 * NAME: fddi_dmx_add_mac
 *                                                                    
 * FUNCTION: 
 * 	Adds a FDDI_DEMUX_MAC filter.
 *                                                                    
 * EXECUTION ENVIRONMENT: process
 *                                                                   
 * NOTES: 
 * 	Assumes demuxer lock is held.
 *
 * RECOVERY OPERATION: 
 *      None.
 *
 * DATA STRUCTURES: 
 *      NDD struct (ndd_t)
 *      NS status user struct (ns_user_t)
 *      FDDI demuxer control struct (struct fddi_dmx_ctl) - internal struct
 *
 * ROUTINES CALLED:
 *	None.
 *
 * RETURNS:  
 *              0 - success
 *		EXIST - already exists
 */  
fddi_dmx_add_mac(nddp, ns_user)
	ndd_t		*nddp;
	ns_user_t	*ns_user;
{
	struct fddi_dmx_ctl	*fddi_dmx_ctl;

	fddi_dmx_ctl = (struct fddi_dmx_ctl *) nddp->ndd_specdemux;
	if (fddi_dmx_ctl->mac_user.isr != NULL)
		return(EEXIST);

	fddi_dmx_ctl->mac_user = *ns_user;	
	return(0);
}

/*
 * NAME: fddi_dmx_del_mac
 *                                                                    
 * FUNCTION: 
 * 	Deletes a mac filter.
 *                                                                    
 * EXECUTION ENVIRONMENT: process
 *                                                                   
 * NOTES: 
 * 	Assumes demuxer lock is held.
 *
 * RECOVERY OPERATION: 
 *      None.
 *
 * DATA STRUCTURES: 
 *      NDD struct (ndd_t)
 *      FDDI demuxer control struct (struct fddi_dmx_ctl) 
 *
 * ROUTINES CALLED:
 *	None.
 *
 * RETURNS:  
 *              0 - success
 *		ENOENT - no mac user exists
 */  
fddi_dmx_del_mac(nddp)
	ndd_t		*nddp;
{
	struct fddi_dmx_ctl	*fddi_dmx_ctl;

	fddi_dmx_ctl = (struct fddi_dmx_ctl *) nddp->ndd_specdemux;
	if (fddi_dmx_ctl->mac_user.isr == NULL)
		return(ENOENT);

	bzero(&fddi_dmx_ctl->mac_user, sizeof(fddi_dmx_ctl->mac_user));
	return(0);
}

/*
 * NAME: fddi_dmx_mac_receive
 *                                                                    
 * FUNCTION: 
 *      Process incoming MAC packets from a NDD.  
 *      The packets are routed to the MAC user if one exists.
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt
 *                                                                   
 * NOTES: 
 *      This routine is called with a ptr to a NDD struct (nddp), 
 *      an mbuf ptr (m) and a FDDI mac ptr (macp).  
 *
 * RECOVERY OPERATION: 
 *      None.
 *
 * DATA STRUCTURES: 
 *      NDD structure (ndd_t)
 *      mbuf structure (struct mbuf)
 *      FDDI demuxer ctl struct (struct fddi_dmx_ctl) - internal struct
 *      FDDI demuxer stats struct
 *	
 * ASSUMES demuxer lock held (nddp->ndd_demux_lock)
 *
 * ROUTINES CALLED:
 *      m_freem
 *
 * RETURNS:  
 *              none
 */  
void
fddi_dmx_mac_receive(nddp, m, macp)
ndd_t			*nddp;
struct mbuf		*m;
struct fddi_mac_hdr	*macp;
{
	struct fddi_dmx_ctl	*fddi_dmx_ctl;
	struct isr_data_ext	ext, *extp;

	fddi_dmx_ctl = (struct fddi_dmx_ctl *) nddp->ndd_specdemux;
        if (fddi_dmx_ctl->mac_user.isr == NULL) {
                m_freem(m);
		fddi_dmx_ctl->stats.nd_mac_rejects++;
		nddp->ndd_nsdemux->nd_dmxstats.nd_nofilter++;
	}
	if (fddi_dmx_ctl->mac_user.pkt_format & NS_HANDLE_HEADERS) {
		ext.dstp = macp->mac_dst_f;
		ext.srcp = macp->mac_src_f;
		ext.dstlen = ext.srclen = FDDI_ADDRLEN;
		if (has_route_f(macp)) {
			ext.segp = (caddr_t)&macp->mac_rcf_f;
			ext.seglen = route_bytes_f(macp);
		} else {
			ext.segp = 0;	
			ext.seglen = 0;	
		}
		ext.isr_data = fddi_dmx_ctl->mac_user.isr_data;
		extp = &ext;
	} else
		extp = (struct isr_data_ext *)fddi_dmx_ctl->mac_user.isr_data;
	DELIVER_PACKET((&(fddi_dmx_ctl->mac_user)), nddp, m, macp, extp);
	fddi_dmx_ctl->stats.nd_mac_accepts++;
}

/*
 * NAME: fddi_dmx_add_smt
 *                                                                    
 * FUNCTION: 
 *      Process user request for adding smt filters.
 *                                                                    
 * EXECUTION ENVIRONMENT: process
 *                                                                   
 * NOTES: 
 *
 *      This routine will handle the following types of filters:
 *		FDDI_DEMUX_SMT_NSA
 *		FDDI_DEMUX_SMT
 *
 * RECOVERY OPERATION: 
 *      None.
 *
 * DATA STRUCTURES: 
 *      NDD struct (ndd_t)
 *	ns_user struct
 *	fddi_dmx_filter struct
 *	fddi_dmx_ctl struct
 *
 * ROUTINES CALLED:
 *	NET_MALLOC()
 *	insqueue()
 *
 * RETURNS:  
 *              0       - successful
 *              ENOMEM  - No filter exists
 */  
fddi_dmx_add_smt(nddp, ns_user, filter)
struct	ndd		*nddp;
ns_user_t		*ns_user;
fddi_dmx_filter_t	*filter;
{
	fddi_dmx_smt_user_t	*smt_user, *s;
	fddi_dmx_ctl_t		*fddi_dmx_ctl;

	fddi_dmx_ctl = (fddi_dmx_ctl_t *)nddp->ndd_specdemux;

	NET_MALLOC(smt_user, fddi_dmx_smt_user_t *, 
		   sizeof(*smt_user), M_TEMP, M_NOWAIT); 
	if (smt_user == (fddi_dmx_smt_user_t *)NULL)
		return(ENOMEM);

	/* The address of smt_user is unique */
	filter->id = (int)smt_user;

	smt_user->filter = *filter;
	smt_user->user = *ns_user;
	
	insque(smt_user, &fddi_dmx_ctl->smt_user);
	return(0);
}

/*
 * NAME: fddi_dmx_del_smt
 *                                                                    
 * FUNCTION: 
 *      Process user request for deleting smt filters.
 *                                                                    
 * EXECUTION ENVIRONMENT: process
 *                                                                   
 * NOTES: 
 *
 *      This routine will handle the following types of filters:
 *		FDDI_DEMUX_SMT_NSA
 *		FDDI_DEMUX_SMT
 *
 * RECOVERY OPERATION: 
 *      None.
 *
 * DATA STRUCTURES: 
 *      NDD struct (ndd_t)
 *	ns_user struct
 *	fddi_dmx_filter struct
 *	fddi_dmx_ctl struct
 *
 * ROUTINES CALLED:
 *	NET_FREE()
 *	remqueue()
 *
 * RETURNS:  
 *              0       - successful
 *              ENOENT  - No filter exists
 */  
fddi_dmx_del_smt(nddp, filter)
struct	ndd		*nddp;
fddi_dmx_filter_t	*filter;
{
	fddi_dmx_smt_user_t	*smt_user;
	fddi_dmx_ctl_t		*fddi_dmx_ctl;
	int			error;

	fddi_dmx_ctl = (fddi_dmx_ctl_t *)nddp->ndd_specdemux;

	/*
	 * Find the smt filter in our list of smt users that 
	 * matches the filter passed in.
	 */
	for (smt_user = fddi_dmx_ctl->smt_user.next; 
		smt_user != &fddi_dmx_ctl->smt_user &&
		smt_user->filter.id != filter->id;
		smt_user = smt_user->next) ;
	
	if (smt_user == &fddi_dmx_ctl->smt_user)
		return(ENOENT);
	remque(smt_user);
	NET_FREE(smt_user, M_TEMP);
	return(0);
}

/*
 * NAME: fddi_dmx_smt_receive
 *                                                                    
 * FUNCTION: 
 *      Demuxes smt frames...
 *                                                                    
 * EXECUTION ENVIRONMENT: process
 *                                                                   
 * NOTES: 
 *      The input for this routine is:
 *              struct ndd      *nddp
 *		struct mbuf 	*m
 *		struct fddi_mac_hdr *macp
 *
 * RECOVERY OPERATION: 
 *      None.
 *
 * DATA STRUCTURES: 
 *	fddi_dmx_ctl_t struct
 *	fddi_dmx_stats_t struct
 *
 * ROUTINES CALLED:
 *	DELIVER_PACKET() macro.
 *	m_copym().
 *
 * RETURNS:  
 *	None.
 */  
void
fddi_dmx_smt_receive(nddp, m, macp)
struct ndd		*nddp;
struct mbuf		*m;
struct fddi_mac_hdr	*macp;
{
	struct mbuf		*newm=NULL;
	fddi_dmx_ctl_t		*fddi_dmx_ctl;
	fddi_dmx_smt_user_t	*smt_user;
	int			frame_type;
	struct fddi_mac_hdr	*newmacp;
	struct isr_data_ext	ext, *extp;


	fddi_dmx_ctl = (fddi_dmx_ctl_t *)nddp->ndd_specdemux;
	frame_type = (macp->mac_fcf_f == FDDI_DEMUX_SMT_NSA_FRAME) ? 
		FDDI_DEMUX_SMT_NSA : FDDI_DEMUX_SMT;
		
	/*
	 * Deliver the packet to all frame_type users.
	 */
	for (smt_user = fddi_dmx_ctl->smt_user.next;
		smt_user != &fddi_dmx_ctl->smt_user;
		smt_user = smt_user->next) {
		if (smt_user->filter.filter.filtertype == frame_type) {

			/*
			 * Copy packet.  If this fails, continue through
			 * the loop (maybe the next user will get it).
			 */
			newm = m_copym(m, 0, M_COPYALL, M_DONTWAIT);
			if (newm) {
				newmacp = mtod(newm, struct fddi_mac_hdr *);
				if (smt_user->user.pkt_format & 
					NS_HANDLE_HEADERS) {
					ext.dstp = macp->mac_dst_f;
					ext.srcp = macp->mac_src_f;
					ext.dstlen = ext.srclen = FDDI_ADDRLEN;
					if (has_route_f(macp)) {
						ext.segp = (caddr_t)
							&macp->mac_rcf_f;
						ext.seglen =route_bytes_f(macp);
					} else {
						ext.segp = 0;	
						ext.seglen = 0;	
					}
					ext.isr_data = fddi_dmx_ctl->
						mac_user.isr_data;
					extp = &ext;
				} else
					extp = (struct isr_data_ext *)
						smt_user->user.isr_data;
				DELIVER_PACKET((&(smt_user->user)), nddp, 
					newm, newmacp, extp);
			}
		}

	}

	/*
	 * If any user got the packet, then bump the accepts stat.  
	 * Else bump the rejects stat.
	 */
	if (newm == NULL) {
		fddi_dmx_ctl->stats.nd_smt_rejects++;
		nddp->ndd_nsdemux->nd_dmxstats.nd_nofilter++;
	}
	else
		fddi_dmx_ctl->stats.nd_smt_accepts++;
	m_freem(m);
}
