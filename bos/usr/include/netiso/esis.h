/* @(#)49	1.6  src/bos/usr/include/netiso/esis.h, sockinc, bos411, 9428A410j 3/5/94 12:41:04 */
/*
 * 
 * COMPONENT_NAME: SOCKINC
 * 
 * FUNCTIONS: 
 *
 * ORIGINS: 26 27 
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*
 * ARGO Project, Computer Sciences Dept., University of Wisconsin - Madison
 */
/*
 *	Header: esis.h,v 4.7 88/09/15 11:24:18 hagens Exp $
 *	Source: /usr/argo/sys/netiso/RCS/esis.h,v $
 *	esis.h	7.3 (Berkeley) 8/29/89 *
 */

#ifndef BYTE_ORDER
/*
 * Definitions for byte order,
 * according to byte significance from low address to high.
 */
#define	LITTLE_ENDIAN	1234	/* least-significant byte first (vax) */
#define	BIG_ENDIAN	4321	/* most-significant byte first (IBM, net) */
#define	PDP_ENDIAN	3412	/* LSB first in word, MSW first in long (pdp) */

#ifdef vax
#define	BYTE_ORDER	LITTLE_ENDIAN
#else
#define	BYTE_ORDER	BIG_ENDIAN	/* mc68000, tahoe, most others */
#endif
#endif /* BYTE_ORDER */

#define	SNPAC_AGE		60			/* seconds */
#define	ESIS_CONFIG		60			/* seconds */
#define	ESIS_HT			(ESIS_CONFIG * 2)

/*
 *	Fixed part of an ESIS header
 */
struct esis_fixed {
	u_char	esis_proto_id;		/* network layer protocol identifier */
	u_char	esis_hdr_len;		/* length indicator (octets) */
	u_char	esis_vers;			/* version/protocol identifier extension */
	u_char	esis_res1;			/* reserved */
	u_char	esis_type;			/* type code */
/* technically, type should be &='d 0x1f */
#define ESIS_ESH	0x02		/* End System Hello */
#define ESIS_ISH	0x04		/* Intermediate System Hello */
#define ESIS_RD		0x06		/* Redirect */
	u_char	esis_ht_msb;		/* holding time (seconds) high byte */
	u_char	esis_ht_lsb;		/* holding time (seconds) low byte */
	u_char	esis_cksum_msb;		/* checksum high byte */
	u_char	esis_cksum_lsb;		/* checksum low byte */
};
/*
 * Values for ESIS datagram options
 */
#define ESISOVAL_NETMASK	0xe1	/* address mask option, RD PDU only */
#define ESISOVAL_SNPAMASK	0xe2	/* snpa mask option, RD PDU only */
#define ESISOVAL_ESCT		0xc6	/* end system conf. timer, ISH PDU only */


#define	ESIS_CKSUM_OFF		0x07
#define ESIS_CKSUM_REQUIRED(pdu)\
	((pdu->esis_cksum_msb != 0) || (pdu->esis_cksum_lsb != 0))

#define	ESIS_VERSION	1

struct esis_stat {
	u_short		es_nomem;			/* insufficient memory to send hello */
	u_short		es_badcsum;			/* incorrect checksum */
	u_short		es_badvers;			/* incorrect version number */
	u_short		es_badtype;			/* unknown pdu type field */
	u_short		es_toosmall;		/* packet too small */
	u_short		es_eshsent;			/* ESH sent */
	u_short		es_eshrcvd;			/* ESH rcvd */
	u_short		es_ishsent;			/* ISH sent */
	u_short		es_ishrcvd;			/* ISH rcvd */
	u_short		es_rdsent;			/* RD sent */
	u_short		es_rdrcvd;			/* RD rcvd */
};

#ifdef	_KERNEL
struct esis_stat esis_stat;
#endif	/* _KERNEL */
