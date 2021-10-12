/* @(#)23	1.6  src/bos/kernel/sys/fddi_demux.h, sysxdmx, bos411, 9428A410j 3/17/94 11:43:35 */
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
#ifndef _NET_FDDI_DEMUX_H
#define _NET_FDDI_DEMUX_H

#ifndef _NET_ND_LAN_H
#include <net/nd_lan.h>
#endif

/* 
 * This struct used to describe FDDI smt and smt_nsa filters.
 */
struct fddi_dmx_filter {
	ns_8022_t	filter;
	int		id;
};
typedef struct fddi_dmx_filter fddi_dmx_filter_t;

/*
 * The smt user list is a circularly linked list of all users/filters
 * wanting smt or smt_nsa frames.  insque/remque are used to maintain
 * this list.  
 */
struct fddi_dmx_smt_user {
	struct fddi_dmx_smt_user  	*next;
	struct fddi_dmx_smt_user  	*prev;
	ns_user_t			user;
	fddi_dmx_filter_t		filter;
};
typedef struct fddi_dmx_smt_user fddi_dmx_smt_user_t;

struct fddi_dmx_stats {
	u_long		nd_mac_accepts;		/* # MAC frames accepted */
	u_long		nd_mac_rejects;		/* # MAC frames rejected */
	u_long		nd_smt_accepts;		/* # SMT frames accepted */
	u_long		nd_smt_rejects;		/* # SMT frames rejected */
};
typedef struct fddi_dmx_stats fddi_dmx_stats_t;

/*
 * The fddi_dmx_ctl structure contains fddi-specific user/filters.
 * one mac user, one tap user, and many smt users.
 */
struct fddi_dmx_ctl {
	ns_user_t			mac_user;
	ns_user_t 			tap_user;
	fddi_dmx_smt_user_t		smt_user;
	fddi_dmx_stats_t		stats;
};
typedef struct fddi_dmx_ctl fddi_dmx_ctl_t;

#define FDDI_DEMUX_MAC		(NS_LAST_FILTER + 1)	/* Mac Beacon type */
#define	FDDI_DEMUX_SMT_NSA	(NS_LAST_FILTER + 2)	/* SMT NSA type */
#define	FDDI_DEMUX_SMT		(NS_LAST_FILTER + 3)	/* other SMT types */

#define FDDI_DEMUX_FC_MASK		0xF0	/* Mask high nibble */
#define FDDI_DEMUX_MAC_FRAME		0xC0	/* Cx are MAC frames */
#define FDDI_DEMUX_SMT_NSA_FRAME	0x4F	/* Specific NSA frame */
#define FDDI_DEMUX_SMT_FRAME		0x40	/* 4x are SMT frames */
#define FDDI_DEMUX_LLC_FRAME		0x50	/* 5x are LLC frames */

#define FDDI_DEMUX_ADDRCOPY(s, d) \
	{ \
                *(struct char_6 {char x[6];}*)(d) = *(struct char_6 *)(s); \
	}

#ifndef _NETINET_IF_FDDI_H
#define _NETINET_IF_FDDI_H

#define	RI_PRESENT		0x80	/* turn on bit 0 of byte 0 src addr */
#define	RCF_ALL_BROADCAST	0x8000	/* all routes broadcast		*/
#define	RCF_LOCAL_BROADCAST	0x4000	/* single route broadcast	*/
#define	RCF_DIRECTION		0x0080	/* direction			*/
#define FDDI_ADDRLEN 	6 

/*
 * FDDI MAC header
 */
struct fddi_mac_hdr {
	struct {
		u_char	reserved[3];	/* Reserved 3 bytes for adapter */
		u_char	Mac_fcf;	/* frame control field		*/
		u_char	Mac_dst[6];	/* destination address		*/
		u_char	Mac_src[6];	/* source address		*/
	} _First;
	struct {
		u_short Mac_rcf;	/* routing control field	*/
		u_short Mac_seg[14];	/* 30 routing segments 		*/
	} _Variable;
};

#define	mac_fcf_f	_First.Mac_fcf
#define	mac_dst_f	_First.Mac_dst
#define	mac_src_f	_First.Mac_src
#define mac_rcf_f	_Variable.Mac_rcf
#define mac_seg_f	_Variable.Mac_seg

#define	route_bytes_f(mac)	(((mac)->mac_rcf_f >> 8) & 0x1f)

#define	has_route_f(mac)		((mac)->mac_src_f[0] & RI_PRESENT)

#define	mac_size_f(mac)		\
	(sizeof ((mac)->_First) + (has_route_f((mac)) ? route_bytes_f((mac)) : 0))

#endif /* _NETINET_IF_FDDI_H */
#endif
