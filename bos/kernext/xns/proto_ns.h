/* @(#)97	1.3  src/bos/kernext/xns/proto_ns.h, sysxxns, bos411, 9428A410j 5/23/94 15:02:49 */
/*
 *   COMPONENT_NAME: SYSXXNS
 *
 *   FUNCTIONS: P
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
	struct ifnet		*p1;
	struct mbuf		*p2;
	struct route		*p3;
	struct sockaddr		*p4;
	struct sockaddr_ns	*p5;
	struct socket		*p6;
	struct nspcb		*p7;
	struct ns_ifaddr	*p8;
	struct ns_addr		*p9;
	struct in_addr		*p10;
	struct ifnet_en		*p11;
	struct sppcb		*p12;
	struct spidp		*p13;
} proto_ns_union_t;

/* idp_usrreq.c */
void	idp_input P((struct mbuf *, struct nspcb *));
void	idp_abort P((struct nspcb *));
struct	nspcb *idp_drop P((struct nspcb *, int));
int	idp_output P((struct nspcb *, struct mbuf *));
int	idp_ctloutput P((int, struct socket *, int, int, struct mbuf **));
int	idp_usrreq P((struct socket *, int, struct mbuf *,
				struct mbuf *, struct mbuf *));
int	idp_raw_usrreq P((struct socket *, int, struct mbuf *,
				struct mbuf *, struct mbuf *));

/* ns.c */
int	ns_control P((struct socket *, int, caddr_t, struct ifnet *));
int	ns_ifscrub P((struct ifnet *, struct ns_ifaddr *));
int	ns_ifinit P((struct ifnet *, struct ns_ifaddr *,
				struct sockaddr_ns *, int));
struct	ns_ifaddr *ns_iaonnetof P((struct ns_addr *));

/* somewhere */
u_short	ns_cksum P((struct mbuf *, int));

/* ns_error.c */
int	ns_err_x P((int));
int	ns_error P((struct mbuf *, int, int));
int	ns_printhost P((struct ns_addr *));
void	ns_err_input P((struct mbuf *));
u_long	nstime P((void));
int	ns_echo P((struct mbuf *));

/* ns_input.c */
void	ns_init P((void));
void	nsintr P((void));
void	idp_ctlinput P((int, struct sockaddr *, caddr_t));
int	idp_forward P((struct mbuf *));
int	idp_do_route P((struct ns_addr *, struct route *));
int	idp_undo_route P((struct route *));
int	ns_watch_output P((struct mbuf *, struct ifnet *));
int	ns_arpresolve P((struct arpcom *, struct mbuf *, 
		struct sockaddr *, struct ether_header *));


/* ns_ip.c */
struct	ifnet_en *nsipattach P((void));
int	nsipioctl P((struct ifnet *, int, caddr_t));
void	idpip_input P((struct mbuf *, int));
int	nsipoutput P((struct ifnet *, struct mbuf *, struct sockaddr *,
				struct rtentry *));
int	nsipstart P((struct ifnet *));
int	nsip_route P((struct socket *, struct mbuf *));
int	nsip_free P((struct ifnet *));
void	nsip_ctlinput P((int, struct sockaddr *, caddr_t));
int	nsip_rtchange P((struct in_addr *));

/* ns_output.c */
int	ns_output P((struct mbuf *, struct route *, int));

/* ns_pcb.c */
int	ns_pcballoc P((struct socket *, struct nspcb *));
int	ns_pcbbind P((struct nspcb *, struct mbuf *));
int	ns_pcbconnect P((struct nspcb *, struct mbuf *));
int	ns_pcbdisconnect P((struct nspcb *));
int	ns_pcbdetach P((struct nspcb *));
int	ns_setsockaddr P((struct nspcb *, struct mbuf *));
int	ns_setpeeraddr P((struct nspcb *, struct mbuf *));
int	ns_pcbnotify P((struct ns_addr *, int, void (*)(struct nspcb *), long));
int	ns_rtchange P((struct nspcb *));
struct	nspcb *ns_pcblookup P((struct ns_addr *, u_short, int));

/* ns_proto.c */
int	ns_configure ();		/* No prototypes here */

/* spp_debug.c */
int	spp_trace P((int, u_char, struct sppcb *, struct spidp *, int));

/* spp_usrreq.c */
void	spp_init P((void));
void	spp_input P((struct mbuf *, struct nspcb *));
int	spp_reass P((struct sppcb *, struct spidp *));
void	spp_ctlinput P((int, struct sockaddr *, caddr_t));
void	spp_quench P((struct nspcb *));
int	spp_fixmtu P((struct nspcb *));
int	spp_output P((struct sppcb *, struct mbuf *));
int	spp_setpersist P((struct sppcb *));
int	spp_ctloutput P((int, struct socket *, int, int, struct mbuf **));
int	spp_usrreq P((struct socket *, int, struct mbuf *,
				struct mbuf *, struct mbuf *));
int	spp_usrreq_sp P((struct socket *, int, struct mbuf *,
				struct mbuf *, struct mbuf *));
int	spp_template P((struct sppcb *));
struct	sppcb *spp_close P((struct sppcb *));
struct	sppcb *spp_usrclosed P((struct sppcb *));
struct	sppcb *spp_disconnect P((struct sppcb *));
struct	sppcb *spp_drop P((struct sppcb *, int));
void	spp_abort P((struct nspcb *));
void	spp_fasttimo P((void));
void	spp_slowtimo P((void));
struct	sppcb *spp_timers P((struct sppcb *, int));

#undef P
