static char sccsid[] = "@(#)11	1.10  src/bos/kernel/net/nd_lan.c, sysnet, bos411, 9428A410j 4/21/94 11:05:28";
/*
 *   COMPONENT_NAME: SYSNET
 *
 *   FUNCTIONS: dmx_8022_add_filter
 *		dmx_8022_del_filter
 *		dmx_8022_get_user
 *		dmx_8022_receive
 *		dmx_add_status
 *		dmx_del_status
 *		dmx_init
 *		dmx_non_ui
 *		dmx_status
 *		dmx_term
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

#include <sys/errno.h>
#include <sys/types.h>
#include <sys/syspest.h>
#include <sys/syslog.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/cdli.h>
#include <net/nd_lan.h>
#include <sys/ndd.h>
#include <net/netisr.h>
#include <net/if.h>

struct ns_8022_user * dmx_8022_get_user();

#define THEMAX (2 * AF_MAX)
struct af_ent	af_table[THEMAX];

/***************************************************************************
*
*	Process incoming 802.2 packets. Look for user and distribute packets.
*
***************************************************************************/
void
dmx_8022_receive(nddp, m, llcoffset, extp)
	struct ndd		*nddp;
	struct mbuf		*m;		/* input packet		*/
	int			llcoffset;
	struct isr_data_ext	*extp;
{
	struct ns_8022_user	*ns_8022_user;
	caddr_t 		macp;		/* pointer to mac header */
	struct ie2_llc_snaphdr  *llcp;
	int 			headerlen = 0;	/* length of header the 
						   isr doesn't care about */
	int			pullup_len;	/* len to pull up in 1st mbuf */

	assert(nddp->ndd_nsdemux != (struct ns_dmx_ctl *) NULL);

	if (m->m_flags & M_BCAST) {
		nddp->ndd_nsdemux->nd_dmxstats.nd_bcast++;
	} else {
		if (m->m_flags & M_MCAST)
			nddp->ndd_nsdemux->nd_dmxstats.nd_mcast++;
	}

	/*
	 * Make sure enough of the header is contiguous
	 */
	macp = mtod(m, caddr_t);
	llcp = (struct ie2_llc_snaphdr *) (macp + llcoffset);

	/* 
	 * Pull up at least the maclen and a ie2 hdr.  If possible
	 * pull up a snap header too...
 	 */
	if (m->m_pkthdr.len < (llcoffset + sizeof(struct ie2_llc_snaphdr)))
		pullup_len = llcoffset + sizeof(struct ie2_llc_hdr);
	else
		pullup_len = llcoffset + sizeof(struct ie2_llc_snaphdr);
	if (m->m_len < pullup_len) {
		m = m_pullup(m, pullup_len);
		if (m == (struct mbuf *) NULL) {
			nddp->ndd_nsdemux->nd_dmxstats.nd_nobufs++;
			return;
		}
	}


	/* 
	 * Locate user that's done an ns_add_filter for this packet.
	 *
	 * If we don't have a snap header's worth of llc data, then
	 * pass null values for type and protid to DMX_8022_GET_USER...
	 */
	if (m->m_pkthdr.len < (llcoffset + sizeof(struct ie2_llc_snaphdr))) {
		DMX_8022_GET_USER(ns_8022_user, llcp->dsap, 0, 0,
			nddp->ndd_nsdemux);
	} else {
		DMX_8022_GET_USER(ns_8022_user, llcp->dsap, llcp->type, 
			llcp->prot_id, nddp->ndd_nsdemux);
	}

	if (ns_8022_user == NULL) {

		/*
		 * Handle null sap non ui frames with no user.
		 * We must still respond to these...
		 */
		if ((llcp->ctrl != CTRL_UI) && (llcp->dsap == NULL)) {
			dmx_non_ui(nddp, m, llcoffset);
			return;
		}
		nddp->ndd_nsdemux->nd_dmxstats.nd_nofilter++;
		m_freem(m);
		return;
	}

	/* 
	 *	If it's not a UI packet, see what user wants done.
	 */
	if ( (llcp->ctrl != CTRL_UI) && 
	     (ns_8022_user->user.pkt_format & NS_HANDLE_NON_UI) ) {
		dmx_non_ui(nddp, m, llcoffset);
		return;
	}

	/*
	 *	Fix up headers as requested.
	 */
	switch (ns_8022_user->user.pkt_format & 0xFF) {
		case NS_PROTO:
			headerlen = llcoffset + sizeof(struct ie2_llc_hdr); 
			break;
		case NS_PROTO_SNAP:
			headerlen = llcoffset + sizeof(struct ie2_llc_snaphdr); 
			break;
		case NS_INCLUDE_LLC:
			headerlen = llcoffset;
			break;
		case NS_INCLUDE_MAC:
		default:
			break;
	}

	if (headerlen) {
		m->m_pkthdr.len -= headerlen;
		m->m_len -= headerlen;
		m->m_data += headerlen;
	}

	if (ns_8022_user->user.pkt_format & NS_HANDLE_HEADERS) {

		/*
		 * Set llclen in extp.
		 */
		extp->llcp = (caddr_t)llcp;
		if (ns_8022_user->filter.filtertype == NS_8022_LLC_DSAP)
			extp->llclen = sizeof(struct ie2_llc_hdr);
		else
			extp->llclen = sizeof(struct ie2_llc_snaphdr);
		extp->isr_data = ns_8022_user->user.isr_data;
	} else 
		extp = (struct isr_data_ext *)ns_8022_user->user.isr_data;
	IFSTUFF_AND_DELIVER(ns_8022_user, nddp, m, macp, extp);
	return;
}

/***************************************************************************
*
*	Respond to non-ui (ie XID and TEST) requests.
* 	DA/SA swapping is done in demuxer's nd_response()
*
***************************************************************************/
dmx_non_ui(nddp, m, llcoffset)
struct ndd	*nddp;
struct mbuf	*m;
int		llcoffset;
{
	caddr_t 			macp;
	struct ie2_xid_ifield		*llcp;
	unsigned char			t;

	macp = mtod(m, caddr_t);
	llcp = (struct ie2_xid_ifield *) (macp + llcoffset);

	if (llcp->ssap & SSAP_RESP) {		/* rcv Response	*/
		m_freem(m);
		return;
	}

	/*
	 * swap DSAP/SSAP
	 */
	t = llcp->dsap;
	llcp->dsap = llcp->ssap;
	llcp->ssap = t | SSAP_RESP;			/* set response bit  */

	if ((llcp->ctrl & 0xef) == CTRL_XID ) {
		llcp->xid_info[0] = 0x81; /* IEEE 802.2 format */
		llcp->xid_info[1] = 0x01; /* connectionless */
		llcp->xid_info[2] = 0x00; /* window size */
		m->m_pkthdr.len = m->m_len = 
			llcoffset + sizeof (struct ie2_xid_ifield);
	} else {
		if ((llcp->ctrl & 0xe3) != CTRL_TEST ) {
			m_freem(m);
			return;
		}
	}

	if (nddp->ndd_demuxer->nd_response)
		(*(nddp->ndd_demuxer->nd_response)) (nddp, m, llcoffset);
	else
		m_freem(m);
	return;
}

/***************************************************************************
*
*	dmx_8022_add_filter() -  Add 802.2 filter to the demuxer
*	
*	RETURNS :	0      - no errors 
*			EEXIST - sap/type already in table by another user
*			EALREADY - sap/type already in table by you!
*			ENOMEM - couldn't allocate entry
***************************************************************************/
dmx_8022_add_filter(nddp, filter, ns_user)
	struct ndd		*nddp;
	struct ns_8022		*filter;
	struct ns_user		*ns_user;	/* the details		    */
{
	int			error;
	struct ns_8022_user	*new_user, *fup;

	NET_MALLOC(new_user, struct ns_8022_user *, sizeof(struct ns_8022_user),
		M_TEMP, M_WAITOK); 
			
	if (new_user == (struct ns_8022_user *) NULL)
		return(ENOMEM);
	
	bzero(new_user, sizeof(struct ns_8022_user));
	new_user->user = *ns_user;
	new_user->filter = *filter;

	if ((fup=dmx_8022_get_user(filter, nddp))) {
		NET_FREE(new_user, M_TEMP);
		if (!(bcmp(&fup->user, ns_user, sizeof(ns_user_t))))
			return(EALREADY);
		else
			return(EEXIST);
	}
	insque(new_user, nddp->ndd_nsdemux->dsap_heads[filter->dsap].prev);
	return(0);
}


/***************************************************************************
*	dmx_8022_del_filter() - Deletes a 802.2 filter
*	
*	RETURNS :	0      - no errors 
*			ENOENT - the type was not found
***************************************************************************/
dmx_8022_del_filter(nddp, filter)
	struct ndd		*nddp;
	struct ns_8022		*filter;
{
	struct ns_8022_user	*fup;

	fup = dmx_8022_get_user(filter, nddp);
	if (fup == NULL)
		return(ENOENT);

	remque(fup);
	NET_FREE(fup, M_TEMP);
	return(0);
}

/***************************************************************************
*
*	dmx_add_status() -  Add status filter to the demuxer
*	
***************************************************************************/
dmx_add_status(nddp, filter, ns_statuser)
	struct ndd		*nddp;
	struct ns_com_status	*filter;
	struct ns_statuser	*ns_statuser;
{
	struct com_status_user	*new_status_user;
	struct com_status_user	*status_head;

	if (filter->filtertype != NS_STATUS_MASK)
		return(EINVAL);

	status_head = &(nddp->ndd_nsdemux->com_status_head);

	NET_MALLOC(new_status_user, struct com_status_user *, 
		sizeof(struct com_status_user), M_TEMP, M_WAITOK); 
	if (new_status_user == (struct com_status_user *)NULL)
		return(ENOMEM);
	bzero(new_status_user, sizeof(*new_status_user));
	filter->sid = new_status_user;
	new_status_user->user = *ns_statuser;
	new_status_user->filter = *filter;
	insque(new_status_user, status_head->prev);
	return(0);
}

/***************************************************************************
*
*	dmx_del_status() -  Delete a status filter from the demuxer
*	
***************************************************************************/
dmx_del_status(nddp, filter)
	struct ndd		*nddp;
	struct ns_com_status	*filter;
{
	struct com_status_user	*sup;
	struct com_status_user	*status_head;

	if (filter->filtertype != NS_STATUS_MASK)
		return(EINVAL);

	status_head = &(nddp->ndd_nsdemux->com_status_head);
	for (sup = status_head->next; sup != status_head; sup = sup->next) {
		if (sup->filter.sid == filter->sid)	
			break;
	}
	
	if (sup != status_head) {
		remque(sup);
		NET_FREE(sup, M_TEMP);
		return(0);
	} 

	return(ENOENT);
}

/***************************************************************************
*
*	Deliver status to all registered users that match the status.
*	
***************************************************************************/
void
dmx_status(nddp, status)
	struct ndd		*nddp;
	struct ndd_statblk  	*status;
{
	struct com_status_user	*sup;
	struct com_status_user	*status_head;

	if (nddp->ndd_nsdemux == (struct ns_dmx_ctl *) NULL)
		return;

	status_head = &(nddp->ndd_nsdemux->com_status_head);

	for (sup = status_head->next; sup != status_head; sup = sup->next) {
		if ( (sup->filter.mask & status->code) != 0 )
			(*(sup->user.isr)) (nddp, status, sup->user.isr_data);
	}
	return;
}


/***************************************************************************
*
*	dmx_8022_get_user() - retrieves a user based upon filter
*
***************************************************************************/
struct ns_8022_user *
dmx_8022_get_user(filter, nddp)
	struct ns_8022		*filter;
	struct ndd		*nddp;
{
	struct ns_8022_user	*ns_8022_user;

	ASSERT(nddp->ndd_nsdemux != (struct ns_dmx_ctl *) NULL);
	if (nddp->ndd_nsdemux == (struct ns_dmx_ctl *) NULL)
		return(NULL);

	DMX_8022_GET_USER(ns_8022_user, filter->dsap, filter->ethertype, 
		filter->orgcode, nddp->ndd_nsdemux);
	return(ns_8022_user);
}


/***************************************************************************
*
*	Initialize common demuxer services for a NDD.	
*
***************************************************************************/
dmx_init(nddp) 
	struct ndd			*nddp;
{
	struct com_status_user		*status_head;
	struct ns_8022_user_head	*dsap_heads;
	int				i;

	if (nddp->ndd_nsdemux != NULL)
		return(0);

	NET_MALLOC(nddp->ndd_nsdemux, struct ns_dmx_ctl *, 
		sizeof(struct ns_dmx_ctl), M_TEMP, M_WAITOK); 
	if (nddp->ndd_nsdemux == NULL)
		return(ENOMEM);
	bzero(nddp->ndd_nsdemux, sizeof(*nddp->ndd_nsdemux));
	
	status_head = &(nddp->ndd_nsdemux->com_status_head);
	status_head->next = status_head->prev = status_head;
	dsap_heads = nddp->ndd_nsdemux->dsap_heads;
	for (i = 0 ; i < NS_MAX_SAPS ; i++)
		dsap_heads[i].next = dsap_heads[i].prev = 
			(struct ns_8022_user *) &dsap_heads[i];
	return(0);
}

/***************************************************************************
*
*	Terminate common demuxer services for a NDD.	
*
***************************************************************************/
void
dmx_term(nddp) 
	struct ndd	*nddp;
{
	if (nddp->ndd_nsdemux == NULL)
		return;

	NET_FREE(nddp->ndd_nsdemux, M_TEMP);
	nddp->ndd_nsdemux = NULL;
	return;
}


/***************************************************************************
*
*	RETURNS :	
*			EEXIST - af already in table
* 			EINVAL - af out of range
*			0      - no errors 
*
***************************************************************************/
nd_config_proto(af, config_proto)
	u_short		af;
	struct config_proto	*config_proto;
{
	if (af >= THEMAX)
		return(EINVAL);

	if (af_table[af].sap)
		return(EEXIST);

	af_table[af].config = *config_proto;
	af_table[af].sap = 1;
	return(0);
}

/***************************************************************************
*	Path for DLPI protocols to register.
*	
*	RETURNS :	
*			EEXIST - sap/type already in table
*			ENOMEM - table overflow.
*			0      - no errors 
*
***************************************************************************/
int nd_config_sap(sap, type, config_proto, af)
	u_short		sap;
	u_short		type;
	struct config_proto	*config_proto;
	u_short		*af;
{
	int		i, x;

	for (i=(THEMAX/2); i < THEMAX; i++) {
		if (af_table[i].sap) {
			if (af_table[i].sap == sap && af_table[i].type == type)
				return (EEXIST);
			continue;
		} else 
			break;
	}
	
	if (i >= THEMAX)
		return (ENOMEM);
		
	if (x = nd_config_proto(i,config_proto))
		return(x);

	af_table[i].sap = sap;
	af_table[i].type = type;
	*af = i;
	return(0);
}

/***************************************************************************
*	Routine for DLPI to find registered address families
*	
*	RETURNS :	
*			af
*			0      - not found
*
***************************************************************************/
int nd_find_af(sap, type)
	u_short		sap;
	u_short		type;
{
	int		i;

	for (i=(THEMAX/2); i < THEMAX; i++) {
		if (af_table[i].sap) {
			if (af_table[i].sap == sap && af_table[i].type == type)
				return (i);
			continue;
		} else 
			return(0);
	}
	return(0);
}
