static char sccsid[] = "@(#)16	1.1  src/bos/kernel/db/POWER/dbnet.c, sysdb, bos411, 9428A410j 6/20/94 16:12:25";
/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/debug.h>			/* generalized debug field      */
#include <sys/socket.h>
#include <sys/mbuf.h>
#include <sys/ndd.h>
#include <sys/cdli.h>
#include <sys/socketvar.h>
#include "parse.h"			/* Parser structure.		*/
#include "add_cmd.h"                    /* defines for Copyin() */
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/in_pcb.h>     /* inpcb struct */
#include <netinet/ip_var.h>
#include <netinet/tcp.h>
#include <netinet/tcp_fsm.h>
#include <netinet/tcp_seq.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcpip.h>
#include <netinet/tcp_var.h>    /* tcpcb struct */
#include <netinet/udp.h>
#include <netinet/udp_var.h>
#include "dbnet.h"


char *
getflags(val, flaglist)
	int val;
	struct flagent *flaglist;
{
	static char buf[128];
	struct flagent *f = flaglist;

	buf[0] = '\0';
	while (f->name) {
		if (val & f->bit) {
			if (buf[0]) strcat(buf, "|");
			strcat(buf, f->name);
		}
		f++;
	}
	return(buf);
}

char *
gettype(val, typelist, dflt)
	int val;
	struct typeent *typelist;
	char *dflt;
{
	struct typeent *t = typelist;

	while (t->name) {
		if (val == t->val) {
			return(t->name);
		}
		t++;
	}
	return(dflt);
}


/*
 * NAME: showsock
 *                                                                    
 * FUNCTION:  
 *                                                                    
 * RETURN VALUE:  Must always return 0.
 */  
showsock(ps)
struct parse_out *ps;
{
	struct socket	so_buf;
	struct socket	*so;

	if (ps->token[1].tflags & HEX_OVERFLOW) {
		printf("Usage: socket addr\n");
		return(0);
	}

	if (!Copyin(ps->token[1].hv, &so_buf, sizeof(so_buf))) {
		printf ("%08x paged out of memory.\n", ps->token[1].hv);
		return(0);
	}

	clrdsp();
	so = &so_buf;
        printf("    --------------------- SOCKET INFO -------------------\n");
        printf("    type:0x%04x (%s)    opts:0x%04x  (%s)\n",
               so->so_type, gettype(so->so_type, socktypes, "BOGUS"),
               so->so_options, getflags(so->so_options, sockoptflags));
        printf("    state:0x%04x  (%s)\n",
               so->so_state, getflags(so->so_state, sockstates));
        printf("    linger:0x%04x  pcb:0x%08x  proto:0x%08x\n",
               so->so_linger, so->so_pcb, so->so_proto);
        printf("    q0:0x%08x  q0len:%5d q:0x%08x\n",
               so->so_q0, so->so_q0len, so->so_q);
        printf("    qlen:%5d  qlimit:%5u  head:0x%08x\n",
               so->so_qlen, so->so_qlimit, so->so_head);
        printf("    timeo:%5d  error:%5u  oobmark:%5u  pgid:%5u\n\n",
               so->so_timeo, so->so_error, so->so_oobmark,
               so->so_pgid);
        prsock_sb("snd", &(so->so_snd));
        printf("\n");
        prsock_sb("rcv", &(so->so_rcv));
	printf("\n");
	return(0);
}

prsock_sb( which, sb )
	char *which;		/* say which one you are */
	struct sockbuf *sb;
{
	printf("    %s: cc:%5u  hiwat:%5u  mbcnt:%5u  mbmax:%5u\n", which,
	    	sb->sb_cc, sb->sb_hiwat, sb->sb_mbcnt, sb->sb_mbmax);
	printf("    lowat:%5u  mb:0x%08x  events:0x%4x\n",
	    	sb->sb_lowat, sb->sb_mb, sb->sb_reqevents);
	printf("    iodone:0x%08x  ioargs:0x%08x ",
		sb->sb_iodone,sb->sb_ioarg);
	printf("    flags:0x%04x  (%s)\n",
	       sb->sb_flags, getflags(sb->sb_flags, sbflags));
	return(0);
}

prndd(ps)
struct parse_out *ps;
{
	struct ndd interface, *i;
	int a;
	char physaddr[64];
	struct ndd *nddaddr;

	if (ps->token[1].tflags & HEX_OVERFLOW) {
		printf("Usage: ndd addr\n");
		return(0);
	}
	nddaddr = (struct ndd *) ps->token[1].hv;

	if (!Copyin(nddaddr, &interface, sizeof(interface))) {
		printf ("%08x paged out of memory.\n", nddaddr);
		return(0);
	}
	i = &interface;

	printf("    ---------------------- NDD INFO -----(@0x%08x)----------\n",
	       nddaddr);
	printf("    name: %s\t alias: %s      ndd_next:0x%08x\n",
	       i->ndd_name,  i->ndd_alias ? i->ndd_alias : "none", i->ndd_next);
	printf("    flags:0x%08x \t(%s) \n",
	       i->ndd_flags, getflags(i->ndd_flags, ddflags));
	printf("\n");

	printf("    ndd_open():  0x%08x  ndd_close():0x%08x  ndd_output():0x%08x\n",
	       i->ndd_open, i->ndd_close, i->ndd_output);
	printf("    ndd_ctl():   0x%08x  ndd_stat(): 0x%08x  receive():   0x%08x\n",
	       i->ndd_ctl, i->nd_status, i->nd_receive);

	printf("    ndd_correlator: 0x%08x   ndd_refcnt:%10d \n", 
		i->ndd_correlator, i->ndd_refcnt);
	printf("    ndd_mtu:        %8d     ndd_mintu:    %8d\n",
	       i->ndd_mtu, i->ndd_mintu);
	printf("    ndd_addrlen:    %8d     ndd_hdrlen:   %8d \n",
	       i->ndd_addrlen, i->ndd_hdrlen);
	Copyin(i->ndd_physaddr, physaddr, sizeof(physaddr));
	printf("    ndd_physaddr: ");
	for (a = 0 ; a < i->ndd_addrlen ; a++) 
		printf("%02x", physaddr[a]);
	printf("   ndd_type:    %8d (%s)\n",
	       i->ndd_type, gettype(i->ndd_type, dmxtypes, "unknown"));
	printf("\n");
	printf("    ndd_demuxer:    0x%08x\tndd_nsdemux:     0x%08x \n", 
		i->ndd_demuxer, i->ndd_nsdemux);
	printf("    ndd_specdemux:  0x%08x\tndd_demuxsource: %d\n", 
		i->ndd_specdemux, i->ndd_demuxsource);
	printf("    ndd_demux_lock: 0x%08x\tndd_lock:        0x%08x \n", 
		i->ndd_demux_lock, i->ndd_lock);
	printf("    ndd_trace:      0x%08x\tndd_trace_arg:   0x%08x \n", 
		i->ndd_trace_arg, i->ndd_trace_arg);
	printf("    ndd_specstats:  0x%08x\tndd_speclen:     %d \n", 
		i->ndd_specstats, i->ndd_speclen);
	printf("\n");
	printf("    ndd_ipackets:   %12u    ndd_opackets:     %12u\n",
	       i->ndd_genstats.ndd_ipackets_lsw, i->ndd_genstats.ndd_opackets_lsw);
	printf("    ndd_ierrors:    %12u    ndd_oerrors:      %12u\n",
	       i->ndd_genstats.ndd_ierrors, i->ndd_genstats.ndd_oerrors);
	printf("    ndd_ibytes:     %12u    ndd_obytes:       %12u\n",
	       i->ndd_genstats.ndd_ibytes_lsw, i->ndd_genstats.ndd_obytes_lsw);
	printf("    ndd_recvintr:   %12u    ndd_xmitintr:     %12u\n",
	       i->ndd_genstats.ndd_recvintr_lsw, i->ndd_genstats.ndd_xmitintr_lsw);
	printf("    ndd_ipackets_drop: %9u    ndd_nobufs:       %12u\n",
	       i->ndd_genstats.ndd_ipackets_drop, i->ndd_genstats.ndd_nobufs);
	printf("    ndd_xmitque_max:%12u    ndd_xmitque_ovf:  %12u",
	       i->ndd_genstats.ndd_xmitque_max,i->ndd_genstats.ndd_xmitque_ovf);
	printf("\n");
	return(0);
}

prinpcb(ps)
struct parse_out *ps;
{
	struct inpcb tcbbuf, *inp;
	caddr_t inp_addr;

	if (ps->token[1].tflags & HEX_OVERFLOW) {
		printf("Usage: inpcb addr\n");
		return(0);
	}
	inp_addr = (caddr_t) ps->token[1].hv;

	if (!Copyin(inp_addr, &tcbbuf, sizeof(tcbbuf))) {
		printf ("%08x paged out of memory.\n", inp_addr);
		return(0);
	}
	inp = &tcbbuf;

	printf("---------------------- INPCB  INFO -----------------------\n");
	printf("    next:0x%08x    prev:  0x%08x    head: 0x%08x\n",
	       inp->inp_next, inp->inp_prev, inp->inp_head);
	printf("    ppcb:0x%08x    socket:0x%08x    flags:0x%08x\n\n",
	       inp->inp_ppcb, inp->inp_socket, inp->inp_flags);
	printf("    lport:%5d   laddr:0x%08x \n",
	       inp->inp_lport, inp->inp_laddr);
	printf("    fport:%5d   faddr:0x%08x \n\n",
	       inp->inp_fport, inp->inp_faddr);

	ps->token[1].hv = (u_long) inp->inp_socket;
	showsock(ps);
	return(0);
}

prmbuf(ps)
struct parse_out *ps;
{
	struct mbuf	mbuf;
	struct mbuf	*m;

	if (ps->token[1].tflags & HEX_OVERFLOW) {
		printf("Usage: mbuf addr\n");
		return(0);
	}

	if (!Copyin(ps->token[1].hv, &mbuf, sizeof(mbuf))) {
		printf ("%08x paged out of memory.\n", ps->token[1].hv);
		return(0);
	}

	printf("\n");
	printf("m:      0x%08x    m_next:  0x%08x   m_nextpkt: 0x%08x\n",
	       ps->token[1].hv, mbuf.m_next, mbuf.m_nextpkt);
	printf("m_len:  0x%08x    m_data:  0x%08x\n", mbuf.m_len, mbuf.m_data);
	printf("m_type: %s        m_flags: %s\n", 
		gettype(mbuf.m_type, mbuftypes, "weird"),
		getflags(mbuf.m_flags, mbufflags));
	if (mbuf.m_flags & M_PKTHDR)
		printf("m_pkthdr.len:  %8d    m_pkthdr.rcvif:  0x%08x\n", 
			mbuf.m_pkthdr.len, mbuf.m_pkthdr.rcvif);
	if (mbuf.m_flags & M_EXT) {
		printf("ext_buf  0x%08x ext_free:  0x%08x  ext_size:  %8d\n", 
			mbuf.m_ext.ext_buf, mbuf.m_ext.ext_free, 
			mbuf.m_ext.ext_size);
		printf("ext_arg  0x%08x ext_forw:  0x%08x  ext_back:  0x%08x\n",
			mbuf.m_ext.ext_arg, mbuf.m_forw, mbuf.m_back);
	}
	printf("-----------------------------------------------------------\n");

	debug_display(mbuf.m_data, mbuf.m_len, 1);
	return(0);
}

prtcpcb(ps)
struct parse_out *ps;
{
	struct tcpcb tcpcb, *tp;
	caddr_t tcpcbaddr;
	int i;

	if (ps->token[1].tflags & HEX_OVERFLOW) {
		printf("Usage: tcpcb addr\n");
		return(0);
	}
	tcpcbaddr = (caddr_t) ps->token[1].hv;

	if (!Copyin(tcpcbaddr, &tcpcb, sizeof(tcpcb))) {
		printf ("%08x paged out of memory.\n", tcpcbaddr);
		return(0);
	}
	tp = &tcpcb;
	
	printf("  ------------------------ TCPCB ----------------------\n");
	printf("  seg_next 0x%08x  seg_prev 0x%08x  t_state 0x%02x  (%s)\n",
	       tp->seg_next, tp->seg_prev, tp->t_state,
	       gettype(tp->t_state, tcpstates, ""));
	printf("  timers:");
	for (i=0; i<TCPT_NTIMERS; i++)
		printf("   %s:%d", gettype(i, tcptimers, ""), tp->t_timer[i]);
	printf("\n");
	printf("  t_txtshift %d  t_txtcur %d  t_dupacks %d  t_maxseg %d  t_force %1d\n",
	       tp->t_rxtshift, tp->t_rxtcur, tp->t_dupacks,
	       tp->t_maxseg, tp->t_force);
	printf("  flags:0x%04x  (%s)\n",
	       tp->t_flags, getflags(tp->t_flags, tcpcbflags));
	printf("  t_template 0x%08x  inpcb 0x%08x  snd_cwnd:%05d  snd_ssthresh:%05d\n",
	       tp->t_template, tp->t_inpcb, tp->snd_cwnd, tp->snd_ssthresh);
	printf("  snd_una=%05d  snd_nxt=%05d  snd_up=%05d  snd=wl1=%05d  snd_wl2=%05d  iss=%05d\n",
	       tp->snd_una, tp->snd_nxt, tp->snd_up, tp->snd_wl1, tp->snd_wl2,
	       tp->iss);
	printf("  snd_wnd:%6d  rcv_wnd:%6d\n", tp->snd_wnd, tp->rcv_wnd);
	printf("  t_idle=%05d  t_rtt=%05d t_rtseq=%05d  t_srtt=%05d  t_rttvar=%05d\n",
	       tp->t_idle, tp->t_rtt, tp->t_rtseq, tp->t_srtt, tp->t_rttvar);
	printf("  max_sndwnd=%05d  t_iobc=0x%02x  t_oobflags=0x%02x (%s)\n",
	       tp->max_sndwnd, tp->t_iobc, tp->t_oobflags,
	       gettype(tp->t_oobflags, oobflags, ""));
	return(0);
}
