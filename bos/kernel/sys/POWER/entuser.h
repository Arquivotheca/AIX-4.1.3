/* @(#)67	1.17.1.1  src/bos/kernel/sys/POWER/entuser.h, diagddent, bos411, 9438B411a 9/19/94 18:11:26 */
#ifndef _H_entUSER
#define _H_entUSER

/*****************************************************************************/
/*                                                                           */
/* COMPONENT_NAME: sysxent -- Ethernet Communications Code Device Driver     */
/*                                                                           */
/* FUNCTIONS: entuser.h                                                      */
/*                                                                           */
/* ORIGINS: 27                                                               */
/*                                                                           */
/* (C) COPYRIGHT International Business Machines Corp. 1990                  */
/* All Rights Reserved                                                       */
/* Licensed Materials - Property of IBM                                      */
/*                                                                           */
/* US Government Users Restricted Rights - Use, duplication or               */
/* disclosure restricted by GSA ADP Schedule Contract with IBM Corp.         */
/*                                                                           */
/*****************************************************************************/


/*****************************************************************************/
#define ent_MAX_ADAPTERS  (8)       /* maximum adapters in system            */
#define ent_NADR_LENGTH   (6)       /* bytes in hardware network address     */
#define ent_MIN_PACKET    (60)      /* minimum packet accepted               */
#define ent_MAX_PACKET    (1514)    /* maximum packet accepted               */
#define ent_MAX_GATHERS   (6)       /* maximum areas gathered for kernel wrt */
#define ent_MAX_OPENS     (6)       /* maximum opens per adapter             */
#define MAX_NETID         (32)      /* maximum Network ID's                  */

#define ENT_SET_MULTI     (CCC_IOCTL_MAX+0x01) /* set multicast address      */

#define ENT_CFG     	  (CCC_IOCTL_MAX+0x02) /* configure the ethernet hw  */
#define ENT_NOP     	  (CCC_IOCTL_MAX+0x03) /* NOP command */
#define ENT_POS     	  (CCC_IOCTL_MAX+0x04) 
#define ENT_SELFTEST   	  (CCC_IOCTL_MAX+0x05) /* ethernet self-test command */
#define ENT_DUMP   	  (CCC_IOCTL_MAX+0x06) /* ethernet dump command */

#define ENT_ALOC_BUF      (CCC_IOCTL_MAX+0x21) /* allocate dma buffer        */
#define ENT_COPY_BUF      (CCC_IOCTL_MAX+0x22) /* copy dma buffer            */
#define ENT_FREE_BUF      (CCC_IOCTL_MAX+0x23) /* free dma buffer            */
#define ENT_CONF_BUF      (CCC_IOCTL_MAX+0x24) /* Get Config values for DMA  */
#define ENT_LOCK_DMA      (CCC_IOCTL_MAX+0x25) /* Config DMA for user buffer */
#define ENT_UNLOCK_DMA    (CCC_IOCTL_MAX+0x26) /* Release DMA for user buffer*/
#define ENT_PROMISCUOUS_ON   (CCC_IOCTL_MAX+0x29) /* Enable promiscuous mode */
#define ENT_PROMISCUOUS_OFF  (CCC_IOCTL_MAX+0x2A) /* Disable promiscuous mode*/
#define ENT_BADFRAME_ON   (CCC_IOCTL_MAX+0x2B) /* Enable bad frame reception */
#define ENT_BADFRAME_OFF  (CCC_IOCTL_MAX+0x2C) /* Disable bad frame reception*/
#define ENT_RCV_SIG       (CCC_IOCTL_MAX+0x2D) /* Signal on receive */


/* device-specific statistics for RAS.ds section of DDS and CIO_QUERY ioctl  */
struct ent_stats {
   /* --------------------- these items are required for the common code     */
   ulong intr_lost;        /* interrupts lost due to full offlevel que       */
   ulong wdt_lost;         /* wdt intr lost due to full offlevel que         */
   long timo_lost;        /* timeout intr lost due to full offlevel que     */
   ulong sta_que_overflow; /* status lost due to full status que             */
   ulong rec_que_overflow; /* receive packet lost due to full recv que       */
   ulong rec_no_mbuf;      /* no mbuf available for received data            */
   ulong rec_no_mbuf_ext;  /* no mbuf extension available for received data  */
   ulong recv_intr_cnt;    /* number of receive interrupts                   */
   ulong xmit_intr_cnt;    /* number of transmit interrupts                  */
   /* --------------------- these optional items are device specific         */
   ulong crc_error;        /* CRC error count                                */
   ulong align_error;      /* Alignment error count                          */
   ulong overrun;          /* Receive overrun count                          */
   ulong too_short;        /* Packet too short count                         */
   ulong too_long;         /* Packet too long count                          */
   ulong no_resources;     /* Receive out of resources count                 */
   ulong pckts_discard;    /* Receive Packets Discarded count                */
   ulong max_collision;    /* Transmit Maximum Collisions count              */
   ulong late_collision;   /* A collision after the slot time elapsed	     */
   ulong carrier_lost;     /* Transmit Carrier Lost Count                    */
   ulong underrun;         /* Transmit Underrun count                        */
   ulong cts_lost;         /* Transmit Clear To Send Lost count              */
   ulong xmit_timeouts;    /* Transmit Timeouts count                        */
   ulong par_err_cnt;      /* Parity Error count                             */
   ulong diag_over_flow;   /* Diagnotic overflow error count                 */
   ulong exec_over_flow;   /* Execute q overflow error count                 */
   ulong exec_cmd_errors;  /* Execute command error count                    */
   ulong host_rec_eol;     /* Host side End of List Bit seen                 */
   ulong adpt_rec_eol;     /* Adapter/586 End of List bit seen               */
   ulong adpt_rec_pack;    /* adpt rec packets to be uploaded to host        */
   ulong host_rec_pack;    /* adpt rec packets actually upload to host       */
   ulong start_recp_cmd;   /* Start receptions commands issued to adapter    */
   ulong rec_dma_to;       /* Receive DMA time outs due to lock up           */
   ushort reserved[5];     /* 3com internal use only state variables  - HEX  */
};

typedef struct ent_stats ent_stats_t;

typedef struct {
   cio_stats_t cc;
   ent_stats_t ds;
   } ent_query_stats_t;

/* codes for use set multicast address ioctl                                 */
#define ENT_ADD           (1) /* Add the multicast address                   */
#define ENT_DEL           (2) /* Delete the multicast address                */
#define MULTI_BIT_MASK (0x01) /* Multicast Bit Mask                          */

/* data structure used with ethernet set multicast ioctl                     */
typedef struct {
   ushort opcode;          /* Add or Delete address                          */
   uchar  multi_addr[6];   /* Current network address                        */
} ent_set_multi_t;

/* data structure for ethernet diagnostic DMA Facility                       */
typedef struct {
   ulong  p_bus;           /* DMA address of the user buffer                 */
   uchar  *p_user;         /* Address of the user buffer                     */
   ulong  length;          /* Length of user buffer                          */
} ent_dma_buf_t;

/* ethernet exception code                                                   */
typedef enum {
   ENT_OK = CCC_EXCEPT_MAX+1,
   ENT_TX_MAX_COLLNS,      /* Transmit Maximum Collisions                 01 */
   ENT_TX_UNDERRUN,        /* Transmit FIFO Underrun                      02 */
   ENT_TX_CD_LOST,         /* Transmit Carrier Sense Lost                 03 */
   ENT_TX_CTS_LOST,        /* Transmit Clear to Send Lost                 04 */
   ENT_TX_TIMEOUT,         /* Transmit Time out                           05 */
   ENT_TX_BAD_SIZE,        /* Transmit Packet Too Large or Too Small      06 */
   ENT_TX_LATECOLL,        /* Transmit Late Collision			  07 */
   ENT_RX_CRC_ERROR,       /* Receive CRC Error                           01 */
   ENT_RX_OVERRUN,         /* Receive FIFO Over run                       02 */
   ENT_RX_ALIGN_ERROR,     /* Receive Alignment Error                     03 */
   ENT_RX_NO_RESOURSE,     /* Receive No Resources, buffers etc           04 */
   ENT_RX_TOO_SMALL,       /* Receive packet too short                    05 */
   ENT_RX_TOO_LARGE,       /* Receive packet too long                     06 */
   ENT_RX_COLL,		   /* Receive with collision			     */
   ENT_BAD_FRAME,	   /* bad frame reception. This is the option[0]     */
			   /* code in the status block for receiving bad     */
			   /* frames on the network			     */
   ENT_EXCEPT_MAX
} ENT_EXCEPT;




#endif /* ! _H_entUSER */
