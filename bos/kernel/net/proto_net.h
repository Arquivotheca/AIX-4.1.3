/* @(#)18	1.3  src/bos/kernel/net/proto_net.h, sysnet, bos411, 9428A410j 3/5/94 17:16:38 */
/*
 *   COMPONENT_NAME: SYSNET
 *
 *   FUNCTIONS: 
 *		
 *
 *   ORIGINS: 27,85
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

# define	P(s) s
/* Avoid scoping problems */
typedef union {
	struct sockaddr		*p1;
	struct socket		*p2;
	struct sockproto	*p3;
	struct mbuf		*p4;
	struct ifnet		*p5;
	struct ifqueue		*p6;
	struct ifaddr		*p7;
	struct ether_header	*p8;
	struct tty		*p9;
	struct slcompress	*p10;
	struct ip		*p11;
	struct in_addr		*p12;
	struct arpcom		*p13;
	struct arpent		*p14;
	struct domain		*p15;
	struct netisr		*p16;
	struct radix_node	*p17;
	struct radix_node_head	*p18;
	struct rawcb		*p19;
	struct route		*p20;
	struct rtentry		*p21;
	struct rt_metrics	*p22;
	struct sl_softc		*p23;
	struct arptab		*p24;
} proto_net_union_t;

/* if.c */
void	ifinit P((void));
void	ifubareset P((int));
void	ifreset P((int));
void	if_attach P((struct ifnet *));
struct	ifaddr *ifa_ifwithaddr P((struct sockaddr *));
struct	ifaddr *ifa_ifwithdstaddr P((struct sockaddr *));
struct	ifaddr *ifa_ifwithnet P((struct sockaddr *));
struct	ifaddr *ifa_ifwithaf P((int));
struct	ifaddr *ifaof_ifpforaddr P((struct sockaddr *, struct ifnet *));
void	link_rtrequest P((int, struct rtentry *, struct sockaddr *));
void	if_down P((struct ifnet *));
void	if_qflush P((struct ifqueue *));
void	if_slowtimo P((void));
struct	ifnet *ifunit P((char *));
int	ifioctl P((struct socket *, int, caddr_t));
int	ifconf P((int, caddr_t));

#ifdef _AIX_FULLOSF
/* if_arp.c */
void	arpinit P((void));
void	arpattach P((struct arpent *));
void	arptimer P((void));
void	arpintr P((void));
void	arpwhohas P((struct arpcom *, struct in_addr *));
int	arpresolve P((struct arpcom *, struct mbuf *, struct sockaddr *,
				u_char *, int *));
int	arpoutput P((struct arpcom *, struct mbuf *, u_char *));
int	arpioctl P((int, caddr_t));
void	arptfree(struct arptab *);
struct	arptab *arptlookup(struct arptab *, struct sockaddr *, struct ifnet *);
struct	arptab *arptnew(struct arptab *, struct sockaddr *, struct ifnet *);
void	arpttimer(struct arptab *);
char	*arp_sprintf P((char *, u_char *, int));

/* if_ethersubr.c */
void	etherinit P((void));
int	ether_arpoutput P((struct arpcom *, struct mbuf *, u_char *));
int	ether_output P((struct ifnet *, struct mbuf *, struct sockaddr *,
			struct rtentry *));
void	ether_input P((struct ifnet *, struct ether_header *, struct mbuf *));
char	*ether_sprintf P((u_char *));
#endif

/* if_loop.c */
void	loinit P((void));
void	loattach P((void));
int	looutput P((struct ifnet *, struct mbuf *, struct sockaddr *,
			struct rtentry *));
void	lortrequest P((int, struct rtentry *, struct sockaddr *));
int	loioctl P((struct ifnet *, int, caddr_t));

#ifdef _AIX_FULLOSF
/* if_sl.c */
void	slattach P((void));
int	slinit P((struct sl_softc *));
void	sldinit P((struct sl_softc *));
struct	mbuf *sl_btom P((struct sl_softc *, int));
int	sl_output P((struct ifnet *, struct mbuf *, struct sockaddr *,
                        struct rtentry *));
int	sl_ioctl P((struct ifnet *, int, caddr_t));

/* slcompress.c */
void	sl_compress_init P((struct slcompress *));
int	sl_compress_tcp P((struct mbuf *, struct ip *,struct slcompress *,int));
int	sl_uncompress_tcp P((u_char **, int, int, struct slcompress *));
#endif /* _AIX_FULLOSF */

/* netisr.c */
void	netinit P((int));
void	netisrinit P((void));
struct	netisr *netisr_lookup P((int));
int	netisr_add P((int, void (*)(void), struct ifqueue *, struct domain *));
int	netisr_del P((int));
int	netisr_input P((int, struct mbuf *, caddr_t, int));
int	netisr_af P((int));
void	netisr_timeout P((caddr_t));
void	Netintr P((void));
void	netisr_thread P((void));

/* radix.c */
struct	radix_node *rn_search P((caddr_t, struct radix_node *));
struct	radix_node *rn_search_m P((caddr_t, struct radix_node *, caddr_t));
struct	radix_node *rn_match P((caddr_t, struct radix_node *));
struct	radix_node *rn_newpair P((caddr_t, int, struct radix_node *));
struct	radix_node *rn_insert P((caddr_t, struct radix_node *, int *,
			struct radix_node *));
struct	radix_node *rn_addmask P((caddr_t, int, int));
struct	radix_node *rn_addroute P((caddr_t, caddr_t, struct radix_node *,
			struct radix_node *));
struct	radix_node *rn_delete P((caddr_t, caddr_t, struct radix_node *));
int	rn_inithead P((struct radix_node_head **, int, int));

/* raw_cb.c */
int	raw_attach P((struct socket *, int));
void	raw_detach P((struct rawcb *));
void	raw_disconnect P((struct rawcb *));
int	raw_bind P((struct socket *, struct mbuf *));

/* raw_usrreq.c */
void	raw_init P((void));
int	raw_input P((struct mbuf *, struct sockproto *,
			struct sockaddr *, struct sockaddr *));
void	raw_ctlinput P((int, struct sockaddr *, caddr_t));
int	raw_usrreq P((struct socket *, int, struct mbuf *, struct mbuf *,
			struct mbuf *));

/* route.c */
void	rtinithead P((int, int, void (*)(struct sockaddr *,struct sockaddr *)));
void	rtalloc P((struct route *));
void	rtalloc_nolock P((struct route *));
struct	rtentry *rtalloc1 P((struct sockaddr *, int));
struct	rtentry *rtalloc1_nolock P((struct sockaddr *, int));
void	rtfree P((struct rtentry *));
void	rtfree_nolock P((struct rtentry *));
void	rtredirect P((struct sockaddr *, struct sockaddr *, struct sockaddr *,
			int, struct sockaddr *, struct rtentry **));
int	rtioctl P((struct socket *, int, caddr_t));
struct	ifaddr *ifa_ifwithroute P((int, struct sockaddr *, struct sockaddr *));
int	rtrequest P((int, struct sockaddr *, struct sockaddr *,
			struct sockaddr *, int, struct rtentry **));
int	rtrequest_nolock P((int, struct sockaddr *, struct sockaddr *,
			struct sockaddr *, int, struct rtentry **));
void	rt_maskedcopy P((struct sockaddr *, struct sockaddr *,
			struct sockaddr *));
int	rtinit P((struct ifaddr *, int, int));

/* rtsock.c */
int	route_usrreq P((struct socket *, int, struct mbuf *, struct mbuf *,
			struct mbuf *));
int	route_output P((struct mbuf *, struct socket *));
void	rt_setmetrics P((u_long, struct rt_metrics *, struct rt_metrics *));
void	m_copyback P((struct mbuf *, int, int, caddr_t));
void	rt_missmsg P((int, struct sockaddr *, struct sockaddr *,
			struct sockaddr *, struct sockaddr *, int, int));
int	route_configure P((void));

#undef P
