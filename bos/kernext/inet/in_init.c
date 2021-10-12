static char sccsid[] = "@(#)88	1.36.1.17  src/bos/kernext/inet/in_init.c, sysxinet, bos412, 9446B 11/16/94 12:30:54";
/*
 *   COMPONENT_NAME: SYSXINET
 *
 *   FUNCTIONS: config_inet
 *		in_arpintr
 *		in_arpresolve
 *		in_rarpintr
 *		ipstatus
 *		netdmpf
 *		
 *
 *   ORIGINS: 27,89
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <net/net_globals.h>

#include <sys/types.h>
#include <sys/uio.h>		
#include <sys/device.h>
#include <sys/domain.h>	
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_types.h>
#include <sys/devinfo.h>
#include <sys/dump.h>
#include <sys/errno.h>
#include <sys/timer.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/mbuf.h>
#include <sys/time.h>

#include <net/netisr.h>
#include <net/netopt.h>
#include <net/route.h>		/* needed by in_pcb.h */
#include <net/spl.h>
#include <net/if_arp.h>
#include <sys/cdli.h>
#include <sys/ndd.h>
#include <net/nd_lan.h>

#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/in_pcb.h>	/* for tcb and udb (struct inpcb) */
#include <netinet/ip_var.h>	/* for ipq, etc. */
#include <netinet/tcp.h>
#include <netinet/tcp_fsm.h>
#include <netinet/tcp_seq.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcpip.h>
#include <netinet/tcp_var.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp_var.h>
#include <netinet/tcp_debug.h>
#ifdef IP_MULTICAST
#include <netinet/igmp_var.h>
#endif

int extension_loaded = 0;	/* flag indicating extension loaded  */
extern struct mbstat mbstat;
extern struct tcp_debug *tcp_debug;

struct cdt_head *netdmpf();

NETOPTINT(subnetsarelocal);
NETOPTINT(maxttl);
NETOPTINT(ipfragttl);
NETOPTINT(ipsendredirects);
NETOPTINT(ipforwarding);
NETOPTINT(udp_ttl);
NETOPTINT(tcp_ttl);
NETOPTINT(arpt_killc);
NETOPTINT(tcp_sendspace);
NETOPTINT(tcp_recvspace);
NETOPTINT(udp_sendspace);
NETOPTINT(udp_recvspace);
NETOPTINT(rfc1122addrchk);
NETOPTINT(nonlocsrcroute);
NETOPTINT(tcp_keepintvl);
NETOPTINT(tcp_keepidle);
NETOPTINT(icmpaddressmask);
NETOPTINT(tcp_keepinit);
#ifdef IP_MULTICAST	
int ie5_old_multicast_mapping=0;
int ie5_old_multicast_mapping_dflt=0;
NETOPTINT(ie5_old_multicast_mapping); /* patch to one for b-cast */
#endif
#if INETPRINTFS
NETOPTINT(inetprintfs);
#endif
NETOPTINT(bcastping);
NETOPTINT(udpcksum);
NETOPTINT(tcp_mssdflt);
NETOPTINT(rfc1323);
/* NETOPTINT for ipqmaxlen */
struct netopt ipqmaxlen_opt; extern int ipqmaxlen_dflt; 
NETOPTINT(directed_broadcast);

/* Dump table */
/* Addresses for ifnet and route in the netdmp structure */
/* are set by the netdmpf function 			*/
struct {
	struct cdt_head cdt_head;
#ifdef IP_MULTICAST
	struct cdt_entry cdt_entry[10];
#else
	struct cdt_entry cdt_entry[9];
#endif
} netdmp = {
	{ DMP_MAGIC, "netstat", sizeof (netdmp) },
	{
		{ "mbstat", sizeof(struct mbstat), &mbstat, 0 },
		{ "ipstat", sizeof(struct ipstat), &ipstat, 0 },
		{ "tcpstat", sizeof(struct tcpstat), &tcpstat, 0 },
		{ "udpstat", sizeof(struct udpstat), &udpstat, 0 },
		{ "icmpsta", sizeof(struct icmpstat), &icmpstat, 0 },
#ifdef IP_MULTICAST
		{ "igmpsta", sizeof(struct igmpstat), &igmpstat, 0 },
#endif
		{ "rtstat", sizeof(struct rtstat), &rtstat, 0 },
		{ "arptabp", 0, 0, 0 },
		{ "ifnet", sizeof(struct ifnet), 0, 0 }
	}
};

/*
 *
 * config_inet - entry point for netinet kernel extension
 *
 */
config_inet(cmd, uio)
	int cmd;
	struct uio *uio;
{
	struct config_proto config_proto;
	int error;

	switch (cmd) {
	case CFG_INIT:
		/* check if kernel extension already loaded */	
		if (extension_loaded++)
			return(EALREADY);

		if (error=arpinit())
			return(error);
		dmp_add(netdmpf);
		dmp_prinit(AF_INET, arptabp);
		pincode(config_inet);
		tcp_timer_init();

		/* TCP must initialize the PCB head locks BEFORE the 
		 * netinet domain is added since the slow and fast timers
		 * might go off before the protocols are initialized...
	 	 */
		tcp_init();
		NET_MALLOC(tcp_debug, struct tcp_debug *,
		     sizeof(struct tcp_debug) * tcp_ndebug, M_KTABLE, M_WAITOK);

		/* 
		 * Add internet entry to the NIT for backwards 
		 * compatibility (ESCON, PCA, and SOCC IFs).
		 */

		add_input_type(0x0800, NET_KPROC, ipintr, &ipintrq, AF_INET);

		/* Add internet domain */	
		domain_add(&inetdomain);
			
		config_proto.loop = ipintr;
		config_proto.loopq = &ipintrq;
		config_proto.netisr = NETISR_IP;
		config_proto.resolve = in_arpresolve;
		config_proto.ioctl = arpioctl;
		config_proto.whohas = NULL;
		nd_config_proto(AF_INET, &config_proto);

		/* 
		 * RFC 1323 - initialize the timestamp clock to
		 * current time.
		 */
		timestamp_clock=time;

		/* Add network options */
		ADD_NETOPT(subnetsarelocal, "%d");
		ADD_NETOPT(maxttl, "%d");
		ADD_NETOPT(ipfragttl, "%d");
		ADD_NETOPT(ipsendredirects,"%d");
		ADD_NETOPT(ipforwarding, "%d");
		ADD_NETOPT(udp_ttl, "%d");
		ADD_NETOPT(tcp_ttl, "%d");
		ADD_NETOPT(arpt_killc, "%d");
		ADD_NETOPT(tcp_sendspace, "%d");
		ADD_NETOPT(tcp_recvspace, "%d");
                ADD_NETOPT(udp_sendspace, "%d");
                ADD_NETOPT(udp_recvspace, "%d");
		ADD_NETOPT(rfc1122addrchk, "%d");
		ADD_NETOPT(nonlocsrcroute, "%d");
		ADD_NETOPT(tcp_keepintvl, "%d");
		ADD_NETOPT(tcp_keepidle, "%d");
#if	INETPRINTFS
		ADD_NETOPT(inetprintfs, "%d");
#endif
		ADD_NETOPT(bcastping, "%d");
		ADD_NETOPT(udpcksum, "%d");
		ADD_NETOPT(tcp_mssdflt, "%d");
		ADD_NETOPT(icmpaddressmask, "%d");
		ADD_NETOPT(tcp_keepinit, "%d");
#ifdef IP_MULTICAST	
		ADD_NETOPT(ie5_old_multicast_mapping, "%d"); 
#endif
		ADD_NETOPT(rfc1323, "%d");

		/* ADD_NETOPT for ipqmaxlen */
		ipqmaxlen_opt.name = "ipqmaxlen";
		ipqmaxlen_opt.size = sizeof( ipintrq.ifq_maxlen );
		ipqmaxlen_opt.obj = (caddr_t)&ipintrq.ifq_maxlen;
		ipqmaxlen_opt.deflt = (caddr_t)&ipqmaxlen_dflt;
		ipqmaxlen_opt.format = "%d";
		ipqmaxlen_opt.next = NULL;
		add_netoption(&ipqmaxlen_opt);

		ADD_NETOPT(directed_broadcast, "%d");

		break;

	default:
		return(EINVAL);

	}

	return(0);
}


void
in_arpintr(nddp, m, hp)
	struct ndd *nddp;
	struct mbuf *m;
	caddr_t hp;
{
	struct arpcom *ac;

	ac = (struct arpcom *) (m->m_pkthdr.rcvif);

	if (ac->ac_if.if_arpinput)
		(* ac->ac_if.if_arpinput)(ac, m, hp);
	else
		m_freem(m);
	return;
}

in_arpresolve(ac, m, destip, daddr)
	struct arpcom *ac;
	struct mbuf *m;
	struct in_addr *destip;
	caddr_t daddr;
{
	if (ac->ac_if.if_arpres)
		return((* ac->ac_if.if_arpres)(ac, m, destip, daddr));
	else {
		if (m) {
			m_freem(m);
			return(0);
		}
	}
}
#ifdef _SUN

void
in_rarpintr(nddp, m, hp)
	struct ndd *nddp;
	struct mbuf *m;
	caddr_t hp;
{
	struct arpcom *ac;

	ac = (struct arpcom *) (m->m_pkthdr.rcvif);

	if (ac->ac_if.if_arprev)
		(* ac->ac_if.if_arprev)(ac, m);
	else
		m_freem(m);

	return;
}
#endif /* _SUN */

struct cdt_head *netdmpf()
{
	int i, num;
	struct cdt *cdtab;
	struct cdt_entry *cep;

	cdtab = (struct cdt *)&netdmp;
	num = NUM_ENTRIES (cdtab);

	for (i = 0; i < num; i++)
	{
		cep = &(cdtab->cdt_entry[i]);
		if (!strcmp ("ifnet", cep->d_name))
			cep->d_ptr = (char *) ifnet;
		if (!strcmp ("arptabp", cep->d_name)) {
			cep->d_len = arptabsize * sizeof(struct arptab);
			cep->d_ptr = (char *)arptabp;
		}
	}
	return(&netdmp);
}
