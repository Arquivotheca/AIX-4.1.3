static char sccsid[] = "@(#)24	1.18  src/bos/kernext/dmx/tok_demux.c, sysxdmx, bos41J, 9516B_all 4/14/95 10:08:51";
/*
 *   COMPONENT_NAME: SYSXDMX
 *
 *   FUNCTIONS: tok_add_demuxer
 *		tok_add_filter
 *		tok_add_mac
 *		tok_add_status
 *		tok_add_tap
 *		tok_arptimer
 * 		tok_arp_init
 *		tok_config
 *		tok_del_filter
 *		tok_del_mac
 *		tok_del_status
 *		tok_del_tap
 * 		tok_dmx_address_input
 * 		tok_dmx_address_resolve
 *		tok_dmx_init
 *		tok_mac_receive
 *		tok_receive
 *		tok_response
 *		tok_status
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
/* Ugh These include files need to be fixed. */
#include <net/net_globals.h>
#include <net/net_malloc.h>
#include <net/nd_lan.h>
#include <sys/ndd.h>
#include <net/spl.h>
#include <net/netisr.h>
#include <net/if_arp.h>
#include <netinet/if_802_5.h>
#include <sys/tok_demux.h>

#define	FCF_MASK	0xC0
#define	MAC_FRAME	0x00

struct	ns_demuxer	demuxer;		/* Demuxer struct */
int                     tok_users;		/* number of filters */

struct	arptab		*tok_arptabp;		/* Default arp table */
struct	trb		*tok_arptime = NULL;
int 	tok_arptabbsiz;
int 	tok_arptabnb;
#define TOK_ARPTAB_HASH(a) ((u_long) (a) % tok_arptabnb)
#if	NETSYNC_LOCK
Simple_lock	tok_arptab_lock;
#endif
#define ARPTAB_LOCK_DECL(s) int s;

/*
 * The following lock is used to seriallize:
 * 	Access to the static variable initialized (in tok_config).
 *	Access to tok_users.
 */
lock_t			cfg_lock = LOCK_AVAIL;

/*
 * Forward declarations.
 */
void
tok_mac_receive(ndd_t *, struct mbuf *, struct ie5_mac_hdr *);
int tok_arp_init(void);
void	tok_arptimer(void);

#ifdef DMXDEBUG
#define DBGMSG(m) bsdlog(0, m)
#else
#define DBGMSG(m)
#endif

/*
 * NAME: tok_config
 *                                                                    
 * FUNCTION: 
 * 	Kernel extension entry point to configure the tokernet demuxer.
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
 *      Token Ring demuxer control struct (struct tok_dmx_ctl) - internal struct
 *
 * ROUTINES CALLED:
 *	pincode
 *	tok_add_demuxer
 *
 * RETURNS:  
 *              0 - success
 *		EINVAL - invalid configure command
 *		rc from pincode
 *		rc from tok_add_demuxer
 */  
tok_config(cmd, uio)
int		cmd;
struct uio	*uio;
{
	int				error=0;
	static int 			initialized=0;
	ARPTAB_LOCK_DECL(s)

	if ( ((cmd != CFG_INIT) && (cmd != CFG_TERM)) )
		return(EINVAL);

	lockl(&cfg_lock, LOCK_SHORT);
	if (cmd == CFG_INIT) {
		DBGMSG("TOK_DEMUX: CFG_INIT called\n");
		if (initialized++ == 0) {
			if (!(tok_arptime = talloc()))
				error = ENOMEM;
			if (!error)
				error = pincode(tok_config);
			if (!error) {
				error = tok_add_demuxer();
				if (error) {
					unpincode(tok_config);
					initialized--;
				} else
					DBGMSG("TOK_DEMUX: configured cule\n");
			} else 
				initialized--;
			if (!initialized && tok_arptime)
				tfree(tok_arptime);
		}
	} 
	else {
		if (initialized == 1) {
			if (tok_users == 0) {
				error = ns_del_demux(NDD_ISO88025);
				if (!error) {
					while (tstop(tok_arptime));
					ARPTAB_LOCK(&tok_arptab_lock);
					if (tok_arptabp)
						NET_FREE(tok_arptabp, M_KTABLE);
					tok_arptabp = NULL;
					ARPTAB_UNLOCK(&tok_arptab_lock);
					lock_free(&tok_arptab_lock);
					initialized = 0;
					unpincode(tok_config);
					if (tok_arptime)
						tfree(tok_arptime);
				}
			} else {
				DBGMSG("TOK_DEMUX: cannot unload, users exist\n");
				error = EBUSY;
			}
		} else if (initialized > 1)
			initialized--;
		else {
			error = EINVAL;
		}
	}
	unlockl(&cfg_lock);
	return(error);
}


/*
 * NAME: tok_add_filter
 *                                                                    
 * FUNCTION: 
 *      Process user request for adding filters.
 *      This routine handles both standard and 802.3 tokernet.
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
 *              TOK_DEMUX_MAC
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
 * 	tok_add_tokertype
 *	tok_add_tap
 *	tok_dmx_init
 *
 * RETURNS:  
 *              0       - successful
 *              ENOMEM  - Unable to allocate required memory
 *              EEXIST  - Filter already in use
 *              EALREADY  - Filter already in added by this user.
 *              EINVAL  - Invalid filter type
 *              rc      - return codes from dmx_8022_add_filter()
 *              rc      - return codes from NDD's NDD_ADD_FILTER operation
 *              rc      - return codes from tok_add_tokertype
 *              rc      - return codes from tok_add_tap
 */  
tok_add_filter(nddp, filter, len, ns_user)
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
	if (len != sizeof(struct ns_8022))
		return(EINVAL);

	DEMUXER_LOCK(&nddp->ndd_demux_lock);

	/*
	 * If this is the first add, then initialize the token ring control
	 * structure.
	 */
	if (nddp->ndd_specdemux == (caddr_t) NULL) {
		if (tok_dmx_init(nddp)) {
			DEMUXER_UNLOCK(&nddp->ndd_demux_lock);
			return(ENOMEM);
		}
	}
	switch (filter->filtertype) {
		case NS_8022_LLC_DSAP:
		case NS_8022_LLC_DSAP_SNAP:
			error = dmx_8022_add_filter(nddp, filter, ns_user);
			break;
		case TOK_DEMUX_MAC:
			error = tok_add_mac(nddp, ns_user);
			break;
		case NS_TAP:
			error = tok_add_tap(nddp, ns_user);
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
			if (filter->filtertype == TOK_DEMUX_MAC)
				(void)tok_del_mac(nddp);
			else
				(void)dmx_8022_del_filter(nddp, filter);
		} else
			error = 0;
	}
        if (!error) {
                tok_users++;
	}
	DEMUXER_UNLOCK(&nddp->ndd_demux_lock);
	return(error);
}

/*
 * NAME: tok_del_filter
 *                                                                    
 * FUNCTION: 
 *      Process user request for deleting filters.
 *      This routine will handle both standard and 802.3 tokernet
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
 * 	tok_del_tokertype
 *	tok_del_tap
 *
 * RETURNS:  
 *              0       - successful
 *              EINVAL  - Invalid filter type
 *              rc      - return codes from dmx_8022_del_filter()
 *		rc	- return codes from tok_del_tokertype
 *		rc	- return codes from tok_del_tap
 */  
tok_del_filter(nddp, filter, len)
	struct ndd		*nddp;
	struct ns_8022		*filter;
	int			len;
{
	int 	error;
	DEMUXER_LOCK_DECL();

	if (len != sizeof(struct ns_8022))
		return(EINVAL);

	DEMUXER_LOCK(&nddp->ndd_demux_lock);

	/* If there is no token ring control structure in the NDD, then
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
		case TOK_DEMUX_MAC:
			error = tok_del_mac(nddp);
			break;
		case NS_TAP:
			error = tok_del_tap(nddp);
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
                tok_users--;
		if (tok_users == 0) {
			NET_FREE(nddp->ndd_specdemux, M_TEMP);
			nddp->ndd_specdemux=(caddr_t) NULL;
		}
	}
	DEMUXER_UNLOCK(&nddp->ndd_demux_lock);
	return(error);
}

/*
 * NAME: tok_add_status
 *                                                                    
 * FUNCTION: 
 *      Process user request for adding status filters.
 *      This routine handles both standard and 802.3 tokernet.
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
tok_add_status(nddp, filter, len, ns_statuser)
	struct ndd		*nddp;
	struct ns_com_status	*filter;
	int			len;
	struct ns_statuser	*ns_statuser;
{
	int	error;
	DEMUXER_LOCK_DECL();

	DEMUXER_LOCK(&nddp->ndd_demux_lock);

	/*
	 * If this is the first add, then initialize the token ring control
	 * structure.
	 */
	if (nddp->ndd_specdemux == (caddr_t) NULL) {
		if (tok_dmx_init(nddp)) {
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
		tok_users++;
	}
	DEMUXER_UNLOCK(&nddp->ndd_demux_lock);
	return(error);
}

/*
 * NAME: tok_del_status
 *                                                                    
 * FUNCTION: 
 *      Process user request for deleting status filters.
 *      This routine handles both standard and 802.3 tokernet.
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
tok_del_status(nddp, filter, len)
	struct ndd		*nddp;
	struct ns_com_status	*filter;
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
		tok_users--;
		if (tok_users == 0) {
			NET_FREE(nddp->ndd_specdemux, M_TEMP);
			nddp->ndd_specdemux=(caddr_t) NULL;
		}
	}
	DEMUXER_UNLOCK(&nddp->ndd_demux_lock);
	return(error);
}

/*
 * NAME: tok_receive
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
 *      This function must demux the following types of token ring pkts:
 *              LLC frames - 48 bit address
 *                      with or without SNAP
 *              MAC frames - 48 bit address 
 *
 * RECOVERY OPERATION: 
 *      None.
 *
 * DATA STRUCTURES: 
 *      Token ring packet header (struct ie5_hdr)
 *      NDD structure (ndd_t)
 *      mbuf structure (struct mbuf)
 *      Token ring demuxer ctl struct (struct tok_dmx_ctl) - internal struct
 *	
 * ASSUMPTION:  the mac_header is ALWAYS in the first mbuf of a chain.
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
tok_receive(nddp, m)
	struct ndd		*nddp;
	struct mbuf		*m;
{
        struct ie5_mac_hdr     	*macp;
	struct mbuf		*mnextpkt;
	struct tok_dmx_ctl	*tok_dmx_ctl;
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
	tok_dmx_ctl = (struct tok_dmx_ctl *)nddp->ndd_specdemux;
	while ( m != (struct mbuf *) NULL ) {
		mnextpkt = m->m_nextpkt;
		m->m_nextpkt = (struct mbuf *) NULL;
		macp = mtod(m, struct ie5_mac_hdr *);

		/*
		 * Give to tap user if one exists.  Note that the tap user
		 * must copy the packet (we retain ownership of mbuf chain).
	 	 */
		if (tok_dmx_ctl->tap_user.isr != NULL) {
			if (tok_dmx_ctl->tap_user.pkt_format & 
				NS_HANDLE_HEADERS) {

				ext.dstp = macp->mac_dst_802_5;
				ext.srcp = macp->mac_src_802_5;
				ext.dstlen = ext.srclen = IE8025_ADDRLEN;
				if (has_route(macp)) {
					ext.segp = (caddr_t)
						&macp->mac_rcf;
					ext.seglen = route_bytes(macp);
				} else {
					ext.segp = 0;	
					ext.seglen = 0;	
				}
				ext.isr_data = tok_dmx_ctl->tap_user.isr_data;
				ext.llcp = (caddr_t)macp + mac_size(macp);
				ext.llclen = 0;
				extp = &ext;
			} else
				extp = (struct isr_data_ext *)
					tok_dmx_ctl->tap_user.isr_data;
			(*(tok_dmx_ctl->tap_user.isr)) (nddp, m, macp, extp);
		}

		if ((macp->mac_fcf & FCF_MASK) == MAC_FRAME) {
			tok_mac_receive(nddp, m, macp);
		} else {
			if (nddp->ndd_flags & NDD_PROMISC) {
			     if  ((m->m_flags & (M_BCAST|M_MCAST)) == 0) {
				if (bcmp((caddr_t)nddp->ndd_physaddr,
				   	(caddr_t)macp->mac_dst_802_5, 
					nddp->ndd_addrlen)) {
					m_freem(m);
					m = mnextpkt;
					continue;
				}
			     }
			}

			ext.dstp = macp->mac_dst_802_5;
			ext.srcp = macp->mac_src_802_5;
			ext.dstlen = ext.srclen = IE8025_ADDRLEN;
			if (has_route(macp)) {
				ext.segp = (caddr_t)&macp->mac_rcf;
				ext.seglen = route_bytes(macp);
			} else {
				ext.segp = 0;	
				ext.seglen = 0;	
			}
			
			dmx_8022_receive(nddp, m, mac_size(macp), &ext);
		}
		m = mnextpkt;
	}
	DEMUXER_UNLOCK(&nddp->ndd_demux_lock);
	return;
}


/*
 * NAME: tok_response
 *                                                                    
 * FUNCTION: 
 *      Adjust the destination and source address in a frame so 
 *      that the frame can be transmitted to the originator
 *      of the frame.  Place src addr in dst, and our addr in src.
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
tok_response(nddp, m, llcoffset)
	struct ndd		*nddp;
	struct mbuf		*m;
	int			llcoffset;
{
	struct ie5_mac_hdr		*ie5macp;
	caddr_t				llcp;

	ie5macp = mtod(m, struct ie5_mac_hdr *);
	llcp = (caddr_t)ie5macp + llcoffset;
	bcopy(ie5macp->mac_src_802_5, ie5macp->mac_dst_802_5, 
		IE8025_ADDRLEN);
	bcopy((caddr_t)nddp->ndd_physaddr, ie5macp->mac_src_802_5, 
		IE8025_ADDRLEN);

	/* 
	 * optional information field
	 * preserve the routing info if needed
	 */
	ie5macp->mac_src_802_5[0] |= ie5macp->mac_dst_802_5[0] & RI_PRESENT;
	ie5macp->mac_dst_802_5[0] &= ~RI_PRESENT;
	
	/*
	 * Reset the acf byte.
	 */
	ie5macp->mac_acf &= 0x07;
	if (has_route(ie5macp)) {
		ie5macp->mac_rcf ^=  RCF_DIRECTION;
		ie5macp->mac_rcf &= ~(RCF_ALL_BROADCAST | RCF_LOCAL_BROADCAST);
	}

	if ((*(nddp->ndd_output))(nddp, m))
		m_freem(m);
	return;
}

/*
 * NAME: tok_status
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
tok_status(nddp, statusp)
struct ndd		*nddp;
struct ndd_statblk	*statusp;
{
	DEMUXER_LOCK_DECL();
	DEMUXER_LOCK(&nddp->ndd_demux_lock);
	dmx_status(nddp, statusp);
	DEMUXER_UNLOCK(&nddp->ndd_demux_lock);
}

/*
 * NAME: tok_dmx_address_input
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
 *      ie5_mac_hdr struct
 *
 * ROUTINES CALLED:
 *	tok_arp_init
 *
 * RETURNS:  
 *              None.
 */  
void
tok_dmx_address_input(mac, arp)
struct ie5_mac_hdr *mac;
int arp;
{
	register struct arptab *at, *ato;
	u_long src;
	int n;
	u_char o_brother[IE8025_ADDRLEN];
	ARPTAB_LOCK_DECL(s)

	/* do no arping unless requested */
	if (!arp)
		return;

	ARPTAB_LOCK(&tok_arptab_lock);
	if (!tok_arptabp)  /* Not initialized. Set it up. One long interrupt. */
		if (tok_arp_init()) {/* We'll try it again on another packet. */
			ARPTAB_UNLOCK(&tok_arptab_lock);
			return;
		}

	if (has_route(mac)) {

		/*
		 * reverse the direction of the route, and remove
		 * any broadcast bits.
		 */
		mac->mac_rcf ^=  RCF_DIRECTION;
		mac->mac_rcf &= ~(RCF_ALL_BROADCAST | RCF_LOCAL_BROADCAST);
	}
	bcopy(mac->mac_src_802_5, o_brother, IE8025_ADDRLEN);
	o_brother[0] &= ~RI_PRESENT;
	src = (u_long) o_brother[0] + (u_long) o_brother[IE8025_ADDRLEN-1]; 
	at = ato = &tok_arptabp[TOK_ARPTAB_HASH(src) * tok_arptabbsiz];
	for (n = 0 ; n < tok_arptabbsiz ; n++, at++) {
		if (!(bcmp(at->hwaddr, o_brother, IE8025_ADDRLEN)))
			break;
		if (!at->at_flags)
			ato = at;
		if (ato->at_flags && at->at_timer > ato->at_timer)
			ato = at;
	}
	if  (n >= tok_arptabbsiz) {
		
		/*
		 * Nuke the oldest one and proceed.
		 */
		at = ato;
		bzero(at, sizeof(*at));
		
	}

	bcopy(o_brother, (caddr_t)at->at_traddr,IE8025_ADDRLEN);
	/*
	 * snarf any routing info
	 */
	if (has_route(mac)
	    && route_bytes(mac) > sizeof (mac->mac_rcf)) {
		at->at_rcf = mac->mac_rcf;
		bcopy(mac->mac_seg, at->at_seg, 
		      sizeof (at->at_seg));
	} else
		at->at_rcf = 0;

	/*
	 * Set the flag indicating the entry in use
	 */
	at->at_flags = (ATF_INUSE);
	ARPTAB_UNLOCK(&tok_arptab_lock);
	mac->mac_src_802_5[0] &= ~RI_PRESENT;
}

/*
 * NAME: tok_arp_init
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
int tok_arp_init(void)
{
int thesize;

	thesize = arptabsize * sizeof(struct arptab);
	if (thesize > MAXALLOCSAVE) {
		tok_arptabbsiz = arptab_bsiz_dflt;
		tok_arptabnb = arptab_nb_dflt;
		thesize = (sizeof(struct arptab)) * tok_arptabbsiz * 
			tok_arptabnb;
	} else {
		tok_arptabbsiz = arptab_bsiz;
		tok_arptabnb = arptab_nb;
	}
	NET_MALLOC(tok_arptabp, struct arptab *, thesize, M_KTABLE, M_NOWAIT);
	if (!tok_arptabp) {
		return(1);
	}
	bzero((caddr_t)tok_arptabp, thesize);
	tok_arptime->timeout.it_value.tv_sec = 60;
	tok_arptime->timeout.it_value.tv_nsec = 0;
	tok_arptime->ipri = PL_IMP;
	tok_arptime->flags = 0;
	tok_arptime->func = (void (*)())tok_arptimer;
	tok_arptime->func_data = (ulong) 0;
	tstart(tok_arptime);
	return(0);
}

/*
 * NAME: tok_arptimer
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
tok_arptimer()
{
	register  struct  arptab	*at;
	register  int	i, killer;
	ARPTAB_LOCK_DECL(s)

	killer = ARPT_KILLC;
	ARPTAB_LOCK(&tok_arptab_lock);
	if (!tok_arptabp) {
		ARPTAB_UNLOCK(&tok_arptab_lock);
		return;
	}
	at = tok_arptabp;
	for (i = 0; i <  (tok_arptabbsiz * tok_arptabnb); i++ , at++) {
		if   (at->at_flags == 0 || (at->at_flags & ATF_PERM))
			continue;
		if (++at->at_timer <  killer)
				continue;
		/* timer has expired, clear entry	*/
		bzero(at, sizeof(*at));
	}
	ARPTAB_UNLOCK(&tok_arptab_lock);
	tok_arptime->timeout.it_value.tv_sec = 60;
	tok_arptime->timeout.it_value.tv_nsec = 0;
	tok_arptime->ipri = PL_IMP;
	tstart(tok_arptime);
	return;
}

/*
 * NAME: tok_dmx_address_resolve
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
 *      ie5_mac_hdr struct
 *      ie2_llc_snaphdr struct
 *
 * ROUTINES CALLED:
 *	tok_arp_init
 *
 * RETURNS:  
 *              EINVAL output_bundle in error.
 *              rc from ndd_output function.
 *		0 success.
*/
int
tok_dmx_address_resolve(obp, m, nddp)
struct output_bundle	*obp;
struct mbuf 		*m;
struct ndd 		*nddp; 
{
	struct ie5_hdr mac;
	struct ie5_mac_hdr *macp;
	struct ie2_llc_snaphdr *llcp;
	struct arptab *at;
	int llcsize;
	int rc,n;
	u_long dest;
	ARPTAB_LOCK_DECL(s)

	ARPTAB_LOCK(&tok_arptab_lock);
	if (!tok_arptabp) 
		if (tok_arp_init()) {
			ARPTAB_UNLOCK(&tok_arptab_lock);
			m_freem(m);
			return(ENOBUFS);
		}

	macp = (struct ie5_mac_hdr *) &mac;
	llcp = (struct ie2_llc_snaphdr *)(macp+1);
	switch(obp->helpers.pkt_format&0xFF) {
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
			ARPTAB_UNLOCK(&tok_arptab_lock);
			m_freem(m);
			return(EINVAL);
	}

	macp->mac_acf     = ACF_PRIORITY3;
	macp->mac_fcf     = FCF_LLC_FRAME;
	
	bcopy(obp->key_to_find, macp->mac_dst_802_5, IE8025_ADDRLEN);
	bcopy(nddp->ndd_physaddr, macp->mac_src_802_5, IE8025_ADDRLEN);

	/* use user-provided route info if present */
	if (obp->helpers.seglen > 0) {
		macp->mac_src_802_5[0] |= RI_PRESENT;
		bcopy(obp->helpers.segp, &macp->mac_rcf, obp->helpers.seglen);
		goto done;
	}

	/*
	 * check for broadcast.
	 */
 	if ((m->m_flags & M_BCAST) || (macp->mac_dst_802_5[0] & 0x80)) {
		macp->mac_rcf =  RCF_FRAME2 | sizeof (macp->mac_rcf) << 8;
		macp->mac_rcf |= RCF_ALL_BROADCAST;
		goto done;
	}
	dest = (u_long) macp->mac_dst_802_5[0] + 
		(u_long) macp->mac_dst_802_5[IE8025_ADDRLEN-1]; 
	at = &tok_arptabp[TOK_ARPTAB_HASH(dest) * tok_arptabbsiz];
	for (n = 0 ; n < tok_arptabbsiz ; n++, at++)
		if (!(bcmp(at->hwaddr, macp->mac_dst_802_5, IE8025_ADDRLEN)))
			break;
	if  (n >= tok_arptabbsiz)
		at = 0; 
	if (at == 0) {
		/*
		 * can't ask for it. Set no route and pray for rain.
		 */
		macp->mac_src_802_5[0] &= ~RI_PRESENT;
	} else {
		/*
		 * routing control info present?
		 */
		if (at->at_rcf) {
			macp->mac_src_802_5[0] |= RI_PRESENT;
			macp->mac_rcf     = at->at_rcf;
			bcopy((caddr_t) at->at_seg
				, macp->mac_seg
				, sizeof (macp->mac_seg));
		} else  {
			macp->mac_src_802_5[0] &= ~RI_PRESENT;
		}
		at->at_timer = 0; /* zero out the usage count for timer aging*/
	}
done:
	ARPTAB_UNLOCK(&tok_arptab_lock);
	M_PREPEND(m, mac_size(macp) + llcsize, M_DONTWAIT);
	if (m == 0) {
		nddp->ndd_nsdemux->nd_dmxstats.nd_nobufs++;
		return(ENOBUFS);
	}
	bcopy(macp, m->m_data, mac_size(macp));
	if (llcsize)
		bcopy(llcp, m->m_data+mac_size(macp), llcsize);
	
	rc = (*(nddp->ndd_output))(nddp, m); 
	if (rc)
		m_freem(m);
	return(rc);
}
/*
 * NAME: tok_add_demuxer
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
tok_add_demuxer(ndd_type)
int	ndd_type;
{
	
	bzero(&demuxer, sizeof(struct ns_demuxer));
	demuxer.nd_add_filter = tok_add_filter;
	demuxer.nd_del_filter = tok_del_filter;
	demuxer.nd_add_status = tok_add_status;
	demuxer.nd_del_status = tok_del_status;
	demuxer.nd_receive = tok_receive;
	demuxer.nd_status = tok_status;
	demuxer.nd_response = tok_response;
	demuxer.nd_use_nsdmx = 1;
	demuxer.nd_address_resolve = tok_dmx_address_resolve;
	demuxer.nd_address_input = tok_dmx_address_input;
	tok_arptabp = (struct arptab *) NULL;
	ARPTAB_LOCKINIT(&tok_arptab_lock);
	return(ns_add_demux(NDD_ISO88025, &demuxer));
}

/*
 * NAME: tok_dmx_init
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
 *      Token Ring demuxer control struct (struct tok_dmx_ctl) - internal struct
 *
 * ROUTINES CALLED:
 *	NET_MALLOC
 *
 * RETURNS:  
 *              0 - success
 *		ENOMEM - no mem
 */  
tok_dmx_init(nddp) 
	ndd_t		*nddp;
{
	struct tok_dmx_ctl	*tok_dmx_ctl;
	int			i;

	NET_MALLOC(tok_dmx_ctl, struct tok_dmx_ctl *, 
		   sizeof(struct tok_dmx_ctl), M_TEMP, M_WAITOK); 
	if (tok_dmx_ctl == (struct tok_dmx_ctl *) NULL)
		return(ENOMEM);
	bzero((caddr_t)tok_dmx_ctl, sizeof(struct tok_dmx_ctl));
	nddp->ndd_specdemux = (caddr_t) tok_dmx_ctl;
	return(0);
}

/*
 * NAME: tok_add_tap
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
 *      Token Ring demuxer control struct (struct tok_dmx_ctl) - internal struct
 *
 * ROUTINES CALLED:
 *	None.
 *
 * RETURNS:  
 *              0 - success
 *		EXIST - already exists
 */  
tok_add_tap(nddp, ns_user)
	ndd_t		*nddp;
	ns_user_t	*ns_user;
{
	struct tok_dmx_ctl	*tok_dmx_ctl;

	tok_dmx_ctl = (struct tok_dmx_ctl *) nddp->ndd_specdemux;
	if (tok_dmx_ctl->tap_user.isr != NULL)
		return(EEXIST);

	tok_dmx_ctl->tap_user = *ns_user;	
	return(0);
}

/*
 * NAME: tok_del_tap
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
 *      Token Ring demuxer control struct (struct tok_dmx_ctl) - internal struct
 *
 * ROUTINES CALLED:
 *	None.
 *
 * RETURNS:  
 *              0 - success
 *		ENOENT - no tap user exists
 */  
tok_del_tap(nddp)
	ndd_t		*nddp;
{
	struct tok_dmx_ctl	*tok_dmx_ctl;

	tok_dmx_ctl = (struct tok_dmx_ctl *) nddp->ndd_specdemux;
	if (tok_dmx_ctl->tap_user.isr == NULL)
		return(ENOENT);

	bzero(&tok_dmx_ctl->tap_user, sizeof(tok_dmx_ctl->tap_user));
	return(0);
}

/*
 * NAME: tok_add_mac
 *                                                                    
 * FUNCTION: 
 * 	Adds a TOK_DEMUX_MAC filter.
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
 *      Token Ring demuxer control struct (struct tok_dmx_ctl) - internal struct
 *
 * ROUTINES CALLED:
 *	None.
 *
 * RETURNS:  
 *              0 - success
 *		EXIST - already exists
 */  
tok_add_mac(nddp, ns_user)
	ndd_t		*nddp;
	ns_user_t	*ns_user;
{
	struct tok_dmx_ctl	*tok_dmx_ctl;

	DBGMSG("TOK_DEMUX: adding mac user\n");
	tok_dmx_ctl = (struct tok_dmx_ctl *) nddp->ndd_specdemux;
	if (tok_dmx_ctl->mac_user.isr != NULL)
		return(EEXIST);

	tok_dmx_ctl->mac_user = *ns_user;	
	return(0);
}

/*
 * NAME: tok_del_mac
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
 *      Token Ring demuxer control struct (struct tok_dmx_ctl) 
 *
 * ROUTINES CALLED:
 *	None.
 *
 * RETURNS:  
 *              0 - success
 *		ENOENT - no mac user exists
 */  
tok_del_mac(nddp)
	ndd_t		*nddp;
{
	struct tok_dmx_ctl	*tok_dmx_ctl;

	DBGMSG("TOK_DEMUX: deleting mac user\n");
	tok_dmx_ctl = (struct tok_dmx_ctl *) nddp->ndd_specdemux;
	if (tok_dmx_ctl->mac_user.isr == NULL)
		return(ENOENT);

	bzero(&tok_dmx_ctl->mac_user, sizeof(tok_dmx_ctl->mac_user));
	return(0);
}

/*
 * NAME: tok_mac_receive
 *                                                                    
 * FUNCTION: 
 *      Process incoming MAC packets from a NDD.  
 *      The packets are routed to the MAC user if one exists.
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt
 *                                                                   
 * NOTES: 
 *      This routine is called with a ptr to a NDD struct (nddp), 
 *      an mbuf ptr (m) and a token ring mac ptr (macp).  
 *
 * RECOVERY OPERATION: 
 *      None.
 *
 * DATA STRUCTURES: 
 *      NDD structure (ndd_t)
 *      mbuf structure (struct mbuf)
 *      Token ring demuxer ctl struct (struct tok_dmx_ctl) - internal struct
 *      Token ring demuxer stats struct
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
tok_mac_receive(nddp, m, macp)
ndd_t			*nddp;
struct mbuf		*m;
struct ie5_mac_hdr	*macp;
{
	struct tok_dmx_ctl	*tok_dmx_ctl;
	struct isr_data_ext	ext, *extp;

	DBGMSG("TOK_DEMUX: received a mac packet!!\n");
	tok_dmx_ctl = (struct tok_dmx_ctl *) nddp->ndd_specdemux;
        if (tok_dmx_ctl->mac_user.isr == NULL) {
                m_freem(m);
		tok_dmx_ctl->stats.nd_mac_rejects++;
		nddp->ndd_nsdemux->nd_dmxstats.nd_nofilter++;
		return;
	}
	DBGMSG("TOK_DEMUX: delivering a mac packet!!\n");
	if (tok_dmx_ctl->mac_user.pkt_format & NS_HANDLE_HEADERS) {
		bzero((caddr_t)&ext, sizeof(ext));
		ext.dstp = macp->mac_dst_802_5;
		ext.srcp = macp->mac_src_802_5;
		ext.dstlen = ext.srclen = IE8025_ADDRLEN;
		ext.isr_data = tok_dmx_ctl->mac_user.isr_data;
		extp = &ext;
	} else
		extp = (struct isr_data_ext *)tok_dmx_ctl->mac_user.isr_data;
	DELIVER_PACKET((&(tok_dmx_ctl->mac_user)), nddp, m, macp, extp);
	tok_dmx_ctl->stats.nd_mac_accepts++;
}
