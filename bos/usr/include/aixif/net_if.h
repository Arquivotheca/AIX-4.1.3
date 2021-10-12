/* @(#)70	1.9  src/bos/usr/include/aixif/net_if.h, sockinc, bos411, 9428A410j 6/14/94 17:04:53 */
/*
 *   COMPONENT_NAME: SYSXAIXIF
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _H_NET_IF
#define _H_NET_IF

/*******************************************************************************
* net_if.h -	definitions pertaining to the the network drivers	       *
*******************************************************************************/

struct en_ddi {
	int			reserved;
};

struct ie3_ddi {
	int			reserved;
};

struct ie5_ddi {
	int			reserved;
};

struct x25_ddi {
	int			reserved;
};

struct sl_ddi {
	short			if_connect;	/* auto-disc flag for SLIP  */
};

/*******************************************************************************
*	Device request structure - used to pass configuration information      *
*				   to interface drivers			       *
*******************************************************************************/
struct device_req {
	char			dev_name[IFNAMSIZ];	/* device name        */
	dev_t			devno;		/* device number of interface */
};


#define	MAX_NETIDS	16
/***************************************************************************
 *	netid list used during driver attach
 **************************************************************************/
struct netid_list {
	u_long		attach_snooze;	/* thing to sleep on		*/
	u_short		start_complete;	/* true if all starts worked	*/
	u_short		complete_count;	/* # of start completes so far	*/
	u_short		id_count;	/* number of netids to start	*/
	u_short		id_length;	/* length of netids		*/
	u_short		id[MAX_NETIDS];	/* list of netids to start	*/
	Simple_lock	slock;		/* locking for starts   	*/
};


/* 
 * queued write paramaters 
 */
struct qwrite {
	struct file 	*netfp;
	struct ifnet	*ifp;
	int		length;
	struct mbuf	*ext;
};

/* 
 * queued ioctl paramaters 
 */
struct qioctl {
	struct file 	*netfp;
	int		cmd;
	struct mbuf	*arg;
	struct mbuf	*ext;
};

/* 
 * header for packet tracing
 */
struct o_packet_trace_header {
	u_short	type;			/* packet type */
	u_char	hlen;			/* length of packet header */
	u_char	unit;			/* unit number of interface */
	char	ifname[IFNAMSIZ];	/* name of interface */
	u_char	iftype;			/* interface type */
	u_char	xmit_pkt;		/* xmit or receive packet? */
};

struct packet_trace_header {
	u_short	type;			/* packet type */
	u_char	hlen;			/* length of packet header */
	u_char	unit;			/* unit number of interface */
	char	ifname[IFNAMSIZ];	/* name of interface */
	u_char	iftype;			/* interface type */
	u_char	xmit_pkt;		/* xmit or receive packet? */
	u_short	rdrops;			/* number dropped since last success */
	struct timestruc_t ts;		/* time packet recveived/transmitted */
};

#endif /* _H_NET_IF */
