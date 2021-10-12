/* @(#)78       1.5  src/bos/kernel/sys/dlpistats.h, sysxdlpi, bos41J, 9514A_all 4/4/95 19:33:42 */
/*
 *   COMPONENT_NAME: SYSXDLPI
 *
 *   FUNCTIONS:
 *
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993, 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _SYS_DLPISTATS_H
#define _SYS_DLPISTATS_H

/* 
 * Statistics on both a per Streams and global basis obtainable via the 
 * DL_GET_STATISTICS_REQ primitive.
 */

typedef struct statistics {
	u_long   rx_pkts;	/* Number of received packets */
	u_long   tx_pkts;	/* Number of transmitted packets */
	u_long   rx_bytes;	/* Number of received bytes */
	u_long   tx_bytes;	/* Number of transmitted bytes */
	u_long   rx_discards;	/* Number of incoming packets discarded */
	u_long   tx_discards;	/* Number of outgoing packets discarded */
	u_long   no_bufs;	/* Number of times buffers not avail */
	u_long   binds;		/* Number of successful binds */
	u_long   unknown_msgs;	/* Number of unknown message types */
				/* Status of promiscuous mode levels : */
	u_short  promisc_phys;	/*   Per Stream: physical level (ON or OFF) */
				/*   Global: count of promisc_phys ON */
	u_short  promisc_sap;	/*   Per Stream: SAP level (ON or OFF) */
				/*   Global: count of promisc_sap ON */
	u_short  promisc_multi;	/*   Per Stream: multicast addrs (ON or OFF) */
				/*   Global: count of promisc_multi ON */
	u_long   multicast_addrs;/* Number of multicast enabled addresses */
} stats_t;

#define  PROMISCUOUS_OFF	(u_short)0
#define  PROMISCUOUS_ON		(u_short)1

/* for backwards source compatibility */
#include <sys/dlpi_aix.h>

#endif /* _SYS_DLPISTATS_H */
