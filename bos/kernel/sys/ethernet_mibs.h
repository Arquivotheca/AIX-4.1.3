/* @(#)63	1.3  src/bos/kernel/sys/ethernet_mibs.h, snmp, bos411, 9428A410j 2/10/94 16:18:32 */
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Licensed Material - Property of IBM
 *
 * FILE:        /usr/include/sys/ethernet_mibs.h
 */

#ifndef _H_Ethernet_MIBS
#define _H_Ethernet_MIBS

#include  <sys/generic_mibs.h>


/* The Ethernet-like Statistics group */
typedef struct  Dot3StatsEntry  {
#define ETHER_ALIGN_ERRS	30
	unsigned long  align_errs;	/* dot3StatsAlignmentErrors */

#define ETHER_FCS_ERRS		31
	unsigned long  fcs_errs;	/* dot3StatsFCSErrors */

#define ETHER_S_COLL_FRAMES	32
	unsigned long  s_coll_frames;	/* dot3StatsSingleCollisionFrames */

#define ETHER_M_COLL_FRAMES	33
	unsigned long  m_coll_frames;	/* dot3StatsMultipleCollisionFrames */

#define ETHER_SQETEST_ERRS	34
	unsigned long  sqetest_errs;	/* dot3StatsSQETestErrors */

#define ETHER_DEFER_TX		35
	unsigned long  defer_tx;	/* dot3StatsDeferredTransmissions */

#define ETHER_LATE_COLLISIONS	36
	unsigned long  late_collisions;	/* dot3StatsLateCollisions */

#define ETHER_EXCESS_COLLISIONS	37
	unsigned long  excess_collisions;/* dot3StatsExcessiveCollisions */

#define ETHER_MAC_TX_ERRS	38
	unsigned long  mac_tx_errs;	/* dot3StatsInternalMacTransmitErrors */

#define ETHER_CARRIERS_SENSE	39
	unsigned long  carriers_sense;	/* dot3StatsCarrierSenseErrors */

#define ETHER_LONG_FRAMES	40
	unsigned long  long_frames;	/* dot3StatsFrameTooLongs */

#define ETHER_MAC_RX_ERRS	41
	unsigned long  mac_rx_errs;	/* dot3StatsInternalMacReceiveErrors */
} Dot3StatsEntry_t;


/* The Ethernet-like Collision Statistics Group */
typedef struct  Dot3CollEntry  {
#define ETHER_COUNT		50
	unsigned long   count[16];	/* dot3CollCount */

#define ETHER_FREQ		51
	unsigned long   freq[16];	/* dot3CollFrequencies */
}  Dot3CollEntry_t;


typedef struct ethernet_mibs  {
	Dot3StatsEntry_t  Dot3StatsEntry;
	Dot3CollEntry_t	  Dot3CollEntry;
}  ethernet_mibs_t;

typedef struct ethernet_all_mib  {
	generic_mib_t     Generic_mib;
	ethernet_mibs_t   Ethernet_mib;
} ethernet_all_mib_t;


/* 802.3 Tests */
#define  ETH_MIB_INITERROR		80231 
#define  ETH_MIB_LOOPBACKERROR		80232


/* 802.3 Hardware Chipsets -- dot3ChipSets */
/* AMD Chipsets */
#define  ETH_MIB_AMD7990	"\000\000\000\013\000\000\000\001\000\000\000\003\000\000\000\006\000\000\000\001\000\000\000\002\000\000\000\001\000\000\000\012\000\000\000\007\000\000\000\010\000\000\000\001\000\000\000\001\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"

#define  ETH_MIB_AMD79900	"\000\000\000\013\000\000\000\001\000\000\000\003\000\000\000\006\000\000\000\001\000\000\000\002\000\000\000\001\000\000\000\012\000\000\000\007\000\000\000\010\000\000\000\001\000\000\000\002\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
#define  ETH_MIB_AMD79C940	"\000\000\000\013\000\000\000\001\000\000\000\003\000\000\000\006\000\000\000\001\000\000\000\002\000\000\000\001\000\000\000\012\000\000\000\007\000\000\000\010\000\000\000\001\000\000\000\003\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"

/* Intel Chipsets */
#define  ETH_MIB_Intel82586	"\000\000\000\013\000\000\000\001\000\000\000\003\000\000\000\006\000\000\000\001\000\000\000\002\000\000\000\001\000\000\000\012\000\000\000\007\000\000\000\010\000\000\000\002\000\000\000\001\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
#define  ETH_MIB_Intel82596	"\000\000\000\013\000\000\000\001\000\000\000\003\000\000\000\006\000\000\000\001\000\000\000\002\000\000\000\001\000\000\000\012\000\000\000\007\000\000\000\010\000\000\000\002\000\000\000\002\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"

/* Seeq Chipsets */
#define  ETH_MIB_Seeq8003	"\000\000\000\013\000\000\000\001\000\000\000\003\000\000\000\006\000\000\000\001\000\000\000\002\000\000\000\001\000\000\000\012\000\000\000\007\000\000\000\010\000\000\000\003\000\000\000\001\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"

/* National Chipsets */
#define  ETH_MIB_National8390	"\000\000\000\013\000\000\000\001\000\000\000\003\000\000\000\006\000\000\000\001\000\000\000\002\000\000\000\001\000\000\000\012\000\000\000\007\000\000\000\010\000\000\000\004\000\000\000\001\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
#define  ETH_MIB_NationalSonic	"\000\000\000\013\000\000\000\001\000\000\000\003\000\000\000\006\000\000\000\001\000\000\000\002\000\000\000\001\000\000\000\012\000\000\000\007\000\000\000\010\000\000\000\004\000\000\000\002\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"

/* Fujitsu Chipsets */
#define  ETH_MIB_Fujitsu86950	"\000\000\000\013\000\000\000\001\000\000\000\003\000\000\000\006\000\000\000\001\000\000\000\002\000\000\000\001\000\000\000\012\000\000\000\007\000\000\000\010\000\000\000\005\000\000\000\001\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
#define  ETH_MIB_Fujitsu86960	"\000\000\000\013\000\000\000\001\000\000\000\003\000\000\000\006\000\000\000\001\000\000\000\002\000\000\000\001\000\000\000\012\000\000\000\007\000\000\000\010\000\000\000\005\000\000\000\002\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"

#endif  /* _H_Ethernet_MIBS */
