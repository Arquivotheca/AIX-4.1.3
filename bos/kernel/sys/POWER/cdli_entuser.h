/* @(#)13  1.13  src/bos/kernel/sys/POWER/cdli_entuser.h, sysxent, bos41B 1/25/95 11:12:06 */
/*
 * COMPONENT_NAME: SYSXENT
 *
 * FUNCTIONS: none.
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_CDLI_ENTUSER
#define _H_CDLI_ENTUSER

#include <sys/ndd.h>

/*
 *  Miscellaneous definitions
 */
#define ENT_NADR_LENGTH		(6)      /* Ethernet address byte length */
#define ENT_MAX_MTU             (1514)   /* max packet data size */
#define ENT_MIN_MTU             (60)     /* min packet data size */


/* 
 *  Additional flags for the ndd_flags field in the ndd.h file
 */
#define ENT_RCV_BAD_FRAME	(NDD_SPECFLAGS)	/* rcv bad frames mode */

/*
 *  Ethernet device generic statistics 
 */

struct ent_genstats {
  ulong device_type;	  /* flags for interpreting the device specific     */
			  /* statistics extension 			    */
  ulong dev_elapsed_time; /* time in seconds since last reset		    */
  ulong ndd_flags;	  /* a copy of the ndd_flags field defined in ndd.h */
  char  ent_nadr[ENT_NADR_LENGTH];	/* Ethernet network address	    */

/* receive statistics */
  ulong mcast_rx_ok;      /* no. of multicast packets received		    */
  ulong bcast_rx_ok;      /* no. of broadcast packets received		    */
  ulong fcs_errs;         /* Frame Check Sequence error count               */
  ulong align_errs;       /* Alignment error count                          */
  ulong overrun;          /* Receive overrun count                          */
  ulong short_frames;     /* Packet too short error count                   */
  ulong long_frames;      /* Packet too long error count                    */
  ulong no_resources;     /* Receive out of resources count                 */
  ulong rx_collisions;	  /* Receive collisions error count 		    */
  ulong rx_drop;   	  /* Packets discarded by adapter		    
				- not supported by ENT_IEN_ISA		    */
  ulong start_rx;   	  /* no. of times that receiver was started	    
				- not supported by ENT_IEN_ISA		    */

/* transmit statistics */
  ulong mcast_tx_ok;      /* no. of multicast packets transmitted	    */
  ulong bcast_tx_ok;      /* no. of broadcast packets transmitted	    */
  ulong carrier_sense;    /* Lost Carrier Sense signal count                */
  ulong underrun;         /* Transmit underrun error count                  */
  ulong cts_lost;         /* Lost Clear-To-Send signal count       	    
				- not supported by ENT_IEN_ISA		    */
  ulong excess_collisions;/* Maximum collisions exceeded error count        */
  ulong late_collisions;  /* Collision after the slot time error count	    
				- not supported by ENT_3COM, ENT_IEN_ISA    */
  ulong tx_timeouts;      /* Transmit timeout count                         */
  ulong sqetest;	  /* SQE test 
				- not supported by ENT_3COM, ENT_IEN_ISA    */
  ulong defer_tx;	  /* Transmit deferred count 
				- not supported by ENT_3COM, ENT_IEN_ISA    */
  ulong s_coll_frames; 	  /* Transmit with single collision count	    */
  ulong m_coll_frames;    /* Transmit with multiple collision count	    */
  ulong sw_txq_len;       /* Current sw transmit queue length		    */
  ulong hw_txq_len;       /* Current hw transmit queue length		    
				- not supported by ENT_IEN_ISA		    */
  ulong restart_count;	  /* times the adapter error recovery performed     */

  ulong reserved1;	  /* reserved for future use			    */
  ulong reserved2;	  /* reserved for future use			    */
  ulong reserved3;	  /* reserved for future use			    */
  ulong reserved4;	  /* reserved for future use			    */
};

typedef struct ent_genstats ent_genstats_t;

/* 
 * flags for the device type field in the ent_genstats.
 */
#define ENT_3COM	0x00000001	/* for the en3com_stats extension */
#define ENT_IENT	0x00000002	/* for the ient_stats   extension */
#define ENT_IEN_ISA	0x00000003	/* for the ien_isa_stats extension */
#define ENT_LCE		0x00000004	/* for the enlce_stats  extension */
#define ENT_KEN_PCI	0x00000005	/* for the kent_stats  extension */

/*
 * Ethernet device all generic statistics (for NDD_GET_STATS)
 */
struct ent_ndd_stats {
	struct  ndd_genstats ent_ndd_genstats;     /* network neneric stats */
	struct  ent_genstats ent_ent_genstats;	/* ethernet generic stats */
};
typedef struct ent_ndd_stats ent_ndd_stats_t;


/*
 *  Ethernet device specific statistics (for NDD_GET_ALL_STATS)
 */

/* High Performance Ethernet adapter (3com) specific statistics */
struct en3com_stats {
  ulong multi_promis_mode;   /* Receive in promiscuous mode due to 
			        extended multicast support		    */
  ulong rv_pool_size;	  /* driver's no. of receive buffers		    */
  ulong tx_pool_size;	  /* driver's no. of transmit buffers (hw queue)    */
  ulong host_rcv_eol;     /* Host side End-of-List bit seen                 */
  ulong adpt_rcv_eol;     /* Adapter/586 End-of-List bit seen               */
  ulong adpt_rcv_pack;    /* adpt rec packets to be uploaded to host        */
  ulong rcv_dma_to;       /* Receive DMA time outs due to lock up           */
  ushort reserved[5];     /* 3com internal use only state variables  - HEX  */
};
typedef struct en3com_stats en3com_stats_t;

struct en3com_all_stats {
	struct  ndd_genstats ent_ndd_stats;     /* network neneric stats */
	struct  ent_genstats ent_gen_stats;	/* ethernet generic stats */
	struct  en3com_stats en3com_stats;	/* device specific stats */
};
typedef struct en3com_all_stats en3com_all_stats_t;


/* Integrated Ethernet adapter specific statistics */
struct ient_stats{
  int coll_freq[16];		/* Frequency of no. of collisions */
};
typedef struct ient_stats ient_stats_t;

struct ient_all_stats{
        struct  ndd_genstats ent_ndd_stats;     /* network neneric stats */
        struct  ent_genstats ent_gen_stats;     /* ethernet generic stats */
        struct  ient_stats ient_stats;      	/* device specific stats */
};
typedef struct ient_all_stats ient_all_stats_t;


/* IBM ISA Ethernet adapter specific statistics */
typedef struct {
  ulong multi_promis_mode;   	/* Receive in promiscuous mode due to
                                   extended multicast support		*/
  int coll_freq[16]; 		/* Frequency of no. of collisions 	*/
} ien_isa_stats_t;

typedef struct {
  	struct  ndd_genstats ent_ndd_stats;     /* network generic stats */
  	struct  ent_genstats ent_gen_stats;     /* ethernet generic stats */
  	ien_isa_stats_t      ien_isa_stats;     /* device specific stats */
} ien_isa_all_stats_t;


/* Low Cost High Performance Ethernet adapter (LCE) specific statistics */
typedef struct {
  int adapter_type;    /* AUI/10baseT or 10base2            */
  int media_selection; /* AUI/10baseT or 10base2            */
  int mace_version;    /* Version number of the MACE chip   */
  int mcnet_version;   /* Version number of the McNet chip  */
  ushort reserved[5];  /* internal use only                 */
} enlce_stats_t;

/*
 * Flags for  for LCE adapter type
 */
#define AUI_BASET 0       /* Adapter with AUI and 10baseT ports   */
#define BASE2_ONLY 1      /* Adapter with 10base2 port only       */

/*
 * Flags for LCE media selection
 */
#define AUI   0           /* Adapter is using the AUI port        */
#define BASET 1           /* Adapter is using 10baseT port        */
#define BASE2 2           /* Adapter is using the 10base2 port    */
#define AUTO  3           /* Adapter is in autosense mode         */

typedef struct {
	struct  ndd_genstats ent_ndd_stats;     /* network neneric stats */
	struct  ent_genstats ent_gen_stats;	/* ethernet generic stats */
	enlce_stats_t        enlce_stats;	/* device specific stats */
} enlce_all_stats_t;


/* IBM PCI Ethernet adapter specific statistics */
struct kent_stats{
  int coll_freq[16];		/* Frequency of no. of collisions */
};
typedef struct kent_stats kent_stats_t;

struct kent_all_stats{
        struct  ndd_genstats ent_ndd_stats;     /* network neneric stats */
        struct  ent_genstats ent_gen_stats;     /* ethernet generic stats */
        struct  kent_stats kent_stats;      	/* device specific stats */
};
typedef struct kent_all_stats kent_all_stats_t;




/*
 * Ethernet incoming bad packet error code
 * These error code is used in the NDD_BAD_PKTS asynchronous status block
 * option[0] as reason of the bad packet.
 */
#define ENT_RCV_CRC_ERR		NDD_REASON_CODE + 1	/* CRC error */
#define ENT_RCV_OVRUN_ERR	NDD_REASON_CODE + 2	/* FIFO overrun */
#define ENT_RCV_ALIGN_ERR	NDD_REASON_CODE + 3	/* Alignment error */
#define ENT_RCV_RSC_ERR		NDD_REASON_CODE + 4	/* No resource error */
#define ENT_RCV_SHORT_ERR	NDD_REASON_CODE + 5	/* Packet too short */
#define ENT_RCV_LONG_ERR	NDD_REASON_CODE + 6	/* Packet too long */
#define ENT_RCV_COLL		NDD_REASON_CODE + 7     /* late collision */

/*
 * Ethernet error code returned in the status blocks and system error logs.
 */
#define ENT_NOBUFS	NDD_REASON_CODE + 8	/* buffer/memory not 
						   available */
#define ENT_DMA_FAIL	NDD_REASON_CODE + 9	/* DMA operation failure */
#define ENT_PARITY_ERR	NDD_REASON_CODE + 10	/* parity error */

/*
 *  Trace hook numbers
 */

/* High Performance Ethernet adapter (3com) trace hook numbers */
#define HKWD_EN3COM_XMIT	0x351		/* transmit events */
#define HKWD_EN3COM_RECV	0x352		/* receive events  */
#define HKWD_EN3COM_OTHER	0x353		/* other events    */
#define HKWD_EN3COM_ERR		0x354		/* error events	   */

/* Integrated Ethernet adapter trace hook numbers */
#define HKWD_IENT_XMIT		0x320      	/* transmit events */
#define HKWD_IENT_RECV		0x321      	/* receive events  */
#define HKWD_IENT_OTHER		0x322      	/* other events    */
#define HKWD_IENT_ERR		0x323      	/* error events    */

/* Low Cost High Performance Ethernet adapter (LCE) trace hook numbers */

#define HKWD_CDLI_LCE_RECV	0x327		/* receive events  */
#define HKWD_CDLI_LCE_OTHER	0x328		/* other events    */
#define HKWD_CDLI_LCE_XMIT	0x27D		/* transmit events */
#define HKWD_CDLI_LCE_ERR	0x27E		/* error events    */

/* IBM ISA 16 bit ethernet adapter (ient) trace hook numbers */

#define HKWD_IEN_ISA_XMIT          0x330           /* transmit events */
#define HKWD_IEN_ISA_RECV          0x331           /* receive events  */
#define HKWD_IEN_ISA_OTHER         0x332           /* other events    */
#define HKWD_IEN_ISA_ERR           0x333           /* error events    */

/* IBM PCI ethernet adapter (kent) trace hook numbers */

#define HKWD_KEN_PCI_XMIT	0x2A4		/* transmit events */
#define HKWD_KEN_PCI_RECV	0x2A5		/* receive events */
#define HKWD_KEN_PCI_OTHER	0x2A6		/* other events */

#endif  /* _H_CDLI_ENTUSER */

