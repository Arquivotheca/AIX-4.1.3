/* @(#)64	1.3  src/bos/kernel/sys/generic_mibs.h, snmp, bos411, 9428A410j 2/10/94 16:18:43 */
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Licensed Material - Property of IBM
 *
 * FILE:	/usr/include/sys/generic_mibs.h
 */
    
#ifndef _H_Generic_MIBS
#define _H_Generic_MIBS

	  
/* Generic Interface Extension Table */
typedef struct ifExtnsEntry  {
/*
 *  The Chipset structure is now an unsigned int and 15 unsigned ints 
 *  representing the object identifier.  There are byte strings defined 
 *  for the appropriate drivers so that they can bcopy the 64 bytes into 
 *  the space.  If a driver has no chipsets or it is unknown, return the
 *  UNKNOWNCHIPSET value.
 */
#define GENERIC_CHIPSET 	1
        unsigned long  chipset[16];		/* ifExtnsChipSet */
#define CHIPSETLENGTH	64
#define UNKNOWNCHIPSET	"\000\000\000\002\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"

#define GENERIC_REVWARE		2
        char           revware[16];	/* ifExtnsRevWare */

#define GENERIC_MCAST_TX_OK	3
	unsigned long  mcast_tx_ok;	/* ifExtnsMulticastsTransmittededOks */

#define GENERIC_BCAST_TX_OK	4
	unsigned long  bcast_tx_ok;	/* ifExtnsBroadcastsTransmittedOks */

#define GENERIC_MCAST_RX_OK	5
	unsigned long  mcast_rx_ok;	/* ifExtnsMulticastsReceivedOks */

#define GENERIC_BCAST_RX_OK	6
	unsigned long  bcast_rx_ok;	/* ifExtnsBroadcastsReceivedOks */

#define GENERIC_PROMISCUOUS	7
	unsigned long  promiscuous;	/* ifExtnsPromiscuous */
#define PROMTRUE	1
#define PROMFALSE	2

}  ifExtnsEntry_t;


/*  Generic Interface Test Table */
typedef struct ifExtnsTestEntry  {
#define GENERIC_COMMUNITY	10
        char           *community;	/* ifExtnsTestCommunity - snmpd only */

#define GENERIC_REQUEST_ID	11
        unsigned long   request_id;	/* ifExtnsTestRequestId - snmpd only */

#define GENERIC_TYPE		12
        unsigned long   type;		/* ifExtnsTestType */
#define GEN_MIB_NOTEST				0
#define GEN_MIB_TESTFULLDUPLEXLOOPBACK		1

#define GENERIC_RESULT		13
        unsigned short  result;		/* ifExtnsTestResult */
#define GEN_MIB_NONE 		1	/* no test yet requested */
#define GEN_MIB_SUCCESS		2
#define GEN_MIB_INPROGRESS	3
#define GEN_MIB_NOTSUPPORTED	4
#define GEN_MIB_UNABLETORUN	5  	/* due to state of system */
#define GEN_MIB_ABORTED		6
#define GEN_MIB_FAILED		7

#define GENERIC_CODE		14
	unsigned long	code;		/* ifExtnsTestCode */
}  ifExtnsTestEntry_t;

/*
 * The Generic Rcv Table is currently set by using the NDD_ENABLE_ADDRESS
 * and NDD_DISABLE_ADDRESS.  If the SNMP set protocol to the drivers were to
 * be used.  The GENERIC_ADDRESS_STATUS would be used to indicate to the 
 * device driver this is an address request. 
 */
#define GENERIC_ADDRESS_STATUS	15

typedef struct generic_mib {
	ifExtnsEntry_t		ifExtnsEntry;
	ifExtnsTestEntry_t	ifExtnsTestEntry;
	int 			RcvAddrTable;
} generic_mib_t;

#endif  /* _H_Generic_MIBS */
