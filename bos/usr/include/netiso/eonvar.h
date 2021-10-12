/* @(#)48	1.4  src/bos/usr/include/netiso/eonvar.h, sockinc, bos411, 9428A410j 5/10/91 16:40:34 */

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
 *	(#)eonvar.h	7.4 (Berkeley) 5/24/90
 */

#define EON_986_VERSION 0x3
#define EON_VERSION 0x1

#define EON_CACHESIZE 30

#define E_FREE 	1
#define E_LINK	2
#define E_ES 	3
#define E_IS 	4
 

/* 
 * this overlays a sockaddr_iso
 */

struct sockaddr_eon {
	u_char 			seon_len;	/* Length */
	u_char 			seon_family;	/* AF_ISO */
	u_char			seon_status;	/* overlays session suffixlen */
#define EON_ESLINK_UP		0x1
#define EON_ESLINK_DOWN		0x2
#define EON_ISLINK_UP		0x10
#define EON_ISLINK_DOWN		0x20
/* no change is neither up or down */
	u_char			seon_pad1;	/* 0, overlays tsfxlen */
	u_char			seon_adrlen;
	u_char			seon_afi;		/* 47 */
	u_char			seon_idi[2];	/* 0006 */
	u_char			seon_vers;		/* 03 */
	u_char			seon_glbnum[2];	/* see RFC 1069 */
	u_char			seon_RDN[2];	/* see RFC 1070 */
	u_char			seon_pad2[3];	/* see RFC 1070 */
	u_char			seon_LAREA[2];	/* see RFC 1070 */
	u_char			seon_pad3[2];	/* see RFC 1070 */
		/* right now ip addr is  aligned  -- be careful --
		 * future revisions may have it u_char[4]
		 */
	u_int			seon_ipaddr;	/* a.b.c.d */
	u_char			seon_protoid;	/* NSEL */
};

#ifdef EON_TEMPLATE
struct sockaddr_eon eon_template = {
	sizeof (eon_template), AF_ISO, 0, 0, 0x14,
	0x47, 0x0, 0x6, 0x3, 0
};
#endif

#define DOWNBITS ( EON_ESLINK_DOWN | EON_ISLINK_DOWN )
#define UPBITS ( EON_ESLINK_UP | EON_ISLINK_UP )

#define	SIOCSEONCORE _IOWR('i',10, struct iso_ifreq) /* EON core member */
#define	SIOCGEONCORE _IOWR('i',11, struct iso_ifreq) /* EON core member */

struct eon_hdr {
	u_char 	eonh_vers; /* value 1 */
	u_char 	eonh_class;  /* address multicast class, below */
#define		EON_NORMAL_ADDR		0x0
#define		EON_MULTICAST_ES	0x1
#define		EON_MULTICAST_IS	0x2
#define		EON_BROADCAST		0x3
	u_short eonh_csum;  /* osi checksum (choke)*/
};
struct eon_iphdr {
	struct	ip	ei_ip;
	struct	eon_hdr	ei_eh;
};
#define EONIPLEN (sizeof(struct eon_hdr) + sizeof(struct ip))

/* stole these 2 fields of the flags for I-am-ES and I-am-IS */
#define	IFF_ES	0x400
#define	IFF_IS	0x800

struct eon_stat {
	int	es_in_multi_es;
	int	es_in_multi_is;
	int	es_in_broad;
	int	es_in_normal;
	int	es_out_multi_es;
	int	es_out_multi_is;
	int	es_out_broad;
	int	es_out_normal;
	int	es_ipout;

	int	es_icmp[PRC_NCMDS];
	/* errors */
	int	es_badcsum;
	int	es_badhdr;
} eonstat;

#undef IncStat
#define IncStat(xxx) eonstat.xxx++

typedef struct qhdr {
	struct qhdr *link, *rlink;
} *queue_t;

struct eon_llinfo {
	struct	qhdr el_qhdr;		/* keep all in a list */
	int	el_flags;		/* cache valid ? */
	int	el_snpaoffset;		/* IP address contained in dst nsap */
	struct	rtentry *el_rt;		/* back pointer to parent route */
	struct	eon_iphdr el_ei;	/* precomputed portion of hdr */
	struct	route el_iproute;	/* if direct route cache IP info */
					/* if gateway, cache secondary route */
};
#define el_iphdr el_ei.ei_ip
#define el_eonhdr el_ei.ei_eh
