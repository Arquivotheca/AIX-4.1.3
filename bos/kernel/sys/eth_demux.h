/* @(#)21	1.3  src/bos/kernel/sys/eth_demux.h, sysxdmx, bos411, 9428A410j 2/27/94 15:05:34 */
/*
 *   COMPONENT_NAME: SYSXDMX
 *
 *   FUNCTIONS: none
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
#ifndef _NET_ETH_DEMUX_H
#define _NET_ETH_DEMUX_H

#define	ETHDMX_HASH		18

/*
 * The eth_user struct allows for a linked list of ethertype users
 * that hash into the same cell.
 */
struct eth_user {
	struct eth_user		*next;
	struct eth_user		*prev;
	struct ns_user 		user;
	struct ns_8022 		filter;
};

struct eth_dmx_stats {
	u_long		nd_ethertype_accepts;	/* # ethertypes accepted */
	u_long		nd_ethertype_rejects;	/* # ethertypes rejected */
};

/*
 * The eth_dmx_ctl struct contains all the ethertype users in a hash table,
 * as well as the single tap user.
 */
struct eth_dmx_ctl {
	struct eth_user		hash_heads[ETHDMX_HASH];
	struct ns_user 		tap_user;
	struct eth_dmx_stats	stats;
};

#endif
