/* @(#)49	1.10  src/bos/kernel/sys/nettrace.h, sockinc, bos411, 9428A410j 6/18/91 15:36:00 */
/*
 * COMPONENT_NAME: (SYSNET) Network trace and error logging
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989 
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#ifndef _H_NETTRACE
#define _H_NETTRACE

#include <sys/trchkid.h>

#ifdef DEBUG
#define NETTRACE(a,b) {			\
	printf a;			\
	TRCHKT(HKWD_NETERR|(b));	\
}

#else
#define NETTRACE(a,b)  TRCHKT(HKWD_NETERR|(b))	
#endif

#ifdef DEBUG
#define PERFTRC(a)  	TRCHKT(HKWD_NETPERF|(a))	
#define PERFTRC1(a,b)	TRCHKL1(HKWD_NETPERF|(a),(b))	
#else
#define PERFTRC(a) 
#define PERFTRC1(a,b) 
#endif

#define NETTRC(a)		TRCHKT(a)
#define NETTRC1(a,b)		TRCHKL1T(a,b);
#define NETTRC2(a,b,c)		TRCHKL2T(a,b,c);
#define NETTRC3(a,b,c,d)	TRCHKL3T(a,b,c,d);
#define NETTRC4(a,b,c,d,e)	TRCHKL4T(a,b,c,d,e);

/* constants for net_error() service  and others */
#define INV_TX_INTR		100     
#define INV_ARP_IFTYPE  	101
#define IF_DETACH_FAIL		102
#define INV_INPUT_TYPE		103
#define NO_MBUFS		104
#define IF_NOT_RUNNING		105
#define CLR_INDIC		106
#define UNK_PKT_TYPE		107
#define NET_XMIT_FAIL		108
#define NET_DETACH_FAIL		109
#define ARP_WRONG_HDR		110
#define ARP_UNK_PROTO		111
#define ARP_IPBRDCAST_ADDR	112
#define ARP_DUP_ADDR		113
#define ARP_TABFULL		114
#define SL_OVRUNERR		115
#define SL_FRAMERR		116
#define SL_STATERR		117
#define SL_MBUFERR		118
#define SL_LENERR		119

#endif  /*_H_NETTRACE */
