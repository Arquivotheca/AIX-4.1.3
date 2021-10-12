/* @(#)82	1.8  src/bos/kernel/sys/ndd.h, sysnet, bos411, 9436E411a 8/25/94 13:45:36 */
/*
 *   COMPONENT_NAME: SOCKINC
 *
 *   FUNCTIONS: 
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

#ifndef	_H_NDD
#define	_H_NDD

#include <sys/types.h>
#include <sys/lockl.h>

#define	NDD_MAXNAMELEN	(16)

/*
 * Structure for generic statistics
 */
struct ndd_genstats
{
	u_long	ndd_elapsed_time;	/* time in seconds since last reset */
	u_long	ndd_ipackets_msw;	/* packets received on interface(msw) */
	u_long	ndd_ipackets_lsw;	/* packets received on interface(lsw) */
	u_long	ndd_ibytes_msw;		/* total # of octets received(msw) */
	u_long	ndd_ibytes_lsw;		/* total # of octets received(lsw) */
	u_long	ndd_recvintr_msw;	/* number of receive interrupts(msw) */
	u_long	ndd_recvintr_lsw;	/* number of receive interrupts(lsw) */
	u_long	ndd_ierrors;		/* input errors on interface */
	u_long	ndd_opackets_msw;	/* packets sent on interface(msw) */
	u_long	ndd_opackets_lsw;	/* packets sent on interface(lsw) */
	u_long	ndd_obytes_msw;		/* total number of octets sent(msw) */
	u_long	ndd_obytes_lsw;		/* total number of octets sent(lsw) */
	u_long	ndd_xmitintr_msw;	/* number of transmit interrupts(msw) */
	u_long	ndd_xmitintr_lsw;	/* number of transmit interrupts(lsw) */
	u_long	ndd_oerrors;		/* output errors on interface */
	u_long	ndd_nobufs;		/* no buffers available */
	u_long	ndd_xmitque_max;	/* max transmits ever queued */
	u_long	ndd_xmitque_ovf;	/* number of transmit queue overflows */
	u_long	ndd_ipackets_drop;	/* number of packets not passed up */
	u_long	ndd_ibadpackets;	/* # of bad pkts recv'd from adapter */
	u_long	ndd_opackets_drop;	/* number of packets not transmitted */
	u_long	ndd_xmitque_cur;	/* sum of driver+adapter xmit queues */
	u_long	ndd_ifOutUcastPkts_msw;	/* outbound unicast pkts requested */
	u_long	ndd_ifOutUcastPkts_lsw;	/* on interface (msw and lsw) */
	u_long	ndd_ifOutMcastPkts_msw; /* outbound multicast pkts requested */
	u_long	ndd_ifOutMcastPkts_lsw; /* on interface (msw and lsw) */
	u_long	ndd_ifOutBcastPkts_msw; /* outbound broadcast pkts requested */
	u_long	ndd_ifOutBcastPkts_lsw; /* on interface (msw and lsw) */
	u_long	ndd_stat_reserved[23];	/* reserved */
};
typedef	struct ndd_genstats ndd_genstats_t;

/*
 * Structure defining a network device driver interface.
 */
struct ndd {
	struct	ndd *ndd_next;		/* next ndd in chain */
	u_long	ndd_refcnt;		/* number of allocs outstanding */
	char	*ndd_name;		/* name, e.g. ``en0'' or ``tr0'' */
	char	*ndd_alias;		/* alternate name */
	u_long	ndd_flags;		/* up/down, broadcast, etc. */
	caddr_t	ndd_correlator;		/* correlator for NDD use */
/* procedure handles */
	int	(*ndd_open)();		/* init function */
	int	(*ndd_close)();		/* close function */
	int	(*ndd_output)();	/* output function */
	int	(*ndd_ctl)();		/* control function */
/* user receive and status functions */
	void	(*nd_receive)();	/* demuxer receive function */
	void	(*nd_status)();		/* status notification */
/* driver description */
	u_long	ndd_mtu;		/* maximum transmission unit */
	u_long	ndd_mintu;		/* minimum transmission unit */
	u_long	ndd_type;		/* ethernet, etc (see interface types */
	u_long	ndd_addrlen;		/* media address length */
	u_long	ndd_hdrlen;		/* media header length */
	caddr_t	ndd_physaddr; 		/* medias physical address */
/* demuxer linkage */
	struct	ns_demuxer *ndd_demuxer;/* back pointer to associated demuxer */
	struct  ns_dmx_ctl *ndd_nsdemux;/* ptr to common demux control */
	caddr_t	ndd_specdemux;		/* ptr to demuxer specific control */
	int	ndd_demuxsource;	/* 0 if system dmx, 1 if NDD provided */
	Simple_lock ndd_demux_lock;     /* lock for dmx ctl structs */
/* packet tracing */
	void	(*ndd_trace)();		/* packet trace function */
	caddr_t	ndd_trace_arg;		/* argument to trace function */
	Complex_lock ndd_lock;		/* NDD lock for network services */
	u_long	ndd_reserved[16];	/* reserved */
/* stats */
	caddr_t	ndd_specstats;		/* pointer to device specific stats */
	u_long	ndd_speclen;		/* length of device specific stats */
	struct	ndd_genstats ndd_genstats;	/* generic network stats */
};
typedef	struct ndd	ndd_t;

typedef	struct ndd	* nddp_t;

#define	NDD_LOCK(nddp)		lock_write(&(nddp)->ndd_lock)
#define	NDD_UNLOCK(nddp)	lock_done(&(nddp)->ndd_lock)

/* flag values */
#define	NDD_UP		(0x00000001)	/* NDD is opened */
#define	NDD_BROADCAST	(0x00000002)	/* broadcast address valid */
#define	NDD_DEBUG	(0x00000004)	/* operating in debug mode */
#define	NDD_RUNNING	(0x00000008)	/* NDD is operational */
#define	NDD_SIMPLEX	(0x00000010)	/* can't hear own transmissions */
#define	NDD_DEAD	(0x00000020)	/* device is not operational */
#define	NDD_LIMBO	(0x00000040)	/* in network recovery mode */
#define	NDD_PROMISC	(0x00000080)	/* in promiscuous mode */
#define	NDD_ALTADDRS	(0x00000100)	/* receiving additional addresses */
#define	NDD_MULTICAST	(0x00000200)	/* receiving all multicasts */
#define	NDD_DETACHED	(0x00000400)	/* NDD has been detached */


#define	NDD_SPECFLAGS	(0x00100000)	/* min value of device specific flags */

/* interface types for benefit of parsing media address headers (ndd_type) */
#define NDD_OTHER	0x1		/* none of the following */
#define NDD_1822	0x2		/* old-style arpanet imp */
#define NDD_HDH1822	0x3		/* HDH arpanet imp */
#define NDD_X25DDN	0x4		/* x25 to imp */
#define NDD_X25		0x5		/* PDN X25 interface */
#define	NDD_ETHER	0x6		/* Ethernet I or II */
#define	NDD_ISO88023	0x7		/* Ethernet 802.3 */
#define	NDD_ISO88024	0x8		/* Token Bus */
#define	NDD_ISO88025	0x9		/* Token Ring */
#define	NDD_ISO88026	0xa		/* MAN */
#define	NDD_STARLAN	0xb
#define	NDD_P10		0xc		/* Proteon 10MBit ring */
#define	NDD_P80		0xd		/* Proteon 10MBit ring */
#define NDD_HY		0xe		/* Hyperchannel */
#define NDD_FDDI	0xf
#define NDD_LAPB	0x10
#define NDD_SDLC	0x11
#define NDD_T1		0x12
#define NDD_CEPT	0x13
#define NDD_ISDNBASIC	0x14
#define NDD_ISDNPRIMARY	0x15
#define NDD_PTPSERIAL	0x16
#define	NDD_LOOP	0x18		/* loopback */
#define NDD_EON		0x19		/* ISO over IP */
#define	NDD_XETHER	0x1a		/* obsolete 3MB experimental ethernet */
#define	NDD_NSIP	0x1b		/* XNS over IP */
#define	NDD_SLIP	0x1c		/* IP over generic TTY */

#define	NDD_SCB		0x127		/* SCB device driver */
#define	NDD_FCS		0x128		/* FCS device driver */
#define	NDD_SCSI	0x129		/* SCSI device driver */

/* 
 * The following primitives define common NDD control operations
 */
#define	NDD_GET_STATS		(0x00000001)
#define	NDD_ENABLE_MULTICAST	(0x00000002)
#define	NDD_DISABLE_MULTICAST	(0x00000003)
#define	NDD_PROMISCUOUS_ON	(0x00000004)
#define	NDD_PROMISCUOUS_OFF	(0x00000005)
#define	NDD_ADD_FILTER		(0x00000006)
#define	NDD_DEL_FILTER		(0x00000007)
#define	NDD_MIB_QUERY		(0x00000008)
#define	NDD_MIB_GET		(0x00000009)
#define	NDD_MIB_SET		(0x0000000A)
#define	NDD_ADD_STATUS		(0x0000000B)
#define	NDD_DEL_STATUS		(0x0000000C)
#define	NDD_ENABLE_ADDRESS	(0x0000000D)
#define	NDD_DISABLE_ADDRESS	(0x0000000E)
#define	NDD_MIB_ADDR		(0x0000000F)
#define	NDD_CLEAR_STATS		(0x00000010)

#define	NDD_DUMP_ADDR		(0x00000100)
#define	NDD_PRIORITY_ADDR	(0x00000101)
#define	NDD_GET_ALL_STATS	(0x00000102)

#define	NDD_DEVICE_SPECIFIC	(0x00010000)

/* 
 * Structure returned by getkerninfo KINFO_NDD
 */
struct kinfo_ndd {
	char	ndd_name[NDD_MAXNAMELEN];/* name, e.g. ``en0'' or ``tr0'' */
	char	ndd_alias[NDD_MAXNAMELEN];/* alternate name */
	u_long	ndd_flags;		/* up/down, broadcast, etc. */
	u_long	ndd_mtu;		/* maximum transmission unit */
	u_long	ndd_mintu;		/* minimum transmission unit */
	u_long	ndd_type;		/* ethernet, etc (see interface types */
	u_long	ndd_addrlen;		/* physical address length */
	u_char	ndd_addr[64];		/* physical address */
};
/*
 * Flag values for NDD_MIB_QUERY
 */
#define	MIB_NOT_SUPPORTED	(0x00)	/* MIB variable not supported by NDD */
#define	MIB_READ_ONLY		(0x01)	/* MIB variable is read only */
#define	MIB_READ_WRITE		(0x02)	/* MIB variable can be read or set */
#define	MIB_WRITE_ONLY		(0x03)	/* MIB variable can only be set */

struct ndd_mib_query {
	u_int 		mib_get[1];	/* MAC specific MIB structure */
};
typedef	struct ndd_mib_query ndd_mib_query_t;

struct ndd_mib_get {
	u_int 		mib_get[1];	/* MAC specific MIB structure */
};
typedef	struct ndd_mib_get ndd_mib_get_t;

/*
 * status codes for NDD_MIB_SET operations 
 */
#define	MIB_OK			0	/* operation was successful */
#define	MIB_NOT_SETTABLE	1	/* MIB variable is not settable */
#define	MIB_WRONG_STATE		2	/* variable is settable but not now */
#define	MIB_FAILED		3	/* NDD was unable to set variable */
#define	MIB_BAD_VALUE		4	/* incorrect value for variable */

typedef	u_int	mib_t;


/*
 * The MIB set and addr structures are used to represent a packed dynamic 
 * data structure.  Both structures contain a length field as the first few
 * bytes.  The length indicates the number of structures in the rest of the
 * data space.  Since each sub-structure is dynamic, the sub-structure cannot
 * be accessed by indexing the array pointer.  The array pointer, mib and 
 * mib_addr, in ndd_mib_set and ndd_mib_addr respectively, must be used as 
 * starting points in the data space and pointers should be used to walk 
 * through the structure.
 */
struct ndd_mib {
	mib_t	mib;	    	/* a MIB variable to set */
	u_int	status;	    	/* return status from set operation */
	u_short	mib_len;    	/* length of the MIB variable value */
	u_char 	mib_value[2];   /* value to set, length is mib_len */
};
typedef struct ndd_mib ndd_mib_t;

struct ndd_mib_set {
	u_int		count;		/* number of MIB variables to set */
	ndd_mib_t	mib[1];		/* MIB variable. */
};
typedef	struct ndd_mib_set ndd_mib_set_t;

/*
 * These structures are used to receive/get the Address Table by the
 * NDD_MIB_ADDR ioctl().
 */
#define NDD_MIB_OTHER           1
#define NDD_MIB_INVALID         2
#define NDD_MIB_VOLATILE        3
#define NDD_MIB_NONVOLATILE     4

struct ndd_mib_addr_elem  {
	u_int		status;
	u_short		addresslen;
	u_char		address[2];
};
typedef struct ndd_mib_addr_elem ndd_mib_addr_elem_t;

struct ndd_mib_addr {
	u_int			count;
	ndd_mib_addr_elem_t	mib_addr[1];
};
typedef struct ndd_mib_addr ndd_mib_addr_t;


struct ndd_statblk {
	u_int		code;		/* status block code		*/
	u_int		option[10];	/* additional information 	*/
};
typedef struct ndd_statblk ndd_statblk_t;

/* 
 * Status block codes for ndd_statblk
 */
#define	NDD_HARD_FAIL	(0x00000001)	/* A hardware failure has occurred */
#define	NDD_LIMBO_ENTER	(0x00000002)	/* Entered Network Recovery Mode */
#define	NDD_LIMBO_EXIT	(0x00000004)	/* Exited Network Recovery Mode */
#define	NDD_CONNECTED	(0x00000008)	/* Device open successful */
#define	NDD_STATUS	(0x00000010)	/* Various status and event info */
#define	NDD_BAD_PKTS	(0x00000020)	/* A bad packet was received */

#define	NDD_STAT_BLOCK	(0x00100000)	/* Start device specific codes here */

/* 
 * Reason codes for ndd_statblk
 */
#define	NDD_ADAP_CHECK	(0x00000001)	/* Adapter checkstop condition */
#define	NDD_BUS_ERROR	(0x00000002)	/* Bus error */
#define	NDD_CMD_FAIL	(0x00000003)	/* A device command has failed */
#define	NDD_PIO_FAIL	(0x00000004)	/* A PIO operation has failed */
#define	NDD_UCODE_FAIL	(0x00000005)	/* Failure of device microcode */
#define	NDD_TX_ERROR	(0x00000006)	/* A transmission error has occurred */
#define	NDD_TX_TIMEOUT	(0x00000007)	/* Transmission timeout error */
#define	NDD_RCV_ERROR	(0x00000008)	/* A receive error has occured */
#define	NDD_AUTO_RMV	(0x00000009)	/* Auto-remove error detected */

#define	NDD_REASON_CODE	(0x00000100)	/* Start device specific reasons here */


struct ndd_config {
	int	seq_number;
	caddr_t	dds;
	int	l_vpd;
	caddr_t	p_vpd;
	caddr_t	ucode;
};
typedef struct ndd_config	ndd_config_t;


#ifdef _KERNEL
extern struct ndd *ndd;
#endif /* _KERNEL */

#endif	/* _H_NDD */

