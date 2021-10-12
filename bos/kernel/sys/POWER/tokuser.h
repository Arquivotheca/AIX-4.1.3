/* @(#)40	1.22.1.2  src/bos/kernel/sys/POWER/tokuser.h, diagddtok, bos411, 9428A410j 6/4/93 08:50:05 */
/*
 * COMPONENT_NAME: (SYSXTOK) - Token-Ring device handler
 *
 * FUNCTIONS: tokuser.h
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_TOKUSER
#define _H_TOKUSER
#include <sys/intr.h>

/* -------------------------------------------------------------------- */
/*  Misc. #defines                                                      */
/* -------------------------------------------------------------------- */
#define TOK_PREFIX      (TOK_DRIVER << 16)     /*
                                                * define unique prefix
                                                * used in error code
                                                * definitions
                                                */
#define TOK_MAX_ADAPTERS   (8)     /* maximum adapters in system */
#define TOK_MAX_OPENS      (6)     /* Max number of opens */
#define TOK_NADR_LENGTH    (6)     /* bytes in hardware network address */
#define TOK_MIN_PACKET     (16)    /* minimum packet size accepted */
#define TOK_16M_MAX_PACKET (17800) /* maximum packet size for 16M rate. */
#define TOK_4M_MAX_PACKET  (4472)  /* maximum packet size for 4M rate. */
#define TOK_MAX_GATHERS    (6)     /* maximum areas gathered for kernel wrt */
#define TOK_MAX_NETIDS         (257)
#define TOK_MAC_FRAME_NETID    (TOK_MAX_NETIDS - 1)
#define TOK_READ               (121)
#define TOK_WRITE              (122)
#define TOK_4M                 (0x0)
#define TOK_16M                (0x1)
#define TOK_ADD                (0x81)
#define TOK_DEL                (0x82)
#define TOK_OPLEVEL	       (INTOFFL2) /* tokfastwrt operational level */

/* -------------------------------------------------------------------- */
/*  Token-Ring Ioctl Command definitions                                */
/* -------------------------------------------------------------------- */

#define TOK_GRP_ADDR           (CIO_IOCTL |    0x0103)
#define TOK_FUNC_ADDR          (CIO_IOCTL |    0x0104)
#define TOK_QVPD               (CIO_IOCTL |    0x0105)
#define TOK_ACCESS_POS         (CIO_IOCTL |    0x0107)
#define TOK_SET_ADAP_IPARMS    (CIO_IOCTL |    0x0108)
#define TOK_SET_OPEN_PARMS     (CIO_IOCTL |    0x0109)
#define TOK_RING_INFO          (CIO_IOCTL |    0x0113)
#define TOK_DOWNLOAD_UCODE     (CIO_IOCTL |    0x0114)

/* -------------------------------------------------------------------- */
/*      Structures for CIO_QUERY statistics ioctl                       */
/* -------------------------------------------------------------------- */
typedef struct ADAP_ERROR_LOG               /* Error Log Data Area */
{
    unsigned char         line_err_count;
    unsigned char         internal_err_count;
    unsigned char         burst_err_count;
    unsigned char         ari_fci_err_count;
    unsigned char         abort_del_err_count;  /*  Abort Delimeter*/
    unsigned char         res1;                   /*  Reserved */
    unsigned char         lost_frame_err_count;
    unsigned char         rec_cong_err_count;   /* Rec. Congestion */
    unsigned char         frame_cpy_err_count;
    unsigned char         res2;                   /* Reserved */
    unsigned char         token_err_count;
    unsigned char         res3;                   /* Reserved */
    unsigned char         dma_bus_err_count;
    unsigned char         dma_parity_err_count;
} tok_adap_error_log_t;


/* device-specific statistics for CIO_QUERY ioctl */
typedef struct TOK_STATS
{
   unsigned long intr_lost;  /* interrupts lost due to full offlevel que */
   unsigned long wdt_lost;   /* wdt intr lost due to full offlevel que */
   unsigned long timo_lost;  /* timeout intr lost due to full offlevel que */
   unsigned long sta_que_overflow;  /* status lost due to full status que */
   unsigned long rec_que_overflow;  /* rcv packet lost due to full recv que */
   unsigned long rec_no_mbuf;       /* no mbuf available for receive */
   unsigned long rec_no_mbuf_ext; /* no mbuf extension available for rcv data */
   unsigned long recv_intr_cnt;     /* number of receive interrupts */
   unsigned long xmit_intr_cnt;     /* number of transmit interrupts */
   unsigned long ctr_pkt_rej_cnt;/* Packets Rejected No NetID */
   unsigned long pkt_acc_cnt;    /* Packets Accepted Valid NetID */
   unsigned long rcv_byt_cnt;    /* Bytes Received Ctr. */
   unsigned long trx_byt_cnt;    /* Bytes Transmitted Ctr. */
   unsigned long pkt_trx_cnt;    /* Packets Transmit Counter */
   unsigned long ovflo_pkt_cnt;  /* Overflow Packets Received */
   unsigned long tx_err_cnt;     /* Packets Transmitted and the adapter
                                  *  detected an error during transmission
                                  */
   tok_adap_error_log_t    adap_err_log;   /* adapter error log struct */
} tok_stats_t;

   typedef struct {
       cio_stats_t cc;
       tok_stats_t ds;
   } tok_query_stats_t;

/* -------------------------------------------------------------------- */
/*  Token-Ring VPD structure and status codes                           */
/* -------------------------------------------------------------------- */

#define TOK_VPD_VALID      0x00    /* VPD obtained is valid */
#define TOK_VPD_NOT_READ   0x01    /* VPD has not been read from adapter */
#define TOK_VPD_INVALID    0x02    /* VPD obtained is invalid */
#define TOK_VPD_LENGTH     0x67    /* VPD length of  bytes */

typedef struct TOK_VPD
{
   unsigned long   status;      /* status of VPD */
   unsigned long   l_vpd;       /* length of VPD returned (may be <= TOK_VPD_LENGTH) */
   unsigned char   vpd[TOK_VPD_LENGTH];    /* VPD */
} tok_vpd_t;


/* -------------------------------------------------------------------- */
/*  Structure for the TOK_ADAP_INFO ioctl.                              */
/* -------------------------------------------------------------------- */
typedef struct RING_INFO            /* Token-Ring Information */
{
   unsigned short  adap_phys_addr[2];      /* Adapter Physical Address */
   unsigned short  upstream_node_addr[3];  /* Upstream Node Address */
   unsigned short  upstream_phys_addr[2];  /* Upstream Physical Addr */
   unsigned short  last_poll_addr[3];  /* Last Poll Address */
   unsigned short  author_env;         /* Authorized Environment */
   unsigned short  tx_access_prior;    /* Transmit Access Priority */
   unsigned short  src_class_author;   /* Source Class Authorization */
   unsigned short  last_atten_code;    /* Last Attention Code */
   unsigned short  last_src_addr[3];   /* Last Source Address */
   unsigned short  last_bcon_type;     /* Last Beacon Type */
   unsigned short  last_maj_vector;    /* Last Major Vector */
   unsigned short  ring_status;        /* Ring Status */
   unsigned short  sft_err_time_val;   /* Soft Error Timer Value */
   unsigned short  front_end_time_val; /* Front End Timer Value */
   unsigned short  res1;               /* Reserved */
   unsigned short  monitor_err_code;   /* Monitor Error Code */
   unsigned short  bcon_tx_type;       /* Beacon Transmit Type */
   unsigned short  bcon_rcv_type;      /* Beacon Receive Type */
   unsigned short  frame_corr_save;    /* Frame Correlator Save */
   unsigned short  bcon_station_naun[3];   /* Beaconing Station NAUN */
   unsigned short  res2[2];                /* Reserved */
   unsigned short  bcon_station_phys_addr[2];  /* Beaconing Station */
                                               /* Physical Address */
} tok_ring_info_t;
#define TOK_RING_INFO_SIZE (0x44)  /*
                                    * size of the Token-Ring
                                    * information structure
                                    */



typedef struct
{
   unsigned int    status;         /* Returned status */
   caddr_t         p_info;         /* location to put Ring Information */
   unsigned int    l_buf;          /* length of buffer for Ring Info. */
} tok_q_ring_info_t;


/* -------------------------------------------------------------------- */
/*      Structure for Set Adapter Open Options Ioctl                    */
/* -------------------------------------------------------------------- */
typedef struct SET_OPEN_OPTS
{
   unsigned int     status;
   unsigned short   options;
   short            buf_size;
   char             xmit_buf_min_cnt;
   char             xmit_buf_max_cnt;
   unsigned short   i_addr1;
   unsigned short   i_addr2;
   unsigned short   i_addr3;
} tok_set_open_opts_t;


/* -------------------------------------------------------------------- */
/*      Structure for Access POS Registers Ioctl                        */
/* -------------------------------------------------------------------- */
typedef struct TOK_POS_REG
{
   unsigned int         status;
   unsigned short       opcode;
   unsigned char        pos_reg;
   unsigned char        pos_val;
} tok_pos_reg_t;



/* -------------------------------------------------------------------- */
/*      Structure for Set Adapter Initialization Parameters Ioctl       */
/* -------------------------------------------------------------------- */


typedef struct SET_ADAP_I_PARMS
{
   unsigned int        status;
   unsigned short      init_options;
   unsigned short      rcv_burst_size;
   unsigned short      xmit_burst_size;
   unsigned short      dma_abort_thresh;
} tok_set_adap_i_parms_t;


/* -------------------------------------------------------------------- */
/*  Group Address ioctl structure                                       */
/* -------------------------------------------------------------------- */

typedef struct GROUP_ADDR
{
   unsigned int        status;         /* Returned status code */
   unsigned int        opcode;         /* Add or delete group address */
   unsigned int        group_addr;     /* group address */
}  tok_group_addr_t;

/* -------------------------------------------------------------------- */
/*  Functional Address ioctl structure                                  */
/* -------------------------------------------------------------------- */

typedef struct FUNC_ADDR
{
   unsigned int        status;         /* Returned status code */
   unsigned int        opcode;         /* Add or delete func. address */
   unsigned int        func_addr;      /* functional address */
   netid_t             netid;          /* Net ID for this functional address */
}  tok_func_addr_t;

/* -------------------------------------------------------------------- */
/*      Structure for Download Adapter Microcode Ioctl                  */
/* -------------------------------------------------------------------- */

typedef struct DOWNLOAD_UCODE
{
   unsigned int status;                 /* Returned status */
   char        *p_mcload;               /* microcode loader image pointer */
   int         l_mcload;                /* microcode loader length    */
   char        *p_mcode;                /* microcode image pointer */
   int         l_mcode;                 /* microcode length    */
} tok_download_t;


/* -------------------------------------------------------------------- */
/*  Returned Status Code Defines                                        */
/*  OR in the TOK_PREFIX to generate a unique status code.              */
/* -------------------------------------------------------------------- */


#define TOK_ADAP_INIT_PARMS_FAIL       (TOK_PREFIX | 0x8830)
#define TOK_ADAP_INIT_FAIL      (TOK_PREFIX | 0x8842)
#define TOK_ADAP_INIT_TIMEOUT   (TOK_PREFIX | 0x8832)
#define TOK_LOBE_MEDIA_TST_FAIL (TOK_PREFIX | 0x4210)
#define TOK_PHYS_INSERT         (TOK_PREFIX | 0x4220)
#define TOK_ADDR_VERIFY_FAIL    (TOK_PREFIX | 0x4230)
#define TOK_RING_POLL           (TOK_PREFIX | 0x4240)
#define TOK_REQ_PARMS           (TOK_PREFIX | 0x4250)

#define TOK_ADAP_CONFIG         (TOK_PREFIX | 0x4260)


#define TOK_NO_GROUP            (TOK_PREFIX | 0x0210)
#define TOK_NO_POS              (TOK_PREFIX | 0x0220)
#define TOK_NO_PARMS            (TOK_PREFIX | 0x0550)
#define TOK_NO_RING_INFO        (TOK_PREFIX | 0x0660)
#define TOK_NOT_DIAG_MODE       (TOK_PREFIX | 0xd000)
#define TOK_NO_DOWNLOAD         (TOK_PREFIX | 0xd001)
#define TOK_BAD_UCODE_LEVEL     (TOK_PREFIX | 0xd003)
#define TOK_LOADER_FAIL         (TOK_PREFIX | 0xd004)
#define TOK_UCODE_FAIL          (TOK_PREFIX | 0xd005)
#define TOK_BAD_RANGE           (CIO_BAD_RANGE)
#define TOK_INV_CMD             (CIO_INV_CMD)

#define TOK_LOBE_WIRE_FAULT     (TOK_PREFIX | 0x0800)
#define TOK_AUTO_REMOVE         (TOK_PREFIX | 0x0400)
#define TOK_ADAP_CHECK          (TOK_PREFIX | 0x0102)
#define TOK_RING_STATUS         (TOK_PREFIX | 0x0104)
#define TOK_IMPL_FORCE          (TOK_PREFIX | 0x0108)
#define TOK_CMD_FAIL            (TOK_PREFIX | 0x0110)
#define TOK_SIGNAL_LOSS         (TOK_PREFIX | 0x0112)
#define TOK_REMOVED_RECEIVED    (TOK_PREFIX | 0x0114)
#define TOK_TX_ERROR            (TOK_PREFIX | 0x0118)
#define TOK_PIO_FAIL            (TOK_PREFIX | 0x0200)
#define TOK_RCVRY_THRESH        (TOK_PREFIX | 0x0202)
#define TOK_MC_ERROR            (TOK_PREFIX | 0x0204)
#define TOK_RING_BEACONING	(TOK_PREFIX | 0x0208)
#define TOK_RING_RECOVERED	(TOK_PREFIX | 0x0212)

/*------------------------------------------------------------------------*/
/*                      TOKEN RING FRAME DEFINITIONS                      */
/*------------------------------------------------------------------------*/

/* Access Control Field: */

# define AC_PRIORITY    0xE0    /* Mask for Token Priority */
# define AC_TOK         0x10    /* Mask for Token/Frame flag */
# define AC_MONITOR     0x80    /* Mask for Monitor field */
# define AC_RES         0x07    /* Mask for Reservation level */

/* Frame Control Field: */

# define FC_TYPE        0xC0    /* Mask for type field: */
# define MAC_TYPE       0x00    /*      Medium Access Control Frame type */
# define LLC_TYPE       0x40    /*      Logical Link Control Frame type */

# define FC_CONTROL     0x3F    /* Mask for control field: */
# define MAC_DAT        0x00    /*      Duplicate Address Test MAC frame */
# define MAC_BCN        0x02    /*      Beacon MAC frame */
# define MAC_CL_TK      0x03    /*      Claim Token MAC frame */
# define MAC_PRG        0x04    /*      Purge MAC frame */
# define MAC_AMP        0x05    /*      Active Monitor Present MAC frame */
# define MAC_SMP        0x06    /*      Standby Monitor Present MAC frame */




/* -------------------------------------------------------------------- */
/*         Product ID Information Structure                             */
/* -------------------------------------------------------------------- */

/*
 *  The following Product ID Information is taken from the
 *  Token-Ring Network Architecture Reference manual in
 *  the MAC Frames section.
 */
typedef struct PROD_ID
{
   unsigned char   hardware;      /*
                                   * Bits 0-3 - Reserved
                                   * Bits 4-7 - Product classification
                                   *       0x1:    IBM Hardware
                                   *       0x3:    IBM or non-IBM HW
                                   *       0x4:    IBM software
                                   *       0x9:    Non-IBM hardware
                                   *       0xc:    Non-IBM software
                                   *       0xe:    IBM or non-IBM SW
                                   */

   unsigned char   format_type;   /*
                                   * Format type:
                                   *   0x10 - Product instance is
                                   *       identified by a serial number
                                   *       (that is, IBM plant of manufacture
                                   *       and sequence number) unique by
                                   *       machine type.
                                   *
                                   *   0x11 - Product instance is
                                   *       identified by a serial number
                                   *       unique by machine type and
                                   *       model number.
                                   *
                                   *   0x12 - Product instance is identified
                                   *       by machine type (as in Format 0x10)
                                   *       This format provides the model
                                   *       number not to identify a product
                                   *       instance uniquely, but for
                                   *       additional information only.
                                   */


   unsigned char   machine_type[4];
                                          /*
                                           * Machine type: 4 numeric EBCDIC
                                           * characters
                                           */


   unsigned char   mach_model_num[3];     /*
                                           *  Machine model number: 3 upper
                                           *  case alphanumeric EBCDIC chars
                                           *  for format types 0x11 and 0x12;
                                           *  these bytes are reserved by IBM
                                           *  future use in format type 0x10.
                                           */

   unsigned char   sn_modifier[2];        /*
                                           *  Serial number modifier -
                                           *  IBM plant of manufacture: 2
                                           *  numeric EBCDIC characters.
                                           */

   unsigned char   seq_num[7];            /*
                                           *  Sequence number: 7 upper case
                                           *  alphanumeric EBCDIC characters,
                                           *  right justified with EBCDIC zeros
                                           *  (0xf0) fill on the left.
                                           */


} tok_prod_id_t;



/* -------------------------------------------------------------------- */
/*                         DDI Structure                                */
/* -------------------------------------------------------------------- */
typedef struct DDI
{
   int   bus_type;       /* the bus type */
   int   bus_id;         /* the bus id */
   int   bus_int_lvl;    /* the interrupt level */
   int   intr_priority;  /* for use with i_init */
   int   xmt_que_size;   /* one queue for the adapter shared by all opens */
   int   rec_que_size;   /* one for each open from a user process */
   int   sta_que_size;   /* one for each open from a user process */
   int   rdto;           /* Receive data transfer offset */

   unsigned int    slot;           /* card slot number */
   unsigned char   *bus_io_addr;   /* PIO bus address */
   unsigned int    dma_lvl;        /* DMA arbitration level */
   unsigned int    dma_base_addr;  /* DMA base address */
   unsigned int    dma_bus_length; /* length of DMA address space */

                                   /* adapter initialization paramters */
   unsigned char   ring_speed;     /* Ring Speed: 0=4Mb, 1=16Mb */
   unsigned short  open_options;   /* adapter open options */
   unsigned short  buf_size;       /* adapter buffer size */

   int             use_alt_addr;   /* non-zero => use the following net addr */

                           /* alternate addr may replace EPROM addr */
   unsigned char   alt_addr[TOK_NADR_LENGTH];

   tok_prod_id_t   prod_id_if;    /* Product ID Information */

   char		   dev_name[16];  /* logical name of device */

} ddi_t;

/* -------------------------------------------------------------------- */
/*             Token-Ring adapter possible PIO addresses                */
/* -------------------------------------------------------------------- */

#define PIO_86A0   0x86a0
#define PIO_96A0   0x96a0
#define PIO_A6A0   0xa6a0
#define PIO_B6A0   0xb6a0
#define PIO_C6A0   0xc6a0
#define PIO_D6A0   0xd6a0
#define PIO_E6A0   0xe6a0
#define PIO_F6A0   0xf6a0
#endif /* ! _H_TOKUSER */
