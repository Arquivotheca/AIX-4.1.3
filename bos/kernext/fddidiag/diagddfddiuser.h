/* @(#)72       1.1  src/bos/kernext/fddidiag/diagddfddiuser.h, diagddfddi, bos411, 9428A410j 11/1/93 10:59:39 */
/*
 *   COMPONENT_NAME: DIAGDDFDDI
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1990,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_FDDIUSER
#define _H_FDDIUSER

/* -------------------------------------------------------------------- */
/*  Misc. #defines                                                      */
/* -------------------------------------------------------------------- */
#define FDDI_PREFIX      (FDDI_DRIVER << 16)   /*
                                                * define unique prefix
                                                * used in error code
                                                * definitions
                                                */
#define FDDI_NADR_LENGTH 	(6)     /* bytes in hardware network address*/
#define FDDI_MIN_PACKET		(17)    /* minimum packet size accepted */
#define FDDI_MAX_LLC_PACKET	(4491)  /* max LLC packet size accepted */
#define FDDI_MAX_SMT_PACKET	(4356)  /* max SMT packet size accepted */
#define FDDI_MAX_GATHERS 	(3)     /* max areas gathered for kernel wrt*/
#define FDDI_OPLEVEL		(INTOFFL1)

/* Increase MAX NETIDS for support of odd netids and the smt netid */
#define FDDI_MAX_NETIDS         (0x101)	/* max number of Network IDs */
#define FDDI_SMT_NETID		(0x100) /* The netid value for the SMT user */

#define FDDI_MAX_ADDRS		(12) 	/* max number of addrs setable */

			/* max number of addrs setable */
#define FDDI_TMAX_ADDRS		(FDDI_MAX_ADDRS + 1) 	

#define FDDI_MAX_MINOR		(32)	/* max minor number */
#define FDDI_ADD                (81) 	/* used in the set long address */
					/*  command to identify the command */
					/*  as a set address command */
#define FDDI_DEL                (82) 	/* used like the FDDI_ADD only it */
					/*  identifies a clear address command*/
#define FDDI_MAX_PCHAIN		(16)	/* max chain of frames for fastwrt */

/* -------------------------------------------------------------------- */
/*  FDDI Ioctl Command definitions                                	*/
/* -------------------------------------------------------------------- */

#define FDDI_IOCTL	(DD_FDDI << 8) 	/* DD_FDDI is defined as a 'F' */

#define FDDI_SET_LONG_ADDR 	(FDDI_IOCTL | 0x0001)
#define FDDI_QUERY_ADDR		(FDDI_IOCTL | 0x0002)

#define FDDI_DWNLD_MCODE 	(FDDI_IOCTL | 0x0d01)
#define FDDI_MEM_ACC     	(FDDI_IOCTL | 0x0d02)
#define FDDI_HCR_CMD     	(FDDI_IOCTL | 0x0d03)

/* -------------------------------------------------------------------- */
/*  Set Address ioctl structure                                       	*/
/* -------------------------------------------------------------------- */

struct fddi_set_addr
{
	ulong 	status;			/* Returned status code */
	ushort 	opcode;  		/* Add or delete address */
	uchar 	addr[FDDI_NADR_LENGTH];	/* address */
};
typedef struct fddi_set_addr fddi_set_addr_t;

/* -------------------------------------------------------------------- */
/*  Query Address ioctl structure                                       */
/* -------------------------------------------------------------------- */
struct fddi_query_addr
{
	ulong 		status;		/* Returned status code */
	int		addr_cnt;	/* number of valid addresses */
					/* table of addresses */
	uchar		addrs[FDDI_TMAX_ADDRS][FDDI_NADR_LENGTH]; 
};
typedef struct fddi_query_addr fddi_query_addr_t;


/* -------------------------------------------------------------------- */
/*      Structure for Download Adapter Microcode Ioctl                  */
/* -------------------------------------------------------------------- */

struct fddi_dwnld_mcode
{
	ulong 	status; 	/* Returned status */
	int	l_mcode; 	/* microcode length    */
	caddr_t	p_mcode; 	/* pointer to microcode image */
};
typedef struct fddi_dwnld_mcode fddi_dwnld_t;


/* -------------------------------------------------------------------- */
/*      Structure for Mem/DMA Access ioctl 				*/
/* -------------------------------------------------------------------- */

#define FDDI_WR_MEM_FDDI_RAM		0x10 /* Write Memory FDDI RAM Buffer */
#define FDDI_RD_MEM_FDDI_RAM		0x11 /* Read Memory FDDI RAM Buffer */
#define FDDI_WR_MEM_NP_BUS_DATA		0x30 /* Write Mem NP Bus Data Store */
#define FDDI_RD_MEM_NP_BUS_DATA		0x31 /* Read Memory NP Bus Data Store */
#define FDDI_WR_MEM_NP_BUS_PROGRAM	0x50 /* Write Mem NP Bus Prog Store */
#define FDDI_RD_MEM_NP_BUS_PROGRAM	0x51 /* Read Memory NP Bus Prog Store */
#define FDDI_WR_SHARED_RAM		0x70 /* Write Shared RAM */
#define FDDI_RD_SHARED_RAM		0x71 /* Read Shared RAM */


struct fddi_mem_acc_tag
{
	ulong 	status;          /* returned status code */
	ushort 	opcode;          /* Read or Write Appropiate Data Buffer */
	uint 	ram_offset;      /* RAM Offset */
	uint 	num_transfer;    /* Number of Transfer Buffers */
	uchar	*buffer_1;       /* Data Buffer_1 Pointer */
	uint 	buff_len1;       /* Length of Data Buffer 1 */
	uchar 	*buffer_2;       /* Data Buffer_2 Pointer */
	uint 	buff_len2;       /* Length of Data Buffer 2 */
	uchar 	*buffer_3;       /* Data Buffer_3 Pointer */
	uint 	buff_len3;       /* Length of Data Buffer 3 */
};
typedef struct fddi_mem_acc_tag fddi_mem_acc_t;


/* -------------------------------------------------------------------- */
/*      Structure for Issue HCR Command 				*/
/* -------------------------------------------------------------------- */

#define FDDI_CPB_SIZE       	(20)   	/* CPB size (in words) */

struct fddi_hcr_cmd
{
	ulong	status;			/* returned status */
	ushort	hcr_val;		/* HCR cmd to issue */
	ushort	hsr_val;		/* return code from adapter */
	ushort	l_cpb;			/* length of the cpb */
	ushort	cpb[FDDI_CPB_SIZE];	/* cmd parameter block */
};
typedef struct fddi_hcr_cmd fddi_hcr_cmd_t;


/* -------------------------------------------------------------------- */
/*      Structures for Error Logging					*/
/* -------------------------------------------------------------------- */

/*
 * FDDI Link Statistics
 *	consists of errors, events, and status 
 * state definitions for the values returned in the assorted adapter states 
 */
#define FDDI_ECM_OUT		(0x0000) /* OUT state for the ECM */
#define FDDI_ECM_IN		(0x0001) /* IN state for the ECM */
#define FDDI_ECM_TRC		(0x0002) /* TRC state for the ECM */
#define FDDI_ECM_LEAVE		(0x0003) /* LEAVE state for the ECM */
#define FDDI_ECM_PATH_TEST	(0x0004) /* P TEST state for the ECM */
#define FDDI_ECM_INSERT		(0x0005) /* INSERT state for the ECM */
#define FDDI_ECM_CHK		(0x0006) /* CHK state for the ECM */
#define FDDI_ECM_DEINSERT	(0x0007) /* DEINSERT state for the ECM */

#define FDDI_PCM_OFF	(0x0000) /* OFF state for the PCM */
#define FDDI_PCM_BRK	(0x0001) /* BRK state for the PCM */
#define FDDI_PCM_TRC	(0x0002) /* TRC state for the PCM */
#define FDDI_PCM_CON	(0x0003) /* CON state for the PCM */
#define FDDI_PCM_NXT	(0x0004) /* NXT state for the PCM */
#define FDDI_PCM_SIG	(0x0005) /* SIG state for the PCM */
#define FDDI_PCM_JOIN	(0x0006) /* JOIN state for the PCM */
#define FDDI_PCM_VRFY	(0x0007) /* VRFY state for the PCM */
#define FDDI_PCM_ACT	(0x0008) /* ACT state for the PCM */
#define FDDI_PCM_MAINT	(0x0009) /* MAINT state for the PCM */

#define FDDI_CFM_ISOLATED 	(0x0000) /* ISOLATED state for the CFM */
#define FDDI_CFM_LOCAL	 	(0x0001) /* LOCAL state for the CFM */
#define FDDI_CFM_SEC	 	(0x0002) /* SECONDARY state for the CFM */
#define FDDI_CFM_PRIM 		(0x0003) /* PRIMARY state for the CFM */
#define FDDI_CFM_CONCAT 	(0x0004) /* CONCATENATED state for the CFM */
#define FDDI_CFM_THRU	  	(0x0005) /* THRU state for the CFM */

#define FDDI_CF_ISOLATED	(0x0000) /* ISOLATED state for the CF */
#define FDDI_CF_LOCAL_A		(0x0001) /* LOCAL_A state for the CF */
#define FDDI_CF_LOCAL_B		(0x0002) /* LOCAL_B state for the CF */
#define FDDI_CF_LOCAL_AB	(0x0003) /* LOCAL_AB state for the CF */
#define FDDI_CF_LOCAL_S		(0x0004) /* LOCAL_S state for the CF */
#define FDDI_CF_WRAP_A		(0x0005) /* WRAP_A state for the CF */
#define FDDI_CF_WRAP_B		(0x0006) /* WRAP_B state for the CF */
#define FDDI_CF_WRAP_AB		(0x0007) /* WRAP_AB state for the CF */
#define FDDI_CF_WRAP_S		(0x0008) /* WRAP_S state for the CF */
#define FDDI_CF_C_WRAP_A	(0x0009) /* C_WRAP_A state for the CF */
#define FDDI_CF_C_WRAP_B	(0x000A) /* C_WRAP_B state for the CF */
#define FDDI_CF_C_WRAP_S	(0x000B) /* C_WRAP_S state for the CF */
#define FDDI_CF_THRU		(0x000C) /* THRU state for the CF */

#define FDDI_MCFM_ISOLATED	(0x0000) /* ISOLATED state for the MAC CFM */
#define FDDI_MCFM_LOCAL		(0x0001) /* LOCAL state for the MAC CFM */
#define FDDI_MCFM_SEC		(0x0002) /* SECONDARY state for the MAC CFM */
#define FDDI_MCFM_PRIM		(0x0003) /* PRIMARY state for the MAC CFM */

#define FDDI_RMT_ISOLATED	(0x0000) /* ISOLATED state for the RMT */
#define FDDI_RMT_NON_OP	  	(0x0001) /* NON_OP state for the RMT */
#define FDDI_RMT_RING_OP  	(0x0002) /* RING_OP state for the RMT */
#define FDDI_RMT_DETECT	  	(0x0003) /* DETECT state for the RMT */
#define FDDI_RMT_NON_OP_DUP	(0x0004) /* NON_OP_DUP state for the RMT */
#define FDDI_RMT_RING_OP_DUP	(0x0005) /* RING_OP_DUP state for the RMT */
#define FDDI_RMT_DIRECTED  	(0x0006) /* DIRECTED state for the RMT */
#define FDDI_RMT_TRC	  	(0x0007) /* TRC state for the RMT */

struct	fddi_links
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
	ushort		pframe_cnt;	/* purge frame counter */
	ushort 		ecm_sm;         /* ecm state machine */
	ushort 		pcm_a_sm;       /* port a pcm state machine */
	ushort 		pcm_b_sm;       /* port b pcm state machine */
	ushort 		cfm_a_sm;       /* port a cfm state machine */
	ushort 		cfm_b_sm;       /* port b cfm state machine */
	ushort 		cf_sm;          /* cf state machine */
	ushort		mac_cfm_sm;     /* mac cfm state machine */
	ushort 		rmt_sm;         /* rmt state machine */
	ushort 		sba_alloc_lo;   /* sba allocation low */
	ushort 		sba_alloc_hi;   /* sba allocation hi */
	ushort 		tneg_lo;        /* t_neg word low */
	ushort 		tneg_hi;        /* t_new word hi */
	ushort 		payload_lo;     /* sba payload desired word low */
	ushort 		payload_hi;     /* sba payload desired word low */
	ushort 		overhead_lo;    /* sba overhead desired low */
	ushort 		overhead_hi;    /* sba overhead desired hi */
	ushort 		res1[7];       	/* reserved for future use */
	ushort		ucode_ver;	/* microcode version level */
	ushort		res2;		/* reserved for future use */
	ushort		res3;		/* reserved for future use */

};
typedef	struct fddi_links	fddi_links_t;

struct fddi_elog_data
{
	struct err_rec0	errhead;
	char		file[32];	/* file name of error */
	uchar		pos[8];		/* current POS register settings */
	uchar		src_addr[FDDI_NADR_LENGTH];
	ushort		attach_class;	/* Attachment Class */
	uint		mcerr;		/* Last MC error */
	uint		iox;		/* Last PIO exception code */

	cio_stat_blk_t	nrm;		/* Network Recovery Mode Entry 
					 * or fatal error status block
					 */

	fddi_links_t	ls;		/* Last snapshot of link statistics */
	ushort		stestrc[11];	/* adapter self tests results */


					/* Sense Data */
	ushort		status1;
	uint		status2;
	uint		status3;
	uint		state;		
	uchar		carryover;
	uchar		piowall;
};
typedef struct fddi_elog_data fddi_elog_t;

/* -------------------------------------------------------------------- */
/*      Structures for CIO_QUERY statistics ioctl                       */
/* -------------------------------------------------------------------- */

/* device-specific statistics for CIO_QUERY ioctl */

struct fddi_stats
{
	ulong stat_que_ovflw;	/* status lost due to full status que */
	ulong rcv_que_ovflw; 	/* rcv packet lost due to full recv que */
	ulong tx_que_ovflw; 	/* tx calls which dropped frames due to space */
	ulong rcv_no_mbuf; 	/* no mbuf available for receive */
	ulong rcv_cmd_cmplt; 	/* no of rcv cmd cmplt interrupts */
	ulong rcv_intr_cnt; 	/* number of receive interrupts */
	ulong tx_intr_cnt; 	/* number of transmit interrupts */
	ulong pkt_rej_cnt;	/* Packets Rejected No NetID */
	ulong ovflw_pkt_cnt; 	/* Overflow Packets Received */
	ulong adap_err_cnt;	/* adapter errors detected */
};
typedef struct fddi_stats fddi_stats_t;

struct fddi_query_stats
{
       cio_stats_t 	cc;		/* Communication I/O statistics */
       fddi_stats_t 	ds;		/* FDDI specific statistics */
       fddi_links_t	ls;		/* Link Statistics */
};
typedef struct fddi_query_stats fddi_query_stats_t;

/* -------------------------------------------------------------------- */
/*  Returned Status Code Defines                                        */
/*  OR in the FDDI_PREFIX to generate a unique status code. 		*/
/* -------------------------------------------------------------------- */

#define FDDI_ADAP_CHECK 	(FDDI_PREFIX | 0x0001)
#define FDDI_MC_ERROR	  	(FDDI_PREFIX | 0x0002)
#define FDDI_CMD_FAIL 	  	(FDDI_PREFIX | 0x0003)
#define FDDI_RING_STATUS   	(FDDI_PREFIX | 0x0004)
#define FDDI_SMT_EVENT   	(FDDI_PREFIX | 0x0005)
#define FDDI_SMT_ERROR   	(FDDI_PREFIX | 0x0006)
#define FDDI_PORT_EVENT   	(FDDI_PREFIX | 0x0007)
#define FDDI_REMOTE_DISCONNECT 	(FDDI_PREFIX | 0x0008)
#define FDDI_LLC_DISABLE	(FDDI_PREFIX | 0x0009)
#define FDDI_LLC_ENABLE		(FDDI_PREFIX | 0x000a)
#define FDDI_REMOTE_SELF_TEST	(FDDI_PREFIX | 0x000b)
#define FDDI_REMOTE_T_REQ	(FDDI_PREFIX | 0x000c)
#define FDDI_PIO_FAIL	 	(FDDI_PREFIX | 0x000d)
#define FDDI_TX_ERROR	 	(FDDI_PREFIX | 0x000f)
#define FDDI_RCV_ERROR 		(FDDI_PREFIX | 0x0010)
#define FDDI_PHYS_INSERT 	(FDDI_PREFIX | 0x0011)
#define FDDI_ADAP_INIT_FAIL 	(FDDI_PREFIX | 0x0012)
#define FDDI_NO_ADDR		(FDDI_PREFIX | 0x0013)
#define FDDI_NOT_DIAG_MODE	(FDDI_PREFIX | 0x0014)
#define FDDI_ADDR_ADDED		(FDDI_PREFIX | 0x0015)
#define FDDI_RTT		(FDDI_PREFIX | 0x0016)
#define FDDI_PATH_TEST		(FDDI_PREFIX | 0x0017)
#define FDDI_MAC_DISCONNECT	(FDDI_PREFIX | 0x0018)
#define FDDI_DIRECT_BCON	(FDDI_PREFIX | 0x0019)
#define FDDI_VER_MISMATCH	(FDDI_PREFIX | 0x001a)
#define FDDI_ADDR_RMVD		(FDDI_PREFIX | 0x001b)
#define FDDI_MAC_FRAME_ERR	(FDDI_PREFIX | 0x001c)
#define FDDI_PORT_STUCK		(FDDI_PREFIX | 0x001d)
#define FDDI_PORT_DISABLED	(FDDI_PREFIX | 0x001e)
#define FDDI_PORT_STOP		(FDDI_PREFIX | 0x001f)
#define FDDI_BYPASS_STUCK	(FDDI_PREFIX | 0x0020)
#define FDDI_TRACE_MAX		(FDDI_PREFIX | 0x0021)
#define FDDI_SELF_TEST 		(FDDI_PREFIX | 0x0022)
#define FDDI_STARTED 		(FDDI_PREFIX | 0x0023)
#define FDDI_ROP		(FDDI_PREFIX | 0x0024)
#define FDDI_NO_ROP		(FDDI_PREFIX | 0x0025)

/* -------------------------------------------------------------------- */
/* 			FDDI HEADER DEFINITION				*/
/* -------------------------------------------------------------------- */

struct fddi_hdr
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
typedef struct fddi_hdr fddi_hdr_t;

/* -------------------------------------------------------------------- */
/*	FDDI tx ext flags						*/
/* -------------------------------------------------------------------- */
/* TRF support for the control of the frame processing */
#define FDDI_TX_NORM_XMIT	(0x00)  /* Normal xmit of the frame by default*/
#define FDDI_TX_LOOPBACK	(0x02) 	/* re-xmit to host not to media */
#define FDDI_TX_PROC_ONLY	(0x04) 	/* process the frame without xmit */
#define FDDI_TX_PROC_XMIT	(0x06) 	/* process the frame and xmit it */
#endif /* ! _H_FDDIUSER */
