static char sccsid[] = "@(#)20	1.17  src/bos/kernext/dmx/eth_demux.c, sysxdmx, bos41J, 9518A_all 5/1/95 14:37:31";
/*
 *   COMPONENT_NAME: SYSXDMX
 *
 *   FUNCTIONS: eth_add_demuxer
 *		eth_add_ethertype
 *		eth_add_filter
 *		eth_add_status
 *		eth_add_tap
 *		eth_config
 *		eth_del_ethertype
 *		eth_del_filter
 *		eth_del_status
 *		eth_del_tap
 *		eth_dmx_init
 *		eth_get_user
 *		eth_receive
 *		eth_response
 *		eth_status
 *		eth_std_receive
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
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/mbuf.h>
#include <sys/cdli.h>
#include <sys/ndd.h>
#include <net/if.h>
#include <net/netisr.h>
#include <net/nd_lan.h>
#include <net/spl.h>
#include <netinet/if_ether.h>
#include <aixif/net_if.h>
#include <sys/eth_demux.h>

struct ns_demuxer	demuxer;		/* Ethernet demuxer struct */
int			eth_users;		/* # of filters added */

/*
 * The following lock is used to seriallize:
 * 	Access to the static variable initialized (in eth_dmx_config).
 *	Access to eth_users.
 */
lock_t			cfg_lock = LOCK_AVAIL;	/* Config lock */

/*
 * Forward declarations.
 */
void eth_std_receive(ndd_t *, struct mbuf *, struct ether_header *);


#ifdef DEBUG
#define DBGMSG(m) bsdlog(0, m)
#else
#define DBGMSG(m)
#endif

/*
 * NAME: eth_config
 *                                                                    
 * FUNCTION: 
 * 	Kernel extension entry point to configure the ethernet demuxer.
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
 *      Ethernet demuxer control struct (struct eth_dmx_ctl) - internal struct
 *
 * ROUTINES CALLED:
 *	pincode
 *	eth_add_demuxer
 *
 * RETURNS:  
 *              0 - success
 *		EINVAL - invalid configure command
 *		rc from pincode
 *		rc from eth_add_demuxer
 */  
eth_config(cmd, uio)
int		cmd;
struct uio	*uio;
{
	int				error=0;
	static int 			initialized=0;

	if ( ((cmd != CFG_INIT) && (cmd != CFG_TERM)) )
		return(EINVAL);

	lockl(&cfg_lock, LOCK_SHORT);
	if (cmd == CFG_INIT) {
		DBGMSG("ETH_DEMUX: CFG_INIT called\n");
		if (initialized++ == 0) {
			error = pincode(eth_config);
			if (!error) {
				DBGMSG("ETH_DEMUX: pinned code ok\n");
				error = eth_add_demuxer();
				if (error) {
					DBGMSG("ETH_DEMUX: failed to add demuxer\n");
					unpincode(eth_config);
					initialized--;
				}
			} else
				initialized--;
		}
	} 
	else {
		DBGMSG("ETH_DEMUX: CFG_TERM called \n");
		if (initialized == 1) {
			if (eth_users == 0) {
				DBGMSG("ETH_DEMUX: removing demuxer\n");
				error = ns_del_demux(NDD_ISO88023);
				if (!error) {
					unpincode(eth_config);
					initialized = 0;
					DBGMSG("ETH_DEMUX: unpinned code\n");
				}
			} else {
				DBGMSG("ETH_DEMUX: cannot unload, users exist\n");
				error = EBUSY;
			}
		} else  if (initialized > 1)
			initialized--;
		else {
			error = EINVAL;
			DBGMSG("ETH_DEMUX: Bogus TERM...initialized <=0\n");
		}
	}
	unlockl(&cfg_lock);
	return(error);
}


/*
 * NAME: eth_add_filter
 *                                                                    
 * FUNCTION: 
 *      Process user request for adding filters.
 *      This routine handles both standard and 802.3 ethernet.
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
 *              NS_ETHERTYPE
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
 * 	eth_add_ethertype
 *	eth_add_tap
 *	eth_dmx_init
 *
 * RETURNS:  
 *              0       - successful
 *              ENOMEM  - Unable to allocate required memory
 *              EEXIST  - Filter already in use by someone else
 *		EALREADY- Filter already added by you!
 *              EINVAL  - Invalid filter type
 *              rc      - return codes from dmx_8022_add_filter()
 *              rc      - return codes from NDD's NDD_ADD_FILTER operation
 *              rc      - return codes from eth_add_ethertype
 *              rc      - return codes from eth_add_tap
 */  

eth_add_filter(nddp, filter, len, ns_user)
	ndd_t		*nddp;
	ns_8022_t	*filter;
	int		len;
	ns_user_t	*ns_user;
{
	int 	error;
	DEMUXER_LOCK_DECL();

	if (ns_user->isr == NULL && ns_user->netisr == 0)
		return(EINVAL);
	if (ns_user->protoq != NULL && ns_user->netisr == 0)
		return(EINVAL);
	if (len != sizeof(ns_8022_t))
		return(EINVAL);

	DEMUXER_LOCK(&nddp->ndd_demux_lock);

	/*
	 * If this is the first add, then initialize the ethernet control
	 * structure.
	 */
	if (nddp->ndd_specdemux == (caddr_t) NULL) {
		if (eth_dmx_init(nddp)) {
			DEMUXER_UNLOCK(&nddp->ndd_demux_lock);
			return(ENOMEM);
		}
	}

	switch (filter->filtertype) {
		case NS_8022_LLC_DSAP:
		case NS_8022_LLC_DSAP_SNAP:
			error = dmx_8022_add_filter(nddp, filter, ns_user);
			break;
		case NS_ETHERTYPE:
			error = eth_add_ethertype(nddp, filter, ns_user);
			break;
		case NS_TAP:
			error = eth_add_tap(nddp, ns_user);
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
			if (filter->filtertype == NS_ETHERTYPE)
				(void)eth_del_ethertype(nddp, filter);
			else
				(void)dmx_8022_del_filter(nddp, filter);
		} else
			error = 0;
	}
	if (!error) {
		eth_users++;
	}
	DEMUXER_UNLOCK(&nddp->ndd_demux_lock);
	return(error);
}

/*
 * NAME: eth_del_filter
 *                                                                    
 * FUNCTION: 
 *      Process user request for deleting filters.
 *      This routine will handle both standard and 802.3 ethernet
 *      filters.
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
 *              NS_ETHERTYPE
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
 * 	eth_del_ethertype
 *	eth_del_tap
 *
 * RETURNS:  
 *              0       - successful
 *              EINVAL  - Invalid filter type
 *              rc      - return codes from dmx_8022_del_filter()
 *		rc	- return codes from eth_del_ethertype
 *		rc	- return codes from eth_del_tap
 */  
eth_del_filter(nddp, filter, len)
	ndd_t		*nddp;
	ns_8022_t	*filter;
	int		len;
{
	int 			error;
	DEMUXER_LOCK_DECL();

	if (len != sizeof(ns_8022_t))
		return(EINVAL);

	DEMUXER_LOCK(&nddp->ndd_demux_lock);

	/* If there is no ethernet control structure in the NDD, then
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
		case NS_ETHERTYPE:
			error = eth_del_ethertype(nddp, filter);
			break;
		case NS_TAP:
			error = eth_del_tap(nddp);
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
	if (error == EOPNOTSUPP)
		error = 0;
	if (!error) {
		eth_users--;
		if (eth_users == 0) {
			NET_FREE(nddp->ndd_specdemux, M_TEMP);
			nddp->ndd_specdemux=(caddr_t)NULL;
		}
	}
	DEMUXER_UNLOCK(&nddp->ndd_demux_lock);
	return(error);
}

/*
 * NAME: eth_add_status
 *                                                                    
 * FUNCTION: 
 *      Process user request for adding status filters.
 *      This routine handles both standard and 802.3 ethernet.
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

eth_add_status(nddp, filter, len, ns_statuser)
	ndd_t			*nddp;
	ns_com_status_t		*filter;
	int			len;
	ns_statuser_t		*ns_statuser;
{
	int	error;
	DEMUXER_LOCK_DECL();

	if (ns_statuser->isr == NULL)
		return(EINVAL);
	if (len != sizeof(ns_com_status_t))
		return(EINVAL);

	DEMUXER_LOCK(&nddp->ndd_demux_lock);

	/*
	 * If this is the first add, then initialize the ethernet control
	 * structure.
	 */
	if (nddp->ndd_specdemux == (caddr_t) NULL) {
		if (eth_dmx_init(nddp)) {
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
		eth_users++;
	}
	DEMUXER_UNLOCK(&nddp->ndd_demux_lock);
	return(error);
}

/*
 * NAME: eth_del_status
 *                                                                    
 * FUNCTION: 
 *      Process user request for deleting status filters.
 *      This routine handles both standard and 802.3 ethernet.
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

eth_del_status(nddp, filter, len)
	ndd_t			*nddp;
	ns_com_status_t		*filter;
	int			len;
{
	int	error;
	DEMUXER_LOCK_DECL();

	DEMUXER_LOCK(&nddp->ndd_demux_lock);
	error = dmx_del_status(nddp, filter);
	if (!error) {
		DEMUXER_UNLOCK(&nddp->ndd_demux_lock);
		((*(nddp->ndd_ctl)) (nddp, NDD_DEL_STATUS, filter, len));
		DEMUXER_LOCK(&nddp->ndd_demux_lock);
		if (error == EOPNOTSUPP)
			error = 0;
		if (!error) {
			eth_users--;
			if (eth_users == 0) {
				NET_FREE(nddp->ndd_specdemux, M_TEMP);
				nddp->ndd_specdemux=(caddr_t)NULL;
			}
		}
	}
	DEMUXER_UNLOCK(&nddp->ndd_demux_lock);
	return(error);
}

/*
 * NAME: eth_get_user
 *                                                                    
 * FUNCTION: 
 * 	Returns the ethernet user given the ndd and ethertype.
 *                                                                    
 * EXECUTION ENVIRONMENT: process
 *                                                                   
 * NOTES: 
 *
 * RECOVERY OPERATION: 
 *      None.
 *
 * DATA STRUCTURES: 
 *      Common status filter struct (ns_com_status_t)
 *      NDD struct (ndd_t)
 *      NS status user struct (ns_statuser_t)
 *      Ethernet demuxer control struct (struct eth_dmx_ctl) - internal struct
 *      Ethernet user struct (struct eth_user) - internal struct
 *
 * ROUTINES CALLED:
 *      None.
 *
 * RETURNS:  
 *              (struct eth_user *)NULL	- User not found.
 *              (struct eth_user *)userp - pointer to user structure.
 */  

struct eth_user	*
eth_get_user(etype, nddp)
	u_short		etype;
	ndd_t		*nddp;
{
	struct eth_user		*filter_head;
	struct eth_user		*fup;
	struct eth_dmx_ctl	*eth_dmx_ctl;

	eth_dmx_ctl = (struct eth_dmx_ctl *) nddp->ndd_specdemux;
	filter_head = &eth_dmx_ctl->hash_heads[etype % ETHDMX_HASH];
	for ( fup = filter_head->next; fup != filter_head; fup = fup->next) {
		if (fup->filter.ethertype == etype)
			return(fup);
	}
	return(NULL);
}

/*
 * NAME: eth_add_ethertype
 *                                                                    
 * FUNCTION: 
 * 	Adds a NS_ETHERTYPE filter.
 *                                                                    
 * EXECUTION ENVIRONMENT: process
 *                                                                   
 * NOTES: 
 *
 * RECOVERY OPERATION: 
 *      None.
 *
 * DATA STRUCTURES: 
 *      NDD struct (ndd_t)
 *      NS status user struct (ns_statuser_t)
 *      Ethernet demuxer control struct (struct eth_dmx_ctl) - internal struct
 *      Ethernet user struct (struct eth_user) - internal struct
 *
 * ROUTINES CALLED:
 *      NET_MALLOC
 * 	eth_get_user
 *	insqueue
 *	NET_FREE
 *
 * RETURNS:  
 *              0 - success
 *              ENOMEM - no mem
 *		EXIST - already added by another user
 *		EALREADY - already added by you!
 */  
eth_add_ethertype(nddp, filter, ns_user)
	ndd_t		*nddp;
	ns_8022_t	*filter;
	ns_user_t	*ns_user;
{
	struct eth_user		*new_user, *fup;
	struct eth_dmx_ctl	*eth_dmx_ctl;

	NET_MALLOC(new_user, struct eth_user *, sizeof(struct eth_user),
		M_TEMP, M_WAITOK); 
			
	if (new_user == (struct eth_user *) NULL)
		return(ENOMEM);
	
	bzero(new_user, sizeof(struct eth_user));
	new_user->user = *ns_user;
	new_user->filter = *filter;

	if ((fup=eth_get_user(filter->ethertype, nddp))) {
		NET_FREE(new_user, M_TEMP);
		if (!bcmp((caddr_t)&fup->user,(caddr_t)ns_user, 
			sizeof(ns_user_t)))
			return(EALREADY);
		else
			return(EEXIST);
	}
	eth_dmx_ctl = (struct eth_dmx_ctl *) nddp->ndd_specdemux;
	insque(new_user, 
	       eth_dmx_ctl->hash_heads[filter->ethertype % ETHDMX_HASH].prev);
	return(0);
}

/*
 * NAME: eth_del_ethertype
 *                                                                    
 * FUNCTION: 
 * 	Adds a NS_ETHERTYPE filter.
 *                                                                    
 * EXECUTION ENVIRONMENT: process
 *                                                                   
 * NOTES: 
 *
 * RECOVERY OPERATION: 
 *      None.
 *
 * DATA STRUCTURES: 
 *      NDD struct (ndd_t)
 *      NS status user struct (ns_statuser_t)
 *      Ethernet demuxer control struct (struct eth_dmx_ctl) - internal struct
 *      Ethernet user struct (struct eth_user) - internal struct
 *
 * ROUTINES CALLED:
 * 	eth_get_user
 *	remque
 *	NET_FREE
 *
 * RETURNS:  
 *              0 - success
 *              ENOENT - did not exist
 */  
eth_del_ethertype(nddp, filter)
	ndd_t		*nddp;
	ns_8022_t	*filter;
{
	struct eth_user		*fup;

	fup = eth_get_user(filter->ethertype, nddp);
	if (fup == NULL) {
		return(ENOENT);
	}
	remque(fup);
	NET_FREE(fup, M_TEMP);
	return(0);
}


/*
 * NAME: eth_add_tap
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
 *      Ethernet demuxer control struct (struct eth_dmx_ctl) - internal struct
 *
 * ROUTINES CALLED:
 *	None.
 *
 * RETURNS:  
 *              0 - success
 *		EXIST - already exists
 */  
eth_add_tap(nddp, ns_user)
	ndd_t		*nddp;
	ns_user_t	*ns_user;
{
	struct eth_dmx_ctl	*eth_dmx_ctl;

	eth_dmx_ctl = (struct eth_dmx_ctl *) nddp->ndd_specdemux;
	if (eth_dmx_ctl->tap_user.isr != NULL)
		return(EEXIST);

	eth_dmx_ctl->tap_user = *ns_user;	
	return(0);
}


/*
 * NAME: eth_del_tap
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
 *      Ethernet demuxer control struct (struct eth_dmx_ctl) - internal struct
 *
 * ROUTINES CALLED:
 *	None.
 *
 * RETURNS:  
 *              0 - success
 *		ENOENT - no tap user exists
 */  
eth_del_tap(nddp)
	ndd_t		*nddp;
{
	struct eth_dmx_ctl	*eth_dmx_ctl;

	eth_dmx_ctl = (struct eth_dmx_ctl *) nddp->ndd_specdemux;
	if (eth_dmx_ctl->tap_user.isr == NULL)
		return(ENOENT);

	bzero(&eth_dmx_ctl->tap_user, sizeof(eth_dmx_ctl->tap_user));
	return(0);
}

/*
 * NAME: eth_receive
 *                                                                    
 * FUNCTION: 
 *      Process incoming Std. Ethernet and 802.3 packets from a NDD.  
 *      The packets are routed to the appropriate users.
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt
 *                                                                   
 * NOTES: 
 *      This routine is called with a ptr to a NDD struct (nddp) and 
 *      an mbuf ptr (m).  The mbuf chain may contain more than 1
 *      pkt.
 *
 *      This function must demux the following types of Ethernet pkts:
 *              Std. Ethernet LLC frames - 48 bit address
 *              802.3 LLC frames - 48 bit address 
 *                      with or without SNAP
 *
 * RECOVERY OPERATION: 
 *      None.
 *
 * DATA STRUCTURES: 
 *      Ethernet packet header (eth_hdr_t)
 *      NDD structure (ndd_t)
 *      mbuf structure (struct mbuf)
 *      Ethernet demuxer user struct (struct eth_user) - internal struct
 *      Ethernet demuxer control struct (struct eth_dmx_ctl) - internal struct
 *
 *
 * ROUTINES CALLED:
 *      Common 802.2 demuxer routine (dmx_8022_receive)
 *	eth_std_receive (demuxing NS_ETHERTYPE packets)
 *      m_freem
 *
 * RETURNS:  
 *              none
 */  
void
eth_receive(nddp, m)
	ndd_t		*nddp;
	struct mbuf	*m;
{
	struct mbuf		*mnextpkt;
	struct ether_header	*eh;
	struct eth_dmx_ctl	*eth_dmx_ctl;
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
	eth_dmx_ctl = (struct eth_dmx_ctl *)nddp->ndd_specdemux;

	/* While more packets, demux! */
	while ( m != (struct mbuf *) NULL ) {
		mnextpkt = m->m_nextpkt;
		m->m_nextpkt = (struct mbuf *) NULL;
		eh = mtod(m, struct ether_header *);

		/* strip 802.3 padding */
		if (eh->ether_type <= ETHERMTU) {
			ushort len = eh->ether_type + sizeof(*eh);
			if (len < m->m_pkthdr.len) {
				m->m_pkthdr.len = len;
				if (m->m_len < len) {
					m = m_pullup(m, len);
					if (!m) {
						++nddp->ndd_nsdemux->
							nd_dmxstats.nd_nobufs;
						return;
					}
				}
				m->m_len = len;
			}
		}

		/*
		 * Give to tap user if one exists.  Note that the tap user
		 * must copy the packet (we retain ownership of mbuf chain).
	 	 */
		if (eth_dmx_ctl->tap_user.isr != NULL) {

			if (eth_dmx_ctl->tap_user.pkt_format & 
				NS_HANDLE_HEADERS) {
				ext.dstp = eh->ether_dhost;
				ext.srcp = eh->ether_shost;
				ext.dstlen = ext.srclen = ETHER_ADDRLEN;
				ext.segp = 0;
				ext.seglen = 0;
				if (eh->ether_type > ETHERMTU)
					ext.llcp = (caddr_t)&(eh->ether_type);
				else
					ext.llcp = (caddr_t)eh + sizeof(*eh);
				ext.llclen = 0;
				ext.isr_data = eth_dmx_ctl->tap_user.isr_data;
				extp = &ext;
			} else
				extp = (struct isr_data_ext *)eth_dmx_ctl->
					tap_user.isr_data;
			(*(eth_dmx_ctl->tap_user.isr))(nddp, m, eh, extp);
		}

		/* If the NDD is in promicsuous mode, then we need to
		 * make sure this packet is really for us.
	 	 */
		if (nddp->ndd_flags & NDD_PROMISC) {
		     if  ((m->m_flags & (M_BCAST|M_MCAST)) == 0) {
			if (memcmp((caddr_t)nddp->ndd_physaddr, 
			      (caddr_t)eh->ether_dhost, ETHER_ADDRLEN)) {
				m_freem(m);
				m = mnextpkt;
				continue;
			}
		     }
		}
		if (eh->ether_type > ETHERMTU)
			eth_std_receive(nddp, m, eh);
		else {
			ext.dstp = eh->ether_dhost;
			ext.srcp = eh->ether_shost;
			ext.dstlen = ext.srclen = ETHER_ADDRLEN;
			ext.segp = 0;
			ext.seglen = 0;
			dmx_8022_receive(nddp, m, sizeof(struct ie3_mac_hdr), 
				&ext);
		}

		m = mnextpkt;
	}
	DEMUXER_UNLOCK(&nddp->ndd_demux_lock);
	return;
}


/*
 * NAME: eth_status
 *                                                                    
 * FUNCTION: 
 * 	Handles demuxing status interrupts.
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt
 *                                                                   
 * NOTES: 
 * 	Simple calls dmx_status
 *
 * RECOVERY OPERATION: 
 *      None.
 *
 * DATA STRUCTURES: 
 *      NDD struct (ndd_t)
 *      NS status block struct (ns_statblk_t)
 *
 * ROUTINES CALLED:
 *	dmx_status.
 *
 * RETURNS:  
 *              None.
 */  
void
eth_status(nddp, statusp)
ndd_t		*nddp;
ndd_statblk_t	*statusp;
{
	DEMUXER_LOCK_DECL();
	DEMUXER_LOCK(&nddp->ndd_demux_lock);
	dmx_status(nddp, statusp);
	DEMUXER_UNLOCK(&nddp->ndd_demux_lock);
}


/*
 * NAME: eth_std_receive
 *                                                                    
 * FUNCTION: 
 * 	Demuxes NS_ETHERTYPE packets.
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt
 *                                                                   
 * NOTES: 
 *
 * RECOVERY OPERATION: 
 *      None.
 *
 * DATA STRUCTURES: 
 *      NDD struct (ndd_t)
 *      mbuf struct (packet)
 *      Ethernet demuxer control struct (struct eth_dmx_ctl) - internal struct
 *	Ethernet user struct (struct eth_user).
 *
 * ROUTINES CALLED:
 *	None.
 *
 * RETURNS:  
 *		None.
 */  
void
eth_std_receive(nddp, m, eh)
	ndd_t			*nddp;
	struct mbuf		*m;
	struct ether_header	*eh;
{
	struct eth_user		*fup;
	struct eth_user		*filter_head;
	struct eth_dmx_ctl	*eth_dmx_ctl;
	struct isr_data_ext	ext, *extp;

	/* 
	 * Hash in on ether type, then walk the linked list of users til
	 * you find the right match.
	 */
	eth_dmx_ctl = (struct eth_dmx_ctl *) nddp->ndd_specdemux;
	filter_head = &eth_dmx_ctl->hash_heads[eh->ether_type % ETHDMX_HASH];
	
	for ( fup = filter_head->next; fup != filter_head; fup = fup->next) {
		if (fup->filter.ethertype == eh->ether_type) {
			if (!(fup->user.pkt_format & NS_INCLUDE_MAC)) {
				m->m_pkthdr.len -= sizeof(struct ether_header);
				m->m_len -= sizeof(struct ether_header);
				m->m_data += sizeof(struct ether_header);
			}
			if (fup->user.pkt_format & NS_HANDLE_HEADERS) {
				ext.dstp = eh->ether_dhost;
				ext.srcp = eh->ether_shost;
				ext.dstlen = ext.srclen = ETHER_ADDRLEN;
				ext.segp = 0;
				ext.seglen = 0;
				ext.llcp = (caddr_t)&(eh->ether_type);
				ext.llclen = sizeof(eh->ether_type);
				ext.isr_data = fup->user.isr_data;
				extp = &ext;
			} else
				extp = (struct isr_data_ext *)fup->
					user.isr_data;
			IFSTUFF_AND_DELIVER(fup, nddp, m, eh, extp);
			eth_dmx_ctl->stats.nd_ethertype_accepts++;
			return;
		}
	}
	
	eth_dmx_ctl->stats.nd_ethertype_rejects++;
	nddp->ndd_nsdemux->nd_dmxstats.nd_nofilter++;
	m_freem(m);
	return;
}


/*
 * NAME: eth_response
 *                                                                    
 * FUNCTION: 
 *      Adjust the destination and source address in a frame so 
 *      that the frame can be transmitted to the originator
 *      of the frame.  (swap src and dst hwaddrs)
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt or process
 *                                                                   
 * NOTES: 
 *      This routine will not handle multiple packets.  
 *
 * RECOVERY OPERATION: 
 *      None.
 *
 * DATA STRUCTURES: 
 *      Mbuf structure (struct mbuf)
 *      NDD (ndd_t)
 *
 *
 * ROUTINES CALLED:
 *      NDD output routine (nddp->ndd_output)
 *
 * RETURNS:  
 *              none 
 */  

void
eth_response(nddp, m, llcoffset)
	ndd_t		*nddp;
	struct mbuf	*m;
	int		llcoffset;
{
	struct ie3_mac_hdr		*ie3macp;

	ie3macp = mtod(m, struct ie3_mac_hdr *);
	bcopy(ie3macp->ie3_mac_src, ie3macp->ie3_mac_dst, ETHER_ADDRLEN);
	bcopy(nddp->ndd_physaddr, ie3macp->ie3_mac_src, ETHER_ADDRLEN);
	if ((*(nddp->ndd_output))(nddp, m))
			m_freem(m);
	return;
}


/*
 * NAME: eth_dmx_init
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
 *      Ethernet demuxer control struct (struct eth_dmx_ctl) - internal struct
 *
 * ROUTINES CALLED:
 *	NET_MALLOC
 *
 * RETURNS:  
 *              0 - success
 *		ENOMEM - no mem
 */  
eth_dmx_init(nddp) 
	ndd_t		*nddp;
{
	struct eth_dmx_ctl	*eth_dmx_ctl;
	int			i;

	NET_MALLOC(eth_dmx_ctl, struct eth_dmx_ctl *, 
		   sizeof(struct eth_dmx_ctl), M_TEMP, M_WAITOK); 
	if (eth_dmx_ctl == (struct eth_dmx_ctl *) NULL)
		return(ENOMEM);
	bzero((caddr_t)eth_dmx_ctl, sizeof (*eth_dmx_ctl));
	nddp->ndd_specdemux = (caddr_t) eth_dmx_ctl;
	for (i = 0 ; i < ETHDMX_HASH ; i++) {
		eth_dmx_ctl->hash_heads[i].next = 
			&eth_dmx_ctl->hash_heads[i];
		eth_dmx_ctl->hash_heads[i].prev = 
			&eth_dmx_ctl->hash_heads[i];
	}
	return(0);
}


void
eth_dmx_address_input(eh)
struct ether_header *eh;
{
}

int
eth_dmx_address_resolve(obp, m, nddp)
struct output_bundle	*obp;
struct mbuf 		*m;
struct ndd 		*nddp; 
{
	struct ie3_mac_hdr *macp;
	struct ie2_llc_snaphdr *llcp;
	struct ether_header *eh;
	int rc;

	if (obp->helpers.ethertype) {
		M_PREPEND(m, sizeof(struct ether_header), M_DONTWAIT);
		if (m == 0) {
			nddp->ndd_nsdemux->nd_dmxstats.nd_nobufs++;
			return(ENOBUFS);
		}
		eh = mtod(m, struct ether_header *);
		eh->ether_type = obp->helpers.ethertype;
		bcopy(obp->key_to_find, eh->ether_dhost, ETHER_ADDRLEN);
		bcopy(nddp->ndd_physaddr, eh->ether_shost, ETHER_ADDRLEN);
		rc = (*(nddp->ndd_output))(nddp, m); 
		if (rc)
			m_freem(m);
                return(rc);
	} else {
		short datalen = (short)m->m_pkthdr.len;

		switch(obp->helpers.pkt_format & 0xFF) {

			case NS_PROTO:
				M_PREPEND(m, sizeof(struct ie3_mac_hdr) +
					sizeof(struct ie2_llc_hdr), M_DONTWAIT);
				if (m == 0) {
					nddp->ndd_nsdemux->nd_dmxstats.
						nd_nobufs++;
					return(ENOBUFS);
				}
				macp = mtod(m, struct ie3_mac_hdr *);
				datalen += sizeof(struct ie2_llc_hdr);
				llcp = (struct ie2_llc_snaphdr *)(macp+1);
				llcp->dsap = obp->helpers.sapu.llc.dsap;
				llcp->ssap = obp->helpers.sapu.llc.ssap;
				llcp->ctrl = obp->helpers.sapu.llc.ctrl;
				break;
					
			case NS_PROTO_SNAP:
				M_PREPEND(m, sizeof(struct ie3_mac_hdr) +
					sizeof(struct ie2_llc_snaphdr), 
					M_DONTWAIT);
				if (m == 0) {
					nddp->ndd_nsdemux->nd_dmxstats.
						nd_nobufs++;
					return(ENOBUFS);
				}
				macp = mtod(m, struct ie3_mac_hdr *);
				datalen += sizeof(struct ie2_llc_snaphdr);
				llcp = (struct ie2_llc_snaphdr *)(macp+1);
				llcp->dsap = obp->helpers.sapu.llc.dsap;
				llcp->ssap = obp->helpers.sapu.llc.ssap;
				llcp->ctrl = obp->helpers.sapu.llc.ctrl;
				bcopy(obp->helpers.sapu.llcsnap.prot_id, 
					llcp->prot_id, 3);
				llcp->type = obp->helpers.sapu.llcsnap.type;
				break;

			case NS_INCLUDE_LLC:
				M_PREPEND(m, sizeof(struct ie3_mac_hdr), 
					M_DONTWAIT);
				if (m == 0) {
					nddp->ndd_nsdemux->
						nd_dmxstats.nd_nobufs++;
					return(ENOBUFS);
				}
				macp = mtod(m, struct ie3_mac_hdr *);
				break;
			default:
				return(EINVAL);
		}

		bcopy(obp->key_to_find, macp->ie3_mac_dst, ETHER_ADDRLEN);
		bcopy(nddp->ndd_physaddr, macp->ie3_mac_src, ETHER_ADDRLEN);
		macp->ie3_mac_len = datalen;
		rc = (*(nddp->ndd_output))(nddp, m); 
		if (rc)
			m_freem(m);
                return(rc);
	}
}


/*
 * NAME: eth_add_demuxer
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
eth_add_demuxer()
{
	int error;

	bzero(&demuxer, sizeof(struct ns_demuxer));
	demuxer.nd_add_filter = eth_add_filter;
	demuxer.nd_del_filter = eth_del_filter;
	demuxer.nd_add_status = eth_add_status;
	demuxer.nd_del_status = eth_del_status;
	demuxer.nd_receive = eth_receive;
	demuxer.nd_status = eth_status;
	demuxer.nd_response = eth_response;
	demuxer.nd_use_nsdmx = 1;
	demuxer.nd_address_input = eth_dmx_address_input;
	demuxer.nd_address_resolve = eth_dmx_address_resolve;
	return(ns_add_demux(NDD_ISO88023, &demuxer));
}
