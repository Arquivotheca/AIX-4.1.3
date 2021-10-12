/* @(#)41       1.1  src/bos/diag/tu/mps/mpstools.h, tu_mps, bos411, 9437B411a 8/23/94 16:27:10 */
/*****************************************************************************
 * COMPONENT_NAME: (tu_mps)  Wildwood LAN adapter test units
 *
 * FUNCTIONS: MPS Tools Header File
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *****************************************************************************/
#ifndef ADAPTERS
#define ADAPTERS

#define ADAPTER_COMMAND COMMAND
#define ADAPTER_RESPONSE char*
typedef unsigned long int ADAPTER_HANDLE;

#define ADAPTER_TIMEOUT_ERROR -1
/*
#define INTERRUPT_HANDLER_PATH    "/usr/lpp/htx/etc/kernext/mps_intr"
*/

/********************/
/*   Header Files   */
/********************/
#include <sys/xmem.h>
#include <stddef.h>
#include <sys/mbuf.h>

#include <sys/comio.h>
#include <sys/ciouser.h>

#include "getmps.h"
/********************************
 *     Constant Definitions
 ********************************/

/* these defines effect the DDS size                                         */
#define RAM_SIZE         (0x4000)   /* Adapter RAM size - 16 K               */
#define MAX_TX_LIST      (1)        /* Maximum Transmit frame list           */
#define MAX_RX_LIST      (2)        /* Maximum Receive  buffer list          */
#define RX_LIST_SIZE     (16)       /* size of receive elem                  */
#define TX_LIST_SIZE     (20)       /* size of transmit elem                 */

/**********************************
 *  Watchdog Timer Definitions
 *********************************/
typedef enum {                   /* Who set the watchdog timer            */
   WDT_INACTIVE,                 /* WDT is inactive                       */
   WDT_INIT = 0xD00,             /* Initialization is progress            */
   WDT_OPEN,                     /* Open in progress                      */
   WDT_XMIT,                     /* xmit in progress                      */
   WDT_COMMAND,                  /* command in progress                   */
   WDT_RECV,                     /* recv in progress                      */
   WDT_CLOSE                     /* Adapter close in progress             */
} WDT_SETTER;

typedef struct {
     struct watchdog  wd_timer;       /* watchdog timer structure            */
     ulong            bus_id;         /* Bus id                              */
     ulong            bus_io_addr;    /* Bus io address                      */
     int              int_level;      /* Interrupt level for INTTIMER        */
 } WDT_STRUCT;

/*****************************************************************************/
/*                  Transmit List type definition                            */
/*****************************************************************************/
 typedef struct TX_LIST {
  ulong       fw_pointer;       /* transmit forward pointer          */
  ulong       xmit_status;      /* transmit status                   */
  ushort      frame_len;        /* frame length                      */
  ushort      buf_count;        /* data buffer length                */
  ulong       data_pointer;     /* data buffer pointer               */
  ushort      buf_len;          /* frame length                      */
  ushort      reserved;         /* reserved                          */
  ulong       Z[3];
} tx_list_t;

struct xmit_elem {                      /* a transmit queue element     */
        struct mbuf     *mbufp;         /* pointer to mbuf with data    */
        chan_t          chan;           /* which opener this element is from */
        cio_write_ext_t wr_ext;         /* complete extension supplied  */
        short           bytes;          /* number of bytes in packet    */
        char            free;           /* flag to free mbuf            */
        char            in_use;         /* use flag                     */
        tx_list_t       *tx_ds;         /* xmit descriptor              */
};
typedef struct xmit_elem xmit_elem_t;

/*****************************************************************************/
/*                  Receive List type definition                             */
/*****************************************************************************/
typedef struct RX_LIST {
  ulong       fw_pointer;         /* receive forward pointer           */
  ulong       recv_status;        /* receive status                    */
  ulong       data_pointer;       /* data buffer pointer               */
  ushort      data_len;           /* data buffer length                */
  ushort      fr_len;             /* frame length                      */
} rx_list_t;


/**************************************************************/
/*               Main Structure For Adapter                   */
/**************************************************************/

typedef struct {
     diag_struc_t  *handle;           /* handle for adapter used by diagex   */
     mps_dds       dds;               /* device structure used by diagex     */
     char          *vpd_info;         /* vendor product data for adapter     */
     uint          card_id;           /* card id for adapter                 */
     int           error_log[100];    /* contains errors on adapter          */
     int           num_errors;        /* contains number of errors           */
     int           int_type[16];      /* interrupt type                      */
     int           num_interrupts;    /* number of interrupts                */
     ushort        cmd_reg;           /* command from bus master interrupt   */
     ushort        source_address[3]; /* source address for token ring       */
     ushort        dest_address[3];   /* dest address for token ring         */
     char          transmit_string[4096];  /* pointer to transmit string     */
     char          receive_string[4096];   /* pointer to receive string      */

     WDT_SETTER       wd_setter;      /* watchdog timer setter               */
     int   dma_channel;               /* for use with DMA services d_xxxx    */
     uchar   channel_alocd;           /* State of DMA_CHANNEL                */

     /* Address of LAP registers                                             */
     ushort  asb_address;                 /* address of ARB                  */
     ushort  srb_address;                 /* address of SRB                  */
     ushort  arb_address;                 /* address of ARB                  */
     ushort  trb_address;                 /* address of TRB                  */

     /*           Adapter Control Area control variables                     */
     struct xmem    rx_mem_block;
     struct xmem    tx_mem_block;
     struct xmem    rx_xmemp[MAX_RX_LIST];
     struct xmem    tx_xmemp[MAX_TX_LIST];
            uchar  *r_p_mem_block;        /* pointer to dynamic control block*/
            uchar  *t_p_mem_block;        /* pointer to dynamic control block*/

     /* for transmit                                                         */
     uchar      *xmit_buf;                /* pinned buffer of transmit data  */
     int         tx_buf_size;             /* size of transmit memory         */
     struct xmit_elem *xmit_queue;
     ushort      xmits_queued;         /* number of transmits on xmit queue */
     short       tx_list_next_buf;     /* Index to next in to sofware queue  */
     short       tx_list_next_in;      /* Index to next in for adapter       */
     short       tx_list_next_out;     /* next to be ACKed by adapter        */
     short       tx_tcw_use_count;     /* number of xmit buffers used        */
     tx_list_t  *xmit_list[MAX_TX_LIST];  /* transmit memory descriptor list */
     tx_list_t  *xmit_vadr[MAX_TX_LIST];  /* virt transmit memory list       */
     uchar      *xmit_mbuf[MAX_TX_LIST];  /* array of memory                 */
     uchar      *xmit_addr[MAX_TX_LIST];  /* dma addrs of memory             */

     /* Information for receive list                                         */
     char       *recv_buf;                /* pinned buffer for receive data */
     int         read_index;              /* receive buffer array index      */
     rx_list_t   *recv_list[MAX_RX_LIST]; /* Receive buffer descriptor list  */
     rx_list_t   *recv_vadr[MAX_RX_LIST]; /* virt Receive buffer list        */
     struct mbuf *recv_mbuf[MAX_RX_LIST]; /* array of mbufs                  */
     uchar       *recv_addr[MAX_RX_LIST]; /* dma addrs of mbufs              */
     struct mbuf *mhead;                  /* head of array of mbufs          */
     struct mbuf *mtail;                  /* tail of array of mbufs          */

} ADAPTER_STRUCT;

typedef struct {
     char *user_address;
     char *page_address;
     unsigned long dma_page;
     unsigned long dma_address;
} DMA_STRUCT;

DMA_STRUCT dma_allocate();

/*  Return Codes  */
#define XMFREE_FAIL      20
#define IINIT_FAIL       30
#define XMATTACH_FAIL    40
#define BOUND_FAIL       50
#define XMALLOC_FAIL     60
#define INTR_INIT_FAIL   70
#define KMOD_FAIL        80
#define PIN_FAIL         90
#define LENGTH_FAIL      100
#define TYPE_FAIL        110

#define OPEN_FAILED       120
#define DMA_FAILED        130
#define PINU_FAILED       140
#define PINCODE_FAILED    150
#define COPYIN_FAILED     160

#endif

