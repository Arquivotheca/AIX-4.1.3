/* @(#)71	1.10  src/bos/usr/include/arpa/inet.h, sockinc, bos411, 9438C411e 9/25/94 13:19:57 */
/*
 * COMPONENT_NAME: INCPROTO
 *
 * FUNCTIONS:
 *
 * ORIGINS: 26,27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */

/*
 * External definitions for
 * functions in inet(3N)
 */

/*
 * COSE defines the following types and structure in netinet/in.h,
 * and netdb.h. The defines will ensure that an application only
 * see them once.
 */
#ifndef _IP_TYPES_T
typedef unsigned short	in_port_t;
typedef	unsigned long	in_addr_t;
#define _IP_TYPES_T
#endif /* _IP_TYPES_T */

#ifndef _IN_ADDR_STRUCT
struct in_addr {
	in_addr_t	s_addr;
};
#define _IN_ADDR_STRUCT
#endif /* _IN_ADDR_STRUCT */

#ifdef _XOPEN_EXTENDED_SOURCE

in_addr_t	inet_addr(const char *);
in_addr_t	inet_lnaof(struct in_addr);
struct in_addr	inet_makeaddr(in_addr_t, in_addr_t);
in_addr_t	inet_netof(struct in_addr);
in_addr_t	inet_network(const char *);
char		*inet_ntoa(struct in_addr);

#else

/*
 * We should have prototypes here but our users can't handle it.
 */

extern u_long   inet_addr();
extern u_long   inet_lnaof();
extern struct   in_addr inet_makeaddr();
extern u_long   inet_network();
extern u_long   inet_netof();
extern char *   inet_ntoa();

#endif	/* _XOPEN_EXTENDED_SOURCE */
