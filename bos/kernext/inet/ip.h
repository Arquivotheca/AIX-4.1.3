/* @(#)51	1.16  src/bos/kernext/inet/ip.h, sockinc, bos411, 9428A410j 9/15/93 16:31:43 */
/*
 *   COMPONENT_NAME: SYSXINET
 *
 *   FUNCTIONS: IPOPT_CLASS
 *		IPOPT_COPIED
 *		IPOPT_NUMBER
 *		
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
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
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
 *	Base:	ip.h	7.8 (Berkeley) 2/20/89
 *	Merged:	ip.h	7.10 (Berkeley) 6/28/90
 */

#ifndef	_NETINET_IP_H
#define	_NETINET_IP_H

#include <netinet/in_systm.h>

/*
 * Definitions for internet protocol version 4.
 * Per RFC 791, September 1981.
 */
#define	IPVERSION	4

/*
 * Structure of an internet header, naked of options.
 *
 * Beware: We now correctly declare ip_len and ip_off to be u_short.
 *
 * First we need to define a structure overlay to provide ANSI-C
 * compatibility with bitfield widths. The use of bitfields as
 * structure overlays, especially for packets off the network,
 * is deprecated, and recoding to use the _NO_BITFIELDS version
 * is recommended. This structure is also used by ip_var.h and tcp.h.
 */

struct ip_firstfour {
#if	defined(_NO_BITFIELDS)
	u_char	ip_fvhl;			/* version and header length */
	u_char	ip_ftos;			/* type of service */
	u_short	ip_flen;			/* total length or tcp window */
#define	ip_fwin	ip_flen
#else
/* BSD Provide code compat with bitfields */
	union {
		u_long	ip_w;			/* provides historical width */
		struct {
		    unsigned int
#if	BYTE_ORDER == BIG_ENDIAN
			ip_xv:4,		/* version */
			ip_xhl:4,		/* header length */
			ip_xtos:8,		/* type of service */
			ip_xlen:16;		/* total length */
#endif
#if	BYTE_ORDER == LITTLE_ENDIAN || BYTE_ORDER == PDP_ENDIAN
			ip_xhl:4,		/* header length */
			ip_xv:4,		/* version */
			ip_xtos:8,		/* type of service */
			ip_xlen:16;		/* total length */
#endif
		} ip_x;
	} ip_vhltl;
#define ip_fv	ip_vhltl.ip_x.ip_xv
#define	ip_fhl	ip_vhltl.ip_x.ip_xhl
#define	ip_ftos	ip_vhltl.ip_x.ip_xtos
#define	ip_flen	ip_vhltl.ip_x.ip_xlen
#define	ip_fwin	ip_flen
#endif
};

struct ip {
	struct	ip_firstfour ip_ff;	/* see above */
#define	ip_v	ip_ff.ip_fv
#define	ip_hl	ip_ff.ip_fhl
#define	ip_vhl	ip_ff.ip_fvhl
#define	ip_tos	ip_ff.ip_ftos
#define	ip_len	ip_ff.ip_flen
	u_short	ip_id;			/* identification */
	u_short	ip_off;			/* fragment offset field */
#define	IP_DF 0x4000			/* dont fragment flag */
#define	IP_MF 0x2000			/* more fragments flag */
	u_char	ip_ttl;			/* time to live */
	u_char	ip_p;			/* protocol */
	u_short	ip_sum;			/* checksum */
	struct	in_addr ip_src,ip_dst;	/* source and dest address */
};

#define	IP_MAXPACKET	65535		/* maximum packet size */

/*
 * Definitions for IP type of service (ip_tos)
 */
#define	IPTOS_LOWDELAY		0x10
#define	IPTOS_THROUGHPUT	0x08
#define	IPTOS_RELIABILITY	0x04

/*
 * Definitions for IP precedence (also in ip_tos) (hopefully unused)
 */
#define	IPTOS_PREC_NETCONTROL		0xe0
#define	IPTOS_PREC_INTERNETCONTROL	0xc0
#define	IPTOS_PREC_CRITIC_ECP		0xa0
#define	IPTOS_PREC_FLASHOVERRIDE	0x80
#define	IPTOS_PREC_FLASH		0x60
#define	IPTOS_PREC_IMMEDIATE		0x40
#define	IPTOS_PREC_PRIORITY		0x20
#define	IPTOS_PREC_ROUTINE		0x10

/*
 * Definitions for options.
 */
#define	IPOPT_COPIED(o)		((o)&0x80)
#define	IPOPT_CLASS(o)		((o)&0x60)
#define	IPOPT_NUMBER(o)		((o)&0x1f)

#define	IPOPT_CONTROL		0x00
#define	IPOPT_RESERVED1		0x20
#define	IPOPT_DEBMEAS		0x40
#define	IPOPT_RESERVED2		0x60

#define	IPOPT_EOL		0		/* end of option list */
#define	IPOPT_NOP		1		/* no operation */

#define	IPOPT_RR		7		/* record packet route */
#define	IPOPT_TS		68		/* timestamp */
#define	IPOPT_SECURITY		130		/* provide s,c,h,tcc */
#define	IPOPT_LSRR		131		/* loose source route */
#define	IPOPT_SATID		136		/* satnet id */
#define	IPOPT_SSRR		137		/* strict source route */

/*
 * Offsets to fields in options other than EOL and NOP.
 */
#define	IPOPT_OPTVAL		0		/* option ID */
#define	IPOPT_OLEN		1		/* option length */
#define IPOPT_OFFSET		2		/* offset within option */
#define	IPOPT_MINOFF		4		/* min value of above */

/*
 * Time stamp option structure.
 */
struct	ip_timestamp {
#if	defined(_KERNEL) || defined(_NO_BITFIELDS)
	u_char	ipt_code;		/* IPOPT_TS */
	u_char	ipt_len;		/* size of structure (variable) */
	u_char	ipt_ptr;		/* index of current entry */
	u_char	ipt_oflg;		/* overflow and flags */
#else
/* BSD Provide code compat with bitfields */
	union {
		u_long	ipt_w;		/* provides historical width */
		struct {
		    unsigned int
#if	BYTE_ORDER == BIG_ENDIAN
			ipt_xcode:8,	/* IPOPT_TS */
			ipt_xlen:8,	/* size of structure (variable) */
			ipt_xptr:8,	/* index of current array */
			ipt_xoflw:4,	/* overflow counter */
			ipt_xflg:4;	/* flags, see below */
#endif
#if	BYTE_ORDER == LITTLE_ENDIAN || BYTE_ORDER == PDP_ENDIAN
			ipt_xcode:8,	/* IPOPT_TS */
			ipt_xlen:8,	/* size of structure (variable) */
			ipt_xptr:8,	/* index of current array */
			ipt_xflg:4,	/* flags, see below */
			ipt_xoflw:4;	/* overflow counter */
#endif
		} ip_x;
	} ipt_clpof;
#define	ipt_code	ipt_clpof.ip_x.ipt_xcode
#define	ipt_len		ipt_clpof.ip_x.ipt_xlen
#define	ipt_ptr		ipt_clpof.ip_x.ipt_xptr
#define	ipt_oflw	ipt_clpof.ip_x.ipt_xoflw
#define	ipt_flg		ipt_clpof.ip_x.ipt_xflg
#endif
	union ipt_timestamp {
		n_long	ipt_time[1];
		struct	ipt_ta {
			struct in_addr ipt_addr;
			n_long ipt_time;
		} ipt_ta[1];
	} ipt_timestamp;
};

/* flag bits for ipt_flg */
#define	IPOPT_TS_TSONLY		0		/* timestamps only */
#define	IPOPT_TS_TSANDADDR	1		/* timestamps and addresses */
#define	IPOPT_TS_PRESPEC	3		/* specified modules only */

/* bits for security (not byte swapped) */
#define	IPOPT_SECUR_UNCLASS	0x0000
#define	IPOPT_SECUR_CONFID	0xf135
#define	IPOPT_SECUR_EFTO	0x789a
#define	IPOPT_SECUR_MMMM	0xbc4d
#define	IPOPT_SECUR_PROG	0x5e26
#define	IPOPT_SECUR_RESTR	0xaf13
#define	IPOPT_SECUR_SECRET	0xd788
#define	IPOPT_SECUR_TOPSECRET	0x6bc5

/*
 * Internet implementation parameters.
 */
#define	MAXTTL		255		/* maximum time to live (seconds) */
#define	IPFRAGTTL	60		/* time to live for frags, slowhz */
#define	IPTTLDEC	1		/* subtracted when forwarding */

#define	IP_MSS		576		/* default maximum segment size */
#endif
