/* @(#)59	1.5  src/bos/usr/include/netiso/tp_meas.h, sockinc, bos411, 9428A410j 3/5/94 12:41:33 */

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
/*	(#)tp_meas.h	7.4 (Berkeley) 9/22/89 */
#ifdef TP_PERF_MEAS
#define tpmeas(a, b, t, c, d, e) \
	Tpmeas((u_int)(a), (u_int)(b), t, (u_int)(c), (u_int)(d), (u_int)(e))

struct tp_Meas {
	int			tpm_tseq;
	u_char		tpm_kind;
	u_short 	tpm_ref;
	u_short		tpm_size;
	u_short		tpm_window;
	u_int		tpm_seq;
	struct timeval	tpm_time;
};

#define TPMEASN 4000
extern int tp_Measn;
extern struct tp_Meas tp_Meas[];

/*
 * the kinds of events for packet tracing are:
 */
#define TPtime_from_session	0x01
#define TPtime_to_session	0x02
#define TPtime_ack_rcvd		0x03 
#define TPtime_ack_sent		0x04
#define TPtime_from_ll		0x05
#define TPtime_to_ll		0x06
#define TPsbsend			0x07 
#define TPtime_open			0x08
#define TPtime_open_X		0x28 /* xtd format */
#define TPtime_close		0x09

#endif /* TP_PERF_MEAS */
