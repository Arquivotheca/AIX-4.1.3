/* @(#)94	1.1  src/bos/kernext/xns/ns_error.h, sysxxns, bos411, 9428A410j 7/24/93 14:02:31 */
/*
 *   COMPONENT_NAME: SYSXXNS
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 26,27,85
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 1.2
 */
/*
 * Copyright (c) 1984, 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *      Base:   ns_error.h      7.5 (Berkeley) 6/28/90
 */

/*
 * Xerox NS error messages
 */

struct ns_errp {
	u_short		ns_err_num;		/* Error Number */
	u_short		ns_err_param;		/* Error Parameter */
	struct idp	ns_err_idp;		/* Initial segment of offending
						   packet */
	u_char		ns_err_lev2[12];	/* at least this much higher
						   level protocol */
};
struct  ns_epidp {
	struct idp ns_ep_idp;
	struct ns_errp ns_ep_errp;
};

#define	NS_ERR_UNSPEC	0	/* Unspecified Error detected at dest. */
#define	NS_ERR_BADSUM	1	/* Bad Checksum detected at dest */
#define	NS_ERR_NOSOCK	2	/* Specified socket does not exist at dest*/
#define	NS_ERR_FULLUP	3	/* Dest. refuses packet due to resource lim.*/
#define	NS_ERR_UNSPEC_T	0x200	/* Unspec. Error occured before reaching dest*/
#define	NS_ERR_BADSUM_T	0x201	/* Bad Checksum detected in transit */
#define	NS_ERR_UNREACH_HOST	0x202	/* Dest cannot be reached from here*/
#define	NS_ERR_TOO_OLD	0x203	/* Packet x'd 15 routers without delivery*/
#define	NS_ERR_TOO_BIG	0x204	/* Packet too large to be forwarded through
				   some intermediate gateway.  The error
				   parameter field contains the max packet
				   size that can be accommodated */
#define NS_ERR_MAX 20

/*
 * Variables related to this implementation
 * of the network systems error message protocol.
 */
struct	ns_errstat {
/* statistics related to ns_err packets generated */
	int	ns_es_error;		/* # of calls to ns_error */
	int	ns_es_oldshort;		/* no error 'cuz old ip too short */
	int	ns_es_oldns_err;	/* no error 'cuz old was ns_err */
	int	ns_es_outhist[NS_ERR_MAX];
/* statistics related to input messages processed */
	int	ns_es_badcode;		/* ns_err_code out of range */
	int	ns_es_tooshort;		/* packet < IDP_MINLEN */
	int	ns_es_checksum;		/* bad checksum */
	int	ns_es_badlen;		/* calculated bound mismatch */
	int	ns_es_reflect;		/* number of responses */
	int	ns_es_inhist[NS_ERR_MAX];
	u_short	ns_es_codes[NS_ERR_MAX];/* which error code for outhist
					   since we might not know all */
};

#ifdef _KERNEL
extern	struct	ns_errstat ns_errstat;
#endif
