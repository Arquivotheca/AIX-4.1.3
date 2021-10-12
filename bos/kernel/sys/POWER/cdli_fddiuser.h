/* @(#)14       1.5  src/bos/kernel/sys/POWER/cdli_fddiuser.h, sysxfddi, bos411, 9428A410j 3/7/94 11:25:03 */
/*
 *   COMPONENT_NAME: SYSXFDDI
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_CDLI_FDDIUSER
#define _H_CDLI_FDDIUSER

#include <sys/ndd.h>

/* -------------------------------------------------------------------- */
/*  Misc. #defines                                                      */
/* -------------------------------------------------------------------- */
#define CFDDI_NADR_LENGTH 	(6)     /* bytes in hardware network address*/
#define CFDDI_HDRLEN		(54)	/* size of max fddi header */
#define CFDDI_MIN_PACKET	(17)    /* minimum packet size accepted */
#define CFDDI_MAX_LLC_PACKET	(4491)  /* max LLC packet size accepted */
#define CFDDI_MAX_SMT_PACKET	(4356)  /* max SMT packet size accepted */
#define CFDDI_MAX_GATHERS 	(3)     /* max areas gathered for kernel wrt*/

/* -------------------------------------------------------------------- */
/* 	Additional flags for the ndd_flags field			*/
/* -------------------------------------------------------------------- */
#define CFDDI_NDD_LLC_DOWN	(0x00100000) /* driver is in LLC_DOWN mode*/
#define CFDDI_NDD_BEACON	(0x00200000) /* 
					      * driver is set to receive beacon
					      * Beacon frames 
					      */
#define CFDDI_NDD_SMT		(0x00400000) /*
					      * driver is set to receive smt
					      * frames
					      */
#define CFDDI_NDD_NSA		(0x00800000) /*
					      * driver is set to receive nsa
					      * frames
					      */
#define CFDDI_NDD_BF		(0x01000000) /*
					      * driver is set to receive bad
					      * frames
					      */
#define CFDDI_NDD_DAC		(0x02000000) /*
					      * driver is configured as a 
					      * dual attach station
					      */

/* -------------------------------------------------------------------- */
/* 	Statistics							*/
/* -------------------------------------------------------------------- */
/*
 * FDDI Specific Statistics 
 */
#define CFDDI_ECM_OUT		(0x0000) /* OUT state for the ECM */
#define CFDDI_ECM_IN		(0x0001) /* IN state for the ECM */
#define CFDDI_ECM_TRC		(0x0002) /* TRC state for the ECM */
#define CFDDI_ECM_LEAVE		(0x0003) /* LEAVE state for the ECM */
#define CFDDI_ECM_PATH_TEST	(0x0004) /* P TEST state for the ECM */
#define CFDDI_ECM_INSERT	(0x0005) /* INSERT state for the ECM */
#define CFDDI_ECM_CHK		(0x0006) /* CHK state for the ECM */
#define CFDDI_ECM_DEINSERT	(0x0007) /* DEINSERT state for the ECM */

#define CFDDI_PCM_OFF	(0x0000) /* OFF state for the PCM */
#define CFDDI_PCM_BRK	(0x0001) /* BRK state for the PCM */
#define CFDDI_PCM_TRC	(0x0002) /* TRC state for the PCM */
#define CFDDI_PCM_CON	(0x0003) /* CON state for the PCM */
#define CFDDI_PCM_NXT	(0x0004) /* NXT state for the PCM */
#define CFDDI_PCM_SIG	(0x0005) /* SIG state for the PCM */
#define CFDDI_PCM_JOIN	(0x0006) /* JOIN state for the PCM */
#define CFDDI_PCM_VRFY	(0x0007) /* VRFY state for the PCM */
#define CFDDI_PCM_ACT	(0x0008) /* ACT state for the PCM */
#define CFDDI_PCM_MAINT	(0x0009) /* MAINT state for the PCM */

#define CFDDI_CFM_ISOLATED 	(0x0000) /* ISOLATED state for the CFM */
#define CFDDI_CFM_LOCAL	 	(0x0001) /* LOCAL state for the CFM */
#define CFDDI_CFM_SEC	 	(0x0002) /* SECONDARY state for the CFM */
#define CFDDI_CFM_PRIM 		(0x0003) /* PRIMARY state for the CFM */
#define CFDDI_CFM_CONCAT 	(0x0004) /* CONCATENATED state for the CFM */
#define CFDDI_CFM_THRU	  	(0x0005) /* THRU state for the CFM */

#define CFDDI_CF_ISOLATED	(0x0000) /* ISOLATED state for the CF */
#define CFDDI_CF_LOCAL_A	(0x0001) /* LOCAL_A state for the CF */
#define CFDDI_CF_LOCAL_B	(0x0002) /* LOCAL_B state for the CF */
#define CFDDI_CF_LOCAL_AB	(0x0003) /* LOCAL_AB state for the CF */
#define CFDDI_CF_LOCAL_S	(0x0004) /* LOCAL_S state for the CF */
#define CFDDI_CF_WRAP_A		(0x0005) /* WRAP_A state for the CF */
#define CFDDI_CF_WRAP_B		(0x0006) /* WRAP_B state for the CF */
#define CFDDI_CF_WRAP_AB	(0x0007) /* WRAP_AB state for the CF */
#define CFDDI_CF_WRAP_S		(0x0008) /* WRAP_S state for the CF */
#define CFDDI_CF_C_WRAP_A	(0x0009) /* C_WRAP_A state for the CF */
#define CFDDI_CF_C_WRAP_B	(0x000A) /* C_WRAP_B state for the CF */
#define CFDDI_CF_C_WRAP_S	(0x000B) /* C_WRAP_S state for the CF */
#define CFDDI_CF_THRU		(0x000C) /* THRU state for the CF */

#define CFDDI_MCFM_ISOLATED	(0x0000) /* ISOLATED state for the MAC CFM */
#define CFDDI_MCFM_LOCAL	(0x0001) /* LOCAL state for the MAC CFM */
#define CFDDI_MCFM_SEC		(0x0002) /* SECONDARY state for the MAC CFM */
#define CFDDI_MCFM_PRIM		(0x0003) /* PRIMARY state for the MAC CFM */

#define CFDDI_RMT_ISOLATED	(0x0000) /* ISOLATED state for the RMT */
#define CFDDI_RMT_NON_OP  	(0x0001) /* NON_OP state for the RMT */
#define CFDDI_RMT_RING_OP  	(0x0002) /* RING_OP state for the RMT */
#define CFDDI_RMT_DETECT  	(0x0003) /* DETECT state for the RMT */
#define CFDDI_RMT_NON_OP_DUP	(0x0004) /* NON_OP_DUP state for the RMT */
#define CFDDI_RMT_RING_OP_DUP	(0x0005) /* RING_OP_DUP state for the RMT */
#define CFDDI_RMT_DIRECTED  	(0x0006) /* DIRECTED state for the RMT */
#define CFDDI_RMT_TRC	  	(0x0007) /* TRC state for the RMT */

struct	fddi_spec_stats
{
	ushort		smt_error_lo;	/* smt error */
	ushort		smt_error_hi;	/* smt error */
	ushort		smt_event_lo;	/* smt event */
	ushort		smt_event_hi;	/* smt event */
	ushort		cpv;		/* connection policy violation */
	ushort		port_event;	/* port event word */
	ushort		setcount_lo;	/* set count used in command protocol */
	ushort		setcount_hi;	/* set count used in command protocol */
	ushort		aci_code;	/* adapter check interrupt id-code */
	ushort 		pframe_cnt;	/* purge count */
	ushort 		ecm_sm;         /* ecm state machine */
	ushort 		pcm_a_sm;       /* port a pcm state machine */
	ushort 		pcm_b_sm;       /* port b pcm state machine */
	ushort 		cfm_a_sm;       /* port a cfm state machine */
	ushort 		cfm_b_sm;       /* port b cfm state machine */
	ushort 		cf_sm;          /* cf state machine */
	ushort		mac_cfm_sm;     /* mac cfm state machine */
	ushort 		rmt_sm;         /* rmt state machine */
	ulong		mcast_tx_ok;	/* number of multicast packets 
					 * transmitted
					 */
	ulong		bcast_tx_ok; 	/* number of broadcast packets
					 * transmitted
					 */
	ulong		mcast_rx_ok;	/* number of multicast packets 
					 * received
					 */
	ulong		bcast_rx_ok; 	/* number of broadcast packets
					 * received
					 */
	
	ulong		ndd_flags;	/* up/down, broadcast, etc. */
	
};
typedef	struct fddi_spec_stats	fddi_spec_stats_t;

/* 
 * fddi_all_stats_t defines the structure used in the NDD_GET_STATS ctl cmd.
 * It contains the ndd generic statistics and the CFDDI specific statistics.
 */

struct fddi_ndd_stats
{
	ndd_genstats_t  	genstats;
	fddi_spec_stats_t	fddistats;
};
typedef struct fddi_ndd_stats fddi_ndd_stats_t;

/* -------------------------------------------------------------------- */
/*  Returned Status Code Defines                                        */
/* -------------------------------------------------------------------- */

#define CFDDI_SMT_EVENT   	(0x0001)
#define CFDDI_SMT_ERROR   	(0x0002)
#define CFDDI_PORT_EVENT   	(0x0003)
#define CFDDI_REMOTE_DISCONNECT	(0x0004)
#define CFDDI_LLC_DISABLE	(0x0005)
#define CFDDI_LLC_ENABLE	(0x0006)
#define CFDDI_REMOTE_SELF_TEST	(0x0007)
#define CFDDI_REMOTE_T_REQ	(0x0008)
#define CFDDI_TX_ERROR	 	(0x0009)
#define CFDDI_RTT		(0x000a)
#define CFDDI_PATH_TEST		(0x000b)
#define CFDDI_PORT_STUCK	(0x000c)
#define CFDDI_BYPASS_STUCK	(0x000d)
#define CFDDI_SELF_TEST 	(0x000e)
#define CFDDI_ROP		(0x000f)
#define CFDDI_NO_ROP		(0x0010)

/* -------------------------------------------------------------------- */
/* 			CFDDI HEADER DEFINITION				*/
/* -------------------------------------------------------------------- */

struct cfddi_hdr
{
	uchar	flag;		/* flag to hold the TRF flags */
	uchar	res[2];		/* 3 reserved bytes */
	uchar	fc;		/* Frame control field */
	uchar	dest[6];	/* Destination address */
	uchar	src[6];		/* Source address */
	uchar	data[31];	/* netid and possible routing information */
				/* Data follows the netid (which is after */
				/* the routing information, if any)  	  */
				/* The definition of the data packet      */
				/* the data field is left to the user.    */
};
typedef struct cfddi_hdr cfddi_hdr_t;

/* -------------------------------------------------------------------- */
/*	CFDDI tx ext flags						*/
/* -------------------------------------------------------------------- */
/* TRF support for the control of the frame processing */
#define CFDDI_TX_NORM_XMIT	(0x00)  /* Normal xmit of the frame by default*/
#define CFDDI_TX_LOOPBACK	(0x02) 	/* re-xmit to host not to media */
#define CFDDI_TX_PROC_ONLY	(0x04) 	/* process the frame without xmit */
#define CFDDI_TX_PROC_XMIT	(0x06) 	/* process the frame and xmit it */

/* -------------------------------------------------------------------- */
/*	CFDDI Trace Hook IDs						*/
/* -------------------------------------------------------------------- */
#define HKWD_CFDDI_SFF_XMIT 	0x30A	/* ID for the transmit path */
#define HKWD_CFDDI_SFF_RECV	0x30B	/* ID for the receive path */
#define HKWD_CFDDI_SFF_OTHER	0x30C	/* ID for everything else */
#endif /* ! _H_CDLI_CFDDIUSER */

