/* @(#)54	1.5  src/bos/usr/include/netiso/iso_snpac.h, sockinc, bos411, 9428A410j 3/5/94 12:41:15 */

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
/*	(#)iso_snpac.h	7.7 (Berkeley) 6/22/90 */

struct snpa_req {
	struct iso_addr	sr_isoa;		/* nsap address */
	u_char			sr_len;			/* length of snpa */
	u_char			sr_snpa[MAX_SNPALEN];	/* snpa associated 
												with nsap address */
	u_char			sr_flags;		/* true if entry is valid */
	u_short			sr_ht;			/* holding time */
};

#define	SNPA_VALID		0x01
#define	SNPA_ES			0x02
#define SNPA_IS			0x04
#define	SNPA_PERM		0x10

struct systype_req {
	short	sr_holdt;		/* holding timer */
	short	sr_configt;		/* configuration timer */
	short	sr_esconfigt;	/* suggested ES configuration timer */
	char	sr_type;		/* SNPA_ES or SNPA_IS */
};

struct esis_req {
	short	er_ht;			/* holding time */
	u_char	er_flags;		/* type and validity */
};
/*
 * Space for this structure gets added onto the end of a route
 * going to an ethernet or other 802.[45x] device.
 */

struct llinfo_llc {
	struct	llinfo_llc *lc_next;	/* keep all llc routes linked */
	struct	llinfo_llc *lc_prev;	/* keep all llc routes linked */
	struct	rtentry *lc_rt;			/* backpointer to route */
	struct	esis_req lc_er;			/* holding time, etc */
#define lc_ht		lc_er.er_ht
#define lc_flags	lc_er.er_flags
};


/* ISO arp IOCTL data structures */

#define	SIOCSSTYPE 	_IOW('a', 39, struct systype_req) /* set system type */
#define	SIOCGSTYPE 	_IOR('a', 40, struct systype_req) /* get system type */

#ifdef	_KERNEL
struct llinfo_llc llinfo_llc;	/* head for linked lists */
#endif	/* _KERNEL */
