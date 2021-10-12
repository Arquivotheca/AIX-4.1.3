/* @(#)80	1.4  src/bos/kernel/sys/cdli.h, sysnet, bos411, 9428A410j 4/14/94 08:38:45 */
/*
 *   COMPONENT_NAME: SOCKINC
 *
 *   FUNCTIONS: 
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _SYS_CDLI_H
#define _SYS_CDLI_H

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <net/if.h>

struct ns_user {
	void			(*isr)();/* protocol input function	*/
	caddr_t			isr_data;/* arg to isr			*/
        struct ifqueue 		*protoq; /* input queue, may be NULL	*/
	u_long			pkt_format; /* specifies hdr presentation */
	u_long			netisr;	 /* isr number for schednetisr	*/
	struct ifnet		*ifp;    /* ifnet ptr for socket users	*/
};
typedef struct ns_user ns_user_t;

/* values for pkt_format */
#define	NS_PROTO		0x0001
#define NS_PROTO_SNAP		0x0002
#define	NS_INCLUDE_LLC		0x0004
#define	NS_INCLUDE_MAC		0x0008
#define	NS_HANDLE_NON_UI	0x0100
#define	NS_HANDLE_HEADERS	0x0200	/* Used by DLPI */

struct nd_dmxstats {
	u_long		nd_nofilter;	/* packets dropped due to no user */
	u_long		nd_nobufs;	/* packets dropped due to no buffers */
	u_long		nd_bcast;	/* # of broadcast packets received */
	u_long		nd_mcast;	/* # of multicast packets received */
};
typedef struct nd_dmxstats nd_dmxstats_t;

struct ns_demuxer {
	struct ns_demuxer *nd_next;/* link em together			   */
	u_short	nd_inuse;	   /* any users?			   */
	u_short	nd_use_nsdmx;	   /* boolean- true => common dmx services */
	int	(*nd_add_filter)();/* add func for receive filter (ie sap) */
	int	(*nd_del_filter)();/* delete func for removing filter      */
	int	(*nd_add_status)();/* add func for receive filter (ie sap) */
	int	(*nd_del_status)();/* delete func for removing filter	   */
	void	(*nd_receive)();   /* parser/demuxer for input packets     */
	void	(*nd_status)();	   /* asynchronous status handler          */
	void	(*nd_response)();  /* XID and TEST response function       */
	int	(*nd_address_resolve)(); /* mac address resolver output	   */
	void	(*nd_address_input)();   /* mac address resolver input     */
	struct nd_dmxstats nd_dmxstats;	/* common demuxer statistics       */
	u_int	nd_speclen;	   /* length of demuxer specific stats     */
	caddr_t	nd_specstats;	   /* ptr to demuxer spcecific stats       */
	u_long	nd_type;	   /* NDD type of this demuxer		   */
};
typedef struct ns_demuxer ns_demuxer_t;

struct ns_statuser {
	void		(*isr)();	/* status input function	*/
	caddr_t		isr_data;	/* arg to isr			*/
};
typedef struct ns_statuser ns_statuser_t;


#define	NS_8022_LLC_DSAP	(0x00000001)	/* filter on DSAP only 	*/
#define	NS_8022_LLC_DSAP_SNAP	(0x00000002)	/* filter on DSAP & SNAP*/
#define	NS_TAP			(0x00000003)	/* network tap		*/
#define	NS_ETHERTYPE		(0x00000004)	/* ethertype only 	*/
#define NS_LAST_FILTER		(0x00000010)	/* < this are reserved  */

struct ns_8022 {
	u_int		filtertype;
	u_char		dsap;		/* DSAP				*/
	u_char		orgcode[3];	/* SNAP organization code	*/
	u_short		ethertype;	/* SNAP ethertype		*/
};
typedef struct ns_8022 ns_8022_t;

#define	NS_STATUS_MASK		(0x00000001)	/* status mask filtering*/

struct ns_com_status {
	u_int		filtertype;	/* type of filter		*/
	u_int 		mask;		/* status code mask value 	*/
	u_int		sid;		/* returned user status id	*/
};
typedef struct ns_com_status ns_com_status_t;


#define ND_CONFIG_VERSION_1	0x01

typedef struct nd_config {
	int	version;
	int	errcode;
	int	ndd_type;	/* type of demuxer. see sys/ndd.h */
} nd_config_t;

#ifdef _KERNEL

/*
 * Internal ioctl defines/structs for if ioctls to add delete CDLI filters...
 */
struct if_filter {
	ns_user_t 	user;
	ns_8022_t 	filter;
};

#ifndef _H_IOCTL
#include <sys/ioctl.h>
#endif

#define IFIOCTL_ADD_FILTER	_IO('I', 1)	/* Add NDD filters */
#define IFIOCTL_DEL_FILTER	_IO('I', 2)	/* Delete NDD filters */

#endif /* _KERNEL */

#endif /* _SYS_CDLI_H */
