/* @(#)54       1.12  src/bos/kernel/sys/POWER/cdli_tokuser.h, sysxtok, bos41J, 9520B_all 5/18/95 11:46:01 */
/*
 *   COMPONENT_NAME: SYSXTOK
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_CDLI_TOKUSER
#define _H_CDLI_TOKUSER

#include <sys/ndd.h>

/*
 *	This file is shared by all Token-Ring device drivers which
 *	have implemented the CDLI interface
 */

/*
 *	miscellaneous definitions
 */

#define CTOK_NADR_LENGTH    (6)     /* bytes in hardware network address */
#define CTOK_MIN_PACKET     (16)    /* minimum packet size accepted	 */
#define CTOK_16M_MAX_PACKET (17800) /* maximum packet size for 16M rate	 */
#define CTOK_4M_MAX_PACKET  (4472)  /* maximum packet size for 4M rate	 */

/*
 *	additional flags for the ndd_flags field in the ndd.h file
 */
#define TOK_ATTENTION_MAC	(NDD_SPECFLAGS) /* rcv attention MAC frames */
#define TOK_BEACON_MAC		(2*NDD_SPECFLAGS) /* rcv beacon MAC frames */
#define TOK_RECEIVE_FUNC	(4*NDD_SPECFLAGS) /* rcv functional address */
#define TOK_RECEIVE_GROUP	(8*NDD_SPECFLAGS) /* rcv group address */
#define TOK_RING_SPEED_4	(16*NDD_SPECFLAGS) /* 4 Mbps ring speed */
#define TOK_RING_SPEED_16	(32*NDD_SPECFLAGS) /* 16 Mbps ring speed */

/*
 * Token-Ring device generic statistics
 */

struct tok_genstats {
	ulong  device_type;	/* flags for interpreting the device	*/
				/* specific statistics extension	*/
	ulong  dev_elapsed_time; /* seconds since last reset of device	*/
	ulong  ndd_flags;	/* ndd_flags field at time of request	*/
	uchar  tok_nadr[CTOK_NADR_LENGTH];  /* physical address in use	*/
	ulong  mcast_xmit;	/* frames transmitted with multicast @	*/
	ulong  bcast_xmit;	/* frames transmitted with broadcast @	*/
	ulong  mcast_recv;	/* frames received with multicast addr	*/
	ulong  bcast_recv;	/* frames received with broadcast addr	*/
	ulong  line_errs;	/* line error count			*/
	ulong  burst_errs;	/* burst error count			*/
	ulong  ac_errs;		/* ACE error count
				   - not supported by TOK_MON, TOK_MPS	*/
	ulong  abort_errs;	/* abort transmit error count
				   - not supported by TOK_MPS		*/
	ulong  int_errs;	/* internal error count 		*/
	ulong  lostframes;	/* lost frame error count		*/
	ulong  rx_congestion;	/* device had no buffer space count	*/
	ulong  framecopies;	/* frame copied error count		*/
	ulong  token_errs;	/* token error count			*/
	ulong  soft_errs;	/* soft error count			*/
	ulong  hard_errs;	/* hard error count			*/
	ulong  signal_loss;	/* signal loss error count		*/
	ulong  tx_beacons;	/* transmit beacon count		*/
	ulong  recoverys;	/* ring recovered count			*/
	ulong  lobewires;	/* lobe wire fault count		*/
	ulong  removes;		/* remove ring station MAC frame count	*/
	ulong  singles;		/* only station on the ring count	*/
	ulong  freq_errs;	/* signal frequency error count
				   - not supported by TOK_MON, TOK_MPS	*/
	ulong  tx_timeouts;	/* transmit timeout count		*/
	ulong  sw_txq_len;	/* current software xmit queue length	*/
	ulong  hw_txq_len;	/* current hardware xmit queue length
				   - not supported by TOK_IBM_ISA	*/
	ulong  reserve1;	/* reserved for future use		*/
	ulong  reserve2;	/* reserved for future use		*/
	ulong  reserve3;	/* reserved for future use		*/
	ulong  reserve4;	/* reserved for future use		*/
};
typedef struct tok_genstats tok_genstats_t;

/*	flags for the device_type field in the tok_genstats structure	*/
#define TOK_MON		0x00000001	/* Token-Ring High-Performance	*/
					/* Adapter			*/
#define TOK_MPS		0x00000002	/* 32 Bit High-Performance	*/
					/* Token-Ring Adapter		*/
#define TOK_IBM_ISA	0x00000003	/* Token-Ring IBM ISA Adapter	*/
#define TOK_SKY		0x00000004	/* Token-Ring IBM PCI Adapter	*/

/* 	general statistics table returned on NDD_GET_STATS command	*/
struct tok_ndd_stats {
	struct ndd_genstats tok_ndd_stats;	/* network general stats */
	struct tok_genstats tok_gen_stats;	/* device general stats  */
};
typedef struct tok_ndd_stats tok_ndd_stats_t;

/*
 * Token-Ring device specific statistics
 */

/*
 * Token-Ring High-Performance Adapter specific statistics
 */
struct tr_mon_stats {
	ulong	ARI_FCI_errors;		/* ARI/FCI error count		*/
	ulong	DMA_bus_errors;		/* DMA bus error count		*/
	ulong	DMA_parity_errors;	/* DMA parity error count	*/
};
typedef struct tr_mon_stats tr_mon_stats_t;

/* 	specific statistics table returned on NDD_GET_ALL_STATS command	*/
struct mon_all_stats {
	struct ndd_genstats tok_ndd_stats;	/* network general stats */
	struct tok_genstats tok_gen_stats;	/* device general stats  */
	struct tr_mon_stats tok_mon_stats;	/* device specific stats */
};
typedef struct mon_all_stats mon_all_stats_t;

/*
 * 32 bit High-Performance Token-Ring Adapter specific statistics
 */
struct tr_mps_stats {
	ulong	recv_overrun;		/* receive overrun count	*/
	ulong	xmit_underrun;		/* transmit underrun count	*/
	ulong   rx_frame_err;           /* receive frame error          */
};
typedef struct tr_mps_stats tr_mps_stats_t;

/*	specific statistics table returned on NDD_GET_ALL_STATS command	*/
struct mps_all_stats {
	struct ndd_genstats tok_ndd_stats;	/* network general stats */
	struct tok_genstats tok_gen_stats;	/* device general stats  */
	struct tr_mps_stats tok_mps_stats;	/* device specific stats */
};
typedef struct mps_all_stats mps_all_stats_t;

/*
 * IBM ISA Token-Ring Adapter specific statistics
 */
struct tr_ibm_isa_stats {
	ulong	read_logs;	/* Number of read log commands issued	*/
};
typedef struct tr_ibm_isa_stats tr_ibm_isa_stats_t;

/*	specific statistics table returned on NDD_GET_ALL_STATS command	*/
struct tr_ibm_isa_all_stats {
	struct ndd_genstats tok_ndd_stats;	/* network general stats */
	struct tok_genstats tok_gen_stats;	/* device general stats  */
	struct tr_ibm_isa_stats tok_spec_stats;	/* device specific stats */
};
typedef struct tr_ibm_isa_all_stats tr_ibm_isa_all_stats_t;

/*
 * PCI Token-Ring Adapter specific statistics
 */
struct tr_sky_stats {
	ulong	recv_overrun;		/* receive overrun count	*/
	ulong	xmit_underrun;		/* transmit underrun count	*/
	ulong   no_txq_resource;        /* no resource for txq          */
	ulong   rx_frame_err;           /* receive frame error          */
};
typedef struct tr_sky_stats tr_sky_stats_t;

/*	specific statistics table returned on NDD_GET_ALL_STATS command	*/
struct sky_all_stats {
	struct ndd_genstats tok_ndd_stats;	/* network general stats */
	struct tok_genstats tok_gen_stats;	/* device general stats  */
	struct tr_sky_stats tok_sky_stats;	/* device specific stats */
};
typedef struct sky_all_stats sky_all_stats_t;

/*
 *	Asynchronous status block codes
 */

#define TOK_ADAP_INIT		(NDD_REASON_CODE)
#define TOK_ADAP_OPEN		(NDD_REASON_CODE + 1)
#define TOK_DMA_FAIL		(NDD_REASON_CODE + 2)
#define TOK_DUP_ADDR		(NDD_REASON_CODE + 3)
#define TOK_PERM_HW_ERR		(NDD_REASON_CODE + 4)
#define TOK_RECOVERY_THRESH	(NDD_REASON_CODE + 5)
#define TOK_BEACONING		(NDD_REASON_CODE + 6)
#define TOK_RING_SPEED		(NDD_REASON_CODE + 7)
#define TOK_RMV_ADAP		(NDD_REASON_CODE + 8)
#define TOK_WIRE_FAULT		(NDD_REASON_CODE + 9)
#define TOK_PARITY_ERR		(NDD_REASON_CODE + 10)

/*
 *	Trace hook numbers
 */

/* Token-Ring High-Performance Adapter trace hook numbers		*/
#define HKWD_CDLI_MON_XMIT	0x26A	/* transmit events		*/
#define HKWD_CDLI_MON_RECV	0x26B	/* receive events		*/
#define HKWD_CDLI_MON_OTHER	0x26C	/* other events	including errors */

/* 32 bit High-Performance Token-Ring Adapter trace hook numbers	*/
#define HKWD_CDLI_MPS_XMIT	0x32D	/* transmit events		*/
#define HKWD_CDLI_MPS_RECV	0x32E	/* receive events		*/
#define HKWD_CDLI_MPS_ERR	0x32F	/* error events			*/
#define HKWD_CDLI_MPS_OTHER	0x338	/* other events			*/

/* IBM ISA Token-Ring Adapter trace hook numbers			*/
#define HKWD_ITOK_ISA_XMIT	0x334	/* transmit events		*/
#define HKWD_ITOK_ISA_RECV	0x335	/* receive events		*/
#define HKWD_ITOK_ISA_OTHER	0x336	/* other events			*/
#define HKWD_ITOK_ISA_ERR	0x337	/* error events			*/

/* PCI Token-Ring Adapter trace hook numbers	                        */
#define HKWD_STOK_SFF_XMIT	0x2A7	/* transmit events		*/
#define HKWD_STOK_SFF_RECV	0x2A8	/* receive events		*/
#define HKWD_STOK_SFF_ERR	0x2A9	/* error events			*/
#define HKWD_STOK_SFF_OTHER	0x2AA	/* other events			*/

#endif /* ! _H_CDLI_TOKUSER */
