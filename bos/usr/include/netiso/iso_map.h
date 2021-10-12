/* @(#)52	1.4  src/bos/usr/include/netiso/iso_map.h, sockinc, bos411, 9428A410j 5/10/91 16:40:41 */

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

/*
 * ARGO Project, Computer Sciences Dept., University of Wisconsin - Madison
 */
struct maptab {
	struct iso_addr	map_isoa;		/* iso address */
	u_char			map_enaddr[6];	/* ethernet address */
	u_char			map_valid;		/* true if entry is valid */
};
