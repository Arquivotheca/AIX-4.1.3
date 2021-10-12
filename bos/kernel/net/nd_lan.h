/* @(#)12       1.10  src/bos/kernel/net/nd_lan.h, sysnet, bos41J, 9514A_all 4/2/95 18:14:36  */
/*
 *   COMPONENT_NAME: SYSNET
 *
 *   FUNCTIONS: DELIVER_PACKET
 *		DMX_8022_GET_USER
 *		IFSTUFF_AND_DELIVER
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _NET_ND_LAN_H
#define _NET_ND_LAN_H

#ifndef _802_2_LLC
#define	_802_2_LLC
/*
 * 802.2 LLC header
 */
struct ie2_llc_hdr {
	unsigned char	dsap;		/* DSAP				*/
	unsigned char	ssap;		/* SSAP				*/
	unsigned char	ctrl;		/* control field		*/
};

struct ie2_llc_snaphdr {
	unsigned char	dsap;		/* DSAP				*/
	unsigned char	ssap;		/* SSAP				*/
	unsigned char	ctrl;		/* control field		*/
	unsigned char	prot_id[3];	/* protocol id			*/
	unsigned short	type;		/* type field			*/
};

struct ie2_xid_ifield {
	unsigned char	dsap;		/* DSAP				*/
	unsigned char	ssap;		/* SSAP				*/
	unsigned char	ctrl;		/* control field		*/
	unsigned char	xid_info[3];	/* XID information field	*/
};

#endif

struct config_proto {
	int			(*loop)();/* protocol loopback function	*/
        struct ifqueue 		*loopq; /* input queue, may be NULL	*/
	u_short			netisr;	 /* isr number for schednetisr	*/
	int			(*resolve)(); /* address resolution 	*/
	int			(*ioctl)(); /* address resolution ioctl */
	int			(*whohas)(); /* address resolution whohas */
};

/*
 * SAPS
 */
#define	DSAP_SNAP	0xaa		/* SNAP SSAP */
#define	SSAP_SNAP	0xaa		/* SNAP DSAP */
#define	DSAP_INET	DSAP_SNAP	/* INET SSAP */
#define	SSAP_INET	SSAP_SNAP	/* INET DSAP */
#define DSAP_APPLETALK 	0xe0 		/* NETWARE APPLETALK DSAP*/
#define SSAP_APPLETALK 	0xe0		/* NETWARE APPLETALK SSAP*/
#define DSAP_ISO        0xfe            /* ISO DSAP */
#define SSAP_ISO        0xfe            /* ISO SSAP */
#define DSAP_XNS        0xfa            /* XNS DSAP */
#define SSAP_XNS        0xfa            /* XNS SSAP */
#define DSAP_NETWARE 	0xff		/* NetWare DSAP */
#define SSAP_NETWARE 	0xff		/* NetWare SSAP */
#define DSAP_NETBIOS 	0xf0		/* NETBIOS DSAP */
#define SSAP_NETBIOS 	0xf0		/* NETBIOS SSAP */

#define	CTRL_UI		0x03		/* unnumbered info		*/
#define CTRL_XID	0xaf		/* eXchange IDentifier		*/
#define	CTRL_TEST	0xe3		/* test frame			*/
#define	SSAP_RESP	0x01		/* SSAP response bit */

#define	NS_MAX_SAPS	256

/*
 * SNAP TYPES.  This usually correspond to ethertypes...but not always...
 */
#define SNAP_TYPE_AP	0x809b
#define SNAP_TYPE_ARP	0x0806
#define SNAP_TYPE_IP	0x0800

struct ns_8022_user {
	struct ns_8022_user	*next;
	struct ns_8022_user	*prev;
	struct ns_user 		user;
	struct ns_8022 		filter;
};

struct ns_8022_user_head {
	struct ns_8022_user	*next;
	struct ns_8022_user	*prev;
};

struct com_status_user {
	struct com_status_user	*next;
	struct com_status_user	*prev;
	struct ns_statuser 	user;
	struct ns_com_status	filter;
};

#ifndef _SYS_CDLI_H
#include <sys/cdli.h>
#endif

struct ns_dmx_ctl {
	struct ns_8022_user_head	dsap_heads[NS_MAX_SAPS];
	struct com_status_user		com_status_head;	
	nd_dmxstats_t			nd_dmxstats;
};



#define	DMX_8022_GET_USER(retfup, sap, etype, org, nsdemux) { 		\
	struct ns_8022_user	*fup;					\
	struct ns_8022_user	*filter_head;				\
									\
	filter_head = (struct ns_8022_user *) &((nsdemux)->dsap_heads[sap]);\
	fup = filter_head->next;					\
	if (fup->filter.filtertype != NS_8022_LLC_DSAP) { 		\
		for ( ; fup != filter_head; fup = fup->next) {		\
			if ( (fup->filter.ethertype == etype) &&	\
			     (!memcmp(fup->filter.orgcode, org, sizeof(org))) )\
				break;					\
		}							\
	} 								\
	if (fup == filter_head)						\
		retfup = NULL;						\
	else								\
		retfup = fup;						\
}



#define IFSTUFF_AND_DELIVER(nsuser, nddp, m, macp, extp) {		\
	IFQ_LOCK_DECL()							\
	LOCK_ASSERTL_DECL						\
									\
	if (nsuser->user.ifp) {						\
		if ((nsuser->user.ifp->if_flags & IFF_UP) == 0) {	\
			m_freem(m);					\
			nsuser->user.ifp->if_iqdrops++;			\
			goto out;					\
		} else {						\
			nsuser->user.ifp->if_ipackets++;		\
			nsuser->user.ifp->if_ibytes += m->m_pkthdr.len;	\
			m->m_pkthdr.rcvif = nsuser->user.ifp;		\
		}							\
	}								\
	if (nsuser->user.protoq == NULL) {				\
			(*(nsuser->user.isr))(nddp, m, macp, (extp));	\
	}								\
	else {								\
		IFQ_LOCK(nsuser->user.protoq);				\
		if (IF_QFULL(nsuser->user.protoq)) {			\
			IF_DROP(nsuser->user.protoq);			\
			nddp->ndd_demuxer->nd_dmxstats.nd_nobufs++;	\
			m_freem(m);					\
		} else {						\
			IF_ENQUEUE_NOLOCK(nsuser->user.protoq, m);	\
		}							\
		IFQ_UNLOCK(nsuser->user.protoq);			\
		schednetisr(nsuser->user.netisr);			\
	}								\
out:	;								\
}

#define DELIVER_PACKET(nsuser, nddp, m, macp, extp) {			\
	IFQ_LOCK_DECL()							\
	LOCK_ASSERTL_DECL						\
									\
	if (nsuser->protoq == NULL) {					\
		(*(nsuser->isr))(nddp, m, macp, (extp));		\
	}								\
	else {								\
		IFQ_LOCK(nsuser->protoq);				\
		if (IF_QFULL(nsuser->protoq)) {				\
			IF_DROP(nsuser->protoq);			\
			nddp->ndd_demuxer->nd_dmxstats.nd_nobufs++;	\
			m_freem(m);					\
		} else {						\
			IF_ENQUEUE_NOLOCK(nsuser->protoq, m);		\
		}							\
		IFQ_UNLOCK(nsuser->protoq);				\
		schednetisr(nsuser->netisr);				\
	}								\
}

#define DEMUXER_LOCK_DECL()	int _dmx_s;

#if     NETSYNC_LOCK
#define DEMUXER_LOCKINIT(lockp) {					\
	lock_alloc(lockp, LOCK_ALLOC_PIN, DEMUXER_LOCK_FAMILY, lockp);	\
	simple_lock_init((lockp));					\
}
#define DEMUXER_LOCK(lockp) 	_dmx_s = disable_lock(PL_IMP, (lockp))
#define DEMUXER_UNLOCK(lockp)    unlock_enable(_dmx_s, (lockp))
#else
#define DEMUXER_LOCKINIT(lockp)
#define DEMUXER_LOCK(lockp)      NETSPL(_dmx_s, imp)
#define DEMUXER_UNLOCK(lockp)    NETSPLX(_dmx_s)
#endif


#ifdef _KERNEL
struct af_ent {
	struct config_proto config;
	u_short sap;
	u_short type;
};
extern struct af_ent	af_table[];
#endif

/* 
 * This struct passed up to demuxer users who register for non-isr
 * service.
 */
struct isr_data_ext {
		caddr_t 	isr_data; 	/* Cookie as registered. */
		caddr_t 	dstp; 		/* Destination address. */
		int 		dstlen; 
		caddr_t 	srcp; 		/* source address */
		int 		srclen;
		caddr_t		segp;		/* routing segment pointer */
		int 		seglen;
		caddr_t 	llcp;	 	/* pointer to llc */
		int 		llclen;
		ulong 		reserved[6];	/* for future use */
	};

struct helpers {
	int 				pkt_format;
		/* Possible values are:
			NS_PROTO
			NS_PROTO_SNAP
			NS_INCLUDE_LLC
		*/
	ushort				ethertype;
		/* If >0, then MAC type is standard ethernet */
	union {
		struct ie2_llc_hdr 	llc;
		struct ie2_llc_snaphdr 	llcsnap;
	} sapu;
	caddr_t				segp;
	ulong				seglen;
};

struct output_bundle {
	caddr_t  	key_to_find; 	/* Usually the destination hwaddr */
	struct helpers 	helpers;	/* Hints, like sap, snap type, etc */
};

/*
 * ie2_llc -    fill in an IEEE 802.2 LLC field.
 *
 * Input:
 *      llc     -       ^ to LLC
 *      type    -       pkt type
 */
#define ie2_llc(llc, snap_type)						\
        ((struct ie2_llc_snaphdr *)(llc))->dsap       = DSAP_INET; 	\
        ((struct ie2_llc_snaphdr *)(llc))->ssap       = SSAP_INET;	\
        ((struct ie2_llc_snaphdr *)(llc))->ctrl       = CTRL_UI;	\
        ((struct ie2_llc_snaphdr *)(llc))->prot_id[0] = 0;		\
        ((struct ie2_llc_snaphdr *)(llc))->prot_id[1] = 0;		\
        ((struct ie2_llc_snaphdr *)(llc))->prot_id[2] = 0;		\
        ((struct ie2_llc_snaphdr *)(llc))->type       = (snap_type);

#endif
