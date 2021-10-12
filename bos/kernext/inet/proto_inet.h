/* @(#)84	1.13  src/bos/kernext/inet/proto_inet.h, sysxinet, bos411, 9438C411a 9/23/94 10:15:24 */
/*
 *   COMPONENT_NAME: SYSXINET
 *
 *   FUNCTIONS: P
 *		
 *
 *   ORIGINS: 27,85,89
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * 
 * (c) Copyright 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 * 
 */
/*
 * OSF/1 1.2
 */

# define	P(s) s
/* Avoid scoping problems */
struct arpcom; struct in_addr; struct arptab;
struct mbuf; struct ifnet;
struct sockaddr; struct sockaddr_in; struct socket;
struct in_ifaddr; struct inpcb; struct ipq;
struct ipasfrag; struct route; struct ip;
struct tcpcb; struct tcpiphdr;
/* The tcp_seq typedef is a problem. Better to cast or rearrange? */
#define proto_tcp_seq	u_long

/* if_ether.c */
void	in_arpinput P((struct arpcom *, struct mbuf *));
void	arpoutput P((struct arpcom *, struct mbuf *, u_char *, u_short));
int	arpresolve P((struct arpcom *, struct mbuf *, struct sockaddr_in *, 
		struct ether_header *));
int	arpwhohas P((struct arpcom *, struct in_addr *));

/* if_ie5.c */
int	ie5_arpinput P((struct arpcom *, struct mbuf *,
		caddr_t));
int	ie5_arpresolve P((struct arpcom *, struct mbuf *, struct sockaddr_in *, 
		caddr_t));
int	ie5_arpwhohas P((struct arpcom *, struct in_addr *));

/* if_fddi.c */
int	fddi_arpinput P((struct arpcom *, struct mbuf *, 
		struct fddi_mac_hdr *));
int	fddi_arpresolve P((struct arpcom *, struct mbuf *, 
		struct sockaddr_in *, caddr_t));
int	fddi_arpwhohas P((struct arpcom *, struct in_addr *));

/* if_arp.c */
void	arptfree P((struct arptab *));
void	arptimer P((void));
int	arpinit P((void));
struct	arptab *arptnew P((struct in_addr *, struct ifnet *));

/* in_init.c */
int	in_arpresolve P((struct arpcom *, struct mbuf *, struct in_addr *, 
		u_char *));
void 	in_arpintr P((struct ndd *, struct mbuf *, caddr_t));
#ifdef _SUN
void	in_rarpintr P((struct ndd *, struct mbuf *, caddr_t));
#endif
int	arpioctl P((int, caddr_t));

/* in.c */
struct	in_addr in_makeaddr P((u_long, u_long));
u_long	in_netof P((struct in_addr));
void	in_sockmaskof P((struct in_addr, struct sockaddr_in *));
u_long	in_lnaof P((struct in_addr));
int	in_localaddr P((struct in_addr));
int	in_canforward P((struct in_addr));
int	in_control P((struct socket *, int, caddr_t, struct ifnet *));
void	in_ifscrub P((struct ifnet *, struct in_ifaddr *));
int	in_ifinit P((struct ifnet *, struct in_ifaddr *, struct sockaddr_in *,
				int));
struct	in_ifaddr *in_iaonnetof P((u_long));
int	in_broadcast P((struct in_addr));

#ifdef AIX_FULLOSF
/* in_cksum.c or machine dependent */
int	in_cksum P((struct mbuf *, int));
#endif

/* in_pcb.c */
int	in_pcballoc P((struct socket *, struct inpcb *));
int	in_pcbbind P((struct inpcb *, struct mbuf *));
int	in_pcbbind_nolock P((struct inpcb *, struct mbuf *));
int	in_pcbconnect P((struct inpcb *, struct mbuf *));
int	in_pcbconnect_nolock P((struct inpcb *, struct mbuf *));
void	in_pcbdisconnect P((struct inpcb *));
void	in_pcbdisconnect_nolock P((struct inpcb *));
void	in_pcbdetach P((struct inpcb *));
void	in_pcbdetach_nolock P((struct inpcb *));
void	in_pcbfree P((struct inpcb *));
void	in_pcbfree_nolock P((struct inpcb *));
void	in_pcb_hash_del P((struct inpcb *));
void	in_pcb_hash_ins P((struct inpcb *));
void	in_setsockaddr P((struct inpcb *, struct mbuf *));
void	in_setpeeraddr P((struct inpcb *, struct mbuf *));
void	in_pcbnotify P((struct inpcb *, struct sockaddr *, u_short,
		struct in_addr, u_short, int, void (*)(struct inpcb *, int)));
void	in_losing P((struct inpcb *));
void	in_rtchange P((struct inpcb *, int));
struct	inpcb *in_pcblookup P((struct inpcb *, struct in_addr,
				u_short, struct in_addr, u_short, int));
struct	inpcb *in_pcblookup_nolock P((struct inpcb *, struct in_addr,
				u_short, struct in_addr, u_short, int));
struct	inpcb *in_pcbmatch P((struct inpcb *, struct in_addr,
				u_short, struct in_addr, u_short));
struct 	inpcb *in_pcbhashlookup();

/* in_proto.c */
int	inet_configure ();		/* No prototypes here */

/* ip_icmp.c */
void	icmp_error P((struct mbuf *, int, int, struct in_addr));
void	icmp_input P((struct mbuf *, int));
void	icmp_send P((struct mbuf *, struct mbuf *));
void	icmp_reflect P((struct mbuf *));
void	dequeue_pfctlinput P((void));
void	queue_pfctlinput P((int, struct sockaddr_in *));
struct	in_ifaddr *ifptoia P((struct ifnet *));
n_time	iptime P((void));

#ifdef IP_MULTICAST
/* igmp.c */
void igmp_init P((void));
void igmp_input P((struct mbuf *, int));
void igmp_fasttimo P((void));
void igmp_joingroup P((struct in_multi *));
void igmp_leavegroup P((struct in_multi *));
void igmp_sendreport P((struct in_multi *inm));

/* if_mcast_aux.c */
void diver_addmulti P((struct ifreq, struct arpcom, void (*)(struct sock_addr_in *, u_char *, u_char *), int *, char *));
void diver_delmulti P((struct ifreq, struct arpcom, void (*)(struct sock_addr_in *, u_char *, u_char *), int *, char *));

#endif /* IP_MULTICAST */

/* ip_input.c */
void	ip_init P((void));
void	ipintr P((void));
void 	ipintr_noqueue P((struct ndd *, struct mbuf *));
struct	mbuf *ip_reass P((struct mbuf *));
void	ip_freef P((struct ipq *));
void	ip_enq P((struct ipasfrag *, struct ipasfrag *));
void	ip_deq P((struct ipasfrag *));
void	ip_slowtimo P((void));
void	ip_drain P((void));
int	ip_dooptions P((struct mbuf *));
struct	in_ifaddr *ip_rtaddr P((struct in_addr));
void	save_rte P((u_char *, struct in_addr));
struct	mbuf *ip_srcroute P((void));
void	ip_stripoptions P((struct mbuf *, struct mbuf *));
void	ip_forward P((struct mbuf *, int));
void	mach_net_ipinit P((void));
void	mach_net_ipdone P((void));
int	mach_net_ipsend P((caddr_t, int));
struct	mbuf *mach_net_ipreceive P((struct mbuf *, int));

/* ip_output.c */
#ifdef IP_MULTICAST
int	ip_output P((struct mbuf *, struct mbuf *, struct route *, int, struct mbuf *));
#else
int	ip_output P((struct mbuf *, struct mbuf *, struct route *, int));
#endif /* IP_MULTICAST */
struct	mbuf *ip_insertoptions P((struct mbuf *, struct mbuf *, int *));
int	ip_optcopy P((struct ip *, struct ip *));
int	ip_ctloutput P((int, struct socket *, int, int, struct mbuf **));
int	ip_pcbopts P((struct mbuf **, struct mbuf *));
#ifdef IP_MULTICAST
int ip_setmoptions P((struct socket *, int, struct mbuf **, struct mbuf *));
int ip_getmoptions P((int, struct mbuf *, struct mbuf **m));
void ip_freemoptions P((struct mbuf *));
void ip_mloopback P((struct ifnet *, struct mbuf *, struct sockaddr_in *));
#endif /* IP_MULTICAST */

/* raw_ip.c */
void	rip_input P((struct mbuf *, int));
int	rip_output P((struct mbuf *, struct socket *));
int	rip_ctloutput P((int, struct socket *, int, int, struct mbuf **));
int	rip_usrreq P((struct socket *, int, struct mbuf *, struct mbuf *,
				struct mbuf *));

/* tcp_debug.c */
void	tcp_trace P((int, int, struct tcpcb *, struct tcpiphdr *, int));

/* tcp_input.c */
int	tcp_reass P((struct tcpcb *, struct tcpiphdr *, struct mbuf *));
void	tcp_input P((struct mbuf *, int));
void	tcp_dooptions P((struct tcpcb *, struct mbuf *, struct tcpiphdr *,
				u_long *, u_long *, int *));
void	tcp_pulloutofband P((struct socket *, struct tcpiphdr *,
				struct mbuf *));
void	tcp_xmit_timer();
int	tcp_mss P((register struct tcpcb *, u_short));

/* tcp_output.c */
int	tcp_output P((struct tcpcb *));
void	tcp_setpersist P((struct tcpcb *));

/* tcp_subr.c */
void	tcp_init P((void));
void	tcp_template P((struct tcpcb *));
void	tcp_drain P((void));
void	tcp_quench P((struct inpcb *, int));
void	tcp_ctlinput P((int, struct sockaddr *, caddr_t));
void	tcp_respond P((struct tcpcb *, struct tcpiphdr *, struct mbuf *,
				proto_tcp_seq, proto_tcp_seq, int));
struct	tcpcb *tcp_newtcpcb P((struct inpcb *));
struct	tcpcb *tcp_drop P((struct tcpcb *, int));
struct	tcpcb *tcp_close P((struct tcpcb *));
void	tcp_notify P((struct inpcb *, int));

/* tcp_timer.c */
void	tcp_fasttimo P((void));
void	tcp_slowtimo P((void));
void	tcp_canceltimers P((struct tcpcb *));
struct	tcpcb *tcp_timers P((struct tcpcb *, int));

/* tcp_usrreq.c */
int	tcp_usrreq P((struct socket *, int, struct mbuf *, struct mbuf *,
				struct mbuf *));
int	tcp_ctloutput P((int, struct socket *, int, int, struct mbuf **));
int	tcp_attach P((struct socket *));
struct	tcpcb *tcp_disconnect P((struct tcpcb *));
struct	tcpcb *tcp_usrclosed P((struct tcpcb *));
int	tcp_send();

/* udp_usrreq.c */
void	udp_init P((void));
void	udp_input P((struct mbuf *, int));
void	udp_ctlinput P((int, struct sockaddr *, caddr_t));
void	udp_notify P((struct inpcb *, int));
struct	mbuf *udp_saveopt P((caddr_t, int, int));
int	udp_output P((struct inpcb *, struct mbuf *, struct mbuf *,
				struct mbuf *));
int	udp_usrreq P((struct socket *, int, struct mbuf *, struct mbuf *,
				struct mbuf *));
int	udp_send();
int	udp_receive();
#ifndef CONST
#define CONST
#endif

/* externs */

/* Any 'no' options used in multiple modules are externed here. */
extern int rfc1122addrchk;
extern int maxttl;
extern int rfc1323;
extern int rfc1323_dflt;

/* 
 * RFC 1323 timestamp clock.
 */
extern u_long timestamp_clock;

extern struct ifqueue pfctlinputq;
extern CONST u_char inetctlerrmap[];
#ifdef IP_MULTICAST
extern struct igmpstat igmpstat;
#endif
extern int in_interfaces;
extern CONST struct   in_addr zeroin_addr;
#ifdef INETPRINTFS
extern int inetprintfs;
extern int inetprintfs_dflt;
#endif
#undef P
