/* @(#)88	1.1  src/bos/kernext/inet/igmp_var.h, sysxinet, bos411, 9428A410j 7/24/93 13:52:05 */
/*
 *   COMPONENT_NAME: SYSXINET
 *
 *   FUNCTIONS: IGMP_RANDOM_DELAY
 *		
 *
 *   ORIGINS: 26,27,85,89
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * 
 * (c) Copyright 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 * 
 */
/*
 * OSF/1 1.2
 */
/*
 * Internet Group Management Protocol (IGMP),
 * implementation-specific definitions.
 *
 * Written by Steve Deering, Stanford, May 1988.
 *
 * MULTICAST 1.1
 */

struct igmpstat {
	u_int	igps_rcv_total;		/* total IGMP messages received    */
	u_int	igps_rcv_tooshort;	/* received with too few bytes     */
	u_int	igps_rcv_badsum;	/* received with bad checksum      */
	u_int	igps_rcv_queries;	/* received membership queries     */
	u_int	igps_rcv_badqueries;	/* received invalid queries        */
	u_int	igps_rcv_reports;	/* received membership reports     */
	u_int	igps_rcv_badreports;	/* received invalid reports        */
	u_int	igps_rcv_ourreports;	/* received reports for our groups */
	u_int	igps_snd_reports;	/* sent membership reports         */
#if	defined(_KERNEL) && LOCK_NETSTATS
	simple_lock_data_t igps_lock;	/* statistics lock */
#endif
};

#ifdef _KERNEL
extern struct igmpstat igmpstat;

/*
 * Macro to compute a random timer value between 1 and (IGMP_MAX_REPORT_DELAY
 * * countdown frequency).  We generate a "random" number by adding
 * the total number of IP packets received, our primary IP address, and the
 * multicast address being timed-out.  The 4.3 random() routine really
 * ought to be available in the kernel!
 */
#define IGMP_RANDOM_DELAY(multiaddr)					\
	/* struct in_addr multiaddr; */					\
	( (ipstat.ips_total +						\
	   ntohl(IA_SIN(in_ifaddr)->sin_addr.s_addr) +			\
	   ntohl((multiaddr).s_addr)					\
	  )								\
	  % (IGMP_MAX_HOST_REPORT_DELAY * PR_FASTHZ) + 1		\
	)

#endif
