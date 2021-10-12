/* @(#)25	1.3  src/bos/kernel/sys/tok_demux.h, sysxdmx, bos411, 9428A410j 2/27/94 15:05:39 */
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
#ifndef _NET_TOK_DEMUX_H
#define _NET_TOK_DEMUX_H

struct tok_dmx_stats {
	u_long		nd_mac_accepts;		/* # MAC frames accepted */
	u_long		nd_mac_rejects;		/* # MAC frames rejected */
};

/*
 * Token Ring demuxer supports one MAC user and one TAP user.
 */
struct tok_dmx_ctl {
	ns_user_t 		mac_user;
	ns_user_t 		tap_user;
	struct tok_dmx_stats	stats;
};

#define	TOK_DEMUX_MAC	(NS_LAST_FILTER + 1)	/* Mac user filter type */

#endif
