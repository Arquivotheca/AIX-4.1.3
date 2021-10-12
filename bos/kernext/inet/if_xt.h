/* @(#)27	1.6  src/bos/kernext/inet/if_xt.h, sockinc, bos411, 9428A410j 6/19/91 11:02:31 */
/* 
 * COMPONENT_NAME: (SOCKET) Socket services
 * 
 * FUNCTIONS: 
 *
 * ORIGINS: 26 27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1990 
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */
 
#define	X25_SZ_LOGICAL		14		/* size of X.25 logical addr */
#define	X25_SZ_PHYSICAL		12		/* size of X.25 physical addr*/
#define	X25_SZ_MAX		16		/* maximum size of X.25 addr */
#define	X25_STANDARD		0x0401		/* X.25 Standard Service     */
#define	X25_MAX_SESSIONS	256		/* Max. # of X.25 sessions   */
#define	X25_MAX_FACLEN		64		/* Max. length of user facs  */

#define	X25_MTU			1500
#define	IP_X25			0x0cc
#define	IP_TYPE			0x0800


/*
 * X25 software status: there is only one X.25 interface
 *
 * Each interface is referenced by a network interface structure,
 * xt_if, which the routing code uses to locate the interface.
 * This structure contains the output queue for the interface, its address, ...
 */
struct	xt_softc {
	struct	arpcom xt_ac;		/* network common part		*/
	struct  file *netfp;		/* file pointer for driver	*/
	u_short	xt_flags;		/* private flags		*/
	x25_devinfo_t iocinfo;		/* ddi info			*/
};

#define	xt_if	xt_ac.ac_if

/************************************************************************
 * convert x25 packet size facilities code to maximum packet size	*
 *	max packet size is 2 to the max_packet_code			*	
 ************************************************************************/
#define	X25_CODE_TO_MAX_PKT(max_packet_code) 	(1 << (max_packet_code))

/*
 * X.25 DDN address
 */
struct ddn_addr {
	unsigned char 	zzzz[2];		/* must be zero		*/
	unsigned char	ddnhostid[4];		/* DDN Host identifier	*/
	unsigned char	sub_addr[1];		/* Sub-address (optional)*/
};

 
/*
 * X.25 generic address
 */
struct x25_addr {
	union {
	  unsigned char	x25_hostid[X25_SZ_MAX];	/* X.25 Host identifier	*/
	  struct ddn_addr ddn_addr;
	} adrs;
};

#define	x25hostid	adrs.x25_hostid
#define	ddn_hostid	adrs.ddn_addr.ddnhostid

#define	X25_PHYSICAL	0			/* X.25 physical address*/
#define	X25_LOGICAL	1			/* X.25 logical address	*/
#define	X25_LOGICAL_EN	9			/* enable X.25 logical	*/

/*
 * X.25 socket addresses
 */
struct sockaddr_x25 {
	u_char	sa_len;				/* len			*/
	u_char	sa_family;			/* AF_???		*/
	struct x25_addr	sa_x25;			/* X.25 address		*/
	u_short	sa_session_id;			/* X.25 session id	*/
};

struct qx25 {
	int	(*func)();
};

/*
 * X.25 queued incoming call structure
 */
struct qincall {
	struct qx25		qx25;
	struct mbuf		*m;
	int			unit;
	u_short 		call_id;
	u_short 		sess_id;
};

/*
 * X.25 queued outgoing call structure
 */
struct qoutcall {
	struct qx25		qx25;
	int			unit;
	u_long			ip;
	struct arptab		*at;
};

/*
 * X.25 queued clear confirm structure
 */
struct qclrconf {
	struct qx25		qx25;
	int			unit;
	u_short 		sess_id;
};

/*
 * X.25 queued reset confirm structure
 */
struct qrstconf {
	struct qx25		qx25;
	int			unit;
	u_short 		sess_id;
	struct arptab		*at;
};

/*
 * X.25 queued restart PVC struct
 */
struct qrestart {
	struct qx25		qx25;
	int			unit;
	u_short 		sess_id;
	struct arptab		*at;
};
