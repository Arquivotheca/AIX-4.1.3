/* @(#)63	1.5  src/bos/usr/include/netiso/tp_stat.h, sockinc, bos411, 9428A410j 3/5/94 12:41:51 */

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
/* 
 * ARGO TP
 *
 * $Header: tp_stat.h,v 5.4 88/11/18 17:28:38 nhall Exp $
 * $Source: /usr/argo/sys/netiso/RCS/tp_stat.h,v $
 *
 * Here are the data structures in which the global
 * statistics(counters) are gathered.
 */

#ifndef __TP_STAT__
#define __TP_STAT__

struct tp_stat {
	u_long ts_param_ignored;
	u_long ts_unused3;
	u_long ts_bad_csum;

	u_long ts_inv_length;
	u_long ts_inv_pcode;
	u_long ts_inv_dutype;
	u_long ts_negotfailed;
	u_long ts_inv_dref;
	u_long ts_inv_pval;
	u_long ts_inv_sufx;
	u_long ts_inv_aclass;

	u_long ts_xtd_fmt;
	u_long ts_use_txpd;
	u_long ts_csum_off;
	u_long	ts_send_drop;
	u_long	ts_recv_drop;

	u_long ts_xpd_intheway;/* xpd mark caused data flow to stop */
	u_long ts_xpdmark_del;	/* xpd markers thrown away */
	u_long ts_dt_ooo;		/* dt tpdus received out of order */
	u_long ts_dt_niw;		/* dt tpdus received & not in window */
	u_long ts_xpd_niw;		/* xpd tpdus received & not in window */
	u_long ts_xpd_dup;		
	u_long ts_dt_dup;		/* dt tpdus received & are duplicates */

	u_long ts_zfcdt;		/* # times f credit went down to 0 */
	u_long ts_lcdt_reduced; /* 
		# times local cdt reduced on an acknowledgement.
		*/

	u_long	ts_pkt_rcvd; /* from ip */
	u_long	ts_tpdu_rcvd; /* accepted as a TPDU in tp_input */
	u_long	ts_tpdu_sent;
	u_long	ts_unused2;

	u_long	ts_retrans_cr;
	u_long	ts_retrans_cc;
	u_long	ts_retrans_dr;
	u_long	ts_retrans_dt;
	u_long	ts_retrans_xpd;
	u_long	ts_conn_gaveup;

	u_long ts_ER_sent;
	u_long	ts_DT_sent;
	u_long	ts_XPD_sent;
	u_long	ts_AK_sent;
	u_long	ts_XAK_sent;
	u_long	ts_DR_sent;
	u_long	ts_DC_sent;
	u_long	ts_CR_sent;
	u_long	ts_CC_sent;

	u_long ts_ER_rcvd;
	u_long	ts_DT_rcvd;
	u_long	ts_XPD_rcvd;
	u_long	ts_AK_rcvd;
	u_long	ts_XAK_rcvd;
	u_long	ts_DR_rcvd;
	u_long	ts_DC_rcvd;
	u_long	ts_CR_rcvd;
	u_long	ts_CC_rcvd;

	u_long	ts_Eticks;
	u_long	ts_Eexpired;
	u_long	ts_Eset;
	u_long	ts_Ecan_act;
	u_long	ts_Cticks;
	u_long	ts_Cexpired;
	u_long	ts_Cset;
	u_long	ts_Ccan_act;
	u_long	ts_Ccan_inact;

	u_long	ts_concat_rcvd;

	u_long	ts_zdebug; /* zero dref to test timeout on conn estab tp_input.c */
	u_long ts_ydebug; /* throw away pseudo-random pkts tp_input.c */
	u_long ts_unused5;
	u_long ts_unused; /* kludged concat to test separation tp_emit.c */
	u_long ts_vdebug; /* kludge to test input size checking tp_emit.c */
	u_long ts_unused4;
	u_long ts_ldebug; /* faked a renegging of credit */

	u_long ts_mb_small;
	u_long ts_mb_cluster;
	u_long ts_mb_len_distr[17];

	u_long ts_eot_input;
	u_long ts_eot_user;
	u_long	ts_EOT_sent;
	u_long ts_tp0_conn;
	u_long ts_tp4_conn;
	u_long ts_quench;
	u_long ts_rcvdecbit;

#define NRTT_CATEGORIES 4
	/*  The 4 categories are:
	 * 0 --> tp_flags: ~TPF_PEER_ON_SAMENET |  TPF_NL_PDN
	 * 1 --> tp_flags: ~TPF_PEER_ON_SAMENET | ~TPF_NL_PDN
	 * 2 --> tp_flags:  TPF_PEER_ON_SAMENET | ~TPF_NL_PDN
	 * 3 --> tp_flags:  TPF_PEER_ON_SAMENET |  TPF_NL_PDN
	 */
	struct timeval ts_rtt[NRTT_CATEGORIES];
	struct timeval ts_rtv[NRTT_CATEGORIES];

	u_long ts_ackreason[_ACK_NUM_REASONS_];
		/*  ACK_DONT 0 / ACK_STRAT_EACH 0x1 / ACK_STRAT_FULLWIN 0x4
	  	 *	ACK_DUP 0x8 / ACK_EOT 0x10  / ACK_REORDER 0x20
	  	 *	ACK_USRRCV **
	  	 *	ACK_FCC **
		 */
} tp_stat ;

#define IncStat(x) tp_stat./**/x/**/++

#ifdef TP_PERF_MEAS

#define PStat(Tpcb, X) (Tpcb)->tp_p_meas->/**/X/**/
#define IncPStat(Tpcb, X) if((Tpcb)->tp_perf_on) (Tpcb)->tp_p_meas->/**/X/**/++

/* BEWARE OF MACROS like this ^^^ must be sure it's surrounded by {} if
 * it's used in an if-else statement. 
 */


/* for perf measurement stuff: maximum window size it can handle */
#define 	TP_PM_MAX			0xa /* 10 decimal */

struct tp_pmeas {
		/* the first few are distributions as a fn of window size 
		 * only keep enough space for normal format plus 1 slot for
		 * extended format, in case any windows larger than 15 are used
		 */

		/* 
		 * tps_npdusent: for each call to tp_sbsend, we inc the 
		 * element representing the number of pdus sent in this call
		 */
		int		tps_win_lim_by_cdt[TP_PM_MAX+1]; 
		int		tps_win_lim_by_data[TP_PM_MAX+1]; 
		/* 
		 * tps_sendtime: Each call to tp_sbsend() is timed.  For
		 * Each window size, we keep the running average of the time
		 * taken by tp_sbsend() for each window size.
		 */
		struct timeval	tps_sendtime[TP_PM_MAX+1]; 
		/*
		 * n_TMsendack: # times ack sent because timer went off
		 * n_ack_cuz_eot: # times ack sent due to EOTSDU on incoming packet
		 * n_ack_cuz_dup: # times ack sent for receiving a duplicate pkt.
		 * n_ack_cuz_fullwin: # times ack sent for receiving the full window.
		 * n_ack_cuz_doack: # times ack sent for having just reordered data.
		 */
		int		tps_n_TMsendack;
		int		tps_n_ack_cuz_eot;
		int		tps_n_ack_cuz_fullwin;
		int		tps_n_ack_cuz_reorder;
		int		tps_n_ack_cuz_dup;
		int		tps_n_ack_cuz_strat;
		/*
		 * when we send an ack: how much less than the "expected" window
		 * did we actually ack.  For example: if we last sent a credit
		 * of 10, and we're acking now for whatever reason, and have
		 * only received 6 since our last credit advertisement, we'll
		 * keep the difference, 4, in this variable.
		 */
		int		tps_ack_early[TP_PM_MAX+1]; 
		/*
		 * when we ack, for the # pkts we actually acked w/ this ack,
		 * how much cdt are we advertising?
		 * [ size of window acknowledged ] [ cdt we're giving ]
		 */
		int		tps_cdt_acked[TP_PM_MAX+1][TP_PM_MAX+1]; 

		int 	tps_AK_sent;
		int 	tps_XAK_sent;
		int 	tps_DT_sent;
		int 	tps_XPD_sent;
		int 	tps_AK_rcvd;
		int 	tps_XAK_rcvd;
		int 	tps_DT_rcvd;
		int 	tps_XPD_rcvd;

		int		Nb_from_sess;
		int		Nb_to_sess;
		int		Nb_to_ll;
		int		Nb_from_ll;
};

#define  IFPERF(tpcb)  if (tpcb->tp_perf_on && tpcb->tp_p_meas) {
#define  ENDPERF }

#else

#define PStat(tpcb, x)  /* no-op */
#define IncPStat(tpcb, x)  /* no-op */

#ifndef STAR
#define STAR *
#endif	/* STAR */
/* IFPERF and ENDPERF changed due to bug in the preprocessor of AIX
 *
 */

#define IFPERF(tpcb) 	{
#define ENDPERF		}

#endif /* TP_PERF_MEAS */

#endif /* __TP_STAT__ */
