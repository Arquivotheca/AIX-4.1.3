/* @(#)46	1.4  src/bos/usr/include/netiso/cons_ifx25.h, sockinc, bos411, 9428A410j 5/10/91 16:40:31 */

/*
 * 
 * COMPONENT_NAME: (SOCKET) Socket services
 * 
 * FUNCTIONS: 
 *
 * ORIGINS: 26 27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *****************************************************************/

#define MAX_HWADDR 	MAX_SNPALEN

struct  ifx25com {
	struct  ifnet	ac_if;			/* network visible interface */
	u_char		ac_hwaddr[MAX_HWADDR];	/* hardware address */
	u_short		ac_hwlen;		/* length of hw address	*/
	u_long		ifType;			/* interface type */
};

struct	ifx25_softc {
	struct ifx25com	ifx25_com;
	struct  file *netfp;		/* file pointer for driver	*/
	u_short	ifx25_flags;		/* private flags		*/
	x25_devinfo_t iocinfo;		/* ddi info			*/
};

#define ifx25_if	ifx25_com.ac_if
#define ifx25_addr	ifx25_com.ac_hwaddr

