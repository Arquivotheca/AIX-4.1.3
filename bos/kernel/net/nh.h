/* @(#)02	1.4  src/bos/kernel/net/nh.h, sysxinet, bos411, 9434B411a 8/23/94 12:30:02 */
/*
 *   COMPONENT_NAME: SYSNET
 *
 *   FUNCTIONS: HTONL
 *		NTOHL
 *		htonl
 *		htons
 *		ntohl
 *		ntohs
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Macros for number representation conversion.
 *
 */

#include <sys/machine.h>

#if BYTE_ORDER == BIG_ENDIAN
#define ntohl(x)        (x)
#define ntohs(x)        (x)
#define htonl(x)        (x)
#define htons(x)        (x)
#define NTOHL(x)
#define NTOHS(x)
#define HTONL(x)
#define HTONS(x)
#endif

#if BYTE_ORDER == LITTLE_ENDIAN

#ifdef	_NO_PROTO

u_short ntohs(), htons();
u_long  ntohl(), htonl();

#else	/* POSIX required prototypes */

u_short	ntohs(u_short);
u_short	htons(u_short);
u_long	ntohl(u_long);
u_long	htonl(u_long);

#endif
#endif
