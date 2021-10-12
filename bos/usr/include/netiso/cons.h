/* @(#)45     1.7  src/bos/usr/include/netiso/cons.h, sockinc, bos411, 9428A410j 3/5/94 12:40:56 */
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
 * $Header: cons.h,v 4.4 88/09/09 19:01:28 nhall Exp $
 * $Source: /usr/argo/sys/netiso/RCS/cons.h,v $
 *
 * interface between TP and CONS
 */

#define	CONSOPT_X25CRUD	0x01		/* set x.25 call request user data */

/* cons_pcb.h depends upon this */
struct dte_addr {
	u_char 	dtea_addr[7];
	u_char	dtea_niblen;
};

#ifdef	_KERNEL

#define CONN_OPEN		0x33
#define CONN_CONFIRM	0x30
#define CONN_REFUSE		0x31
#define CONN_CLOSE		0x32

#define	CONS_IS_DGM		0x1
#define	CONS_NOT_DGM	0x0

#ifndef	PRC_NCMDS
#include <sys/protosw.h>
#endif	/* PRC_NCMDS */

#define PRC_CONS_SEND_DONE 2 /* something unused in protosw.h */

#define CONS_IFQMAXLEN 5


#define HIGH_NIBBLE 1
#define LOW_NIBBLE 0

#define HTOW(hw1, hw2, w)	((w) = ((hw1) << 16 ) | (hw2))
#define WTOH(hw1, hw2, w)	{ \
				hw1 = (u_short)((w) >> 16); \
				hw2 = (u_short)((w) & 0xffff); \
				}
#define CONS_MTU	1500

struct	ifx25_func {
	int	(*func)();
		};

/* Struct for the different functions queued */
				/* outgoing call request queued function */
struct qoutcall_fn {
	struct ifx25_func ifx25_func;	/* ifx25_call_req() */
	struct co_pcb *copcb;
};

			/* incoming call is queued to either accept or
			 * reject a call. 
			 */
struct qincall_fn {
	struct ifx25_func ifx25_func;	/* qcons_incoming() */
	struct mbuf		*m;
	int			unit;
	u_short 		call_id;
	u_short 		sess_id;
};

		/* que send a clear confirm for a clear indication */
struct qclrconf_fn {
	struct ifx25_func ifx25_func;	/* ifx25_clear_conf() */
	int unit;
	u_short sess_id;
};

		/* we have to queue the clear req to prevent a re-entry
		 * into the x.25 handler. As the clear req could have been
		 * internally initiated by a TP4 (CONS-TP4-CONS path)  */
struct qclrreq_fn {
	struct ifx25_func ifx25_func;	/* ifx25_clear_req() */
	int unit;
	u_short sess_id;
	u_char reason;
	int loop;
};

		/* In the case of senddata initiated by CIO_ACK_TX_DONE
		 * cons_senddata is queued via qcons_senddata()
		 * also data pending for a CONNECT is sent via 
		 * qcons_senddata()
		 */
struct qsenddata_fn {		
	struct ifx25_func ifx25_func;	/* cons_senddata() */
	struct copcb *copcb;
	struct mbuf *m;
};

#define NCONS 4
#endif	/* _KERNEL */
